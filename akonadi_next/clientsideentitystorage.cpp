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

#include <QListIterator>
#include <QTimer>

#include <akonadi/itemfetchscope.h>
#include <akonadi/monitor.h>

#include <kdebug.h>

using namespace Akonadi;

static ClientSideEntityStorage *sClientSideEntityStorage = 0;

ClientSideEntityStorage::Iterator::Iterator( Collection::Id colId )
{
  iter_collection = colId;
  iter_position = 0;
}

bool ClientSideEntityStorage::Iterator::hasNext()
{
  return ( iter_position < sClientSideEntityStorage->d_ptr->m_childEntities.value( iter_collection ).size() );
}

qint64 ClientSideEntityStorage::Iterator::next()
{
  return sClientSideEntityStorage->d_ptr->m_childEntities.value( iter_collection ).at( iter_position++ );
}

ClientSideEntityStorage::ClientSideEntityStorage( Monitor *monitor,
                                                  QStringList mimetypes,
                                                  Collection rootCollection,
                                                  QObject *parent,
                                                  int entitiesToFetch,
                                                  int itemPopulation,
                                                  int includeUnsubscribed )
              : QObject( parent ), d_ptr( new ClientSideEntityStoragePrivate( this ) )
{
  Q_D( ClientSideEntityStorage );
  d->m_monitor = monitor;
  d->m_entitiesToFetch = entitiesToFetch;
  d->m_rootCollection = rootCollection;
  d->m_mimeTypeFilter = mimetypes;
  d->m_itemPopulation = itemPopulation;
  d->m_includeUnsubscribed = ( includeUnsubscribed == IncludeUnsubscribed ) ? true : false;
  sClientSideEntityStorage = this;

  // monitor collection changes
  connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
           SLOT( monitoredCollectionChanged( const Akonadi::Collection& ) ) );
  connect( monitor, SIGNAL( collectionAdded( const Akonadi::Collection &, const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionAdded( const Akonadi::Collection &, const Akonadi::Collection & ) ) );
  connect( monitor, SIGNAL( collectionRemoved( const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionRemoved( const Akonadi::Collection &) ) );
  connect( monitor,
            SIGNAL( collectionMoved( const Akonadi::Collection &, const Akonadi::Collection &, const Akonadi::Collection & ) ),
           SLOT( monitoredCollectionMoved( const Akonadi::Collection &, const Akonadi::Collection &, const Akonadi::Collection & ) ) );

  //TODO: Figure out if the monitor emits these signals even without an item fetch scope.
  // Wrap them in an if() if so.


  // Monitor item changes.
  connect( monitor, SIGNAL( itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ),
           SLOT( monitoredItemAdded( const Akonadi::Item&, const Akonadi::Collection& ) ) );
  connect( monitor, SIGNAL( itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ),
           SLOT( monitoredItemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) ) );
  connect( monitor, SIGNAL( itemRemoved( const Akonadi::Item& ) ),
           SLOT( monitoredItemRemoved( const Akonadi::Item& ) ) );
  connect( monitor, SIGNAL( itemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ),
           SLOT( monitoredItemMoved( const Akonadi::Item, const Akonadi::Collection, const Akonadi::Collection ) ) );

  connect( monitor, SIGNAL( collectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &) ),
           SLOT(monitoredCollectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &) ) );


// TODO:
//   q->connect( monitor, SIGNAL( itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemLinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));
//   q->connect( monitor, SIGNAL( itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)),
//             q, SLOT(itemUnlinked(const Akonadi::Item &item, const Akonadi::Collection &collection)));


  // Start retrieving collections and items at the next event loop.
  QTimer::singleShot( 0, this, SLOT(startFirstListJob()) );

}

Collection ClientSideEntityStorage::rootCollection()
{
  Q_D( ClientSideEntityStorage );
  return d->m_rootCollection;
}

ClientSideEntityStorage::~ClientSideEntityStorage()
{
  Q_D( ClientSideEntityStorage );
  d->m_collections.clear();
  d->m_items.clear();
  d->m_childEntities.clear();
}

Collection ClientSideEntityStorage::getCollection( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );
  return d->m_collections.value(id);
}

Item ClientSideEntityStorage::getItem( qint64 id )
{
  Q_D( ClientSideEntityStorage );
  if (id > 0) id *= -1;
  return d->m_items.value(id);
}

qint64 ClientSideEntityStorage::internalEntityIdentifier( Item item )
{
  return item.id() * -1;
}

