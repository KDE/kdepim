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

#include <KLocale>
#include <KGlobal>

EventListProxy::EventListProxy(QObject* parent) : ListProxy(parent)
{
  setDynamicSortFilter( true );
  sort( 0, Qt::DescendingOrder );
}

QString durationString( const KCalCore::Incidence::Ptr &incidence )
{
  if ( KCalCore::Event::Ptr event = incidence.dynamicCast<KCalCore::Event>() ) {
    if ( event->allDay() )
      return i18n( "Whole day" );

    if ( event->duration() )
      return KGlobal::locale()->formatDuration( event->duration().asSeconds() * 1000 );
    else
      return KGlobal::locale()->formatDuration( event->dtStart().secsTo( event->dtEnd() ) * 1000 );

  } else if ( KCalCore::Todo::Ptr todo = incidence.dynamicCast<KCalCore::Todo>() ) {
    if ( todo->hasDueDate() )
      return i18n( "Due at %1", KGlobal::locale()->formatDateTime( todo->dtDue(), KLocale::FancyShortDate ) );
    else
      return i18n( "No due date" );
  } else {
    kDebug() << "Invalid incindce type" << incidence->typeStr();
    return QString();
  }
}

QVariant EventListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    const KCalCore::Incidence::Ptr event = item.payload<KCalCore::Incidence::Ptr>();
    switch ( role ) {
      case SummaryRole:
        return event->summary();
      case BeginRole:
        return KGlobal::locale()->formatDateTime( event->dtStart(), KLocale::FancyShortDate );
      case DurationRole:
        return durationString( event );
    }
  }
  return QSortFilterProxyModel::data(index, role);
}

void EventListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  ListProxy::setSourceModel(sourceModel);
  QHash<int, QByteArray> names = roleNames();
  names.insert( Akonadi::EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( SummaryRole, "summary" );
  names.insert( BeginRole, "begin" );
  names.insert( DurationRole, "duration" );
  setRoleNames( names );
  kDebug() << names << sourceModel->roleNames();
}

bool EventListProxy::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !leftItem.hasPayload<KCalCore::Event::Ptr>() || !rightItem.hasPayload<KCalCore::Event::Ptr>() )
    return leftItem.id() < rightItem.id();

  const KCalCore::Event::Ptr leftEvent = leftItem.payload<KCalCore::Event::Ptr>();
  const KCalCore::Event::Ptr rightEvent = rightItem.payload<KCalCore::Event::Ptr>();

  if ( leftEvent->dtStart() == rightEvent->dtStart() )
    return leftItem.id() < rightItem.id();
  return leftEvent->dtStart() < rightEvent->dtStart();
}

#include "eventlistproxy.moc"
