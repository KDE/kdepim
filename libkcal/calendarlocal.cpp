/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"
#include "filestorage.h"

#include "calendarlocal.h"

using namespace KCal;

CalendarLocal::CalendarLocal()
  : Calendar(), mEvents( 47 )
{
  init();
}

CalendarLocal::CalendarLocal(const QString &timeZoneId)
  : Calendar(timeZoneId), mEvents( 47 )
{
  init();
}

void CalendarLocal::init()
{
}


CalendarLocal::~CalendarLocal()
{
  close();
}

bool CalendarLocal::load( const QString &fileName )
{
  FileStorage storage( this, fileName );
  return storage.load();
}

bool CalendarLocal::save( const QString &fileName, CalFormat *format )
{
  FileStorage storage( this, fileName, format );
  return storage.save();
}

void CalendarLocal::close()
{
  deleteAllEvents();
  deleteAllTodos();
  deleteAllJournals();

  setModified( false );
}


bool CalendarLocal::addEvent( Event *event )
{
  insertEvent( event );

  event->registerObserver( this );

  setModified( true );

  return true;
}

void CalendarLocal::deleteEvent( Event *event )
{
  kdDebug(5800) << "CalendarLocal::deleteEvent" << endl;

  if ( mEvents.remove( event->uid() ) ) {
    setModified( true );
  } else {
    kdWarning() << "CalendarLocal::deleteEvent(): Event not found." << endl;
  }
}

void CalendarLocal::deleteAllEvents()
{
  // kdDebug(5800) << "CalendarLocal::deleteAllEvents" << endl;
  mEvents.setAutoDelete( true );
  mEvents.clear();
  mEvents.setAutoDelete( false );
}

Event *CalendarLocal::event( const QString &uid )
{
//  kdDebug(5800) << "CalendarLocal::event(): " << uid << endl;
  return mEvents[ uid ];
}

bool CalendarLocal::addTodo( Todo *todo )
{
  mTodoList.append( todo );

  todo->registerObserver( this );

  // Set up subtask relations
  setupRelations( todo );

  setModified( true );

  return true;
}

void CalendarLocal::deleteTodo( Todo *todo )
{
  // Handle orphaned children
  removeRelations( todo );

  if ( mTodoList.removeRef( todo ) ) {
    setModified( true );
  }
}

void CalendarLocal::deleteAllTodos()
{
  // kdDebug(5800) << "CalendarLocal::deleteAllTodos()\n";
  mTodoList.setAutoDelete( true );
  mTodoList.clear();
  mTodoList.setAutoDelete( false );
}

Todo::List CalendarLocal::rawTodos()
{
  return mTodoList;
}

Todo *CalendarLocal::todo( const QString &uid )
{
  Todo::List::ConstIterator it;
  for ( it = mTodoList.begin(); it != mTodoList.end(); ++it ) {
    if ( (*it)->uid() == uid ) return *it;
  }

  return 0;
}

Todo::List CalendarLocal::rawTodosForDate( const QDate &date )
{
  Todo::List todos;

  Todo::List::ConstIterator it;
  for ( it = mTodoList.begin(); it != mTodoList.end(); ++it ) {
    Todo *todo = *it;
    if ( todo->hasDueDate() && todo->dtDue().date() == date ) {
      todos.append( todo );
    }
  }

  return todos;
}

Alarm::List CalendarLocal::alarmsTo( const QDateTime &to )
{
  return alarms( QDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List CalendarLocal::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "CalendarLocal::alarms(" << from.toString() << " - "
//                << to.toString() << ")" << endl;

  Alarm::List alarms;

  EventDictIterator it( mEvents );
  for( ; it.current(); ++it ) {
    Event *e = *it;
    if ( e->doesRecur() ) appendRecurringAlarms( alarms, e, from, to );
    else appendAlarms( alarms, e, from, to );
  }

  Todo::List::ConstIterator it2;
  for( it2 = mTodoList.begin(); it2 != mTodoList.end(); ++it2 ) {
    if (! (*it2)->isCompleted() ) appendAlarms( alarms, *it2, from, to );
  }

  return alarms;
}

void CalendarLocal::appendAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  Alarm::List::ConstIterator it;
  for( it = incidence->alarms().begin(); it != incidence->alarms().end();
       ++it ) {
//    kdDebug(5800) << "CalendarLocal::appendAlarms() '" << alarm->text()
//                  << "': " << alarm->time().toString() << " - " << alarm->enabled() << endl;
    if ( (*it)->enabled() ) {
      if ( (*it)->time() >= from && (*it)->time() <= to ) {
        kdDebug(5800) << "CalendarLocal::appendAlarms() '"
                      << incidence->summary() << "': "
                      << (*it)->time().toString() << endl;
        alarms.append( *it );
      }
    }
  }
}

