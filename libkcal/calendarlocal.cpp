/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
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

#include <QDateTime>
#include <QString>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"
#include "filestorage.h"

#include "calendarlocal.h"

using namespace KCal;

CalendarLocal::CalendarLocal( const QString &timeZoneId )
  : Calendar( timeZoneId )
{
  init();
}

void CalendarLocal::init()
{
  mDeletedIncidences.setAutoDelete( true );
  mFileName.clear();
}


CalendarLocal::~CalendarLocal()
{
  close();
}

bool CalendarLocal::load( const QString &fileName, CalFormat *format )
{
  mFileName = fileName;
  FileStorage storage( this, fileName, format );
  return storage.load();
}

bool CalendarLocal::reload( const QString &tz )
{
  const QString filename = mFileName;
  save();
  close();
  mFileName = filename;
  setTimeZoneId( tz );
  FileStorage storage( this, mFileName );
  return storage.load();
}

bool CalendarLocal::save( const QString &fileName, CalFormat *format )
{
  // Save only if the calendar is either modified, or saved to a
  // different file than it was loaded from
  if ( mFileName != fileName || isModified() ) {
    FileStorage storage( this, fileName, format );
    return storage.save();
  } else {
    return true;
  }
}

void CalendarLocal::close()
{
  setObserversEnabled( false );
  mFileName.clear();

  deleteAllEvents();
  deleteAllTodos();
  deleteAllJournals();

  mDeletedIncidences.clear();
  setModified( false );

  setObserversEnabled( true );
}


bool CalendarLocal::addEvent( Event *event )
{
  insertEvent( event );

  event->registerObserver( this );

  setModified( true );

  notifyIncidenceAdded( event );

  return true;
}

