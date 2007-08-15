/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2004 Helge Deller <deller@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.


    Description:
    This import/export filter reads and writes addressbook entries in the
    gmx format which is natively used by the german freemail provider GMX.
    The big advantage of this format is, that it stores it's information
    very consistent and makes parsing pretty simple. Furthermore, most
    information needed by KABC is available when compared to other formats.
    For further information please visit http://www.gmx.com
*/

#include <QFile>
#include <QMap>
//Added by qt3to4:
#include <QTextStream>

#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kcodecs.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include <kdebug.h>

#include "gmx_xxport.h"

K_EXPORT_KADDRESSBOOK_XXFILTER_CATALOG( libkaddrbk_gmx_xxport, GMXXXPort, "libkaddrbk_gmx_xxport" )

#define GMX_FILESELECTION_STRING "*.gmxa|" + i18n( "GMX addressbook file (*.gmxa)" )

GMXXXPort::GMXXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n( "Import GMX Address Book..." ) );
  createExportAction( i18n( "Export GMX Address Book..." ) );
}

static bool checkDateTime( const QString &dateStr, QDateTime &dt )
{
  if (dateStr.isEmpty())
     return false;
  dt = QDateTime::fromString(dateStr, Qt::ISODate);
  if (dt.isValid() && dt.date().year()>1901)
     return true;
  dt.setDate(QDate());
  return false;
}

/* import */

