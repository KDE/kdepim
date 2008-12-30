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

#ifndef ENTITYTREEMODELPRIVATE_H
#define ENTITYTREEMODELPRIVATE_H

#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>
#include "entitytreemodel.h"

namespace Akonadi
{
class Monitor;
class ItemFetchScope;

/**
 * @internal
 */
class EntityTreeModelPrivate
{
public:

  EntityTreeModelPrivate( EntityTreeModel *parent );
  EntityTreeModel *q_ptr;

  void init();

  /**
  Adds items to the model when notified to do so for @p parent.
  */
  void onRowsInserted( const QModelIndex &parent, int start, int end );
  void collectionsReceived( const Akonadi::Collection::List &cols );
  void itemsReceived( const Item::List &list, Collection::Id colId );

  void collectionAdded( const Akonadi::Collection& , const Akonadi::Collection& );
  void collectionChanged( const Akonadi::Collection& );
  void collectionRemoved( const Akonadi::Collection& );

  /**
  Warning: This slot should never be called directly. It should only be connected to by the itemsRetrieved signal
  of the ItemFetchJob.
  */
  void itemAdded( const Item &item, const Collection& );
  void itemChanged( const Item&, const QSet<QByteArray>& );
  void itemRemoved( const Item& );
  void itemMoved( const Item&, const Collection& src, const Collection& dst );

  void collectionStatisticsChanged( Collection::Id, const Akonadi::CollectionStatistics& );

  QModelIndex indexForId( qint64 id, int column = 0 ) const;
  Collection collectionForIndex( QModelIndex idx ) const;
//   Item itemForIndex(QModelIndex idx);
  int childEntitiesCount( const QModelIndex & parent ) const;

  bool passesFilter( const QStringList &mimetypes );
  bool mimetypeMatches( const QStringList &mimetypes, const QStringList &other );

  Collection findParentCollection( qint64 id ) const;
  Collection findParentCollection( Item item ) const;
  void orderChildEntities( Collection col, QStringList );

  QHash< Collection::Id, Collection > collections;
  QHash< qint64, Item > m_items;
  QHash< Collection::Id, QList< qint64 > > m_childEntities;

  QStringList m_mimeTypeFilter;

  Monitor *monitor;
  EntityUpdateAdapter *entityUpdateAdapter;
  ItemFetchScope m_itemFetchScope;
  Collection m_rootCollection;
  bool m_showStats;
  int m_entitiesToFetch;

  QModelIndex indexForItem( Item );

  /**
  Returns true if the index @p index refers to an item.
  */
  bool isItem( const QModelIndex &index ) const;

  /**
  Returns true if the index @p index refers to a collection.
  */
  bool isCollection( const QModelIndex &index ) const;

  Q_DECLARE_PUBLIC( EntityTreeModel )
};

}

#endif

