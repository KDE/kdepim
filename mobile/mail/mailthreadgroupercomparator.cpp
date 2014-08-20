/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mailthreadgroupercomparator.h"

#include <Akonadi/KMime/MessageFlags>
#include <messagecore/utils/stringutil.h>

#include <KLocalizedString>
#include <kglobal.h>
#include <kcalendarsystem.h>
#include <KLocalizedString>
#include <QLocale>

MailThreadGrouperComparator::MailThreadGrouperComparator()
  : mSortingOption( SortByDateTimeMostRecent ),
    mIsOutboundCollection( false )
{
}

MailThreadGrouperComparator::~MailThreadGrouperComparator()
{
}

QByteArray MailThreadGrouperComparator::identifierForItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

  return identifierForMessage( item.payload<KMime::Message::Ptr>(), item.id() );
}

QByteArray MailThreadGrouperComparator::parentIdentifierForItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

  const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();

  if ( !message->inReplyTo()->isEmpty() ) {
    const QByteArray inReplyTo = message->inReplyTo()->as7BitString( false );
    return inReplyTo.mid( 1, inReplyTo.size() -2 ); // strip '<' and '>'
  }

  return QByteArray();
}

bool MailThreadGrouperComparator::lessThan( const Akonadi::Item &leftItem, const Akonadi::Item &rightItem ) const
{
  Q_ASSERT( leftItem.isValid() );
  Q_ASSERT( rightItem.isValid() );

  const Akonadi::Item leftThreadRootItem = threadItem( leftItem );
  const Akonadi::Item rightThreadRootItem = threadItem( rightItem );

  Q_ASSERT( rightThreadRootItem.isValid() );
  Q_ASSERT( leftThreadRootItem.isValid() );

  const bool leftItemIsThreadLeader = (leftThreadRootItem == leftItem);
  const bool rightItemIsThreadLeader = (rightThreadRootItem == rightItem);
  if ( leftItemIsThreadLeader && rightItemIsThreadLeader ) {
    Q_ASSERT( leftThreadRootItem.hasPayload<KMime::Message::Ptr>() );
    Q_ASSERT( rightThreadRootItem.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr leftThreadRootMessage = messageForItem( leftThreadRootItem );
    const KMime::Message::Ptr rightThreadRootMessage = messageForItem( rightThreadRootItem );

    switch ( mSortingOption ) {
      case SortByDateTime:
        {
          const QDateTime leftThreadRootDateTime = leftThreadRootMessage->date()->dateTime();
          const QDateTime rightThreadRootDateTime = rightThreadRootMessage->date()->dateTime();
          if ( leftThreadRootDateTime != rightThreadRootDateTime ) {
            return leftThreadRootDateTime > rightThreadRootDateTime;
          }
        }
        break;
      case SortByDateTimeMostRecent:
        {
          const QDateTime leftNewest = mostRecentDateTimeInThread( leftThreadRootMessage, leftThreadRootItem.id() );
          const QDateTime rightNewest = mostRecentDateTimeInThread( rightThreadRootMessage, rightThreadRootItem.id() );

          if ( leftNewest != rightNewest ) {
            return leftNewest > rightNewest;
          }
        }
        break;
      case SortBySenderReceiver:
        {
          const QString leftSender = (mIsOutboundCollection ? leftThreadRootMessage->to()->asUnicodeString()
                                                            : leftThreadRootMessage->from()->asUnicodeString());
          const QString rightSender = (mIsOutboundCollection ? rightThreadRootMessage->to()->asUnicodeString()
                                                             : rightThreadRootMessage->sender()->asUnicodeString());

          if ( leftSender != rightSender )
            return (leftSender.localeAwareCompare( rightSender ) < 0);
        }
        break;
      case SortBySubject:
        {
          const QString leftSubject = MessageCore::StringUtil::stripOffPrefixes( leftThreadRootMessage->subject()->asUnicodeString() );
          const QString rightSubject = MessageCore::StringUtil::stripOffPrefixes( rightThreadRootMessage->subject()->asUnicodeString() );

          if ( leftSubject != rightSubject )
            return (leftSubject.compare( rightSubject, Qt::CaseInsensitive ) < 0);
        }
        break;
      case SortBySize:
        {
          const qint64 leftSize = leftThreadRootItem.size();
          const qint64 rightSize = rightThreadRootItem.size();

          if ( leftSize != rightSize )
            return leftSize < rightSize;
        }
        break;
      case SortByActionItem:
        {
          const bool leftIsActionItem = leftThreadRootItem.flags().contains( Akonadi::MessageFlags::ToAct );
          const bool rightIsActionItem = rightThreadRootItem.flags().contains( Akonadi::MessageFlags::ToAct );

          if ( leftIsActionItem != rightIsActionItem )
            return leftIsActionItem;
        }
        break;
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();

  } else if ( leftItemIsThreadLeader && !rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem )
      return true; // right item is in thread of left thread leader -> right item located below left item
    else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  } else if ( !leftItemIsThreadLeader && rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem )
      return false; // left item is in thread of right thread leader -> left item must be located below right item
    else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  } else if ( !leftItemIsThreadLeader && !rightItemIsThreadLeader ) {
    if ( leftThreadRootItem == rightThreadRootItem ) { // both in the same thread
      Q_ASSERT( leftItem.hasPayload<KMime::Message::Ptr>() );
      Q_ASSERT( rightItem.hasPayload<KMime::Message::Ptr>() );

      const KMime::Message::Ptr leftMessage = messageForItem( leftItem );
      const KMime::Message::Ptr rightMessage = messageForItem( rightItem );

      const QDateTime leftDateTime = leftMessage->date()->dateTime();
      const QDateTime rightDateTime = rightMessage->date()->dateTime();

      // Messages in the same thread are ordered most recent last.
      if ( leftDateTime != rightDateTime ) {
        return leftDateTime < rightDateTime;
      }

      return leftItem.id() < rightItem.id(); // default
    } else
      return lessThan( leftThreadRootItem, rightThreadRootItem ); // based on thread leaders order
  }

  return leftItem.id() < rightItem.id();
}

