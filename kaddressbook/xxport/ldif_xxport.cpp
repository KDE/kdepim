/*
    This file is part of KAddressbook.
    Copyright (c) 2000 - 2002 Oliver Strutynski <olistrut@gmx.de>
    Copyright (c) 2002 - 2003 Helge Deller <deller@kde.org>
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

/* 
    Description:
    The LDAP Data Interchange Format (LDIF) is a common ASCII-text based
    Internet interchange format. Most programs allow you to export data in
    LDIF format and e.g. Netscape and Mozilla store by default their
    Personal Address Book files in this format.
    This import and export filter reads and writes any LDIF version 1 files
    from your KDE Addressbook.
*/

#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kabc/vcardconverter.h>  // for time-conversion functions

#include <kdebug.h>

#include "ldif_xxport.h"

class LDIFXXPortFactory : public XXPortFactory
{
  public:
    XXPortObject *xxportObject( KABC::AddressBook *ab, QWidget *parent, const char *name )
    {
      return new LDIFXXPort( ab, parent, name );
    }
};

extern "C"
{
  void *init_libkaddrbk_ldif_xxport()
  {
    return ( new LDIFXXPortFactory() );
  }
}


LDIFXXPort::LDIFXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : XXPortObject( ab, parent, name )
{
  createImportAction( i18n( "Import LDIF Addressbook..." ) );
  createExportAction( i18n( "Export LDIF Addressbook..." ) );
}

/* import */

