/*
    This file is part of KAddressbook.
    Copyright (c) 2003 - 2004 Helge Deller <deller@kde.org>
    Copyright (c) 2009 Laurent Montel <montel@kde.org>
    Copyright (c) 2009 Urs Joss <tschenturs@gmx.ch>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

/*
    Description:

    This import/export filter reads and writes addressbook entries in the
    gmx format which is natively used by the german freemail provider GMX.
    The big advantage of this format is, that it stores it's information
    very consistently and makes parsing pretty simple. Furthermore, most
    information needed by KABC is available when compared to other formats.
    For further information please visit http://www.gmx.com

    A couple of notes:

    a) Categories

    GMX allows to define categories in the UI and assign one or more categories
    to each addressee. The gmxa file contains a list of defined categories in
    the last part of the file, in the section AB_CATEGORIES. The Category_id in
    this list is a 1-based index. The multi-category assignement of each
    contact consists of a bitfield which applies one bit per category. E.g.
    if one contact belongs to category 1 and 3, the bit-field for the category
    assignment for this addressee is 0101 or decimal 5.

    The export of categories into hte gmxa file works flawlessly and also
    transfers nicely into the addressee categories. During the import of a
    gmxa file into GMX, the procedure will correctly manage the category
    bitfield for each contact. However GMX does *not* process the category
    list in the end of the file. In other words: If your categories have
    changed (either in kadderssbook/kcontactmanager or in GMX) since your
    previous import, the addressee records may point to the wrong - or even
    non-existing - categories. After an import it is suggested that you
    manually verify the category list is still valid. If not, you need to
    manually delete any categories which alphabetically list beyond your new
    or deleted category and recreate the remaining categories.

    Only up to 31 different categories over all contacts are exported. Any
    more categories are ignored.

    b) Address types

    Home address and Work address are relatively straight forwardly handled.
    The third address type "Other" is factored out of a bunch of different
    address elements, which have been chosen relatively at random. There is
    some interpretation which is not completely reversible.

    If you thus export a contact from kaddressbook/kcontactmanager to GMX
    and reimport it into kaddressbook/kcontactmanager, you will not exactly
    get the same contact as the original one.

    Also other items affect this non-reversability: GMX should receive a nick
    name in the gmxa file. If there is none defined in kaddressbook/kcontact-
    manager, the export will use the formatted name instead. If you reimport
    such a record, you'll end up with the formatted name in the nickname field.
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


#define GMX_FILESELECTION_STRING "*.gmxa|" + \
  i18n( "GMX address book file (*.gmxa)" )

const int typeHome  = 0;
const int typeWork  = 1;
const int typeOther = 2;

GMXXXPort::GMXXXPort( QWidget *parentWidget )
  : XXPort( parentWidget )
{
}

static bool checkDateTime( const QString &dateStr, QDateTime &dt )
{
  if ( dateStr.isEmpty() )
     return false;
  dt = QDateTime::fromString( dateStr, Qt::ISODate );
  if ( dt.isValid() && dt.date().year()>1901 )
     return true;
  dt.setDate( QDate() );
  return false;
}

/* import */