void MailThreadGrouperComparator::setSortingOption( SortingOption option )
{
  mSortingOption = option;
}

MailThreadGrouperComparator::SortingOption MailThreadGrouperComparator::sortingOption() const
{
  return mSortingOption;
}

void MailThreadGrouperComparator::setGroupingOption( GroupingOption option )
{
  mGroupingOption = option;
}

MailThreadGrouperComparator::GroupingOption MailThreadGrouperComparator::groupingOption() const
{
  return mGroupingOption;
}

void MailThreadGrouperComparator::setIsOutboundCollection( bool outbound )
{
  mIsOutboundCollection = outbound;
}

void MailThreadGrouperComparator::invalidateModel()
{
  invalidate();
}

QString MailThreadGrouperComparator::grouperString( const Akonadi::Item &item ) const
{
  KMime::Message::Ptr msg;

  if ( mSortingOption == SortByDateTimeMostRecent ) {
    const Akonadi::Item rootItem = threadItem( item );
    const Akonadi::Item::Id newestItem = mostRecentIdInThread( messageForItem( rootItem ), rootItem.id() );
    msg = messageForItem( Akonadi::Item( newestItem ) );
  } else {
    const Akonadi::Item rootItem = threadItem( item );
    msg = messageForItem( rootItem );
  }

  if ( mGroupingOption == GroupByDate ) {
    // simplified version taken from libmessagelist
    const QDateTime& dt = msg->date()->dateTime();
    const QDate dDate = dt.date();
    const KCalendarSystem *calendar = KLocale::global()->calendar();
    int daysAgo = -1;
    if ( calendar->isValid( dDate ) && calendar->isValid( QDate::currentDate() ) ) {
      daysAgo = dDate.daysTo( QDate::currentDate() );
    }

    if ( daysAgo < 0 || !dt.isValid() ) // In the future or invalid
      return i18n( "Unknown" );
    else if( daysAgo == 0 ) // Today
      return i18n( "Today" );
    else if ( daysAgo == 1 ) // Yesterday
      return i18n( "Yesterday" );
    else if ( daysAgo > 1 && daysAgo < calendar->daysInWeek( QDate::currentDate() ) ) // Within last seven days
      return KLocale::global()->calendar()->weekDayName( dDate );
    else if( calendar->month( dDate ) == calendar->month( QDate::currentDate() ) && calendar->year( dDate ) == calendar->year( QDate::currentDate() ) ) { // within this month
      const int startOfWeekDaysAgo = ( calendar->daysInWeek( QDate::currentDate() ) + calendar->dayOfWeek( QDate::currentDate() ) -
                                       QLocale().firstDayOfWeek() ) % calendar->daysInWeek( QDate::currentDate() );
      const int weeksAgo = ( ( daysAgo - startOfWeekDaysAgo ) / calendar->daysInWeek( QDate::currentDate() ) ) + 1;
      if ( weeksAgo == 0 )
        return KLocale::global()->calendar()->weekDayName( dDate );
      else
        return i18np( "One Week Ago", "%1 Weeks Ago", weeksAgo );
    } else if ( calendar->year( dDate ) == calendar->year( QDate::currentDate() ) ) { // within this year
      return calendar->monthName( dDate );
    } else { // in previous years
      static QHash<int, QString> yearNameHash;

      QString yearName;
      if ( yearNameHash.contains( dDate.year() ) ) {
        yearName = yearNameHash.value( dDate.year() );
      } else {
        yearName = calendar->formatDate( dDate, KLocale::Year, KLocale::LongNumber );
        yearNameHash.insert( dDate.year(), yearName );
      }
      return i18nc( "Message Aggregation Group Header: Month name and Year number", "%1 %2", calendar->monthName( dDate ), yearName );
    }
  } else if ( mGroupingOption == GroupBySenderReceiver ) {
    QStringList l;
    foreach ( const KMime::Types::Mailbox &mbox, msg->from()->mailboxes() ) {
      if ( mbox.hasName() )
        l.append( mbox.name() );
      else
        l.append( mbox.addrSpec().asPrettyString() );
    }
    return l.join( QLatin1String(", ") );
  } else {
    return QLatin1String( "dummy" );
  }
}

