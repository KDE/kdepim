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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

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

ResourceCalendar *CalendarResources::StandardDestinationPolicy::destination( Incidence * )
{
  return resourceManager()->standardResource();
}

ResourceCalendar *CalendarResources::AskDestinationPolicy::destination( Incidence * )
{
  QPtrList<KRES::Resource> list;

  CalendarResourceManager::ActiveIterator it;
  for( it = resourceManager()->activeBegin();
       it != resourceManager()->activeEnd(); ++it ) {
    if ( !(*it)->readOnly() ) {
      //Insert the first the Standard resource to get be the default selected.
      if ( resourceManager()->standardResource() == *it )
        list.insert( 0,*it );
      else
        list.append( *it );
    }
  }

  KRES::Resource *r;
  r = KRES::SelectDialog::getResource( list, mParent );
  return static_cast<ResourceCalendar *>( r );
}

CalendarResources::CalendarResources( const QString &timeZoneId, const QString &family )
  : Calendar( timeZoneId )
{
  init( family );
}

void CalendarResources::init( const QString &family )
{
  kdDebug(5800) << "CalendarResources::init( " << family << " )" << endl;

  mManager = new CalendarResourceManager( family );
  mManager->addObserver( this );

  mStandardPolicy = new StandardDestinationPolicy( mManager );
  mAskPolicy = new AskDestinationPolicy( mManager );
  mDestinationPolicy = mStandardPolicy;
}

CalendarResources::~CalendarResources()
{
//  kdDebug(5800) << "CalendarResources::destructor" << endl;

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
  kdDebug(5800) << "CalendarResources::load()" << endl;

  if ( !mManager->standardResource() ) {
    kdDebug(5800) << "Warning! No standard resource yet." << endl;
  }

  // set the timezone for all resources. Otherwise we'll have those terrible tz
  // troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = mManager->begin(); i1 != mManager->end(); ++i1 ) {
    (*i1)->setTimeZoneId( timeZoneId() );
  }

  QValueList<ResourceCalendar *> failed;

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

  QValueList<ResourceCalendar *>::ConstIterator it2;
  for( it2 = failed.begin(); it2 != failed.end(); ++it2 ) {
    (*it2)->setActive( false );
    emit signalResourceModified( *it2 );
  }

  mOpen = true;
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
  kdDebug(5800) << "CalendarResources::close" << endl;

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
  kdDebug(5800) << "CalendarResources::save()" << endl;

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

bool CalendarResources::addIncidence( Incidence *incidence, ResourceCalendar *resource )
{
  // FIXME: Use proper locking via begin/endChange!
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) validRes = true;
  }
  ResourceCalendar *oldResource = 0;
  if ( mResourceMap.contains(incidence) ) {
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
    if ( oldResource ) mResourceMap[incidence] = oldResource;
    else mResourceMap.remove( incidence );
  }

  return false;
}

bool CalendarResources::addIncidence( Incidence *incidence )
{
  kdDebug(5800) << "CalendarResources::addIncidence" << this << endl;

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
    kdDebug(5800) << "CalendarResources::addIncidence(): no resource" << endl;

  return false;
}

bool CalendarResources::addEvent( Event *event )
{
  return addIncidence( event );
}

bool CalendarResources::addEvent( Event *anEvent, ResourceCalendar *resource )
{
  return addIncidence( anEvent, resource );
}

void CalendarResources::deleteEvent( Event *event )
{
  kdDebug(5800) << "CalendarResources::deleteEvent" << endl;

  if ( mResourceMap.find( event ) != mResourceMap.end() ) {
    mResourceMap[event]->deleteEvent( event );
    mResourceMap.remove( event );
  } else {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->deleteEvent( event );
    }
  }

  setModified( true );
}


Event *CalendarResources::event( const QString &uid )
{
//  kdDebug(5800) << "CalendarResources::event(): " << uid << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event* event = (*it)->event( uid );
    if ( event )
    {
      mResourceMap[event] = *it;
      return event;
    }
  }

  // Not found
  return 0;
}

bool CalendarResources::addTodo( Todo *todo )
{
  kdDebug(5800) << "CalendarResources::addTodo" << endl;

  return addIncidence( todo );
}

bool CalendarResources::addTodo(Todo *todo, ResourceCalendar *resource)
{
  return addIncidence( todo, resource );
}

