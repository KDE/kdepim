/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <kdebug.h>

#include "calendarsyncee.h"

using namespace KSync;
using namespace KCal;

CalendarSyncEntry::CalendarSyncEntry( Incidence *incidence, Syncee *parent )
  : SyncEntry( parent ), mIncidence( incidence )
{
}

QString CalendarSyncEntry::name()
{
  return mIncidence->summary();
}

QString CalendarSyncEntry::id()
{
  return mIncidence->uid();
}

QString CalendarSyncEntry::timestamp()
{
  // FIXME: last modified isn't sufficient to tell if an event has changed.
  return mIncidence->lastModified().toString();
}

bool CalendarSyncEntry::equals( SyncEntry *entry )
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>(entry);
  if (!calEntry) {
    kdDebug() << "CalendarSyncee::addEntry(): Wrong type." << endl;
    return false;
  }

  kdDebug() << "UID: " << mIncidence->uid() << " <-> "
            << calEntry->incidence()->uid() << endl;
  kdDebug() << "LAM: " << mIncidence->lastModified().toTime_t() << " <-> "
            << calEntry->incidence()->lastModified().toTime_t() << endl;

  if ( mIncidence->uid() != calEntry->incidence()->uid() ) {
    kdDebug() << "UID unequal" << endl;
    return false;
  }
  if ( mIncidence->lastModified() != calEntry->incidence()->lastModified() ) {
    kdDebug() << "LAM unequal" << endl;
    return false;
  }

  if ( *mIncidence == *( calEntry->incidence() ) ) return true;

  return false;
}

CalendarSyncEntry *CalendarSyncEntry::clone()
{
  return new CalendarSyncEntry( *this );
}

CalendarSyncee::CalendarSyncee()
  : mOwnCalendar( true ), mIteratingEvents( true )
{
  mCalendar = new CalendarLocal;
}

CalendarSyncee::CalendarSyncee( CalendarLocal *calendar )
  : mOwnCalendar( false ), mIteratingEvents( true )
{
  mCalendar = calendar;
}

CalendarSyncee::~CalendarSyncee()
{
  clearEntries();

  if ( mOwnCalendar ) delete mCalendar;
}

void CalendarSyncee::reset()
{
  clearEntries();
}

void CalendarSyncee::clearEntries()
{
  QMap<Incidence *, CalendarSyncEntry *>::Iterator it;
  for( it = mEntries.begin(); it != mEntries.end(); ++it ) {
    delete it.data();
  }
  mEntries.clear();
}

CalendarSyncEntry *CalendarSyncee::firstEntry()
{
  mEvents = mCalendar->events();
  mCurrentEvent = mEvents.begin();
  mIteratingEvents = true;
  if( mCurrentEvent == mEvents.end() ) {
    mTodos = mCalendar->todos();
    mCurrentTodo = mTodos.begin();
    mIteratingEvents = false;
    if( mCurrentTodo == mTodos.end() ) {
      return 0;
    }
    return createEntry( *mCurrentTodo );
  }
  return createEntry( *mCurrentEvent );
}

CalendarSyncEntry *CalendarSyncee::nextEntry()
{
  if( mIteratingEvents ) {
    ++mCurrentEvent;
    if ( mCurrentEvent == mEvents.end() ) {
      mTodos = mCalendar->todos();
      mCurrentTodo = mTodos.begin();
      mIteratingEvents = false;
      if( mCurrentTodo == mTodos.end() ) {
          return 0;
      }
      return createEntry( *mCurrentTodo );
    }
    return createEntry( *mCurrentEvent );
  } else {
    ++mCurrentTodo;
    if( mCurrentTodo == mTodos.end() ) {
      return 0;
    }
    return createEntry( *mCurrentTodo );
  }
}

#if 0
CalendarSyncEntry *CalendarSyncee::findEntry(const QString &id)
{
  Event *event = mCalendar->getEvent(id);
  return createEntry(event);
}
#endif

void CalendarSyncee::addEntry( SyncEntry *entry )
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>(entry);
  if (!calEntry) {
    kdDebug() << "CalendarSyncee::addEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    Event *sourceEvent = dynamic_cast<Event *>(calEntry->incidence());
    if (!sourceEvent) {
        Todo *sourceTodo = dynamic_cast<Todo*>(calEntry->incidence());
        if(!sourceTodo) {
           kdDebug() << "CalendarSyncee::addEntry(): Incidence is not of type Event or Todo."
                << endl;
        }
        Todo *todo = dynamic_cast<Todo *>(sourceTodo->clone());
        mCalendar->addTodo(todo);
    } else {
      Event *event = dynamic_cast<Event *>(sourceEvent->clone());
      mCalendar->addEvent(event);
    }
  }
}

void CalendarSyncee::removeEntry( SyncEntry *entry )
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>( entry );
  if ( !calEntry ) {
    kdDebug() << "CalendarSyncee::removeEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    Event *ev = dynamic_cast<Event *>( calEntry->incidence() );
    if ( ev ) {
      mCalendar->deleteEvent( ev );
    } else {
      Todo *td = dynamic_cast<Todo*>( calEntry->incidence() );
      if( !td ) {
        kdDebug() << "CalendarSyncee::removeEntry(): Incidence has wrong type."
                  << endl;
      }
      mCalendar->deleteTodo( td );
    }
  }
}

CalendarSyncEntry *CalendarSyncee::createEntry( Incidence *incidence )
{
  if ( incidence ) {
    QMap<Incidence *,CalendarSyncEntry *>::ConstIterator it;
    it = mEntries.find( incidence );
    if ( it != mEntries.end() ) return it.data();

    CalendarSyncEntry *entry = new CalendarSyncEntry( incidence, this );
    mEntries.insert( incidence, entry );
    return entry;
  } else {
    return 0;
  }
}

bool CalendarSyncee::writeBackup( const QString &filename )
{
  return mCalendar->save( filename );
}

bool CalendarSyncee::restoreBackup( const QString &filename )
{
  mCalendar->close();
  clearEntries();
  return mCalendar->load( filename );
}