void MailThreadGrouperComparator::resetCaches()
{
  mMessageCache.clear();
  mMostRecentCache.clear();
}

QByteArray MailThreadGrouperComparator::identifierForMessage( const KMime::Message::Ptr &message, Akonadi::Item::Id id ) const
{
  QByteArray identifier = message->messageID()->identifier();
  if ( identifier.isEmpty() )
    identifier = QByteArray::number( id );

  return identifier;
}

QDateTime MailThreadGrouperComparator::mostRecentDateTimeInThread( const KMime::Message::Ptr &threadRoot, Akonadi::Item::Id itemId ) const
{
  const QHash<Akonadi::Item::Id, MostRecentEntry>::const_iterator it = mMostRecentCache.constFind( itemId );
  if ( it != mMostRecentCache.constEnd() )
    return (*it).dateTime;

  const QSet<QByteArray> messageIds = threadDescendants( identifierForMessage( threadRoot, itemId ) );

  QDateTime newest = threadRoot->date()->dateTime();
  Akonadi::Item::Id newestId = itemId;

  if ( messageIds.isEmpty() ) {
    MostRecentEntry entry;
    entry.id = newestId;
    entry.dateTime = newest;
    mMostRecentCache.insert( itemId, entry );
    return newest;
  }

  foreach ( const QByteArray &messageId, messageIds ) {
    const Akonadi::Item item = itemForIdentifier( messageId );
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = messageForItem( item );
    const QDateTime messageDateTime = message->date()->dateTime();
    if ( messageDateTime > newest ) {
      newest = messageDateTime;
      newestId = item.id();
    }
  }

  MostRecentEntry entry;
  entry.id = newestId;
  entry.dateTime = newest;

  mMostRecentCache.insert( itemId, entry );
  return newest;
}

Akonadi::Item::Id MailThreadGrouperComparator::mostRecentIdInThread( const KMime::Message::Ptr &threadRoot, Akonadi::Item::Id itemId ) const
{
  const QHash<Akonadi::Item::Id, MostRecentEntry>::const_iterator it = mMostRecentCache.constFind( itemId );
  if ( it != mMostRecentCache.constEnd() )
    return (*it).id;

  const QSet<QByteArray> messageIds = threadDescendants( identifierForMessage( threadRoot, itemId ) );

  QDateTime newest = threadRoot->date()->dateTime();
  Akonadi::Item::Id newestId = itemId;

  if ( messageIds.isEmpty() ) {
    MostRecentEntry entry;
    entry.id = newestId;
    entry.dateTime = newest;
    mMostRecentCache.insert( itemId, entry );
    return itemId;
  }

  foreach ( const QByteArray &messageId, messageIds ) {
    const Akonadi::Item item = itemForIdentifier( messageId );
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = messageForItem( item );
    const QDateTime messageDateTime = message->date()->dateTime();
    if ( messageDateTime > newest )
      newest = messageDateTime;
      newestId = item.id();
  }

  MostRecentEntry entry;
  entry.id = newestId;
  entry.dateTime = newest;

  mMostRecentCache.insert( itemId, entry );
  return itemId;
}

KMime::Message::Ptr MailThreadGrouperComparator::messageForItem( const Akonadi::Item &item ) const
{
  const QHash<Akonadi::Item::Id, KMime::Message::Ptr>::const_iterator it = mMessageCache.constFind( item.id() );
  if ( it != mMessageCache.constEnd() )
    return *it;

  KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
  mMessageCache.insert( item.id(), message );

  return message;
}

