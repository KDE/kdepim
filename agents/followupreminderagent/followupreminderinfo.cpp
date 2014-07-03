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


FollowUpReminderInfo::FollowUpReminderInfo()
    : mId(-1)
{
}

FollowUpReminderInfo::FollowUpReminderInfo(const KConfigGroup &config)
    : mId(-1)
{
    readConfig(config);
}

void FollowUpReminderInfo::readConfig(const KConfigGroup &config)
{
    if (config.hasKey(QLatin1String("followUpReminderDate"))) {
        mFollowUpReminderDate = QDateTime::fromString(config.readEntry("followUpReminderDate"), Qt::ISODate);
    }
    mId = config.readEntry("itemId", -1);
    mMessageId = config.readEntry("messageId", QString());
}

void FollowUpReminderInfo::writeConfig(KConfigGroup &config )
{
    if (mFollowUpReminderDate.isValid()) {
        config.writeEntry("followUpReminderDate", mFollowUpReminderDate.toString(Qt::ISODate) );
    }
    config.writeEntry("messageId", mMessageId);
    config.writeEntry("itemId", mId);
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
    return (mId != -1 && !mMessageId.isEmpty() && mFollowUpReminderDate.isValid() && !mTo.isEmpty());
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



