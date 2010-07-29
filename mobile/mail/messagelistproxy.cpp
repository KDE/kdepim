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

#include <KLocale>
#include <KGlobal>

MessageListProxy::MessageListProxy(QObject* parent) : ListProxy(parent)
{
  setDynamicSortFilter( true );
  sort( 0, Qt::DescendingOrder );
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
        return msg->subject()->asUnicodeString();
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

  return leftMsg->date()->dateTime() < rightMsg->date()->dateTime();
}

#include "messagelistproxy.moc"
