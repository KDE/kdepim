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

#include "gmx_xxport.h"

#include <QtCore/QFile>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QTextStream>

#include <kcodecs.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktemporaryfile.h>
#include <kurl.h>

K_EXPORT_KADDRESSBOOK_XXFILTER_CATALOG( kaddrbk_gmx_xxport, GMXXXPort, "kaddrbk_gmx_xxport" )

#define GMX_FILESELECTION_STRING "*.gmxa|" + i18n( "GMX address book file (*.gmxa)" )

const int typeHome  = 0;
const int typeWork  = 1;
const int typeOther = 2;

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
  if (!line.startsWith(QLatin1String("AB_ADDRESSES:")) || !line2.startsWith(QLatin1String("Address_id"))) {
	KMessageBox::error( parentWidget(), i18n("%1 is not a GMX address book file.", fileName) );
	return addrList;
  }

  QStringList strList;
  typedef QMap<QString, KABC::Addressee *> AddressMap;
  AddressMap addrMap;
  QMap<QString, QString>  catList;

  // "Address_id,Nickname,Firstname,Lastname,Title,Birthday,Comments,Change_date,Status,Address_link_id,Categories"
  line = gmxStream.readLine();
  while ( (line!=QLatin1String("####")) && !gmxStream.atEnd() ) {
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
    addr->setFormattedName(strList[3] + ", " + strList[2]);
    addr->setPrefix(strList[4]);
    if (checkDateTime(strList[5],dt)) addr->setBirthday(dt);
    addr->setNote(strList[6]);
    if (checkDateTime(strList[7],dt)) addr->setRevision(dt);
    // addr->setStatus(strList[8]); Status
    // addr->xxx(strList[9]); Address_link_id
    catList[strList[0]] = strList[10];
    addrMap[strList[0]] = addr;

    line = gmxStream.readLine();
  }

  // now read the address records
  line  = gmxStream.readLine();
  if (!line.startsWith(QLatin1String("AB_ADDRESS_RECORDS:"))) {
	kWarning() <<"Could not find address records!";
	return addrList;
  }
  // Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,Mobile_type,Email,
  // Homepage,Position,Comments,Record_type_id,Record_type,Company,Department,Change_date,Preferred,Status
  line = gmxStream.readLine();
  line = gmxStream.readLine();

  while (!line.startsWith(QLatin1String("####")) && !gmxStream.atEnd()) {
    while (1) {
       strList = line.split('#', QString::KeepEmptyParts );
       if (strList.count() >= 21)
           break;
       line.append('\n');
       line.append(gmxStream.readLine());
    };

    KABC::Addressee *addr = addrMap[strList[0]];
    if (addr) {
	// strList[1] = Record_id (numbered item, ignore here)
	int id = strList[14].toInt(); // Record_type_id (0=home,1=work,2=other)
	KABC::Address::Type type;
	KABC::PhoneNumber::Type phoneType;
	switch ( id ) {
	  case typeHome:
	    type = KABC::Address::Home;
	    phoneType = KABC::PhoneNumber::Home;
	    break;
	  case typeWork:
	    type = KABC::Address::Work;
	    phoneType = KABC::PhoneNumber::Work;
	    break;
	  case typeOther:
	  default:
	    type = KABC::Address::Intl;
	    phoneType = KABC::PhoneNumber::Voice;
	    break;
	}
        KABC::Address adr = addr->address(type);
	adr.setStreet(strList[2]);
	adr.setCountry(strList[3]);
	adr.setPostalCode(strList[4]);
	adr.setLocality(strList[5]);
	if (!strList[6].isEmpty()) {
	  addr->insertPhoneNumber(
	    KABC::PhoneNumber(strList[6], phoneType)
	  );
	}
	if (!strList[7].isEmpty())
	  addr->insertPhoneNumber(
	    KABC::PhoneNumber(strList[7], KABC::PhoneNumber::Fax)
	);
  KABC::PhoneNumber::Type celltype = KABC::PhoneNumber::Cell;
	// strList[9]=Mobile_type // always 0 or -1(default phone).
	//if ( strList[19].toInt() & 4 ) celltype |= KABC::PhoneNumber::Pref;
	//  avoid duplicates
	if (!strList[8].isEmpty())
	  addr->insertPhoneNumber(
	    KABC::PhoneNumber(strList[8], celltype)
	);
	bool preferred = false;
	if (strList[19].toInt() & 1 ) preferred = true;
	addr->insertEmail(strList[10], preferred);
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

  // read the categories
  QStringList categories;
  line = gmxStream.readLine();
  line2 = gmxStream.readLine();
  if ( !line.startsWith( QLatin1String( "AB_CATEGORIES:" ) ) ||
    !line2.startsWith( QLatin1String( "Category_id" ) ) ) {
    kWarning() <<"Could not find categories records!";
  } else {
    while ( !line.startsWith( QLatin1String( "####" ) ) &&
            !gmxStream.atEnd() ) {
      while (1) {
        strList = line.split( '#', QString::KeepEmptyParts );
        if ( strList.count() >= 3 )
          break;
        line.append( '\n' );
        line.append( gmxStream.readLine() );
      };
      categories.append( strList[1] );
      line = gmxStream.readLine();
    };
  }

  // now add the addresses to to addrList
  for ( AddressMap::Iterator it = addrMap.begin(); it != addrMap.end(); ++it ) {
     KABC::Addressee *addr = it.value();
     // Add categories
     int addrCat = catList[it.key()].toInt();
     for ( int i=32; i >= 0; --i ) {
       int cat =  1<<i;
       if ( cat > addrCat ) continue;
       if ( cat & addrCat  && categories.count() > i )
         addr->insertCategory( categories[i] );
     }
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

  if( QFileInfo( url.isLocalFile() ? url.toLocalFile() : url.path() ).exists() ) {
      if(KMessageBox::questionYesNo( parentWidget(), i18n("Do you want to overwrite file \"%1\"", url.isLocalFile() ? url.toLocalFile() : url.path()) ) == KMessageBox::No)
        return false;
  }

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
    QString filename = url.toLocalFile();
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

static const QStringList assignedCategoriesSorted(
  const KABC::AddresseeList &list )
{
  // Walk through the addressees and collect up to 31 categories,
  // returned sorted alphabetically
  QStringList catList;
  KABC::AddresseeList::ConstIterator it;
  const KABC::Addressee *addr;
  for ( it = list.begin(); it != list.end() && catList.count() < 32; ++it ) {
    addr = &(*it);
    if ( addr->isEmpty() ) continue;
    const QStringList categories = addr->categories();
    for ( int i=0; i < categories.count() && catList.count() < 32; ++i ) {
      if ( !catList.contains( categories[i]) )
        catList.append( categories[i] );
    }
  }
  catList.sort();
  return catList;
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

  // Categories: The gmxa file lists the categories in the last section
  // and allows multi-category assignment through bitfields.
  // However, the category definition in GMX is not updated through the
  // import (only the category assignment of the addressees). The addresses
  // may point to non-existing or wrong categories after the import.
  // This implies that the user needs to make sure he/she manually creates
  // the categories in the GMX UI *in the same sequence* as in the gmxa files.
  // Of course this is only necessary if the category list has changed since
  // the last import.
  // The GMX UI displays the categories alphabetically sorted (thus hiding the
  // sequence of entering categories. Therefore the easiest is to output the
  // category list in the gmxa file alphabetically sorted as well. If you need
  // to enter a new category, you can delete any existing category which sorts
  // higher than the new one and then enter the new and remaining category.
  QList<QString> catList;
  catList.append( assignedCategoriesSorted( list ) );

  int no = 0;
  const QChar DELIM('#');
  for ( it = list.begin(); it != list.end(); ++it ) {
     addr = &(*it);
     if (addr->isEmpty())
        continue;
     addrMap[++no] = addr;
  
    // Assign categories as bitfield
    const QStringList categories = addr->categories();
    long int category = 0;
    if ( categories.count() > 0 ) {
      for ( int i=0; i < categories.count(); i++ ) {
        if ( catList.contains( categories[i] ) )
          category |= 1 << catList.indexOf( categories[i], 0 ) ;
      }
    }

    // GMX sorts by nickname by default - don't leave empty
    QString nickName = addr->nickName();
    if ( nickName.isEmpty() )
      nickName = addr->formattedName();

     t << no << DELIM			// Address_id
	<< nickName << DELIM            // Nickname
	<< addr->givenName() << DELIM	// Firstname
	<< addr->familyName() << DELIM	// Lastname
	<< addr->prefix() << DELIM	// Title - Note: ->title()
                                        // refers to the professional title
	<< dateString(addr->birthday()) << DELIM   // Birthday
	<< addr->note() /*.replace('\n',"\r\n")*/ << DELIM // Comments
	<< dateString(addr->revision()) << DELIM   // Change_date
        << "1" << DELIM                 // Status
        << DELIM                        // Address_link_id
        << category << endl;            // Categories
  }

  t << "####\n";
  t << "AB_ADDRESS_RECORDS:\n";
  t << "Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,"
       "Mobile_type,Email,Homepage,Position,Comments,Record_type_id,Record_type,"
       "Company,Department,Change_date,Preferred,Status\n";

  no = 1;
  while ( (addr = addrMap[no]) != NULL ) {

    const KABC::PhoneNumber::List cellPhones =
      addr->phoneNumbers( KABC::PhoneNumber::Cell );

    const QStringList emails = addr->emails();

    for (int record_id=0; record_id<3; ++record_id) {

	KABC::Address address;
  	KABC::PhoneNumber phone, fax, cell;
     
      // address preference flag:
      // & 1: preferred email address
      // & 4: preferred cell phone
      int prefFlag=0;

      switch (record_id) {
      // Assign address, phone and cellphone, fax if applicable
        case typeHome:
          address = addr->address( KABC::Address::Home );
          phone   = addr->phoneNumber( KABC::PhoneNumber::Home );
          if ( cellPhones.count() > 0 ) {
            cell  = cellPhones.at(0);
            if ( !cell.isEmpty() )
              prefFlag |= 4;
          }
          break;
        case typeWork:
          address = addr->address( KABC::Address::Work );
          phone   = addr->phoneNumber( KABC::PhoneNumber::Work );
          if ( cellPhones.count() >= 2 )
            cell  = cellPhones.at(1);
          fax     = addr->phoneNumber( KABC::PhoneNumber::Fax );
          break;
        case typeOther:
        default:
          if ( addr->addresses( KABC::Address::Home ).count() > 1 )
            address = addr->addresses( KABC::Address::Home ).at(1);
          if ( ( address.isEmpty() ) &&
               ( addr->addresses( KABC::Address::Work ).count() > 1 ) )
            address = addr->addresses( KABC::Address::Work ).at(1);
          if ( address.isEmpty() )
            address = addr->address( KABC::Address::Dom );
          if ( address.isEmpty() )
            address = addr->address( KABC::Address::Intl );
          if ( address.isEmpty() )
            address = addr->address( KABC::Address::Postal );
          if ( address.isEmpty() )
            address = addr->address( KABC::Address::Parcel );

          if ( addr->phoneNumbers( KABC::PhoneNumber::Home ).count() > 1 )
            phone = addr->phoneNumbers( KABC::PhoneNumber::Home ).at(1);
          if ( ( phone.isEmpty() ) &&
               ( addr->phoneNumbers( KABC::PhoneNumber::Work ).count() > 1 ) )
            phone = addr->phoneNumbers( KABC::PhoneNumber::Work ).at(1);
          if ( phone.isEmpty() )
            phone = addr->phoneNumber( KABC::PhoneNumber::Voice );
          if ( phone.isEmpty() )
            phone = addr->phoneNumber( KABC::PhoneNumber::Msg );
          if ( phone.isEmpty() )
            phone = addr->phoneNumber( KABC::PhoneNumber::Isdn );
          if ( phone.isEmpty() )
            phone = addr->phoneNumber( KABC::PhoneNumber::Car );
          if ( phone.isEmpty() )
            phone = addr->phoneNumber( KABC::PhoneNumber::Pager );

          switch ( cellPhones.count() ) {
            case 0: break;
            case 1:
            case 2:
              if ( !address.isEmpty() )
                cell = cellPhones.at(0);
              break;
            default:
              cell = cellPhones.at(2);
              break;
          }
          break;
      }

      QString email="";
      if (emails.count()>record_id) {
        email = emails[record_id];
        if ( email == addr->preferredEmail() ) prefFlag |= 1;
      }

      if ( !address.isEmpty() || !phone.isEmpty() ||
           !cell.isEmpty()    || !email.isEmpty() ) {
	t << no << DELIM			// Address_id
	  << record_id << DELIM			// Record_id
	  << address.street() << DELIM		// Street
	  << address.country() << DELIM 	// Country
	  << address.postalCode() << DELIM	// Zipcode
	  << address.locality() << DELIM	// City
	  << phone.number() << DELIM		// Phone
	  << fax.number() << DELIM		// Fax
	  << cell.number() << DELIM		// Mobile
	  << ((record_id==typeWork)?0:1 ) << DELIM   // Mobile_type
	  << email << DELIM			// Email
	  << ((record_id==typeWork)?addr->url().url():QString()) << DELIM // Homepage
	  << ((record_id==typeWork)?addr->role():QString()) << DELIM // Position
	  << ((record_id==typeHome)?addr->custom("KADDRESSBOOK", "X-SpousesName"):QString() ) << DELIM // Comments
	  << record_id << DELIM			// Record_type_id (0,1,2) - see above
	  << DELIM				// Record_type (name of this additional record entry)
	  << ((record_id==typeWork)?addr->organization():QString()) << DELIM // Company
	  << ((record_id==typeWork)?addr->custom("KADDRESSBOOK", "X-Department"):QString() ) << DELIM // Department
	  << dateString(addr->revision()) << DELIM	// Change_date
	  << prefFlag << DELIM			// Preferred:
	                                        // ( & 1: preferred email,
	                                        //   & 4: preferred cell phone )
	  << 1 << endl;				// Status (should always be "1")
      }
    }

    ++no;
  };

  t << "####" << endl;
  t << "AB_CATEGORIES:" << endl;
  t << "Category_id,Name,Icon_id" << endl;

  //  Write Category List (beware: Category_ID 0 is reserved for none
  //  Interestingly: The index here is an int sequence and does not
  //  correspond to the bit reference used above.
  for ( int i = 0; i < catList.size(); i++ ) {
    t << ( i + 1 ) << DELIM << catList.at( i ) << DELIM << 0 << endl;
  }
  t << "####" << endl;
}

#include "gmx_xxport.moc"
