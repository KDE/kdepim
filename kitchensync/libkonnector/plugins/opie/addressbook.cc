

#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kapplication.h>
#include <kdebug.h>

#include "addressbook.h"

using namespace OpieHelper;

AddressBook::AddressBook( CategoryEdit *edit,
                          KonnectorUIDHelper* helper,
                          bool meta )
    : Base( edit,  helper,  meta )
{

}
AddressBook::~AddressBook()
{

}
KAddressbookSyncEntry* AddressBook::toKDE( const QString &fileName )
{
    KAddressbookSyncEntry *entry = new KAddressbookSyncEntry();
    //return entry;
    QFile file( fileName );
    if( !file.open(IO_ReadOnly ) ){
        delete entry;
	return 0;
    }
    QDomDocument doc("mydocument" );
    if( !doc.setContent( &file ) ){
	file.close();
        delete entry;
	return 0l;
    }
    entry->setId( kapp->randomString(8) );
    KABC::AddressBook *abook = new KABC::AddressBook( );
    entry->setAddressbook( abook );


    QDomElement docElem = doc.documentElement( );
    QDomNode n =  docElem.firstChild();
    while(!n.isNull() ){
	QDomElement e = n.toElement();
	if(!e.isNull() ){
	    kdDebug() << e.tagName() << endl;
	    if( e.tagName() == QString::fromLatin1("Contacts" ) ){ // we're looking for them
		QDomNode no = e.firstChild();
		while(!no.isNull() ){
		    QDomElement el = no.toElement();
		    if(!el.isNull() ){
			kdDebug() << "Contacts: " << el.tagName() << endl;
			KABC::Addressee adr;
			adr.setUid( kdeId( "addressbook",  el.attribute("Uid" ) ) );
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
			adr.insertEmail( el.attribute("Emails"), true ); // prefered

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
			abook->insertAddressee(adr );
		    }
		    no = no.nextSibling();
		}
	    }
	}
	n = n.nextSibling();
    }
    /*    KABC::ResourceFile r( abook, "/home/ich/addressbook.vcf" );
    abook->addResource(&r );
    KABC::Ticket *t = abook->requestSaveTicket( &r );
    abook->save( t );*/
    kdDebug() << "Dumped " << endl;
    return entry;
}
QByteArray AddressBook::fromKDE( KAddressbookSyncEntry *entry )
{
    //  ok lets write back the changes from the Konnector
    m_kde2opie.clear(); // clear the reference first
    QValueList<Kontainer> newIds = entry->ids( "addressbook");
    for ( QValueList<Kontainer>::ConstIterator idIt = newIds.begin(); idIt != newIds.end(); ++idIt ) {
        m_helper->addId("addressbook",  (*idIt).first(),  (*idIt).second() );
    }
    QByteArray array;
    QBuffer buffer( array );
    if ( buffer.open( IO_WriteOnly) ) {
        QTextStream stream( &buffer );
        stream.setEncoding( QTextStream::UnicodeUTF8 );
        stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?><!DOCTYPE Addressbook ><AddressBook>" << endl;
        stream << " <Groups>" << endl;
        stream << " </Groups>" << endl;
        stream << " <Contacts> " << endl;
// for all entries
        KABC::AddressBook *addressbook = entry->addressbook();
        KABC::AddressBook::Iterator it;
        for ( it = addressbook->begin(); it != addressbook->end(); ++it ) {
            stream << "FirstName=\"" << (*it).givenName() << "\" ";
            stream << "MiddleName=\"" << (*it).additionalName() << "\" ";
            stream << "LastName=\"" << (*it).familyName() << "\" ";
            stream << "Suffix=\"" << (*it).suffix() << "\" ";
            stream << "FileAs=\"" << (*it).sortString() << "\" ";
            stream << "JobTitle=\"" << (*it).role() << "\" ";
            stream << "Department=\"" << (*it).custom( "opie", "Department" ) << "\" ";
            stream << "Company=\"" << (*it).organization() << "\" ";

            KABC::PhoneNumber businessPhoneNum = (*it).phoneNumber(KABC::PhoneNumber::Work );
            stream << "BusinessPhone=\"" <<businessPhoneNum.number() << "\" ";

            KABC::PhoneNumber businessFaxNum = (*it).phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
            stream << "BusinessFax=\"" << businessFaxNum.number() << "\" ";

            KABC::PhoneNumber businessMobile = (*it).phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
            stream << "BusinessMobile=\"" << businessMobile.number() << "\" ";
            stream << "DefaultEmail=\"" << (*it).preferredEmail() << "\" ";
            QStringList list = (*it).emails();
            if ( list.count() > 0 )
                stream << "Emails=\"" << list[0] << "\" ";

            KABC::PhoneNumber homePhoneNum = (*it).phoneNumber(KABC::PhoneNumber::Home );
            stream << "HomePhone=\"" << homePhoneNum.number() << "\" ";

            KABC::PhoneNumber homeFax = (*it).phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
            stream << "HomeFax=\"" << homeFax.number() << "\" ";

            KABC::PhoneNumber homeMobile = (*it).phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
            stream << "HomeMobile=\"" << homeMobile.number() << "\" ";

            KABC::Address business = (*it).address(KABC::Address::Work  );
            stream << "BusinessStreet=\"" << business.street() << "\" ";
            stream << "BusinessCity=\"" << business.locality() << "\" ";
            stream << "BusinessZip=\"" << business.postalCode() << "\" ";
            stream << "BusinessCountry=\"" << business.country() << "\" ";
            stream << "BusinessState=\"" << business.region() << "\" ";
            //stream << "BusinessPager=\"" << << "\" ";
            stream << "Office=\"" << (*it).custom( "opie",  "Office" ) << "\" ";
            stream << "Profession=\"" << (*it).custom( "opie",  "Profession" )<< "\" ";
            stream << "Assistant=\"" << (*it).custom( "opie",  "Assistant") << "\" ";
            stream << "Manager=\"" << (*it).custom( "opie",  "Manager" ) << "\" ";

            KABC::Address home( KABC::Address::Home );
            stream << "HomeStreet=\"" << home.street() << "\" ";
            stream << "HomeCity=\"" << home.locality() << "\" ";
            stream << "HomeState=\"" << home.region() << "\" ";
            stream << "HomeZip=\"" << home.postalCode() << "\" ";
            stream << "HomeCountry=\"" << home.country() << "\" ";

            stream << "HomeWebPage=\"" << (*it).custom( "opie", "HomeWebPage" ) << "\" ";
            stream << "Spouse=\"" << (*it).custom( "opie",  "Spouse") << "\" ";
            stream << "Gender=\"" << (*it).custom( "opie",  "Gender") << "\" ";
            stream << "Birthday=\"" << (*it).custom( "opie",  "Birthday" ) << "\" ";
            stream << "Anniversary=\"" << (*it).custom( "opie",  "Anniversary" ) << "\" ";
            stream << "Nickname=\"" << (*it).nickName() << "\" ";
            stream << "Children=\"" << (*it).custom("opie", "Children" ) << "\" ";
            stream << "Notes=\"" << (*it).note() << "\" ";
            stream << "Categories=\"" << categoriesToNumber( (*it).categories(),  "Contacts") << "\" ";
            stream << "Uid=\"" << konnectorId( "addressbook", (*it).uid() ) << "\" ";
        } // off for
        stream << "</Contacts>" << endl;
        stream << "</AddressBook>" << endl;
    }
    // now replace the UIDs for us
    m_helper->replaceIds( "addressbook",  m_kde2opie ); // to keep the use small
    return array;
}
