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

#include "eventlistproxy.h"

#include <akonadi/item.h>

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalUtils/IncidenceFormatter>

#include <KGlobal>
#include <KLocale>
#include <KSystemTimeZones>

#include <QItemSelection>

EventListProxy::EventListProxy(QObject* parent) : ListProxy(parent)
{
  setDynamicSortFilter( true );
  sort( 0, Qt::DescendingOrder );

  mCurrentDateTimeReference = QDateTime::currentMSecsSinceEpoch();
  mToday = KDateTime::currentLocalDateTime();
}

QVariant EventListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    const KCalCore::Incidence::Ptr incidence = item.payload<KCalCore::Incidence::Ptr>();
    switch ( role ) {
      case SummaryRole:
        return incidence->summary();
      case BeginRole:
        return KGlobal::locale()->formatDateTime( incidence->dtStart(), KLocale::FancyShortDate );
      case DurationRole:
        return KCalUtils::IncidenceFormatter::durationString( incidence );
    }
  }
  return QSortFilterProxyModel::data(index, role);
}

void EventListProxy::setSourceModel( QAbstractItemModel *model )
{
  if ( sourceModel() )
    disconnect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(dataChanged(QModelIndex,QModelIndex)) );

  ListProxy::setSourceModel( model );

  connect( sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           this, SLOT(dataChanged(QModelIndex,QModelIndex)) );

  QHash<int, QByteArray> names = roleNames();
  names.insert( Akonadi::EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( SummaryRole, "summary" );
  names.insert( BeginRole, "begin" );
  names.insert( DurationRole, "duration" );
  setRoleNames( names );
}

KDateTime EventListProxy::startDateTimeForItem( const Akonadi::Item &item ) const
{
  const QHash<Akonadi::Item::Id, DateTimeHashEntry>::const_iterator it = mDateTimeHash.constFind( item.id() );
  if ( it != mDateTimeHash.constEnd() )
    return (*it).startDateTime;

  const KCalCore::Event::Ptr event = item.payload<KCalCore::Event::Ptr>();

  DateTimeHashEntry entry;
  entry.startDateTime = event->dtStart();
  entry.endDateTime = event->dtEnd();

  mDateTimeHash.insert( item.id(), entry );

  return entry.startDateTime;
}

KDateTime EventListProxy::endDateTimeForItem( const Akonadi::Item &item ) const
{
  const QHash<Akonadi::Item::Id, DateTimeHashEntry>::const_iterator it = mDateTimeHash.constFind( item.id() );
  if ( it != mDateTimeHash.constEnd() )
    return (*it).endDateTime;

  const KCalCore::Event::Ptr event = item.payload<KCalCore::Event::Ptr>();

  DateTimeHashEntry entry;
  entry.startDateTime = event->dtStart();
  entry.endDateTime = event->dtEnd();

  mDateTimeHash.insert( item.id(), entry );

  return entry.endDateTime;
}

bool EventListProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !leftItem.hasPayload<KCalCore::Event::Ptr>() || !rightItem.hasPayload<KCalCore::Event::Ptr>() )
    return leftItem.id() < rightItem.id();

  const KDateTime leftDateTimeStart = startDateTimeForItem( leftItem );
  const KDateTime leftDateTimeEnd = endDateTimeForItem( leftItem );
  const KDateTime rightDateTimeStart = startDateTimeForItem( rightItem );
  const KDateTime rightDateTimeEnd = endDateTimeForItem( rightItem );

  if ( leftDateTimeStart == rightDateTimeStart )
    return leftItem.id() < rightItem.id();

  if ( (mCurrentDateTimeReference + 60000) < QDateTime::currentMSecsSinceEpoch() ) {
    mCurrentDateTimeReference = QDateTime::currentMSecsSinceEpoch();
    mToday = KDateTime::currentLocalDateTime();
  }

  const bool leftIsFuture = (leftDateTimeEnd >= mToday);
  const bool rightIsFuture = (rightDateTimeEnd >= mToday);

  if ( leftIsFuture != rightIsFuture ) {
    return !leftIsFuture;
  }

  if ( leftIsFuture )
    return leftDateTimeStart > rightDateTimeStart;
  else
    return leftDateTimeStart < rightDateTimeStart;
}

void EventListProxy::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  QItemSelection selection;
  selection.select( topLeft, bottomRight );

  foreach ( const QModelIndex &index, selection.indexes() ) {
    const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( !item.hasPayload<KCalCore::Event::Ptr>() )
      continue;

    const KCalCore::Event::Ptr event = item.payload<KCalCore::Event::Ptr>();

    DateTimeHashEntry entry;
    entry.startDateTime = event->dtStart();
    entry.endDateTime = event->dtEnd();

    mDateTimeHash.insert( item.id(), entry );
  }
}

