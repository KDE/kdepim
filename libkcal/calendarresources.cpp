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

#include <kresources/resourcemanager.h>

#include "resourcecalendar.h"
#include "resourcelocal.h"

#include "calendarresources.h"

using namespace KCal;

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

  mManager = new CalendarResourceManager( "calendar" );

#if 0
  if ( mManager->isEmpty() ) {
    QString fileName = locateLocal( "data", "kcal/std.ics" );

    kdDebug() << "DEFAULT: " << fileName << endl;
    
    ResourceCalendar *defaultResource = new ResourceLocal( fileName );
    defaultResource->setResourceName( i18n("Default calendar resource") );
    
    mManager->add( defaultResource );
    mManager->setStandardResource( defaultResource );
  }
#endif

  if ( !mManager->standardResource() ) {
    kdDebug() << "Warning! No standard resource yet." << endl;
  }

  // Open all active resources
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    kdDebug(5800) << "Opening resource " + (*it)->resourceName() << endl;
    bool result = (*it)->open();
    // Really should remove resource if open not successful
  }

  mOpen = true;
}


CalendarResources::~CalendarResources()
{
  kdDebug(5800) << "CalendarResources::destructor" << endl;

  close();

  delete mManager;
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

void CalendarResources::sync()
{
  kdDebug(5800) << "CalendarResources::sync()" << endl;

  if ( mOpen ) {
    CalendarResourceManager::ActiveIterator it;
    for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
      (*it)->sync();
    }

    setModified( false );
  }
}

void CalendarResources::addEvent(Event *anEvent)
{
  kdDebug(5800) << "CalendarResources::addEvent" << endl;

  if ( mManager->standardResource() ) {
    mManager->standardResource()->addEvent( anEvent );
  } else {
    kdDebug() << "FIXME: We don't have a standard resource. Adding events isn't going to work" << endl;
  }

  setModified( true );
}

void CalendarResources::deleteEvent(Event *event)
{
  kdDebug(5800) << "CalendarResources::deleteEvent" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    (*it)->deleteEvent( event );
  }

  setModified( true );
}


Event *CalendarResources::event( const QString &uid )
{
//  kdDebug(5800) << "CalendarResources::event(): " << uid << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Event* event = (*it)->event( uid );
    if ( event ) return event;
  }

  // Not found
  return 0;
}

void CalendarResources::addTodo(Todo *todo)
{
  kdDebug(5800) << "CalendarResources::addTodo" << endl;

  if ( mManager->standardResource() ) {
    mManager->standardResource()->addTodo( todo );
  } else {
    kdDebug() << "FIXME: We don't have a standard resource. Adding todos isn't going to work" << endl;
  }

  setModified( true );
}

void CalendarResources::deleteTodo(Todo *todo)
{
  kdDebug(5800) << "CalendarResources::deleteTodo" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    (*it)->deleteTodo( todo );
  }

  setModified( true );
}

QPtrList<Todo> CalendarResources::rawTodos() const
{
  kdDebug(5800) << "CalendarResources::rawTodos()" << endl;

  QPtrList<Todo> result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    kdDebug(5800) << "Getting raw todos from '" << (*it)->resourceName()
                  << "'" << endl;
    QPtrList<Todo> todos = (*it)->rawTodos();
    Todo *todo;
    for ( todo = todos.first(); todo; todo = todos.next() ) {
      kdDebug(5800) << "Adding todo to result" << endl;
      result.append( todo );
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
    if ( todo ) return todo;
  }

  // not found
  return 0;
}

QPtrList<Todo> CalendarResources::todos( const QDate &date )
{
  kdDebug(5800) << "CalendarResources::todos(date)" << endl;

  QPtrList<Todo> result;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    QPtrList<Todo> todos = (*it)->todos( date );
    Todo* todo;
    for ( todo = todos.first(); todo; todo = todos.next() )
      result.append( todo );
  }

  return result;
}

int CalendarResources::numEvents(const QDate &qd)
{
  kdDebug(5800) << "CalendarResources::numEvents" << endl;

  int count = 0;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    count += (*it)->numEvents( qd );
  }
  return count;
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
QPtrList<Event> CalendarResources::rawEventsForDate(const QDate &qd, bool sorted)
{
//  kdDebug(5800) << "CalendarResources::rawEventsForDate()" << endl;

  QPtrList<Event> result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
//    kdDebug() << "Getting events from '" << (*it)->resourceName() << "'"
//              << endl;
    QPtrList<Event> list = (*it)->rawEventsForDate( qd, sorted );
    if ( sorted ) {
      Event* item;
      uint insertionPoint = 0;
      for ( item = list.first(); item; item = list.next() ) {
        while ( insertionPoint<result.count() && 
                result.at( insertionPoint )->dtStart().time() <= item->dtStart().time() ) 
          insertionPoint++;
        result.insert( insertionPoint, item );
      }
    } else {
      Event* item;
      for ( item = list.first(); item; item = list.next() ) {
        result.append( item );
      }
    }
  }
  return result;
}

QPtrList<Event> CalendarResources::rawEvents( const QDate &start,
                                              const QDate &end,
                                              bool inclusive )
{
  kdDebug(5800) << "CalendarResources::rawEvents(start,end,inclusive)" << endl;

  QPtrList<Event> result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    QPtrList<Event> list = (*it)->rawEvents( start, end, inclusive );
    Event* item;
    for ( item = list.first(); item; item = list.next() ) {
      result.append( item );
    }
  }
  return result;
}

QPtrList<Event> CalendarResources::rawEventsForDate(const QDateTime &qdt)
{
  kdDebug(5800) << "CalendarResources::rawEventsForDate(qdt)" << endl;

  QPtrList<Event> result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    QPtrList<Event> list = (*it)->rawEventsForDate( qdt );
    Event* item;
    for ( item = list.first(); item; item = list.next() ) {
      result.append( item );
    }
  }
  return result;
}

QPtrList<Event> CalendarResources::rawEvents()
{
  kdDebug(5800) << "CalendarResources::rawEvents()" << endl;

  QPtrList<Event> result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    QPtrList<Event> list = (*it)->rawEvents();
    Event* item;
    for ( item = list.first(); item; item = list.next() ) {
      result.append( item );
    }
  }
  return result;
}


void CalendarResources::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  if ( mManager->standardResource() ) {
    mManager->standardResource()->addJournal( journal );
  } else {
    kdDebug() << "FIXME: We don't have a standard resource. Adding journals isn't going to work" << endl;
  }

  setModified( true );
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
    if ( journal ) return journal;
  }
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal* journal = (*it)->journal( date );
    if ( journal ) return journal;
  }

  return 0;
}

Journal *CalendarResources::journal(const QString &uid)
{
  kdDebug(5800) << "CalendarResources::journal(uid)" << endl;

  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    Journal* journal = (*it)->journal( uid );
    if ( journal ) return journal;
  }

  // not found
  return 0;
}

QPtrList<Journal> CalendarResources::journals()
{
  kdDebug(5800) << "CalendarResources::journals()" << endl;

  QPtrList<Journal> result;
  CalendarResourceManager::ActiveIterator it;
  for ( it = mManager->activeBegin(); it != mManager->activeEnd(); ++it ) {
    QPtrList<Journal> list = (*it)->journals();
    Journal* item;
    for ( item = list.first(); item; item = list.next() ) {
      result.append( item );
    }
  }
  return result;
}


void CalendarResources::incidenceUpdated( IncidenceBase * )
{
  kdDebug() << "CalendarResources::incidenceUpdated( IncidenceBase * ): Not yet implemented\n";
}
