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
#include <qdict.h>

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/resourcecalendar.h>

#include "exchangemonitor.h"

class DateSet;

namespace KPIM {
class ExchangeAccount;
class ExchangeClient;
}

namespace KCal {
class Event;
class CalFormat;

/**
  This class provides a calendar stored on a Microsoft Exchange 2000 server
*/
class ResourceExchange : public ResourceCalendar, public IncidenceBase::Observer
{
  Q_OBJECT

  public:
    ResourceExchange( const KConfig * );
    virtual ~ResourceExchange();

    virtual void writeConfig( KConfig* config );

    virtual bool load();

    /**
     Writes calendar to storage. Writes calendar to disk file,
     writes updates to server, whatever.
     */
    virtual bool save();

    /** constructs a new calendar, with variables initialized to sane values. */
//    ExchangeCalendar( KPIM::ExchangeAccount* account );
    /** constructs a new calendar, with variables initialized to sane values. */
//    ExchangeCalendar( KPIM::ExchangeAccount* account, const QString &timeZoneId );
//    virtual ~ExchangeCalendar();
  
    /**
      Semantics not yet defined. Should the Exchange calendar be wiped clean?
      Should the disk calendar be copied to the Exchange calendar?
      At the moment, does nothing.
      @return true, if successful, false on error.
      @param fileName the name of the calendar on disk.
    */
//    bool load( const QString &fileName );
    /**
      Writes out the calendar to disk in the specified \a format.
      ExchangeCalendar takes ownership of the CalFormat object.
      @return true, if successfull, false on error.
      @param fileName the name of the file
    */
//    bool save( const QString &fileName, CalFormat *format = 0 );

    /** clears out the current calendar, freeing all used memory etc. etc. */
//    void close();
  
    /** Add Event to calendar. */
    bool addEvent(Event *anEvent);
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
    
    /** returns the number of events that are present on the specified date. */
    int numEvents(const QDate &qd);
  
    virtual void subscribeEvents( const QDate& start, const QDate& end );

    /**
      Stop receiving event signals for the given period (inclusive). After this call,
      the calendar resource will no longer send eventsAdded, eventsModified or
      eventsDeleted signals for events falling completely in this period. The resource
      MAY delete the Events objects. The application MUST NOT dereference pointers 
      to the relevant Events after this call.
    */
    virtual void unsubscribeEvents( const QDate& start, const QDate& end );

    /**
      Add a todo to the todolist.
    */
    bool addTodo( Todo *todo );
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
    QPtrList<Todo> rawTodos();
    /**
      Returns list of todos due on the specified date.
    */
    QPtrList<Todo> todos( const QDate &date );

    /** Add a Journal entry to calendar */
    virtual bool addJournal(Journal *);
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

    friend class ResourceExchangeConfig;

  protected:
    /**
      Prepare the calendar for use. Load the calendar from disk, 
      open connections to the calendaring server, whatever.
      Must be called before other methods can be called.
    */
    virtual bool doOpen();

    /** clears out the current calendar, freeing all used memory etc. etc. */
    virtual void doClose();

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
//    void insertEvent(const Event *anEvent);
  
    /** Append alarms of incidence in interval to list of alarms. */
//    void appendAlarms( Alarm::List &alarms, Incidence *incidence,
//                       const QDateTime &from, const QDateTime &to );

    /** Append alarms of recurring events in interval to list of alarms. */
//    void appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
//                       const QDateTime &from, const QDateTime &to );

    void uploadEvent( Event* event );

  protected slots:
    void slotMonitorNotify( const QValueList<long>& IDs, const QValueList<KURL>& urls);
    void slotMonitorError( int errorCode, const QString& moreInfo );
    void slotDownloadFinished( int result, const QString& moreinfo );
    void downloadedEvent( KCal::Event*, const KURL& );

  private:
    class EventInfo;
    KPIM::ExchangeAccount* mAccount;
    KPIM::ExchangeClient* mClient;
    KPIM::ExchangeMonitor* mMonitor;
    CalendarLocal* mCache;
    QDict<EventInfo> mEventDict; // maps UIDS to EventInfo records
    QIntDict<EventInfo> mWatchDict; // maps Watch IDs to EventInfo records 
    DateSet* mDates;
    QMap<Event, QDateTime>* mEventDates;
    QMap<QDate, QDateTime>* mCacheDates;
    int mCachedSeconds;
};  

}

#endif
