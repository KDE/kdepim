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

#ifndef VACATIONUTILS_H
#define VACATIONUTILS_H
#include <QStringList>
#include <QString>

class QDate;

namespace KMime
{
namespace Types
{
struct AddrSpec;
typedef QVector<AddrSpec> AddrSpecList;
}
}

namespace KSieveUi
{
namespace VacationUtils
{
QString defaultMessageText();
QString defaultSubject();
int defaultNotificationInterval();
QStringList defaultMailAliases();
bool defaultSendForSpam();
QString defaultDomainName();
QDate defaultStartDate();
QDate defaultEndDate();

QString composeScript(const QString &messageText,
                      const QString &subject,
                      int notificationInterval,
                      const KMime::Types::AddrSpecList &aliases,
                      bool sendForSpam, const QString &excludeDomain,
                      const QDate &startDate, const QDate &endDate);
bool parseScript(const QString &script, QString &messageText,
                 QString &subject,
                 int &notificationInterval, QStringList &aliases,
                 bool &sendForSpam, QString &domainName,
                 QDate &startDate, QDate &endDate);

}
}

#endif // VACATIONUTILS_H
