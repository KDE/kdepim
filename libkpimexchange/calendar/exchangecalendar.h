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
#ifndef KPIM_EXCHANGECALENDAR_H
#define KPIM_EXCHANGECALENDAR_H

#include <qmap.h>
#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>

class DateSet;

namespace KPIM {
class ExchangeAccount;
class ExchangeClient;
}

namespace KCal {
class Event;
}

namespace KCal {

class CalFormat;

/**
  This class provides a calendar stored on a Microsoft Exchange 2000 server
*/
class ExchangeCalendar : public Calendar, public IncidenceBase::Observer
{
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    ExchangeCalendar( KPIM::ExchangeAccount* account );
    /** constructs a new calendar, with variables initialized to sane values. */
    ExchangeCalendar( KPIM::ExchangeAccount* account, const QString &timeZoneId );
    virtual ~ExchangeCalendar();
  
    /**
      Semantics not yet defined. Should the Exchange calendar be wiped clean?
      Should the disk calendar be copied to the Exchange calendar?
      At the moment, does nothing.
      @return true, if successful, false on error.
      @param fileName the name of the calendar on disk.
    */
    bool load( const QString &fileName );
    /**
      Writes out the calendar to disk in the specified \a format.
      ExchangeCalendar takes ownership of the CalFormat object.
      @return true, if successful, false on error.
      @param fileName the name of the file
    */
    bool save( const QString &fileName, CalFormat *format = 0 );

    /** clears out the current calendar, freeing all used memory etc. etc. */
    void close();
  
    /** Add Event to calendar. */
    void addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event(const QString &UniqueStr);
    /**
      Return filtered list of all events in calendar.
    */
//    QPtrList<Event> events();
    /**
      Return unfiltered list of all events in calendar.
      Use with care, since this causes a LOT of network activity
    */
    QPtrList<Event> rawEvents();

    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    QString getHolidayForDate(const QDate &qd);
    
    /**
      Add a todo to the todolist.
    */
    void addTodo( Todo *todo );
    /**
      Remove a todo from the todolist.
    */
    void deleteTodo( Todo * );
    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    Todo *todo( const QString &uid );
    /**
      Return list of all todos.
    */
    QPtrList<Todo> rawTodos() const;
    /**
      Returns list of todos due on the specified date.
    */
    QPtrList<Todo> todos( const QDate &date );
    /**
      Return list of all todos.
      
      Workaround because compiler does not recognize function of base class.
    */
    QPtrList<Todo> todos() { return Calendar::todos(); }

    /** Add a Journal entry to calendar */
    virtual void addJournal(Journal *);
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &);
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID);
    /** Return list of all Journals stored in calendar */
    QPtrList<Journal> journals();

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /** Return all alarms, which ocur before given date. */
    Alarm::List alarmsTo( const QDateTime &to );

  protected:
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    QPtrList<Event> rawEventsForDate( const QDate &date, bool sorted = false );
    /**
      Get unfiltered events for date \a qdt.
    */
    QPtrList<Event> rawEventsForDate( const QDateTime &qdt );
    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
    */
    QPtrList<Event> rawEvents( const QDate &start, const QDate &end,
                               bool inclusive = false );

    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    void update(IncidenceBase *incidence);
  
    /** Notification function of IncidenceBase::Observer. */
    void incidenceUpdated( IncidenceBase *i ) { mCache->update( i ); update( i ); }
  
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
    CalendarLocal* mCache;
    DateSet* mDates;
    QMap<Event, QDateTime>* mEventDates;
    QMap<QDate, QDateTime>* mCacheDates;
    int mCachedSeconds;
};  

}

#endif
