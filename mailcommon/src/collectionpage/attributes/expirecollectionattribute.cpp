/*

  Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>

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

#include "expirecollectionattribute.h"
#include "folder/foldercollection.h"
#include "kernel/mailkernel.h"

#include <KConfigGroup>
#include <QDataStream>

using namespace MailCommon;

ExpireCollectionAttribute::ExpireCollectionAttribute()
    : mExpireMessages(false),
      mUnreadExpireAge(28),
      mReadExpireAge(14),
      mUnreadExpireUnits(ExpireNever),
      mReadExpireUnits(ExpireNever),
      mExpireAction(ExpireDelete),
      mExpireToFolderId(-1)
{
}

QByteArray ExpireCollectionAttribute::type() const
{
    static const QByteArray sType("expirationcollectionattribute");
    return sType;
}

ExpireCollectionAttribute *ExpireCollectionAttribute::clone() const
{
    ExpireCollectionAttribute *expireAttr = new ExpireCollectionAttribute();
    expireAttr->setAutoExpire(mExpireMessages);
    expireAttr->setUnreadExpireAge(mUnreadExpireAge);
    expireAttr->setUnreadExpireUnits(mUnreadExpireUnits);
    expireAttr->setReadExpireAge(mReadExpireAge);
    expireAttr->setReadExpireUnits(mReadExpireUnits);
    expireAttr->setExpireAction(mExpireAction);
    expireAttr->setExpireToFolderId(mExpireToFolderId);
    return expireAttr;
}

void ExpireCollectionAttribute::setAutoExpire(bool enabled)
{
    mExpireMessages = enabled;
}

bool ExpireCollectionAttribute::isAutoExpire() const
{
    return mExpireMessages;
}

void ExpireCollectionAttribute::setUnreadExpireAge(int age)
{
    if (age >= 0 && age != mUnreadExpireAge) {
        mUnreadExpireAge = age;
    }
}

int ExpireCollectionAttribute::unreadExpireAge() const
{
    return mUnreadExpireAge;
}

void ExpireCollectionAttribute::setUnreadExpireUnits(ExpireUnits units)
{
    if (units >= ExpireNever && units < ExpireMaxUnits) {
        mUnreadExpireUnits = units;
    }
}

void ExpireCollectionAttribute::setReadExpireAge(int age)
{
    if (age >= 0 && age != mReadExpireAge) {
        mReadExpireAge = age;
    }
}

int ExpireCollectionAttribute::readExpireAge() const
{
    return mReadExpireAge;
}

void ExpireCollectionAttribute::setReadExpireUnits(ExpireUnits units)
{
    if (units >= ExpireNever && units <= ExpireMaxUnits) {
        mReadExpireUnits = units;
    }
}

void ExpireCollectionAttribute::setExpireAction(ExpireAction a)
{
    mExpireAction = a;
}

ExpireCollectionAttribute::ExpireAction ExpireCollectionAttribute::expireAction() const
{
    return mExpireAction;
}

void ExpireCollectionAttribute::setExpireToFolderId(Akonadi::Collection::Id id)
{
    mExpireToFolderId = id;
}

Akonadi::Collection::Id ExpireCollectionAttribute::expireToFolderId() const
{
    return mExpireToFolderId;
}

ExpireCollectionAttribute::ExpireUnits ExpireCollectionAttribute::unreadExpireUnits() const
{
    return mUnreadExpireUnits;
}

ExpireCollectionAttribute::ExpireUnits ExpireCollectionAttribute::readExpireUnits() const
{
    return mReadExpireUnits;
}

bool ExpireCollectionAttribute::operator==(const ExpireCollectionAttribute &other) const
{
    return (mExpireMessages == other.isAutoExpire()) &&
           (mUnreadExpireAge == other.unreadExpireAge()) &&
           (mReadExpireAge == other.readExpireAge()) &&
           (mUnreadExpireUnits == other.unreadExpireUnits()) &&
           (mReadExpireUnits == other.readExpireUnits()) &&
           (mExpireAction == other.expireAction()) &&
           (mExpireToFolderId == other.expireToFolderId());
}

int ExpireCollectionAttribute::daysToExpire(int number,
        ExpireCollectionAttribute::ExpireUnits units)
{
    switch (units) {
    case ExpireCollectionAttribute::ExpireDays: // Days
        return number;
    case ExpireCollectionAttribute::ExpireWeeks: // Weeks
        return number * 7;
    case ExpireCollectionAttribute::ExpireMonths: // Months - this could be better
        // rather than assuming 31day months.
        return number * 31;
    default: // this avoids a compiler warning (not handled enumeration values)
        ;
    }
    return -1;
}

void ExpireCollectionAttribute::daysToExpire(int &unreadDays, int &readDays)
{
    unreadDays = ExpireCollectionAttribute::daysToExpire(unreadExpireAge(), unreadExpireUnits());
    readDays = ExpireCollectionAttribute::daysToExpire(readExpireAge(), readExpireUnits());
}

QByteArray ExpireCollectionAttribute::serialized() const
{
    QByteArray result;
    QDataStream s(&result, QIODevice::WriteOnly);

    s << mExpireToFolderId;
    s << (int)mExpireAction;
    s << (int)mReadExpireUnits;
    s << mReadExpireAge;
    s << (int)mUnreadExpireUnits;
    s << mUnreadExpireAge;
    s << mExpireMessages;

    return result;
}

void ExpireCollectionAttribute::deserialize(const QByteArray &data)
{
    QDataStream s(data);
    s >> mExpireToFolderId;
    int action;
    s >> action;
    mExpireAction = (ExpireCollectionAttribute::ExpireAction)action;
    int valUnitRead;
    s >> valUnitRead;
    mReadExpireUnits = (ExpireCollectionAttribute::ExpireUnits)valUnitRead;
    s >> mReadExpireAge;
    int valUnitUread;
    s >> valUnitUread;
    mUnreadExpireUnits = (ExpireCollectionAttribute::ExpireUnits)valUnitUread;
    s >> mUnreadExpireAge;
    s >> mExpireMessages;
}

