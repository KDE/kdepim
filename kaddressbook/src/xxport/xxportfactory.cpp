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

#include "xxportfactory.h"

#include "csv/csv_xxport.h"
#include "ldif/ldif_xxport.h"
#include "ldap/ldap_xxport.h"
#include "vcard/vcard_xxport.h"
#include "gmx/gmx_xxport.h"

XXPort *XXPortFactory::createXXPort(const QString &identifier, QWidget *parentWidget) const
{
    if (identifier == QLatin1String("vcard21") || identifier == QLatin1String("vcard30") || identifier == QLatin1String("vcard40")) {
        XXPort *xxport = new VCardXXPort(parentWidget);
        if (identifier == QLatin1String("vcard21")) {
            xxport->setOption(QStringLiteral("version"), QStringLiteral("v21"));
        } else if (identifier == QLatin1String("vcard30")) {
            xxport->setOption(QStringLiteral("version"), QStringLiteral("v30"));
        } else if (identifier == QLatin1String("vcard40")) {
            xxport->setOption(QStringLiteral("version"), QStringLiteral("v40"));
        }
        return xxport;
    } else if (identifier == QLatin1String("csv")) {
        return new CsvXXPort(parentWidget);
    } else if (identifier == QLatin1String("ldif")) {
        return new LDIFXXPort(parentWidget);
    } else if (identifier == QLatin1String("ldap")) {
        return new LDAPXXPort(parentWidget);
    } else if (identifier == QLatin1String("gmx")) {
        return new GMXXXPort(parentWidget);
    } else {
        return Q_NULLPTR;
    }
}
