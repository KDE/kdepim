/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include "grantleecontactutils.h"

#include <KLocalizedString>

GrantleeContactUtils::GrantleeContactUtils()
{

}

void GrantleeContactUtils::insertVariableToQVariantHash(QVariantHash &contactI18n, const QString &variable)
{
    contactI18n.insert(variable, variableI18n(variable));
}

QString GrantleeContactUtils::variableI18n(const QString &variable)
{
    if (variable == QStringLiteral("birthdayi18n")) {
        return i18n("Birthday");
    } else if (variable == QStringLiteral("anniversaryi18n")) {
        return i18nc("The wedding anniversary of a contact", "Anniversary");
    } else if (variable == QStringLiteral("emailsi18n")) {
        return i18n("Emails");
    } else if (variable == QStringLiteral("websitei18n")) {
        return i18n("Website");
    } else if (variable == QStringLiteral("blogUrli18n")) {
        return i18n("Blog Feed");
    } else if (variable == QStringLiteral("addressBookNamei18n")) {
        return i18n("Address Book");
    } else if (variable == QStringLiteral("notei18n")) {
        return i18n("Note");
    } else if (variable == QStringLiteral("departmenti18n")) {
        return i18n("Department");
    } else if (variable == QStringLiteral("Professioni18n")) {
        return i18n("Profession");
    } else if (variable == QStringLiteral("officei18n")) {
        return i18n("Office");
    } else if (variable == QStringLiteral("manageri18n")) {
        return i18n("Manager's Name");
    } else if (variable == QStringLiteral("assistanti18n")) {
        return i18n("Assistant's Name");
    } else if (variable == QStringLiteral("spousei18n")) {
        return i18n("Partner's Name");
    } else if (variable == QStringLiteral("imAddressi18n")) {
        return i18n("IM Address");
    } else if (variable == QStringLiteral("latitudei18n")) {
        return i18n("Latitude");
    } else if (variable == QStringLiteral("longitudei18n")) {
        return i18n("Longitude");
    } else if (variable == QStringLiteral("organizationi18n")) {
        return i18n("Organization");
    } else if (variable == QStringLiteral("namei18n")) {
        return i18n("Name");
    } else if (variable == QStringLiteral("titlei18n")) {
        return i18n("Title");
    } else if (variable == QStringLiteral("nextcontacti18n")) {
        return i18n("Contact");
    } else {
        return variable;
    }
}