qint64 ClientSideEntityStorage::internalEntityIdentifier( Collection col )
{
  return col.id();
}

int ClientSideEntityStorage::childEntitiesCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );
  return d->m_childEntities.value( id ).size();
}

int ClientSideEntityStorage::childItemsCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );

  int itemCount = 0;
  QListIterator<qint64> i( d->m_childEntities.value( id ) );
  while ( i.hasNext() )
  {
    if ( i.next() < 0 )
    {
      itemCount++;
    }
  }
  return itemCount;
}

int ClientSideEntityStorage::childCollectionsCount( Collection::Id id )
{
  Q_D( ClientSideEntityStorage );

  int colCount = 0;
  QListIterator<qint64> i( d->m_childEntities.value( id ) );
  while (i.hasNext())
  {
    if (i.next() > 0)
    {
      colCount++;
    }
  }
  return colCount;
}

qint64 ClientSideEntityStorage::childAt( Collection::Id id, int internalIndex )
{
  Q_D( ClientSideEntityStorage );
  QList<qint64> list = d->m_childEntities.value( id );
  if (list.size() <= internalIndex )
    return -1;
  return list.at( internalIndex );
}

int ClientSideEntityStorage::indexOf( Collection::Id parent, Collection::Id col )
{
//   if ( col == m_rootCollection.id() )
//     return 0;
  Q_D( ClientSideEntityStorage );

  return d->m_childEntities.value(parent).indexOf(col);
}

int ClientSideEntityStorage::entityTypeForInternalIdentifier( qint64 t )
{
  // TODO: Check that the item exists in the QHash first and return UnknownType if it doesn't exist.
  return (t > 0) ? CollectionType : ItemType ;
}

Collection ClientSideEntityStorage::getParentCollection( Collection col )
{
  Q_D( ClientSideEntityStorage );
  return d->m_collections.value( col.parent() );
}

Collection ClientSideEntityStorage::getParentCollection( Item item )
{
  Q_D( ClientSideEntityStorage );
  return d->getParentCollection( item.id() * -1 );
}

void ClientSideEntityStorage::populateCollection( Collection::Id colId )
{
  Q_D( ClientSideEntityStorage );

  if ( d->m_itemPopulation == ImmediatePopulation )
    return;
  else if ( d->m_itemPopulation == LazyPopulation )
  {
    d->m_populatedCols.insert( colId );
    d->fetchItems( d->m_collections.value( colId ), ClientSideEntityStoragePrivate::Base );
  }
}

bool ClientSideEntityStorage::canPopulate( Collection::Id colId )
{
  Q_D( ClientSideEntityStorage );
  if ( d->m_itemPopulation == ClientSideEntityStorage::ImmediatePopulation )
    return false;
  if ( !d->m_collections.contains( colId ) )
    return false;
  if ( d->m_populatedCols.contains( colId ) )
    return false;
  return true;
}

void ClientSideEntityStorage::purgeCollection( Collection::Id colId )
{
  Q_D( ClientSideEntityStorage );
  kDebug() << "foo" << colId;

  if (d->m_itemPopulation == ImmediatePopulation)
    return;

  d->m_populatedCols.remove(colId);

  Iterator i(colId);
  QList <qint64> contiguousItems;
  int idx = 0;
  while (i.hasNext())
  {
    qint64 entId = i.next();
    int entityType = entityTypeForInternalIdentifier(entId);
    if ( CollectionType == entityType )
    {
      if ( contiguousItems.size() > 0 )
      {
        // Don't remove subcollections that have been externally populated.
        if (!d->m_populatedCols.contains(entId))
        {
          kDebug() << colId << idx << contiguousItems;
          beginRemoveEntities(colId, idx, idx + contiguousItems.size() -1 );
          foreach(qint64 id, contiguousItems)
          {
            d->m_items.remove( id );
            d->m_childEntities[ colId ].removeOne( id );
          }
          endRemoveEntities();
          purgeCollection( entId );
        }
      }
      idx++;
      contiguousItems.clear();
    } else if ( ItemType == entityType )
    {
      contiguousItems << entId;
    }
  }
  if (contiguousItems.size() > 0)
  {
    kDebug() << colId << idx << contiguousItems;
    beginRemoveEntities(colId, idx, idx + contiguousItems.size() - 1 );
    foreach(qint64 id, contiguousItems)
    {
      Item i = d->m_items.take( id );
      d->m_childEntities[ colId ].removeOne( id );
    }
    endRemoveEntities();
  }
}


#include "clientsideentitystorage.moc"
