/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "defaultcompletion.h"

QStringList DefaultCompletion::defaultCompetion()
{
    //TODO add to highlighter
    QStringList lst;

    lst << QStringLiteral("<div>")
        << QStringLiteral("contact.birthday")
        << QStringLiteral("contact.birthdayi18n")
        << QStringLiteral("contact.phoneNumbers")
        << QStringLiteral("phoneNumber.smsLink")
        << QStringLiteral("phoneNumber.type")
        << QStringLiteral("contact.emailsi18n")
        << QStringLiteral("contact.websitei18n")
        << QStringLiteral("contact.website")
        << QStringLiteral("contact.blogUrl")
        << QStringLiteral("contact.addresses")
        << QStringLiteral("contact.imAddressi18n")
        << QStringLiteral("contact.imAddress")
        << QStringLiteral("contact.Professioni18n")
        << QStringLiteral("contact.profession")
        << QStringLiteral("contact.hasqrcode")
        << QStringLiteral("contact.note")
        << QStringLiteral("contact.notei18n")
        << QStringLiteral("contact.addressBookName")
        << QStringLiteral("contact.addressBookNamei18n")
        << QStringLiteral("customField.title")
        << QStringLiteral("contact.customFields")
        << QStringLiteral("customField.value")
        << QStringLiteral("contact.customFieldsUrl")
        << QStringLiteral("contact.anniversaryi18n")
        << QStringLiteral("contact.anniversary")
        << QStringLiteral("contact.spousei18n")
        << QStringLiteral("contact.spouse")
        << QStringLiteral("contact.assistant")
        << QStringLiteral("contact.assistanti18n")
        << QStringLiteral("contact.manager")
        << QStringLiteral("contact.manageri18n")
        << QStringLiteral("contact.officei18n")
        << QStringLiteral("contact.office")
        << QStringLiteral("contact.department")
        << QStringLiteral("contact.departmenti18n")
        << QStringLiteral("contact.imAddresses")
        << QStringLiteral("phoneNumber.numberLink")
        << QStringLiteral("contact.imAddresses")
        << QStringLiteral("imAddress.type")
        << QStringLiteral("address.formattedAddressLink")
        << QStringLiteral("address.formattedAddressIcon")
        << QStringLiteral("member.emailLink")
        << QStringLiteral("member.name")
        << QStringLiteral("field.title")
        << QStringLiteral("field.value")
        << QStringLiteral("imAddress.imIcon");
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst << QStringLiteral("Safe");
    return lst;
}
