/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2011, 2012, 2013 Montel Laurent <montel@kde.org>

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
#include "foldercollection.h"
#include "kernel/mailkernel.h"

#include <KConfigGroup>

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
    return "expirationcollectionattribute";
}

Akonadi::Attribute *ExpireCollectionAttribute::clone() const
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

void ExpireCollectionAttribute::loadFromConfig(const Akonadi::Collection &collection)
{
    KConfigGroup configGroup(KernelIf->config(),
                             MailCommon::FolderCollection::configGroupName(collection));

    if (configGroup.hasKey("ExpireMessages")) {
        mExpireMessages = configGroup.readEntry("ExpireMessages", false);

        mReadExpireAge = configGroup.readEntry("ReadExpireAge", 3);

        mReadExpireUnits = (ExpireUnits)configGroup.readEntry("ReadExpireUnits", (int)ExpireMonths);

        mUnreadExpireAge = configGroup.readEntry("UnreadExpireAge", 12);

        mUnreadExpireUnits =
            (ExpireUnits)configGroup.readEntry("UnreadExpireUnits", (int)ExpireNever);

        mExpireAction = configGroup.readEntry("ExpireAction", "Delete") == QLatin1String("Move") ?
                        ExpireMove :
                        ExpireDelete;

        mExpireToFolderId = configGroup.readEntry("ExpireToFolder", -1);
    }
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

ExpireCollectionAttribute *ExpireCollectionAttribute::expirationCollectionAttribute(
    const Akonadi::Collection &collection, bool &mustDeleteExpirationAttribute)
{
    MailCommon::ExpireCollectionAttribute *attr = 0;
    if (collection.hasAttribute<MailCommon::ExpireCollectionAttribute>()) {
        attr = collection.attribute<MailCommon::ExpireCollectionAttribute>();
        mustDeleteExpirationAttribute = false;
    } else {
        attr = new MailCommon::ExpireCollectionAttribute();
        attr->loadFromConfig(collection);
        mustDeleteExpirationAttribute = true;
    }
    return attr;
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

