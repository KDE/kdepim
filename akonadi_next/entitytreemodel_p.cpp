/*
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "entitytreemodel_p.h"
#include "entitytreemodel.h"

#include <KUrl>
#include <KIconLoader>

#include <akonadi/collectionstatistics.h>
#include <akonadi/collectionstatisticsjob.h>
#include <akonadi/collectionfetchjob.h>
#include "collectionchildorderattribute.h"
#include <akonadi/entitydisplayattribute.h>
#include "entityupdateadapter.h"
#include "clientsideentitystorage.h"

#include <kdebug.h>

using namespace Akonadi;

EntityTreeModelPrivate::EntityTreeModelPrivate( EntityTreeModel *parent )
    : q_ptr( parent )
{
}

void EntityTreeModelPrivate::rowsAboutToBeInserted( Collection::Id colId, int start, int end )
{
  Q_Q( EntityTreeModel );

  Collection parent = m_clientSideEntityStorage->getCollection(colId);
  QModelIndex parentIndex;

  if (parent.isValid())
  {
    int parentRow = m_clientSideEntityStorage->indexOf(parent.parent(), parent.id());
    if (parentRow < 0)
      parentIndex = QModelIndex();
    else
      parentIndex = q->createIndex(parentRow, 0, reinterpret_cast<void *>( parent.id() ) );
  }

  q->beginInsertRows(parentIndex, start, end );
}

void EntityTreeModelPrivate::rowsInserted()
{
  Q_Q( EntityTreeModel );
  q->endInsertRows();
}

void EntityTreeModelPrivate::rowsAboutToBeRemoved( Collection::Id colId, int start, int end )
{
  Q_Q( EntityTreeModel );

  Collection parent = m_clientSideEntityStorage->getCollection(colId);
  QModelIndex parentIndex;

  if (parent.isValid())
  {
    int parentRow = m_clientSideEntityStorage->indexOf(parent.parent(), parent.id());
    if (parentRow < 0)
      parentIndex = QModelIndex();
    else
      parentIndex = q->createIndex(parentRow, 0, reinterpret_cast<void *>( parent.id() ) );
  }
  q->beginRemoveRows( parentIndex, start, end );

}

void EntityTreeModelPrivate::rowsRemoved()
{
  Q_Q( EntityTreeModel );
  q->endRemoveRows();
}

void EntityTreeModelPrivate::collectionChanged( const Akonadi::Collection &collection )
{
  // Change could be a change of parent. Handle that with a layout change.
  Q_Q( EntityTreeModel );
  Collection colParent = m_clientSideEntityStorage->getParentCollection(collection);
  int row = m_clientSideEntityStorage->indexOf( colParent.id(), collection.id() );
  qint64 internalId = m_clientSideEntityStorage->internalEntityIdentifier( collection );
  QModelIndex idx = q->createIndex(row, 0, reinterpret_cast<void *>( internalId ) );
  if ( idx.isValid() )
    emit q->dataChanged( idx, idx );
  return;
}

void EntityTreeModelPrivate::itemChanged( const Akonadi::Item &item, const QSet< QByteArray >&parts )
{
  Q_Q( EntityTreeModel );
  Collection col = m_clientSideEntityStorage->getParentCollection(item);
  int row = m_clientSideEntityStorage->indexOf( col.id(), item.id() );
  qint64 internalId = m_clientSideEntityStorage->internalEntityIdentifier( item );
  QModelIndex idx = q->createIndex(row, 0, reinterpret_cast<void *>( internalId ) );
  if ( idx.isValid() )
    emit q->dataChanged( idx, idx );
}

// void EntityTreeModelPrivate::collectionStatisticsChanged( Collection::Id collection,
//     const Akonadi::CollectionStatistics &statistics )
// {
// TODO: Move this to clientSideEntityStorage.
//   Q_Q( EntityTreeModel );
//
//   if ( !collections.contains( collection ) )
//     kWarning( 5250 ) << "Got statistics response for non-existing collection:" << collection;
//   else {
//     collections[ collection ].setStatistics( statistics );
//
//     Collection col = collections.value( collection );
//     QModelIndex startIndex = indexForId( col.id() );
//     QModelIndex endIndex = indexForId( col.id(), q->columnCount( q->parent( startIndex ) ) - 1 );
//     emit q->dataChanged( startIndex, endIndex );
//   }
// }

bool EntityTreeModelPrivate::mimetypeMatches( const QStringList &mimetypes, const QStringList &other )
{
  QStringList::const_iterator constIterator;
  bool found = false;

  for ( constIterator = mimetypes.constBegin(); constIterator != mimetypes.constEnd(); ++constIterator ) {
    if ( other.contains(( *constIterator ) ) ) {
      found = true;
      break;
    }
  }
  return found;
}