KABC::AddresseeList LDIFXXPort::importContacts( const QString& ) const
{
  KABC::AddresseeList addrList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
                      "*.[lL][dD][iI][fF]|" + i18n( "LDIF file (*.ldif)" ), 0 );
  if ( fileName.isEmpty() )
    return addrList;

  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>" );
    KMessageBox::error( parentWidget(), msg.arg( fileName ) );
    return addrList;
  }

  QDateTime dtDefault;
  dtDefault = QFileInfo(file).lastModified();
  if ( !dtDefault.isValid() )
    dtDefault = QDateTime::currentDateTime();
  
  QTextStream t( &file );
  t.setEncoding( QTextStream::Latin1 );
  QString s, completeline, fieldname;

  bool lastWasComment = false;

  KABC::Addressee a;
  a.setRevision(dtDefault);
  KABC::Address homeAddr( KABC::Address::Home );
  KABC::Address workAddr( KABC::Address::Work ); 
  while ( !t.eof() ) {
    s = t.readLine();
    completeline = s;
	
    if ( s.isEmpty() && t.eof() ) {
      // Newline: Write data
writeData:
      if ( !a.formattedName().isEmpty() || !a.name().isEmpty() || 
           !a.familyName().isEmpty() ) {
        if ( !homeAddr.isEmpty() )
          a.insertAddress( homeAddr );
        if ( !workAddr.isEmpty() )
          a.insertAddress( workAddr );
        addrList.append( a );
      }

      // delete old and create a new empty address entry
      a = KABC::Addressee();
      a.setRevision(dtDefault);
      homeAddr = KABC::Address( KABC::Address::Home );
      workAddr = KABC::Address( KABC::Address::Work );

      lastWasComment = false;
      continue;
    }
	
    int position = s.find( "::" );
    if ( position != -1 ) {
      // String is BASE64 encoded
      fieldname = s.left( position ).lower();
      s = QString::fromUtf8( KCodecs::base64Decode(
                             s.mid( position + 3, s.length() - position - 2 ).latin1() ) )
                             .simplifyWhiteSpace();
    } else {
      position = s.find( ":" );
      if ( position != -1 ) {
        fieldname = s.left( position ).lower();
        // Convert Utf8 string to unicode so special characters are preserved
        // We need this since we are reading normal strings from the file
        // which are not converted automatically
        s = QString::fromUtf8( s.mid( position + 2, s.length() - position - 2 ).latin1() );
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
addComment:
        if ( !a.note().isEmpty() )
          a.setNote( a.note() + "\n" );
        a.setNote( a.note() + s );
      }
      continue;
    }
    lastWasComment = false;

    if ( fieldname == QString::fromLatin1( "givenname" ) ) {
      a.setGivenName( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "xmozillanickname" ) ) {
      a.setNickName( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "dn" ) ) // ignore
      goto writeData;

    if ( fieldname == QString::fromLatin1( "sn" ) ) {
      a.setFamilyName( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mail" ) ) {
      a.insertEmail( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillasecondemail" ) ) { // mozilla
      a.insertEmail( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "title" ) ) {
      a.setTitle( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "cn" ) ) {
      a.setFormattedName( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "o" ) ) {
      a.setOrganization( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "description" ) ) {
      lastWasComment = true;
      goto addComment;
    }

    if ( fieldname == QString::fromLatin1( "custom1" ) ||
         fieldname == QString::fromLatin1( "custom2" ) ||
         fieldname == QString::fromLatin1( "custom3" ) ||
         fieldname == QString::fromLatin1( "custom4" ) )
      goto addComment;

    if ( fieldname == QString::fromLatin1( "homeurl" ) ||
         fieldname == QString::fromLatin1( "workurl" ) ) {
      if (a.url().isEmpty()) {
        a.setUrl( s );
        continue;
      }
      if ( a.url().prettyURL() == KURL(s).prettyURL() )
        continue;
      // only one URL possible with kabc; print error message below...
    }

    if ( fieldname == QString::fromLatin1( "homephone" ) ) {
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Home ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "telephonenumber" ) ) {
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Work ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mobile" ) ) { // mozilla
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Cell ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "cellphone" ) ) {
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Cell ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "pager" ) ) {  // mozilla
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Pager ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "facsimiletelephonenumber" ) ) {
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Fax ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "xmozillaanyphone" ) ) { // mozilla
      a.insertPhoneNumber( KABC::PhoneNumber( s, KABC::PhoneNumber::Work ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "streethomeaddress" ) ) {
      homeAddr.setStreet( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "postaladdress" ) ) {  // mozilla
      workAddr.setStreet( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillapostaladdress2" ) ) {  // mozilla
      workAddr.setStreet( workAddr.street() + QString::fromLatin1( "\n" ) + s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "postalcode" ) ) {
      workAddr.setPostalCode( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "homepostaladdress" ) ) {  // mozilla
      homeAddr.setStreet( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillahomepostaladdress2" ) ) {  // mozilla
      homeAddr.setStreet( homeAddr.street() + QString::fromLatin1( "\n" ) + s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillahomelocalityname" ) ) {  // mozilla
      homeAddr.setLocality( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillahomestate" )	) { // mozilla
      homeAddr.setRegion( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillahomepostalcode" ) ) {  // mozilla
      homeAddr.setPostalCode( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "mozillahomecountryname" ) ) { // mozilla
      homeAddr.setCountry( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "locality" ) ) {
      workAddr.setLocality( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "streetaddress" ) ) {
      workAddr.setStreet( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "countryname" ) ) {
      workAddr.setCountry( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "l" ) ) {  // mozilla
      workAddr.setLocality( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "c" ) ) {  // mozilla
      workAddr.setCountry( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "st" ) ) {
      workAddr.setRegion( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "ou" ) ) {
      a.setRole( s );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "member" ) ) {
      // this is a mozilla list member (cn=xxx, mail=yyy)
      QStringList list( QStringList::split( ',', s ) );
      QString name, email;

      QStringList::Iterator it;
      for ( it = list.begin(); it != list.end(); ++it ) {
        if ( (*it).startsWith( "cn=" ) )
          name = (*it).mid( 3 ).stripWhiteSpace();
        if ( (*it).startsWith( "mail=" ) )
          email = (*it).mid( 5 ).stripWhiteSpace();
      }
      if ( !name.isEmpty() && !email.isEmpty() )
        email = " <" + email + ">";
      a.insertEmail( name + email );
      a.insertCategory( i18n( "List of E-Mails" ) );
      continue;
    }

    if ( fieldname == QString::fromLatin1( "modifytimestamp" ) ) {
      QDateTime dt = KABC::VCardStringToDate( s );
      if ( dt.isValid() ) {
          a.setRevision(dt);
          continue;
      }
    }

    if ( fieldname == QString::fromLatin1( "objectclass" ) ) // ignore 
      continue;

    kdWarning() << QString("LDIF import filter: Unable to handle line: %1  for  %2\n")
	.arg(completeline).arg(a.formattedName());
  }

  file.close();

  return addrList;
}


/* export */

bool LDIFXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  KURL url = KFileDialog::getSaveURL( "addressbook.ldif" );
  if ( url.isEmpty() )
      return true;

  if ( !url.isLocalFile() ) {
    KTempFile tmpFile;
    if ( tmpFile.status() != 0 ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.%2.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( url.url() )
                          .arg( strerror( tmpFile.status() ) ) );
      return false;
    }

    doExport( tmpFile.file(), list );
    tmpFile.close();

    return KIO::NetAccess::upload( tmpFile.name(), url, parentWidget() );
  } else {
    QFile file( url.fileName() );
    if ( !file.open( IO_WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>" );
      KMessageBox::error( parentWidget(), txt.arg( url.fileName() ) );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

static void ldif_out( QTextStream *t, const QString &str, const QString &field )
{
  if ( field.isEmpty() )
    return;
  *t << str.arg( field );
}

void LDIFXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  QTextStream t( fp );
  t.setEncoding( QTextStream::UnicodeUTF8 );

  KABC::AddresseeList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( (*it).isEmpty() )
      continue;

    const KABC::Address homeAddr = (*it).address( KABC::Address::Home );
    const KABC::Address workAddr = (*it).address( KABC::Address::Work );

    ldif_out( &t, "%1", QString( "dn: cn=%1,mail=%2\n" )
              .arg( (*it).formattedName() )
              .arg( (*it).preferredEmail() ) );
    ldif_out( &t, "givenname: %1\n", (*it).givenName() );
    ldif_out( &t, "sn: %1\n", (*it).familyName() );
    ldif_out( &t, "cn: %1\n", (*it).formattedName() );
    ldif_out( &t, "xmozillanickname: %1\n", (*it).nickName() );

    ldif_out( &t, "mail: %1\n", (*it).preferredEmail() );
    if ( (*it).emails().count() > 1 )
      ldif_out( &t, "mozillasecondemail: %1\n", (*it).emails()[ 1 ] );
//    ldif_out( &t, "mozilla_AIMScreenName: %1\n", "screen_name" );

    ldif_out( &t, "telephonenumber: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Work ).number() );
    ldif_out( &t, "facsimiletelephonenumber: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Fax ).number() );
    ldif_out( &t, "homephone: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Home ).number() );
    ldif_out( &t, "mobile: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Cell ).number() );
    ldif_out( &t, "cellphone: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Cell ).number() );
    ldif_out( &t, "pager: %1\n", (*it).phoneNumber( KABC::PhoneNumber::Pager ).number() );

    ldif_out( &t, "streethomeaddress: %1\n", homeAddr.street() );
    ldif_out( &t, "postalcode: %1\n", workAddr.postalCode() );

    QStringList streets = QStringList::split( '\n', homeAddr.street() );
    if ( streets.count() > 0 )
      ldif_out( &t, "homepostaladdress: %1\n", streets[ 0 ] );
    if ( streets.count() > 1 )
      ldif_out( &t, "mozillahomepostaladdress2: %1\n", streets[ 1 ] );
    ldif_out( &t, "mozillahomelocalityname: %1\n", homeAddr.locality() );
    ldif_out( &t, "mozillahomestate: %1\n", homeAddr.region() );
    ldif_out( &t, "mozillahomepostalcode: %1\n", homeAddr.postalCode() );
    ldif_out( &t, "mozillahomecountryname: %1\n", homeAddr.country() );
    ldif_out( &t, "locality: %1\n", workAddr.locality() );
    ldif_out( &t, "streetaddress: %1\n", workAddr.street() );

    streets = QStringList::split( '\n', workAddr.street() );
    if ( streets.count() > 0 )
      ldif_out( &t, "postaladdress: %1\n", streets[ 0 ] );
    if ( streets.count() > 1 )
      ldif_out( &t, "mozillapostaladdress2: %1\n", streets[ 1 ] );
    ldif_out( &t, "countryname: %1\n", workAddr.country() );
    ldif_out( &t, "l: %1\n", workAddr.locality() );
    ldif_out( &t, "c: %1\n", workAddr.country() );
    ldif_out( &t, "st: %1\n", workAddr.region() );

    ldif_out( &t, "title: %1\n", (*it).title() );
    ldif_out( &t, "ou: %1\n", (*it).role() );
    ldif_out( &t, "o: %1\n", (*it).organization() );
    ldif_out( &t, "workurl: %1\n", (*it).url().prettyURL() );
    ldif_out( &t, "homeurl: %1\n", (*it).url().prettyURL() );
    ldif_out( &t, "description:: %1\n", KCodecs::base64Encode( (*it).note().utf8() ) );
    if ((*it).revision().isValid())
      ldif_out(&t, "modifytimestamp: %1Z\n", KABC::dateToVCardString((*it).revision()));

    t << "objectclass: top\n";
    t << "objectclass: person\n";

    t << "\n";
  }
}

#include "ldif_xxport.moc"
