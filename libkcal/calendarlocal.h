/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_CALENDARLOCAL_H
#define KCAL_CALENDARLOCAL_H

#include "calendar.h"
#include <qdict.h>
#include <kdepimmacros.h>

namespace KCal {

class CalFormat;

/**
  This class provides a calendar stored as a local file.
*/
class LIBKCAL_EXPORT CalendarLocal : public Calendar
{
  public:
    /**
      Constructs a new calendar, with variables initialized to sane values.
    */
    CalendarLocal( const QString &timeZoneId );
    ~CalendarLocal();

    /**
      Loads a calendar on disk in vCalendar or iCalendar format into the current
      calendar. Incidences already present are preserved. If an event of the
      file to be loaded has the same unique id as an incidence already present
      the new incidence is ignored.

      To load a CalendarLocal object from a file without preserving existing
      incidences call close() before load().

      @return true, if successful, false on error.
      @param fileName the name of the calendar on disk.
    */
    bool load( const QString &fileName );

    /**
      Writes out the calendar to disk in the specified \a format.
      CalendarLocal takes ownership of the CalFormat object.
      @param fileName the name of the file
      @param format the format to use
      @return true, if successful, false on error.
    */
    bool save( const QString &fileName, CalFormat *format = 0 );

    /**
      Clears out the current calendar, freeing all used memory etc. etc.
    */
    void close();

    void save() {}

    /**
      Add Event to calendar.
    */
    bool addEvent( Event *event );
    /**
      Deletes an event from this calendar.
    */
    bool deleteEvent( Event *event );
    /**
      Deletes all events from this calendar.
    */
    void deleteAllEvents();

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event( const QString &uid );
    /**
      Return unfiltered list of all events in calendar.
    */
    Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );

    /**
      Add a todo to the todolist.
    */
    bool addTodo( Todo *todo );
    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo * );
    /**
      Deletes all todos from this calendar.
    */
    void deleteAllTodos();
    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    Todo *todo( const QString &uid );
    /**
      Return list of all todos.
    */
    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Returns list of todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );

    /**
      Add a Journal entry to calendar.
    */
    bool addJournal( Journal * );
    /**
      Remove a Journal from the calendar.
    */
    bool deleteJournal( Journal * );
    /**
      Deletes all journals from this calendar.
    */
    void deleteAllJournals();
    /**
      Return Journal with given UID.
    */
    Journal *journal( const QString &uid );
    /**
       Return list of all journals.
    */
    Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
       Get unfiltered journals for a given date.
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /**
      Return all alarms, which ocur in the given time interval.
    */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /**
      Return all alarms, which ocur before given date.
    */
    Alarm::List alarmsTo( const QDateTime &to );

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    Event::List rawEventsForDate( const QDate &date, EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Get unfiltered events for date \a qdt.
    */
    Event::List rawEventsForDate( const QDateTime &qdt );
    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
      If inclusive is set to false, all events which overlap the range are
      returned. An event's entire time span is considered in evaluating
      whether it should be returned. For a non-recurring event, its span is
      from its start to its end date. For a recurring event, its time span is
      from its first to its last recurrence.
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                               bool inclusive = false );

  protected:

    /** inserts an event into its "proper place" in the calendar. */
    void insertEvent( Event *event );

    /** Append alarms of incidence in interval to list of alarms. */
    void appendAlarms( Alarm::List &alarms, Incidence *incidence,
                       const QDateTime &from, const QDateTime &to );

    /** Append alarms of recurring events in interval to list of alarms. */
    void appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
                       const QDateTime &from, const QDateTime &to );

  private:
    void init();

    typedef QDict<Event> EventDict;
    typedef QDictIterator<Event> EventDictIterator;
    EventDict mEvents;
    Todo::List mTodoList;
    Journal::List mJournalList;

    Incidence::List mDeletedIncidences;
		QString mFileName;

    class Private;
    Private *d;
};

}

#endif
