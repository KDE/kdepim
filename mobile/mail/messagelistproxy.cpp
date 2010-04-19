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

MessageListProxy::MessageListProxy(QObject* parent) : ListProxy(parent)
{
}

QVariant MessageListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KMime::Message::Ptr>() ) {
    const KMime::Message::Ptr msg = item.payload<KMime::Message::Ptr>();
    switch ( role ) {
      case SubjectRole:
        return msg->subject()->asUnicodeString();
      case FromRole:
        return msg->from()->asUnicodeString();
      case DateRole:
        return msg->date()->asUnicodeString();
    }
  }
  return QSortFilterProxyModel::data(index, role);
}

int MessageListProxy::messageCount() const
{
  return rowCount();
}

qint64 MessageListProxy::messageId( int row ) const
{
  if ( row < 0 || row >= rowCount() )
    return -1;

  QModelIndex idx = index( row, 0 );
  if ( !idx.isValid() )
    return -1;

  const Akonadi::Item item = QSortFilterProxyModel::data( idx, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  kDebug() << item.id();
  return item.id();
}

void MessageListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  QSortFilterProxyModel::setSourceModel(sourceModel);
  QHash<int, QByteArray> names = roleNames();
  names.insert( Akonadi::EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( SubjectRole, "subject" );
  names.insert( FromRole, "from" );
  names.insert( DateRole, "date" );
  setRoleNames( names );
  kDebug() << names << sourceModel->roleNames();
}

#include "messagelistproxy.moc"
