/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qdom.h>
#include <qfile.h>

#include <klocale.h>


#include "device.h"
#include "addressbook.h"


using namespace OpieHelper;

AddressBook::AddressBook( CategoryEdit *edit,
                          KSync::KonnectorUIDHelper* helper,
                          const QString &tz,
                          Device *dev )
    : Base( edit,  helper,  tz, dev )
{
}
AddressBook::~AddressBook(){
}

KSync::AddressBookSyncee* AddressBook::toKDE( const QString &fileName, ExtraMap& map )
{
  KSync::AddressBookSyncee *syncee = new KSync::AddressBookSyncee();
  syncee->setTitle( i18n("Opie") );
  syncee->setIdentifier( "Opie-Addresses" );

  //return entry;
  QFile file( fileName );
  if ( !file.open(IO_ReadOnly ) ) {
    //delete syncee; there is not addressbook so to get one synced we need to add an empty Syncee
    return syncee;
  }

  QDomDocument doc("mydocument" );
  if ( !doc.setContent( &file ) ) {
    file.close();
    delete syncee;
    return 0;
  }


  QDomElement docElem = doc.documentElement( );
  QDomNode n =  docElem.firstChild();
  QStringList attr = supportedAttributes();
  while ( !n.isNull() ) {
    QDomElement e = n.toElement();
    if ( !e.isNull() ) {
      if ( e.tagName() == QString::fromLatin1( "Contacts" ) ) { // we're looking for them
        QDomNode no = e.firstChild();
        while ( !no.isNull() ) {
          QDomElement el = no.toElement();
          if ( !el.isNull() ) {
            KABC::Addressee adr;
            adr.setUid( kdeId( "AddressBookSyncEntry",  el.attribute("Uid" ) ) );
            adr.setFamilyName( el.attribute( "LastName" ) );
            adr.setGivenName( el.attribute( "FirstName" ) );
            adr.setAdditionalName( el.attribute( "MiddleName" )  );
            adr.setSuffix( el.attribute( "Suffix" ) );
            adr.setNickName( el.attribute( "Nickname" ) );

            QDate date = dateFromString( el.attribute( "Birthday" ) );
            if ( date.isValid() )
              adr.setBirthday( date );

            adr.setRole( el.attribute( "JobTitle" ) );
            if ( !el.attribute( "FileAs" ).isEmpty() )
              adr.setFormattedName( el.attribute( "FileAs" ) );

            adr.setOrganization( el.attribute( "Company" ) );

            KABC::PhoneNumber businessPhoneNum( el.attribute( "BusinessPhone" ),
                                                KABC::PhoneNumber::Work );
            KABC::PhoneNumber businessFaxNum( el.attribute( "BusinessFax" ),
                                              KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
            KABC::PhoneNumber businessMobile( el.attribute( "BusinessMobile" ),
                                              KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
            KABC::PhoneNumber businessPager( el.attribute( "BusinessPager" ),
                                             KABC::PhoneNumber::Work | KABC::PhoneNumber::Pager );
            if ( !businessPhoneNum.number().isEmpty() )
              adr.insertPhoneNumber( businessPhoneNum );
            if ( !businessFaxNum.number().isEmpty() )
              adr.insertPhoneNumber( businessFaxNum );
            if ( !businessMobile.number().isEmpty() )
              adr.insertPhoneNumber( businessMobile );
            if ( !businessPager.number().isEmpty() )
              adr.insertPhoneNumber( businessPager  );

            // Handle multiple mail addresses
            QString DefaultEmail = el.attribute( "DefaultEmail" );
            if ( !DefaultEmail.isEmpty() )
              adr.insertEmail( DefaultEmail, true ); // preferred

            QString Emails = el.attribute("Emails");
            int emailCount = 1;
            QString Email = Emails.section( ' ', 1, 1, QString::SectionSkipEmpty );
            while ( !Email.isEmpty() ) {
              // Handle all the secondary emails ...
              if ( Email != DefaultEmail )
                adr.insertEmail( Email, false );
              emailCount++;
              Email = Emails.section( ' ', emailCount, emailCount, QString::SectionSkipEmpty );
            }


            KABC::PhoneNumber homePhoneNum( el.attribute( "HomePhone" ),
                                            KABC::PhoneNumber::Home );
            KABC::PhoneNumber homeFax( el.attribute( "HomeFax" ),
                                       KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );

            KABC::PhoneNumber homeMobile( el.attribute( "HomeMobile" ),
                                          KABC::PhoneNumber::Cell );

            if ( !homePhoneNum.number().isEmpty() )
              adr.insertPhoneNumber( homePhoneNum );
            if ( !homeFax.number().isEmpty() )
              adr.insertPhoneNumber( homeFax );
            if ( !homeMobile.number().isEmpty() )
              adr.insertPhoneNumber( homeMobile );

            KABC::Address business( KABC::Address::Work );
            business.setStreet( el.attribute( "BusinessStreet" ) );
            business.setLocality( el.attribute( "BusinessCity"  ) );
            business.setRegion( el.attribute( "BusinessState" ) );
            business.setPostalCode( el.attribute( "BusinessZip" )  );
            business.setCountry( el.attribute( "BusinessCountry" ) );

            if ( !business.isEmpty() )
              adr.insertAddress( business );

            KABC::Address home( KABC::Address::Home );
            home.setStreet( el.attribute( "HomeStreet" ) );
            home.setLocality( el.attribute( "HomeCity" ) );
            home.setRegion( el.attribute( "HomeState" ) );
            home.setPostalCode( el.attribute( "HomeZip" ) );
            home.setCountry( el.attribute( "HomeCountry" ) );

            if ( !home.isEmpty() )
              adr.insertAddress( home );

            adr.setNickName( el.attribute( "Nickname" ) );
            adr.setNote( el.attribute( "Notes" ) );

            {
              QStringList categories = QStringList::split(";", el.attribute("Categories" ) );
              QString cat;
              QStringList added;
              for ( uint i = 0; i < categories.count(); i++ ) {
                cat = m_edit->categoryById( categories[ i ], "Contacts" );

                // if name is not empty and we did not add the
                // cat try to repair broken files
                if ( !cat.isEmpty() && !added.contains( cat ) ) {
                  adr.insertCategory( cat );
                  added << cat;
                }
              }
            }

            if ( !el.attribute( "Department" ).isEmpty() )
              adr.insertCustom( "KADDRESSBOOK", "X-Department",  el.attribute( "Department" ) );
            if ( !el.attribute( "HomeWebPage" ).isEmpty() )
              adr.insertCustom( "opie", "HomeWebPage", el.attribute( "HomeWebPage" ) );
            if ( !el.attribute( "Spouse" ).isEmpty() )
              adr.insertCustom( "KADDRESSBOOK", "X-SpousesName", el.attribute( "Spouse" ) );
            if ( !el.attribute( "Gender" ).isEmpty() )
              adr.insertCustom( "opie", "Gender", el.attribute( "Gender" ) );

            QDate ann = dateFromString( el.attribute( "Anniversary" ) );
            if ( ann.isValid() ) {
              adr.insertCustom( "KADDRESSBOOK", "X-Anniversary", ann.toString( Qt::ISODate ) );
            }

            if ( !el.attribute( "Children" ).isEmpty() )
              adr.insertCustom("opie", "Children", el.attribute("Children") );
            if ( !el.attribute( "Office" ).isEmpty() )
              adr.insertCustom("KADDRESSBOOK", "X-Office", el.attribute("Office") );
            if ( !el.attribute( "Profession" ).isEmpty() )
              adr.insertCustom("KADDRESSBOOK", "X-Profession", el.attribute("Profession") );
            if ( !el.attribute( "Assistant" ).isEmpty() )
              adr.insertCustom("KADDRESSBOOK", "X-AssistantsName", el.attribute("Assistant") );
            if ( !el.attribute( "Manager" ).isEmpty() )
              adr.insertCustom("KADDRESSBOOK", "X-ManagersName", el.attribute("Manager") );

            KSync::AddressBookSyncEntry* entry = new KSync::AddressBookSyncEntry( adr, syncee );
            syncee->addEntry ( entry );

            // now on to the extra stuff
            map.add( "addressbook", el.attribute( "Uid" ), el.attributes(), attr );
          }

          no = no.nextSibling();
        }
      }
    }

    n = n.nextSibling();
  }

  return syncee;
}
KTempFile* AddressBook::fromKDE( KSync::AddressBookSyncee *syncee, ExtraMap& map )
{
    //  ok lets write back the changes from the Konnector
    m_kde2opie.clear(); // clear the reference first
    Kontainer::ValueList newIds = syncee->ids( "AddressBookSyncEntry");
    for ( Kontainer::ValueList::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("AddressBookSyncEntry",  (*idIt).first,  (*idIt).second ); // FIXME update this name later
    }
    KTempFile* tempFile = file();
    if ( tempFile->textStream() ) {
        QTextStream *stream = tempFile->textStream();
        stream->setEncoding( QTextStream::UnicodeUTF8 );
        *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook ><AddressBook>" << endl;
        *stream << " <Groups>" << endl;
        *stream << " </Groups>" << endl;
        *stream << " <Contacts> " << endl;
// for all entries
        KABC::Addressee ab;
        KSync::AddressBookSyncEntry *entry;
        for ( entry = syncee->firstEntry(); entry != 0l;  entry = syncee->nextEntry() ) {
            /* do not safe deleted records */
            if (entry->wasRemoved() )
                continue;

            ab = entry->addressee();
            *stream << "<Contact ";
            *stream << appendText( "FirstName=\"" + escape(ab.givenName()) + "\" ",
                                   "FirstName=\"\" " );
            *stream << appendText( "MiddleName=\"" + escape(ab.additionalName()) + "\" ",
                                   "MiddleName=\"\" " );
            *stream << appendText( "LastName=\"" + escape(ab.familyName()) + "\" ",
                                   "LastName=\"\" " );
            *stream << appendText( "Suffix=\"" + escape(ab.suffix()) + "\" ",
                                   "Suffix=\"\" " );

            QString sortStr;
            sortStr = ab.formattedName();
            /* is formattedName is empty we use the assembled name as fallback */
            if (sortStr.isEmpty() )
                sortStr = ab.assembledName();
            *stream << "FileAs=\"" + escape(sortStr) + "\" ";

            *stream << appendText( "JobTitle=\"" + escape(ab.role()) + "\" ",
                                   "JobTitle=\"\" " );
            *stream << appendText( "Department=\"" + escape(ab.custom( "KADDRESSBOOK", "X-Department" )) + "\" ",
                                   "Department=\"\" ");
            *stream << appendText( "Company=\"" + escape(ab.organization()) + "\" ",
                                   "Company=\"\" " );

            KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
            *stream << appendText( "BusinessPhone=\"" + escape( businessPhoneNum.number() ) + "\" ",
                                   "BusinessPhone=\"\" " );

            KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
            *stream << appendText( "BusinessFax=\"" + escape( businessFaxNum.number() ) + "\" ",
                                   "BusinessFax=\"\" " );

            KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
            *stream << appendText( "BusinessMobile=\"" + escape( businessMobile.number() ) + "\" ",
                                   "BusinessMobile=\"\" " );

            *stream << appendText( "DefaultEmail=\"" + escape( ab.preferredEmail() ) + "\" ",
                                   "DefaultEmail=\"\" " );
            QStringList list = ab.emails();
            if ( list.count() > 0 ) {
		QStringList::Iterator it = list.begin();
                *stream << "Emails=\"" << escape( *it );
		while (++it != list.end())
		  *stream << ' ' << escape( *it );
                *stream << "\" ";
	    }

            KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
            *stream << appendText( "HomePhone=\"" + escape( homePhoneNum.number() ) + "\" ",
                                   "HomePhone=\"\" " );

            KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
            *stream << appendText( "HomeFax=\"" + escape( homeFax.number() ) + "\" ",
                                   "HomeFax=\"\" " );

            KABC::PhoneNumber homeMobile = ab.phoneNumber( KABC::PhoneNumber::Cell );
            *stream << appendText( "HomeMobile=\"" + escape( homeMobile.number() ) + "\" ",
                                   "HomeMobile=\"\" " );
            KABC::Address business = ab.address(KABC::Address::Work  );
            *stream << appendText( "BusinessStreet=\"" + escape( business.street() ) + "\" ",
                                   "BusinessStreet=\"\" " );
            *stream << appendText( "BusinessCity=\"" + escape( business.locality() ) + "\" ",
                                   "BusinessCity=\"\" " );
            *stream << appendText( "BusinessZip=\"" + escape( business.postalCode() ) + "\" ",
                                   "BusinessZip=\"\" " );
            *stream << appendText( "BusinessCountry=\"" + escape( business.country() ) + "\" ",
                                   "BusinessCountry=\"\" " );
            *stream << appendText( "BusinessState=\"" + escape( business.region() ) + "\" ",
                                   "BusinessState=\"\" " );
            //stream << "BusinessPager=\"" << << "\" ";


            *stream << appendText( "Office=\"" + escape( ab.custom( "KADDRESSBOOK",  "X-Office" ) ) + "\" ",
                                   "Office=\"\" " );
            *stream << appendText( "Profession=\"" + escape( ab.custom( "KADDRESSBOOK",  "X-Profession" ) ) + "\" ",
                                   "Profession=\"\" " );
            *stream << appendText( "Assistant=\"" + escape( ab.custom( "KADDRESSBOOK",  "X-AssistantsName") ) + "\" ",
                                   "Assistant=\"\" " );
            *stream << appendText( "Manager=\"" + escape( ab.custom( "KADDRESSBOOK",  "X-ManagersName" ) ) + "\" ",
                                   "Manager=\"\" " );

            KABC::Address home = ab.address( KABC::Address::Home );
            *stream << appendText( "HomeStreet=\"" + escape( home.street() ) + "\" ",
                                   "HomeStreet=\"\" " );
            *stream << appendText( "HomeCity=\"" +  escape( home.locality() ) + "\" ",
                                   "HomeCity=\"\" " );
            *stream << appendText( "HomeState=\"" +  escape( home.region() ) + "\" ",
                                   "HomeState=\"\" " );
            *stream << appendText( "HomeZip=\"" +  escape( home.postalCode() ) + "\" ",
                                   "HomeZip=\"\" " );
            *stream << appendText( "HomeCountry=\"" + escape( home.country() ) + "\" ",
                                   "HomeCountry=\"\" ");

            *stream << appendText( "HomeWebPage=\"" + escape( ab.custom( "opie", "HomeWebPage" ) ) + "\" ",
                                   "HomeWebPage=\"\" " );
            *stream << appendText( "Spouse=\"" + escape( ab.custom( "KADDRESSBOOK",  "X-SpousesName") ) + "\" ",
                                   "Spouse=\"\" " );
            *stream << appendText( "Gender=\"" + escape( ab.custom( "opie",  "Gender") ) + "\" ",
                                   "Gender=\"\" " );

            if ( ab.birthday().date().isValid() )
                *stream << appendText( "Birthday=\"" + escape( dateToString(ab.birthday().date() ) ) + "\" ",
                                       "Birthday=\"\" " );

            /*
             * Anniversary block again
             * Go from ISO -> QDate -> toString and then escape
             */
            {
                QDate ann = QDate::fromString( ab.custom("KADDRESSBOOK", "X-Anniversary"), Qt::ISODate );
                if (ann.isValid() ) {
                    *stream << appendText( "Anniversary=\"" + escape( dateToString( ann )  ) + "\" ",
                                           "Anniversary=\"\" " );
                }
            }
            *stream << appendText( "Nickname=\"" + escape( ab.nickName() ) + "\" ",
                                   "Nickname=\"\" " );
            *stream << appendText( "Children=\"" + escape( ab.custom("opie", "Children" ) ) + "\" ",
                                   "Children=\"\" ");
            *stream << appendText( "Notes=\"" + escape( ab.note() ) + "\" ",
                                   "Notes=\"\" " );
            *stream << appendText("Categories=\"" +
                                  categoriesToNumber( ab.categories(), "Contacts") + "\" ",
                                  "Categories=\"\" " );

            QString uid = konnectorId( "AddressBookSyncEntry", ab.uid() );
            *stream << "Uid=\"" <<  uid << "\" ";
            *stream << map.toString( "addressbook", uid );
            *stream << " />" << endl;
        } // off for
        *stream << "</Contacts>" << endl;
        *stream << "</AddressBook>" << endl;
    }
    // now replace the UIDs for us
    m_helper->replaceIds( "AddressBookSyncEntry",  m_kde2opie ); // to keep the use small

    tempFile->close();

    return tempFile;
}

QStringList AddressBook::supportedAttributes() {
    QStringList lst;
    lst << "FirstName";
    lst << "MiddleName";
    lst << "LastName";
    lst << "Suffix";
    lst << "FileAs";
    lst << "JobTitle";
    lst << "Department";
    lst << "Company";
    lst << "BusinessPhone";
    lst << "BusinessFax";
    lst << "BusinessMobile";
    lst << "DefaultEmail";
    lst << "Emails";
    lst << "HomePhone";
    lst << "HomeFax";
    lst << "HomeMobile";
    lst << "BusinessStreet";
    lst << "BusinessCity";
    lst << "BusinessZip";
    lst << "BusinessCountry";
    lst << "BusinessState";
    lst << "Office";
    lst << "Profession";
    lst << "Assistant";
    lst << "Manager";
    lst << "HomeStreet";
    lst << "HomeCity";
    lst << "HomeState";
    lst << "HomeZip";
    lst << "HomeCountry";
    lst << "HomeWebPage";
    lst << "Spouse";
    lst << "Gender";
    lst << "Anniversary";
    lst << "Nickname";
    lst << "Children";
    lst << "Notes";
    lst << "Categories";
    lst << "Uid";
    lst << "Birthday";

    return lst;
}

// FROM TT timeconversion.cpp GPLed
QDate AddressBook::fromString( const QString &datestr )
{
    if (datestr.isEmpty() )
        return QDate();

    int monthPos = datestr.find('.');
    int yearPos = datestr.find('.', monthPos+1 );
    if ( monthPos == -1 || yearPos == -1 ) {
	return QDate();
    }
    int d = datestr.left( monthPos ).toInt();
    int m = datestr.mid( monthPos+1, yearPos - monthPos - 1 ).toInt();
    int y = datestr.mid( yearPos+1 ).toInt();
    QDate date ( y,m,d );


    return date;
}


QString AddressBook::dateToString( const QDate &d )
{
    if ( d.isNull() || !d.isValid() )
        return QString::null;

    // ISO format in year, month, day (YYYYMMDD); e.g. 20021231
    QString year = QString::number( d.year() );
    QString month = QString::number( d.month() );
    month = month.rightJustify( 2, '0' );
    QString day = QString::number( d.day() );
    day = day.rightJustify( 2, '0' );

    QString str = year + month + day;

    return str;
}

QDate AddressBook::dateFromString( const QString& s )
{
    QDate date;

    if ( s.isEmpty() )
        return date;

    // Be backward compatible to old Opie format:
    // Try to load old format. If it fails, try new ISO-Format!
    date = fromString ( s );
    if ( date.isValid() )
        return date;

    // Read ISO-Format (YYYYMMDD)
    int year = s.mid(0, 4).toInt();
    int month = s.mid(4,2).toInt();
    int day = s.mid(6,2).toInt();

    // do some quick sanity checking
    if ( year < 1900 || year > 3000 )
        return date;

    if ( month < 0 || month > 12 )
        return date;

    if ( day < 0 || day > 31 )
        return date;


    date.setYMD( year, month, day );

    if ( !date.isValid() )
        return QDate();


    return date;
}
