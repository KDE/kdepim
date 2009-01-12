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
#include "entitytreemodel.h"

namespace Akonadi
{
class ClientSideEntityStorage;
/**
 * @internal
 */
class EntityTreeModelPrivate
{
public:

  EntityTreeModelPrivate( EntityTreeModel *parent );
  EntityTreeModel *q_ptr;

  void collectionChanged( const Akonadi::Collection& );
  void itemChanged( const Item&, const QSet<QByteArray>& );

//   void collectionStatisticsChanged( Collection::Id, const Akonadi::CollectionStatistics& );

  bool mimetypeMatches( const QStringList &mimetypes, const QStringList &other );

  void rowsAboutToBeInserted( Collection::Id colId, int start, int end );
  void rowsAboutToBeRemoved( Collection::Id colId, int start, int end );
  void rowsInserted();
  void rowsRemoved();

  EntityUpdateAdapter *entityUpdateAdapter;
  ClientSideEntityStorage *m_clientSideEntityStorage;

  Q_DECLARE_PUBLIC( EntityTreeModel )
};

}

#endif

