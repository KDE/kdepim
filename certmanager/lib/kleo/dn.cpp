/*  -*- mode: C++; c-file-style: "gnu" -*-
    dn.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    DN parsing:
    Copyright (c) 2002 g10 Code GmbH
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "dn.h"

#include "oidmap.h"

#include <kapplication.h>
#include <kconfig.h>

#include <qstringlist.h>
#include <qvaluevector.h>

#include <iostream>
#include <iterator>
#include <algorithm>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

struct Kleo::DN::Private {
  Private() : mRefCount( 0 ) {}
  Private( const Private & other )
    : attributes( other.attributes ),
      reorderedAttributes( other.reorderedAttributes ),
      mRefCount( 0 )
  {

  }

  int ref() {
    return ++mRefCount;
  }

  int unref() {
    if ( --mRefCount <= 0 ) {
      delete this;
      return 0;
    } else
      return mRefCount;
  }

  int refCount() const { return mRefCount; }

  DN::Attribute::List attributes;
  DN::Attribute::List reorderedAttributes;
private:
  int mRefCount;
};

namespace {
  struct DnPair {
    char * key;
    char * value;
  };
}

// copied from CryptPlug and adapted to work on DN::Attribute::List:

// a little helper class for reordering of DN attributes
namespace {

  class DNBeautifier {
  public:
    enum UnknownAttrsHandling { Hide, Prefix, Postfix, Infix };
    // infix: at the position of "_X_", if any, else Postfix

    DNBeautifier();

    Kleo::DN::Attribute::List
    reorder( const Kleo::DN::Attribute::List & dn ) const {
      return reorder( dn, _attrOrder, _unknownAttrsHandling );
    }

    static Kleo::DN::Attribute::List
    reorder( const Kleo::DN::Attribute::List & dn,
	     const QStringList & attrOrder,
	     UnknownAttrsHandling unknownAttrsHandling );

  private:
    QStringList _attrOrder;
    UnknownAttrsHandling _unknownAttrsHandling;
  };

  
  static const char * defaultOrder[] = {
    "CN", "S", "SN", "GN", "T", "UID",
    "MAIL", "EMAIL", "MOBILE", "TEL", "FAX", "STREET",
    "L",  "PC", "SP", "ST",
    "OU",
    "O",
    "C"
  };

  DNBeautifier::DNBeautifier() : _unknownAttrsHandling( Infix ) {
    if ( !kapp ) {
      std::cerr << "WARNING: Kleo::DN::prettyDN() needs a KApplication running." << std::endl;
      return;
    }
    const KConfigGroup config( kapp->config(), "DN" );

    _attrOrder = config.readListEntry( "AttributeOrder" );
    if ( _attrOrder.empty() )
      std::copy( defaultOrder,
		 defaultOrder + sizeof defaultOrder / sizeof *defaultOrder,
		 std::back_inserter( _attrOrder ) );

    const QString str = config.readEntry( "UnknownAttributes", "INFIX" ).upper();
    if ( str == "HIDE" )
      _unknownAttrsHandling = Hide;
    else if ( str == "PREFIX" )
      _unknownAttrsHandling = Prefix;
    else if ( str == "POSTFIX" )
      _unknownAttrsHandling = Postfix;
    else
      _unknownAttrsHandling = Infix;
  }
  
  Kleo::DN::Attribute::List
  DNBeautifier::reorder( const Kleo::DN::Attribute::List & dn,
			 const QStringList & attrOrder,
			 UnknownAttrsHandling unknownAttrsHandling ) {
    if ( attrOrder.empty() )
      return dn;

    Kleo::DN::Attribute::List unknownEntries;
    Kleo::DN::Attribute::List result;
    unknownEntries.reserve( dn.size() );
    result.reserve( dn.size() );

    if ( unknownAttrsHandling != Hide )
      // find all unknown entries in their order of appearance
      for ( Kleo::DN::const_iterator it = dn.begin(); it != dn.end(); ++it )
	if ( attrOrder.find( (*it).name() ) == attrOrder.end() )
	  unknownEntries.push_back( *it );

    if ( unknownAttrsHandling == Prefix ) {
      // prepend the unknown attrs
      result = unknownEntries;
      unknownEntries.clear();
    }
      
    // process the known attrs in the desired order
    for ( QStringList::const_iterator oit = attrOrder.begin() ; oit != attrOrder.end() ; ++oit )
      if ( *oit == "_X_" ) {
	if ( unknownAttrsHandling == Infix ) {
	  // insert the unknown attrs
	  std::copy( unknownEntries.begin(), unknownEntries.end(),
		     std::back_inserter( result ) );
	  unknownEntries.clear();
	}
      } else {
	for ( Kleo::DN::const_iterator dnit = dn.begin() ; dnit != dn.end() ; ++dnit )
	  if ( (*dnit).name() == *oit )
	    result.push_back( *dnit );
      }
      
    // append the unknown attrs: since we cleared unknownEntries
    // whenever we inserted it, we avoid duplicate insertions anyway
    // and don't need to check whether we have done so already:
    std::copy( unknownEntries.begin(), unknownEntries.end(),
	       std::back_inserter( result ) );

    return result;
  }
  

} // anon namespace

#define digitp(p)   (*(p) >= '0' && *(p) <= '9')
#define hexdigitp(a) (digitp (a)                     \
                      || (*(a) >= 'A' && *(a) <= 'F')  \
                      || (*(a) >= 'a' && *(a) <= 'f'))
#define xtoi_1(p)   (*(p) <= '9'? (*(p)- '0'): \
                     *(p) <= 'F'? (*(p)-'A'+10):(*(p)-'a'+10))
#define xtoi_2(p)   ((xtoi_1(p) * 16) + xtoi_1((p)+1))

static char *
trim_trailing_spaces( char *string )
{
    char *p, *mark;

    for( mark = NULL, p = string; *p; p++ ) {
	if( isspace( *p ) ) {
	    if( !mark )
		mark = p;
	}
	else
	    mark = NULL;
    }
    if( mark )
	*mark = '\0' ;

    return string ;
}

/* Parse a DN and return an array-ized one.  This is not a validating
   parser and it does not support any old-stylish syntax; gpgme is
   expected to return only rfc2253 compatible strings. */
