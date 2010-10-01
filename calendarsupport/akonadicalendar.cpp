/*
  This file is part of the kcalcore library.

  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  Author: Sérgio Martins <iamsergio@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "akonadicalendar.h"
#include "akonadicalendar_p.h"
#include "utils.h"

#include <KLocale>

using namespace CalendarSupport;
using namespace KCalCore;

AkonadiCalendar::Private::Private( AkonadiCalendar *qq ) : mCalendarModel( 0 ), q( qq )
{
  connect( mCalendarModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           SLOT(dataChanged(QModelIndex,QModelIndex)) );

  connect( mCalendarModel, SIGNAL(layoutChanged()),
           SLOT(layoutChanged()) );

  connect( mCalendarModel, SIGNAL(modelReset()),
           SLOT(modelReset()) );

  connect( mCalendarModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           SLOT(rowsInserted(QModelIndex,int,int)) );

  connect( mCalendarModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
           SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)) );

    // user information...
  q->owner()->setName( i18n( "Unknown Name" ) );
  q->owner()->setEmail( i18n( "unknown@nowhere" ) );

}

AkonadiCalendar::Private::~Private()
{
  // Todo, use the incidence map
  foreach( const Akonadi::Item &item, m_itemMap ) {
    CalendarSupport::incidence( item )->unRegisterObserver( q );
  }
}

void AkonadiCalendar::Private::load()
{
  if ( !mCalendarModel ) {
    Akonadi::Session *session = new Akonadi::Session( "CalendarETM", this );
    Akonadi::ChangeRecorder *monitor = new Akonadi::ChangeRecorder( this );
    connect( monitor,
             SIGNAL(collectionChanged(Akonadi::Collection,QSet<QByteArray>) ),
             SLOT(slotCollectionChanged(Akonadi::Collection,QSet<QByteArray>) ) );

    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    monitor->setSession( session );
    monitor->setCollectionMonitored( Akonadi::Collection::root() );
    monitor->fetchCollection( true );
    monitor->setItemFetchScope( scope );
    monitor->setMimeTypeMonitored( KCalCore::Event::eventMimeType(), true );
    monitor->setMimeTypeMonitored( KCalCore::Todo::todoMimeType(), true );
    monitor->setMimeTypeMonitored( KCalCore::Journal::journalMimeType(), true );
    mCalendarModel = new CalendarSupport::CalendarModel( monitor, this );
  }
}

Alarm::List AkonadiCalendar::Private::alarmsTo( const KDateTime &to )
{
  return q->alarms( KDateTime( QDate( 1900, 1, 1 ) ), to );
}

void AkonadiCalendar::Private::itemChanged( const Akonadi::Item &item )
{
  kDebug() << "item changed: " << item.id();

  Q_ASSERT( item.isValid() );
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( !incidence ) {
    return;
  }
  updateItem( item, AssertExists );
  emit q->calendarChanged();

}

void AkonadiCalendar::Private::itemsAdded( const Akonadi::Item::List &items )
{
  kDebug() << "adding items: " << items.count();

  foreach ( const Akonadi::Item &item, items ) {
    Q_ASSERT( item.isValid() );
    if ( !hasIncidence( item ) ) {
      continue;
    }
    updateItem( item, AssertNew );
    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  }
  emit q->calendarChanged();
}

void AkonadiCalendar::Private::itemsRemoved( const Akonadi::Item::List &items )
{
  foreach ( const Akonadi::Item &item, items ) {
    Q_ASSERT( item.isValid() );
    Akonadi::Item ci( m_itemMap.take( item.id() ) );

    removeItemFromMaps( ci );

    kDebug() << item.id();
    Q_ASSERT( ci.hasPayload<KCalCore::Incidence::Ptr>() );
    const KCalCore::Incidence::Ptr incidence = ci.payload<KCalCore::Incidence::Ptr>();
    kDebug() << "Remove uid=" << incidence->uid()
             << "summary=" << incidence->summary()
             << "type=" << int( incidence->type() )
             << "; id= " << item.id() << "; revision=" << item.revision()
             << " calendar = "
             << q;

    if ( const KCalCore::Event::Ptr e = incidence.dynamicCast<KCalCore::Event>() ) {
      if ( !e->recurs() ) {
        m_itemIdsForDate.remove( e->dtStart().date().toString(), item.id() );
      }
    } else if ( const KCalCore::Todo::Ptr t = incidence.dynamicCast<KCalCore::Todo>( ) ) {
      if ( t->hasDueDate() ) {
        m_itemIdsForDate.remove( t->dtDue().date().toString(), item.id() );
      }
    } else if ( const KCalCore::Journal::Ptr j = incidence.dynamicCast<KCalCore::Journal>() ) {
      m_itemIdsForDate.remove( j->dtStart().date().toString(), item.id() );
    } else {
      Q_ASSERT( false );
      continue;
    }

    incidence->unRegisterObserver( q );
    q->notifyIncidenceDeleted( incidence );
  }
  emit q->calendarChanged();
}


void AkonadiCalendar::Private::dataChanged( const QModelIndex &topLeft,
                                            const QModelIndex &bottomRight )
{
  Q_ASSERT( topLeft.row() <= bottomRight.row() );
  const int endRow = bottomRight.row();
  QModelIndex i( topLeft );
  int row = i.row();
  while ( row <= endRow ) {
    const Akonadi::Item item = itemFromIndex( i );
    if ( item.isValid() ) {
      updateItem( item, AssertExists );
    }
    ++row;
    i = i.sibling( row, topLeft.column() );
  }
  emit q->calendarChanged();
}

void AkonadiCalendar::Private::removeItemFromMaps( const Akonadi::Item &item )
{
  UnseenItem2 unseen_item;
  UnseenItem2 unseen_parent;

  unseen_item.collection = unseen_parent.collection = item.storageCollectionId();

  unseen_item.uid   = CalendarSupport::incidence( item )->uid();
  unseen_parent.uid = CalendarSupport::incidence( item )->relatedTo();

  if ( m_childToParent.contains( item.id() ) ) {
    Akonadi::Item::Id parentId = m_childToParent.take( item.id() );
    m_parentToChildren[parentId].removeAll( item.id() );
  }

  foreach ( const Akonadi::Item::Id &id, m_parentToChildren[item.id()] ) {
    m_childToUnseenParent[id] = unseen_item;
    m_unseenParentToChildren[unseen_item].push_back( id );
  }

  m_parentToChildren.remove( item.id() );

  m_childToUnseenParent.remove( item.id() );

  m_unseenParentToChildren[unseen_parent].removeAll( item.id() );

  mUnseenToItemId.remove( unseen_item );
}

void AkonadiCalendar::Private::updateItem( const Akonadi::Item &item, UpdateMode mode )
{
  const bool alreadyExisted = m_itemMap.contains( item.id() );
  const Akonadi::Item::Id id = item.id();

  kDebug() << "id=" << item.id()
           << "version=" << item.revision()
           << "alreadyExisted=" << alreadyExisted
           << "; calendar = " << q;
  Q_ASSERT( mode == DontCare || alreadyExisted == ( mode == AssertExists ) );

  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  Q_ASSERT( incidence );

  if ( alreadyExisted ) {

    if ( !m_itemMap.contains( id ) ) {
      // Item was deleted almost at the same time the change was made
      // ignore this change
      return;
    }

    if ( item.storageCollectionId() == -1 ) {
      // A valid item can have an invalid storage id if it was deleted while
      // fetching the ancestor
      return;
    }

    if ( item.storageCollectionId() != m_itemMap.value( id ).storageCollectionId() ) {
      // there was once a bug that resulted in items forget their collectionId...
      kDebug() << "item.storageCollectionId() = " << item.storageCollectionId()
               << "; m_itemMap.value( id ).storageCollectionId() = "
               << m_itemMap.value( id ).storageCollectionId()
               << "; item.isValid() = " << item.isValid()
               << "; calendar = " << q;
      Q_ASSERT_X( false, "updateItem", "updated item has different collection id" );
    }
    // update-only goes here
  } else {
    // new-only goes here
    const Akonadi::Collection::Rights rights = item.parentCollection().rights();
    if ( !( rights & Akonadi::Collection::CanDeleteItem ) &&
         !( rights & Akonadi::Collection::CanChangeItem ) &&
         !incidence->isReadOnly() ) {
      incidence->setReadOnly( true );
    }
  }

  if ( alreadyExisted && m_itemDateForItemId.contains( item.id() ) ) {
    // for changed items, we must remove existing date entries (they might have changed)
    m_itemIdsForDate.remove( m_itemDateForItemId[item.id()], item.id() );
    m_itemDateForItemId.remove( item.id() );
  }

  QString date;
  if ( const KCalCore::Todo::Ptr t = CalendarSupport::todo( item ) ) {
    if ( t->hasDueDate() ) {
      date = t->dtDue().date().toString();
    }
  } else if ( const KCalCore::Event::Ptr e = CalendarSupport::event( item ) ) {
    if ( !e->recurs() && !e->isMultiDay() ) {
      date = e->dtStart().date().toString();
    }
  } else if ( const KCalCore::Journal::Ptr j = CalendarSupport::journal( item ) ) {
    date = j->dtStart().date().toString();
  } else {
    kDebug() << "Item id is " << item.id()
             << item.hasPayload<KCalCore::Incidence::Ptr>()
             << item.hasPayload<KCalCore::Event::Ptr>()
             << item.hasPayload<KCalCore::Todo::Ptr>()
             << item.hasPayload<KCalCore::Journal::Ptr>();
    KCalCore::Incidence::Ptr p = CalendarSupport::incidence( item );
    if ( p ) {
      kDebug() << "incidence uid is " << p->uid()
               << " and type is " << p->typeStr();
    }

    Q_ASSERT( false );
    return;
  }

  if ( !m_itemIdsForDate.contains( date, item.id() ) && !date.isEmpty() ) {
    m_itemIdsForDate.insert( date, item.id() );
    m_itemDateForItemId.insert( item.id(), date );
  }

  m_itemMap.insert( id, item );

  UnseenItem2 ui;
  ui.collection = item.storageCollectionId();
  ui.uid = incidence->uid();

  //REVIEW(AKONADI_PORT)
  //UIDs might be duplicated and thus not unique, so for now we assume that the relatedTo
  // UID refers to an item in the same collection.
  //this might break with virtual collections, so we might fall back to a global UID
  //to akonadi item mapping, and pick just any item (or the first found, or whatever
  //strategy makes sense) from the ones with the same UID
  const QString parentUID = incidence->relatedTo();
  const bool hasParent = !parentUID.isEmpty();
  UnseenItem2 parentItem;
  QMap<UnseenItem2,Akonadi::Item::Id>::const_iterator parentIt = mUnseenToItemId.constEnd();
  bool knowParent = false;
  bool parentNotChanged = false;
  if ( hasParent ) {
    parentItem.collection = item.storageCollectionId();
    parentItem.uid = parentUID;
    parentIt = mUnseenToItemId.constFind( parentItem );
    knowParent = parentIt != mUnseenToItemId.constEnd();
  }

  if ( alreadyExisted ) {
    const bool existedInUidMap = mUnseenToItemId.contains( ui );
    if ( mUnseenToItemId.value( ui ) != item.id() ) {
      kDebug()<< "item.id() = " << item.id() << "; cached id = " << mUnseenToItemId.value( ui )
              << "; item uid = "  << ui.uid
              << "; calendar = " << q
              << "; existed in cache = " << existedInUidMap;
      Q_ASSERT_X( false, "updateItem", "uidToId map disagrees with item id" );
    }

    QHash<Akonadi::Item::Id,Akonadi::Item::Id>::Iterator oldParentIt = m_childToParent.find( id );
    if ( oldParentIt != m_childToParent.end() ) {
      const KCalCore::Incidence::Ptr parentInc =
        CalendarSupport::incidence( m_itemMap.value( oldParentIt.value() ) );
      Q_ASSERT( parentInc );
      if ( parentInc->uid() != parentUID ) {
        //parent changed, remove old entries
        QList<Akonadi::Item::Id>& l = m_parentToChildren[oldParentIt.value()];
        l.removeAll( id );
        m_childToParent.remove( id );
      } else {
        parentNotChanged = true;
      }
    } else { //old parent not seen, maybe unseen?
      QHash<Akonadi::Item::Id,UnseenItem2>::Iterator oldUnseenParentIt =
        m_childToUnseenParent.find( id );
      if ( oldUnseenParentIt != m_childToUnseenParent.end() ) {
        if ( oldUnseenParentIt.value().uid != parentUID ) {
          //parent changed, remove old entries
          QList<Akonadi::Item::Id>& l = m_unseenParentToChildren[oldUnseenParentIt.value()];
          l.removeAll( id );
          m_childToUnseenParent.remove( id );
        } else {
          parentNotChanged = true;
        }
      }
    }

  } else {
    mUnseenToItemId.insert( ui, item.id() );

    //check for already known children:
    const QList<Akonadi::Item::Id> orphanedChildren = m_unseenParentToChildren.value( ui );
    if ( !orphanedChildren.isEmpty() ) {
      m_parentToChildren.insert( id, orphanedChildren );
    }

    Q_FOREACH ( const Akonadi::Item::Id &cid, orphanedChildren ) {
      m_childToParent.insert( cid, id );
    }

    m_unseenParentToChildren.remove( ui );
    m_childToUnseenParent.remove( id );
  }

  if ( hasParent && !parentNotChanged ) {
    if ( knowParent ) {
      Q_ASSERT( !m_parentToChildren.value( parentIt.value() ).contains( id ) );
      const KCalCore::Incidence::Ptr parentInc =
        CalendarSupport::incidence( m_itemMap.value( parentIt.value() ) );
      Q_ASSERT( parentInc );
      m_parentToChildren[parentIt.value()].append( id );
      m_childToParent.insert( id, parentIt.value() );
    } else {
      m_childToUnseenParent.insert( id, parentItem );
      m_unseenParentToChildren[parentItem].append( id );
    }
  }

  if ( !alreadyExisted ) {
    incidence->registerObserver( q );
    q->notifyIncidenceAdded( incidence );
  } else {
    q->notifyIncidenceChanged( incidence );
  }
}

void AkonadiCalendar::Private::readFromModel()
{
  itemsAdded( itemsFromModel( mCalendarModel ) );
}

void AkonadiCalendar::Private::clear()
{
  itemsRemoved( m_itemMap.values() );
  Q_ASSERT( m_itemMap.isEmpty() );
  m_childToParent.clear();
  m_parentToChildren.clear();
  m_childToUnseenParent.clear();
  m_unseenParentToChildren.clear();
  m_itemIdsForDate.clear();
}


void AkonadiCalendar::Private::rowsInserted( const QModelIndex &parent,
                                             int start, int end )
{
  itemsAdded( itemsFromModel( mCalendarModel, parent, start, end ) );
}

void AkonadiCalendar::Private::rowsAboutToBeRemoved( const QModelIndex &parent,
                                                     int start, int end )
{
  itemsRemoved( itemsFromModel( mCalendarModel, parent, start, end ) );
}

void AkonadiCalendar::Private::layoutChanged()
{
  kDebug();
}

void AkonadiCalendar::Private::modelReset()
{
  kDebug();
  clear();
  readFromModel();
}


AkonadiCalendar::AkonadiCalendar( const KDateTime::Spec &timeSpec )
  : KCalCore::Calendar( timeSpec ), d( new Private( this ) )
{

}

AkonadiCalendar::AkonadiCalendar( const QString &timeZoneId )
  : KCalCore::Calendar( timeZoneId ), d( new Private( this ) )
{

}

AkonadiCalendar::~AkonadiCalendar()
{
  delete d;
}

void AkonadiCalendar::close()
{
  // Not applicable
}

bool AkonadiCalendar::save()
{
  // Not applicable
  return true;
}

bool AkonadiCalendar::load()
{
  d->load();
  d->readFromModel();
  return true;
}

bool AkonadiCalendar::reload()
{
  // Not applicable
  return true;
}

bool AkonadiCalendar::isSaving() const
{
  // TODO: we can return if there's a job running?
  return false;
}

bool AkonadiCalendar::addIncidence( const Incidence::Ptr &incidence )
{
  // TODO: Create a job to add an item.
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::deleteIncidence( const Incidence::Ptr &incidence )
{
  //TODO: Create a job to delete an item.
  Q_UNUSED( incidence );
  return true;
}

Incidence::List AkonadiCalendar::incidences() const
{
  // TODO: use the filter proxy here.
  return incidencesFromItems( itemsFromModel( d->mCalendarModel ) );
}

Incidence::List AkonadiCalendar::incidences( const QString &notebook ) const
{
  Q_UNUSED( notebook );
  return Incidence::List();
}

Incidence::List AkonadiCalendar::incidences( const QDate &date ) const
{
  return KCalCore::Calendar::mergeIncidenceList( events( date ),
                                                 todos( date ), journals( date ) );
}

Incidence::List AkonadiCalendar::rawIncidences() const
{
  return incidencesFromItems( itemsFromModel( d->mCalendarModel ) );
}

Incidence::List AkonadiCalendar::instances( const Incidence::Ptr &incidence ) const
{
  Q_UNUSED( incidence );
  return Incidence::List();
}

bool AkonadiCalendar::deleteIncidenceInstances( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::beginChange( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::endChange( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
  return true;
}

bool AkonadiCalendar::addEvent( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

bool AkonadiCalendar::deleteEvent( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

bool AkonadiCalendar::deleteEventInstances( const Event::Ptr &event )
{
  Q_UNUSED( event );
  return true;
}

void AkonadiCalendar::deleteAllEvents()
{
}

Event::List AkonadiCalendar::events( EventSortField sortField,
                                     SortDirection sortDirection) const
{
  const Event::List el = rawEvents( sortField, sortDirection );
  // return applyCalFilter( el, filter() );
  // TODO filters
  return el;
}

Event::List AkonadiCalendar::events( const QDate &date,
                                     const KDateTime::Spec &timeSpec,
                                     EventSortField sortField,
                                     SortDirection sortDirection ) const
{
  const Event::List list = rawEventsForDate( date, timeSpec, sortField, sortDirection );
  //return applyCalFilter( list, filter() );
  // TODO filters
  return list;
}

Event::List AkonadiCalendar::rawEvents( EventSortField sortField,
                                        SortDirection sortDirection ) const
{
  Event::List eventList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    Event::Ptr event = CalendarSupport::event( i.value() );
    if ( event ) {
      eventList.append( event );
    }
  }
  return KCalCore::Calendar::sortEvents( eventList, sortField, sortDirection );
}

Event::List AkonadiCalendar::rawEventsForDate( const KDateTime &kdt ) const
{
  return rawEventsForDate( kdt.date(), kdt.timeSpec() );
}

Event::List AkonadiCalendar::rawEvents( const QDate &start, const QDate &end,
                                        const KDateTime::Spec &timespec,
                                        bool inclusive ) const
{
  Event::List eventList;
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime st( start, ts );
  KDateTime nd( end, ts );
  KDateTime yesterStart = st.addDays( -1 );
  // Get non-recurring events
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( KCalCore::Event::Ptr event = CalendarSupport::event( i.value() ) ) {
      KDateTime rStart = event->dtStart();
      if ( nd < rStart ) continue;
      if ( inclusive && rStart < st ) {
        continue;
      }
      if ( !event->recurs() ) { // non-recurring events
        KDateTime rEnd = event->dtEnd();
        if ( rEnd < st ) {
          continue;
        }
        if ( inclusive && nd < rEnd ) {
          continue;
        }
      } else { // recurring events
        switch( event->recurrence()->duration() ) {
        case -1: // infinite
          if ( inclusive ) {
            continue;
          }
          break;
        case 0: // end date given
        default: // count given
          KDateTime rEnd( event->recurrence()->endDate(), ts );
          if ( !rEnd.isValid() ) {
            continue;
          }
          if ( rEnd < st ) {
            continue;
          }
          if ( inclusive && nd < rEnd ) {
            continue;
          }
          break;
        } // switch(duration)
      } //if (recurs)
      eventList.append( event );
    }
  }
  return eventList;
}

Event::List AkonadiCalendar::rawEventsForDate( const QDate &date,
                                               const KDateTime::Spec &timespec,
                                               EventSortField sortField,
                                               SortDirection sortDirection ) const
{
  Event::List eventList;
  // Find the hash for the specified date
  QString dateStr = date.toString();
  // Iterate over all non-recurring, single-day events that start on this date
  QMultiHash<QString, Akonadi::Item::Id>::const_iterator it =
    d->m_itemIdsForDate.constFind( dateStr );
  KDateTime::Spec ts = timespec.isValid() ? timespec : timeSpec();
  KDateTime kdt( date, ts );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    if ( KCalCore::Event::Ptr ev = CalendarSupport::event( d->m_itemMap[it.value()] ) ) {
      KDateTime end( ev->dtEnd().toTimeSpec( ev->dtStart() ) );
      if ( ev->allDay() ) {
        end.setDateOnly( true );
      } else {
        end = end.addSecs( -1 );
      }
      if ( end >= kdt ) {
        eventList.append( ev );
      }
    }
    ++it;
  }
  // Iterate over all events. Look for recurring events that occur on this date
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    if ( KCalCore::Event::Ptr ev = CalendarSupport::event( i.value() ) ) {
      if ( ev->recurs() ) {
        if ( ev->isMultiDay() ) {
          int extraDays = ev->dtStart().date().daysTo( ev->dtEnd().date() );
          for ( int j = 0; j <= extraDays; ++j ) {
            if ( ev->recursOn( date.addDays( -j ), ts ) ) {
              eventList.append( ev );
              break;
            }
          }
        } else {
          if ( ev->recursOn( date, ts ) ) {
            eventList.append( ev );
          }
        }
      } else {
        if ( ev->isMultiDay() ) {
          if ( ev->dtStart().date() <= date && ev->dtEnd().date() >= date ) {
            eventList.append( ev );
          }
        }
      }
    }
  }
  return KCalCore::Calendar::sortEvents( eventList, sortField, sortDirection );
}

Event::Ptr AkonadiCalendar::event( const QString &uid,
                                   const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  Akonadi::Item::Id itemId = d->mUidToItemId[uid];

  const Akonadi::Item item = d->m_itemMap.value( itemId );
  return CalendarSupport::event( item );
}

Event::Ptr AkonadiCalendar::deletedEvent( const QString &uid,
                                          const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Event::Ptr();
}

Event::List AkonadiCalendar::eventInstances( const Incidence::Ptr &event,
                                             EventSortField sortField,
                                             SortDirection sortDirection ) const
{
  Q_UNUSED( event );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Event::List();
}

bool AkonadiCalendar::addTodo( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

bool AkonadiCalendar::deleteTodo( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

bool AkonadiCalendar::deleteTodoInstances( const Todo::Ptr &todo )
{
  Q_UNUSED( todo );
  return true;
}

void AkonadiCalendar::deleteAllTodos()
{
}

Todo::List AkonadiCalendar::todos( TodoSortField sortField,
                                   SortDirection sortDirection ) const
{

  const Todo::List tl = rawTodos( sortField, sortDirection );
  //return CalendarSupport::applyCalFilter( tl, filter() );
  // TODO: filters
  return tl;
}

Todo::List AkonadiCalendar::todos( const QDate &date ) const
{
  Todo::List tl = rawTodosForDate( date );
  // return applyCalFilter( tl, filter() );
  // TODO: filter
  return tl;
}

Todo::List AkonadiCalendar::todos( const QDate &start, const QDate &end,
                                   const KDateTime::Spec &timespec,
                                   bool inclusive ) const
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timespec );
  Q_UNUSED( inclusive );
  return Todo::List();
}

Todo::List AkonadiCalendar::rawTodos( TodoSortField sortField,
                                      SortDirection sortDirection ) const
{
  Todo::List todoList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    Todo::Ptr todo = CalendarSupport::todo( i.value() );
    if ( todo ) {
      todoList.append( todo );
    }
  }
  return KCalCore::Calendar::sortTodos( todoList, sortField, sortDirection );
}

Todo::List AkonadiCalendar::rawTodosForDate( const QDate &date ) const
{
  Todo::List todoList;
  QString dateStr = date.toString();
  QMultiHash<QString, Akonadi::Item::Id>::const_iterator it =
    d->m_itemIdsForDate.constFind( dateStr );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    Todo::Ptr todo = CalendarSupport::todo( d->m_itemMap[it.value()] );
    if ( todo ) {
      todoList.append( todo );
    }
    ++it;
  }
  return todoList;
}

Todo::List AkonadiCalendar::rawTodos( const QDate &start, const QDate &end,
                                      const KDateTime::Spec &timespec,
                                      bool inclusive ) const
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( timespec );
  Q_UNUSED( inclusive );
  // TODO
  return Todo::List();
}

Todo::Ptr AkonadiCalendar::todo( const QString &uid,
                                 const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );

  Akonadi::Item::Id itemId = d->mUidToItemId[uid];

  const Akonadi::Item item = d->m_itemMap.value( itemId );
  return CalendarSupport::todo( item );
}

Todo::Ptr AkonadiCalendar::deletedTodo( const QString &uid,
                                        const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Todo::Ptr();
}

Todo::List AkonadiCalendar::deletedTodos( TodoSortField sortField,
                                          SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

Todo::List AkonadiCalendar::todoInstances( const Incidence::Ptr &todo,
                                           TodoSortField sortField,
                                           SortDirection sortDirection ) const
{
  Q_UNUSED( todo );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Todo::List();
}

bool AkonadiCalendar::addJournal( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

bool AkonadiCalendar::deleteJournal( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

bool AkonadiCalendar::deleteJournalInstances( const Journal::Ptr &journal )
{
  Q_UNUSED( journal );
  return true;
}

void AkonadiCalendar::deleteAllJournals()
{
}

Journal::List AkonadiCalendar::journals( JournalSortField sortField,
                                         SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List AkonadiCalendar::journals( const QDate &date ) const
{
  Q_UNUSED( date );
  return Journal::List();
}

Journal::List AkonadiCalendar::rawJournals( JournalSortField sortField,
                                            SortDirection sortDirection ) const
{
  Journal::List journalList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    i.next();
    Journal::Ptr journal = CalendarSupport::journal( i.value() );
    if ( journal ) {
      journalList.append( journal );
    }
  }
  return KCalCore::Calendar::sortJournals( journalList, sortField, sortDirection );
}

Journal::List AkonadiCalendar::rawJournalsForDate( const QDate &date ) const
{
  Journal::List journalList;
  QString dateStr = date.toString();
  QMultiHash<QString, Akonadi::Item::Id>::const_iterator it =
    d->m_itemIdsForDate.constFind( dateStr );
  while ( it != d->m_itemIdsForDate.constEnd() && it.key() == dateStr ) {
    Journal::Ptr journal = CalendarSupport::journal( d->m_itemMap[it.value()] );
    if ( journal ) {
      journalList.append( journal );
    }
    ++it;
  }
  return journalList;
}

Journal::Ptr AkonadiCalendar::journal( const QString &uid,
                                       const KDateTime &recurrenceId ) const
{
  Q_UNUSED( recurrenceId );
  Akonadi::Item::Id itemId = d->mUidToItemId[uid];

  const Akonadi::Item item = d->m_itemMap.value( itemId );
  return CalendarSupport::journal( item );
}

Journal::Ptr AkonadiCalendar::deletedJournal( const QString &uid,
                                              const KDateTime &recurrenceId ) const
{
  Q_UNUSED( uid );
  Q_UNUSED( recurrenceId );
  return Journal::Ptr();
}

Journal::List AkonadiCalendar::deletedJournals( JournalSortField sortField,
                                                SortDirection sortDirection ) const
{
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

Journal::List AkonadiCalendar::journalInstances( const Incidence::Ptr &journal,
                                                 JournalSortField sortField,
                                                 SortDirection sortDirection ) const
{
  Q_UNUSED( journal );
  Q_UNUSED( sortField );
  Q_UNUSED( sortDirection );
  return Journal::List();
}

void AkonadiCalendar::setupRelations( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
}

void AkonadiCalendar::removeRelations( const Incidence::Ptr &incidence )
{
  Q_UNUSED( incidence );
}


Alarm::List AkonadiCalendar::alarms( const KDateTime &from, const KDateTime &to ) const
{
  KCalCore::Alarm::List alarmList;
  QHashIterator<Akonadi::Item::Id, Akonadi::Item> i( d->m_itemMap );
  while ( i.hasNext() ) {
    const Akonadi::Item item = i.next().value();
    KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
    if ( !incidence ) {
      continue;
    }

    if ( incidence->recurs() ) {
      appendRecurringAlarms( alarmList, incidence, from, to );
    } else {
      appendAlarms( alarmList, incidence, from, to );
    }
  }
  return alarmList;
}

void AkonadiCalendar::incidenceUpdated( const QString &, const KDateTime & )
{
}

void AkonadiCalendar::incidenceUpdate( const QString &,
                                       const KDateTime & )
{
}

Event::List AkonadiCalendar::deletedEvents(
  EventSortField sortField,
  SortDirection sortDirection) const
{
  Q_UNUSED( sortDirection );
  Q_UNUSED( sortField );
  return Event::List();
}
