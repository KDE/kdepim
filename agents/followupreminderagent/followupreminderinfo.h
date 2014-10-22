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
#include <QDate>
#include "followupreminder_export.h"
class KConfigGroup;
namespace FollowUpReminder {
class FOLLOWUPREMINDER_EXPORT FollowUpReminderInfo
{
public:
    FollowUpReminderInfo();
    FollowUpReminderInfo(const KConfigGroup &config);
    FollowUpReminderInfo(const FollowUpReminderInfo &info);


    //Can be invalid.
    Akonadi::Item::Id originalMessageItemId() const;
    void setOriginalMessageItemId(Akonadi::Item::Id value);

    Akonadi::Item::Id todoId() const;
    void setTodoId(Akonadi::Item::Id value);

    bool isValid() const;

    QString messageId() const;
    void setMessageId(const QString &messageId);

    void setTo(const QString &to);
    QString to() const;

    QDate followUpReminderDate() const;
    void setFollowUpReminderDate(const QDate &followUpReminderDate);


    void writeConfig(KConfigGroup &config, qint32 identifier);

    QString subject() const;
    void setSubject(const QString &subject);

    bool operator ==(const FollowUpReminderInfo &other) const;


    bool answerWasReceived() const;
    void setAnswerWasReceived(bool answerWasReceived);

    Akonadi::Item::Id answerMessageItemId() const;
    void setAnswerMessageItemId(const Akonadi::Item::Id &answerMessageItemId);

    qint32 uniqueIdentifier() const;
    void setUniqueIdentifier(const qint32 &uniqueIdentifier);

private:
    void readConfig(const KConfigGroup &config);
    Akonadi::Item::Id mOriginalMessageItemId;
    Akonadi::Item::Id mAnswerMessageItemId;
    Akonadi::Item::Id mTodoId;
    QString mMessageId;
    QDate mFollowUpReminderDate;
    QString mTo;
    QString mSubject;
    qint32 mUniqueIdentifier;
    bool mAnswerWasReceived;
};
}
#endif // FOLLOWUPREMINDERINFO_H
