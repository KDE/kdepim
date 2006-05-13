/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
/**
   @file calendarresources.cpp
   Provides a calendar composed of several calendar resources.

   @author Cornelius Schumacher
   @author Reinhold Kainhofer
*/
#include <stdlib.h>

#include <QDateTime>
#include <QString>
#include <QList>
//Added by qt3to4:

#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"
#include "filestorage.h"

#include <kresources/manager.h>
#include <kresources/selectdialog.h>
#include <kabc/lock.h>

#include "resourcecalendar.h"
#include "resourcelocal.h"

#include "calendarresources.h"

using namespace KCal;

ResourceCalendar
*CalendarResources::StandardDestinationPolicy::destination( Incidence * )
{
  return resourceManager()->standardResource();
}

ResourceCalendar
*CalendarResources::AskDestinationPolicy::destination( Incidence * )
{
  QList<KRES::Resource*> list;

  CalendarResourceManager::ActiveIterator it;
  for ( it = resourceManager()->activeBegin();
        it != resourceManager()->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() ) {
      //Insert the first the Standard resource to get be the default selected.
      if ( resourceManager()->standardResource() == *it )
        list.insert( 0, *it );
      else
        list.append( *it );
    }
  }

  KRES::Resource *r;
  r = KRES::SelectDialog::getResource( list, mParent );
  return static_cast<ResourceCalendar *>( r );
}

CalendarResources::CalendarResources( const QString &timeZoneId,
                                      const QString &family )
  : Calendar( timeZoneId )
{
  init( family );
}

void CalendarResources::init( const QString &family )
{
  kDebug(5800) << "CalendarResources::init( " << family << " )" << endl;

  mManager = new CalendarResourceManager( family );
  mManager->addObserver( this );

  mStandardPolicy = new StandardDestinationPolicy( mManager );
  mAskPolicy = new AskDestinationPolicy( mManager );
  mDestinationPolicy = mStandardPolicy;
}

CalendarResources::~CalendarResources()
{
  close();
  delete mManager;
}

void CalendarResources::readConfig( KConfig *config )
{
  mManager->readConfig( config );

  CalendarResourceManager::Iterator it;
  for ( it = mManager->begin(); it != mManager->end(); ++it ) {
    connectResource( *it );
  }
}

void CalendarResources::load()
{
  kDebug(5800) << "CalendarResources::load()" << endl;

  if ( !mManager->standardResource() ) {
    kDebug(5800) << "Warning! No standard resource yet." << endl;
  }

  // set the timezone for all resources. Otherwise we'll have those terrible tz
  // troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = mManager->begin(); i1 != mManager->end(); ++i1 ) {
    (*i1)->setTimeZoneId( timeZoneId() );
  }

  QList<ResourceCalendar *> failed;

  // Open all active resources
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( !(*it)->load() ) {
      failed.append( *it );
    }
    Incidence::List incidences = (*it)->rawIncidences();
    Incidence::List::Iterator incit;
    for ( incit = incidences.begin(); incit != incidences.end(); ++incit ) {
      (*incit)->registerObserver( this );
      notifyIncidenceAdded( *incit );
    }
  }

  QList<ResourceCalendar *>::ConstIterator it2;
  for ( it2 = failed.begin(); it2 != failed.end(); ++it2 ) {
    (*it2)->setActive( false );
    emit signalResourceModified( *it2 );
  }

  mOpen = true;
}

bool CalendarResources::reload( const QString &tz )
{
  save();
  close();
  setTimeZoneId( tz );
  load();
  return true;
}

void CalendarResources::setStandardDestinationPolicy()
{
  mDestinationPolicy = mStandardPolicy;
}

void CalendarResources::setAskDestinationPolicy()
{
  mDestinationPolicy = mAskPolicy;
}

void CalendarResources::close()
{
  kDebug(5800) << "CalendarResources::close" << endl;

  if ( mOpen ) {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->close();
    }

    setModified( false );
    mOpen = false;
  }
}

void CalendarResources::save()
{
  kDebug(5800) << "CalendarResources::save()" << endl;

  if ( mOpen && isModified() ) {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->save();
    }

    setModified( false );
  }
}

bool CalendarResources::isSaving()
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it)->isSaving() ) {
      return true;
    }
  }

  return false;
}

