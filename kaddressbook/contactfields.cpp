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
#include <QUrl>

QString ContactFields::label(Field field)
{
    switch (field) {
    case Undefined:
        return i18nc("@item Undefined import field type", "Undefined");
        break;
    case FormattedName:
        return KContacts::Addressee::formattedNameLabel();
        break;
    case Prefix:
        return KContacts::Addressee::prefixLabel();
        break;
    case GivenName:
        return KContacts::Addressee::givenNameLabel();
        break;
    case AdditionalName:
        return KContacts::Addressee::additionalNameLabel();
        break;
    case FamilyName:
        return KContacts::Addressee::familyNameLabel();
        break;
    case Suffix:
        return KContacts::Addressee::suffixLabel();
        break;
    case NickName:
        return KContacts::Addressee::nickNameLabel();
        break;
    case Birthday:
        return KContacts::Addressee::birthdayLabel();
        break;
    case Anniversary:
        return i18nc("The wedding anniversary of a contact", "Anniversary");
        break;
    case HomeAddressStreet:
        return KContacts::Addressee::homeAddressStreetLabel();
        break;
    case HomeAddressPostOfficeBox:
        return KContacts::Addressee::homeAddressPostOfficeBoxLabel();
        break;
    case HomeAddressLocality:
        return KContacts::Addressee::homeAddressLocalityLabel();
        break;
    case HomeAddressRegion:
        return KContacts::Addressee::homeAddressRegionLabel();
        break;
    case HomeAddressPostalCode:
        return KContacts::Addressee::homeAddressPostalCodeLabel();
        break;
    case HomeAddressCountry:
        return KContacts::Addressee::homeAddressCountryLabel();
        break;
    case HomeAddressLabel:
        return KContacts::Addressee::homeAddressLabelLabel();
        break;
    case BusinessAddressStreet:
        return KContacts::Addressee::businessAddressStreetLabel();
        break;
    case BusinessAddressPostOfficeBox:
        return KContacts::Addressee::businessAddressPostOfficeBoxLabel();
        break;
    case BusinessAddressLocality:
        return KContacts::Addressee::businessAddressLocalityLabel();
        break;
    case BusinessAddressRegion:
        return KContacts::Addressee::businessAddressRegionLabel();
        break;
    case BusinessAddressPostalCode:
        return KContacts::Addressee::businessAddressPostalCodeLabel();
        break;
    case BusinessAddressCountry:
        return KContacts::Addressee::businessAddressCountryLabel();
        break;
    case BusinessAddressLabel:
        return KContacts::Addressee::businessAddressLabelLabel();
        break;
    case HomePhone:
        return KContacts::Addressee::homePhoneLabel();
        break;
    case BusinessPhone:
        return KContacts::Addressee::businessPhoneLabel();
        break;
    case MobilePhone:
        return KContacts::Addressee::mobilePhoneLabel();
        break;
    case HomeFax:
        return KContacts::Addressee::homeFaxLabel();
        break;
    case BusinessFax:
        return KContacts::Addressee::businessFaxLabel();
        break;
    case CarPhone:
        return KContacts::Addressee::carPhoneLabel();
        break;
    case Isdn:
        return KContacts::Addressee::isdnLabel();
        break;
    case Pager:
        return KContacts::Addressee::pagerLabel();
        break;
    case PreferredEmail:
        return i18nc("Preferred email address", "EMail (preferred)");
        break;
    case Email2:
        return i18nc("Second email address", "EMail (2)");
        break;
    case Email3:
        return i18nc("Third email address", "EMail (3)");
        break;
    case Email4:
        return i18nc("Fourth email address", "EMail (4)");
        break;
    case Mailer:
        return KContacts::Addressee::mailerLabel();
        break;
    case Title:
        return KContacts::Addressee::titleLabel();
        break;
    case Role:
        return KContacts::Addressee::roleLabel();
        break;
    case Organization:
        return KContacts::Addressee::organizationLabel();
        break;
    case Note:
        return KContacts::Addressee::noteLabel();
        break;
    case Homepage:
        return KContacts::Addressee::urlLabel();
        break;
    case BlogFeed:
        return i18n("Blog Feed");
        break;
    case Profession:
        return i18n("Profession");
        break;
    case Office:
        return i18n("Office");
        break;
    case Manager:
        return i18n("Manager");
        break;
    case Assistant:
        return i18n("Assistant");
        break;
    case Spouse:
        return i18n("Spouse");
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

void ContactFields::setValue(Field field, const QString &value, KContacts::Addressee &contact)
{
    switch (field) {
    case ContactFields::Undefined:
        break;
    case ContactFields::FormattedName:
        contact.setFormattedName(value);
        break;
    case ContactFields::GivenName:
        contact.setGivenName(value);
        break;
    case ContactFields::FamilyName:
        contact.setFamilyName(value);
        break;
    case ContactFields::AdditionalName:
        contact.setAdditionalName(value);
        break;
    case ContactFields::Prefix:
        contact.setPrefix(value);
        break;
    case ContactFields::Suffix:
        contact.setSuffix(value);
        break;
    case ContactFields::NickName:
        contact.setNickName(value);
        break;
    case ContactFields::Birthday:
        contact.setBirthday(QDateTime::fromString(value, Qt::ISODate));
        break;
    case ContactFields::Anniversary:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-Anniversary"), value);
        break;
    case ContactFields::PreferredEmail:
        contact.insertEmail(value, true);
        break;
    case ContactFields::Email2:
        contact.insertEmail(value, false);
        break;
    case ContactFields::Email3:
        contact.insertEmail(value, false);
        break;
    case ContactFields::Email4:
        contact.insertEmail(value, false);
        break;
    case ContactFields::Role:
        contact.setRole(value);
        break;
    case ContactFields::Title:
        contact.setTitle(value);
        break;
    case ContactFields::Mailer:
        contact.setMailer(value);
        break;
    case ContactFields::Homepage:
        contact.setUrl(QUrl(value));
        break;
    case ContactFields::Organization:
        contact.setOrganization(value);
        break;
    case ContactFields::Note:
        contact.setNote(value);
        break;
    case ContactFields::HomePhone: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Home);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::BusinessPhone: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Work);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::MobilePhone: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Cell);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::HomeFax: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Home |
                                        KContacts::PhoneNumber::Fax);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::BusinessFax: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Work |
                                        KContacts::PhoneNumber::Fax);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::CarPhone: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Car);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::Isdn: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Isdn);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;
    case ContactFields::Pager: {
        KContacts::PhoneNumber number = contact.phoneNumber(KContacts::PhoneNumber::Pager);
        number.setNumber(value);
        contact.insertPhoneNumber(number);
    }
    break;

    case ContactFields::HomeAddressStreet: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setStreet(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressPostOfficeBox: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setPostOfficeBox(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressLocality: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setLocality(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressRegion: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setRegion(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressPostalCode: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setPostalCode(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressCountry: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setCountry(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::HomeAddressLabel: {
        KContacts::Address address = contact.address(KContacts::Address::Home);
        address.setLabel(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressStreet: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setStreet(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressPostOfficeBox: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setPostOfficeBox(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressLocality: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setLocality(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressRegion: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setRegion(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressPostalCode: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setPostalCode(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressCountry: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setCountry(value);
        contact.insertAddress(address);
    }
    break;
    case ContactFields::BusinessAddressLabel: {
        KContacts::Address address = contact.address(KContacts::Address::Work);
        address.setLabel(value);
        contact.insertAddress(address);
    }
    break;
    case BlogFeed:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("BlogFeed"), value);
        break;
    case Profession:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-Profession"), value);
        break;
    case Office:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-Office"), value);
        break;
    case Manager:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-ManagersName"), value);
        break;
    case Assistant:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-AssistantsName"), value);
        break;
    case Spouse:
        contact.insertCustom(QLatin1String("KADDRESSBOOK"),
                             QLatin1String("X-SpousesName"), value);
        break;
    }
}

QString ContactFields::value(Field field, const KContacts::Addressee &contact)
{
    switch (field) {
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
    case Birthday: {
        const QDateTime birthday = contact.birthday();
        if (birthday.date().isValid()) {
            return birthday.date().toString(Qt::ISODate);
        } else {
            return QString();
        }
    }
    break;
    case Anniversary:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-Anniversary"));
        break;
    case HomeAddressStreet: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.street();
    }
    break;
    case HomeAddressPostOfficeBox: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.postOfficeBox();
    }
    break;
    case HomeAddressLocality: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.locality();
    }
    break;
    case HomeAddressRegion: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.region();
    }
    break;
    case HomeAddressPostalCode: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.postalCode();
    }
    break;
    case HomeAddressCountry: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.country();
    }
    break;
    case HomeAddressLabel: {
        const KContacts::Address address = contact.address(KContacts::Address::Home);
        return address.label();
    }
    break;
    case BusinessAddressStreet: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.street();
    }
    break;
    case BusinessAddressPostOfficeBox: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.postOfficeBox();
    }
    break;
    case BusinessAddressLocality: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.locality();
    }
    break;
    case BusinessAddressRegion: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.region();
    }
    break;
    case BusinessAddressPostalCode: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.postalCode();
    }
    break;
    case BusinessAddressCountry: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.country();
    }
    break;
    case BusinessAddressLabel: {
        const KContacts::Address address = contact.address(KContacts::Address::Work);
        return address.label();
    }
    break;
    case HomePhone:
        return contact.phoneNumber(KContacts::PhoneNumber::Home).number();
        break;
    case BusinessPhone:
        return contact.phoneNumber(KContacts::PhoneNumber::Work).number();
        break;
    case MobilePhone:
        return contact.phoneNumber(KContacts::PhoneNumber::Cell).number();
        break;
    case HomeFax:
        return contact.phoneNumber(KContacts::PhoneNumber::Home | KContacts::PhoneNumber::Fax).number();
        break;
    case BusinessFax:
        return contact.phoneNumber(KContacts::PhoneNumber::Work | KContacts::PhoneNumber::Fax).number();
        break;
    case CarPhone:
        return contact.phoneNumber(KContacts::PhoneNumber::Car).number();
        break;
    case Isdn:
        return contact.phoneNumber(KContacts::PhoneNumber::Isdn).number();
        break;
    case Pager:
        return contact.phoneNumber(KContacts::PhoneNumber::Pager).number();
        break;
    case PreferredEmail: {
        const QStringList emails = contact.emails();
        if (emails.count() > 0) {
            return emails.at(0);
        } else {
            return QString();
        }
    }
    break;
    case Email2: {
        const QStringList emails = contact.emails();
        if (emails.count() > 1) {
            return emails.at(1);
        } else {
            return QString();
        }
    }
    break;
    case Email3: {
        const QStringList emails = contact.emails();
        if (emails.count() > 2) {
            return emails.at(2);
        } else {
            return QString();
        }
    }
    break;
    case Email4: {
        const QStringList emails = contact.emails();
        if (emails.count() > 3) {
            return emails.at(3);
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
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("BlogFeed"));
        break;
    case Profession:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-Profession"));
        break;
    case Office:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-Office"));
        break;
    case Manager:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-ManagersName"));
        break;
    case Assistant:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-AssistantsName"));
        break;
    case Spouse:
        return contact.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("X-SpousesName"));
        break;
    }

    return QString();
}
