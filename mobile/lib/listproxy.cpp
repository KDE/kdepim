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

#include "listproxy.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/item.h>

ListProxy::ListProxy( QObject* parent ) : QSortFilterProxyModel( parent )
{
}

qint64 ListProxy::itemId( int row ) const
{
  if ( row < 0 || row >= rowCount() )
    return -1;

  QModelIndex idx = index( row, 0 );
  if ( !idx.isValid() )
    return -1;

  const Akonadi::Item item = QSortFilterProxyModel::data( idx, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  return item.id();
}

void ListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  QSortFilterProxyModel::setSourceModel(sourceModel);
}