bool CalendarResources::addIncidence( Incidence *incidence,
                                      ResourceCalendar *resource )
{
  // FIXME: Use proper locking via begin/endChange!
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it) == resource )
      validRes = true;
  }
  ResourceCalendar *oldResource = 0;
  if ( mResourceMap.contains( incidence ) ) {
    oldResource = mResourceMap[incidence];
  }
  mResourceMap[incidence] = resource;
  if ( validRes && beginChange( incidence ) &&
       resource->addIncidence( incidence ) ) {
//    mResourceMap[incidence] = resource;
    incidence->registerObserver( this );
    notifyIncidenceAdded( incidence );
    setModified( true );
    endChange( incidence );
    return true;
  } else {
    if ( oldResource )
      mResourceMap[incidence] = oldResource;
    else
      mResourceMap.remove( incidence );
  }

  return false;
}

bool CalendarResources::addIncidence( Incidence *incidence )
{
  kDebug(5800) << "CalendarResources::addIncidence" << this << endl;

  ResourceCalendar *resource = mDestinationPolicy->destination( incidence );

  if ( resource ) {
    mResourceMap[ incidence ] = resource;

    if ( beginChange( incidence ) && resource->addIncidence( incidence ) ) {
      incidence->registerObserver( this );
      notifyIncidenceAdded( incidence );


      mResourceMap[ incidence ] = resource;
      setModified( true );
      endChange( incidence );
      return true;
    } else {
      mResourceMap.remove( incidence );
    }
  } else
    kDebug(5800) << "CalendarResources::addIncidence(): no resource" << endl;

  return false;
}

bool CalendarResources::addEvent( Event *event )
{
  kDebug(5800) << "CalendarResources::addEvent" << endl;
  return addIncidence( event );
}

bool CalendarResources::addEvent( Event *Event, ResourceCalendar *resource )
{
  return addIncidence( Event, resource );
}

bool CalendarResources::deleteEvent( Event *event )
{
  kDebug(5800) << "CalendarResources::deleteEvent" << endl;

  bool status;
  if ( mResourceMap.find( event ) != mResourceMap.end() ) {
    status = mResourceMap[event]->deleteEvent( event );
    if ( status )
      mResourceMap.remove( event );
  } else {
    status = false;
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      status = (*it)->deleteEvent( event ) || status;
    }
  }

  setModified( status );
  return status;
}

Event *CalendarResources::event( const QString &uid )
{
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event* event = (*it)->event( uid );
    if ( event ) {
      mResourceMap[event] = *it;
      return event;
    }
  }

  // Not found
  return 0;
}

bool CalendarResources::addTodo( Todo *todo )
{
  kDebug(5800) << "CalendarResources::addTodo" << endl;
  return addIncidence( todo );
}

bool CalendarResources::addTodo( Todo *todo, ResourceCalendar *resource )
{
  return addIncidence( todo, resource );
}

bool CalendarResources::deleteTodo( Todo *todo )
{
  kDebug(5800) << "CalendarResources::deleteTodo" << endl;

  bool status;
  if ( mResourceMap.find( todo ) != mResourceMap.end() ) {
    status = mResourceMap[todo]->deleteTodo( todo );
    if ( status )
      mResourceMap.remove( todo );
  } else {
    CalendarResourceManager::ActiveIterator it;
    status = false;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      status = (*it)->deleteTodo( todo ) || status;
    }
  }

  setModified( status );
  return status;
}

Todo::List CalendarResources::rawTodos( TodoSortField sortField,
                                        SortDirection sortDirection )
{
  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Todo::List>( result, (*it)->rawTodos( TodoSortUnsorted ), *it );
  }
  return sortTodos( &result, sortField, sortDirection );
}

Todo *CalendarResources::todo( const QString &uid )
{
  kDebug(5800) << "CalendarResources::todo(uid)" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Todo *todo = (*it)->todo( uid );
    if ( todo ) {
      mResourceMap[todo] = *it;
      return todo;
    }
  }

  // Not found
  return 0;
}

Todo::List CalendarResources::rawTodosForDate( const QDate &date )
{
  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Todo::List>( result, (*it)->rawTodosForDate( date ), *it );
  }

  return result;
}

Alarm::List CalendarResources::alarmsTo( const QDateTime &to )
{
  kDebug(5800) << "CalendarResources::alarmsTo" << endl;

  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    result += (*it)->alarmsTo( to );
  }
  return result;
}

Alarm::List CalendarResources::alarms( const QDateTime &from,
                                       const QDateTime &to )
{
  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    result += (*it)->alarms( from, to );
  }
  return result;
}

