/*
    This file is part of libexchangecalendar.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best

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

#ifndef _CALENDAREXCHANGE_H
#define _CALENDAREXCHANGE_H

#include <qintdict.h>
#include <qmap.h>

#include <libkcal/calendar.h>
#include <libkcal/journal.h>

#include <exchangeaccount.h>
#include <exchangeclient.h>

namespace KCal {

/**
  This class provides access to a calendar on a MS Exchange 2000 server.
*/
class ExchangeCalendar : public Calendar, public IncidenceBase::Observer {
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    ExchangeCalendar( KPIM::ExchangeAccount* account );
//    /** constructs a new calendar, with variables initialized to sane values. */
//    ExchangeCalendar(const QString &timeZoneId);
    virtual ~ExchangeCalendar();
  
    /** clears out the current calendar, freeing all used memory etc. etc. */
    void close();

    bool load (const QString &);
    bool save( const QString &, CalFormat *);
  
    /** Add Event to calendar. */
    void addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /** retrieves an event on the basis of the unique string ID. */
    Event *event(const QString &UniqueStr);
    /** builds and then returns a list of all events that match for the
     * date specified. useful for dayView, etc. etc. */
    QPtrList<Event> events(const QDate &date, bool sorted = FALSE);
    /** Get events for date \a qdt. */
    QPtrList<Event> events(const QDateTime &qdt);
    /** Get events in a range of dates. If inclusive is set to true, only events
     * are returned, which are completely included in the range. */
    QPtrList<Event> events(const QDate &start,const QDate &end,
                             bool inclusive=false);
    /** Return all events in calendar */
    QPtrList<Event> events() { QPtrList<Event> list; return list; }

    QPtrList<Event> rawEvents() { QPtrList<Event> list; return list; }
    QPtrList<Event> rawEventsForDate(const QDate &date, bool sorted = FALSE) { return events( date, sorted ); }
    QPtrList<Event> rawEventsForDate(const QDateTime &qdt) { return events( qdt ); }
    QPtrList<Event> rawEvents(const QDate &start,const QDate &end,
                             bool inclusive=false) { return events( start, end, inclusive ); }
 
    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    QString getHolidayForDate(const QDate &qd);
    
    /** returns the number of events that are present on the specified date. */
    int numEvents(const QDate &qd);
  
    /** add a todo to the todolist. */
    void addTodo(Todo *todo) { return; }
    /** remove a todo from the todolist. */
    void deleteTodo(Todo *) { return; }
    const QPtrList<Todo> &todos() const { QPtrList<Todo> list; return list; }
    /** searches todolist for an event with this unique string identifier,
      returns a pointer or null. */
    Todo *todo(const QString &UniqueStr) { return new Todo(); }
    /** Returns list of todos due on the specified date */
    QPtrList<Todo> todos(const QDate & date) { QPtrList<Todo> list; return list; }

    QPtrList<Todo> rawTodos() const { QPtrList<Todo> list; return list; }
    QPtrList<Todo> rawTodos(const QDate & date) { QPtrList<Todo> list; return list; }

    /** Add a Journal entry to calendar */
    virtual void addJournal(Journal *) { return; }
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &) { return new Journal(); }
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID) { return new Journal; }
    /** Return list of all Journals stored in calendar */
    QPtrList<Journal> journals(){ QPtrList<Journal> list; return list; }

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to ) { Alarm::List list; return list; }

    /** Return all alarms, which ocur before given date. */
    Alarm::List alarmsTo( const QDateTime &to ) { Alarm::List list; return list; }

  protected:
    /** this method should be called whenever a Event is modified directly
     * via its pointer.  It makes sure that the calendar is internally
     * consistent. */
    void update(IncidenceBase *incidence);
  
    /** Notification function of IncidenceBase::Observer. */
    void incidenceUpdated( IncidenceBase *i ) { update( i ); }
  
    /** inserts an event into its "proper place" in the calendar. */
    void insertEvent(const Event *anEvent);
  
    /** Append alarms of incidence in interval to list of alarms. */
//    void appendAlarms( Alarm::List &alarms, Incidence *incidence,
//                       const QDateTime &from, const QDateTime &to );

    /** Append alarms of recurring events in interval to list of alarms. */
//    void appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
//                       const QDateTime &from, const QDateTime &to );

  private:
    void init( KPIM::ExchangeAccount* account );

    KPIM::ExchangeAccount* mAccount;
    KPIM::ExchangeClient* mClient;

    QIntDict<QPtrList<Event> > *mCalDict;    // dictionary of lists of events.
    QPtrList<Event> mRecursList;             // list of repeating events.

    QPtrList<Todo> mTodoList;               // list of todo items.

    QMap<QDate,Journal *> mJournalMap;
  
//    QDate *mOldestDate;
//    QDate *mNewestDate;
};  

}

#endif