KABC::Addressee::List GMXXXPort::importContacts(  ) const
{
  KABC::Addressee::List addresseeList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homePath(),
                       GMX_FILESELECTION_STRING, 0 );
  if ( fileName.isEmpty() )
    return addresseeList;

  QFile file( fileName );
  if ( !file.open( QIODevice::ReadOnly ) ) {
    QString msg = i18n( "<qt>Unable to open <b>%1</b> for reading.</qt>",
                        fileName );
    KMessageBox::error( parentWidget(), msg );
    return addresseeList;
  }

  QDateTime dt;
  QTextStream gmxStream( &file );
  gmxStream.setCodec( "ISO 8859-1" );
  QString line, line2;
  line  = gmxStream.readLine();
  line2 = gmxStream.readLine();
  if ( !line.startsWith( QLatin1String( "AB_ADDRESSES:" ) ) ||
      !line2.startsWith( QLatin1String( "Address_id"    ) ) ) {
    KMessageBox::error( parentWidget(),
      i18n( "%1 is not a GMX address book file.", fileName ) );
    return addresseeList;
  }

  QStringList itemList;
  QMap<QString, QString>  categoriesOfAddressee;
  typedef QMap<QString, KABC::Addressee *> AddresseeMap;
  AddresseeMap addresseeMap;

  // "Address_id,Nickname,Firstname,Lastname,Title,Birthday,Comments,
  // Change_date,Status,Address_link_id,Categories"
  line = gmxStream.readLine();
  while ( ( line!=QLatin1String( "####" ) ) && !gmxStream.atEnd() ) {
    // an addressee entry may spread over several lines in the file
    while ( 1 ) {
       itemList = line.split( '#', QString::KeepEmptyParts );
       if ( itemList.count() >= 11 )
           break;
       line.append( '\n' );
       line.append( gmxStream.readLine() );
    };

    // populate the addressee
    KABC::Addressee *addressee = new KABC::Addressee;
    addressee->setNickName( itemList[1] );
    addressee->setGivenName( itemList[2] );
    addressee->setFamilyName( itemList[3] );
    addressee->setFormattedName( itemList[3] + ", " + itemList[2] );
    addressee->setPrefix( itemList[4] );
    if ( checkDateTime( itemList[5],dt ) ) addressee->setBirthday( dt );
    addressee->setNote( itemList[6] );
    if ( checkDateTime( itemList[7],dt ) ) addressee->setRevision( dt );
    // addressee->setStatus( itemList[8] ); Status
    // addressee->xxx( itemList[9] ); Address_link_id
    categoriesOfAddressee[ itemList[0] ] = itemList[10];
    addresseeMap[ itemList[0] ] = addressee;

    line = gmxStream.readLine();
  }

  // now read the address records
  line  = gmxStream.readLine();
  if ( !line.startsWith( QLatin1String( "AB_ADDRESS_RECORDS:" ) ) ) {
    kWarning() << "Could not find address records!";
    return addresseeList;
  }
  // Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,
  // Mobile_type,Email,Homepage,Position,Comments,Record_type_id,Record_type,
  // Company,Department,Change_date,Preferred,Status
  line = gmxStream.readLine();
  line = gmxStream.readLine();

  while ( !line.startsWith( QLatin1String( "####" ) ) && !gmxStream.atEnd() ) {
    // an address entry may spread over several lines in the file
    while ( 1 ) {
       itemList = line.split( '#', QString::KeepEmptyParts );
       if ( itemList.count() >= 21 )
           break;
       line.append( '\n' );
       line.append( gmxStream.readLine() );
    };

    KABC::Addressee *addressee = addresseeMap[ itemList[0] ];
    if ( addressee ) {
      // itemList[1] = Record_id (numbered item, ignore here)
      int recordTypeId = itemList[14].toInt();
      KABC::Address::Type addressType;
      KABC::PhoneNumber::Type phoneType;
      switch ( recordTypeId ) {
        case typeHome:
          addressType = KABC::Address::Home;
          phoneType = KABC::PhoneNumber::Home;
          break;
        case typeWork:
          addressType = KABC::Address::Work;
          phoneType = KABC::PhoneNumber::Work;
          break;
        case typeOther:
        default:
          addressType = KABC::Address::Intl;
          phoneType = KABC::PhoneNumber::Voice;
          break;
      }
      KABC::Address address = addressee->address( addressType );
      address.setStreet(     itemList[2] );
      address.setCountry(    itemList[3] );
      address.setPostalCode( itemList[4] );
      address.setLocality(   itemList[5] );
      if ( !itemList[6].isEmpty() ) {
        addressee->insertPhoneNumber(
          KABC::PhoneNumber( itemList[6], phoneType ) );
      }
      if ( !itemList[7].isEmpty() )
        addressee->insertPhoneNumber(
          KABC::PhoneNumber( itemList[7], KABC::PhoneNumber::Fax ) );
      KABC::PhoneNumber::Type cellType = KABC::PhoneNumber::Cell;
      // itemList[9]=Mobile_type // always 0 or -1(default phone).
      // if ( itemList[19].toInt() & 4 ) cellType |= KABC::PhoneNumber::Pref;
      // don't do the above to avoid duplicate mobile numbers
      if ( !itemList[8].isEmpty() )
        addressee->insertPhoneNumber(
          KABC::PhoneNumber( itemList[8], cellType ) );
      bool preferred = false;
      if ( itemList[19].toInt() & 1 )
        preferred = true;
      addressee->insertEmail( itemList[10], preferred );
      if ( !itemList[11].isEmpty() )
        addressee->setUrl( itemList[11] );
      if ( !itemList[12].isEmpty() )
        addressee->setRole( itemList[12] );
      // itemList[13]=Comments
      // itemList[14]=Record_type_id (0,1,2) - see above
      // itemList[15]=Record_type (name of this additional record entry)
      if ( !itemList[16].isEmpty() )
        addressee->setOrganization( itemList[16] ); // Company
      if ( !itemList[17].isEmpty() ) addressee->insertCustom(
        "KADDRESSBOOK", "X-Department", itemList[17] ); // Department
      if ( checkDateTime( itemList[18],dt ) )
        addressee->setRevision( dt ); // Change_date
      // itemList[19]=Preferred (see above)
      // itemList[20]=Status (should always be "1")
      addressee->insertAddress( address );
    } else {
      kWarning() << "unresolved line:" << line;
    }
    line = gmxStream.readLine();
  }

  // extract the categories from the list of addressees of the file to import
  QStringList usedCategoryList;
  line = gmxStream.readLine();
  line2 = gmxStream.readLine();
  if ( !line.startsWith( QLatin1String( "AB_CATEGORIES:" ) ) ||
    !line2.startsWith( QLatin1String( "Category_id" ) ) ) {
    kWarning() << "Could not find category records!";
  } else {
    while ( !line.startsWith( QLatin1String( "####" ) ) &&
            !gmxStream.atEnd() ) {
      // a category should not spread over multiple lines, but just in case
      while ( 1 ) {
        itemList = line.split( '#', QString::KeepEmptyParts );
        if ( itemList.count() >= 3 )
          break;
        line.append( '\n' );
        line.append( gmxStream.readLine() );
      };
      usedCategoryList.append( itemList[1] );
      line = gmxStream.readLine();
    };
  }

  // now add the addresses to addresseeList
  for ( AddresseeMap::Iterator addresseeIt = addresseeMap.begin();
    addresseeIt != addresseeMap.end(); ++addresseeIt ) {
    KABC::Addressee *addressee = addresseeIt.value();
    // Add categories
    // catgories is a bitfield with max 31 defined categories
    int categories = categoriesOfAddressee[ addresseeIt.key() ].toInt();
    for ( int i=32; i >= 0; --i ) {
      // convert category index to bitfield value for comparison
      int catBit =  1<<i;
      if ( catBit > categories ) continue; // current index unassigned
      if ( catBit & categories  && usedCategoryList.count() > i )
        addressee->insertCategory( usedCategoryList[i] );
    }
    addresseeList.append( *addressee );
    delete addressee;
  }

  file.close();
  return addresseeList;
}


