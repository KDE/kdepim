/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

// $Id$

#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
// #include <kio/netdavaccess.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"

#include "calendar.h"

#include "exchangeclient.h"
#include "exchangeaccount.h"

#include "exchangecalendar.h"

using namespace KCal;

ExchangeCalendar::ExchangeCalendar( KPIM::ExchangeAccount* account )
  : Calendar()
{
  kdDebug() << "Creating ExchangeCalendar" << endl;
  init( account );
}

/*
ExchangeCalendar::ExchangeCalendar(const QString &timeZoneId)
  : Calendar(timeZoneId)
{
  init();
}
*/

void ExchangeCalendar::init( KPIM::ExchangeAccount* account )
{
  mAccount = account;
  mClient = new KPIM::ExchangeClient( account );
}


ExchangeCalendar::~ExchangeCalendar()
{
  close();
  delete mClient;
}


bool ExchangeCalendar::load(const QString &fileName)
{
  setModified( false );

  return true;
}

bool ExchangeCalendar::save(const QString &fileName,CalFormat *format)
{
  bool success;

  if (format) {
    success = format->save( this, fileName);
  } else {
    CalFormat *format = new ICalFormat;
    success = format->save( this, fileName);
    delete format;
  }

  if ( success ) setModified( false );
  
  return success;
}

void ExchangeCalendar::close()
{
/*
  QIntDictIterator<QPtrList<Event> > qdi(*mCalDict);
  QPtrList<Event> *tmpList;

  // Delete non-recurring events
  qdi.toFirst();
  while (qdi.current()) {
    tmpList = qdi.current();
    QDate keyDate = keyToDate(qdi.currentKey());
    Event *ev;
    for(ev = tmpList->first();ev;ev = tmpList->next()) {
//      kdDebug(5800) << "-----FIRST.  " << ev->summary() << endl;
//      kdDebug(5800) << "---------MUL: " << (ev->isMultiDay() ? "Ja" : "Nein") << endl;
      bool del = false;
      if (ev->isMultiDay()) {
        if (ev->dtStart().date() == keyDate) {
          del = true;
        }
      } else {
        del = true;
      }
      if (del) {
//        kdDebug(5800) << "-----DEL  " << ev->summary() << endl;
        delete ev;
      }
    }
    ++qdi;
  }

  mCalDict->clear();
  mRecursList.clear();
  mTodoList.clear();

  // reset oldest/newest date markers
  delete mOldestDate;
  mOldestDate = 0L;
  delete mNewestDate;
  mNewestDate = 0L;
*/
  setModified( false );
}


void ExchangeCalendar::addEvent(Event *anEvent)
{
  insertEvent(anEvent);
  if (anEvent->organizer() != getEmail()) {
    kdDebug(5800) << "Event " << anEvent->summary() << " Organizer: " << anEvent->organizer()
              << " Email: " << getEmail() << endl;
//    anEvent->setReadOnly(true);
  }
  
  anEvent->registerObserver( this );

  setModified( true );
}

void ExchangeCalendar::deleteEvent(Event *event)
{
  kdDebug(5800) << "ExchangeCalendar::deleteEvent" << endl;

  // Delete event from Exchange store, by looking at uid

  setModified( true );
}

Event *ExchangeCalendar::event(const QString &uid)
{
  kdDebug(5800) << "ExchangeCalendar::getEvent(uid): " << uid << endl;

  Event *anEvent;

  return anEvent;
  
  // catch-all.
  // return (Event *) 0L;
}

/*
void ExchangeCalendar::addTodo(Todo *todo)
{
  mTodoList.append(todo);

  todo->registerObserver( this );

  setModified( true );
}

void ExchangeCalendar::deleteTodo(Todo *todo)
{
  mTodoList.findRef(todo);
  mTodoList.remove();

  setModified( true );
}


const QPtrList<Todo> &ExchangeCalendar::getTodoList() const
{
  return mTodoList;
}

Todo *ExchangeCalendar::todo(const QString &UniqueStr)
{
  Todo *aTodo;
  for (aTodo = mTodoList.first(); aTodo;
       aTodo = mTodoList.next())
    if (aTodo->uid() == UniqueStr)
      return aTodo;
  // not found
  return 0;
}

QPtrList<Todo> ExchangeCalendar::getTodosForDate(const QDate & date)
{
  QPtrList<Todo> todos;

  Todo *aTodo;
  for (aTodo = mTodoList.first();aTodo;aTodo = mTodoList.next()) {
    if (aTodo->hasDueDate() && aTodo->dtDue().date() == date) {
      todos.append(aTodo);
    }
  }

  return todos;
}
*/

