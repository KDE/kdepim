/*
    This file is part of libkcal.
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    if ( !(*it)->readOnly() )
      list.append( *it );
  }

  KRES::Resource *r;
  r = KRES::SelectDialog::getResource( list, mParent );
  return static_cast<ResourceCalendar *>( r );
}

CalendarResources::CalendarResources()
  : Calendar()
{
  init();
}

CalendarResources::CalendarResources(const QString &timeZoneId)
  : Calendar(timeZoneId)
{
  init();
}

void CalendarResources::init()
{
  kdDebug(5800) << "CalendarResources::init" << endl;

  mConfig = new KConfig( "kcalrc" );

  mManager = new CalendarResourceManager( "calendar" );
  mManager->readConfig( mConfig );

  if ( !mManager->standardResource() ) {
    kdDebug() << "Warning! No standard resource yet." << endl;
  }

  // Open all active resources
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    kdDebug(5800) << "Opening resource " + (*it)->resourceName() << endl;
    bool result = (*it)->open();
    result = (*it)->load();
    // Really should remove resource if open not successful
    connectResource( *it );
  }

  mStandardPolicy = new StandardDestinationPolicy( mManager );
  mAskPolicy = new AskDestinationPolicy( mManager );
  mDestinationPolicy = mStandardPolicy;

  mOpen = true;
}


CalendarResources::~CalendarResources()
{
  kdDebug(5800) << "CalendarResources::destructor" << endl;

  close();

  delete mManager;
  delete mConfig;
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

  if ( mOpen ) {
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

bool CalendarResources::addIncidence( Incidence *incidence )
{
  kdDebug(5800) << "CalendarResources::addIncidence" << endl;

  ResourceCalendar *resource = mDestinationPolicy->destination( incidence );

  if ( resource ) {
    resource->addIncidence( incidence );
    mResourceMap[ incidence ] = resource;
  } else {
    kdDebug(5800) << "CalendarResources::addIncidence(): no resource" << endl;
    return false;
  }

  setModified( true );

  return true;
}

bool CalendarResources::addEvent( Event *event )
{
  return addIncidence( event );
}

bool CalendarResources::addEvent(Event *anEvent, ResourceCalendar *resource)
{
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) validRes = true;
  }
  if ( validRes ) {
    resource->addEvent( anEvent );
    mResourceMap[anEvent] = resource;
  } else {
    return false;
  }

  return true;
}

void CalendarResources::deleteEvent(Event *event)
{
  kdDebug(5800) << "CalendarResources::deleteEvent" << endl;

  if ( mResourceMap.find(event)!=mResourceMap.end() ) {
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
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) validRes = true;
  }
  if ( validRes ) {
    resource->addTodo( todo );
    mResourceMap[todo] = resource;
  } else {
    return false;
  }

  return true;
}

void CalendarResources::deleteTodo(Todo *todo)
{
  kdDebug(5800) << "CalendarResources::deleteTodo" << endl;

  Q_ASSERT(todo);

  if ( mResourceMap.find(todo)!=mResourceMap.end() ) {
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

Todo::List CalendarResources::rawTodos()
{
//  kdDebug(5800) << "CalendarResources::rawTodos()" << endl;

  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
//    kdDebug(5800) << "Getting raw todos from '" << (*it)->resourceName()
//                  << "'" << endl;
    Todo::List todos = (*it)->rawTodos();
    Todo::List::ConstIterator it2;
    for ( it2 = todos.begin(); it2 != todos.end(); ++it2 ) {
//      kdDebug(5800) << "Adding todo to result" << endl;
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }

  return result;
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

Todo::List CalendarResources::todos( const QDate &date )
{
//  kdDebug(5800) << "CalendarResources::todos(date)" << endl;

  Todo::List result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Todo::List todos = (*it)->todos( date );
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
  kdDebug(5800) << "CalendarResources::alarms(" << from.toString() << " - " << to.toString() << ")\n";

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
//    kdDebug() << "Getting events from '" << (*it)->resourceName() << "'"
//              << endl;
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

Event::List CalendarResources::rawEvents()
{
  kdDebug(5800) << "CalendarResources::rawEvents()" << endl;

  Event::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event::List list = (*it)->rawEvents();
    Event::List::ConstIterator it2;
    for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return result;
}


bool CalendarResources::addJournal( Journal *journal )
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return addIncidence( journal );
}

void CalendarResources::deleteJournal( Journal * )
{
  kdError(5800) << "CalendarResources not implemented yet." << endl;
}

bool CalendarResources::addJournal(Journal *journal, ResourceCalendar *resource)
{
  bool validRes = false;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    if ( (*it) == resource ) validRes = true;
  }
  if ( validRes ) {
    resource->addJournal( journal );
    mResourceMap[journal] = resource;
  } else {
    return false;
  }

  return true;
}

Journal *CalendarResources::journal(const QDate &date)
{
  kdDebug(5800) << "CalendarResources::journal() " << date.toString() << endl;
  kdDebug(5800) << "FIXME: what to do with the multiple journals from multiple calendar resources????" << endl;

  // If we're on a private resource, return that journal.
  // Else, first see if the standard resource has a journal for this date. If it has, return that journal.
  // If not, check all resources for a journal for this date.

  if ( mManager->standardResource() ) {
    Journal* journal = mManager->standardResource()->journal( date );
    if ( journal ) {
      mResourceMap[journal] = mManager->standardResource();
      return journal;
    }
  }
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal* journal = (*it)->journal( date );
    if ( journal ) {
      mResourceMap[journal] = *it;
      return journal;
    }
  }

  return 0;
}

Journal *CalendarResources::journal(const QString &uid)
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

Journal::List CalendarResources::journals()
{
  kdDebug(5800) << "CalendarResources::journals()" << endl;

  Journal::List result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal::List list = (*it)->journals();
    Journal::List::ConstIterator it2;
    for ( it2 = list.begin(); it2 != list.end(); ++it2 ) {
      result.append( *it2 );
      mResourceMap[ *it2 ] = *it;
    }
  }
  return result;
}


void CalendarResources::incidenceUpdated( IncidenceBase * )
{
  kdDebug() << "CalendarResources::incidenceUpdated( IncidenceBase * ): Not yet implemented\n";
}

void CalendarResources::connectResource( ResourceCalendar *resource )
{
  connect( resource, SIGNAL( resourceChanged( ResourceCalendar * ) ),
           SIGNAL( calendarChanged() ) );
  connect( resource, SIGNAL( resourceSaved( ResourceCalendar * ) ),
           SIGNAL( calendarSaved() ) );
}

ResourceCalendar *CalendarResources::resource(Incidence *inc)
{
  if ( mResourceMap.find(inc)!=mResourceMap.end() ) {
    return mResourceMap[inc];
  }
  return 0;
}

void CalendarResources::resourceAdded( ResourceCalendar *resource )
{
  kdDebug() << "Resource added: " << resource->resourceName() << endl;
}

void CalendarResources::resourceModified( ResourceCalendar *resource )
{
  resourceDeleted( resource );
}

void CalendarResources::resourceDeleted( ResourceCalendar *resource )
{
  typedef QMap<Incidence*, ResourceCalendar*> RMap;
  RMap::Iterator it, old_it;
  for ( it =mResourceMap.begin(); it != mResourceMap.end(); ++it ) {
    if ( it.data() == resource ) {
      if ( it == mResourceMap.begin() ) {
        mResourceMap.remove(it);
        it = mResourceMap.begin();
      } else {
        mResourceMap.remove(it);
        it = old_it;
      }
    }
    old_it = it;
  }

}

#include "calendarresources.moc"