void CalendarLocal::appendRecurringAlarms( Alarm::List &alarms,
                                           Incidence *incidence,
                                           const QDateTime &from,
                                           const QDateTime &to )
{
  Alarm::List::ConstIterator it;
  QDateTime qdt;
  int  endOffset = 0;
  bool endOffsetValid = false;
  for( it = incidence->alarms().begin(); it != incidence->alarms().end();
       ++it ) {
    Alarm *alarm = *it;
    if ( alarm->hasTime() ) {
      // The alarm time is defined as an absolute date/time
      qdt = alarm->time();
    } else {
      // The alarm time is defined by an offset from the event start or end time.
      // Find the offset from the event start time, which is also used as the
      // offset from the recurrence time.
      int offset = 0;
      if ( alarm->hasStartOffset() ) {
        offset = alarm->startOffset().asSeconds();
      } else if ( alarm->hasEndOffset() ) {
        if ( !endOffsetValid ) {
          endOffset = incidence->dtStart().secsTo( incidence->dtEnd() );
          endOffsetValid = true;
        }
        offset = alarm->endOffset().asSeconds() + endOffset;
      }
      // Adjust the 'from' date/time and find the next recurrence at or after it
      qdt = incidence->recurrence()->getNextDateTime( from.addSecs(-offset - 1) );
      if (!qdt.isValid() || incidence->isException(qdt.date()) )
        continue;
      // Remove the adjustment to get the alarm time
      qdt = qdt.addSecs( offset );
    }
    kdDebug(5800) << "CalendarLocal::appendAlarms() '" << incidence->summary()
                  << "': " << qdt.toString() << " - " << (*it)->enabled()
                  << endl;
    if ( (*it)->enabled() ) {
//      kdDebug(5800) << "CalendarLocal::appendAlarms() '" << incidence->summary()
//                    << "': " << (*it)->time().toString() << endl;
      if ( qdt >= from && qdt <= to ) {
        alarms.append( *it );
      }
    }
  }
}