/****************************** PROTECTED METHODS ****************************/

Event::List CalendarResources::rawEventsForDate( const QDate &date,
                                                 EventSortField sortField,
                                                 SortDirection sortDirection )
{
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Event::List>( result, (*it)->rawEventsForDate( date ), *it );
  }
  return sortEvents( &result, sortField, sortDirection );
}

Event::List CalendarResources::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  kDebug(5800) << "CalendarResources::rawEvents(start,end,inclusive)" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Event::List>( result, (*it)->rawEvents( start, end, inclusive ), *it );
  }
  return result;
}

Event::List CalendarResources::rawEventsForDate( const QDateTime &qdt )
{
  kDebug(5800) << "CalendarResources::rawEventsForDate(qdt)" << endl;

  // @TODO: Remove the code duplication by the resourcemap iteration block.
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Event::List>( result, (*it)->rawEventsForDate( qdt ), *it );
  }
  return result;
}

Event::List CalendarResources::rawEvents( EventSortField sortField,
                                          SortDirection sortDirection )
{
  kDebug(5800) << "CalendarResources::rawEvents()" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Event::List>( result, (*it)->rawEvents( EventSortUnsorted ), *it );
  }
  return sortEvents( &result, sortField, sortDirection );
}


bool CalendarResources::addJournal( Journal *journal )
{
  kDebug(5800) << "CalendarResources::addJournal" << endl;
  return addIncidence( journal );
}

bool CalendarResources::deleteJournal( Journal *journal )
{
  kDebug(5800) << "CalendarResources::deleteJournal" << endl;

  bool status;
  if ( mResourceMap.find( journal ) != mResourceMap.end() ) {
    status = mResourceMap[journal]->deleteJournal( journal );
    if ( status )
      mResourceMap.remove( journal );
  } else {
    CalendarResourceManager::ActiveIterator it;
    status = false;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      status = (*it)->deleteJournal( journal ) || status;
    }
  }

  setModified( status );
  return status;
}

bool CalendarResources::addJournal( Journal *journal,
                                    ResourceCalendar *resource
  )
{
  return addIncidence( journal, resource );
}

Journal *CalendarResources::journal( const QString &uid )
{
  kDebug(5800) << "CalendarResources::journal(uid)" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal* journal = (*it)->journal( uid );
    if ( journal ) {
      mResourceMap[journal] = *it;
      return journal;
    }
  }

  // Not found
  return 0;
}

Journal::List CalendarResources::rawJournals( JournalSortField sortField,
                                              SortDirection sortDirection )
{
  kDebug(5800) << "CalendarResources::rawJournals()" << endl;

  Journal::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Journal::List>( result, (*it)->rawJournals( JournalSortUnsorted ), *it );
  }
  return sortJournals( &result, sortField, sortDirection );
}

Journal::List CalendarResources::rawJournalsForDate( const QDate &date )
{

  Journal::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    appendIncidences<Journal::List>( result, (*it)->rawJournalsForDate( date ), *it );
  }
  return result;
}

template< class IncidenceList >
void CalendarResources::appendIncidences(IncidenceList &result, const IncidenceList &extra, ResourceCalendar *resource)
{
  result += extra;
  for (typename IncidenceList::ConstIterator it = extra.begin();  it != extra.end();  ++it)
    mResourceMap[ *it ] = resource;
}

void CalendarResources::connectResource( ResourceCalendar *resource )
{
  connect( resource, SIGNAL( resourceChanged( ResourceCalendar * ) ),
           SIGNAL( calendarChanged() ) );
  connect( resource, SIGNAL( resourceSaved( ResourceCalendar * ) ),
           SIGNAL( calendarSaved() ) );

  connect( resource, SIGNAL( resourceLoadError( ResourceCalendar *,
                                                const QString & ) ),
           SLOT( slotLoadError( ResourceCalendar *, const QString & ) ) );
  connect( resource, SIGNAL( resourceSaveError( ResourceCalendar *,
                                                const QString & ) ),
           SLOT( slotSaveError( ResourceCalendar *, const QString & ) ) );
}

ResourceCalendar *CalendarResources::resource( Incidence *incidence )
{
  if ( mResourceMap.find( incidence ) != mResourceMap.end() ) {
    return mResourceMap[ incidence ];
  }
  return 0;
}

