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
  * This is the main "calendar" object class for KOrganizer.  It holds
  * information like all appointments/events, user information, etc. etc.
  * one calendar is associated with each CalendarView (@see calendarview.h).
  * This is an abstract base class defining the interface to a calendar. It is
  * implemented by subclasses like @see CalendarLocal, which use different
  * methods to store and access the data.
  */
class Calendar {
  public:
    /** constructs a new calendar, with variables initialized to sane values. */
    Calendar();
    Calendar(const QString &timeZoneId);
    virtual ~Calendar();

    /** Return the iCalendar format class the calendar object uses. */
    ICalFormat *iCalFormat();

    /** loads a calendar on disk into the current calendar.
     * Returns TRUE if successful, else returns FALSE.
     * @param fileName the name of the calendar on disk.
     */
    virtual bool load(const QString &fileName) = 0;
    /** writes out the calendar to disk in the format specified by the format
     * parameter. If the format is 0, vCalendar is used. Returns true if
     * successful and false on error.
     * @param fileName the name of the file
     */
    virtual bool save(const QString &fileName,CalFormat *format=0) = 0;
    /** clears out the current calendar, freeing all used memory etc. etc. */
    virtual void close() = 0;
  
    /** set the owner of the calendar.  Should be owner's full name. */
    const QString &getOwner() const;
    /** return the owner of the calendar's full name. */
    void setOwner(const QString &os);
    /** set the email address of the calendar owner. */
    const QString &getEmail();
    /** return the email address of the calendar owner. */
    void setEmail(const QString &);
  
    /** set time zone from a timezone string (e.g. -2:00) */
    void setTimeZone(const QString & tz);
    /** set time zone froma aminutes value (e.g. -60) */
    void setTimeZone(int tz);
    /** Return time zone as offest in minutes */
    int getTimeZone() const;
    /* compute an ISO 8601 format string from the time zone. */
    QString getTimeZoneStr() const;
    /** set time zone id (see /usr/share/zoneinfo/zone.tab for list of legal values) */
    void setTimeZoneId(const QString &);
    /** Return time zone id. */
    QString timeZoneId() const;
  
    /** adds a Event to this calendar object.
     * @param anEvent a pointer to the event to add
     */
    virtual void addEvent(Event *anEvent) = 0;
    /** Delete event from calendar */
    virtual void deleteEvent(Event *) = 0;

    /** retrieves an event on the basis of the unique string ID. */
    virtual Event *getEvent(const QString &UniqueStr) = 0;
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
      The calendar filter is applied.
    */
    QPtrList<Event> getEventsForDate(const QDate &date,bool sorted=false);
    /**
      Get events, which occur on the given date.
      The calendar filter is applied.
    */
    QPtrList<Event> getEventsForDate(const QDateTime &qdt);
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
      The calendar filter is applied.
    */
    QPtrList<Event> getEvents(const QDate &start,const QDate &end,
                             bool inclusive=false);
    /**
      Return all events in calendar
    */
    virtual QPtrList<Event> getAllEvents() = 0;
  
    /**
      Set calendar filter, which filters events for the getEvents* functions.
      Calendar takes ownership of the filter.
    */
    void setFilter(CalFilter *);
    /**
      Return calendar filter.
    */
    CalFilter *filter();
      
    /** returns the number of events that are present on the specified date. */
    virtual int numEvents(const QDate &qd) = 0;
  
    /** add a todo to the todolist. */
    virtual void addTodo(Todo *todo) = 0;
    /** remove a todo from the todolist. */
    virtual void deleteTodo(Todo *) = 0;
    QPtrList<Todo> getFilteredTodoList();
    virtual const QPtrList<Todo> &getTodoList() const = 0;
    /** searches todolist for an event with this unique string identifier,
      returns a pointer or null. */
    virtual Todo *getTodo(const QString &UniqueStr) = 0;
    /** Returns list of todos due on the specified date */
    virtual QPtrList<Todo> getTodosForDate(const QDate & date) = 0;

    /** Add a Journal entry to calendar */
    virtual void addJournal(Journal *) = 0;
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &) = 0;
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID) = 0;
    /** Return list of all Journal entries */
    virtual QPtrList<Journal> journalList() = 0;

    /** Add an incidence to calendar. */
    void addIncidence(Incidence *);
  
    /** Enable/Disable dialogs shown by calendar class */  
    void showDialogs(bool d);

    /** Return all alarms, which ocur in the given time interval. */
    virtual Alarm::List alarms( const QDateTime &from,
                                const QDateTime &to ) = 0;

  protected:
    /**
      Get events, which occur on the given date.
    */
    virtual QPtrList<Event> eventsForDate(const QDate &date,
                                         bool sorted=false) = 0;
    /**
      Get events, which occur on the given date.
    */
    virtual QPtrList<Event> eventsForDate(const QDateTime &qdt) = 0;
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
    */
    virtual QPtrList<Event> events(const QDate &start,const QDate &end,
                                  bool inclusive=false) = 0;
  
    CalFormat *mFormat;
    CalFormat *mDndFormat;  // format used for drag and drop operations
    ICalFormat *mICalFormat;
  
  private:
    void init();
  
    QString mOwner;        // who the calendar belongs to
    QString mOwnerEmail;   // email address of the owner
    int mTimeZone;         // timezone OFFSET from GMT (MINUTES)
    bool mDialogsOn;       // display various GUI dialogs?

    CalFilter *mFilter;
    
    QString mTimeZoneId;
};
  
}

#endif
