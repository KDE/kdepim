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

#ifndef MAILCOMMON_EXPIRECOLLECTIONATTRIBUTE_H
#define MAILCOMMON_EXPIRECOLLECTIONATTRIBUTE_H

#include "mailcommon_export.h"

#include <Attribute>
#include <Collection>

namespace MailCommon {

class MAILCOMMON_EXPORT ExpireCollectionAttribute : public Akonadi::Attribute
{
public:
    ExpireCollectionAttribute();

    /*
     * Define the possible units to use for measuring message expiry.
     * expireNever is used to switch off message expiry, and expireMaxUnits
     * must always be the last in the list (for bounds checking).
     */
    enum ExpireUnits {
        ExpireNever,
        ExpireDays,
        ExpireWeeks,
        ExpireMonths,
        ExpireMaxUnits
    };

    enum ExpireAction {
        ExpireDelete,
        ExpireMove
    };

    QByteArray type() const;
    Attribute *clone() const;
    QByteArray serialized() const;
    void deserialize( const QByteArray &data );

    static int daysToExpire( int number, ExpireCollectionAttribute::ExpireUnits units );

    static ExpireCollectionAttribute *expirationCollectionAttribute(
            const Akonadi::Collection &collection, bool &mustDeleteExpirationAttribute );

    void loadFromConfig( const Akonadi::Collection &collection );

    void daysToExpire( int &unreadDays, int &readDays );

    /**
     * Sets whether this folder automatically expires messages.
     */
    void setAutoExpire( bool enabled );

    /**
     * Returns true if this folder automatically expires old messages.
     */
    bool isAutoExpire() const;

    /**
     * Sets the maximum age for unread messages in this folder.
     * Age should not be negative. Units are set using
     * setUnreadExpireUnits().
     */
    void setUnreadExpireAge( int age );

    /**
     * Sets the units to use for expiry of unread messages.
     * Values are 1 = days, 2 = weeks, 3 = months.
     */
    void setUnreadExpireUnits( ExpireUnits units );

    /**
     * Sets the maximum age for read messages in this folder.
     * Age should not be negative. Units are set using
     * setReadExpireUnits().
     */
    void setReadExpireAge( int age );

    /**
     * Sets the units to use for expiry of read messages.
     * Values are 1 = days, 2 = weeks, 3 = months.
     */
    void setReadExpireUnits( ExpireUnits units );

    /**
     * Returns the age at which unread messages are expired.
     * Units are determined by unreadExpireUnits().
     */
    int unreadExpireAge() const;

    /**
     * Returns the age at which read messages are expired.
     * Units are determined by readExpireUnits().
     */
    int readExpireAge() const;

    /**
     * What should expiry do? Delete or move to another folder?
     */
    ExpireAction expireAction() const;
    void setExpireAction( ExpireAction a );

    /**
     * If expiry should move to folder, return the ID of that folder
     */
    Akonadi::Collection::Id expireToFolderId() const;
    void setExpireToFolderId( Akonadi::Collection::Id id );

    /**
     * Units getUnreadExpireAge() is returned in.
     * 1 = days, 2 = weeks, 3 = months.
     */
    ExpireUnits unreadExpireUnits() const;

    /**
     * Units getReadExpireAge() is returned in.
     * 1 = days, 2 = weeks, 3 = months.
     */
    ExpireUnits readExpireUnits() const;

private:
    bool mExpireMessages;         // true if old messages are expired
    int mUnreadExpireAge;         // Given in unreadExpireUnits
    int mReadExpireAge;           // Given in readExpireUnits
    ExpireCollectionAttribute::ExpireUnits  mUnreadExpireUnits;
    ExpireCollectionAttribute::ExpireUnits  mReadExpireUnits;
    ExpireCollectionAttribute::ExpireAction mExpireAction;
    Akonadi::Collection::Id mExpireToFolderId;
};

}
#endif /* EXPIRATIONCOLLECTIONATTRIBUTE_H */

