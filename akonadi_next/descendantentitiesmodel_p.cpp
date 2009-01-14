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

#include "descendantentitiesmodel_p.h"
#include "descendantentitiesmodel.h"

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

DescendantEntitiesModelPrivate::DescendantEntitiesModelPrivate( DescendantEntitiesModel *parent )
    : q_ptr( parent )
{
}

void DescendantEntitiesModelPrivate::rowsAboutToBeInserted( Collection::Id colId, int start, int end )
{
  Q_Q( DescendantEntitiesModel );

  int beginRow;

  if (start == 0)
  {
    if (Collection::root().id() == colId)
    {
      beginRow = 0;
    } else {
      beginRow = countEntitiesAbove( colId ) + 1;
    }
  } else {
    // entId needs to be the entity the start item will be inserted below.
    qint64 entId = m_clientSideEntityStorage->childAt( colId, start - 1 );
    int ans = countEntitiesAbove( entId );
    int desc = descendantCount( entId );
    beginRow = ans + desc + 1;
  }

  QModelIndex parentIndex;

  q->beginInsertRows( parentIndex, beginRow, beginRow + ( end - start ) );
}

void DescendantEntitiesModelPrivate::rowsInserted()
{
  Q_Q( DescendantEntitiesModel );
  q->endInsertRows();
}

void DescendantEntitiesModelPrivate::rowsAboutToBeRemoved( Collection::Id colId, int start, int end )
{
  Q_Q( DescendantEntitiesModel );

  // TODO: Handle deletion of child entities. Probably need to set a flag on the storage class to tell it to delete recursively or not.

  int beginRow;
  if (start == 0)
  {
    if (Collection::root().id() == colId)
    {
      beginRow = 0;
    } else {
      beginRow = countEntitiesAbove( colId ) + 1;
    }
  } else {
    // entity at start will be removed from beneath entId.
    qint64 entId = m_clientSideEntityStorage->childAt( colId, start - 1 );
    int ans = countEntitiesAbove( entId );
    int desc = descendantCount( entId );
    beginRow = ans + desc + 1;
  }
  QModelIndex parentIndex;
  q->beginRemoveRows( parentIndex, beginRow, beginRow + ( end - start ) );

}

void DescendantEntitiesModelPrivate::rowsRemoved()
{
  Q_Q( DescendantEntitiesModel );
  q->endRemoveRows();
}

void DescendantEntitiesModelPrivate::collectionChanged( const Akonadi::Collection &collection )
{
  Q_Q( DescendantEntitiesModel );
  int row = countEntitiesAbove( collection.id() );

  QModelIndex idx = q->createIndex(row, 0, reinterpret_cast<void *>( collection.id() ) );

  if ( idx.isValid() )
    emit q->dataChanged( idx, idx );
  return;
}

void DescendantEntitiesModelPrivate::itemChanged( const Akonadi::Item &item, const QSet< QByteArray >&parts )
{
  Q_Q( DescendantEntitiesModel );

  int row = countEntitiesAbove( item.id() );
  QModelIndex idx = q->createIndex(row, 0, reinterpret_cast<void *>( item.id() ) );

  if ( idx.isValid() )
    emit q->dataChanged( idx, idx );
}

// int DescendantEntitiesModelPrivate::countEntitiesAbove( Collection::Id colId, qint64 stopId ) const
int DescendantEntitiesModelPrivate::countEntitiesAbove( qint64 stopId ) const
{
  Collection::Id colId = Collection::root().id();

  // Doesn't have any items above it.
  if (stopId == Collection::root().id() )
    return 0;

  Collection parent;

  int entityType = m_clientSideEntityStorage->entityTypeForInternalIdentifier( stopId );
  if (ClientSideEntityStorage::CollectionType == entityType)
  {
    Collection col = m_clientSideEntityStorage->getCollection( stopId );
    parent = m_clientSideEntityStorage->getParentCollection( col );
  } else if (ClientSideEntityStorage::ItemType == entityType)
  {
    Item item = m_clientSideEntityStorage->getItem( stopId );
    parent = m_clientSideEntityStorage->getParentCollection( item );
  }
  int countAbove = m_clientSideEntityStorage->indexOf( parent.id(), stopId );


  // Recursively go up the tree until we reach the root.
  while ( parent.id() != Collection::root().id() )
  {
    // Loop over the uncles of stopId until we reach its parent.
    ClientSideEntityStorage::Iterator i( parent.parent() );
    while (i.hasNext())
    {
      qint64 uncle = i.next();
      // Add the uncle (and our parent)
      countAbove++;
      if (uncle == parent.id())
        break;
      // Add the children of this uncle.
      countAbove += descendantCount(uncle);
    }

    if (parent.parent() == Collection::root().id())
      break;

    int entityType = m_clientSideEntityStorage->entityTypeForInternalIdentifier( parent.parent() );
    if (ClientSideEntityStorage::CollectionType == entityType)
    {
      Collection col = m_clientSideEntityStorage->getCollection( parent.parent() );
      parent = m_clientSideEntityStorage->getParentCollection( col );
    } else if (ClientSideEntityStorage::ItemType == entityType)
    {
      Item item = m_clientSideEntityStorage->getItem( parent.parent() );
      parent = m_clientSideEntityStorage->getParentCollection( item );
    }
  }
  return countAbove;
}

int DescendantEntitiesModelPrivate::descendantCount( Collection::Id colId ) const
{
  // TODO Optimization: maintain a hashtable of the results from here.
  int c = 0;

  c += m_clientSideEntityStorage->childEntitiesCount(colId);

  ClientSideEntityStorage::Iterator i(colId);
  while (i.hasNext())
  {
    qint64 entId = i.next();
    int entityType = m_clientSideEntityStorage->entityTypeForInternalIdentifier(entId);
    if ( ClientSideEntityStorage::CollectionType == entityType )
    {
      c += descendantCount(entId);
    }
  }

  return c;
}

qint64 DescendantEntitiesModelPrivate::findIdForRow( int row, Collection::Id colId, bool *ok ) const
{
  int descCount = descendantCount(colId);

  if (row < descCount)
  {
    ClientSideEntityStorage::Iterator i(colId);
    while (i.hasNext())
    {
      qint64 entId = i.next();
      if (row == 0)
      {
        *ok = true;
        return entId;
      }
      row--;
      int entityType = m_clientSideEntityStorage->entityTypeForInternalIdentifier(entId);
      if ( ClientSideEntityStorage::CollectionType == entityType )
      {
        qint64 id = findIdForRow(row, entId, ok);
        if (*ok)
          return id;
        row -= descendantCount(entId);
      }
    }
  }
  *ok = false;
  return 0;
}