int ExchangeCalendar::numEvents(const QDate &qd)
{
  kdDebug(5800) << "ExchangeCalendar:: numEvent" << endl;
  int count = 0;
  return count;
}

/*
Alarm::List ExchangeCalendar::alarmsTo( const QDateTime &to )
{
  if( mOldestDate )
    return alarms( *mOldestDate, to );
  else
    return alarms( QDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List ExchangeCalendar::alarms( const QDateTime &from, const QDateTime &to )
{
  kdDebug(5800) << "ExchangeCalendar::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  Alarm::List alarms;

  // Check all non-recurring events.
  QIntDictIterator<QPtrList<Event> > it( *mCalDict );
  for( ; it.current(); ++it ) {
    QPtrList<Event> *events = it.current();
    Event *e;
    for( e = events->first(); e; e = events->next() ) {
      appendAlarms( alarms, e, from, to );
    }
  }

  // Check all recurring events.
  Event *e;
  for( e = mRecursList.first(); e; e = mRecursList.next() ) {
    appendRecurringAlarms( alarms, e, from, to );
  }

  // Check all todos.
  Todo *t;
  for( t = mTodoList.first(); t; t = mTodoList.next() ) {
    appendAlarms( alarms, t, from, to );
  }

  return alarms;
}

void ExchangeCalendar::appendAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  QPtrList<Alarm> alarmList = incidence->alarms();
  Alarm *alarm;
  for( alarm = alarmList.first(); alarm; alarm = alarmList.next() ) {
//    kdDebug(5800) << "ExchangeCalendar::appendAlarms() '" << incidence->summary()
//                  << "': " << alarm->time().toString() << " - " << alarm->enabled() << endl;
    if ( alarm->enabled() ) {
      if ( alarm->time() >= from && alarm->time() <= to ) {
        kdDebug(5800) << "ExchangeCalendar::appendAlarms() '" << incidence->summary()
                      << "': " << alarm->time().toString() << endl;
        alarms.append( alarm );
      }
    }
  }
}

void ExchangeCalendar::appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  QPtrList<Alarm> alarmList = incidence->alarms();
  Alarm *alarm;
  QDateTime qdt;
  for( alarm = alarmList.first(); alarm; alarm = alarmList.next() ) {
    if (incidence->recursOn(from.date())) {
      qdt.setTime(alarm->time().time());
      qdt.setDate(from.date());
    }
    else qdt = alarm->time();
    kdDebug(5800) << "ExchangeCalendar::appendAlarms() '" << incidence->summary()
                  << "': " << qdt.toString() << " - " << alarm->enabled() << endl;
    if ( alarm->enabled() ) {
//      kdDebug(5800) << "ExchangeCalendar::appendAlarms() '" << incidence->summary()
//                    << "': " << alarm->time().toString() << endl;
      if ( qdt >= from && qdt <= to ) {
        alarms.append( alarm );
      }
    }
  }
}

*/

/****************************** PROTECTED METHODS ****************************/

// after changes are made to an event, this should be called.
void ExchangeCalendar::update(IncidenceBase *incidence)
{
   kdDebug(5800) << "ExchangeCalendar::update" << endl; 
   incidence->setSyncStatus(Event::SYNCMOD);
  incidence->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  if ( incidence->type() == "Event" ) {
    Event *anEvent = static_cast<Event *>(incidence);
/*
    QIntDictIterator<QPtrList<Event> > qdi(*mCalDict);
    QPtrList<Event> *tmpList;

    // the first thing we do is REMOVE all occurances of the event from
    // both the dictionary and the recurrence list.  Then we reinsert it.
    // We don't bother about optimizations right now.
    qdi.toFirst();
    while ((tmpList = qdi.current()) != 0L) {
      ++qdi;
      tmpList->removeRef(anEvent);
    }
    // take any instances of it out of the recurrence list
    if (mRecursList.findRef(anEvent) != -1)
      mRecursList.take();

    // ok the event is now GONE.  we want to re-insert it.
    insertEvent(anEvent);
*/
  }

  setModified( true );
}