static const unsigned char *
parse_dn_part (DnPair *array, const unsigned char *string)
{
  const unsigned char *s, *s1;
  size_t n;
  char *p;

  /* parse attributeType */
  for (s = string+1; *s && *s != '='; s++)
    ;
  if (!*s)
    return NULL; /* error */
  n = s - string;
  if (!n)
    return NULL; /* empty key */
  array->key = p = (char*)malloc (n+1);
  
  
  memcpy (p, string, n);
  p[n] = 0;
  trim_trailing_spaces ((char*)p);
  // map OIDs to their names:
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( !strcmp ((char*)p, oidmap[i].oid) ) {
      strcpy ((char*)p, oidmap[i].name);
      break;
    }
  string = s + 1;

  if (*string == '#')
    { /* hexstring */
      string++;
      for (s=string; hexdigitp (s); s++)
        s++;
      n = s - string;
      if (!n || (n & 1))
        return NULL; /* empty or odd number of digits */
      n /= 2;
      array->value = p = (char*)malloc (n+1);
      
      
      for (s1=string; n; s1 += 2, n--)
        *p++ = xtoi_2 (s1);
      *p = 0;
   }
  else
    { /* regular v3 quoted string */
      for (n=0, s=string; *s; s++)
        {
          if (*s == '\\')
            { /* pair */
              s++;
              if (*s == ',' || *s == '=' || *s == '+'
                  || *s == '<' || *s == '>' || *s == '#' || *s == ';' 
                  || *s == '\\' || *s == '\"' || *s == ' ')
                n++;
              else if (hexdigitp (s) && hexdigitp (s+1))
                {
                  s++;
                  n++;
                }
              else
                return NULL; /* invalid escape sequence */
            }
          else if (*s == '\"')
            return NULL; /* invalid encoding */
          else if (*s == ',' || *s == '=' || *s == '+'
                   || *s == '<' || *s == '>' || *s == '#' || *s == ';' )
            break; 
          else
            n++;
        }

      array->value = p = (char*)malloc (n+1);
      
      
      for (s=string; n; s++, n--)
        {
          if (*s == '\\')
            { 
              s++;
              if (hexdigitp (s))
                {
                  *p++ = xtoi_2 (s);
                  s++;
                }
              else
                *p++ = *s;
            }
          else
            *p++ = *s;
        }
      *p = 0;
    }
  return s;
}