void CalendarResources::deleteTodo( Todo *todo )
{
  kdDebug(5800) << "CalendarResources::deleteTodo" << endl;

  Q_ASSERT(todo);

  if ( mResourceMap.find(todo) != mResourceMap.end() ) {
    mResourceMap[todo]->deleteTodo( todo );
    mResourceMap.remove( todo );
  } else {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->deleteTodo( todo );
    }
  }

  setModified( true );
}

Todo::List CalendarResources::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
//  kdDebug(5800) << "CalendarResources::rawTodos()" << endl;

  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
//    kdDebug(5800) << "Getting raw todos from '" << (*it)->resourceName()
//                  << "'" << endl;
    Todo::List todos = (*it)->rawTodos( TodoSortUnsorted );
    Todo::List::ConstIterator it2;
    for ( it2 = todos.begin(); it2 != todos.end(); ++it2 ) {
//      kdDebug(5800) << "Adding todo to result" << endl;
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return sortTodos( &result, sortField, sortDirection );
}

Todo *CalendarResources::todo( const QString &uid )
{
  kdDebug(5800) << "CalendarResources::todo(uid)" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Todo *todo = (*it)->todo( uid );
    if ( todo ) {
      mResourceMap[todo] = *it;
      return todo;
    }
  }

  // not found
  return 0;
}

Todo::List CalendarResources::rawTodosForDate( const QDate &date )
{
//  kdDebug(5800) << "CalendarResources::rawTodosforDate(date)" << endl;

  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Todo::List todos = (*it)->rawTodosForDate( date );
    Todo::List::ConstIterator it2;
    for ( it2 = todos.begin(); it2 != todos.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }

  return result;
}


Alarm::List CalendarResources::alarmsTo( const QDateTime &to )
{
  kdDebug(5800) << "CalendarResources::alarmsTo" << endl;

  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Alarm::List list = (*it)->alarmsTo( to );
    Alarm::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      result.append( *it );
  }
  return result;
}

Alarm::List CalendarResources::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "CalendarResources::alarms(" << from.toString() << " - "
//                << to.toString() << ")" << endl;

  Alarm::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Alarm::List list = (*it)->alarms( from, to );
    Alarm::List::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it )
      result.append( *it );
  }
  return result;
}

/****************************** PROTECTED METHODS ****************************/


// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
Event::List CalendarResources::rawEventsForDate( const QDate &qd, bool sorted )
{
//  kdDebug(5800) << "CalendarResources::rawEventsForDate()" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
//    kdDebug(5800) << "Getting events from '" << (*it)->resourceName() << "'"
//                  << endl;
    Event::List list = (*it)->rawEventsForDate( qd, sorted );

    Event::List::ConstIterator it2;
    if ( sorted ) {
      Event::List::Iterator insertionPoint = result.begin();
      for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
        while ( insertionPoint != result.end() &&
                (*insertionPoint)->dtStart().time() <= (*it2)->dtStart().time() )
          insertionPoint++;
        result.insert( insertionPoint, *it2 );
        mResourceMap[ *it2 ] = *it;
      }
    } else {
      for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
        result.append( *it2 );
        mResourceMap[ *it2 ] = *it;
      }
    }
  }

  return result;
}

Event::List CalendarResources::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  kdDebug(5800) << "CalendarResources::rawEvents(start,end,inclusive)" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event::List list = (*it)->rawEvents( start, end, inclusive );
    Event::List::ConstIterator it2;
    for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return result;
}

Event::List CalendarResources::rawEventsForDate(const QDateTime &qdt)
{
  kdDebug(5800) << "CalendarResources::rawEventsForDate(qdt)" << endl;

  // TODO: Remove the code duplication by the resourcemap iteration block.
  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event::List list = (*it)->rawEventsForDate( qdt );
    Event::List::ConstIterator it2;
    for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return result;
}

Event::List CalendarResources::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  kdDebug(5800) << "CalendarResources::rawEvents()" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event::List list = (*it)->rawEvents( EventSortUnsorted );
    Event::List::ConstIterator it2;
    for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return sortEvents( &result, sortField, sortDirection );
}


bool CalendarResources::addJournal( Journal *journal )
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return addIncidence( journal );
}

void CalendarResources::deleteJournal( Journal *journal )
{
  kdDebug(5800) << "CalendarResources::deleteJournal" << endl;

  if ( mResourceMap.find(journal)!=mResourceMap.end() ) {
    mResourceMap[journal]->deleteJournal( journal );
    mResourceMap.remove( journal );
  } else {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->deleteJournal( journal );
    }
  }

  setModified( true );
}