/* export */

bool GMXXXPort::exportContacts( const KABC::AddresseeList &list ) const
{
  KUrl url = KFileDialog::getSaveUrl(
    KUrl( QDir::homePath() + "/addressbook.gmx" ), GMX_FILESELECTION_STRING );
  if ( url.isEmpty() )
      return true;

  if( QFileInfo( url.isLocalFile() ?
    url.toLocalFile() : url.path() ).exists() ) {
    if( KMessageBox::questionYesNo( parentWidget(),
      i18n( "Do you want to overwrite file \"%1\"",
      url.isLocalFile() ? url.toLocalFile() : url.path() ) )==KMessageBox::No )
      return false;
  }

  if ( !url.isLocalFile() ) {
    KTemporaryFile tmpFile;
    if ( !tmpFile.open() ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b></qt>", url.url() );
      KMessageBox::error( parentWidget(), txt );
      return false;
    }

    doExport( &tmpFile, list );
    tmpFile.flush();

    return KIO::NetAccess::upload( tmpFile.fileName(), url, parentWidget() );
  } else {
    QString fileName = url.toLocalFile();
    QFile file( fileName );

    if ( !file.open( QIODevice::WriteOnly ) ) {
      QString txt = i18n( "<qt>Unable to open file <b>%1</b>.</qt>",
                          fileName );
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
  if ( !dt.isValid() )
    return QString::fromLatin1( "1000-01-01 00:00:00" );
  QString d( dt.toString( Qt::ISODate ) );
  d[10] = ' '; // remove the "T" in the middle of the string
  return d;
}

static const QStringList assignedCategoriesSorted(
  const KABC::AddresseeList &list )
{
  // Walk through the addressees and return a unique list of up to 31
  // categories, alphabetically sorted
  QStringList categoryList;
  const KABC::Addressee *addressee;
  for ( KABC::AddresseeList::ConstIterator addresseeIt = list.begin();
    addresseeIt != list.end() && categoryList.count() < 32; ++addresseeIt ) {
    addressee = &( *addresseeIt );
    if ( addressee->isEmpty() ) continue;
    const QStringList categories = addressee->categories();
    for ( int i=0; i < categories.count() && categoryList.count() < 32; ++i ) {
      if ( !categoryList.contains( categories[i]) )
        categoryList.append( categories[i] );
    }
  }
  categoryList.sort();
  return categoryList;
}

void GMXXXPort::doExport( QFile *fp, const KABC::AddresseeList &list ) const
{
  if ( !fp || !list.count() )
    return;

  QTextStream t( fp );
  t.setCodec( "ISO 8859-1" );

  typedef QMap<int, const KABC::Addressee *> AddresseeMap;
  AddresseeMap addresseeMap;
  const KABC::Addressee *addressee;

  t << "AB_ADDRESSES:\n";
  t << "Address_id,Nickname,Firstname,Lastname,Title,Birthday,Comments,"
       "Change_date,Status,Address_link_id,Categories\n";

  QList<QString> categoryMap;
  categoryMap.append( assignedCategoriesSorted( list ) );

  int addresseeId = 0;
  const QChar DELIM( '#' );
  for ( KABC::AddresseeList::ConstIterator it = list.begin();
    it != list.end(); ++it ) {
    addressee = &(*it);
    if ( addressee->isEmpty() )
       continue;
    addresseeMap[ ++addresseeId ] = addressee;

    // Assign categories as bitfield
    const QStringList categories = addressee->categories();
    long int category = 0;
    if ( categories.count() > 0 ) {
      for ( int i=0; i < categories.count(); i++ ) {
        if ( categoryMap.contains( categories[i] ) )
          category |= 1 << categoryMap.indexOf( categories[i], 0 ) ;
      }
    }

    // GMX sorts by nickname by default - don't leave empty
    QString nickName = addressee->nickName();
    if ( nickName.isEmpty() )
      nickName = addressee->formattedName();

    t << addresseeId << DELIM             // Address_id
      << nickName << DELIM                // Nickname
      << addressee->givenName() << DELIM  // Firstname
      << addressee->familyName() << DELIM // Lastname
      << addressee->prefix() << DELIM     // Title - Note: ->title()
                                          // refers to the professional title
      << dateString( addressee->birthday() ) << DELIM   // Birthday
      << addressee->note() /*.replace('\n',"\r\n")*/ << DELIM // Comments
      << dateString( addressee->revision() ) << DELIM   // Change_date
      << "1" << DELIM                     // Status
      << DELIM                            // Address_link_id
      << category << endl;                // Categories
  }

  t << "####\n";
  t << "AB_ADDRESS_RECORDS:\n";
  t << "Address_id,Record_id,Street,Country,Zipcode,City,Phone,Fax,Mobile,"
       "Mobile_type,Email,Homepage,Position,Comments,Record_type_id,Record_type,"
       "Company,Department,Change_date,Preferred,Status\n";

  addresseeId = 1;
  while ( ( addressee = addresseeMap[ addresseeId ] ) != NULL ) {

    const KABC::PhoneNumber::List cellPhones =
      addressee->phoneNumbers( KABC::PhoneNumber::Cell );

    const QStringList emails = addressee->emails();

    for ( int recId=0; recId<3; ++recId ) {
      KABC::Address address;
      KABC::PhoneNumber phone, fax, cell;

      // address preference flag:
      // & 1: preferred email address
      // & 4: preferred cell phone
      int prefFlag=0;

      switch ( recId ) {
      // Assign address, phone and cellphone, fax if applicable
        case typeHome:
          address = addressee->address( KABC::Address::Home );
          phone   = addressee->phoneNumber( KABC::PhoneNumber::Home );
          if ( cellPhones.count() > 0 ) {
            cell  = cellPhones.at( 0 );
            if ( !cell.isEmpty() )
              prefFlag |= 4;
          }
          break;
        case typeWork:
          address = addressee->address( KABC::Address::Work );
          phone   = addressee->phoneNumber( KABC::PhoneNumber::Work );
          if ( cellPhones.count() >= 2 )
            cell  = cellPhones.at( 1 );
          fax     = addressee->phoneNumber( KABC::PhoneNumber::Fax );
          break;
        case typeOther:
        default:
          if ( addressee->addresses( KABC::Address::Home ).count() > 1 )
            address = addressee->addresses( KABC::Address::Home ).at( 1 );
          if ( ( address.isEmpty() ) &&
               ( addressee->addresses( KABC::Address::Work ).count() > 1 ) )
            address = addressee->addresses( KABC::Address::Work ).at( 1 );
          if ( address.isEmpty() )
            address = addressee->address( KABC::Address::Dom );
          if ( address.isEmpty() )
            address = addressee->address( KABC::Address::Intl );
          if ( address.isEmpty() )
            address = addressee->address( KABC::Address::Postal );
          if ( address.isEmpty() )
            address = addressee->address( KABC::Address::Parcel );

          if ( addressee->phoneNumbers( KABC::PhoneNumber::Home ).count() > 1 )
            phone = addressee->phoneNumbers( KABC::PhoneNumber::Home ).at( 1 );
          if ( ( phone.isEmpty() ) && ( addressee->phoneNumbers(
            KABC::PhoneNumber::Work ).count() > 1 ) )
            phone = addressee->phoneNumbers( KABC::PhoneNumber::Work ).at( 1 );
          if ( phone.isEmpty() )
            phone = addressee->phoneNumber( KABC::PhoneNumber::Voice );
          if ( phone.isEmpty() )
            phone = addressee->phoneNumber( KABC::PhoneNumber::Msg );
          if ( phone.isEmpty() )
            phone = addressee->phoneNumber( KABC::PhoneNumber::Isdn );
          if ( phone.isEmpty() )
            phone = addressee->phoneNumber( KABC::PhoneNumber::Car );
          if ( phone.isEmpty() )
            phone = addressee->phoneNumber( KABC::PhoneNumber::Pager );

          switch ( cellPhones.count() ) {
            case 0: break;
            case 1:
            case 2:
              if ( !address.isEmpty() )
                cell = cellPhones.at( 0 );
              break;
            default:
              cell = cellPhones.at( 2 );
              break;
          }
          break;
      }

      QString email="";
      if ( emails.count()>recId ) {
        email = emails[ recId ];
        if ( email == addressee->preferredEmail() ) prefFlag |= 1;
      }

      if ( !address.isEmpty() || !phone.isEmpty() ||
           !cell.isEmpty()    || !email.isEmpty() ) {
        t << addresseeId << DELIM             // Address_id
          << recId << DELIM                   // Record_id
          << address.street() << DELIM        // Street
          << address.country() << DELIM       // Country
          << address.postalCode() << DELIM    // Zipcode
          << address.locality() << DELIM      // City
          << phone.number() << DELIM          // Phone
          << fax.number() << DELIM            // Fax
          << cell.number() << DELIM           // Mobile
          << ((recId==typeWork)?0:1) << DELIM // Mobile_type
          << email << DELIM                   // Email
          << ((recId==typeWork)?addressee->url().url():QString()) << DELIM
                                              // Homepage
          << ((recId==typeWork)?addressee->role():QString()) << DELIM
                                              // Position
          << ((recId==typeHome)?addressee->custom(
             "KADDRESSBOOK", "X-SpousesName" ):QString() ) << DELIM // Comments
          << recId << DELIM                   // Record_type_id (0,1,2)
          << DELIM                            // Record_type
          << ((recId==typeWork)?addressee->organization():QString()) << DELIM
                                              // Company
          << ((recId==typeWork)?addressee->custom(
             "KADDRESSBOOK", "X-Department" ):QString() )<< DELIM // Department
          << dateString( addressee->revision() ) << DELIM // Change_date
          << prefFlag << DELIM                // Preferred:
                                              // ( & 1: preferred email,
                                              //   & 4: preferred cell phone )
          << 1 << endl;                       // Status (should always be "1")
      }
    }

    ++addresseeId;
  };

  t << "####" << endl;
  t << "AB_CATEGORIES:" << endl;
  t << "Category_id,Name,Icon_id" << endl;

  //  Write Category List (beware: Category_ID 0 is reserved for none
  //  Interestingly: The index here is an int sequence and does not
  //  correspond to the bit reference used above.
  for ( int i = 0; i < categoryMap.size(); i++ ) {
    t << ( i + 1 ) << DELIM << categoryMap.at( i ) << DELIM << 0 << endl;
  }
  t << "####" << endl;
}

