
#include "metaaddress.h"

using namespace OpieHelper;

namespace {
    bool test( const KABC::Addressee&,  const KABC::Addressee& );

};

MetaAddressReturn::MetaAddressReturn()
{

}
MetaAddressReturn::~MetaAddressReturn()
{

}
MetaAddressReturn::MetaAddressReturn( const MetaAddressReturn& ole )
{
    (*this) = ole;
}
QValueList<KABC::Addressee> MetaAddressReturn::added()
{
    return m_add;
}
QValueList<KABC::Addressee> MetaAddressReturn::modified()
{
    return m_mod;
}
QValueList<KABC::Addressee> MetaAddressReturn::removed()
{
    return m_rem;
}
MetaAddressReturn &MetaAddressReturn::operator=( const MetaAddressReturn& ole )
{
    m_add = ole.m_add;
    m_mod = ole.m_mod;
    m_rem = ole.m_rem;
    return *this;
}

MetaAddress::MetaAddress()
{

}
MetaAddress::~MetaAddress()
{

}
MetaAddressReturn MetaAddress::doMeta( const QValueList<KABC::Addressee>& newE,
                                       const QValueList<KABC::Addressee>& old )
{
    QValueList<KABC::Addressee> add;
    QValueList<KABC::Addressee> mod;
    QValueList<KABC::Addressee> rem;
    QValueList<KABC::Addressee>::ConstIterator it;
    QValueList<KABC::Addressee>::ConstIterator oldIt;
    bool found;
    for ( it = newE.begin(); it != newE.end(); ++it ) {
        found = false;
        for ( oldIt = old.begin(); oldIt != old.end(); ++oldIt ) {
            if ( (*oldIt).uid() == (*it).uid() ) {
                if ( test( (*it),  (*oldIt ) ) ) {
                    KABC::Addressee adr( (*it) ); // copy just in case
                    mod.append( adr );
                }
                found = true;
                break;
            }
        }
        if (!found ) {
            KABC::Addressee adr( (*it) );
            add.append( adr );
        }
    }
    for ( oldIt = old.begin(); oldIt != old.end(); ++oldIt ) {
        found = false;
        for ( it = newE.begin(); it != newE.end(); ++it ) {
            if ( (*it).uid() == (*oldIt).uid() ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            KABC::Addressee adr( (*oldIt) );
            rem.append( adr );
        }
    }
    MetaAddressReturn ret;
    ret.m_add = add;
    ret.m_mod = mod;
    ret.m_rem = rem;
    return ret;
};

namespace{

    bool test( const KABC::Addressee& ad1,  const KABC::Addressee& ad2 ) {
        bool ret = false;
        if ( ad1.givenName() != ad2.givenName() )
            return true;
        if ( ad1.additionalName() != ad2.additionalName() )
            return true;
        if ( ad1.familyName() != ad2.familyName() )
            return true;
        if ( ad1.suffix() != ad2.suffix() )
            return true;
        if ( ad1.sortString() != ad2.sortString() )
            return true;
        if ( ad1.role() != ad2.role() )
            return true;
        if ( ad1.custom( "opie",  "Department") != ad2.custom( "opie",  "Department") )
            return true;
        if ( ad1.organization() != ad2.organization() )
            return true;

        KABC::PhoneNumber phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work);
        KABC::PhoneNumber phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work);

        if ( phone1.number() != phone2.number() )
            return true;

        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        if ( phone1.number() != phone2.number() )
            return true;

        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
        if ( phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Work | KABC::PhoneNumber::Cell );
        if ( phone1.number() != phone2.number() )
            return true;
        if ( ad1.emails() != ad2.emails() )
            return true;
        phone1 = ad1.phoneNumber( KABC::PhoneNumber::Home );
        phone2 = ad2.phoneNumber( KABC::PhoneNumber::Home );
        if ( phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber(KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
        phone2 = ad2.phoneNumber(KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
        if ( phone1.number() != phone2.number() )
            return true;
        phone1 = ad1.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell);
        phone2 = ad2.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Cell);
        if ( phone1.number() != phone2.number() )
            return true;

        KABC::Address adr1 = ad1.address( KABC::Address::Work );
        KABC::Address adr2 = ad2.address( KABC::Address::Work );
        if ( adr1.street() != adr2.street() )
            return true;
        if ( adr1.locality() != adr2.locality() )
            return true;
        if ( adr1.postalCode() != adr2.postalCode() )
            return true;
        if (adr1.country() != adr2.country() )
            return true;
        if (adr1.region() != adr2.region() )
            return true;
        if (ad1.custom( "opie",  "Office") != ad2.custom("opie",  "Office") )
            return true;
        if (ad1.custom( "opie",  "Profession" ) != ad2.custom( "opie",  "Profession" ))
            return true;
        if (ad1.custom( "opie",  "Assistant") != ad2.custom( "opie",  "Assistant") )
            return true;

        adr1 = ad1.address( KABC::Address::Home );
        adr2 = ad1.address( KABC::Address::Home );
        if ( adr1.street() != adr2.street() )
            return true;
        if ( adr1.locality() != adr2.locality() )
            return true;
        if (adr1.region() != adr2.region() )
            return true;
        if (adr1.postalCode() != adr2.postalCode() )
            return true;
        if (adr1.country() != adr2.country() )
            return true;
        if (ad1.categories() != ad2.categories() )
            return true;

        if (ad1.custom( "opie", "HomeWebPage" ) != ad2.custom( "opie", "HomeWebPage" ) )
            return true;
        if (ad1.custom( "opie",  "Spouse") != ad2.custom( "opie",  "Spouse") )
            return true;
        if (ad1.custom( "opie",  "Gender") != ad2.custom( "opie",  "Gender") )
            return true;
        if (ad1.custom( "opie",  "Birthday" ) != ad2.custom( "opie",  "Birthday" ) )
            return true;
        if (ad1.custom( "opie",  "Anniversary" ) != ad2.custom( "opie",  "Anniversary" ) )
            return true;
        if (ad1.nickName() != ad2.nickName() )
            return true;


        return ret;
    }
};
