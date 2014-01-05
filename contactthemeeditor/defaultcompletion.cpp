/*
  Copyright (c) 2013, 2014 Montel Laurent <montel.org>

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

    lst << QLatin1String( "<div>" )
        << QLatin1String( "contact.birthday")
        << QLatin1String("contact.birthdayi18n")
        << QLatin1String("contact.phoneNumbers")
        << QLatin1String("phoneNumber.smsLink")
        << QLatin1String("phoneNumber.type")
        << QLatin1String("contact.emailsi18n")
        << QLatin1String("contact.websitei18n")
        << QLatin1String("contact.website")
        << QLatin1String("contact.blogUrl")
        << QLatin1String("contact.addresses")
        << QLatin1String("contact.imAddressi18n")
        << QLatin1String("contact.imAddress")
        << QLatin1String("contact.Professioni18n")
        << QLatin1String("contact.profession")
        << QLatin1String("contact.hasqrcode")
        << QLatin1String("contact.note")
        << QLatin1String("contact.notei18n")
        << QLatin1String("contact.addressBookName")
        << QLatin1String("contact.addressBookNamei18n")
        << QLatin1String("customField.title")
        << QLatin1String("contact.customFields")
        << QLatin1String("customField.value")
        << QLatin1String("contact.customFieldsUrl")
        << QLatin1String("contact.anniversaryi18n")
        << QLatin1String("contact.anniversary")
        << QLatin1String("contact.spousei18n")
        << QLatin1String("contact.spouse")
        << QLatin1String("contact.assistant")
        << QLatin1String("contact.assistanti18n")
        << QLatin1String("contact.manager")
        << QLatin1String("contact.manageri18n")
        << QLatin1String("contact.officei18n")
        << QLatin1String("contact.office")
        << QLatin1String("contact.department")
        << QLatin1String("contact.departmenti18n")
        << QLatin1String("contact.imAddresses")
        << QLatin1String("phoneNumber.numberLink")
        << QLatin1String("contact.imAddresses")
        << QLatin1String("imAddress.type")
        << QLatin1String("address.formattedAddressLink")
        << QLatin1String("address.formattedAddressIcon")
        << QLatin1String("member.emailLink")
        << QLatin1String("member.name")
        << QLatin1String("field.title")
        << QLatin1String("field.value")
        << QLatin1String("imAddress.imIcon");
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst <<QLatin1String( "Safe" );
    return lst;
}
