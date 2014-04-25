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

#include "eventsfilterproxymodel.h"

#include <AkonadiCore/entitytreemodel.h>
#include <KCalCore/Event>

static bool eventMatchesFilter( const KCalCore::Event::Ptr &event, const QString &filterString );

using namespace Akonadi;

class EventsFilterProxyModel::Private
{
  public:
    QString mFilter;
};

EventsFilterProxyModel::EventsFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ), d( new Private )
{
  setSortLocaleAware( true );
  setDynamicSortFilter( true );
}

EventsFilterProxyModel::~EventsFilterProxyModel()
{
  delete d;
}

void EventsFilterProxyModel::setFilterString( const QString &filter )
{
  d->mFilter = filter;
  invalidateFilter();
}

bool EventsFilterProxyModel::filterAcceptsRow( int row, const QModelIndex &parent ) const
{
  if ( d->mFilter.isEmpty() )
    return true;

  const QModelIndex index = sourceModel()->index( row, 0, parent );

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( item.hasPayload<KCalCore::Event::Ptr>() ) {
    const KCalCore::Event::Ptr event = item.payload<KCalCore::Event::Ptr>();
    return eventMatchesFilter( event, d->mFilter );
  }

  return true;
}

static bool eventMatchesFilter( const KCalCore::Event::Ptr &event, const QString &filterString )
{
  if ( event->summary().contains( filterString, Qt::CaseInsensitive ) )
    return true;

  if ( event->description().contains( filterString, Qt::CaseInsensitive ) )
    return true;

  return false;
}