// after changes are made to an event, this should be called.
void CalendarLocal::update( IncidenceBase *incidence )
{
  incidence->setSyncStatus( Event::SYNCMOD );
  incidence->setLastModified( QDateTime::currentDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  setModified( true );
}

void CalendarLocal::insertEvent( Event *event )
{
  QString uid = event->uid();
  if ( mEvents[ uid ] == 0 ) {
    mEvents.insert( uid, event );
  }
#ifndef NDEBUG
  else // if we already have an event with this UID, it has to be the same event,
      // otherwise something's really broken
      Q_ASSERT( mEvents[uid] == event );
#endif
}


Event::List CalendarLocal::rawEventsForDate( const QDate &qd, bool sorted )
{
  Event::List eventList;

  EventDictIterator it( mEvents );
  for( ; it.current(); ++it ) {
    Event *event = *it;

    if ( event->doesRecur() ) {
      if ( event->isMultiDay() ) {
        int extraDays = event->dtStart().date().daysTo( event->dtEnd().date() );
        int i;
        for ( i = 0; i <= extraDays; i++ ) {
	  if ( event->recursOn( qd.addDays( -i ) ) ) {
            eventList.append( event );
            break;
	  }
        }
      } else {
        if ( event->recursOn( qd ) )
          eventList.append( event );
      }
    } else {
      if ( event->dtStart().date() <= qd && event->dtEnd().date() >= qd ) {
        eventList.append( event );
      }
    }
  }

  if ( !sorted ) {
    return eventList;
  }

  //  kdDebug(5800) << "Sorting events for date\n" << endl;
  // now, we have to sort it based on dtStart.time()
  Event::List eventListSorted;
  Event::List::Iterator sortIt;
  Event::List::Iterator eit;
  for ( eit = eventList.begin(); eit != eventList.end(); ++eit ) {
    sortIt = eventListSorted.begin();
    while ( sortIt != eventListSorted.end() &&
            (*eit)->dtStart().time() >= (*sortIt)->dtStart().time() ) {
      ++sortIt;
    }
    eventListSorted.insert( sortIt, *eit );
  }
  return eventListSorted;
}


Event::List CalendarLocal::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  Event::List eventList;

  // Get non-recurring events
  EventDictIterator it( mEvents );
  for( ; it.current(); ++it ) {
    Event *event = *it;
    if ( event->doesRecur() ) {
      QDate rStart = event->dtStart().date();
      bool found = false;
      if ( inclusive ) {
        if ( rStart >= start && rStart <= end ) {
          // Start date of event is in range. Now check for end date.
          // if duration is negative, event recurs forever, so do not include it.
          if ( event->recurrence()->duration() == 0 ) {  // End date set
            QDate rEnd = event->recurrence()->endDate();
            if ( rEnd >= start && rEnd <= end ) {  // End date within range
              found = true;
            }
          } else if ( event->recurrence()->duration() > 0 ) {  // Duration set
            // TODO: Calculate end date from duration. Should be done in Event
            // For now exclude all events with a duration.
          }
        }
      } else {
        if ( rStart <= end ) {  // Start date not after range
          if ( rStart >= start ) {  // Start date within range
            found = true;
          } else if ( event->recurrence()->duration() == -1 ) {  // Recurs forever
            found = true;
          } else if ( event->recurrence()->duration() == 0 ) {  // End date set
            QDate rEnd = event->recurrence()->endDate();
            if ( rEnd >= start && rEnd <= end ) {  // End date within range
              found = true;
            }
          } else {  // Duration set
            // TODO: Calculate end date from duration. Should be done in Event
            // For now include all events with a duration.
            found = true;
          }
        }
      }

      if ( found ) eventList.append( event );
    } else {
      QDate s = event->dtStart().date();
      QDate e = event->dtEnd().date();

      if ( inclusive ) {
        if ( s >= start && e <= end ) {
          eventList.append( event );
        }
      } else {
        if ( ( s >= start && s <= end ) || ( e >= start && e <= end ) ) {
          eventList.append( event );
        }
      }
    }
  }

  return eventList;
}

Event::List CalendarLocal::rawEventsForDate( const QDateTime &qdt )
{
  return rawEventsForDate( qdt.date() );
}

// This bool is only used by the regression testing program, to save in a stable order
bool KCal_CalendarLocal_saveOrdered = false;

Event::List CalendarLocal::rawEvents()
{
  Event::List eventList;
  if ( !KCal_CalendarLocal_saveOrdered ) { // normal case: save in random order
    EventDictIterator it( mEvents );
    for( ; it.current(); ++it )
      eventList.append( *it );
  } else { // regression testing: save in sorted order
    Event::List::Iterator sortIt;
    EventDictIterator it( mEvents );
    for( ; it.current(); ++it ) {
      sortIt = eventList.begin();
      while ( sortIt != eventList.end() &&
              it.current()->dtStart().time() >= (*sortIt)->dtStart().time() ) {
        ++sortIt;
      }
      eventList.insert( sortIt, it.current() );
    }
  }
  return eventList;
}

bool CalendarLocal::addJournal(Journal *journal)
{
  if (journal->dtStart().isValid())
    kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  else
    kdDebug(5800) << "Adding Journal without a DTSTART" << endl;

  mJournalList.append(journal);

  journal->registerObserver( this );

  setModified( true );

  return true;
}

void CalendarLocal::deleteJournal( Journal *journal )
{
  if ( mJournalList.removeRef(journal) ) {
    setModified( true );
  }
}

void CalendarLocal::deleteAllJournals()
{
  mJournalList.setAutoDelete( true );
  mJournalList.clear();
  mJournalList.setAutoDelete( false );
}

Journal *CalendarLocal::journal( const QDate &date )
{
//  kdDebug(5800) << "CalendarLocal::journal() " << date.toString() << endl;

  Journal::List::ConstIterator it;
  for ( it = mJournalList.begin(); it != mJournalList.end(); ++it )
    if ( (*it)->dtStart().date() == date )
      return *it;

  return 0;
}

Journal *CalendarLocal::journal( const QString &uid )
{
  Journal::List::ConstIterator it;
  for ( it = mJournalList.begin(); it != mJournalList.end(); ++it )
    if ( (*it)->uid() == uid )
      return *it;

  return 0;
}

Journal::List CalendarLocal::journals()
{
  return mJournalList;
}

