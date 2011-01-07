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

#include <akonadi/kmime/messageflags.h>
#include <messagecore/stringutil.h>

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
          const KDateTime leftThreadRootDateTime = leftThreadRootMessage->date()->dateTime();
          const KDateTime rightThreadRootDateTime = rightThreadRootMessage->date()->dateTime();
          if ( leftThreadRootDateTime != rightThreadRootDateTime ) {
            return leftThreadRootDateTime > rightThreadRootDateTime;
          }
        }
        break;
      case SortByDateTimeMostRecent:
        {
          const KDateTime leftNewest = mostRecentUpdate( leftThreadRootMessage, leftThreadRootItem.id() );
          const KDateTime rightNewest = mostRecentUpdate( rightThreadRootMessage, rightThreadRootItem.id() );

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

      const KDateTime leftDateTime = leftMessage->date()->dateTime();
      const KDateTime rightDateTime = rightMessage->date()->dateTime();

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

  invalidate();
}

MailThreadGrouperComparator::SortingOption MailThreadGrouperComparator::sortingOption() const
{
  return mSortingOption;
}

void MailThreadGrouperComparator::setIsOutboundCollection( bool outbound )
{
  mIsOutboundCollection = outbound;

  invalidate();
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

KDateTime MailThreadGrouperComparator::mostRecentUpdate( const KMime::Message::Ptr &threadRoot, Akonadi::Item::Id itemId ) const
{
  const QHash<Akonadi::Item::Id, KDateTime>::const_iterator it = mMostRecentCache.constFind( itemId );
  if ( it != mMostRecentCache.constEnd() )
    return *it;

  const QSet<QByteArray> messageIds = threadDescendants( identifierForMessage( threadRoot, itemId ) );

  KDateTime newest = threadRoot->date()->dateTime();

  if ( messageIds.isEmpty() ) {
    mMostRecentCache.insert( itemId, newest );
    return newest;
  }

  foreach ( const QByteArray &messageId, messageIds ) {
    const Akonadi::Item item = itemForIdentifier( messageId );
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = messageForItem( item );
    const KDateTime messageDateTime = message->date()->dateTime();
    if ( messageDateTime > newest )
      newest = messageDateTime;
  }

  mMostRecentCache.insert( itemId, newest );
  return newest;
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

#include "mailthreadgroupercomparator.moc"
