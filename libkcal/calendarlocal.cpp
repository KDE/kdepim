/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"

#include "calendarlocal.h"

using namespace KCal;

CalendarLocal::CalendarLocal()
  : Calendar()
{
  init();
}

CalendarLocal::CalendarLocal(const QString &timeZoneId)
  : Calendar(timeZoneId)
{
  init();
}

void CalendarLocal::init()
{
  mOldestDate = 0L;
  mNewestDate = 0L;

  mRecursList.setAutoDelete(TRUE);
  // solves the leak?
  mTodoList.setAutoDelete(TRUE);

  mCalDict = new QIntDict<QPtrList<Event> > (BIGPRIME);
  mCalDict->setAutoDelete(TRUE);
}


CalendarLocal::~CalendarLocal()
{
  close();
  delete mCalDict;
  delete mNewestDate;
  delete mOldestDate;
}

bool CalendarLocal::load(const QString &fileName)
{
  kdDebug(5800) << "CalendarLocal::load(): '" << fileName << "'" << endl;

  // do we want to silently accept this, or make some noise?  Dunno...
  // it is a semantical thing vs. a practical thing.
  if (fileName.isEmpty()) return false;

  delete mFormat;

  // Always try to load with iCalendar. It will detect, if it is actually a
  // vCalendar file.
  mFormat = new ICalFormat(this);

  mFormat->clearException();
  bool success = mFormat->load(fileName);

  if (!success) {
    if (mFormat->exception()) {
//      kdDebug(5800) << "---Error: " << mFormat->exception()->errorCode() << endl;
      if (mFormat->exception()->errorCode() == ErrorFormat::CalVersion1) {
        // Expected non vCalendar file, but detected vCalendar
        kdDebug(5800) << "CalendarLocal::load() Fallback to VCalFormat" << endl;
        delete mFormat;
        mFormat = new VCalFormat(this);
        return mFormat->load(fileName);
      }
      return false;
    } else {
      kdDebug(5800) << "Warning! There should be set an exception." << endl;
      return false;
    }
  } else {
//    kdDebug(5800) << "---Success" << endl;
  }

  return true;
}

bool CalendarLocal::save(const QString &fileName,CalFormat *format)
{
  if (format) {
    return format->save(fileName);
  } else {
    return mFormat->save(fileName);
  }
}

void CalendarLocal::close()
{
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
}


void CalendarLocal::addEvent(Event *anEvent)
{
  insertEvent(anEvent);
  if (anEvent->organizer() != getEmail()) {
    kdDebug(5800) << "Event " << anEvent->summary() << " Organizer: " << anEvent->organizer()
              << " Email: " << getEmail() << endl;
//    anEvent->setReadOnly(true);
  }
  
  anEvent->registerObserver( this );
}

// probably not really efficient, but...it works for now.
void CalendarLocal::deleteEvent(Event *event)
{
  kdDebug(5800) << "CalendarLocal::deleteEvent" << endl;

  QDate date(event->dtStart().date());

  QPtrList<Event> *tmpList;
  Event *anEvent;
  int extraDays, dayOffset;
  QDate startDate, tmpDate;

  tmpList = mCalDict->find(makeKey(date));
  // if tmpList exists, the event is in the normal dictionary;
  // it doesn't recur.
  if (tmpList) {
    for (anEvent = tmpList->first(); anEvent;
	 anEvent = tmpList->next()) {
      if (anEvent == event) {
	if (!anEvent->isMultiDay()) {
	  tmpList->setAutoDelete(FALSE);
	  tmpList->remove();
	  goto FINISH;
	} else {
	  //kdDebug(5800) << "deleting multi-day event" << endl;
	  // event covers multiple days.
	  startDate = anEvent->dtStart().date();
	  extraDays = startDate.daysTo(anEvent->dtEnd().date());
	  for (dayOffset = 0; dayOffset <= extraDays; dayOffset++) {
	    tmpDate = startDate.addDays(dayOffset);
	    tmpList = mCalDict->find(makeKey(tmpDate));
	    if (tmpList) {
	      for (anEvent = tmpList->first(); anEvent;
		   anEvent = tmpList->next()) {
		if (anEvent == event)
		  tmpList->remove();
	      }
	    }
	  }
	  // now we need to free the memory taken up by the event...
	  delete anEvent;
	  goto FINISH;
	}
      }
    }
  }
  for (anEvent = mRecursList.first(); anEvent;
       anEvent = mRecursList.next()) {
    if (anEvent == event) {
      mRecursList.remove();
    }
  }


 FINISH:
  // update oldest / newest dates if necessary
  // basically, first we check to see if this was the oldest
  // date in the calendar.  If it is, then we keep adding 1 to
  // the oldest date until we come up with a location in the
  // QDate dictionary which has some entries.  Now, this might
  // be the oldest date, but we want to check the recurrence list
  // to make sure it has nothing older.  We start looping through
  // it, and each time we find something older, we adjust the oldest
  // date and start the loop again.  If we go through all the entries,
  // we are assured to have the new oldest date.
  //
  // the newest date is analogous, but sort of opposite.
  if (date == (*mOldestDate)) {
    for (; !mCalDict->find(makeKey((*mOldestDate))) &&
	   (*mOldestDate != *mNewestDate);
	 (*mOldestDate) = mOldestDate->addDays(1));
    mRecursList.first();
    while ((anEvent = mRecursList.current())) {
      if (anEvent->dtStart().date() < (*mOldestDate)) {
	(*mOldestDate) = anEvent->dtStart().date();
	mRecursList.first();
      }
      anEvent = mRecursList.next();
    }
  }

  if (date == (*mNewestDate)) {
    for (; !mCalDict->find(makeKey((*mNewestDate))) &&
	   (*mNewestDate != *mOldestDate);
	 (*mNewestDate) = mNewestDate->addDays(-1));
    mRecursList.first();
    while ((anEvent = mRecursList.current())) {
      if (anEvent->dtStart().date() > (*mNewestDate)) {
	(*mNewestDate) = anEvent->dtStart().date();
	mRecursList.first();
      }
      anEvent = mRecursList.next();
    }
  }
}


