/*
    This file is part of KAddressbook.
    Copyright (c) 2000 - 2003 Oliver Strutynski <olistrut@gmx.de>
                              Helge Deller <deller@kde.org>
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
    This import filter reads any LDIF version 1 file and imports the
    entries into your KDE Addressbook.
*/

#include <qfile.h>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kurl.h>
#include <kabc/addressbook.h>

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
  createImportAction( i18n( "Import LDIF Addressbook..." ) );
  createExportAction( i18n( "Export LDIF Addressbook..." ) );
}

/* import */

static bool AddressEmpty( KABC::Address *addr )
{
  return addr->street().isEmpty() &&
         addr->locality().isEmpty() &&
         addr->region().isEmpty() &&
         addr->country().isEmpty();
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

  KABC::AddresseeList addrList;

  kdWarning() << "LDIF import filter started.\n";

  QString empty;

  QTextStream t( &file );
  t.setEncoding(QTextStream::Latin1);
  QString s, completeline, fieldname;

  // Set to true if data currently read is part of
  // a list of names. Lists will be ignored.
  bool isGroup = false;
  bool lastWasComment = false;
  int numEntries = 0;

  KABC::Addressee *a = new KABC::Addressee();
  KABC::Address *homeAddr, *workAddr; 
  homeAddr = new KABC::Address();
  homeAddr->setType(KABC::Address::Home);
  workAddr = new KABC::Address();
  workAddr->setType(KABC::Address::Work);
  while ( !t.eof() ) {
	s = t.readLine();
	completeline = s;
	
	if (s.isEmpty() && t.eof()) {
		// Newline: Write data
writeData:
		if (!isGroup) {
			if (!a->formattedName().isEmpty()) {
				numEntries++;
				if (!AddressEmpty(homeAddr))
					a->insertAddress(*homeAddr);
				if (!AddressEmpty(workAddr))
					a->insertAddress(*workAddr);
				addrList.append(*a);
			}
   		};

		// delete old and create a new empty address entry
		delete a;
		delete homeAddr;
		delete workAddr;
		a = new KABC::Addressee();
		homeAddr = new KABC::Address();
 		homeAddr->setType(KABC::Address::Home);
		workAddr = new KABC::Address();
		workAddr->setType(KABC::Address::Work);

		isGroup = false;
		lastWasComment = false;
		continue;
	}
	
	int position = s.find("::");
    	if (position != -1) {
    		// String is BASE64 encoded
    		fieldname = s.left(position).lower();
    		s = QString::fromUtf8(KCodecs::base64Decode(
						s.mid(position+3, s.length()-position-2).latin1()))
						.simplifyWhiteSpace();
    	} else {
    		position = s.find(":");
		if (position != -1) {
    			fieldname = s.left(position).lower();
    			// Convert Utf8 string to unicode so special characters are preserved
			// We need this since we are reading normal strings from the file
			// which are not converted automatically
			s = QString::fromUtf8(s.mid(position+2, s.length()-position-2).latin1());
		} else {
			fieldname = QString::null;
			s = QString::fromUtf8(s);
		}
	}

	if (s.stripWhiteSpace().isEmpty())
		continue;

	if (fieldname.isEmpty()) {
		if (lastWasComment) {
			// if the last entry was a comment, add this one too, since
			// we might have a multi-line comment entry.
addComment:
			if (!a->note().isEmpty())
				a->setNote(a->note() + "\n");
			a->setNote(a->note() + s);
		}
		continue;
	}
	lastWasComment = false;

	if (fieldname == "givenname")
		{ a->setGivenName(s); continue; }

	if (fieldname == "xmozillanickname")
		{ a->setNickName(s); continue; }

	if (fieldname == "dn")	/* ignore */
		{ goto writeData; }
	
	if (fieldname == "sn")
		{ a->setFamilyName(s); continue; }
	
	if (fieldname == "mail")
		{ a->insertEmail(s); continue; }

	if (fieldname == "mozillasecondemail")	// mozilla
		{ a->insertEmail(s); continue; }

	if (fieldname == "title")
		{ a->setTitle(s); continue; }

	if (fieldname == "cn")
		{ a->setFormattedName(s); continue; }

	if (fieldname == "o")
		{ a->setOrganization(s); continue; }

	if (fieldname == "description")
		{ lastWasComment = true; goto addComment; }

	if (fieldname == "custom1" || fieldname == "custom2" ||
		fieldname == "custom3" || fieldname == "custom4" )
		{ goto addComment; }

	if (a->url().isEmpty() && (fieldname == "homeurl" || fieldname == "workurl"))
		{ a->setUrl(s); continue; } // only one URL allowed, ignore the other one

	if (fieldname == "homephone")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Home ) ); continue; }
	
	if (fieldname == "telephonenumber")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Work ) ); continue; }
	
	if (fieldname == "mobile")	// mozilla
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Cell ) ); continue; }

	if (fieldname == "cellphone")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Cell ) ); continue; }

	if (fieldname == "pager")	// mozilla
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Pager ) ); continue; }

	if (fieldname == "facsimiletelephonenumber")
		{ a->insertPhoneNumber( KABC::PhoneNumber (s, KABC::PhoneNumber::Fax ) ); continue; }
	
	if (fieldname == "streethomeaddress")
		{ homeAddr->setStreet(s); continue; }

	if (fieldname == "postaladdress")		// mozilla
		{ workAddr->setStreet(s); continue; }
	
	if (fieldname == "mozillapostaladdress2")	// mozilla
		{ workAddr->setStreet(workAddr->street()+"\n"+s); continue; }

	if (fieldname == "postalcode")
		{ workAddr->setPostalCode(s); continue; }

	if (fieldname == "homepostaladdress")		// mozilla
		{ homeAddr->setStreet(s); continue; }
	
	if (fieldname == "mozillahomepostaladdress2")	// mozilla
		{ homeAddr->setStreet(homeAddr->street()+"\n"+s); continue; }

	if (fieldname == "mozillahomelocalityname")	// mozilla
		{ homeAddr->setLocality(s); continue; }

	if (fieldname == "mozillahomestate")		// mozilla
		{ homeAddr->setRegion(s); continue; }

	if (fieldname == "mozillahomepostalcode")	// mozilla
		{ homeAddr->setPostalCode(s); continue; }

	if (fieldname == "mozillahomecountryname")	// mozilla
		{ homeAddr->setCountry(s); continue; }

	if (fieldname == "locality")
		{ workAddr->setLocality(s); continue; }
	
	if (fieldname == "streetaddress")
		{ workAddr->setStreet(s); continue; }
	
	if (fieldname == "countryname")
		{ workAddr->setCountry(s); continue; }
		
	if (fieldname == "l")	// mozilla
		{ workAddr->setLocality(s); continue; }

	if (fieldname == "c")	// mozilla
		{ workAddr->setCountry(s); continue; }

	if (fieldname == "st")
		{ workAddr->setRegion(s); continue; }

	if (fieldname == "ou")
		{ a->setRole(s); continue; }

	if (fieldname == "objectclass" && s == "groupOfNames")
			isGroup = true;

	if (fieldname == "modifytimestamp" || fieldname == "objectclass") // ignore 
		{ continue; }

	kdWarning() << "LDIF import filter: Unable to handle line: " << completeline << "\n";
  	  	
  } /* while !eof(file) */

  delete a;
  delete homeAddr;
  delete workAddr;

  file.close();

  kdWarning() << "LDIF import filter finished.\n";

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
      KMessageBox::error( core(), txt.arg( url.url() )
                          .arg( strerror( tmpFile.status() ) ) );
      return false;
    }

    doExport( tmpFile.file(), list );
    tmpFile.close();

    return KIO::NetAccess::upload( tmpFile.name(), url, core() );
  } else {
    QFile file( url.fileName() );
    if ( !file.open( IO_WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>" );
      KMessageBox::error( core(), txt.arg( url.fileName() ) );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

static void ldif_out( QTextStream *t, QString str, QString field )
{
  if (field.isEmpty())
    return;
  *t << str.arg(field);
}

void LDIFXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  QTextStream t( fp );
  t.setEncoding(QTextStream::Latin1);

  KABC::AddresseeList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    const KABC::Addressee *addr = &(*it);
    if (addr->isEmpty())
	continue;

    ldif_out(&t, "dn: cn=%1,mail=", addr->formattedName());
    ldif_out(&t, "%1", addr->preferredEmail());
    t << "\n";
    ldif_out(&t, "givenName: %1\n", addr->givenName());
    ldif_out(&t, "sn: %1\n", addr->familyName());
    ldif_out(&t, "cn: %1\n", addr->formattedName());
    ldif_out(&t, "xmozillanickname: %1\n", addr->nickName());
    ldif_out(&t, "mail: %1\n", addr->preferredEmail());
    if (addr->emails().count() > 1)
       ldif_out(&t, "mozillaSecondEmail: %1\n", *(addr->emails().at(1)));
//    ldif_out(&t, "mozilla_AimScreenName: %1\n", "screen_name");
//    ldif_out(&t, "telephoneNumber: %1\n", "111");
//    ldif_out(&t, "facsimileTelephoneNumber: %1\n", "111");

    ldif_out(&t, "title: %1\n", addr->title());
    ldif_out(&t, "ou: %1\n", addr->role());
    ldif_out(&t, "o: %1\n", addr->organization());
    ldif_out(&t, "workurl: %1\n", addr->url().prettyURL());
    ldif_out(&t, "description: %1\n", addr->note().replace("\n", "\\n"));

    t << "\n";
  }
}

#include "ldif_xxport.moc"
