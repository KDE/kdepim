

#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kdebug.h>

#include <kabc/resourcefile.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

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
KSync::AddressBookSyncee* AddressBook::toKDE( const QString &fileName )
{
    KSync::AddressBookSyncee *syncee = new KSync::AddressBookSyncee();
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
        //delete syncee; same as above...
	return syncee;
    }


    QDomElement docElem = doc.documentElement( );
    QDomNode n =  docElem.firstChild();
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
			adr.setBirthday( QDate::fromString(el.attribute("Birthday")  ) );
			adr.setRole(el.attribute("JobTitle" ) );
			// inside into custom
			adr.setSortString( el.attribute("FileAs" ) );

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
                        QString email = el.attribute("Emails");
                        if (!email.isEmpty() )
                            adr.insertEmail( email, true ); // prefered

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

			QStringList categories = QStringList::split(";", el.attribute("Categories" ) );
			for(uint i=0; i < categories.count(); i++ ){
			  adr.insertCategory(m_edit->categoryById(categories[i], "Contacts"  ) );
			}
                        adr.insertCustom("opie", "Department",  el.attribute("Department") );
			adr.insertCustom("opie", "HomeWebPage", el.attribute("HomeWebPage") );
			adr.insertCustom("opie", "Spouse", el.attribute("Spouse") );
			adr.insertCustom("opie", "Gender", el.attribute("Gender") );
			adr.insertCustom("opie", "Anniversary", el.attribute("Anniversary") );
			adr.insertCustom("opie", "Children", el.attribute("Children") );
			adr.insertCustom("opie", "Office", el.attribute("Office") );
			adr.insertCustom("opie", "Profession", el.attribute("Profession") );
			adr.insertCustom("opie", "Assistant", el.attribute("Assistant") );
			adr.insertCustom("opie", "Manager", el.attribute("Manager") );
                        adr.setRevision( QDateTime::currentDateTime() );
                        KSync::AddressBookSyncEntry* entry = new KSync::AddressBookSyncEntry( adr );
			syncee->addEntry ( entry );
		    }
		    no = no.nextSibling();
		}
	    }
	}
	n = n.nextSibling();
    }
    return syncee;
}
KTempFile* AddressBook::fromKDE( KSync::AddressBookSyncee *syncee )
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
            *stream << "FirstName=\"" << ab.givenName() << "\" ";
            *stream << "MiddleName=\"" << ab.additionalName() << "\" ";
            *stream << "LastName=\"" << ab.familyName() << "\" ";
            *stream << "Suffix=\"" << ab.suffix() << "\" ";
            QString sortStr;
            sortStr = ab.sortString();
            if (sortStr.isEmpty() )
                sortStr = ab.formattedName();
            *stream << "FileAs=\"" << sortStr << "\" ";
            *stream << "JobTitle=\"" << ab.role() << "\" ";
            *stream << "Department=\"" << ab.custom( "opie", "Department" ) << "\" ";
            *stream << "Company=\"" << ab.organization() << "\" ";

            KABC::PhoneNumber businessPhoneNum = ab.phoneNumber(KABC::PhoneNumber::Work );
            *stream << "BusinessPhone=\"" <<businessPhoneNum.number() << "\" ";

            KABC::PhoneNumber businessFaxNum = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
            *stream << "BusinessFax=\"" << businessFaxNum.number() << "\" ";

            KABC::PhoneNumber businessMobile = ab.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
            *stream << "BusinessMobile=\"" << businessMobile.number() << "\" ";
            *stream << "DefaultEmail=\"" << ab.preferredEmail() << "\" ";
            QStringList list = ab.emails();
            if ( list.count() > 0 )
                *stream << "Emails=\"" << list[0] << "\" ";

            KABC::PhoneNumber homePhoneNum = ab.phoneNumber(KABC::PhoneNumber::Home );
            *stream << "HomePhone=\"" << homePhoneNum.number() << "\" ";

            KABC::PhoneNumber homeFax = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
            *stream << "HomeFax=\"" << homeFax.number() << "\" ";

            KABC::PhoneNumber homeMobile = ab.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
            *stream << "HomeMobile=\"" << homeMobile.number() << "\" ";

            KABC::Address business = ab.address(KABC::Address::Work  );
            *stream << "BusinessStreet=\"" << business.street() << "\" ";
            *stream << "BusinessCity=\"" << business.locality() << "\" ";
            *stream << "BusinessZip=\"" << business.postalCode() << "\" ";
            *stream << "BusinessCountry=\"" << business.country() << "\" ";
            *stream << "BusinessState=\"" << business.region() << "\" ";
            //stream << "BusinessPager=\"" << << "\" ";
            *stream << "Office=\"" << ab.custom( "opie",  "Office" ) << "\" ";
            *stream << "Profession=\"" << ab.custom( "opie",  "Profession" )<< "\" ";
            *stream << "Assistant=\"" << ab.custom( "opie",  "Assistant") << "\" ";
            *stream << "Manager=\"" << ab.custom( "opie",  "Manager" ) << "\" ";

            KABC::Address home = ab.address( KABC::Address::Home );
            *stream << "HomeStreet=\"" << home.street() << "\" ";
            *stream << "HomeCity=\"" << home.locality() << "\" ";
            *stream << "HomeState=\"" << home.region() << "\" ";
            *stream << "HomeZip=\"" << home.postalCode() << "\" ";
            *stream << "HomeCountry=\"" << home.country() << "\" ";

            *stream << "HomeWebPage=\"" << ab.custom( "opie", "HomeWebPage" ) << "\" ";
            *stream << "Spouse=\"" << ab.custom( "opie",  "Spouse") << "\" ";
            *stream << "Gender=\"" << ab.custom( "opie",  "Gender") << "\" ";
            *stream << "Birthday=\"" << ab.birthday().date().toString("dd.MM.yyyy") << "\" ";
            *stream << "Anniversary=\"" << ab.custom( "opie",  "Anniversary" ) << "\" ";
            *stream << "Nickname=\"" << ab.nickName() << "\" ";
            *stream << "Children=\"" << ab.custom("opie", "Children" ) << "\" ";
            *stream << "Notes=\"" << ab.note() << "\" ";
            *stream << "Categories=\"" << categoriesToNumber( ab.categories(),  "Contacts") << "\" ";
            *stream << "Uid=\"" << konnectorId( "AddressBookSyncEntry", ab.uid() ) << "\" ";
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
