/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
/*
  @file calendarnull.h
  A null calendar class with does nothing.

  @author Cornelius Schumacher
*/
#ifndef KCAL_CALENDARNULL_H
#define KCAL_CALENDARNULL_H

#include "calendar.h"
#include "libkcal_export.h"

class KConfig;

/**
   @namespace KCal
   Namespace KCal is for global classes, objects and/or functions in libkcal.
*/
namespace KCal {

/**
   @class CalendarNull

   This is a null calendar class which does nothing.  It can be passed to
   functions which need a calendar object when there actually isn't a real
   calendar yet.  CalendarNull can be used to implement the null object
   design pattern.  Instead of passing a 0 pointer and checking for 0 with
   each access a CalendarNull object can be passed.
*/
class LIBKCAL_EXPORT CalendarNull : public Calendar
{
  public:
    /**
       Constructor.
    */
    CalendarNull( const QString &timeZoneId );

    /**
       Destructor.
    */
    ~CalendarNull() {}

    /**
       Returns a pointer to a CalendarNull object, which is constructed
       if necessary.
    */
    static CalendarNull *self();

    /**
       Clears out the current Calendar, freeing all used memory etc.
    */
    void close() {}

    /**
       Sync changes in memory to persistant storage.
    */
    void save() {}

// Event Specific Methods //

    /**
       Insert an Evenet into the Calendar.

       First parameter is a pointer to the Event to insert.

       Returns false.
    */
    bool addEvent( Event * /*event*/ )
      { return false; }

    /**
       Remove an Event from the Calendar.

       First parameter is a pointer to the Event to remove.

       Returns false.
    */
    bool deleteEvent( Event * /*event*/ )
      { return false; }

    /**
       Return a sorted, unfiltered list of all Events for this Calendar.

       First parameter specifies the EventSortField.\n
       Second parameter specifies the SortDirection.

       Returns an empty Event List.
    */
    Event::List rawEvents( EventSortField /*sortField*/,
                           SortDirection /*sortDirection*/ )
      { return Event::List(); }

    /**
       Return an unfiltered list of all Events occurring within a date range.

       First parameter is the starting date.\n
       Second parameter is the ending date.\n
       Third parameter, if true, specifies that only Events which are
       completely included within the date range are returned.

       Returns an empty Event List.
    */
    Event::List rawEvents( const QDate & /*start*/, const QDate & /*end*/,
                           bool /*inclusive*/ )
      { return Event::List(); }

    /**
       Return an unfiltered list of all Events which occur on the given
       timestamp.

       First parameter is a QDateTime to return unfiltered events for.

       Returns an empty Event List.
    */
    Event::List rawEventsForDate( const QDateTime & /*qdt*/ )
      { return Event::List(); }

    /**
       Return a sorted, unfiltered list of all Events which occur on the given
       date.  The Events are sorted according to @a sortField and
       @a sortDirection.

       First parameter is a QDate to return unfiltered Events for.\n
       Second parameter specifies the EventSortField.\n
       Third parameter specifies the SortDirection.

       Returns an empty Event List.
    */
    Event::List rawEventsForDate(
      const QDate & /*date*/,
      EventSortField /*sortField=EventSortUnsorted*/,
      SortDirection /*sortDirection=SortDirectionAscending*/ )
      { return Event::List(); }

    /**
       Returns the Event associated with the given unique identifier.

       First parameter is a unique identifier string.

       Return a null Event pointer.
    */
    Event *event( const QString & /*uid*/ )
      { return 0; }

// Todo Specific Methods //

    /**
       Insert a Todo into the Calendar.

       First parameter is a pointer to the Todo to insert.

       Returns false.
    */
    bool addTodo( Todo * /*todo*/ )
      { return false; }

    /**
       Remove a Todo from the Calendar.

       First parameter is a pointer to the Todo to remove.

       Returns false.
    */
    bool deleteTodo( Todo * /*todo*/ )
      { return false; }

    /**
       Return a sorted, unfiltered list of all Todos for this Calendar.

       First parameter specifies the TodoSortField.\n
       Second parameter specifies the SortDirection.

       Returns an empty Todo List.
    */
    Todo::List rawTodos( TodoSortField /*sortField*/,
                         SortDirection /*sortDirection*/ )
      { return Todo::List(); }

    /**
       Return an unfiltered list of all Todos for this Calendar which
       are due on the specifed date.

       First parameter is the due date to return unfiltered Todos for.

       Returns an empty Todo List.
    */
    Todo::List rawTodosForDate( const QDate & /*date*/ )
      { return Todo::List(); }

    /**
       Returns the Todo associated with the given unique identifier.

       First parameter is a unique identifier string.

       Returns a null Todo pointer.
    */
    Todo *todo( const QString & /*uid*/ )
      { return 0; }

// Journal Specific Methods //

    /**
       Insert a Journal into the Calendar.

       First parameter is a pointer to the Journal to insert.

       Returns false.
    */
    bool addJournal( Journal * /*journal*/ )
      { return false; }

    /**
       Remove a Journal from the Calendar.

       First parameter is a pointer to the Journal to remove.

       Returns false.
    */
    bool deleteJournal( Journal * /*journal*/ )
      { return false; }

    /**
       Return a sorted, filtered list of all Journals for this Calendar.

       First parameter specifies the JournalSortField.\n
       Second parameterd specifies the SortDirection.

       Returns an empty Journal List.
    */
    Journal::List rawJournals( JournalSortField /*sortField*/,
                               SortDirection /*sortDirection*/ )
      { return Journal::List(); }

    /**
       Return an unfiltered list of all Journals for on the specifed date.

       First parameter specifies the data to return the unfiltered Journals for.

       Returns an empty Journal List.
    */
    Journal::List rawJournalsForDate( const QDate & /*date*/ )
      { return Journal::List(); }

    /**
       Returns the Journal associated with the given unique identifier.

       First parameter is a unique identifier string.

       Returns an null Journal pointer.
    */
    Journal *journal( const QString & /*uid*/ )
      { return 0; }

// Alarm Specific Methods //

    /**
       Return a list of Alarms within a time range for this Calendar.

       First parameter is the starting timestamp.\n
       Second parameter is the ending timestamp.

       Returns an empty Alarm List.
    */

    Alarm::List alarms( const QDateTime & /*from*/, const QDateTime & /*to*/ )
      { return Alarm::List(); }

// Observer Specific Methods //

    /**
       The Observer interface. So far not implemented.
       First parameter is a pointer an IncidenceBase object.
    */
    void incidenceUpdated( IncidenceBase * /*incidenceBase*/ ) {}

  private:
    static CalendarNull *mSelf;

    class Private;
    Private *d;
};

}

#endif
