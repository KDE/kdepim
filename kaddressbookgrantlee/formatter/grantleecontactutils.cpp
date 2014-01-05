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


#include "grantleecontactutils.h"

#include <KLocale>

QString GrantleeContactUtils::variableI18n(const QString &variable)
{
    if (variable == QLatin1String("birthdayi18n")) {
        return i18n( "Birthday" );
    } else if (variable == QLatin1String("anniversaryi18n")) {
        return i18nc( "The wedding anniversary of a contact", "Anniversary" );
    } else if (variable == QLatin1String("emailsi18n")) {
        return i18n( "Emails" );
    } else if (variable == QLatin1String("websitei18n")) {
        return i18n("Website");
    } else if (variable == QLatin1String("blogUrli18n")) {
        return i18n( "Blog Feed" );
    } else if (variable == QLatin1String("addressBookNamei18n")) {
        return i18n( "Address Book" );
    } else if (variable == QLatin1String("notei18n")) {
        return i18n( "Note" );
    } else if (variable == QLatin1String("departmenti18n")) {
        return i18n( "Department" );
    } else if (variable == QLatin1String("Professioni18n")) {
        return i18n( "Profession" );
    } else if (variable == QLatin1String("officei18n")) {
        return i18n( "Office" );
    } else if (variable == QLatin1String("manageri18n")) {
        return i18n( "Manager's Name" );
    } else if (variable == QLatin1String("assistanti18n")) {
        return i18n( "Assistant's Name" );
    } else if (variable == QLatin1String("spousei18n")) {
        return i18n( "Partner's Name" );
    } else if (variable == QLatin1String("imAddressi18n")) {
        return i18n( "IM Address" );
    } else {
        return variable;
    }
}
