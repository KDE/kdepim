/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Holger Hans Peter Freyther <freyther@kde.org>

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

#include "calendarsyncee.h"
#include "calendarmerger.h"

#include <libkcal/filestorage.h>
#include <libkcal/calformat.h>
#include <libkdepim/calendardiffalgo.h>

#include <kdebug.h>


using namespace KSync;
using namespace KCal;

CalendarSyncEntry::CalendarSyncEntry( Incidence *incidence, Syncee *parent )
  : SyncEntry( parent ), mIncidence( incidence )
{
  setType( QString::fromLatin1( "CalendarSyncEntry" ) );
}

CalendarSyncEntry::CalendarSyncEntry( Syncee* parent )
  : SyncEntry( parent )
{
  /* that is hard, use todo or calendar as the default? */
  mIncidence = new KCal::Todo;
  setType( QString::fromLatin1( "CalendarSyncEntry" ) );
}


CalendarSyncEntry::CalendarSyncEntry( const CalendarSyncEntry& ent )
  : SyncEntry( ent ), mIncidence( ent.mIncidence->clone() )
{}

QString CalendarSyncEntry::name()
{
  return mIncidence->summary();
}

QString CalendarSyncEntry::id()
{
  return mIncidence->uid();
}

void CalendarSyncEntry::setId(const QString& id)
{
  mIncidence->setUid( id );
}

QString CalendarSyncEntry::timestamp()
{
  // FIXME: last modified isn't sufficient to tell if an event has changed.
  return mIncidence->lastModified().toString();
}

KCal::Incidence *CalendarSyncEntry::incidence()const {
    return mIncidence;
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

KPIM::DiffAlgo* CalendarSyncEntry::diffAlgo( SyncEntry *syncEntry, SyncEntry *targetEntry )
{
  CalendarSyncEntry *calSyncEntry = dynamic_cast<CalendarSyncEntry*>( syncEntry );
  CalendarSyncEntry *calTargetEntry = dynamic_cast<CalendarSyncEntry*>( targetEntry );

  if ( !calSyncEntry || !calTargetEntry )
    return 0;

  return new KPIM::CalendarDiffAlgo( calSyncEntry->incidence(), calTargetEntry->incidence() );
}


/// Syncee starts here
CalendarSyncee::CalendarSyncee( Calendar *calendar, CalendarMerger* merger )
    : Syncee( merger ), mIteratingEvents( true )
{
  setType( QString::fromLatin1( "CalendarSyncee" ) );
  mCalendar = calendar;
}

CalendarSyncee::~CalendarSyncee()
{
  clearEntries();
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
  if (calEntry) {
    Event *sourceEvent = dynamic_cast<Event *>(calEntry->incidence());
    if (!sourceEvent) {
        Todo *sourceTodo = dynamic_cast<Todo*>(calEntry->incidence());
        if(!sourceTodo) {
           kdDebug() << "CalendarSyncee::addEntry(): Incidence is not of type Event or Todo."
                << endl;
        }
        Todo *todo = dynamic_cast<Todo *>(sourceTodo);
        mCalendar->addTodo(todo);
    } else {
      Event *event = dynamic_cast<Event *>(sourceEvent);
      mCalendar->addEvent(event);
    }
    /* do not lose the syncStatus and insert the Entry directly */
    mEntries.insert(calEntry->incidence(), calEntry);
  }
}

void CalendarSyncee::removeEntry( SyncEntry *entry )
{
  CalendarSyncEntry *calEntry = dynamic_cast<CalendarSyncEntry *>( entry );
  if ( calEntry ) {
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
    mEntries.remove( calEntry->incidence() );
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
  KCal::FileStorage storage( mCalendar, filename );

  bool ok = true;
  ok = ok && storage.open();
  ok = ok && storage.save();
  ok = ok && storage.close();

  return ok;
}

bool CalendarSyncee::restoreBackup( const QString &filename )
{
  mCalendar->close();

  KCal::FileStorage storage( mCalendar, filename );

  bool ok = true;
  ok = ok && storage.open();
  ok = ok && storage.load();
  ok = ok && storage.close();

  clearEntries();

  return ok;
}

QString CalendarSyncee::generateNewId() const {
    return KCal::CalFormat::createUniqueId();
}
