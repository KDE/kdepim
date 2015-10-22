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
#include <kmime/kmime_header_parsing.h>

class QDate;

namespace KSieveUi
{
namespace VacationUtils
{

enum MailAction {
    Keep,
    Discard,
    Sendto,
    CopyTo,
};

QString defaultMessageText();
QString defaultSubject();
MailAction defaultMailAction();
int defaultNotificationInterval();
KMime::Types::AddrSpecList defaultMailAliases();
bool defaultSendForSpam();
QString defaultDomainName();
QDate defaultStartDate();
QDate defaultEndDate();

struct Vacation {
    Vacation()
        : notificationInterval(1)
        , mailAction(Keep)
        , valid(false)
        , active(false)
        , sendForSpam(true)
    {
    }

    bool isValid() const
    {
        return valid;
    }

    QString mailActionRecipient;
    QString messageText;
    QString subject;
    KMime::Types::AddrSpecList aliases;
    QString excludeDomain;
    QDate startDate;
    QTime startTime;
    QDate endDate;
    QTime endTime;
    int notificationInterval;
    MailAction mailAction;
    bool valid;
    bool active;
    bool sendForSpam;
};

QString composeScript(const Vacation &vacation);

KSieveUi::VacationUtils::Vacation parseScript(const QString &script);

QString mergeRequireLine(const QString &script, const QString &scriptUpdate);

QString updateVacationBlock(const QString &oldScript, const QString &newScript);

QString mailAction(MailAction action);

}
}

#endif // VACATIONUTILS_H
