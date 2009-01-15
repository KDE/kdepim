/*
    Copyright (c) 2009 Stephen Kelly <steveire@gmail.com>

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

#include "clientsideentitystorage.h"
#include "clientsideentitystorage_p.h"

#include <QVariant>

#include <akonadi/itemfetchjob.h>
#include <akonadi/monitor.h>

#include <kdebug.h>

using namespace Akonadi;

ClientSideEntityStoragePrivate::ClientSideEntityStoragePrivate( ClientSideEntityStorage *parent ) : q_ptr( parent )
{
}

void ClientSideEntityStoragePrivate::startFirstListJob()
{
  m_collections.insert( m_rootCollection.id(), m_rootCollection );

  // Includes recursive trees. Lower levels are fetched in the onRowsInserted slot if
  // necessary.
  kDebug() << m_entitiesToFetch;
  if (m_entitiesToFetch &
        ( ClientSideEntityStorage::FetchFirstLevelChildCollections | ClientSideEntityStorage::FetchCollectionsRecursive )
     )
  {
    fetchCollections( m_rootCollection, CollectionFetchJob::FirstLevel );
  }
  // If the root collection is not collection::root, then it could have items, and they will need to be
  // retrieved now.

  if ( m_itemPopulation == ClientSideEntityStorage::LazyPopulation )
    fetchItems( m_rootCollection, Base );

}

void ClientSideEntityStoragePrivate::fetchJobDone( KJob *job )
{
  if ( job->error() ) {
    kWarning( 5250 ) << "Job error: " << job->errorString() << endl;
  }
}

void ClientSideEntityStoragePrivate::updateJobDone( KJob *job )
{
  if ( job->error() ) {
    // TODO: handle job errors
    kWarning( 5250 ) << "Job error:" << job->errorString();
  } else {
    // TODO: Is this trying to do the job of collectionstatisticschanged?
//     CollectionStatisticsJob *csjob = static_cast<CollectionStatisticsJob*>( job );
//     Collection result = csjob->collection();
//     collectionStatisticsChanged( result.id(), csjob->statistics() );
  }
}


Collection ClientSideEntityStoragePrivate::getParentCollection( qint64 id )
{
  QHashIterator< Collection::Id, QList<qint64> > iter( m_childEntities );
  while ( iter.hasNext() ) {
    iter.next();
    if ( iter.value().contains( id ) ) {
      return m_collections.value( iter.key() );
    }
  }
  return Collection();
}

void ClientSideEntityStoragePrivate::collectionsFetched( const Akonadi::Collection::List& cols )
{
  Q_Q( ClientSideEntityStorage );
  QHash<Collection::Id, Collection> newCollections;
  QHash< Collection::Id, QList< Collection::Id > > newChildCollections;
  foreach( Collection col, cols ) {
//     kDebug() << col.name() << m_entitiesToFetch;
    if ( m_collections.contains( col.id() ) ) {
      // If the collection is already known to the model, we simply update it...
      // Notify akonadi about this? Do I need a col stats job? Or something in entityUpdateAdapter?
//       col.setStatistics( m_collections.value( col.id() ).statistics() );
//       m_collections[ col.id()] = col;
      // Surely this is useless. startIndex and endIndex are the same...
      // Maybe I can check for contiguous indexes and emit in groups.
//       QModelIndex startIndex = indexForId( col.id() );
//       QModelIndex endIndex = indexForId( col.id(), q->columnCount( q->parent( startIndex ) ) - 1 );
//       emit q->dataChanged( startIndex, endIndex );
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

    if ( m_collections.contains( parentId ) ) {
      int startRow = 0; // Prepend collections.

  //       TODO: account for ordering.
      q->beginInsertEntities( parentId, startRow, startRow + newChildCount - 1 );
        foreach( Collection::Id id, newChildCols ) {
          Collection c = newCollections.value( id );
          m_collections.insert( id, c );
          m_childEntities[ parentId ].prepend( id );
        }
      q->endInsertEntities();

      foreach( Collection::Id id, newChildCols ) {
        Collection col = newCollections.value( id );
        // Fetch the next level of collections if neccessary.
        if ( m_entitiesToFetch & ClientSideEntityStorage::FetchCollectionsRecursive )
        {
          fetchCollections( col, CollectionFetchJob::FirstLevel );
        }
        // Fetch items if neccessary. If we don't fetch them now, we'll wait for an application
        // to request them through ClientSideEntityStorage::populateCollection
        if ( m_itemPopulation == ClientSideEntityStorage::ImmediatePopulation )
        {
          fetchItems( col, Base );
        }
      }
    }
    // TODO: Fetch parent again so that its entities get ordered properly. Or start a modify job?
    // Should I do this for all other cases as well instead of using transactions?
    // Could be a way around the fact that monitor could notify me of things out of order. a parent could
    // be 'changed' for its entities to be reordered before one of its entities is in the model yet.
  }
}

void ClientSideEntityStoragePrivate::itemsFetched( const Akonadi::Item::List& list )
{
  Q_Q( ClientSideEntityStorage );
  QObject *job = q->sender();
  if ( job ) {
    Collection::Id colId = job->property( ItemFetchCollectionId() ).value<Collection::Id>();
    Item::List itemsToInsert;
    Item::List itemsToUpdate;

    foreach( Item item, list ) {
      kDebug() << "from job" << item.remoteId();
      if ( m_items.contains( item.id() * -1 ) ) {
        itemsToUpdate << item;
      } else {
        if ( passesFilter( QStringList() << item.mimeType() ) ) {
          itemsToInsert << item;
        }
      }
    }
    if ( itemsToInsert.size() > 0 ) {
      int startRow = m_childEntities.value( colId ).size();
      q->beginInsertEntities(colId, startRow, startRow + itemsToInsert.size() - 1 );
      foreach( Item item, itemsToInsert ) {
        qint64 itemId = item.id() * -1;
        m_items.insert( itemId, item );
        m_childEntities[ colId ].append( itemId );
      }
      q->endInsertEntities();
    }
    if ( itemsToUpdate.size() > 0 ) {
      // TODO: Implement
      // ... Or remove. This slot is only for new collections and items fetched from akonadi.
    }
  }
}

void ClientSideEntityStoragePrivate::monitoredCollectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent )
{
  Q_Q( ClientSideEntityStorage );
  if ( !passesFilter( collection.contentMimeTypes() ) )
    return;

  // TODO: Use order attribute of parent if available
  // Otherwise prepend collections and append items. Currently this prepends all collections.

  // Ah! Actually I can prepend and append for single signals, then 'change' the parent.

//   QList<qint64> childCols = m_childEntities.value( parent.id() );
//   int row = childCols.size();
//   int numChildCols = childCollections.value(parent.id()).size();
  int row = 0;

  q->beginInsertEntities( parent.id(), row, row );
  m_collections.insert( collection.id(), collection );
  m_childEntities[ parent.id()].prepend( collection.id() );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredCollectionRemoved( const Akonadi::Collection& collection )
{
  // TODO: can I be sure child indexes are already gone from the model? I don't think I can be.
  // However, I don't think it matters. Maybe beginRemoveRows takes that into account.
  // Other wise I'll have to persist pending deletes somewhere.
  Q_Q( ClientSideEntityStorage );

  int row = q->indexOf( collection.parent(), collection.id() );

  q->beginRemoveEntities(collection.parent(), row, row);

  m_collections.remove( collection.id() );                 // Remove deleted collection.
  m_childEntities.remove( collection.id() );               // Remove children of deleted collection.
  m_childEntities[ collection.parent() ].removeAt( row );  // Remove deleted collection from its parent.

  q->endRemoveEntities();
}

void ClientSideEntityStoragePrivate::monitoredCollectionMoved( const Akonadi::Collection& col, const Akonadi::Collection& src, const Akonadi::Collection& dest)
{
  // This should possibly tell the model layoutAboutToBeChanged instead, but
  // Then I think I'd also have to update persistent model indexes, which is troublesome.
  // Instead I remove and add.
  // This has the benefit of making the api of this storage class simpler (no need for move
  // signals, nothing model-specific in the api ) but may make actual models slower, especially when
  // moving collections containing many items.
  // Stephen Kelly, 11 Jan 2009.

  Q_Q( ClientSideEntityStorage );
  int srcRow = q->indexOf( src.id(), col.id() );

  q->beginRemoveEntities( src.id(), srcRow, srcRow );
  Collection c = m_collections.take( col.id() );
  m_childEntities[ src.id()].removeOne( col.id() );
  q->endRemoveEntities();

  int dstRow = 0; // Prepend collections

  q->beginInsertEntities( dest.id(), dstRow, dstRow );
  m_collections.insert( col.id(), col );
  m_childEntities[ dest.id() ].prepend( col.id() );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredCollectionChanged( const Akonadi::Collection &col)
{
  Q_Q( ClientSideEntityStorage );
  m_collections[ col.id() ] == col;
  q->collectionChanged(col);
}

void ClientSideEntityStoragePrivate::monitoredCollectionStatisticsChanged( Akonadi::Collection::Id collection, const Akonadi::CollectionStatistics &statistics )
{
kDebug();
}

void ClientSideEntityStoragePrivate::monitoredItemAdded( const Akonadi::Item& item, const Akonadi::Collection& col)
{
  Q_Q( ClientSideEntityStorage );

  if ( !passesFilter( QStringList() << item.mimeType() ) )
    return;

  int row = q->childEntitiesCount( col.id() );

  q->beginInsertEntities( col.id(), row, row );
  m_items.insert( item.id() * -1, item );
  m_childEntities[ col.id()].append( item.id() * -1 );
  q->endInsertEntities();

}

void ClientSideEntityStoragePrivate::monitoredItemRemoved( const Akonadi::Item &item )
{
  Q_Q( ClientSideEntityStorage );

  Collection col = q->getParentCollection( item );

  int row = q->indexOf(col.id(), item.id() * -1);

  q->beginRemoveEntities(col.id(), row, row);
  m_items.remove( item.id() * -1 );
  m_childEntities[ col.id() ].removeAt( row );
  q->endRemoveEntities();
}

void ClientSideEntityStoragePrivate::monitoredItemChanged( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
  Q_Q( ClientSideEntityStorage );
  m_items[ item.id() ] == item;
  q->itemChanged(item, parts);
}

void ClientSideEntityStoragePrivate::monitoredItemMoved( const Akonadi::Item& item,
                  const Akonadi::Collection& src, const Akonadi::Collection& dest )
{
  Q_Q( ClientSideEntityStorage );
  int srcRow = q->indexOf( src.id(), item.id() * -1 );

  q->beginRemoveEntities( src.id(), srcRow, srcRow );
  Item i = m_items.take( item.id() * -1 );
  m_childEntities[ src.id()].removeOne( item.id() * -1 );
  q->endRemoveEntities();

  int dstRow = q->childEntitiesCount( dest.id() );

  q->beginInsertEntities( dest.id(), dstRow, dstRow );
  m_items.insert( item.id() * -1, i );
  m_childEntities[ dest.id()].append( item.id() * -1 );
  q->endInsertEntities();
}

bool ClientSideEntityStoragePrivate::passesFilter( const QStringList &mimetypes )
{
  if (m_mimeTypeFilter.size() == 0)
    return true;
  return mimetypeMatches( m_mimeTypeFilter, mimetypes );
}

bool ClientSideEntityStoragePrivate::mimetypeMatches( const QStringList &mimetypes, const QStringList &other )
{
  QStringList::const_iterator constIterator;
  bool found = false;

  for ( constIterator = mimetypes.constBegin(); constIterator != mimetypes.constEnd(); ++constIterator ) {
    if ( other.contains( ( *constIterator ) ) ) {
      found = true;
      break;
    }
  }
  return found;
}

void ClientSideEntityStoragePrivate::fetchItems( Collection parent, int retrieveDepth )
{
  Q_Q( ClientSideEntityStorage );
  Akonadi::ItemFetchJob *itemJob = new Akonadi::ItemFetchJob( parent );
  itemJob->setFetchScope( m_monitor->itemFetchScope() );
  kDebug() << parent.id();

  // ### HACK: itemsReceivedFromJob needs to know which collection items were added to.
  // That is not provided by akonadi, so we attach it in a property.
  itemJob->setProperty( ItemFetchCollectionId(), QVariant( parent.id() ) );

  q->connect( itemJob, SIGNAL( itemsReceived( Akonadi::Item::List ) ),
           q, SLOT( itemsFetched( Akonadi::Item::List ) ) );
  q->connect( itemJob, SIGNAL( result( KJob* ) ),
           q, SLOT( fetchJobDone( KJob* ) ) );

  if ( retrieveDepth == Recursive )
  {
    ClientSideEntityStorage::Iterator i(parent.id());
    while (i.hasNext())
    {
      qint64 eid = i.next();
      if (eid > 0)
      {
        fetchItems( q->getCollection( eid ), Recursive );
      }
    }
  }
}

void ClientSideEntityStoragePrivate::fetchCollections( Collection col, CollectionFetchJob::Type type )
{
  Q_Q( ClientSideEntityStorage );
  // Session?
  kDebug() << col.name();
  CollectionFetchJob *job = new CollectionFetchJob( col, type );
  job->includeUnsubscribed( m_includeUnsubscribed );
  q->connect( job, SIGNAL( collectionsReceived( Akonadi::Collection::List ) ),
           q, SLOT( collectionsFetched( Akonadi::Collection::List ) ) );
  q->connect( job, SIGNAL( result( KJob* ) ),
           q, SLOT( fetchJobDone( KJob* ) ) );

}


