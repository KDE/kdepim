/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "contactfields.h"

#include <KLocalizedString>

QString ContactFields::label( Field field )
{
    switch ( field ) {
    case Undefined:
        return i18nc( "@item Undefined import field type", "Undefined" );
        break;
    case FormattedName:
        return KABC::Addressee::formattedNameLabel();
        break;
    case Prefix:
        return KABC::Addressee::prefixLabel();
        break;
    case GivenName:
        return KABC::Addressee::givenNameLabel();
        break;
    case AdditionalName:
        return KABC::Addressee::additionalNameLabel();
        break;
    case FamilyName:
        return KABC::Addressee::familyNameLabel();
        break;
    case Suffix:
        return KABC::Addressee::suffixLabel();
        break;
    case NickName:
        return KABC::Addressee::nickNameLabel();
        break;
    case Birthday:
        return KABC::Addressee::birthdayLabel();
        break;
    case Anniversary:
        return i18nc( "The wedding anniversary of a contact", "Anniversary" );
        break;
    case HomeAddressStreet:
        return KABC::Addressee::homeAddressStreetLabel();
        break;
    case HomeAddressPostOfficeBox:
        return KABC::Addressee::homeAddressPostOfficeBoxLabel();
        break;
    case HomeAddressLocality:
        return KABC::Addressee::homeAddressLocalityLabel();
        break;
    case HomeAddressRegion:
        return KABC::Addressee::homeAddressRegionLabel();
        break;
    case HomeAddressPostalCode:
        return KABC::Addressee::homeAddressPostalCodeLabel();
        break;
    case HomeAddressCountry:
        return KABC::Addressee::homeAddressCountryLabel();
        break;
    case HomeAddressLabel:
        return KABC::Addressee::homeAddressLabelLabel();
        break;
    case BusinessAddressStreet:
        return KABC::Addressee::businessAddressStreetLabel();
        break;
    case BusinessAddressPostOfficeBox:
        return KABC::Addressee::businessAddressPostOfficeBoxLabel();
        break;
    case BusinessAddressLocality:
        return KABC::Addressee::businessAddressLocalityLabel();
        break;
    case BusinessAddressRegion:
        return KABC::Addressee::businessAddressRegionLabel();
        break;
    case BusinessAddressPostalCode:
        return KABC::Addressee::businessAddressPostalCodeLabel();
        break;
    case BusinessAddressCountry:
        return KABC::Addressee::businessAddressCountryLabel();
        break;
    case BusinessAddressLabel:
        return KABC::Addressee::businessAddressLabelLabel();
        break;
    case HomePhone:
        return KABC::Addressee::homePhoneLabel();
        break;
    case BusinessPhone:
        return KABC::Addressee::businessPhoneLabel();
        break;
    case MobilePhone:
        return KABC::Addressee::mobilePhoneLabel();
        break;
    case HomeFax:
        return KABC::Addressee::homeFaxLabel();
        break;
    case BusinessFax:
        return KABC::Addressee::businessFaxLabel();
        break;
    case CarPhone:
        return KABC::Addressee::carPhoneLabel();
        break;
    case Isdn:
        return KABC::Addressee::isdnLabel();
        break;
    case Pager:
        return KABC::Addressee::pagerLabel();
        break;
    case PreferredEmail:
        return i18nc( "Preferred email address", "EMail (preferred)" );
        break;
    case Email2:
        return i18nc( "Second email address", "EMail (2)" );
        break;
    case Email3:
        return i18nc( "Third email address", "EMail (3)" );
        break;
    case Email4:
        return i18nc( "Fourth email address", "EMail (4)" );
        break;
    case Mailer:
        return KABC::Addressee::mailerLabel();
        break;
    case Title:
        return KABC::Addressee::titleLabel();
        break;
    case Role:
        return KABC::Addressee::roleLabel();
        break;
    case Organization:
        return KABC::Addressee::organizationLabel();
        break;
    case Note:
        return KABC::Addressee::noteLabel();
        break;
    case Homepage:
        return KABC::Addressee::urlLabel();
        break;
    case BlogFeed:
        return i18n( "Blog Feed" );
        break;
    case Profession:
        return i18n( "Profession" );
        break;
    case Office:
        return i18n( "Office" );
        break;
    case Manager:
        return i18n( "Manager" );
        break;
    case Assistant:
        return i18n( "Assistant" );
        break;
    case Spouse:
        return i18n( "Spouse" );
        break;
    }

    return QString();
}

