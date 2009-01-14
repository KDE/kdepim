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

#ifndef CLIENTSIDEENTITYSTORAGE_P_H
#define CLIENTSIDEENTITYSTORAGE_P_H

#include "clientsideentitystorage.h"

#include <KJob>

#include <akonadi/collectionfetchjob.h>
#include <akonadi/itemfetchscope.h>

namespace Akonadi{

class ClientSideEntityStoragePrivate
{
public:
  ClientSideEntityStoragePrivate( ClientSideEntityStorage *storage );

  Q_DECLARE_PUBLIC( ClientSideEntityStorage )
  ClientSideEntityStorage *q_ptr;

  QHash< Collection::Id, Collection > m_collections;
  QHash< qint64, Item > m_items;
  QHash< Collection::Id, QList< qint64 > > m_childEntities;

  Monitor *m_monitor;
  int m_entitiesToFetch;
  Collection m_rootCollection;
  QStringList m_mimeTypeFilter;
  ItemFetchScope m_itemFetchScope;
  bool m_includeUnsubscribed;

  void startFirstListJob();

  void fetchJobDone( KJob *job );
  void updateJobDone( KJob *job );

  bool passesFilter( const QStringList &mimetypes );
  bool mimetypeMatches( const QStringList &mimetypes, const QStringList &other );
  Collection getParentCollection( qint64 id );


  void fetchCollections( Collection col, CollectionFetchJob::Type = CollectionFetchJob::FirstLevel );
  void fetchItems( Collection col );
  void collectionsFetched( const Akonadi::Collection::List& );
  void itemsFetched( const Akonadi::Item::List& );

  void monitoredCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& );
  void monitoredCollectionRemoved( const Akonadi::Collection& );
  void monitoredCollectionChanged( const Akonadi::Collection& );
  void monitoredCollectionStatisticsChanged( Akonadi::Collection::Id, const Akonadi::CollectionStatistics &);
  void monitoredCollectionMoved( const Akonadi::Collection&, const Akonadi::Collection&, const Akonadi::Collection&);
  void monitoredItemAdded( const Akonadi::Item&, const Akonadi::Collection& );
  void monitoredItemRemoved( const Akonadi::Item& );
  void monitoredItemChanged( const Akonadi::Item&, const QSet<QByteArray>& );
  void monitoredItemMoved( const Akonadi::Item&, const Akonadi::Collection&, const Akonadi::Collection& );

  /**
  The id of the collection which starts an item fetch job. This is part of a hack with QObject::sender
  in itemsReceivedFromJob to correctly insert items into the model.
  */
  static QByteArray ItemFetchCollectionId() {
    return "ItemFetchCollectionId";
  }

};

}

#endif