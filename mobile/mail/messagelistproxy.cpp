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

#include <AkonadiCore/item.h>
#include <KMime/Message>

#include <Akonadi/KMime/MessageStatus>

#include <KCalendarSystem>
#include <KLocale>
#include <KGlobal>
#include <KFormat>

inline uint qHash( const QDate &date )
{
  return date.toJulianDay();
}

MessageListProxy::MessageListProxy(QObject* parent) : ListProxy(parent)
{
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
        return l.join( QLatin1String(", ") );
      }
      case DateRole:
      {
        static QHash<QDate, QString> dateNameHash;

        const KDateTime &dateTime = msg->date()->dateTime().toLocalZone();
        const QDate date = dateTime.date();
        if ( date == QDate::currentDate() ) {
          return KLocale::global()->formatTime( dateTime.time() );
        }

        const QHash<QDate, QString>::const_iterator key = dateNameHash.constFind( date );
        if ( key != dateNameHash.constEnd() )
          return *key;

        const QString dateName = KLocale::global()->formatDate( date, KLocale::FancyShortDate );
        dateNameHash.insert( date, dateName );

        return dateName;
      }
      case SizeRole:
      {
        return KFormat().formatByteSize( qMax( 0LL, item.size() ) );
      }
      case IsUnreadRole:
        return !messageStatus.isRead();
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
                                           KLocale::global()->weekStartDay() ) % calendar->daysInWeek( QDate::currentDate() );
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
      }
      case SenderGroupRole:
      {
        QStringList l;
        foreach ( const KMime::Types::Mailbox &mbox, msg->from()->mailboxes() ) {
          if ( mbox.hasName() )
            l.append( mbox.name() );
          else
            l.append( mbox.addrSpec().asPrettyString() );
        }
        return l.join( QLatin1String(", ") );
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
  names.insert( SizeRole, "size" );
  names.insert( IsUnreadRole, "is_unread" );
  names.insert( IsImportantRole, "is_important" );
  names.insert( IsActionItemRole, "is_action_item" );
  names.insert( HasAttachmentRole, "has_attachment" );
  names.insert( IsRepliedRole, "is_replied" );
  names.insert( IsForwardedRole, "is_forwarded" );
  names.insert( IsSignedRole, "is_signed" );
  names.insert( IsEncryptedRole, "is_encrypted" );
  names.insert( DateGroupRole, "dateGroup" );
  names.insert( SenderGroupRole, "senderGroup" );
  setRoleNames( names );
}

