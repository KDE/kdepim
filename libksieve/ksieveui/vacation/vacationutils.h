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

#ifndef VACATIONUTILS_H
#define VACATIONUTILS_H
#include <QStringList>
#include <QString>
#include <QDate>
#include <kmime/kmime_header_parsing.h>

namespace KSieveUi {
namespace VacationUtils {
QString defaultMessageText();
QString defaultSubject();
int defaultNotificationInterval();
KMime::Types::AddrSpecList defaultMailAliases();
bool defaultSendForSpam();
QString defaultDomainName();
QDate defaultStartDate();
QDate defaultEndDate();

struct Vacation {
    Vacation():valid(false), active(false), notificationInterval(1), sendForSpam(true) {}
    bool isValid() const {return valid;}

    bool valid;
    QString messageText;
    QString subject;
    bool active;
    int notificationInterval;
    KMime::Types::AddrSpecList aliases;
    bool sendForSpam;
    QString excludeDomain;
    QDate startDate;
    QTime startTime;
    QDate endDate;
    QTime endTime;
};

QString composeScript(const Vacation &vacation);

KSieveUi::VacationUtils::Vacation parseScript(const QString &script);

QString mergeRequireLine(const QString &script1, const QString script2);

QString updateVacationBlock(const QString &oldScript, const QString &newScript);

}
}

#endif // VACATIONUTILS_H
