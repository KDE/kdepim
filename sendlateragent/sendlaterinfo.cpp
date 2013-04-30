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

SendLaterInfo::SendLaterInfo()
    : mId(-1),
      mRecursiveEachValue(1),
      mRecursiveUnit(None),
      mRecursive(false)
{
}

SendLaterInfo::SendLaterInfo(const KConfigGroup& config)
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
    mTime = time;
}

QDateTime SendLaterInfo::dateTime() const
{
    return mTime;
}

void SendLaterInfo::readConfig(const KConfigGroup& config)
{
    mRecursive = config.readEntry("recursive", false);
    mRecursiveEachValue = config.readEntry("recursiveValue",1);
    mRecursiveUnit = static_cast<RecursiveUnit>(config.readEntry("recursiveUnit", (int)None));
    mId = config.readEntry("itemId", -1);
}

void SendLaterInfo::writeConfig(KConfigGroup & config )
{
    config.writeEntry("recursive", mRecursive);
    config.writeEntry("recursiveValue", mRecursiveEachValue );
    config.writeEntry("recursiveUnit", (int)mRecursiveUnit);
    config.writeEntry("itemId", mId);
    config.sync();
}