/* Parse a DN and return an array-ized one.  This is not a validating
   parser and it does not support any old-stylish syntax; gpgme is
   expected to return only rfc2253 compatible strings. */
static Kleo::DN::Attribute::List
parse_dn( const unsigned char * string ) {
  if ( !string )
    return QValueVector<Kleo::DN::Attribute>();

  QValueVector<Kleo::DN::Attribute> result;
  while (*string)
    {
      while (*string == ' ')
        string++;
      if (!*string)
        break; /* ready */

      DnPair pair = { 0, 0 };
      string = parse_dn_part (&pair, string);
      if (!string)
	goto failure;
      if ( pair.key && pair.value )
	result.push_back( Kleo::DN::Attribute( QString::fromUtf8( pair.key ),
					       QString::fromUtf8( pair.value ) ) );
      free( pair.key );
      free( pair.value );

      while (*string == ' ')
        string++;
      if (*string && *string != ',' && *string != ';' && *string != '+')
        goto failure; /* invalid delimiter */
      if (*string)
        string++;
    }
  return result;

failure:
  return QValueVector<Kleo::DN::Attribute>();
}

static QValueVector<Kleo::DN::Attribute>
parse_dn( const QString & dn ) {
  return parse_dn( (const unsigned char*)dn.utf8().data() );
}

static QString
serialise( const QValueVector<Kleo::DN::Attribute> & dn ) {
  QStringList result;
  for ( QValueVector<Kleo::DN::Attribute>::const_iterator it = dn.begin() ; it != dn.end() ; ++it )
    if ( !(*it).name().isEmpty() && !(*it).value().isEmpty() )
      result.push_back( (*it).name().stripWhiteSpace() + '=' + (*it).value().stripWhiteSpace() );
  return result.join( "," );
}

static QValueVector<Kleo::DN::Attribute>
reorder_dn( const QValueVector<Kleo::DN::Attribute> & dn ) {
  static const DNBeautifier beautifier;
  return beautifier.reorder( dn );
}


//
//
// class DN
//
//

Kleo::DN::DN() {
  d = new Private();
  d->ref();
}

Kleo::DN::DN( const QString & dn ) {
  d = new Private();
  d->ref();
  d->attributes = parse_dn( dn );
}

Kleo::DN::DN( const char * utf8DN ) {
  d = new Private();
  d->ref();
  if ( utf8DN )
    d->attributes = parse_dn( (const unsigned char*)utf8DN );
}

Kleo::DN::DN( const DN & other )
  : d( other.d )
{
  if ( d ) d->ref();
}

Kleo::DN::~DN() {
  if ( d ) d->unref();
}

const Kleo::DN & Kleo::DN::operator=( const DN & that ) {
  if ( this->d == that.d )
    return *this;

  if ( that.d )
    that.d->ref();
  if ( this->d )
    this->d->unref();

  this->d = that.d;

  return *this;
}

QString Kleo::DN::prettyDN() const {
  if ( !d )
    return QString::null;
  if ( d->reorderedAttributes.empty() )
    d->reorderedAttributes = reorder_dn( d->attributes );
  return serialise( d->reorderedAttributes );
}

QString Kleo::DN::dn() const {
  return d ? serialise( d->attributes ) : QString::null ;
}

void Kleo::DN::detach() {
  if ( !d ) {
    d = new Kleo::DN::Private();
    d->ref();
  } else if ( d->refCount() > 1 ) {
    Kleo::DN::Private * d_save = d;
    d = new Kleo::DN::Private( *d );
    d->ref();
    d_save->unref();
  }
}

void Kleo::DN::append( const Attribute & attr ) {
  detach();
  d->attributes.push_back( attr );
  d->reorderedAttributes.clear();
}

QString Kleo::DN::operator[]( const QString & attr ) const {
  if ( !d )
    return QString::null;
  const QString attrUpper = attr.upper();
  for ( QValueVector<Attribute>::const_iterator it = d->attributes.begin() ;
	it != d->attributes.end() ; ++it )
    if ( (*it).name() == attrUpper )
      return (*it).value();
  return QString::null;
}

static QValueVector<Kleo::DN::Attribute> empty;

Kleo::DN::const_iterator Kleo::DN::begin() const {
  return d ? d->attributes.begin() : empty.begin() ;
}

Kleo::DN::const_iterator Kleo::DN::end() const {
  return d ? d->attributes.end() : empty.end() ;
}