bool CalendarResources::addJournal(Journal *journal, ResourceCalendar *resource)
{
  return addIncidence( journal, resource );
}

Journal *CalendarResources::journal( const QString &uid )
{
  kdDebug(5800) << "CalendarResources::journal(uid)" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal* journal = (*it)->journal( uid );
    if ( journal ) {
      mResourceMap[journal] = *it;
      return journal;
    }
  }

  // not found
  return 0;
}

Journal::List CalendarResources::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  kdDebug(5800) << "CalendarResources::rawJournals()" << endl;

  Journal::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal::List journals = (*it)->rawJournals( JournalSortUnsorted );
    Journal::List::ConstIterator it2;
    for ( it2 = journals.begin(); it2 != journals.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return sortJournals( &result, sortField, sortDirection );
}

Journal::List CalendarResources::rawJournalsForDate( const QDate &date )
{

  Journal::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal::List journals = (*it)->rawJournalsForDate( date );
    Journal::List::ConstIterator it2;
    for ( it2 = journals.begin(); it2 != journals.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return result;
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

ResourceCalendar *CalendarResources::resource(Incidence *inc)
{
  if ( mResourceMap.find( inc ) != mResourceMap.end() ) {
    return mResourceMap[ inc ];
  }
  return 0;
}

void CalendarResources::resourceAdded( ResourceCalendar *resource )
{
  kdDebug(5800) << "Resource added: " << resource->resourceName() << endl;

  if ( !resource->isActive() ) return;

  if ( resource->open() ) {
    resource->load();
  }

  connectResource( resource );

  emit signalResourceAdded( resource );
}

void CalendarResources::resourceModified( ResourceCalendar *resource )
{
  kdDebug(5800) << "Resource modified: " << resource->resourceName() << endl;

  emit signalResourceModified( resource );
}

void CalendarResources::resourceDeleted( ResourceCalendar *resource )
{
  kdDebug(5800) << "Resource deleted: " << resource->resourceName() << endl;

  emit signalResourceDeleted( resource );
}

void CalendarResources::doSetTimeZoneId( const QString &tzid )
{
  // set the timezone for all resources. Otherwise we'll have those terrible
  // tz troubles ;-((
  CalendarResourceManager::Iterator i1;
  for ( i1 = mManager->begin(); i1 != mManager->end(); ++i1 ) {
    (*i1)->setTimeZoneId( tzid );
  }
}

CalendarResources::Ticket *CalendarResources::requestSaveTicket( ResourceCalendar *resource )
{
  kdDebug(5800) << "CalendarResources::requestSaveTicket()" << endl;

  KABC::Lock *lock = resource->lock();
  if ( !lock ) return 0;
  if ( lock->lock() ) return new Ticket( resource );
  else return 0;
}

bool CalendarResources::save( Ticket *ticket, Incidence *incidence )
{
  kdDebug(5800) << "CalendarResources::save( Ticket *)" << endl;

  if ( !ticket || !ticket->resource() ) return false;

  kdDebug(5800) << "tick " << ticket->resource()->resourceName() << endl;

    // @TODO: Check if the resource was changed at all. If not, don't save.
  if ( ticket->resource()->save(incidence) ) {
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
  kdDebug(5800) << "CalendarResources::beginChange()" << endl;

  ResourceCalendar *r = resource( incidence );
  if ( !r ) {
    r = mDestinationPolicy->destination( incidence );
    if ( !r ) {
      kdError() << "Unable to get destination resource." << endl;
      return false;
    }
    mResourceMap[ incidence ] = r;
  }

  int count = incrementChangeCount( r );
  if ( count == 1 ) {
    Ticket *ticket = requestSaveTicket( r );
    if ( !ticket ) {
      kdDebug(5800) << "CalendarResources::beginChange(): unable to get ticket."
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
  kdDebug(5800) << "CalendarResource::endChange()" << endl;

  ResourceCalendar *r = resource( incidence );
  if ( !r ) return false;

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
    kdError() << "No change count for resource." << endl;
    return 0;
  }

  int count = mChangeCounts[ r ];
  --count;
  if ( count < 0 ) {
    kdError() << "Can't decrement change count. It already is 0." << endl;
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
