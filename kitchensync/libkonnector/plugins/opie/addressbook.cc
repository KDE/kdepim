

#include <qdom.h>
#include <qfile.h>

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
			adr.setUid(el.attribute("Uid" ) );
			adr.setFamilyName(el.attribute("LastName" ) );
			adr.setGivenName(el.attribute("FirstName" ) );
			adr.setAdditionalName(el.attribute("MiddleName" )  );
			adr.setSuffix(el.attribute("Suffix") );
			adr.setNickName(el.attribute("Nickname" ) );
			adr.setBirthday( QDate::fromString(el.attribute("Birthday")  ) );
			adr.setRole(el.attribute("JobTitle" ) );
			// inside into custom
			adr.setSortString( el.attribute("FileAs" ) );
			el.attribute("Department");
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
			adr.insertEmail( el.attribute("Emails") );

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

			adr.insertAddress( business );

			KABC::Address home( KABC::Address::Home );
			home.setStreet( el.attribute("HomeStreet") );
			home.setLocality( el.attribute("HomeCity") );
			home.setRegion( el.attribute("HomeState") );
			home.setPostalCode( el.attribute("HomeZip") );
			adr.insertAddress( home );
			//el.attribute("Birthday");

			adr.setNickName( el.attribute("Nickname") );
			adr.setNote( el.attribute("Notes") );

			QStringList categories = QStringList::split(";", el.attribute("Categories" ) );
			for(uint i=0; i < categories.count(); i++ ){
			  adr.insertCategory(m_edit->categoryById(categories[i], "Contacts"  ) );
			}
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

}
QByteArray AddressBook::fromKDE( KAddressbookSyncEntry *entry )
{
    QByteArray array;

    return array;
}
