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

#include "vacationutils.h"
#include "sieve-vacation.h"
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>

#include <KLocale>
#include <KGlobal>
#include <QDate>

QString KSieveUi::VacationUtils::defaultMessageText() {
    return i18n( "I am out of office till %1.\n"
                 "\n"
                 "In urgent cases, please contact Mrs. \"vacation replacement\"\n"
                 "\n"
                 "email: \"email address of vacation replacement\"\n"
                 "phone: +49 711 1111 11\n"
                 "fax.:  +49 711 1111 12\n"
                 "\n"
                 "Yours sincerely,\n"
                 "-- \"enter your name and email address here\"\n",
                 KGlobal::locale()->formatDate( QDate::currentDate().addDays( 1 ) ) );
}

int KSieveUi::VacationUtils::defaultNotificationInterval() {
    return 7; // days
}

QStringList KSieveUi::VacationUtils::defaultMailAliases()
{
    QStringList sl;
    KPIMIdentities::IdentityManager manager( true );
    KPIMIdentities::IdentityManager::ConstIterator end(manager.end());
    for ( KPIMIdentities::IdentityManager::ConstIterator it = manager.begin(); it != end ; ++it ) {
        if ( !(*it).primaryEmailAddress().isEmpty() ) {
            sl.push_back( (*it).primaryEmailAddress() );
        }
        sl += (*it).emailAliases();
    }
    return sl;
}

bool KSieveUi::VacationUtils::defaultSendForSpam() {
    return VacationSettings::outOfOfficeReactToSpam();
}

QString KSieveUi::VacationUtils::defaultDomainName() {
    return VacationSettings::outOfOfficeDomain();
}
