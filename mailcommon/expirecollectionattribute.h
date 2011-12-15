/* -*- mode: C++; c-file-style: "gnu" -*-
 This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef EXPIRATIONCOLLECTIONATTRIBUTE_H
#define EXPIRATIONCOLLECTIONATTRIBUTE_H

#include <akonadi/attribute.h>
#include <akonadi/collection.h>
#include "mailcommon_export.h"
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
  enum ExpireUnits { ExpireNever, ExpireDays, ExpireWeeks, ExpireMonths, ExpireMaxUnits };
  enum ExpireAction { ExpireDelete, ExpireMove };

  virtual QByteArray type() const;
  virtual Attribute *clone() const;
  virtual QByteArray serialized() const;
  virtual void deserialize( const QByteArray &data );

  static int daysToExpire( int number, ExpireCollectionAttribute::ExpireUnits units );

  static ExpireCollectionAttribute *expirationCollectionAttribute( const Akonadi::Collection& collection, bool &mustDeleteExpirationAttribute );

  void loadFromConfig( const Akonadi::Collection& collection );
  void daysToExpire(int& unreadDays, int& readDays);

  /**
   * Set whether this folder automatically expires messages.
   */
  void setAutoExpire(bool enabled);

  /**
   * Does this folder automatically expire old messages?
   */
  bool isAutoExpire() const;

  /**
   * Set the maximum age for unread messages in this folder.
   * Age should not be negative. Units are set using
   * setUnreadExpireUnits().
   */
  void setUnreadExpireAge(int age);

  /**
   * Set units to use for expiry of unread messages.
   * Values are 1 = days, 2 = weeks, 3 = months.
   */
  void setUnreadExpireUnits(ExpireUnits units);

  /**
   * Set the maximum age for read messages in this folder.
   * Age should not be negative. Units are set using
   * setReadExpireUnits().
   */
  void setReadExpireAge(int age);

  /**
   * Set units to use for expiry of read messages.
   * Values are 1 = days, 2 = weeks, 3 = months.
   */
  void setReadExpireUnits(ExpireUnits units);

  /**
   * Get the age at which unread messages are expired.
   * Units are determined by unreadExpireUnits().
   */
  int unreadExpireAge() const;

  /**
   * Get the age at which read messages are expired.
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
  bool         mExpireMessages;          // true if old messages are expired
  int          mUnreadExpireAge;         // Given in unreadExpireUnits
  int          mReadExpireAge;           // Given in readExpireUnits
  ExpireCollectionAttribute::ExpireUnits  mUnreadExpireUnits;
  ExpireCollectionAttribute::ExpireUnits  mReadExpireUnits;
  ExpireCollectionAttribute::ExpireAction mExpireAction;
  Akonadi::Collection::Id mExpireToFolderId;
};

}
#endif /* EXPIRATIONCOLLECTIONATTRIBUTE_H */

