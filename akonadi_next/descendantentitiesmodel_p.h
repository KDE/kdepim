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

#ifndef DESCENDANTENTITIESMODELPRIVATE_H
#define DESCENDANTENTITIESMODELPRIVATE_H

#include <akonadi/item.h>
#include "descendantentitiesmodel.h"

namespace Akonadi
{
class ClientSideEntityStorage;
/**
 * @internal
 */
class DescendantEntitiesModelPrivate
{
public:

  DescendantEntitiesModelPrivate( DescendantEntitiesModel *parent );
  DescendantEntitiesModel *q_ptr;

  void collectionChanged( const Akonadi::Collection& );
  void itemChanged( const Item&, const QSet<QByteArray>& );

//   void collectionStatisticsChanged( Collection::Id, const Akonadi::CollectionStatistics& );

  void rowsAboutToBeInserted( Collection::Id colId, int start, int end );
  void rowsAboutToBeRemoved( Collection::Id colId, int start, int end );
  void rowsInserted();
  void rowsRemoved();

  /**
  Sum of all ((great)*grand)?children of colId
  */
  int descendantCount( Collection::Id colId ) const;

  /**
  Sum of entities above stopId. For example, consider the tree:
  @code
  -> Foo       (countEntitiesAbove == 0)
    |-> Bar    (countEntitiesAbove == 1)
  -> Bat       (countEntitiesAbove == 2)
  @endcode
  */
  int countEntitiesAbove( qint64 stopId ) const;

  /**
  Searches below colId for the row^th entity. Sets @p ok to true if the row exists,
  and false if row is greater than the descendantCount of colId.

  Search uses pre-order traversal.
  */
  qint64 findIdForRow( int row, Collection::Id colId, bool *ok ) const;

  EntityUpdateAdapter *entityUpdateAdapter;
  ClientSideEntityStorage *m_clientSideEntityStorage;

  Q_DECLARE_PUBLIC( DescendantEntitiesModel )
};

}

#endif

