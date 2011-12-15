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
#include "expirationcollectionattribute.h"
#include "foldercollection.h"
#include "mailkernel.h"
#include <KConfigGroup>

using namespace MailCommon;

ExpirationCollectionAttribute::ExpirationCollectionAttribute()
  : mExpireMessages( false ),
    mUnreadExpireAge( 28 ),
    mReadExpireAge( 14 ),
    mUnreadExpireUnits( ExpireNever ),
    mReadExpireUnits( ExpireNever ),
    mExpireAction( ExpireDelete ),
    mExpireToFolderId( -1 )
{
}

QByteArray ExpirationCollectionAttribute::type() const
{
  return "expirationcollectionattribute";
}

Akonadi::Attribute *ExpirationCollectionAttribute::clone() const
{
  ExpirationCollectionAttribute *expireAttr = new ExpirationCollectionAttribute();
  expireAttr->setAutoExpire(mExpireMessages);
  expireAttr->setUnreadExpireAge( mUnreadExpireAge );
  expireAttr->setUnreadExpireUnits( mUnreadExpireUnits );
  expireAttr->setReadExpireAge( mReadExpireAge );
  expireAttr->setReadExpireUnits( mReadExpireUnits );
  expireAttr->setExpireAction( mExpireAction );
  expireAttr->setExpireToFolderId(mExpireToFolderId);
  return expireAttr;
}

void ExpirationCollectionAttribute::loadFromConfig( const Akonadi::Collection& collection )
{
  KConfigGroup configGroup( KernelIf->config(), MailCommon::FolderCollection::configGroupName(collection) );
  if ( configGroup.hasKey( "ExpireMessages" ) ) {
    mExpireMessages = configGroup.readEntry( "ExpireMessages", false );
    mReadExpireAge = configGroup.readEntry( "ReadExpireAge", 3 );
    mReadExpireUnits = (ExpireUnits)configGroup.readEntry( "ReadExpireUnits", (int)ExpireMonths );
    mUnreadExpireAge = configGroup.readEntry( "UnreadExpireAge", 12 );
    mUnreadExpireUnits = (ExpireUnits)
      configGroup.readEntry( "UnreadExpireUnits", (int)ExpireNever );
    mExpireAction = configGroup.readEntry( "ExpireAction", "Delete") == QLatin1String( "Move" ) ? ExpireMove : ExpireDelete;
    mExpireToFolderId = configGroup.readEntry( "ExpireToFolder", -1 );
  }
}


void ExpirationCollectionAttribute::setAutoExpire( bool enabled )
{
  mExpireMessages = enabled;
}
bool ExpirationCollectionAttribute::isAutoExpire() const
{
  return mExpireMessages;
}


void ExpirationCollectionAttribute::setUnreadExpireAge( int age )
{
  if( age >= 0 && age != mUnreadExpireAge ) {
    mUnreadExpireAge = age;
  }
}

int ExpirationCollectionAttribute::unreadExpireAge() const
{
  return mUnreadExpireAge;
}


void ExpirationCollectionAttribute::setUnreadExpireUnits( ExpireUnits units )
{
  if (units >= ExpireNever && units < ExpireMaxUnits) {
    mUnreadExpireUnits = units;
  }
}

void ExpirationCollectionAttribute::setReadExpireAge( int age )
{
  if( age >= 0 && age != mReadExpireAge ) {
    mReadExpireAge = age;
  }
}

int ExpirationCollectionAttribute::readExpireAge() const
{
  return mReadExpireAge;
}


void ExpirationCollectionAttribute::setReadExpireUnits( ExpireUnits units )
{
  if (units >= ExpireNever && units <= ExpireMaxUnits) {
    mReadExpireUnits = units;
  }
}

void ExpirationCollectionAttribute::setExpireAction( ExpireAction a )
{
  mExpireAction = a;
}

ExpirationCollectionAttribute::ExpireAction ExpirationCollectionAttribute::expireAction() const
{
  return mExpireAction;
}


void ExpirationCollectionAttribute::setExpireToFolderId( Akonadi::Collection::Id id )
{
  mExpireToFolderId = id;
}

Akonadi::Collection::Id ExpirationCollectionAttribute::expireToFolderId() const
{
  return mExpireToFolderId;
}


ExpirationCollectionAttribute::ExpireUnits ExpirationCollectionAttribute::unreadExpireUnits() const
{
  return mUnreadExpireUnits;
}

ExpirationCollectionAttribute::ExpireUnits ExpirationCollectionAttribute::readExpireUnits() const
{
  return mReadExpireUnits;
}

int ExpirationCollectionAttribute::daysToExpire( int number, ExpirationCollectionAttribute::ExpireUnits units )
{
  switch (units) {
  case ExpirationCollectionAttribute::ExpireDays: // Days
    return number;
  case ExpirationCollectionAttribute::ExpireWeeks: // Weeks
    return number * 7;
  case ExpirationCollectionAttribute::ExpireMonths: // Months - this could be better rather than assuming 31day months.
    return number * 31;
  default: // this avoids a compiler warning (not handled enumeration values)
    ;
  }
  return -1;
}

void ExpirationCollectionAttribute::daysToExpire(int& unreadDays, int& readDays) {
  unreadDays = ExpirationCollectionAttribute::daysToExpire( unreadExpireAge(), unreadExpireUnits() );
  readDays = ExpirationCollectionAttribute::daysToExpire( readExpireAge(), readExpireUnits() );
}

ExpirationCollectionAttribute* ExpirationCollectionAttribute::expirationCollectionAttribute( const Akonadi::Collection& collection, bool &mustDeleteExpirationAttribute )
{
  MailCommon::ExpirationCollectionAttribute *attr = 0;
  if ( collection.hasAttribute<MailCommon::ExpirationCollectionAttribute>() ) {
    attr = collection.attribute<MailCommon::ExpirationCollectionAttribute>();
    mustDeleteExpirationAttribute = false;
  } else {
    attr = new MailCommon::ExpirationCollectionAttribute();
    attr->loadFromConfig( collection );
    mustDeleteExpirationAttribute = true;
  }
  return attr;
}


QByteArray ExpirationCollectionAttribute::serialized() const
{
  QByteArray result;
  QDataStream s( &result, QIODevice::WriteOnly );

  s << mExpireToFolderId;
  s << ( int )mExpireAction;
  s << ( int )mReadExpireUnits;
  s << mReadExpireAge;
  s <<  ( int )mUnreadExpireUnits;
  s << mUnreadExpireAge;
  s << mExpireMessages;

  return result;
}

void ExpirationCollectionAttribute::deserialize( const QByteArray &data )
{
  QDataStream s( data );
  s >> mExpireToFolderId;
  int val;
  s >> val;
  mExpireAction = ( ExpirationCollectionAttribute::ExpireAction )val;
  int valUnit;
  s >> valUnit;
  mReadExpireUnits =  ( ExpirationCollectionAttribute::ExpireUnits )valUnit;
  s >> mReadExpireAge;
  int valUnitUread;
  s >> valUnitUread;
  mUnreadExpireUnits =  ( ExpirationCollectionAttribute::ExpireUnits )valUnitUread;
  s >> mUnreadExpireAge;
  s >> mExpireMessages;
}