bool CalendarLocal::deleteEvent( Event *event )
{
//  kDebug(5800) << "CalendarLocal::deleteEvent" << endl;

  if ( mEvents.remove( event->uid() ) ) {
    setModified( true );
    notifyIncidenceDeleted( event );
    mDeletedIncidences.append( event );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteEvent(): Event not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllEvents()
{
  // kDebug(5800) << "CalendarLocal::deleteAllEvents" << endl;
  foreach ( Event *e, mEvents )
    notifyIncidenceDeleted( e );

  qDeleteAll( mEvents );
  mEvents.clear();
}

Event *CalendarLocal::event( const QString &uid )
{
//  kDebug(5800) << "CalendarLocal::event(): " << uid << endl;
  return mEvents.value( uid );
}

bool CalendarLocal::addTodo( Todo *todo )
{
  mTodoList.append( todo );

  todo->registerObserver( this );

  // Set up subtask relations
  setupRelations( todo );

  setModified( true );

  notifyIncidenceAdded( todo );

  return true;
}

bool CalendarLocal::deleteTodo( Todo *todo )
{
  // Handle orphaned children
  removeRelations( todo );

  if ( mTodoList.removeRef( todo ) ) {
    setModified( true );
    notifyIncidenceDeleted( todo );
    mDeletedIncidences.append( todo );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteTodo(): Todo not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllTodos()
{
  // kDebug(5800) << "CalendarLocal::deleteAllTodos()\n";
  Todo::List::ConstIterator it;
  for( it = mTodoList.begin(); it != mTodoList.end(); ++it ) {
    notifyIncidenceDeleted( *it );
  }

  mTodoList.setAutoDelete( true );
  mTodoList.clear();
  mTodoList.setAutoDelete( false );
}

Todo::List CalendarLocal::rawTodos( TodoSortField sortField,
                                    SortDirection sortDirection )
{
  return sortTodos( &mTodoList, sortField, sortDirection );
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
//  kDebug(5800) << "CalendarLocal::alarms(" << from.toString() << " - "
//                << to.toString() << ")" << endl;

  Alarm::List alarms;

  foreach ( Event *e, mEvents ) {
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
  QDateTime preTime = from.addSecs(-1);
  Alarm::List::ConstIterator it;
  for( it = incidence->alarms().begin(); it != incidence->alarms().end();
       ++it ) {
    if ( (*it)->enabled() ) {
      QDateTime dt = (*it)->nextRepetition(preTime);
      if ( dt.isValid() && dt <= to ) {
        kDebug(5800) << "CalendarLocal::appendAlarms() '"
                      << incidence->summary() << "': "
                      << dt.toString() << endl;
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
  QDateTime qdt;
  int  endOffset = 0;
  bool endOffsetValid = false;
  int  period = from.secsTo(to);
  Alarm::List::ConstIterator it;
  for( it = incidence->alarms().begin(); it != incidence->alarms().end();
       ++it ) {
    Alarm *alarm = *it;
    if ( alarm->enabled() ) {
      if ( alarm->hasTime() ) {
        // The alarm time is defined as an absolute date/time
        qdt = alarm->nextRepetition( from.addSecs(-1) );
        if ( !qdt.isValid() || qdt > to )
          continue;
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
        if ( !qdt.isValid()
        ||   (qdt = qdt.addSecs( offset )) > to )    // remove the adjustment to get the alarm time
        {
          // The next recurrence is too late.
          if ( !alarm->repeatCount() )
            continue;
          // The alarm has repetitions, so check whether repetitions of previous
          // recurrences fall within the time period.
          bool found = false;
          qdt = from.addSecs( -offset );
          while ( (qdt = incidence->recurrence()->getPreviousDateTime( qdt )).isValid() ) {
            int toFrom = qdt.secsTo( from ) - offset;
            if ( toFrom > alarm->duration() )
              break;     // this recurrence's last repetition is too early, so give up
            // The last repetition of this recurrence is at or after 'from' time.
            // Check if a repetition occurs between 'from' and 'to'.
            int snooze = alarm->snoozeTime() * 60;   // in seconds
            if ( period >= snooze
            ||   toFrom % snooze == 0
            ||   (toFrom / snooze + 1) * snooze <= toFrom + period ) {
              found = true;
#ifndef NDEBUG
              qdt = qdt.addSecs( offset + ((toFrom-1) / snooze + 1) * snooze );   // for debug output
#endif
              break;
            }
          }
          if ( !found )
            continue;
        }
      }
      kDebug(5800) << "CalendarLocal::appendAlarms() '" << incidence->summary()
                    << "': " << qdt.toString() << endl;
      alarms.append( alarm );
    }
  }
}


void CalendarLocal::insertEvent( Event *event )
{
  QString uid = event->uid();
  if ( mEvents.value( uid ) == 0 ) {
    mEvents.insert( uid, event );
  }
#ifndef NDEBUG
  else // if we already have an event with this UID, it has to be the same event,
      // otherwise something's really broken
      Q_ASSERT( mEvents.value( uid ) == event );
#endif
}

Event::List CalendarLocal::rawEventsForDate( const QDate &qd,
                                             EventSortField sortField,
                                             SortDirection sortDirection )
{
  Event::List eventList;

  foreach ( Event *event, mEvents ) {

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
      if ( event->dtStart().date() <= qd && event->dateEnd() >= qd ) {
        eventList.append( event );
      }
    }
  }

  return sortEvents( &eventList, sortField, sortDirection );
}

Event::List CalendarLocal::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  Event::List eventList;

  // Get non-recurring events
  foreach ( Event *event, mEvents ) {
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

Event::List CalendarLocal::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  Event::List eventList;
  foreach ( Event *e, mEvents )
    eventList.append( e );
  return sortEvents( &eventList, sortField, sortDirection );
}

bool CalendarLocal::addJournal(Journal *journal)
{
  if (journal->dtStart().isValid())
    kDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  else
    kDebug(5800) << "Adding Journal without a DTSTART" << endl;

  mJournalList.append(journal);

  journal->registerObserver( this );

  setModified( true );

  notifyIncidenceAdded( journal );

  return true;
}

bool CalendarLocal::deleteJournal( Journal *journal )
{
  if ( mJournalList.removeRef( journal ) ) {
    setModified( true );
    notifyIncidenceDeleted( journal );
    mDeletedIncidences.append( journal );
    return true;
  } else {
    kWarning() << "CalendarLocal::deleteJournal(): Journal not found." << endl;
    return false;
  }
}

void CalendarLocal::deleteAllJournals()
{
  Journal::List::ConstIterator it;
  for( it = mJournalList.begin(); it != mJournalList.end(); ++it ) {
    notifyIncidenceDeleted( *it );
  }

  mJournalList.setAutoDelete( true );
  mJournalList.clear();
  mJournalList.setAutoDelete( false );
}

Journal *CalendarLocal::journal( const QString &uid )
{
  Journal::List::ConstIterator it;
  for ( it = mJournalList.begin(); it != mJournalList.end(); ++it )
    if ( (*it)->uid() == uid )
      return *it;

  return 0;
}

Journal::List CalendarLocal::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return sortJournals( &mJournalList, sortField, sortDirection );
}

Journal::List CalendarLocal::rawJournalsForDate( const QDate &date )
{
  Journal::List journals;

  Journal::List::ConstIterator it;
  for ( it = mJournalList.begin(); it != mJournalList.end(); ++it ) {
    Journal *journal = *it;
    if ( journal->dtStart().date() == date ) {
      journals.append( journal );
    }
  }

  return journals;
}

void CalendarLocal::setTimeZoneIdViewOnly( const QString& tz )
{
  const QString question( i18n("The time zone setting was changed. In order to display the calendar "
      "you are looking at in the new time zone, it needs to be saved. Do you want to save the pending "
      "changes or rather wait and apply the new time zone on the next reload?" ) );
  int rc = KMessageBox::Yes;
  if ( isModified() ) {
    rc = KMessageBox::questionYesNo( 0, question,
                                       i18n("Save before applying time zones?"),
                                       KStdGuiItem::save(),
                                       KGuiItem(i18n("Apply Time Zone Change on Next Reload")),
                                       "calendarLocalSaveBeforeTimeZoneShift");
  }
  if ( rc == KMessageBox::Yes ) {
    reload( tz );
  }
}