// this function will take a VEvent and insert it into the event
// dictionary for the ExchangeCalendar.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
void ExchangeCalendar::insertEvent(const Event *anEvent)
{
  kdDebug(5800) << "Inserting event into ExchangeCalendar" << endl;

/*
  // initialize if they haven't been allocated yet;
  if (!mOldestDate) {
    mOldestDate = new QDate();
    (*mOldestDate) = anEvent->dtStart().date();
  }
  if (!mNewestDate) {
    mNewestDate = new QDate();
    (*mNewestDate) = anEvent->dtStart().date();
  }

  // update oldest and newest dates if necessary.
  if (anEvent->dtStart().date() < (*mOldestDate))
    (*mOldestDate) = anEvent->dtStart().date();
  if (anEvent->dtStart().date() > (*mNewestDate))
    (*mNewestDate) = anEvent->dtStart().date();

  if (anEvent->recurrence()->doesRecur()) {
    mRecursList.append(anEvent);
  } else {
    // set up the key
    extraDays = anEvent->dtStart().date().daysTo(anEvent->dtEnd().date());
    for (dayCount = 0; dayCount <= extraDays; dayCount++) {
      tmpKey = makeKey(anEvent->dtStart().addDays(dayCount));
      // insert the item into the proper list in the dictionary
      if ((eventList = mCalDict->find(tmpKey)) != 0) {
	eventList->append(anEvent);
      } else {
	// no items under that date yet
	eventList = new QPtrList<Event>;
	eventList->append(anEvent);
	mCalDict->insert(tmpKey, eventList);
      }
    }
  }
*/
}

// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
// BL: an the returned list should be deleted!!!
QPtrList<Event> ExchangeCalendar::events(const QDate &qd, bool sorted)
{
   kdDebug(5800) << "ExchangeCalendar::events(QDate, bool)" << endl; 

   QPtrList<Event> eventList = mClient->events( qd );
   Event *anEvent;
 
   //   	mAccount
//   KURL url = m
//   QDomDocument response = NetDavAccess::search( url, "DAV:", "sql", query );


   
/*
 * // Search non-recurring events
  QPtrList<Event> eventList;
  QPtrList<Event> *tmpList;
 tmpList = mCalDict->find(makeKey(qd));
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next())
      eventList.append(anEvent);
  }

  // Search recurring events
  int extraDays, i;
  for (anEvent = mRecursList.first(); anEvent; anEvent = mRecursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->dtStart().date().daysTo(anEvent->dtEnd().date());
      for (i = 0; i <= extraDays; i++) {
	if (anEvent->recursOn(qd.addDays(-i))) {
	  eventList.append(anEvent);
	  break;
	}
      }
    } else {
      if (anEvent->recursOn(qd))
	eventList.append(anEvent);
    }
  }
*/
  if (!sorted) {
    return eventList;
  }

  kdDebug(5800) << "Sorting getEvents for date\n" << endl;
  // now, we have to sort it based on getDtStart.time()

  QPtrList<Event> eventListSorted;

  int i;
  for (anEvent = eventList.first(); anEvent; anEvent = eventList.next()) {
    if (!eventListSorted.isEmpty() &&
	anEvent->dtStart().time() < eventListSorted.at(0)->dtStart().time()) {
      eventListSorted.insert(0,anEvent);
      goto nextToInsert;
    }
    for (i = 0; (uint) i+1 < eventListSorted.count(); i++) {
      if (anEvent->dtStart().time() > eventListSorted.at(i)->dtStart().time() &&
	  anEvent->dtStart().time() <= eventListSorted.at(i+1)->dtStart().time()) {
	eventListSorted.insert(i+1,anEvent);
	goto nextToInsert;
      }
    }
    eventListSorted.append(anEvent);
  nextToInsert:
    continue;
  }

  return eventListSorted;
}


