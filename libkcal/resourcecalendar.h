/*
    This file is part of libkdepim.
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

#ifndef KCAL_RESOURCECALENDAR_H
#define KCAL_RESOURCECALENDAR_H

#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>

#include <kconfig.h>

#include "alarm.h"

#include <kresources/resource.h>
#include <kresources/resourcemanager.h>

namespace KCal {

class CalFormat;
class Event;
class Todo;
class IncidenceBase;
class Journal;

/**
  @internal
*/
class ResourceCalendar : public QObject, public KRES::Resource
{ 
    Q_OBJECT
  public:
    ResourceCalendar( const KConfig * );
    virtual ~ResourceCalendar();

    virtual void writeConfig( KConfig* config );

    /**
      Load resource data. After calling this function all data is accessible by
      calling the incidence/event/todo/etc. accessor functions.
      
      If data is actually loaded within this function or the loading is delayed
      until it is accessed by another function depends on the implementation of
      the resource.
      
      If loading the data takes significant time, the resource should return
      cached values, if available and return the results via the resourceChanged
      signal. When the resource has finished loading the resourceLoaded() signal
      is emitted.
      
      Calling this function multiple times should have the same effect as
      calling it once, given that the data isn't changed between calls.
    */
    virtual bool load() = 0;

    /**
      Save resource data. After calling this function it is save to close the
      resource without losing data.
      
      If data is actually saved within this function or saving is delayed
      depends on the implementation of the resource.
      
      If saving the data takes significant time, the resource should return from
      the function, do the saving in the background and notify the end of the
      save by emitting the signal resourceSaved().
    */
    virtual bool save() = 0;

    virtual bool isSaving() { return false; }

    virtual bool addIncidence( Incidence * );

    /** Add Event to calendar. */
    virtual bool addEvent(Event *anEvent) = 0;

    /** deletes an event from this calendar. */
    virtual void deleteEvent(Event *) = 0;

    /** signals that an event has been changed by the app */
    // virtual void eventChanged(Event *) = 0;

    /** Synchronous functions */

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    virtual Event *event(const QString &UniqueStr) = 0;

    /**
      Return unfiltered list of all events in calendar. Use with care,
      this can be a bad idea for server-based calendars.
    */
    virtual QPtrList<Event> rawEvents() = 0;

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    virtual QPtrList<Event> rawEventsForDate( const QDate &date, bool sorted = false ) = 0;

    /**
      Get unfiltered events for date \a qdt.
    */
    virtual QPtrList<Event> rawEventsForDate( const QDateTime &qdt ) = 0;

    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
    */
    virtual QPtrList<Event> rawEvents( const QDate &start, const QDate &end,
                               bool inclusive = false ) = 0;

    /** Asynchronous functions */

    /**
      Request events for the given period (inclusive). This will result
      in one or more eventsAdded() signals and, in the longer term, also
      possibly in eventsModified() and eventsDeleted() signals.
    */
    // should be pure virtual
//    virtual void subscribeEvents( const QDate& start, const QDate& end ) {};

    /**
      Stop receiving event signals for the given period (inclusive). After this call,
      the calendar resource will no longer send eventsAdded, eventsModified or
      eventsDeleted signals for events falling completely in this period. The resource
      MAY delete the Events objects. The application MUST NOT dereference pointers 
      to the relevant Events after this call.
    */
    // should be pure virtual
//    virtual void unsubscribeEvents( const QDate& start, const QDate& end ) {};

  signals:
    // Maybe: make one signal, with bools or a flag for "is deleted" and "is new"?
    /** These Events are added to the calendar, or they are in the calendar
     * and I haven't yet told you about them
     */
    void eventsAdded( QPtrList<Event>& events );
    /**
     * The Events in events have been modified
     */
    void eventsModified( QPtrList<Event>& events );
    /**
     * The Events in events have been deleted. Do not reference these
     * events after this call.
     */
    void eventsDeleted( QPtrList<Event>& events );

    void resourceChanged( ResourceCalendar * );
    void resourceLoaded( ResourceCalendar * );
    void resourceSaved( ResourceCalendar * );

  public:
    /**
      Add a todo to the todolist.
    */
    virtual bool addTodo( Todo *todo ) = 0;
    /**
      Remove a todo from the todolist.
    */
    virtual void deleteTodo( Todo * ) = 0;
    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    virtual Todo *todo( const QString &uid ) = 0;
    /**
      Return list of all todos.
    */
    virtual QPtrList<Todo> rawTodos() = 0;
    /**
      Returns list of todos due on the specified date.
    */
    virtual QPtrList<Todo> todos( const QDate &date ) = 0;


    /** Add a Journal entry to calendar */
    virtual bool addJournal(Journal *) = 0;

    /** Remove a Journal entry from calendar */
    // virtual void deleteJournal(Journal *) = 0;

    /** Return Journal for given date */
    virtual Journal *journal(const QDate &) = 0;

    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID) = 0;

    /** Return list of all Journals stored in calendar */
    virtual QPtrList<Journal> journals() = 0;


    /** Return all alarms, which ocur in the given time interval. */
    virtual Alarm::List alarms( const QDateTime &from, const QDateTime &to ) = 0;

    /** Return all alarms, which ocur before given date. */
    virtual Alarm::List alarmsTo( const QDateTime &to ) = 0;
    

    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    virtual void update(IncidenceBase *incidence) = 0;

    /** Returns a list of all incideces */
    QPtrList<Incidence> rawIncidences();

  protected:

};

typedef KRES::ResourceManager<ResourceCalendar> CalendarResourceManager;

}

#endif
