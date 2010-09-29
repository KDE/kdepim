/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "tasksfilterproxymodel.h"

#include <akonadi/entitytreemodel.h>
#include <KCalCore/Todo>

static bool taskMatchesFilter( const KCalCore::Todo::Ptr &task, const QString &filterString );

using namespace Akonadi;

class TasksFilterProxyModel::Private
{
  public:
    QString mFilter;
};

TasksFilterProxyModel::TasksFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ), d( new Private )
{
  setSortLocaleAware( true );
  setDynamicSortFilter( true );
}

TasksFilterProxyModel::~TasksFilterProxyModel()
{
  delete d;
}

void TasksFilterProxyModel::setFilterString( const QString &filter )
{
  d->mFilter = filter;
  invalidateFilter();
}

bool TasksFilterProxyModel::filterAcceptsRow( int row, const QModelIndex &parent ) const
{
  if ( d->mFilter.isEmpty() )
    return true;

  const QModelIndex index = sourceModel()->index( row, 0, parent );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.hasPayload<KCalCore::Todo::Ptr>() ) {
    const KCalCore::Todo::Ptr task = item.payload<KCalCore::Todo::Ptr>();
    return taskMatchesFilter( task, d->mFilter );
  }

  return true;
}

static bool taskMatchesFilter( const KCalCore::Todo::Ptr &task, const QString &filterString )
{
  if ( task->summary().contains( filterString, Qt::CaseInsensitive ) )
    return true;

  if ( task->description().contains( filterString, Qt::CaseInsensitive ) )
    return true;

  return false;
}

#include "tasksfilterproxymodel.moc"