ContactFields::Fields ContactFields::allFields()
{
    Fields fields;

    fields << Undefined
           << FormattedName
           << Prefix
           << GivenName
           << AdditionalName
           << FamilyName
           << Suffix
           << NickName
           << Birthday
           << Anniversary
           << PreferredEmail
           << Email2
           << Email3
           << Email4
           << HomeAddressStreet
           << HomeAddressPostOfficeBox
           << HomeAddressLocality
           << HomeAddressRegion
           << HomeAddressPostalCode
           << HomeAddressCountry
           << HomeAddressLabel
           << BusinessAddressStreet
           << BusinessAddressPostOfficeBox
           << BusinessAddressLocality
           << BusinessAddressRegion
           << BusinessAddressPostalCode
           << BusinessAddressCountry
           << BusinessAddressLabel
           << HomePhone
           << BusinessPhone
           << MobilePhone
           << HomeFax
           << BusinessFax
           << CarPhone
           << Isdn
           << Pager
           << Mailer
           << Title
           << Role
           << Organization
           << Note
           << Homepage
           << BlogFeed
           << Profession
           << Office
           << Manager
           << Assistant
           << Spouse;

    return fields;
}

void ContactFields::setValue( Field field, const QString &value, KABC::Addressee &contact )
{
    switch ( field ) {
    case ContactFields::Undefined:
        break;
    case ContactFields::FormattedName:
        contact.setFormattedName( value );
        break;
    case ContactFields::GivenName:
        contact.setGivenName( value );
        break;
    case ContactFields::FamilyName:
        contact.setFamilyName( value );
        break;
    case ContactFields::AdditionalName:
        contact.setAdditionalName( value );
        break;
    case ContactFields::Prefix:
        contact.setPrefix( value );
        break;
    case ContactFields::Suffix:
        contact.setSuffix( value );
        break;
    case ContactFields::NickName:
        contact.setNickName( value );
        break;
    case ContactFields::Birthday:
        contact.setBirthday( QDateTime::fromString( value, Qt::ISODate ) );
        break;
    case ContactFields::Anniversary:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-Anniversary" ), value );
        break;
    case ContactFields::PreferredEmail:
        contact.insertEmail( value, true );
        break;
    case ContactFields::Email2:
        contact.insertEmail( value, false );
        break;
    case ContactFields::Email3:
        contact.insertEmail( value, false );
        break;
    case ContactFields::Email4:
        contact.insertEmail( value, false );
        break;
    case ContactFields::Role:
        contact.setRole( value );
        break;
    case ContactFields::Title:
        contact.setTitle( value );
        break;
    case ContactFields::Mailer:
        contact.setMailer( value );
        break;
    case ContactFields::Homepage:
        contact.setUrl( KUrl( value ) );
        break;
    case ContactFields::Organization:
        contact.setOrganization( value );
        break;
    case ContactFields::Note:
        contact.setNote( value );
        break;
    case ContactFields::HomePhone:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Home );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::BusinessPhone:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Work );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::MobilePhone:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Cell );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::HomeFax:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Home |
                                                        KABC::PhoneNumber::Fax );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::BusinessFax:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Work |
                                                        KABC::PhoneNumber::Fax );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::CarPhone:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Car );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::Isdn:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Isdn );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;
    case ContactFields::Pager:
    {
        KABC::PhoneNumber number = contact.phoneNumber( KABC::PhoneNumber::Pager );
        number.setNumber( value );
        contact.insertPhoneNumber( number );
    }
        break;

    case ContactFields::HomeAddressStreet:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setStreet( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressPostOfficeBox:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setPostOfficeBox( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressLocality:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setLocality( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressRegion:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setRegion( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressPostalCode:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setPostalCode( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressCountry:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setCountry( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::HomeAddressLabel:
    {
        KABC::Address address = contact.address( KABC::Address::Home );
        address.setLabel( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressStreet:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setStreet( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressPostOfficeBox:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setPostOfficeBox( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressLocality:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setLocality( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressRegion:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setRegion( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressPostalCode:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setPostalCode( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressCountry:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setCountry( value );
        contact.insertAddress( address );
    }
        break;
    case ContactFields::BusinessAddressLabel:
    {
        KABC::Address address = contact.address( KABC::Address::Work );
        address.setLabel( value );
        contact.insertAddress( address );
    }
        break;
    case BlogFeed:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "BlogFeed" ), value );
        break;
    case Profession:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-Profession" ), value );
        break;
    case Office:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-Office" ), value );
        break;
    case Manager:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-ManagersName" ), value );
        break;
    case Assistant:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-AssistantsName" ), value );
        break;
    case Spouse:
        contact.insertCustom( QLatin1String( "KADDRESSBOOK" ),
                              QLatin1String( "X-SpousesName" ), value );
        break;
    }
}

QString ContactFields::value( Field field, const KABC::Addressee &contact )
{
    switch ( field ) {
    case Undefined:
        return QString();
        break;
    case FormattedName:
        return contact.formattedName();
        break;
    case Prefix:
        return contact.prefix();
        break;
    case GivenName:
        return contact.givenName();
        break;
    case AdditionalName:
        return contact.additionalName();
        break;
    case FamilyName:
        return contact.familyName();
        break;
    case Suffix:
        return contact.suffix();
        break;
    case NickName:
        return contact.nickName();
        break;
    case Birthday:
    {
        const QDateTime birthday = contact.birthday();
        if ( birthday.date().isValid() ) {
            return birthday.date().toString( Qt::ISODate );
        } else {
            return QString();
        }
    }
        break;
    case Anniversary:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Anniversary" ) );
        break;
    case HomeAddressStreet:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.street();
    }
        break;
    case HomeAddressPostOfficeBox:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.postOfficeBox();
    }
        break;
    case HomeAddressLocality:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.locality();
    }
        break;
    case HomeAddressRegion:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.region();
    }
        break;
    case HomeAddressPostalCode:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.postalCode();
    }
        break;
    case HomeAddressCountry:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.country();
    }
        break;
    case HomeAddressLabel:
    {
        const KABC::Address address = contact.address( KABC::Address::Home );
        return address.label();
    }
        break;
    case BusinessAddressStreet:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.street();
    }
        break;
    case BusinessAddressPostOfficeBox:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.postOfficeBox();
    }
        break;
    case BusinessAddressLocality:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.locality();
    }
        break;
    case BusinessAddressRegion:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.region();
    }
        break;
    case BusinessAddressPostalCode:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.postalCode();
    }
        break;
    case BusinessAddressCountry:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.country();
    }
        break;
    case BusinessAddressLabel:
    {
        const KABC::Address address = contact.address( KABC::Address::Work );
        return address.label();
    }
        break;
    case HomePhone:
        return contact.phoneNumber( KABC::PhoneNumber::Home ).number();
        break;
    case BusinessPhone:
        return contact.phoneNumber( KABC::PhoneNumber::Work ).number();
        break;
    case MobilePhone:
        return contact.phoneNumber( KABC::PhoneNumber::Cell ).number();
        break;
    case HomeFax:
        return contact.phoneNumber( KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax ).number();
        break;
    case BusinessFax:
        return contact.phoneNumber( KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax ).number();
        break;
    case CarPhone:
        return contact.phoneNumber( KABC::PhoneNumber::Car ).number();
        break;
    case Isdn:
        return contact.phoneNumber( KABC::PhoneNumber::Isdn ).number();
        break;
    case Pager:
        return contact.phoneNumber( KABC::PhoneNumber::Pager ).number();
        break;
    case PreferredEmail:
    {
        const QStringList emails = contact.emails();
        if ( emails.count() > 0 ) {
            return emails.at( 0 );
        } else {
            return QString();
        }
    }
        break;
    case Email2:
    {
        const QStringList emails = contact.emails();
        if ( emails.count() > 1 ) {
            return emails.at( 1 );
        } else {
            return QString();
        }
    }
        break;
    case Email3:
    {
        const QStringList emails = contact.emails();
        if ( emails.count() > 2 ) {
            return emails.at( 2 );
        } else {
            return QString();
        }
    }
        break;
    case Email4:
    {
        const QStringList emails = contact.emails();
        if ( emails.count() > 3 ) {
            return emails.at( 3 );
        } else {
            return QString();
        }
    }
        break;
    case Mailer:
        return contact.mailer();
        break;
    case Title:
        return contact.title();
        break;
    case Role:
        return contact.role();
        break;
    case Organization:
        return contact.organization();
        break;
    case Note:
        return contact.note();
        break;
    case Homepage:
        return contact.url().url();
        break;
    case BlogFeed:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "BlogFeed" ) );
        break;
    case Profession:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Profession" ) );
        break;
    case Office:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-Office" ) );
        break;
    case Manager:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-ManagersName" ) );
        break;
    case Assistant:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-AssistantsName" ) );
        break;
    case Spouse:
        return contact.custom( QLatin1String( "KADDRESSBOOK" ), QLatin1String( "X-SpousesName" ) );
        break;
    }

    return QString();
}
