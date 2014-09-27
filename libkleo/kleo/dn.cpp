/*
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

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "libkleo/ui/dnattributeorderconfigwidget.h"

#include <kconfig.h>
#include <KLocalizedString>

#include <QStringList>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <map>

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <kconfiggroup.h>
#include <KSharedConfig>

class Kleo::DN::Private {
public:
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
  p = (char*)malloc (n+1);


  memcpy (p, string, n);
  p[n] = 0;
  trim_trailing_spaces ((char*)p);
  // map OIDs to their names:
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( !strcasecmp ((char*)p, oidmap[i].oid) ) {
      free( p );
      p = strdup( oidmap[i].name );
      break;
    }
  array->key = p;
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
    return QVector<Kleo::DN::Attribute>();

  QVector<Kleo::DN::Attribute> result;
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
  return QVector<Kleo::DN::Attribute>();
}

static QVector<Kleo::DN::Attribute>
parse_dn( const QString & dn ) {
  return parse_dn( (const unsigned char*)dn.toUtf8().data() );
}

static QString dn_escape( const QString & s ) {
    QString result;
    for ( unsigned int i = 0, end = s.length() ; i != end ; ++i ) {
        const QChar ch = s[i];
        switch ( ch.unicode() ) {
        case ',':
        case '+':
        case '"':
        case '\\':
        case '<':
        case '>':
        case ';':
            result += QLatin1Char('\\');
            // fall through
        default:
            result += ch;
        }
    }
    return result;
}

static QString
serialise( const QVector<Kleo::DN::Attribute> & dn, const QString & sep ) {
  QStringList result;
  for ( QVector<Kleo::DN::Attribute>::const_iterator it = dn.begin() ; it != dn.end() ; ++it )
    if ( !(*it).name().isEmpty() && !(*it).value().isEmpty() )
      result.push_back( (*it).name().trimmed() + QLatin1Char('=') + dn_escape( (*it).value().trimmed() ) );
  return result.join( sep );
}

static Kleo::DN::Attribute::List
reorder_dn( const Kleo::DN::Attribute::List & dn ) {
  const QStringList & attrOrder = Kleo::DNAttributeMapper::instance()->attributeOrder();

  Kleo::DN::Attribute::List unknownEntries;
  Kleo::DN::Attribute::List result;
  unknownEntries.reserve( dn.size() );
  result.reserve( dn.size() );

  // find all unknown entries in their order of appearance
  for ( Kleo::DN::const_iterator it = dn.begin(); it != dn.end(); ++it )
    if ( !attrOrder.contains( (*it).name() )  )
      unknownEntries.push_back( *it );

  // process the known attrs in the desired order
  for ( QStringList::const_iterator oit = attrOrder.begin() ; oit != attrOrder.end() ; ++oit )
    if ( *oit == QLatin1String("_X_") ) {
      // insert the unknown attrs
      std::copy( unknownEntries.begin(), unknownEntries.end(),
                 std::back_inserter( result ) );
      unknownEntries.clear(); // don't produce dup's
    } else {
      for ( Kleo::DN::const_iterator dnit = dn.begin() ; dnit != dn.end() ; ++dnit )
        if ( (*dnit).name() == *oit )
          result.push_back( *dnit );
    }

  return result;
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
    return QString();
  if ( d->reorderedAttributes.empty() )
    d->reorderedAttributes = reorder_dn( d->attributes );
  return serialise( d->reorderedAttributes, QLatin1String(",") );
}

QString Kleo::DN::dn() const {
  return d ? serialise( d->attributes, QLatin1String(",") ) : QString() ;
}

QString Kleo::DN::dn( const QString & sep ) const {
  return d ? serialise( d->attributes, sep ) : QString() ;
}

// static
QString Kleo::DN::escape( const QString & value ) {
    return dn_escape( value );
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
    return QString();
  const QString attrUpper = attr.toUpper();
  for ( QVector<Attribute>::const_iterator it = d->attributes.constBegin() ;
        it != d->attributes.constEnd() ; ++it )
    if ( (*it).name() == attrUpper )
      return (*it).value();
  return QString();
}

static QVector<Kleo::DN::Attribute> empty;

Kleo::DN::const_iterator Kleo::DN::begin() const {
  return d ? d->attributes.constBegin() : empty.constBegin() ;
}

Kleo::DN::const_iterator Kleo::DN::end() const {
  return d ? d->attributes.constEnd() : empty.constEnd() ;
}


/////////////////////

namespace {
  struct ltstr {
    bool operator()( const char * s1, const char * s2 ) const {
      return qstrcmp( s1, s2 ) < 0 ;
    }
  };
}

static const char *const defaultOrder[] = {
  "CN", "L", "_X_", "OU", "O", "C"
};

static std::pair<const char*,const char*> const attributeLabels[] = {
#define MAKE_PAIR(x,y) std::pair<const char*,const char*>( x, y )
  MAKE_PAIR( "CN", I18N_NOOP("Common name") ),
  MAKE_PAIR( "SN", I18N_NOOP("Surname") ),
  MAKE_PAIR( "GN", I18N_NOOP("Given name") ),
  MAKE_PAIR( "L",  I18N_NOOP("Location") ),
  MAKE_PAIR( "T",  I18N_NOOP("Title") ),
  MAKE_PAIR( "OU", I18N_NOOP("Organizational unit") ),
  MAKE_PAIR( "O",  I18N_NOOP("Organization") ),
  MAKE_PAIR( "PC", I18N_NOOP("Postal code") ),
  MAKE_PAIR( "C",  I18N_NOOP("Country code") ),
  MAKE_PAIR( "SP", I18N_NOOP("State or province") ),
  MAKE_PAIR( "DC", I18N_NOOP("Domain component") ),
  MAKE_PAIR( "BC", I18N_NOOP("Business category") ),
  MAKE_PAIR( "EMAIL", I18N_NOOP("Email address") ),
  MAKE_PAIR( "MAIL", I18N_NOOP("Mail address") ),
  MAKE_PAIR( "MOBILE", I18N_NOOP("Mobile phone number") ),
  MAKE_PAIR( "TEL", I18N_NOOP("Telephone number") ),
  MAKE_PAIR( "FAX", I18N_NOOP("Fax number") ),
  MAKE_PAIR( "STREET", I18N_NOOP("Street address") ),
  MAKE_PAIR( "UID", I18N_NOOP("Unique ID") )
#undef MAKE_PAIR
};
static const unsigned int numAttributeLabels = sizeof attributeLabels / sizeof *attributeLabels ;

class Kleo::DNAttributeMapper::Private {
public:
  Private();
  std::map<const char*,const char*,ltstr> map;
  QStringList attributeOrder;
};

Kleo::DNAttributeMapper::Private::Private()
  : map( attributeLabels, attributeLabels + numAttributeLabels ) {}

Kleo::DNAttributeMapper::DNAttributeMapper() {
  d = new Private();
  const KConfigGroup config( KSharedConfig::openConfig(), "DN" );
  d->attributeOrder = config.readEntry( "AttributeOrder" , QStringList() );
  if ( d->attributeOrder.empty() )
    std::copy( defaultOrder, defaultOrder + sizeof defaultOrder / sizeof *defaultOrder,
               std::back_inserter( d->attributeOrder ) );
  mSelf = this;
}

Kleo::DNAttributeMapper::~DNAttributeMapper() {
  mSelf = 0;
  delete d; d = 0;
}

Kleo::DNAttributeMapper * Kleo::DNAttributeMapper::mSelf = 0;

const Kleo::DNAttributeMapper * Kleo::DNAttributeMapper::instance() {
  if ( !mSelf )
    (void)new DNAttributeMapper();
  return mSelf;
}

QString Kleo::DNAttributeMapper::name2label( const QString & s ) const {
  const std::map<const char*,const char*,ltstr>::const_iterator it
    = d->map.find( s.trimmed().toUpper().toLatin1() );
  if ( it == d->map.end() )
    return QString();
  return i18n( it->second );
}

QStringList Kleo::DNAttributeMapper::names() const {
  QStringList result;
  for ( std::map<const char*,const char*,ltstr>::const_iterator it = d->map.begin() ; it != d->map.end() ; ++it )
    result.push_back( QLatin1String(it->first) );
  return result;
}

const QStringList & Kleo::DNAttributeMapper::attributeOrder() const {
  return d->attributeOrder;
}

void Kleo::DNAttributeMapper::setAttributeOrder( const QStringList & order ) {
  d->attributeOrder = order;
  if ( order.empty() )
    std::copy( defaultOrder, defaultOrder + sizeof defaultOrder / sizeof *defaultOrder,
               std::back_inserter( d->attributeOrder ) );
  KConfigGroup config( KSharedConfig::openConfig(), "DN" );
  config.writeEntry( "AttributeOrder", order );
}

Kleo::DNAttributeOrderConfigWidget * Kleo::DNAttributeMapper::configWidget( QWidget * parent ) const {
  return new DNAttributeOrderConfigWidget( mSelf, parent );
}
