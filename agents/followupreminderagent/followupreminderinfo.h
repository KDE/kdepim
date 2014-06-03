/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#ifndef FOLLOWUPREMINDERINFO_H
#define FOLLOWUPREMINDERINFO_H

#include <Akonadi/Item>
#include <QDateTime>
class KConfigGroup;
class FollowUpReminderInfo
{
public:
    FollowUpReminderInfo();
    FollowUpReminderInfo(const KConfigGroup &config);

    Akonadi::Item::Id id() const;
    void setId(Akonadi::Item::Id value);

    bool isValid() const;

    QString messageId() const;
    void setMessageId(const QString &messageId);

    QDateTime followUpReminderDate() const;
    void setFollowUpReminderDate(const QDateTime &followUpReminderDate);

    void writeConfig(KConfigGroup &config);

private:
    void readConfig(const KConfigGroup &config);
    Akonadi::Item::Id mId;
    QString mMessageId;
    QDateTime mFollowUpReminderDate;
};

#endif // FOLLOWUPREMINDERINFO_H
