/*
    This file is part of KAddressbook.
    Copyright (c) 2000 - 2003 Oliver Strutynski <olistrut@gmx.de>
                              Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>

#include <kdebug.h>

#include "ldif_xxport.h"

class LDIFXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABCore *core, QObject *parent, const char *name )
    {
      return new LDIFXXPort( core, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_ldif_xxport()
  {
    return ( new LDIFXXPortFactory() );
  }
}


LDIFXXPort::LDIFXXPort( KABCore *core, QObject *parent, const char *name )
  : XXPortObject( core, parent, name )
{
  createImportAction( i18n( "Import LDIF..." ) );
}

KABC::AddresseeList LDIFXXPort::importContacts( const QString& ) const
{
  QString fileName = KFileDialog::getOpenFileName( QDir::homeDirPath(), "*.ldif *.LDIF", 0 );
  if ( fileName.isEmpty() )
    return KABC::AddresseeList();

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>" );
    KMessageBox::error( core(), msg.arg( fileName ) );
    return KABC::AddresseeList();
  }

  QString empty;
  QTextStream t( &file );
  QString s, completeline, fieldname;

  // Set to true if data currently read is part of
  // a list of names. Lists will be ignored.
  bool isGroup = false;
  bool lastWasComment = false;
  int numEntries = 0;

  KABC::Addressee *a = new KABC::Addressee();
  KABC::Address *addr = new KABC::Address();
  KABC::AddresseeList addrList;
  while ( !t.eof() ) {
    s = t.readLine();
    completeline = s;

    if ( s.isEmpty() && t.eof() ) {
      // Newline: Write data
writeData:
      if ( !isGroup ) {
        if ( !a->formattedName().isEmpty() && a->emails().count() > 0 ) {
          if ( !addr->isEmpty() )
            a->insertAddress( *addr );
          addrList.append( *a );
          delete a;
          delete addr;
          a = new KABC::Addressee();
          addr = new KABC::Address();
        }
      }

      isGroup = false;
      lastWasComment = false;
      continue;
    }
	
    int pos = s.find( "::" );
    if ( pos != -1 ) {
      // String is BASE64 encoded
      fieldname = s.left( pos ).lower();
      s = QString::fromUtf8( KCodecs::base64Decode(
                             s.mid( pos + 3, s.length() - pos - 2 ).latin1() ) )
                             .simplifyWhiteSpace();
    } else {
      pos = s.find( ":" );
      if ( pos != -1 ) {
        fieldname = s.left( pos ).lower();
        // Convert Utf8 string to unicode so special characters are preserved
        // We need this since we are reading normal strings from the file
        // which are not converted automatically
        s = QString::fromUtf8( s.mid( pos + 2, s.length() - pos - 2 ).latin1() );
      } else {
        fieldname = QString::null;
        s = QString::fromUtf8( s );
      }
    }

    if ( s.stripWhiteSpace().isEmpty() )
      continue;

    if ( fieldname.isEmpty() ) {
      if ( lastWasComment ) {
        // if the last entry was a comment, add this one too, since
        // we might have a multi-line comment entry.
        if ( !a->note().isEmpty() )
          a->setNote( a->note() + "\n" );
          a->setNote( a->note() + s );
      }
      continue;
    }
    lastWasComment = false;

    if ( fieldname == "givenname" ) {
      a->setGivenName( s );
      continue;
    }

    if ( fieldname == "xmozillanickname" ) {
      a->setNickName( s );
      continue;
    }

    if ( fieldname == "dn" ) // ignore
      goto writeData;

    if ( fieldname == "sn" ) {
      a->setFamilyName( s );
      continue;
    }

    if ( fieldname == "mail" ) {
      a->insertEmail( s );
      continue;
    }

    if ( fieldname == "title" ) {
      a->setTitle( s );
      continue;
    }

    if ( fieldname == "cn" ) {
      a->setFormattedName( s );
      continue;
    }

    if ( fieldname == "o" ) {
      a->setOrganization( s );
      continue;
    }

    if ( fieldname == "description" ) {
      a->setNote( s );
      lastWasComment = true;
      continue;
    }

    if ( fieldname == "homeurl" ) {
      a->setUrl( s );
      continue;
    }

    if ( fieldname == "homephone" || fieldname == "telephonenumber" ) {
      a->insertPhoneNumber( KABC::PhoneNumber ( s, KABC::PhoneNumber::Home ) );
      continue;
    }

    if ( fieldname == "postalcode" ) {
      addr->setPostalCode( s );
      continue;
    }

    if ( fieldname == "facsimiletelephonenumber" ) {
      a->insertPhoneNumber( KABC::PhoneNumber ( s, KABC::PhoneNumber::Fax ) );
      continue;
    }

    if ( fieldname == "streetaddress" ) {
      addr->setStreet( s );
      continue;
    }

    if ( fieldname == "locality" ) {
      addr->setLocality( s );
      continue;
    }

    if ( fieldname == "countryname" ) {
      addr->setCountry( s );
      continue;
    }

    if ( fieldname == "cellphone" ) {
      a->insertPhoneNumber( KABC::PhoneNumber ( s, KABC::PhoneNumber::Cell ) );
      continue;
    }

    if ( fieldname == "st" ) {
      addr->setRegion( s );
      continue;
    }

    if ( fieldname == "ou" ) {
      a->setRole( s );
      continue;
    }

    if ( fieldname == "objectclass" && s == "groupOfNames" )
      isGroup = true;
  }

  delete a;
  delete addr;

  file.close();

  return addrList;
}

#include "ldif_xxport.moc"