QPtrList<Event> ExchangeCalendar::events(const QDate &start,const QDate &end,
                                    bool inclusive)
{
    kdDebug(5800) << "ExchangeCalendar::events()" << endl; 
 // QIntDictIterator<QPtrList<Event> > qdi(*mCalDict);
  QPtrList<Event> matchList, *tmpList, tmpList2;
  Event *ev = 0;
/*
  qdi.toFirst();

  // Get non-recurring events
  while (qdi.current()) {
    QDate keyDate = keyToDate(qdi.currentKey());
    if (keyDate >= start && keyDate <= end) {
      tmpList = qdi.current();
      for(ev = tmpList->first();ev;ev = tmpList->next()) {
        bool found = false;
        if (ev->isMultiDay()) {  // multi day event
          QDate mStart = ev->dtStart().date();
          QDate mEnd = ev->dtEnd().date();

          // Check multi-day events only on one date of its duration, the first
          // date which lies in the specified range.
          if ((mStart >= start && mStart == keyDate) ||
              (mStart < start && start == keyDate)) {
            if (inclusive) {
              if (mStart >= start && mEnd <= end) {
                // Event is completely included in range
                found = true;
              }
            } else {
              // Multi-day event has a day in the range
              found = true;
            }
          }
        } else {  // single day event
          found = true;
        }
        if (found) matchList.append(ev);
      }
    }
    ++qdi;
  }

  // Get recurring events
  for(ev = mRecursList.first();ev;ev = mRecursList.next()) {
    QDate rStart = ev->dtStart().date();
    bool found = false;
    if (inclusive) {
      if (rStart >= start && rStart <= end) {
        // Start date of event is in range. Now check for end date.
        // if duration is negative, event recurs forever, so do not include it.
        if (ev->recurrence()->duration() == 0) {  // End date set
          QDate rEnd = ev->recurrence()->endDate();
          if (rEnd >= start && rEnd <= end) {  // End date within range
            found = true;
          }
        } else if (ev->recurrence()->duration() > 0) {  // Duration set
          // TODO: Calculate end date from duration. Should be done in Event
          // For now exclude all events with a duration.
        }
      }
    } else {
      if (rStart <= end) {  // Start date not after range
        if (rStart >= start) {  // Start date within range
          found = true;
        } else if (ev->recurrence()->duration() == -1) {  // Recurs forever
          found = true;
        } else if (ev->recurrence()->duration() == 0) {  // End date set
          QDate rEnd = ev->recurrence()->endDate();
          if (rEnd >= start && rEnd <= end) {  // End date within range
            found = true;
          }
        } else {  // Duration set
          // TODO: Calculate end date from duration. Should be done in Event
          // For now include all events with a duration.
          found = true;
        }
      }
    }

    if (found) matchList.append(ev);
  }
*/
  return matchList;
}

/*
QPtrList<Event> ExchangeCalendar::getAllEvents()
{
  QPtrList<Event> eventList;

  if( mOldestDate && mNewestDate )
    eventList = events(*mOldestDate,*mNewestDate);

  return eventList;
}
*/

// taking a QDateTime, this function will look for an eventlist in the dict
// with that date attached.
QPtrList<Event> ExchangeCalendar::events(const QDateTime &qdt)
{
  return events(qdt.date());
}

/*
void ExchangeCalendar::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  mJournalMap.insert(journal->dtStart().date(),journal);

  journal->registerObserver( this );

  setModified( true );
}

Journal *ExchangeCalendar::journal(const QDate &date)
{
//  kdDebug(5800) << "ExchangeCalendar::journal() " << date.toString() << endl;

  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.find(date);
  if (it == mJournalMap.end()) return 0;
  else {
//    kdDebug(5800) << "  Found" << endl;
    return *it;
  }
}

Journal *ExchangeCalendar::journal(const QString &uid)
{
  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.begin();
  QMap<QDate,Journal *>::ConstIterator end = mJournalMap.end();
  for(;it != end; ++it) {
    if ((*it)->uid() == uid) return *it;
  }
  return 0;
}

QPtrList<Journal> ExchangeCalendar::journalList()
{
  QPtrList<Journal> list;

  QMap<QDate,Journal *>::Iterator it;
  for( it = mJournalMap.begin(); it != mJournalMap.end(); ++it ) {
    list.append(*it);
  }

  return list;
}
*/
