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

#include <messagecore/stringutil.h>


MailThreadGrouperComparator::MailThreadGrouperComparator()
  : m_sortingOption( SortByDateTimeMostRecent )
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

  if ( leftThreadRootItem != rightThreadRootItem ) {
    Q_ASSERT( leftThreadRootItem.hasPayload<KMime::Message::Ptr>() );
    Q_ASSERT( rightThreadRootItem.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr leftThreadRootMessage = leftThreadRootItem.payload<KMime::Message::Ptr>();
    const KMime::Message::Ptr rightThreadRootMessage = rightThreadRootItem.payload<KMime::Message::Ptr>();

    switch ( m_sortingOption ) {
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
          const QString leftSender = leftThreadRootMessage->sender()->asUnicodeString();
          const QString rightSender = rightThreadRootMessage->sender()->asUnicodeString();

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
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();
  }

  if ( leftThreadRootItem == leftItem )
    return true;

  if ( rightThreadRootItem == rightItem )
    return false;

  Q_ASSERT( leftItem.hasPayload<KMime::Message::Ptr>() );
  Q_ASSERT( rightItem.hasPayload<KMime::Message::Ptr>() );

  const KMime::Message::Ptr leftMessage = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightMessage = rightItem.payload<KMime::Message::Ptr>();

  const KDateTime leftDateTime = leftMessage->date()->dateTime();
  const KDateTime rightDateTime = rightMessage->date()->dateTime();

  // Messages in the same thread are ordered most recent last.
  if ( leftDateTime != rightDateTime ) {
    return leftDateTime < rightDateTime;
  }

  return leftItem.id() < rightItem.id();
}

void MailThreadGrouperComparator::setSortingOption( SortingOption option )
{
  m_sortingOption = option;

  invalidate();
}

MailThreadGrouperComparator::SortingOption MailThreadGrouperComparator::sortingOption() const
{
  return m_sortingOption;
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
  const QSet<QByteArray> messageIds = threadDescendants( identifierForMessage( threadRoot, itemId ) );

  KDateTime newest = threadRoot->date()->dateTime();

  if ( messageIds.isEmpty() )
    return newest;

  foreach ( const QByteArray &messageId, messageIds ) {
    const Akonadi::Item item = itemForIdentifier( messageId );
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    const KDateTime messageDateTime = message->date()->dateTime();
    if ( messageDateTime > newest )
      newest = messageDateTime;
  }

  return newest;
}

#include "mailthreadgroupercomparator.moc"
