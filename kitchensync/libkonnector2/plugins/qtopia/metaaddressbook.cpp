#include "metaaddressbook.h"

using namespace OpieHelper;


MetaAddressbook::MetaAddressbook()
    : MD5Template<KSync::AddressBookSyncee, KSync::AddressBookSyncEntry>()
{
}
MetaAddressbook::~MetaAddressbook() {

}
QString MetaAddressbook::string( KSync::AddressBookSyncEntry* entry) {
    KABC::Addressee adr = entry->addressee();

    QString str;
    str = adr.givenName();
    str += adr.additionalName();
    str += adr.familyName();
    str += adr.suffix();
    str += adr.role();
    str += adr.organization();


    /*
     * busines numbers
     */
    KABC::PhoneNumber number = adr.phoneNumber( KABC::PhoneNumber::Work );
    str += number.number();

    number = adr.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
    str += number.number();

    number = adr.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
    str += number.number();

    str += adr.preferredEmail();
    str += adr.emails().join(";");

    /*
     * home/private numbers
     */
    number = adr.phoneNumber( KABC::PhoneNumber::Home );
    str += number.number();

    number = adr.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
    str += number.number();

    number = adr.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell );
    str += number.number();

    /*
     * address bits
     */
    KABC::Address ad = adr.address( KABC::Address::Work );
    str += ad.street();
    str += ad.locality();
    str += ad.postalCode();
    str += ad.country();
    str += ad.region();


    /*
     * home address
     */
    ad = adr.address( KABC::Address::Home );
    str += ad.street();
    str += ad.locality();
    str += ad.region();
    str += ad.postalCode();
    str += ad.country();

    str += adr.custom( "KADDRESSBOOK", "X-Department" );
    str += adr.custom( "KADDRESSBOOK", "X-SpouseName" );
    str += adr.custom( "KADDRESSBOOK", "X-Office" );
    str += adr.custom( "KADDRESSBOOK", "X-Profession" );
    str += adr.custom( "KADDRESSBOOK", "AssistantsName" );
    str += adr.custom( "KADDRESSBOOK", "ManagersName" );
    str += adr.custom( "opie", "Children" );
    str += adr.custom( "opie", "HomeWebPage" );
    str += adr.custom( "opie", "Gender" );
//    str += adr.custom( "opie", "Birthday" );
    str += adr.birthday().date().toString(Qt::ISODate);
    str += adr.custom( "KADDRESSBOOK", "X-Anniversary" );

    str += adr.note();
    str += adr.nickName();
    str += adr.categories().join(";");


    return str;
}
