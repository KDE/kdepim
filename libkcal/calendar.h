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

#ifndef CALENDAR_H
#define CALENDAR_H

#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>

#include "event.h"
#include "todo.h"

#include "calformat.h"

#define _TIME_ZONE "-0500" /* hardcoded, overridden in config file. */

class KConfig;

namespace KCal {

class VCalDrag;
class ICalFormat;
class CalFilter;

/**
  This is the main "calendar" object class for KOrganizer.  It holds
  information like all appointments/events, user information, etc. etc.
  one calendar is associated with each CalendarView (@see calendarview.h).
  This is an abstract base class defining the interface to a calendar. It is
  implemented by subclasses like @see CalendarLocal, which use different
  methods to store and access the data.

  Ownership of events etc. is handled by the following policy: As soon as an
  event (or any other subclass of IncidenceBase) object is added to the
  Calendar by addEvent() it is owned by the Calendar object. The Calendar takes
  care of deleting it. All Events returned by the query functions are returned
  as pointers, that means all changes to the returned events are immediately
  visible in the Calendar. You shouldn't delete any Event object you get from
  Calendar.
*/
class Calendar {
  public:
    Calendar();
    Calendar(const QString &timeZoneId);
    virtual ~Calendar();

    /**
      Return the calendar format class the calendar object uses for load
      operations, and the default format it uses for save operations.
    */
    CalFormat *calFormat();
    /**
      Return the iCalendar format class the calendar object uses.
    */
    ICalFormat *iCalFormat();

    /**
      Loads a calendar on disk into the current calendar.
      @return true if successful, else returns false.
      @param fileName the name of the calendar on disk.
    */
    virtual bool load( const QString &fileName ) = 0;
    /**
      Writes out the calendar to disk in the format specified by the format
      parameter. If the format is 0, vCalendar is used.
      @return true if successful and false on error.
      @param fileName the name of the file
    */
    virtual bool save( const QString &fileName, CalFormat *format = 0 ) = 0;
    /**
      Clears out the current calendar, freeing all used memory etc.
    */
    virtual void close() = 0;
  
    /**
      Return the owner of the calendar's full name.
    */
    const QString &getOwner() const;
    /**
      Set the owner of the calendar. Should be owner's full name.
    */
    void setOwner( const QString &os );
    /**
      Return the email address of the calendar owner.
    */
    const QString &getEmail();
    /**
      Set the email address of the calendar owner.
    */
    void setEmail( const QString & );
  
    /**
      Set time zone from a timezone string (e.g. -2:00)
    */
    void setTimeZone( const QString &tz );
    /**
      Set time zone from a minutes value (e.g. -60)
    */
    void setTimeZone( int tz );
    /**
      Return time zone as offest in minutes.
    */
    int getTimeZone() const;
    /**
      Compute an ISO 8601 format string from the time zone.
    */
    QString getTimeZoneStr() const;
    /**
      Set time zone id (see /usr/share/zoneinfo/zone.tab for list of legal
      values).
    */
    void setTimeZoneId( const QString & );
    /**
      Return time zone id.
    */
    QString timeZoneId() const;
    /**
      Use local time, not UTC or a time zone.
    */
    void setLocalTime();
    /**
      Return whether local time is being used.
    */
    bool isLocalTime() const;
  
    /**
      Adds a Event to this calendar object.
      @param anEvent a pointer to the event to add
    */
    virtual void addEvent( Event *anEvent ) = 0;
    /**
      Delete event from calendar.
    */
    virtual void deleteEvent( Event * ) = 0;

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    virtual Event *getEvent( const QString &UniqueStr ) = 0;
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
      The calendar filter is applied.
    */
    QPtrList<Event> getEventsForDate( const QDate &date, bool sorted = false);
    /**
      Get events, which occur on the given date.
      The calendar filter is applied.
    */
    QPtrList<Event> getEventsForDate( const QDateTime &qdt );
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
      The calendar filter is applied.
    */
    QPtrList<Event> getEvents( const QDate &start, const QDate &end,
                               bool inclusive = false);
    /**
      Return all events in calendar
    */
    virtual QPtrList<Event> getAllEvents() = 0;
  
    /**
      Set calendar filter, which filters events for the getEvents* functions.
      The Filter object is owned by the caller.
    */
    void setFilter( CalFilter * );
    /**
      Return calendar filter.
    */
    CalFilter *filter();
      
    /**
      Returns the number of events that are present on the specified date.
    */
    virtual int numEvents( const QDate &qd ) = 0;
  
    /**
      Add a todo to the todolist.
    */
    virtual void addTodo( Todo *todo ) = 0;
    /**
      Remove a todo from the todolist.
    */
    virtual void deleteTodo( Todo * ) = 0;
    /**
      Return filterd list of todos.
    */
    QPtrList<Todo> getFilteredTodoList();
    /**
      Return unfiltered list of todos.
    */
    virtual const QPtrList<Todo> &getTodoList() const = 0;
    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    virtual Todo *getTodo( const QString &UniqueStr ) = 0;
    /**
      Returns list of todos due on the specified date.
    */
    virtual QPtrList<Todo> getTodosForDate( const QDate &date ) = 0;

    /**
      Add a Journal entry to calendar.
    */
    virtual void addJournal( Journal * ) = 0;
    /**
      Return Journal for given date.
    */
    virtual Journal *journal( const QDate & ) = 0;
    /**
      Return Journal with given UID.
    */
    virtual Journal *journal( const QString &UID ) = 0;
    /**
      Return list of all Journal entries.
    */
    virtual QPtrList<Journal> journals() = 0;

    /**
      Add an incidence to calendar.
    */
    void addIncidence( Incidence * );
  
    /**
      Return all alarms, which ocur in the given time interval.
    */
    virtual Alarm::List alarms( const QDateTime &from,
                                const QDateTime &to ) = 0;

    class Observer {
      public:
        virtual void calendarModified( bool, Calendar * ) = 0;
    };
  
    void registerObserver( Observer * );

    void setModified( bool );

  protected:
    /**
      Get unfiltered events, which occur on the given date.
    */
    virtual QPtrList<Event> rawEventsForDate( const QDate &date,
                                              bool sorted = false ) = 0;
    /**
      Get unfiltered events, which occur on the given date.
    */
    virtual QPtrList<Event> rawEventsForDate( const QDateTime &qdt ) = 0;
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
    */
    virtual QPtrList<Event> rawEvents( const QDate &start, const QDate &end,
                                       bool inclusive = false ) = 0;
  
    CalFormat *mFormat;     // format used for load, and default for save, operations
    CalFormat *mDndFormat;  // format used for drag and drop operations
    ICalFormat *mICalFormat;
  
  private:
    void init();
  
    QString mOwner;        // who the calendar belongs to
    QString mOwnerEmail;   // email address of the owner
    int mTimeZone;         // timezone OFFSET from GMT (MINUTES)
    bool mLocalTime;       // use local time, not UTC or a time zone

    CalFilter *mFilter;
    CalFilter *mDefaultFilter;
    
    QString mTimeZoneId;

    Observer *mObserver;
    bool mNewObserver;
    
    bool mModified;
};
  
}

#endif