KABC::Addressee::List GMXXXPort::importContacts( const QString& ) const
{
  KABC::Addressee::List addrList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homePath(),
                      GMX_FILESELECTION_STRING, 0 );
  if ( fileName.isEmpty() )
    return addrList;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>",
                        fileName );
    KMessageBox::error( parentWidget(), msg );
    return addrList;
  }

  QDateTime dt;
  QTextStream gmxStream( &file );
  gmxStream.setCodec( "ISO 8859-1" );
  QString line, line2;
  line  = gmxStream.readLine();
  line2 = gmxStream.readLine();
  if (!line.startsWith("AB_ADDRESSES:") || !line2.startsWith("Address_id")) {
	KMessageBox::error( parentWidget(), i18n("%1 is not a GMX address book file.", fileName) );
	return addrList;
  }

  QStringList strList;
  typedef QMap<QString, KABC::Addressee *> AddressMap;
  AddressMap addrMap;

  // "Address_id,Nickname,Firstname,Lastname,Title,Birthday,Comments,Change_date,Status,Address_link_id,Categories"
  line = gmxStream.readLine();
  while (!line.startsWith("####") && !gmxStream.atEnd()) {
    while (1) {
       strList = line.split('#', QString::KeepEmptyParts );
       if (strList.count() >= 11)
           break;
       line.append('\n');
       line.append(gmxStream.readLine());
    };

    KABC::Addressee *addr = new KABC::Addressee;
    addr->setNickName(strList[1]);
    addr->setGivenName(strList[2]);
    addr->setFamilyName(strList[3]);
    addr->setTitle(strList[4]);
    if (checkDateTime(strList[5],dt)) addr->setBirthday(dt);
    addr->setNote(strList[6]);
    if (checkDateTime(strList[7],dt)) addr->setRevision(dt);
    // addr->setStatus(strList[8]); Status
    // addr->xxx(strList[9]); Address_link_id
    // addr->setCategory(strList[10]); Categories
    addrMap[strList[0]] = addr;

    line = gmxStream.readLine();
  }

  // now read the address records
  line  = gmxStream.readLine();
  if (!line.startsWith("AB_ADDRESS_RECORDS:")) {
	kWarning() <<"Could not find address records!";
	return addrList;
  }
  // Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,Mobile_type,Email,
  // Homepage,Position,Comments,Record_type_id,Record_type,Company,Department,Change_date,Preferred,Status
  line = gmxStream.readLine();
  line = gmxStream.readLine();

  while (!line.startsWith("####") && !gmxStream.atEnd()) {
    while (1) {
       strList = line.split('#', QString::KeepEmptyParts );
       if (strList.count() >= 21)
           break;
       line.append('\n');
       line.append(gmxStream.readLine());
    };

    KABC::Addressee *addr = addrMap[strList[0]];
    if (addr) {
	for ( QStringList::Iterator it = strList.begin(); it != strList.end(); ++it )
		*it = (*it).simplified();
	// strList[1] = Record_id (numbered item, ignore here)
	int id = strList[14].toInt(); // Record_type_id (0=work,1=home,2=other)
  KABC::Address::Type type = (id==0) ? KABC::Address::Work : KABC::Address::Home;
	if (!strList[19].isEmpty() && strList[19].toInt()!=0)
		type |= KABC::Address::Pref; // Preferred address (seems to be bitfield for telephone Prefs)
        KABC::Address adr = addr->address(type);
	adr.setStreet(strList[2]);
	adr.setCountry(strList[3]);
	adr.setPostalCode(strList[4]);
	adr.setLocality(strList[5]);
	addr->insertPhoneNumber( KABC::PhoneNumber(strList[6], KABC::PhoneNumber::Home) );
	addr->insertPhoneNumber( KABC::PhoneNumber(strList[7], KABC::PhoneNumber::Fax) );
  KABC::PhoneNumber::Type celltype = KABC::PhoneNumber::Cell;
	// strList[9]=Mobile_type // always 0 or -1(default phone).
	if (strList[9].toInt()) celltype |= KABC::PhoneNumber::Pref;
	addr->insertPhoneNumber( KABC::PhoneNumber(strList[8], celltype) );
	addr->insertEmail(strList[10]);
	if (!strList[11].isEmpty()) addr->setUrl(strList[11]);
	if (!strList[12].isEmpty()) addr->setRole(strList[12]);
	// strList[13]=Comments
	// strList[14]=Record_type_id (0,1,2) - see above
	// strList[15]=Record_type (name of this additional record entry)
	if (!strList[16].isEmpty()) addr->setOrganization(strList[16]); // Company
	if (!strList[17].isEmpty()) addr->insertCustom(
			"KADDRESSBOOK", "X-Department", strList[17]); // Department
        if (checkDateTime(strList[18],dt)) addr->setRevision(dt); // Change_date
	// strList[19]=Preferred (see above)
	// strList[20]=Status (should always be "1")
	addr->insertAddress(adr);
    } else {
	kWarning() <<"unresolved line:" << line;
    }

    line = gmxStream.readLine();
  }

  // now add the addresses to to addrList
  for ( AddressMap::Iterator it = addrMap.begin(); it != addrMap.end(); ++it ) {
     KABC::Addressee *addr = it.value();
     addrList.append(*addr);
     delete addr;
  }

  file.close();
  return addrList;
}


/* export */

bool GMXXXPort::exportContacts( const KABC::AddresseeList &list, const QString& )
{
  KUrl url = KFileDialog::getSaveUrl( KUrl( QDir::homePath() + "/addressbook.gmx" ),
			GMX_FILESELECTION_STRING );
  if ( url.isEmpty() )
      return true;

  if ( !url.isLocalFile() ) {
    KTemporaryFile tmpFile;
    if ( !tmpFile.open() ) {
      QString txt = i18n("<qt>Unable to open file <b>%1</b></qt>", url.url());
      KMessageBox::error( parentWidget(), txt );
      return false;
    }

    doExport( &tmpFile, list );
    tmpFile.flush();

    return KIO::NetAccess::upload( tmpFile.fileName(), url, parentWidget() );
  } else {
    QString filename = url.path();
    QFile file( filename );

    if ( !file.open( QIODevice::WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>",
                          filename );
      KMessageBox::error( parentWidget(), txt );
      return false;
    }

    doExport( &file, list );
    file.close();

    return true;
  }
}

static const QString dateString( const QDateTime &dt )
{
  if (!dt.isValid())
	return QString::fromLatin1("1000-01-01 00:00:00");
  QString d(dt.toString(Qt::ISODate));
  d[10] = ' '; // remove the "T" in the middle of the string
  return d;
}

