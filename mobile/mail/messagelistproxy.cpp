/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "messagelistproxy.h"

#include <akonadi/item.h>
#include <KMime/Message>

#include <akonadi/kmime/messagestatus.h>

#include <KCalendarSystem>
#include <KLocale>
#include <KGlobal>

MessageListProxy::MessageListProxy(QObject* parent) : ListProxy(parent)
{
  // Sorting is done higher up now in the thread grouping proxy.
//   setDynamicSortFilter( true );
//   sort( 0, Qt::DescendingOrder );
}

QVariant MessageListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KMime::Message::Ptr>() ) {
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    Akonadi::MessageStatus messageStatus;
    messageStatus.setStatusFromFlags(item.flags());

    switch ( role ) {
      case SubjectRole:
        return msg->subject()->asUnicodeString().trimmed();
      case FromRole:
      {
        QStringList l;
        foreach ( const KMime::Types::Mailbox &mbox, msg->from()->mailboxes() ) {
          if ( mbox.hasName() )
            l.append( mbox.name() );
          else
            l.append(  mbox.addrSpec().asPrettyString() );
        }
        return l.join( ", " );
      }
      case DateRole:
      {
        const KDateTime& dt = msg->date()->dateTime();
        if ( dt.date() == QDate::currentDate() )
          return KGlobal::locale()->formatTime( dt.time() );
        return KGlobal::locale()->formatDate( dt.date(), KLocale::FancyShortDate );
      }
      case IsUnreadRole:
        return messageStatus.isUnread();
      case IsImportantRole:
        return messageStatus.isImportant();
      case IsActionItemRole:
        return messageStatus.isToAct();
      case HasAttachmentRole:
        return messageStatus.hasAttachment();
      case IsRepliedRole:
        return messageStatus.isReplied();
      case IsForwardedRole:
        return messageStatus.isForwarded();
      case IsSignedRole:
        return messageStatus.isSigned();
      case IsEncryptedRole:
        return messageStatus.isEncrypted();
      case DateGroupRole:
      {
        // simplified version taken from libmessagelist
        const KDateTime& dt = msg->date()->dateTime();
        const QDate dDate = dt.date();
        const KCalendarSystem *calendar = KGlobal::locale()->calendar();
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
          return KGlobal::locale()->calendar()->weekDayName( dDate );
        else if( calendar->month( dDate ) == calendar->month( QDate::currentDate() ) && calendar->year( dDate ) == calendar->year( QDate::currentDate() ) ) { // within this month
          const int startOfWeekDaysAgo = ( calendar->daysInWeek( QDate::currentDate() ) + calendar->dayOfWeek( QDate::currentDate() ) -
                                           KGlobal::locale()->weekStartDay() ) % calendar->daysInWeek( QDate::currentDate() );
          const int weeksAgo = ( ( daysAgo - startOfWeekDaysAgo ) / calendar->daysInWeek( QDate::currentDate() ) ) + 1;
          if ( weeksAgo == 0 )
            return KGlobal::locale()->calendar()->weekDayName( dDate );
          else
            return i18np( "One Week Ago", "%1 Weeks Ago", weeksAgo );
        } else if ( calendar->year( dDate ) == calendar->year( QDate::currentDate() ) ) { // within this year
          return calendar->monthName( dDate );
        } else { // in previous years
          return i18nc( "Message Aggregation Group Header: Month name and Year number", "%1 %2", calendar->monthName( dDate ), calendar->yearString( dDate ) );
        }
      }
    }
  }
  return QSortFilterProxyModel::data(index, role);
}

void MessageListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  ListProxy::setSourceModel(sourceModel);
  QHash<int, QByteArray> names = roleNames();
  names.insert( Akonadi::EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( SubjectRole, "subject" );
  names.insert( FromRole, "from" );
  names.insert( DateRole, "date" );
  names.insert( IsUnreadRole, "is_unread" );
  names.insert( IsImportantRole, "is_important" );
  names.insert( IsActionItemRole, "is_action_item" );
  names.insert( HasAttachmentRole, "has_attachment" );
  names.insert( IsRepliedRole, "is_replied" );
  names.insert( IsForwardedRole, "is_forwarded" );
  names.insert( IsSignedRole, "is_signed" );
  names.insert( IsEncryptedRole, "is_encrypted" );
  names.insert( DateGroupRole, "dateGroup" );
  setRoleNames( names );
  kDebug() << names << sourceModel->roleNames();
}

bool MessageListProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !leftItem.hasPayload<KMime::Message::Ptr>() || !rightItem.hasPayload<KMime::Message::Ptr>() )
    return leftItem.id() < rightItem.id();

  const KMime::Message::Ptr leftMsg = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightMsg = rightItem.payload<KMime::Message::Ptr>();

  if ( leftMsg->date()->dateTime() == rightMsg->date()->dateTime() )
    return leftItem.id() < rightItem.id();
  return leftMsg->date()->dateTime() < rightMsg->date()->dateTime();
}

#include "messagelistproxy.moc"
