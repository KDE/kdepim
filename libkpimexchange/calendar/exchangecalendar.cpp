/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/

#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/journal.h>

#include "dateset.h"
#include "exchangeaccount.h"
#include "exchangeclient.h"

#include "exchangecalendar.h"

using namespace KCal;
using namespace KPIM;

ExchangeCalendar::ExchangeCalendar( KPIM::ExchangeAccount* account )
  : Calendar()
{
  init( account );
  mCache = new CalendarLocal();
}

ExchangeCalendar::ExchangeCalendar( KPIM::ExchangeAccount* account, const QString &timeZoneId)
  : Calendar(timeZoneId)
{
  init( account );
  mCache = new CalendarLocal( timeZoneId );
}

void ExchangeCalendar::init( KPIM::ExchangeAccount* account )
{
  kdDebug() << "ExchangeCalendar::init()" << endl;
  mAccount = account;
  mClient = new ExchangeClient( account );
  mDates = new DateSet();

  mEventDates = new QMap<Event,QDateTime>();
  mCacheDates = new QMap<QDate, QDateTime>();

  mCachedSeconds = 600; // After 5 minutes, reread from server
  // mOldestDate = 0L;
  // mNewestDate = 0L;
}


ExchangeCalendar::~ExchangeCalendar()
{
  kdDebug() << "Destructing ExchangeCalendar" << endl;
  close();
  // delete mNewestDate;
  // delete mOldestDate;
  delete mDates;
  delete mClient;
  delete mEventDates;
  delete mCacheDates;
  delete mCache;
}
 

bool ExchangeCalendar::load( const QString &fileName )
{
  // return mCache->load( fileName );
  return true;
}

bool ExchangeCalendar::save( const QString &fileName, CalFormat *format )
{
  return mCache->save( fileName, format );
}

void ExchangeCalendar::close()
{
  mCache->close();
  setModified( false );
}


void ExchangeCalendar::addEvent(Event *anEvent)
{
  kdDebug() << "ExchangeCalendar::addEvent" << endl;
  mCache->addEvent( anEvent );
  insertEvent(anEvent);
 
  anEvent->registerObserver( this );

  setModified( true );
}

// probably not really efficient, but...it works for now.
void ExchangeCalendar::deleteEvent(Event *event)
{
  kdDebug(5800) << "ExchangeCalendar::deleteEvent" << endl;
  mCache->deleteEvent( event );
  setModified( true );
}


Event *ExchangeCalendar::event( const QString &uid )
{
  kdDebug(5800) << "ExchangeCalendar::event(): " << uid << endl;

  return mCache->event( uid );
}

void ExchangeCalendar::addTodo(Todo *todo)
{
  mCache->addTodo( todo );

  todo->registerObserver( this );

  setModified( true );
}

void ExchangeCalendar::deleteTodo(Todo *todo)
{
  mCache->deleteTodo( todo );

  setModified( true );
}

QPtrList<Todo> ExchangeCalendar::rawTodos() const
{
  return mCache->rawTodos();
}

Todo *ExchangeCalendar::todo( const QString &uid )
{
  return mCache->todo( uid );
}

QPtrList<Todo> ExchangeCalendar::todos( const QDate &date )
{
  return mCache->todos( date );
}

Alarm::List ExchangeCalendar::alarmsTo( const QDateTime &to )
{
  return mCache->alarmsTo( to );
}

Alarm::List ExchangeCalendar::alarms( const QDateTime &from, const QDateTime &to )
{
  kdDebug(5800) << "ExchangeCalendar::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  return mCache->alarms( from, to );
}

/****************************** PROTECTED METHODS ****************************/

// after changes are made to an event, this should be called.
void ExchangeCalendar::update(IncidenceBase *incidence)
{
  setModified( true );
}

// this function will take a VEvent and insert it into the event
// dictionary for the ExchangeCalendar.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
void ExchangeCalendar::insertEvent(const Event *anEvent)
{
  kdDebug() << "ExchangeCalendar::insertEvent" << endl;
 
}

// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
QPtrList<Event> ExchangeCalendar::rawEventsForDate(const QDate &qd, bool sorted)
{
  kdDebug() << "ExchangeCalendar::rawEventsForDate(" << qd.toString() << "," << sorted << ")" << endl;
 
  // If the events for this date are not in the cache, or if they are old,
  // get them again
  QDateTime now = QDateTime::currentDateTime();
  // kdDebug() << "Now is " << now.toString() << endl;
  // kdDebug() << "mDates: " << mDates << endl;
  // kdDebug() << "mDates->contains(qd) is " << mDates->contains( qd ) << endl;
  QDate start = QDate( qd.year(), qd.month(), 1 ); // First day of month
  if ( !mDates->contains( start ) || (*mCacheDates)[start].secsTo( now ) > mCachedSeconds ) {
    kdDebug() << "Reading events for month of " << start.toString() << endl;
    QDate end = start.addMonths( 1 ).addDays( -1 ); // Last day of month
    mClient->downloadSynchronous( mCache, start, end, true ); // Show progress dialog
    mDates->add( start );
    mCacheDates->insert( start, now );
  }

  // Events are safely in the cache now, return them from cache
  QPtrList<Event> events = mCache->rawEventsForDate( qd, sorted );
  kdDebug() << "Found " << events.count() << " events." << endl;
  return events;

/*
  if (!sorted) {
    return eventList;
  }

  //  kdDebug(5800) << "Sorting events for date\n" << endl;
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
*/
}


QPtrList<Event> ExchangeCalendar::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
   kdDebug() << "ExchangeCalendar::rawEvents(start,end,inclusive)" << endl;
 return mCache->rawEvents( start, end, inclusive );
}

QPtrList<Event> ExchangeCalendar::rawEventsForDate(const QDateTime &qdt)
{
   kdDebug() << "ExchangeCalendar::rawEventsForDate(qdt)" << endl;
 return rawEventsForDate( qdt.date() );
}

QPtrList<Event> ExchangeCalendar::rawEvents()
{
   kdDebug() << "ExchangeCalendar::rawEvents()" << endl;
 return mCache->rawEvents();
}

void ExchangeCalendar::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  mCache->addJournal( journal );

  journal->registerObserver( this );

  setModified( true );
}

Journal *ExchangeCalendar::journal(const QDate &date)
{
//  kdDebug(5800) << "ExchangeCalendar::journal() " << date.toString() << endl;
  return mCache->journal( date );
}

Journal *ExchangeCalendar::journal(const QString &uid)
{
  return mCache->journal( uid );
}

QPtrList<Journal> ExchangeCalendar::journals()
{
  return mCache->journals();
}
