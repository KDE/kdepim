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

#include <kdebug.h>


#include "device.h"
#include "addressbook.h"


using namespace OpieHelper;

AddressBook::AddressBook( CategoryEdit *edit,
                          KSync::KonnectorUIDHelper* helper,
                          const QString &tz,
                          bool meta, Device *dev )
    : Base( edit,  helper,  tz,  meta, dev )
{
}
AddressBook::~AddressBook(){
}
KSync::AddressBookSyncee* AddressBook::toKDE( const QString &fileName, ExtraMap& map )
{
    KSync::AddressBookSyncee *syncee = new KSync::AddressBookSyncee();
    syncee->setSource( "Opie");
    if( device() )
	syncee->setSupports( device()->supports( Device::Addressbook ) );

    //return entry;
    QFile file( fileName );
    if( !file.open(IO_ReadOnly ) ){
        //delete syncee; there is not addressbook so to get one synced we need to add an empty Syncee
	return syncee;
    }
    QDomDocument doc("mydocument" );
    if( !doc.setContent( &file ) ){
	file.close();
        delete syncee;
	return 0;
    }


    QDomElement docElem = doc.documentElement( );
    QDomNode n =  docElem.firstChild();
    QStringList attr = attributes();
    while(!n.isNull() ){
	QDomElement e = n.toElement();
	if(!e.isNull() ){
	    kdDebug(5228) << "Tage Name" << e.tagName() << endl;
	    if( e.tagName() == QString::fromLatin1("Contacts" ) ){ // we're looking for them
		QDomNode no = e.firstChild();
		while(!no.isNull() ){
		    QDomElement el = no.toElement();
		    if(!el.isNull() ){
			kdDebug(5228) << "Contacts: " << el.tagName() << endl;
			KABC::Addressee adr;
			adr.setUid( kdeId( "AddressBookSyncEntry",  el.attribute("Uid" ) ) );
			adr.setFamilyName(el.attribute("LastName" ) );
			adr.setGivenName(el.attribute("FirstName" ) );
			adr.setAdditionalName(el.attribute("MiddleName" )  );
			adr.setSuffix(el.attribute("Suffix") );
			adr.setNickName(el.attribute("Nickname" ) );

                        QDate date = dateFromString(el.attribute("Birthday") );
                        if (date.isValid() )
                            adr.setBirthday( date );

			adr.setRole(el.attribute("JobTitle" ) );
                        if ( !el.attribute("FileAs").isEmpty() )
                            adr.setFormattedName( el.attribute("FileAs" ) );

			adr.setOrganization( el.attribute("Company") );
			KABC::PhoneNumber businessPhoneNum(el.attribute("BusinessPhone"),
							   KABC::PhoneNumber::Work   );
			KABC::PhoneNumber businessFaxNum ( el.attribute("BusinessFax"),
							   KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax  );
			KABC::PhoneNumber businessMobile ( el.attribute("BusinessMobile"),
							   KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
			KABC::PhoneNumber businessPager( el.attribute("BusinessPager"),
							 KABC::PhoneNumber::Work | KABC::PhoneNumber::Pager);
			adr.insertPhoneNumber( businessPhoneNum );
			adr.insertPhoneNumber( businessFaxNum );
			adr.insertPhoneNumber( businessMobile );
			adr.insertPhoneNumber( businessPager  );

			// Handle multiple mail addresses
                        QString DefaultEmail = el.attribute("DefaultEmail");
                        if ( !DefaultEmail.isEmpty() )
                            adr.insertEmail( DefaultEmail, true ); // preferred
                        QString Emails = el.attribute("Emails");
			int emailCount = 1;
			QString Email = Emails.section( ' ', 1, 1, QString::SectionSkipEmpty);
			while ( !Email.isEmpty() ) {
			    // Handle all the secondary emails ...
                            if (Email != DefaultEmail)
                                adr.insertEmail( Email, false );
			    emailCount++;
			    Email = Emails.section( ' ', emailCount, emailCount, QString::SectionSkipEmpty);
			}


			KABC::PhoneNumber homePhoneNum( el.attribute("HomePhone"),
							KABC::PhoneNumber::Home );

			KABC::PhoneNumber homeFax (el.attribute("HomeFax"),
						   KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );

			KABC::PhoneNumber homeMobile( el.attribute("HomeMobile"),
						     KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
			adr.insertPhoneNumber(homePhoneNum );
			adr.insertPhoneNumber(homeFax );
			adr.insertPhoneNumber(homeMobile );
			KABC::Address business( KABC::Address::Work );

			business.setStreet  ( el.attribute("BusinessStreet") );
			business.setLocality( el.attribute("BusinessCity"  ) );
			business.setRegion  ( el.attribute("BusinessState" ) );
			business.setPostalCode( el.attribute("BusinessZip")  );
                        business.setCountry( el.attribute("BusinessCountry") );

			adr.insertAddress( business );

			KABC::Address home( KABC::Address::Home );
			home.setStreet( el.attribute("HomeStreet") );
			home.setLocality( el.attribute("HomeCity") );
			home.setRegion( el.attribute("HomeState") );
			home.setPostalCode( el.attribute("HomeZip") );
                        home.setCountry( el.attribute("HomeCountry") );
			adr.insertAddress( home );
			//el.attribute("Birthday");

			adr.setNickName( el.attribute("Nickname") );
			adr.setNote( el.attribute("Notes") );

		        /* extra block to let memory cleared... */
			{
			QStringList categories = QStringList::split(";", el.attribute("Categories" ) );
                        QString cat;
			QStringList added;
			for(uint i=0; i < categories.count(); i++ ){
                            cat = m_edit->categoryById(categories[i], "Contacts"  );
			    /* if name is not empty and we did not add the cat
			     * try to repair broken files
			     */
                            if (!cat.isEmpty() && !added.contains(cat) ){
                                adr.insertCategory( cat );
				added << cat;
			    }
			}
			}

                        adr.insertCustom("KADDRESSBOOK", "X-Department",  el.attribute("Department") );
			adr.insertCustom("opie", "HomeWebPage", el.attribute("HomeWebPage") );
			adr.insertCustom("KADDRESSBOOK", "X-SpouseName", el.attribute("Spouse") );
			adr.insertCustom("opie", "Gender", el.attribute("Gender") );

                        /*
                         * Anniversary block
                         * KADDRESSBOOK what it as ISO DATE string
                         * and we either have it in Opie old or Qtopia1.6 format
                         * So from String to Date and this Date will be exported to ISO Date again
                         */
                        {
                            QDate ann = dateFromString( el.attribute("Anniversary") );
                            if (ann.isValid() ) {
                                adr.insertCustom("KADDRESSBOOK", "X-Anniversary", ann.toString( Qt::ISODate) );
                            }
                        }
			adr.insertCustom("opie", "Children", el.attribute("Children") );
			adr.insertCustom("KADDRESSBOOK", "X-Office", el.attribute("Office") );
			adr.insertCustom("KADDRESSBOOK", "X-Profession", el.attribute("Profession") );
			adr.insertCustom("KADDRESSBOOK", "X-AssistantsName", el.attribute("Assistant") );
			adr.insertCustom("KADDRESSBOOK", "X-ManagersName", el.attribute("Manager") );
                        adr.setRevision( QDateTime::currentDateTime() );
                        KSync::AddressBookSyncEntry* entry = new KSync::AddressBookSyncEntry( adr );
			syncee->addEntry ( entry );

                        // now on to the extra stuff
                        map.add( "addressbook", el.attribute("Uid"), el.attributes(), attr );
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
    kdDebug(5228 ) << "From KDE " << endl;
    //  ok lets write back the changes from the Konnector
    m_kde2opie.clear(); // clear the reference first
    Kontainer::ValueList newIds = syncee->ids( "AddressBookSyncEntry");
    for ( Kontainer::ValueList::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("AddressBookSyncEntry",  (*idIt).first(),  (*idIt).second() ); // FIXME update this name later
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
            if (entry->state() == KSync::SyncEntry::Removed )
                continue;
            ab = entry->addressee();
            *stream << "<Contact ";
            *stream << "FirstName=\"" << escape(ab.givenName()) << "\" ";
            *stream << "MiddleName=\"" << escape(ab.additionalName()) << "\" ";
            *stream << "LastName=\"" << escape(ab.familyName()) << "\" ";
            *stream << "Suffix=\"" << escape(ab.suffix()) << "\" ";

            QString sortStr;
            sortStr = ab.formattedName();
            /* is formattedName is empty we use the assembled name as fallback */
            if (sortStr.isEmpty() )
                sortStr = ab.assembledName();
            *stream << "FileAs=\"" << escape(sortStr) << "\" ";

            *stream << "JobTitle=\"" << escape(ab.role()) << "\" ";
            *stream << "Department=\"" << escape(ab.custom( "KADDRESSBOOK", "X-Department" )) << "\" ";
            *stream << "Company=\"" << escape(ab.organization()) << "\" ";

            KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
            *stream << "BusinessPhone=\"" << escape( businessPhoneNum.number() ) << "\" ";

            KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
            *stream << "BusinessFax=\"" << escape( businessFaxNum.number() )<< "\" ";

            KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
            *stream << "BusinessMobile=\"" << escape( businessMobile.number() ) << "\" ";

            *stream << "DefaultEmail=\"" << escape( ab.preferredEmail() ) << "\" ";
            QStringList list = ab.emails();
            if ( list.count() > 0 ) {
		QStringList::Iterator it = list.begin();
                *stream << "Emails=\"" << escape( *it );
		while (++it != list.end())
		  *stream << ' ' << escape( *it );
                *stream << "\" ";
	    }

            KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
            *stream << "HomePhone=\"" << escape( homePhoneNum.number() ) << "\" ";

            KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
            *stream << "HomeFax=\"" << escape( homeFax.number() ) << "\" ";

            KABC::PhoneNumber homeMobile = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
            *stream << "HomeMobile=\"" << escape( homeMobile.number() ) << "\" ";

            KABC::Address business = ab.address(KABC::Address::Work  );
            *stream << "BusinessStreet=\"" << escape( business.street() ) << "\" ";
            *stream << "BusinessCity=\"" << escape( business.locality() ) << "\" ";
            *stream << "BusinessZip=\"" << escape( business.postalCode() ) << "\" ";
            *stream << "BusinessCountry=\"" << escape( business.country() ) << "\" ";
            *stream << "BusinessState=\"" << escape( business.region() ) << "\" ";
            //stream << "BusinessPager=\"" << << "\" ";
            *stream << "Office=\"" << escape( ab.custom( "KADDRESSBOOK",  "X-Office" ) ) << "\" ";
            *stream << "Profession=\"" << escape( ab.custom( "KADDRESSBOOK",  "X-Profession" ) ) << "\" ";
            *stream << "Assistant=\"" << escape( ab.custom( "KADDRESSBOOK",  "X-AssistantsName") ) << "\" ";
            *stream << "Manager=\"" << escape( ab.custom( "KADDRESSBOOK",  "X-ManagersName" ) ) << "\" ";

            KABC::Address home = ab.address( KABC::Address::Home );
            *stream << "HomeStreet=\"" << escape( home.street() ) << "\" ";
            *stream << "HomeCity=\"" <<  escape( home.locality() ) << "\" ";
            *stream << "HomeState=\"" <<  escape( home.region() ) << "\" ";
            *stream << "HomeZip=\"" <<  escape( home.postalCode() ) << "\" ";
            *stream << "HomeCountry=\"" << escape( home.country() ) << "\" ";

            *stream << "HomeWebPage=\"" << escape( ab.custom( "opie", "HomeWebPage" ) ) << "\" ";
            *stream << "Spouse=\"" << escape( ab.custom( "KADDRESSBOOK",  "X-SpousesName") ) << "\" ";
            *stream << "Gender=\"" << escape( ab.custom( "opie",  "Gender") ) << "\" ";

            if ( ab.birthday().date().isValid() )
                *stream << "Birthday=\"" << escape( dateToString(ab.birthday().date() ) ) << "\" ";

            /*
             * Anniversary block again
             * Go from ISO -> QDate -> toString and then escape
             */
            {
                QDate ann = QDate::fromString( ab.custom("KADDRESSBOOK", "X-Anniversary"), Qt::ISODate );
                if (ann.isValid() ) {
                    *stream << "Anniversary=\"" << escape( dateToString( ann )  ) << "\" ";
                }
            }
            *stream << "Nickname=\"" << escape( ab.nickName() ) << "\" ";
            *stream << "Children=\"" << escape( ab.custom("opie", "Children" ) ) << "\" ";
            *stream << "Notes=\"" << escape( ab.note() ) << "\" ";
            *stream << "Categories=\"" << categoriesToNumber( ab.categories(),  "Contacts") << "\" ";

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

QStringList AddressBook::attributes()const {
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
