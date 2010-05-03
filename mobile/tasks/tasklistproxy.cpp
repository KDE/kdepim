/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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
#include "tasklistproxy.h"

#include <kcal/todo.h>

#include <akonadi/entitytreemodel.h>

using namespace Akonadi;

TaskListProxy::TaskListProxy( int customRoleBaseline, QObject* parent )
  : ListProxy( parent ),
    mCustomRoleBaseline( customRoleBaseline )
{ }

QVariant TaskListProxy::data( const QModelIndex& index, int role ) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.isValid() && item.hasPayload<KCal::Todo::Ptr>() ) {
    const KCal::Todo::Ptr incidence = item.payload<KCal::Todo::Ptr>();
    switch ( relativeCustomRole( role ) ) {
    case Summary:
      return incidence->summary();
    case Description:
      return incidence->description();
    case PercentComplete:
      return incidence->percentComplete();
    }
  }

  return QSortFilterProxyModel::data(index, role);
}

void TaskListProxy::setSourceModel( QAbstractItemModel* sourceModel )
{
  ListProxy::setSourceModel(sourceModel);

  QHash<int, QByteArray> names = roleNames();
  names.insert( EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( absoluteCustomRole( Summary ), "summary" );
  names.insert( absoluteCustomRole( Description ), "description" );
  names.insert( absoluteCustomRole( PercentComplete ), "percentComplete" );
  setRoleNames( names );
}

