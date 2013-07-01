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


#include "sendlaterinfo.h"

#include <KConfigGroup>

using namespace SendLater;

SendLaterInfo::SendLaterInfo()
    : mId(-1),
      mRecursiveEachValue(1),
      mRecursiveUnit(None),
      mRecursive(false)
{
}

SendLaterInfo::SendLaterInfo(const KConfigGroup &config)
    : mId(-1),
      mRecursiveEachValue(1),
      mRecursiveUnit(None),
      mRecursive(false)
{
    readConfig(config);
}

SendLaterInfo::SendLaterInfo(const SendLaterInfo &info)
{
    mId = info.itemId();
    mRecursiveEachValue = info.recursiveEachValue();
    mRecursiveUnit = info.recursiveUnit();
    mRecursive = info.isRecursive();
    mSubject = info.subject();
}

SendLaterInfo::~SendLaterInfo()
{
}

bool SendLaterInfo::isRecursive() const
{
    return mRecursive;
}

void SendLaterInfo::setRecursive(bool b)
{
    mRecursive = b;
}

void SendLaterInfo::setRecursiveUnit(SendLaterInfo::RecursiveUnit unit)
{
    mRecursiveUnit = unit;
}

SendLaterInfo::RecursiveUnit SendLaterInfo::recursiveUnit() const
{
    return mRecursiveUnit;
}

void SendLaterInfo::setRecursiveEachValue(int value)
{
    mRecursiveEachValue = value;
}

int SendLaterInfo::recursiveEachValue() const
{
    return mRecursiveEachValue;
}

void SendLaterInfo::setItemId(Akonadi::Item::Id id)
{
    mId = id;
}

Akonadi::Item::Id SendLaterInfo::itemId() const
{
    return mId;
}

void SendLaterInfo::setDateTime(const QDateTime &time)
{
    mDateTime = time;
}

QDateTime SendLaterInfo::dateTime() const
{
    return mDateTime;
}

void SendLaterInfo::setLastDateTimeSend( const QDateTime &dateTime )
{
    mLastDateTimeSend = dateTime;
}

QDateTime SendLaterInfo::lastDateTimeSend() const
{
    return mLastDateTimeSend;
}

void SendLaterInfo::setSubject( const QString &subject )
{
    mSubject = subject;
}

QString SendLaterInfo::subject() const
{
    return mSubject;
}


void SendLaterInfo::readConfig(const KConfigGroup &config)
{
    if (config.hasKey(QLatin1String("lastDateTimeSend"))) {
        mLastDateTimeSend = QDateTime::fromString(config.readEntry("lastDateTimeSend"),Qt::ISODate);
    }
    mDateTime = config.readEntry("date", QDateTime());
    mRecursive = config.readEntry("recursive", false);
    mRecursiveEachValue = config.readEntry("recursiveValue",1);
    mRecursiveUnit = static_cast<RecursiveUnit>(config.readEntry("recursiveUnit", (int)None));
    mId = config.readEntry("itemId", -1);
    mSubject = config.readEntry("subject");
}

void SendLaterInfo::writeConfig(KConfigGroup &config )
{
    if (mLastDateTimeSend.isValid()) {
        config.writeEntry("lastDateTimeSend", mLastDateTimeSend.toString(Qt::ISODate) );
    }
    config.writeEntry("date", mDateTime);
    config.writeEntry("recursive", mRecursive);
    config.writeEntry("recursiveValue", mRecursiveEachValue );
    config.writeEntry("recursiveUnit", (int)mRecursiveUnit);
    config.writeEntry("itemId", mId);
    config.writeEntry("subject", mSubject);
    config.sync();
}