Event *CalendarLocal::getEvent(const QString &uid)
{
  kdDebug(5800) << "CalendarLocal::getEvent(): " << uid << endl;

  QPtrList<Event> *eventList;
  QIntDictIterator<QPtrList<Event> > dictIt(*mCalDict);
  Event *anEvent;

  while (dictIt.current()) {
    eventList = dictIt.current();
    for (anEvent = eventList->first(); anEvent;
	 anEvent = eventList->next()) {
      if (anEvent->VUID() == uid) {
	return anEvent;
      }
    }
    ++dictIt;
  }
  for (anEvent = mRecursList.first(); anEvent;
       anEvent = mRecursList.next()) {
    if (anEvent->VUID() == uid) {
      return anEvent;
    }
  }
  // catch-all.
  return (Event *) 0L;
}

void CalendarLocal::addTodo(Todo *todo)
{
  mTodoList.append(todo);

  todo->registerObserver( this );
}

void CalendarLocal::deleteTodo(Todo *todo)
{
  mTodoList.findRef(todo);
  mTodoList.remove();
}


const QPtrList<Todo> &CalendarLocal::getTodoList() const
{
  return mTodoList;
}

Todo *CalendarLocal::getTodo(const QString &UniqueStr)
{
  Todo *aTodo;
  for (aTodo = mTodoList.first(); aTodo;
       aTodo = mTodoList.next())
    if (aTodo->VUID() == UniqueStr)
      return aTodo;
  // not found
  return 0;
}

QPtrList<Todo> CalendarLocal::getTodosForDate(const QDate & date)
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

int CalendarLocal::numEvents(const QDate &qd)
{
  QPtrList<Event> *tmpList;
  Event *anEvent;
  int count = 0;
  int extraDays, i;

  // first get the simple case from the dictionary.
  tmpList = mCalDict->find(makeKey(qd));
  if (tmpList)
    count += tmpList->count();

  // next, check for repeating events.  Even those that span multiple days...
  for (anEvent = mRecursList.first(); anEvent; anEvent = mRecursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->dtStart().date().daysTo(anEvent->dtEnd().date());
      //kdDebug(5800) << "multi day event w/" << extraDays << " days" << endl;
      for (i = 0; i <= extraDays; i++) {
	if (anEvent->recursOn(qd.addDays(i))) {
	  ++count;
	  break;
	}
      }
    } else {
      if (anEvent->recursOn(qd))
	++count;
    }
  }
  return count;
}


