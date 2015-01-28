/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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
        << QStringLiteral("contact.birthdayi18n" )
        << QStringLiteral("contact.anniversaryi18n")
        << QStringLiteral("contact.emailsi18n" )
        << QStringLiteral("contact.websitei18n" )
        << QStringLiteral("contact.blogUrli18n" )
        << QStringLiteral("contact.addressBookNamei18n" )
        << QStringLiteral("contact.notei18n" )
        << QStringLiteral("contact.departmenti18n" )
        << QStringLiteral("contact.Professioni18n" )
        << QStringLiteral("contact.officei18n" )
        << QStringLiteral("contact.manageri18n" )
        << QStringLiteral("contact.assistanti18n" )
        << QStringLiteral("contact.spousei18n" )
        << QStringLiteral("contact.imAddressi18n" )
        << QStringLiteral("contact.latitudei18n" )
        << QStringLiteral("contact.longitudei18n" )
        << QStringLiteral("contact.organizationi18n" )
        << QStringLiteral("contact.titlei18n" )
        << QStringLiteral("contact.nextcontacti18n" )
        << QStringLiteral("contact.name")
        << QStringLiteral("contact.realName")
        << QStringLiteral("contact.formattedName")
        << QStringLiteral("contact.prefix")
        << QStringLiteral("contact.givenName")
        << QStringLiteral("contact.additionalName")
        << QStringLiteral("contact.familyName")
        << QStringLiteral("contact.suffix")
        << QStringLiteral("contact.nickName")
        << QStringLiteral("contact.emails")
        << QStringLiteral("contact.organization")
        << QStringLiteral("contact.note")
        << QStringLiteral("contact.webSite")
        << QStringLiteral("contact.title")
        << QStringLiteral("contact.preferredEmail")
        << QStringLiteral("contact.role")
        << QStringLiteral("contact.birthday")
        << QStringLiteral("contact.department")
        << QStringLiteral("contact.addresses")
        << QStringLiteral("contact.phones")
        << QStringLiteral("contact.addressBookName")
        << QStringLiteral("contact.instantManging")
        << QStringLiteral("contact.geo")
        << QStringLiteral("contact.photo")
        << QStringLiteral("contact.logo")
        << QStringLiteral("contact.crypto")
        << QStringLiteral("contact.anniversary")
        << QStringLiteral("contact.profession")
        << QStringLiteral("contact.office")
        << QStringLiteral("contact.manager")
        << QStringLiteral("contact.assistant")
        << QStringLiteral("contact.spouse")
        << QStringLiteral("contact.languages");

    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst << QStringLiteral("Safe");
    return lst;
}
