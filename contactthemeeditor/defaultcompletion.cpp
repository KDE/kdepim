/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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
        << QLatin1String("contact.profession");
    return lst;
}

QStringList DefaultCompletion::defaultOptions()
{
    QStringList lst;
    lst <<QLatin1String( "Safe" );
    return lst;
}