Alarm::List CalendarLocal::alarmsTo( const QDateTime &to )
{
  if( mOldestDate )
    return alarms( *mOldestDate, to );
  else
    return alarms( QDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List CalendarLocal::alarms( const QDateTime &from, const QDateTime &to )
{
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
    appendAlarms( alarms, e, from, to );
  }

  // Check all todos.
  Todo *t;
  for( t = mTodoList.first(); t; t = mTodoList.next() ) {
    appendAlarms( alarms, t, from, to );
  }  

  return alarms;
}

void CalendarLocal::appendAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  QPtrList<Alarm> alarmList = incidence->alarms();
  Alarm *alarm;
  for( alarm = alarmList.first(); alarm; alarm = alarmList.next() ) {  
    if ( alarm->enabled() ) {
//      kdDebug(5800) << "CalendarLocal::appendAlarms() '" << incidence->summary()
//                    << "': " << alarm->time().toString() << endl;
      if ( alarm->time() >= from && alarm->time() <= to ) {
        alarms.append( alarm );
      }
    }
  }
}


/****************************** PROTECTED METHODS ****************************/

// after changes are made to an event, this should be called.
void CalendarLocal::update(IncidenceBase *incidence)
{
  incidence->setSyncStatus(Event::SYNCMOD);
  incidence->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  Event *anEvent = dynamic_cast<Event *>(incidence);
  if ( anEvent ) {
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
  }
}

// this function will take a VEvent and insert it into the event
// dictionary for the CalendarLocal.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
void CalendarLocal::insertEvent(const Event *anEvent)
{
  long tmpKey;
  QString tmpDateStr;
  QPtrList<Event> *eventList;
  int extraDays, dayCount;

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
}

// make a long dict key out of a QDateTime
long int CalendarLocal::makeKey(const QDateTime &dt)
{
  QDate tmpD;
  QString tmpStr;

  tmpD = dt.date();
  tmpStr.sprintf("%d%.2d%.2d",tmpD.year(), tmpD.month(), tmpD.day());
//  kdDebug(5800) << "CalendarLocal::makeKey(): " << tmpStr << endl;
  return tmpStr.toLong();
}

// make a long dict key out of a QDate
long int CalendarLocal::makeKey(const QDate &d)
{
  QString tmpStr;

  tmpStr.sprintf("%d%.2d%.2d",d.year(), d.month(), d.day());
  return tmpStr.toLong();
}

QDate CalendarLocal::keyToDate(long int key)
{
  QString dateStr = QString::number(key);
//  kdDebug(5800) << "CalendarLocal::keyToDate(): " << dateStr << endl;
  QDate date(dateStr.mid(0,4).toInt(),dateStr.mid(4,2).toInt(),
             dateStr.mid(6,2).toInt());

//  kdDebug(5800) << "  QDate: " << date.toString() << endl;

  return date;
}


// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
// BL: an the returned list should be deleted!!!
QPtrList<Event> CalendarLocal::eventsForDate(const QDate &qd, bool sorted)
{
  // Search non-recurring events
  QPtrList<Event> eventList;
  QPtrList<Event> *tmpList;
  Event *anEvent;
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

  if (!sorted) {
    return eventList;
  }

  //  kdDebug(5800) << "Sorting getEvents for date\n" << endl;
  // now, we have to sort it based on getDtStart.time()
  QPtrList<Event> eventListSorted;
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


QPtrList<Event> CalendarLocal::events(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QIntDictIterator<QPtrList<Event> > qdi(*mCalDict);
  QPtrList<Event> matchList, *tmpList, tmpList2;
  Event *ev = 0;

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

  return matchList;
}

QPtrList<Event> CalendarLocal::getAllEvents()
{
  QPtrList<Event> eventList;

  if( mOldestDate && mNewestDate )
    eventList = events(*mOldestDate,*mNewestDate);

  return eventList;
}


// taking a QDateTime, this function will look for an eventlist in the dict
// with that date attached.
QPtrList<Event> CalendarLocal::eventsForDate(const QDateTime &qdt)
{
  return eventsForDate(qdt.date());
}

void CalendarLocal::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  mJournalMap.insert(journal->dtStart().date(),journal);

  journal->registerObserver( this );
}

Journal *CalendarLocal::journal(const QDate &date)
{
//  kdDebug(5800) << "CalendarLocal::journal() " << date.toString() << endl;

  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.find(date);
  if (it == mJournalMap.end()) return 0;
  else {
//    kdDebug(5800) << "  Found" << endl;
    return *it;
  }
}

Journal *CalendarLocal::journal(const QString &UID)
{
  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.begin();
  QMap<QDate,Journal *>::ConstIterator end = mJournalMap.end();
  for(;it != end; ++it) {
    if ((*it)->VUID() == UID) return *it;
  }
  return 0;
}

QPtrList<Journal> CalendarLocal::journalList()
{
  QPtrList<Journal> list;

  QMap<QDate,Journal *>::Iterator it;
  for( it = mJournalMap.begin(); it != mJournalMap.end(); ++it ) {
    list.append(*it);
  }

  return list;
}
