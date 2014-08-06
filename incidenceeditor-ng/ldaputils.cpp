/*
 * Copyright 2014  Sandro Knauß <knauss@kolabsys.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ldaputils.h"
#include <klocalizedstring.h>


QString translateLDAPAttributeForDisplay(const QString &attribute)
{
    QString ret = attribute;
    if (attribute ==  QLatin1String("cn")) {
        ret = i18nc("ldap attribute cn", "Common name");
    } else if (attribute ==  QLatin1String("mail")) {
        ret = i18nc("ldap attribute mail", "Email");
    } else if (attribute ==  QLatin1String("givenname")) {
        ret = i18nc("ldap attribute givenname", "Given name");
    } else if (attribute ==  QLatin1String("sn")) {
        ret = i18nc("ldap attribute sn", "Surname");
    } else if (attribute ==  QLatin1String("ou")) {
        ret = i18nc("ldap attribute ou", "Organization");
    } else if (attribute ==  QLatin1String("objectClass")) {
        ret = i18nc("ldap attribute objectClass", "Object class");
    } else if (attribute ==  QLatin1String("description")) {
        ret = i18nc("ldap attribute description", "Description");
    } else if (attribute ==  QLatin1String("telephoneNumber")) {
        ret = i18nc("ldap attribute telephoneNumber",  "Telephone");
    } else if (attribute ==  QLatin1String("mobile")) {
        ret = i18nc("ldap attribute mobile",  "Mobile");
    }
    return ret;
}

QString translateKolabLDAPAttributeForDisplay(const QString &attribute)
{
    QString ret = attribute;
    if (attribute ==  QLatin1String("numseats")) {
        ret = i18nc("kolabldap", "Number of seats");
    } else if (attribute ==  QLatin1String("beamer_present")) {
        ret = i18nc("kolabldap", "Beamer");
    } else if (attribute ==  QLatin1String("conf_phone_present")) {
        ret = i18nc("kolabldap", "Conference phone");
    }
    return ret;
}
