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

#include "followupreminderinfo.h"

#include <KConfigGroup>
using namespace FollowUpReminder;

FollowUpReminderInfo::FollowUpReminderInfo()
    : mId(-1),
      mAnswerWasReceived(false)
{
}

FollowUpReminderInfo::FollowUpReminderInfo(const KConfigGroup &config)
    : mId(-1),
      mAnswerWasReceived(false)
{
    readConfig(config);
}

FollowUpReminderInfo::FollowUpReminderInfo(const FollowUpReminderInfo &info)
{
    mFollowUpReminderDate = info.followUpReminderDate();
    mId = info.id();
    mMessageId = info.messageId();
    mTo = info.to();
    mSubject = info.subject();
    mAnswerWasReceived = info.answerWasReceived();
}

void FollowUpReminderInfo::readConfig(const KConfigGroup &config)
{
    if (config.hasKey(QLatin1String("followUpReminderDate"))) {
        mFollowUpReminderDate = QDateTime::fromString(config.readEntry("followUpReminderDate"), Qt::ISODate);
    }
    mId = config.readEntry("itemId", -1);
    mMessageId = config.readEntry("messageId", QString());
    mTo = config.readEntry("to", QString());
    mSubject = config.readEntry("subject", QString());
    mAnswerWasReceived = config.readEntry("answerWasReceived", false);
}
bool FollowUpReminderInfo::answerWasReceived() const
{
    return mAnswerWasReceived;
}

void FollowUpReminderInfo::setAnswerWasReceived(bool answerWasReceived)
{
    mAnswerWasReceived = answerWasReceived;
}

QString FollowUpReminderInfo::subject() const
{
    return mSubject;
}

void FollowUpReminderInfo::setSubject(const QString &subject)
{
    mSubject = subject;
}

void FollowUpReminderInfo::writeConfig(KConfigGroup &config )
{
    if (mFollowUpReminderDate.isValid()) {
        config.writeEntry("followUpReminderDate", mFollowUpReminderDate.toString(Qt::ISODate) );
    }
    config.writeEntry("messageId", mMessageId);
    config.writeEntry("itemId", mId);
    config.writeEntry("to", mTo);
    config.writeEntry("subject", mSubject);
    config.writeEntry("answerWasReceived", mAnswerWasReceived);
    config.sync();
}

Akonadi::Item::Id FollowUpReminderInfo::id() const
{
    return mId;
}

void FollowUpReminderInfo::setId(Akonadi::Item::Id value)
{
    mId = value;
}

bool FollowUpReminderInfo::isValid() const
{
    return (mId != -1 &&
            !mMessageId.isEmpty() &&
            mFollowUpReminderDate.isValid() &&
            !mTo.isEmpty());
}

QString FollowUpReminderInfo::messageId() const
{
    return mMessageId;
}

void FollowUpReminderInfo::setMessageId(const QString &messageId)
{
    mMessageId = messageId;
}

void FollowUpReminderInfo::setTo(const QString &to)
{
    mTo = to;
}

QString FollowUpReminderInfo::to() const
{
    return mTo;
}

QDateTime FollowUpReminderInfo::followUpReminderDate() const
{
    return mFollowUpReminderDate;
}

void FollowUpReminderInfo::setFollowUpReminderDate(const QDateTime &followUpReminderDate)
{
    mFollowUpReminderDate = followUpReminderDate;
}

bool FollowUpReminderInfo::operator==( const FollowUpReminderInfo& other ) const
{
    return mId == other.id()
            && mMessageId == other.messageId()
            && mTo == other.to()
            && mFollowUpReminderDate == other.followUpReminderDate()
            && mSubject == other.subject()
            && mAnswerWasReceived == other.answerWasReceived();
}

