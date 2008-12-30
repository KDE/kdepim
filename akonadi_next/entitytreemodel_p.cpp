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
#include <akonadi/monitor.h>

#include <kdebug.h>

using namespace Akonadi;

EntityTreeModelPrivate::EntityTreeModelPrivate( EntityTreeModel *parent )
    : q_ptr( parent )
{
}

void EntityTreeModelPrivate::init()
{
  Q_Q( EntityTreeModel );

  // ### Hack to get the kmail resource folder icons . REMOVE
  KIconLoader::global()->addAppDir( QLatin1String( "kmail" ) );
  KIconLoader::global()->addAppDir( QLatin1String( "kdepim" ) );

  // Used to get child collections and items when a collection is added.
  q->connect( q, SIGNAL( rowsInserted( const QModelIndex &, int, int ) ),
              q, SLOT( onRowsInserted( const QModelIndex &, int, int ) ) );

  q->connect( entityUpdateAdapter, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ),
              q, SLOT( collectionsReceived( Akonadi::Collection::List ) ) );
  q->connect( entityUpdateAdapter, SIGNAL( itemsReceived( Akonadi::Item::List, Collection::Id ) ),
              q, SLOT( itemsReceived( Akonadi::Item::List, Collection::Id ) ) );

  // monitor collection changes
  q->connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
              q, SLOT( collectionChanged( const Akonadi::Collection& ) ) );
  q->connect( monitor, SIGNAL( collectionAdded( Akonadi::Collection, Akonadi::Collection ) ),
              q, SLOT( collectionAdded( Akonadi::Collection, Akonadi::Collection ) ) );
  q->connect( monitor, SIGNAL( collectionRemoved( Akonadi::Collection ) ),
              q, SLOT( collectionRemoved( Akonadi::Collection ) ) );

  // Monitor item changes.
  q->connect( monitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
              q, SLOT( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );
  q->connect( monitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
              q, SLOT( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
  q->connect( monitor, SIGNAL( itemRemoved( const Akonadi::Item& ) ),
              q, SLOT( itemRemoved( const Akonadi::Item& ) ) );
  q->connect( monitor, SIGNAL( itemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ),
              q, SLOT( itemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ) );

  q->connect( monitor, SIGNAL( collectionStatisticsChanged( Akonadi::Collection::Id, Akonadi::CollectionStatistics ) ),
              q, SLOT( collectionStatisticsChanged( Akonadi::Collection::Id, Akonadi::CollectionStatistics ) ) );

// TODO:
//   q->connect( monitor, SIGNAL( itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));
//   q->connect( monitor, SIGNAL( itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));


  collections.insert( m_rootCollection.id(), m_rootCollection );
  kDebug() << m_rootCollection.id() << m_rootCollection.name();

  // Includes recursive trees. Lower levels are fetched in the onRowsInserted slot if
  // necessary.
  if (m_entitiesToFetch & EntityTreeModel::FetchFirstLevelChildCollections)
  {
    entityUpdateAdapter->fetchCollections( m_rootCollection, CollectionFetchJob::FirstLevel );
  }
  if (m_entitiesToFetch & EntityTreeModel::FetchItems)
  {
    entityUpdateAdapter->fetchItems( m_rootCollection );
  }
  // else someone wants a model which has neither items nor collections.
  // Not that anyone would want such a thing...
}


void EntityTreeModelPrivate::collectionRemoved( const Akonadi::Collection &collection )
{
  // TODO: can I be sure child indexes are already gone from the model? I don't think I can be.
  // However, I don't think it matters. Maybe beginRemoveRows takes that into account.
  // Other wise I'll have to persist pending deletes somewhere.
  kDebug() << collection.name() << collection.id();
  Q_Q( EntityTreeModel );
  QModelIndex colIndex = indexForId( collection.id() );
  if ( colIndex.isValid() ) {
    QModelIndex parentIndex = q->parent( colIndex );
    QList<qint64> list = m_childEntities.value( collection.parent() );
    int row = list.indexOf( collection.id() );
    q->beginRemoveRows( parentIndex, row, row );

    collections.remove( collection.id() );                // Remove deleted collection.
    m_childEntities.remove( collection.id() );           // Remove children of deleted collection.
    m_childEntities[ collection.parent()].removeAt( row );  // Remove deleted collection from its parent.

    // Removal of items in this collection is handled by itemRemoved.

    q->endRemoveRows();
  }
}

void EntityTreeModelPrivate::collectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent )
{
  Q_Q( EntityTreeModel );
  if ( !passesFilter( collection.contentMimeTypes() ) )
    return;
  kDebug() << collection.id() << parent.id();
  QModelIndex parentIndex = indexForId( parent.id() );



  // TODO: Use order attribute of parent if available
  // Otherwise prepend collections and append items. Currently this prepends all collections.

  // Ah! Actually I can prepend and append for single signals, then 'change' the parent.

//   QList<qint64> childCols = m_childEntities.value( parent.id() );
//   int row = childCols.size();
//   int numChildCols = childCollections.value(parent.id()).size();
  int row = 0;

  q->beginInsertRows( parentIndex, row, row );
  collections.insert( collection.id(), collection );
  m_childEntities[ parent.id()].prepend( collection.id() );
  q->endInsertRows();
}

void EntityTreeModelPrivate::collectionChanged( const Akonadi::Collection &collection )
{
  // Change could be a change of parent. Handle that with a layout change.
  Q_Q( EntityTreeModel );
  // What kind of change is it ?
  Collection::Id oldParentId = collections.value( collection.id() ).parent();
  Collection::Id newParentId = collection.parent();
  if ( newParentId != oldParentId && oldParentId >= 0 ) { // It's a move to a different parent.

    q->layoutAboutToBeChanged();
//     updatePersistentIndexes. TODO: Do this properly.
//     QList<Collection::Id> oldColList = childCollections[oldParentId];
// //     QList<Item::Id> oldItemList = childCollections[oldParentId];
//
//     int oldColIndex = oldColList.indexOf(collection.id());
//     int oldColListSize = oldColList.size();
//     if (oldColIndex != -1)
//     {
//       oldColIndex++; // Skip past the found id.
//       while (oldColIndex < oldColListSize)
//       {
//           QModelIndex oldIndex = indexForId(oldColList[oldColIndex]);
//           q->changePersistentIndex(
//               oldIndex,
//               q->createIndex(oldIndex.row() -1, oldIndex.column(), reinterpret_cast<void*>(oldIndex.internalId()))
//           );
//           ++oldColIndex;
//       }
//     }

    // move it
    collections.insert( collection.id(), collection );
    m_childEntities[ oldParentId ].removeOne( collection.id() );
    m_childEntities[ newParentId ].prepend( collection.id() );
    // Simply prepend the collection. Order is restored when its parent is 'changed'.

    q->layoutChanged();
  } else { // It's a simple change. Could be that an item was added or removed from it?
    kDebug() << "simple change";

//     update display attributes etc.

    Collection col = collections.value( newParentId );
    if ( col.hasAttribute<CollectionChildOrderAttribute>() ) {
      CollectionChildOrderAttribute *a = col.attribute<CollectionChildOrderAttribute>();
      QStringList l = a->orderList();
      kDebug() << l;
      orderChildEntities( col, l );
    }
  }
}

void EntityTreeModelPrivate::orderChildEntities( Collection col, QStringList l )
{
// TODO:
//     q->layoutAboutToBeChanged();

//     q->layoutChanged();
}


void EntityTreeModelPrivate::onRowsInserted( const QModelIndex & parent, int start, int end )
{
  Q_Q( EntityTreeModel );

  // This slot can be notified of several new collections in the model.
  // Iterate to add items for each one.
  for ( int iCount = start; iCount <= end; iCount++ ) {
    QModelIndex i = q->index( iCount, 0, parent );
    if ( !i.isValid() )
      continue;

    if ( isItem( i ) )
      continue;

    QVariant datav = q->data( i, EntityTreeModel::CollectionRole );
    if ( !datav.isValid() ) {
      // This is not a valid collection, so it doesn't have any items.
      continue;
    }
    // A new collection was inserted. get its items and child collections.

    Akonadi::Collection col = qvariant_cast< Akonadi::Collection > ( datav );

    if (m_entitiesToFetch == EntityTreeModel::FetchCollectionsRecursive)
    {
      // Fetch the next level of collections if neccessary.
      entityUpdateAdapter->fetchCollections( col, CollectionFetchJob::FirstLevel );
    }
    if (m_entitiesToFetch & EntityTreeModel::FetchItems)
    {
      // Fetch items if neccessary.
      entityUpdateAdapter->fetchItems( col );
    }
  }
}

void EntityTreeModelPrivate::collectionsReceived( const Collection::List &cols )
{
  Q_Q( EntityTreeModel );
  QHash<Collection::Id, Collection> newCollections;
  QHash< Collection::Id, QList< Collection::Id > > newChildCollections;
  foreach( Collection col, cols ) {
    if ( collections.contains( col.id() ) ) {
      // If the collection is already known to the model, we simply update it...
      // Notify akonadi about this? Do I need a col stats job? Or something in entityUpdateAdapter?
      col.setStatistics( collections.value( col.id() ).statistics() );
      collections[ col.id()] = col;
      // Surely this is useless. startIndex and endIndex are the same...
      // Maybe I can check for contiguous indexes and emit in groups.
      QModelIndex startIndex = indexForId( col.id() );
      QModelIndex endIndex = indexForId( col.id(), q->columnCount( q->parent( startIndex ) ) - 1 );
      emit q->dataChanged( startIndex, endIndex );
      continue;
    }
    // ... otherwise we add it to the set of collections we need to handle.
    if ( passesFilter( col.contentMimeTypes() ) ) {
      newChildCollections[ col.parent()].append( col.id() );
      newCollections.insert( col.id(), col );
    }
  }

  // Insert new child collections into model.
  QHashIterator< Collection::Id, QList< Collection::Id > > i( newChildCollections );
  while ( i.hasNext() ) {
    i.next();

    // the key is the parent of new collections.
    Collection::Id parentId = i.key();

    QList< Collection::Id > newChildCols = i.value();
    int newChildCount = newChildCols.size();

    if ( collections.contains( parentId ) ) {
      QModelIndex parentIndex = indexForId( parentId );
      kDebug() << parentIndex;
//       int startRow = childEntities.value( parentId ).size();
      int startRow = 0; // Prepend collections.

//       TODO: account for ordering.

      q->beginInsertRows( parentIndex,
                          startRow,      // Start index is at the end of existing collections.
                          startRow + newChildCount - 1 ); // End index is the result of the insertion.
      foreach( Collection::Id id, newChildCols ) {
        Collection c = newCollections.take( id );
        kDebug() << id << c.name();
        collections.insert( id, c );
        m_childEntities[ parentId ].prepend( id );
      }
      q->endInsertRows();
    }
    // TODO: Fetch parent again so that its entities get ordered properly. Or start a modify job?
    // Should I do this for all other cases as well instead of using transactions?
    // Could be a way around the fact that monitor could notify me of things out of order. a parent could
    // be 'changed' for its entities to be reordered before one of its entities is in the model yet.
  }
}

// TODO: Change to be more like collectionsReceived?
void EntityTreeModelPrivate::itemsReceived( const Akonadi::Item::List &list, Collection::Id colId )
{
  Q_Q( EntityTreeModel );
  Item::List itemsToInsert;
  Item::List itemsToUpdate;

  foreach( Item item, list ) {
    if ( m_items.contains( item.id() * -1 ) ) {
      kDebug() << "update" << m_items.value( item.id() * -1 ).remoteId();
      itemsToUpdate << item;
    } else {
      if ( passesFilter( QStringList() << item.mimeType() ) ) {
        kDebug() << "insert" << item.id() << item.remoteId();
        itemsToInsert << item;
      }
    }
  }
  if ( itemsToInsert.size() > 0 ) {
    QModelIndex parentIndex = indexForId( colId );
    int startRow = childEntitiesCount( parentIndex );
    q->beginInsertRows( parentIndex,
                        startRow,
                        startRow + itemsToInsert.size() - 1 );
    foreach( Item item, itemsToInsert ) {
      kDebug() << "inserting" << item.remoteId();
      qint64 itemId = item.id() * -1;
      m_items.insert( itemId, item );
      m_childEntities[ colId ].append( itemId );
    }
    q->endInsertRows();
  }
  if ( itemsToUpdate.size() > 0 ) {
// TODO: Implement
  }
}

void EntityTreeModelPrivate::itemChanged( const Akonadi::Item &item, const QSet< QByteArray >& )
{
  kDebug() << item.remoteId();
  Q_Q( EntityTreeModel );
  m_items[ item.id() * -1 ] = item;
  QModelIndex i = indexForItem( item );
  if ( i.isValid() )
    emit q->dataChanged( i, i );
}


void EntityTreeModelPrivate::itemMoved( const Akonadi::Item &item, const Akonadi::Collection& colSrc, const Akonadi::Collection& colDst )
{
  Q_Q( EntityTreeModel );
  kDebug() << "item.remoteId=" << item.remoteId() << "sourceCollectionRemoteId=" << colSrc.remoteId()
  << "destCollectionRemoteId=" << colDst.remoteId() << "destCollectionId=" << colDst.id();


// Updating persistent indexes could be troublesome. I remove and reinsert items instead.
//   q->layoutAboutToBeChanged();

  // updatePersistentIndexes();

//     QList<Collection::Id> oldColList = childCollections[oldParentId];
// //     QList<Item::Id> oldItemList = m_itemsInCollection[oldParentId];
//
//     int oldColIndex = oldColList.indexOf(collection.id());
//     int oldColListSize = oldColList.size();
//     if (oldColIndex != -1)
//     {
//       oldColIndex++; // Skip past the found id.
//       while (oldColIndex < oldColListSize)
//       {
//           QModelIndex oldIndex = indexForId(oldColList[oldColIndex]);
//           q->changePersistentIndex(
//               oldIndex,
//               q->createIndex(oldIndex.row() -1, oldIndex.column(), reinterpret_cast<void*>(oldIndex.internalId()))
//           );
//           ++oldColIndex;
//       }
//     }

  QModelIndex srcParentIndex = indexForId( colSrc.id() );
  QModelIndex dstParentIndex = indexForId( colDst.id() );
  QModelIndex itemIndex = indexForItem( item );
  int srcRow = itemIndex.row();
  q->beginRemoveRows( srcParentIndex, srcRow, srcRow );
  Item i = m_items.take( item.id() * -1 );
  m_childEntities[ colSrc.id()].removeOne( item.id() * -1 );
  q->endRemoveRows();

  int dstRow = childEntitiesCount( dstParentIndex );
  q->beginInsertRows( dstParentIndex, dstRow, dstRow );
  m_items.insert( item.id() * -1, i );
  m_childEntities[ colDst.id()].append( item.id() * -1 );
  q->endInsertRows();

//   q->layoutChanged();

}

void EntityTreeModelPrivate::itemAdded( const Akonadi::Item &item, const Akonadi::Collection& col )
{
  Q_Q( EntityTreeModel );

  if ( !passesFilter( QStringList() << item.mimeType() ) )
    return;

  QModelIndex parentIndex = indexForId( col.id() );
  q->beginInsertRows( parentIndex, childEntitiesCount( parentIndex ), childEntitiesCount( parentIndex ) );
  m_items.insert( item.id() * -1, item );
  m_childEntities[ col.id()].append( item.id() * -1 );
  q->endInsertRows();
}

void EntityTreeModelPrivate::itemRemoved( const Akonadi::Item &item )
{
  Q_Q( EntityTreeModel );
  QModelIndex itemIndex = indexForItem( item );
  QModelIndex parentIndex = itemIndex.parent();
  Collection col = collectionForIndex(parentIndex);
  QList<qint64> list = m_childEntities.value( col.id() );

  int row = itemIndex.row();

  q->beginRemoveRows( parentIndex, row, row );

  m_items.remove( item.id() * -1 );
  m_childEntities[ col.id() ].removeAt( row );

  q->endRemoveRows();
}

void EntityTreeModelPrivate::collectionStatisticsChanged( Collection::Id collection,
    const Akonadi::CollectionStatistics &statistics )
{
  Q_Q( EntityTreeModel );

  if ( !collections.contains( collection ) )
    kWarning( 5250 ) << "Got statistics response for non-existing collection:" << collection;
  else {
    collections[ collection ].setStatistics( statistics );

    Collection col = collections.value( collection );
    QModelIndex startIndex = indexForId( col.id() );
    QModelIndex endIndex = indexForId( col.id(), q->columnCount( q->parent( startIndex ) ) - 1 );
    emit q->dataChanged( startIndex, endIndex );
  }
}

Collection EntityTreeModelPrivate::findParentCollection( Item item ) const
{
  return findParentCollection( item.id() * -1 );
}

Collection EntityTreeModelPrivate::findParentCollection( qint64 id ) const
{
  QHashIterator< Collection::Id, QList<qint64> > iter( m_childEntities );
  while ( iter.hasNext() ) {
    iter.next();
    if ( iter.value().contains( id ) ) {
      return collections.value( iter.key() );
    }
  }
  return Collection( -1 );
}

QModelIndex EntityTreeModelPrivate::indexForItem( Item item )
{
  return indexForId( item.id() * -1 );
}

QModelIndex EntityTreeModelPrivate::indexForId( qint64 id, int column ) const
{
  Q_Q( const EntityTreeModel );

  // Early return.
  if ( id == m_rootCollection.id() )
    return QModelIndex();

  Collection col = findParentCollection( id );
  if ( !col.isValid() )
    return QModelIndex();

  int row = m_childEntities.value( col.id() ).indexOf( id );

  return q->createIndex( row, column, reinterpret_cast<void*>( id ) );
}

bool EntityTreeModelPrivate::passesFilter( const QStringList &mimetypes )
{
  if (m_mimeTypeFilter.size() == 0)
    return true;
  return mimetypeMatches( m_mimeTypeFilter, mimetypes );
}

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

Collection EntityTreeModelPrivate::collectionForIndex( QModelIndex idx ) const
{
  qint64 id;
  if ( idx.isValid() )
    id = idx.internalId();
  else
    id = m_rootCollection.id();
  return collections.value( id );
}

// Item itemForIndex(QModelIndex idx)
// {
//   return d->m_items.value( idx.internalId() );
// }

int EntityTreeModelPrivate::childEntitiesCount( const QModelIndex & parent ) const
{
  Collection col = collectionForIndex( parent );
  return m_childEntities.value( col.id() ).size();
}

bool EntityTreeModelPrivate::isItem( const QModelIndex &index ) const
{
  return ( index.internalId() < 0 );
}

bool EntityTreeModelPrivate::isCollection( const QModelIndex &index ) const
{
  // Note: Collection::root().id() is 0.
  return ( index.internalId() >= 0 );
}