void CalendarResources::resourceAdded( ResourceCalendar *resource )
{
  kDebug(5800) << "Resource added: " << resource->resourceName() << endl;

  if ( !resource->isActive() )
    return;

  if ( resource->open() ) {
    resource->load();
  }

  connectResource( resource );

  emit signalResourceAdded( resource );
}

void CalendarResources::resourceModified( ResourceCalendar *resource )
{
  kDebug(5800) << "Resource modified: " << resource->resourceName() << endl;

  emit signalResourceModified( resource );
}

void CalendarResources::resourceDeleted( ResourceCalendar *resource )
{
  kDebug(5800) << "Resource deleted: " << resource->resourceName() << endl;

  emit signalResourceDeleted( resource );
}

void CalendarResources::doSetTimeZoneId( const QString &timeZoneId )
{
  // set the timezone for all resources. Otherwise we'll have those terrible
  // tz troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = mManager->begin(); i1 != mManager->end(); ++i1 ) {
    (*i1)->setTimeZoneId( timeZoneId );
  }
}

void CalendarResources::setTimeZoneIdViewOnly( const QString &timeZoneId )
{
  reload( timeZoneId );
}

CalendarResources::Ticket
*CalendarResources::requestSaveTicket( ResourceCalendar *resource )
{
  kDebug(5800) << "CalendarResources::requestSaveTicket()" << endl;

  KABC::Lock *lock = resource->lock();
  if ( !lock )
    return 0;
  if ( lock->lock() )
    return new Ticket( resource );
  else
    return 0;
}

bool CalendarResources::save( Ticket *ticket, Incidence *incidence )
{
  kDebug(5800) << "CalendarResources::save( Ticket *)" << endl;

  if ( !ticket || !ticket->resource() )
    return false;

  kDebug(5800) << "tick " << ticket->resource()->resourceName() << endl;

    // @TODO: Check if the resource was changed at all. If not, don't save.
  if ( ticket->resource()->save( incidence ) ) {
    releaseSaveTicket( ticket );
    return true;
  }

  return false;
}

void CalendarResources::releaseSaveTicket( Ticket *ticket )
{
  ticket->resource()->lock()->unlock();
  delete ticket;
}

bool CalendarResources::beginChange( Incidence *incidence )
{
  kDebug(5800) << "CalendarResources::beginChange()" << endl;

  ResourceCalendar *r = resource( incidence );
  if ( !r ) {
    r = mDestinationPolicy->destination( incidence );
    if ( !r ) {
      kError() << "Unable to get destination resource." << endl;
      return false;
    }
    mResourceMap[ incidence ] = r;
  }

  int count = incrementChangeCount( r );
  if ( count == 1 ) {
    Ticket *ticket = requestSaveTicket( r );
    if ( !ticket ) {
      kDebug(5800) << "CalendarResources::beginChange(): unable to get ticket."
                    << endl;
      decrementChangeCount( r );
      return false;
    } else {
      mTickets[ r ] = ticket;
    }
  }

  return true;
}

bool CalendarResources::endChange( Incidence *incidence )
{
  kDebug(5800) << "CalendarResource::endChange()" << endl;

  ResourceCalendar *r = resource( incidence );
  if ( !r )
    return false;

  int count = decrementChangeCount( r );

  if ( count == 0 ) {
    bool ok = save( mTickets[ r ], incidence );
    if ( ok ) {
      mTickets.remove( r );
    } else {
      return false;
    }
  }

  return true;
}

int CalendarResources::incrementChangeCount( ResourceCalendar *r )
{
  if ( !mChangeCounts.contains( r ) ) {
    mChangeCounts.insert( r, 0 );
  }

  int count = mChangeCounts[ r ];
  ++count;
  mChangeCounts[ r ] = count;

  return count;
}

int CalendarResources::decrementChangeCount( ResourceCalendar *r )
{
  if ( !mChangeCounts.contains( r ) ) {
    kError() << "No change count for resource." << endl;
    return 0;
  }

  int count = mChangeCounts[ r ];
  --count;
  if ( count < 0 ) {
    kError() << "Can't decrement change count. It already is 0." << endl;
    count = 0;
  }
  mChangeCounts[ r ] = count;

  return count;
}

void CalendarResources::slotLoadError( ResourceCalendar *, const QString &err )
{
  emit signalErrorMessage( err );
}

void CalendarResources::slotSaveError( ResourceCalendar *, const QString &err )
{
  emit signalErrorMessage( err );
}

#include "calendarresources.moc"