void GMXXXPort::doExport( QFile *fp, const KABC::AddresseeList &list )
{
  if (!fp || !list.count())
    return;

  QTextStream t( fp );
  t.setCodec( "ISO 8859-1" );

  KABC::AddresseeList::ConstIterator it;
  typedef QMap<int, const KABC::Addressee *> AddressMap;
  AddressMap addrMap;
  const KABC::Addressee *addr;

  t << "AB_ADDRESSES:\n";
  t << "Address_id,Nickname,Firstname,Lastname,Title,Birthday,Comments,"
       "Change_date,Status,Address_link_id,Categories\n";

  int no = 0;
  const QChar DELIM('#');
  for ( it = list.begin(); it != list.end(); ++it ) {
     addr = &(*it);
     if (addr->isEmpty())
        continue;
     addrMap[++no] = addr;
     t << no << DELIM			// Address_id
	<< addr->nickName() << DELIM	// Nickname
	<< addr->givenName() << DELIM	// Firstname
	<< addr->familyName() << DELIM	// Lastname
	<< addr->title() << DELIM	// Title
	<< dateString(addr->birthday()) << DELIM   // Birthday
	<< addr->note() /*.replace('\n',"\r\n")*/ << DELIM // Comments
	<< dateString(addr->revision()) << DELIM   // Change_date
	<< "1##0\n";			// Status, Address_link_id, Categories
  }

  t << "####\n";
  t << "AB_ADDRESS_RECORDS:\n";
  t << "Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,"
       "Mobile_type,Email,Homepage,Position,Comments,Record_type_id,Record_type,"
       "Company,Department,Change_date,Preferred,Status\n";

  no = 1;
  while ( (addr = addrMap[no]) != NULL ) {
    for (int record_id=0; record_id<3; record_id++) {

	KABC::Address address;
  	KABC::PhoneNumber phone, fax, cell;


        if (record_id == 0) {
		address = addr->address(KABC::Address::Work);
		phone = addr->phoneNumber(KABC::PhoneNumber::Work);
		fax   = addr->phoneNumber(KABC::PhoneNumber::Fax);
		cell  = addr->phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell);
	} else {
		address = addr->address(KABC::Address::Home);
		phone = addr->phoneNumber(KABC::PhoneNumber::Home);
		cell  = addr->phoneNumber(KABC::PhoneNumber::Cell);
	}

	const QStringList emails = addr->emails();
	QString email;
	if (emails.count()>record_id) email = emails[record_id];

	t << no << DELIM			// Address_id
	  << record_id << DELIM			// Record_id
	  << address.street() << DELIM		// Street
	  << address.country() << DELIM 	// Country
	  << address.postalCode() << DELIM	// Zipcode
	  << address.locality() << DELIM	// City
	  << phone.number() << DELIM		// Phone
	  << fax.number() << DELIM		// Fax
	  << cell.number() << DELIM		// Mobile
	  << ((cell.type()&KABC::PhoneNumber::Pref)?-1:0) << DELIM // Mobile_type
	  << email << DELIM			// Email
	  << ((record_id==0)?addr->url().url():QString::null) << DELIM // Homepage	//krazy:exclude=nullstrassign for old broken gcc
	  << ((record_id==0)?addr->role():QString::null) << DELIM	// Position	//krazy:exclude=nullstrassign for old broken gcc
	  << DELIM				// Comments
	  << record_id << DELIM			// Record_type_id (0,1,2) - see above
	  << DELIM				// Record_type (name of this additional record entry)
	  << ((record_id==0)?addr->organization():QString::null) << DELIM // Company	//krazy:exclude=nullstrassign for old broken gcc
	  << ((record_id==0)?addr->custom("KADDRESSBOOK", "X-Department"):QString::null) << DELIM // Department	//krazy:exclude=nullstrassign for old broken gcc
	  << dateString(addr->revision()) << DELIM	// Change_date
	  << 5 << DELIM				// Preferred
	  << 1 << endl;				// Status (should always be "1")
    }

    ++no;
  };

  t << "####";
}

#include "gmx_xxport.moc"

