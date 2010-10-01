/*
  Copyright (C) 2009 KDAB (author: Frank Osterfeld <osterfeld@kde.org>)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "utils.h"
#include "kcalprefs.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionDialog>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>

#include <KHolidays/Holidays>

#include <KCalCore/CalFilter>
#include <KCalCore/Event>
#include <KCalCore/FreeBusy>
#include <KCalCore/Incidence>
#include <KCalCore/Journal>
#include <KCalCore/MemoryCalendar>
#include <KCalCore/Todo>

#include <KCalUtils/DndFactory>
#include <KCalUtils/ICalDrag>
#include <KCalUtils/VCalDrag>

#include <KIconLoader>
#include <KUrl>

#include <QAbstractItemModel>
#include <QDrag>
#include <QMimeData>
#include <QModelIndex>
#include <QPixmap>
#include <QPointer>

#include <boost/bind.hpp>

using namespace CalendarSupport;
using namespace KHolidays;

KCalCore::Incidence::Ptr CalendarSupport::incidence( const Akonadi::Item &item )
{
  return
    item.hasPayload<KCalCore::Incidence::Ptr>() ?
    item.payload<KCalCore::Incidence::Ptr>() :
    KCalCore::Incidence::Ptr();
}

KCalCore::Event::Ptr CalendarSupport::event( const Akonadi::Item &item )
{
  return
    item.hasPayload<KCalCore::Event::Ptr>() ?
    item.payload<KCalCore::Event::Ptr>() :
    KCalCore::Event::Ptr();
}

KCalCore::Event::List CalendarSupport::eventsFromItems( const Akonadi::Item::List &items )
{
  KCalCore::Event::List events;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    if ( const KCalCore::Event::Ptr e = CalendarSupport::event( item ) ) {
      events.push_back( e );
    }
  }
  return events;
}

KCalCore::Incidence::List CalendarSupport::incidencesFromItems( const Akonadi::Item::List &items )
{
  KCalCore::Incidence::List incidences;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    if ( const KCalCore::Incidence::Ptr e = CalendarSupport::incidence( item ) ) {
      incidences.push_back( e );
    }
  }
  return incidences;
}

KCalCore::Todo::Ptr CalendarSupport::todo( const Akonadi::Item &item )
{
  return
    item.hasPayload<KCalCore::Todo::Ptr>() ?
    item.payload<KCalCore::Todo::Ptr>() :
    KCalCore::Todo::Ptr();
}

KCalCore::Journal::Ptr CalendarSupport::journal( const Akonadi::Item &item )
{
  return
    item.hasPayload<KCalCore::Journal::Ptr>() ?
    item.payload<KCalCore::Journal::Ptr>() :
    KCalCore::Journal::Ptr();
}

bool CalendarSupport::hasIncidence( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Incidence::Ptr>();
}

bool CalendarSupport::hasEvent( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Event::Ptr>();
}

bool CalendarSupport::hasTodo( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Todo::Ptr>();
}

bool CalendarSupport::hasJournal( const Akonadi::Item &item )
{
  return item.hasPayload<KCalCore::Journal::Ptr>();
}

QMimeData *CalendarSupport::createMimeData( const Akonadi::Item::List &items,
                                            const KDateTime::Spec &timeSpec )
{
  if ( items.isEmpty() ) {
    return 0;
  }

  KCalCore::MemoryCalendar::Ptr cal( new KCalCore::MemoryCalendar( timeSpec ) );

  QList<QUrl> urls;
  int incidencesFound = 0;
  Q_FOREACH ( const Akonadi::Item &item, items ) {
    const KCalCore::Incidence::Ptr incidence( CalendarSupport::incidence( item ) );
    if ( !incidence ) {
      continue;
    }
    ++incidencesFound;
    urls.push_back( item.url() );
    KCalCore::Incidence::Ptr i( incidence->clone() );
    cal->addIncidence( i );
  }

  if ( incidencesFound == 0 ) {
    return 0;
  }

  std::auto_ptr<QMimeData> mimeData( new QMimeData );

  mimeData->setUrls( urls );

  KCalUtils::ICalDrag::populateMimeData( mimeData.get(), cal );
  KCalUtils::VCalDrag::populateMimeData( mimeData.get(), cal );

  return mimeData.release();
}

QMimeData *CalendarSupport::createMimeData( const Akonadi::Item &item,
                                            const KDateTime::Spec &timeSpec )
{
  return createMimeData( Akonadi::Item::List() << item, timeSpec );
}

#ifndef QT_NO_DRAGANDDROP
QDrag *CalendarSupport::createDrag( const Akonadi::Item &item,
                                    const KDateTime::Spec &timeSpec, QWidget *parent )
{
  return createDrag( Akonadi::Item::List() << item, timeSpec, parent );
}
#endif

static QByteArray findMostCommonType( const Akonadi::Item::List &items )
{
  QByteArray prev;
  if ( items.isEmpty() ) {
    return "Incidence";
  }

  Q_FOREACH( const Akonadi::Item &item, items ) {
    if ( !CalendarSupport::hasIncidence( item ) ) {
      continue;
    }
    const QByteArray type = CalendarSupport::incidence( item )->typeStr();
    if ( !prev.isEmpty() && type != prev ) {
      return "Incidence";
    }
    prev = type;
  }
  return prev;
}

#ifndef QT_NO_DRAGANDDROP
QDrag *CalendarSupport::createDrag( const Akonadi::Item::List &items,
                                    const KDateTime::Spec &timeSpec, QWidget *parent )
{
  std::auto_ptr<QDrag> drag( new QDrag( parent ) );
  drag->setMimeData( CalendarSupport::createMimeData( items, timeSpec ) );

  const QByteArray common = findMostCommonType( items );
  if ( common == "Event" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-day" ) ) );
  } else if ( common == "Todo" ) {
    drag->setPixmap( BarIcon( QLatin1String( "view-calendar-tasks" ) ) );
  }

  return drag.release();
}
#endif

static bool itemMatches( const Akonadi::Item &item, const KCalCore::CalFilter *filter )
{
  assert( filter );
  KCalCore::Incidence::Ptr inc = CalendarSupport::incidence( item );
  if ( !inc ) {
    return false;
  }
  return filter->filterIncidence( inc );
}

Akonadi::Item::List CalendarSupport::applyCalFilter( const Akonadi::Item::List &items_,
                                                     const KCalCore::CalFilter *filter )
{
  Q_ASSERT( filter );
  Akonadi::Item::List items( items_ );
  items.erase( std::remove_if( items.begin(), items.end(),
                               !bind( itemMatches, _1, filter ) ), items.end() );
  return items;
}

bool CalendarSupport::isValidIncidenceItemUrl( const KUrl &url,
                                               const QStringList &supportedMimeTypes )
{
  if ( !url.isValid() ) {
    return false;
  }

  if ( url.scheme() != QLatin1String( "akonadi" ) ) {
    return false;
  }

  return supportedMimeTypes.contains( url.queryItem( QLatin1String( "type" ) ) );
}

bool CalendarSupport::isValidIncidenceItemUrl( const KUrl &url )
{
  return isValidIncidenceItemUrl( url,
                                  QStringList() << KCalCore::Event::eventMimeType()
                                                << KCalCore::Todo::todoMimeType()
                                                << KCalCore::Journal::journalMimeType()
                                                << KCalCore::FreeBusy::freeBusyMimeType() );
}

static bool containsValidIncidenceItemUrl( const QList<QUrl>& urls )
{
  return
    std::find_if( urls.begin(), urls.end(),
                  bind( CalendarSupport::isValidIncidenceItemUrl, _1 ) ) != urls.constEnd();
}

bool CalendarSupport::isValidTodoItemUrl( const KUrl &url )
{
  if ( !url.isValid() || url.scheme() != QLatin1String( "akonadi" ) ) {
    return false;
  }

  return url.queryItem( QLatin1String( "type" ) ) == KCalCore::Todo::todoMimeType();
}

bool CalendarSupport::canDecode( const QMimeData *md )
{
  Q_ASSERT( md );
  return
    containsValidIncidenceItemUrl( md->urls() ) ||
    KCalUtils::ICalDrag::canDecode( md ) ||
    KCalUtils::VCalDrag::canDecode( md );
}

QList<KUrl> CalendarSupport::incidenceItemUrls( const QMimeData *mimeData )
{
  QList<KUrl> urls;
  Q_FOREACH( const KUrl &i, mimeData->urls() ) {
    if ( isValidIncidenceItemUrl( i ) ) {
      urls.push_back( i );
    }
  }
  return urls;
}

QList<KUrl> CalendarSupport::todoItemUrls( const QMimeData *mimeData )
{
  QList<KUrl> urls;

  Q_FOREACH( const KUrl &i, mimeData->urls() ) {
    if ( isValidIncidenceItemUrl( i, QStringList() << KCalCore::Todo::todoMimeType() ) ) {
      urls.push_back( i );
    }
  }
  return urls;
}

bool CalendarSupport::mimeDataHasTodo( const QMimeData *mimeData )
{
  return !todoItemUrls( mimeData ).isEmpty() || !todos( mimeData, KDateTime::Spec() ).isEmpty();
}

KCalCore::Todo::List CalendarSupport::todos( const QMimeData *mimeData,
                                             const KDateTime::Spec &spec )
{
  KCalCore::Todo::List todos;

#ifndef QT_NO_DRAGANDDROP
  KCalCore::Calendar::Ptr cal( KCalUtils::DndFactory::createDropCalendar( mimeData, spec ) );
  if ( cal ) {
    Q_FOREACH( const KCalCore::Todo::Ptr &i, cal->todos() ) {
      todos.push_back( KCalCore::Todo::Ptr( i->clone() ) );
    }
  }
#endif

  return todos;
}

Akonadi::Collection CalendarSupport::selectCollection( QWidget *parent,
                                                       int &dialogCode,
                                                       const QStringList &mimeTypes,
                                                       const Akonadi::Collection &defCollection )
{
  QPointer<Akonadi::CollectionDialog> dlg( new Akonadi::CollectionDialog( parent ) );

  kDebug() << "selecting collections with mimeType in " << mimeTypes;

  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  if ( defCollection.isValid() ) {
    dlg->setDefaultCollection( defCollection );
  }
  Akonadi::Collection collection;

  dialogCode = dlg->exec();
  if ( dialogCode == QDialog::Accepted ) {
    collection = dlg->selectedCollection();
  }
  delete dlg;
  return collection;
}

Akonadi::Item CalendarSupport::itemFromIndex( const QModelIndex &idx )
{
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  item.setParentCollection(
    idx.data( Akonadi::EntityTreeModel::ParentCollectionRole ).value<Akonadi::Collection>() );
  return item;
}

Akonadi::Collection::List CalendarSupport::collectionsFromModel( const QAbstractItemModel *model,
                                                                 const QModelIndex &parentIndex,
                                                                 int start, int end )
{
  const int endRow = end >= 0 ? end : model->rowCount( parentIndex ) - 1;
  Akonadi::Collection::List collections;
  int row = start;
  QModelIndex i = model->index( row, 0, parentIndex );
  while ( row <= endRow ) {
    const Akonadi::Collection collection = collectionFromIndex( i );
    if ( collection.isValid() ) {
      collections << collection;
      QModelIndex childIndex = i.child( 0, 0 );
      if ( childIndex.isValid() ) {
        collections << collectionsFromModel( model, i );
      }
    }
    ++row;
    i = i.sibling( row, 0 );
  }
  return collections;
}

Akonadi::Item::List CalendarSupport::itemsFromModel( const QAbstractItemModel * model,
                                                     const QModelIndex &parentIndex,
                                                     int start, int end )
{
  const int endRow = end >= 0 ? end : model->rowCount( parentIndex ) - 1;
  Akonadi::Item::List items;
  int row = start;
  QModelIndex i = model->index( row, 0, parentIndex );
  while ( row <= endRow ) {
    const Akonadi::Item item = itemFromIndex( i );
    if ( CalendarSupport::hasIncidence( item ) ) {
      items << item;
    } else {
      QModelIndex childIndex = i.child( 0, 0 );
      if ( childIndex.isValid() ) {
        items << itemsFromModel( model, i );
      }
    }

    ++row;
    i = i.sibling( row, 0 );
  }
  return items;
}

Akonadi::Collection CalendarSupport::collectionFromIndex( const QModelIndex &index )
{
  return index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

Akonadi::Collection::Id CalendarSupport::collectionIdFromIndex( const QModelIndex &index )
{
  return index.data( Akonadi::EntityTreeModel::CollectionIdRole ).value<Akonadi::Collection::Id>();
}

Akonadi::Collection::List CalendarSupport::collectionsFromIndexes( const QModelIndexList &indexes )
{
  Akonadi::Collection::List l;
  Q_FOREACH( const QModelIndex &idx, indexes ) {
    l.push_back( collectionFromIndex( idx ) );
  }
  return l;
}

QString CalendarSupport::displayName( const Akonadi::Collection &c )
{
  const Akonadi::EntityDisplayAttribute *attr = c.attribute<Akonadi::EntityDisplayAttribute>();
  return ( attr && !attr->displayName().isEmpty() ) ? attr->displayName() : c.name();
}

QString CalendarSupport::subMimeTypeForIncidence( const KCalCore::Incidence::Ptr &incidence )
{
  return incidence->mimeType();
}

QList<QDate> CalendarSupport::workDays( const QDate &startDate,
                                        const QDate &endDate )
{
  QList<QDate> result;

  const int mask( ~( KCalPrefs::instance()->mWorkWeekMask ) );
  const int numDays = startDate.daysTo( endDate ) + 1;

  for ( int i = 0; i < numDays; ++i ) {
    const QDate date = startDate.addDays( i );
    if ( !( mask & ( 1 << ( date.dayOfWeek() - 1 ) ) ) ) {
      result.append( date );
    }
  }

  if ( KCalPrefs::instance()->mExcludeHolidays ) {
    // NOTE: KOGlobals, where this method comes from, used to hold a pointer to
    //       a KHolidays object. I'm not sure about how expensive it is, just
    //       creating one here.
    const HolidayRegion holidays( KCalPrefs::instance()->mHolidays );
    const Holiday::List list = holidays.holidays( startDate, endDate );
    for ( int i = 0; i < list.count(); ++i ) {
      const Holiday &h = list.at( i );
      const QString dateString = h.date().toString();
      if ( h.dayType() == Holiday::NonWorkday ) {
        result.removeAll( h.date() );
      }
    }
  }

  return result;
}

QStringList CalendarSupport::holiday( const QDate &date )
{
  QStringList hdays;

  const HolidayRegion holidays( KCalPrefs::instance()->mHolidays );
  const Holiday::List list = holidays.holidays( date );

  for ( int i = 0; i < list.count(); ++i ) {
    hdays.append( list.at( i ).text() );
  }
  return hdays;
}
