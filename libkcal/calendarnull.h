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
#ifndef KCAL_CALENDARNULL_H
#define KCAL_CALENDARNULL_H

#include "calendar.h"
#include "libkcal_export.h"

class KConfig;

namespace KCal {

  /**
   *  This is a null calendar object which does nothing. It can be passed to
   * functions which need a calendar object when there actually isn't a real
   * calendar yet. CalendarNull can be used to implement the null object design
   * pattern. Instead of passing a 0 pointer and checking for 0 with each access
   * a CalendarNull object can be passed.
   */
  class LIBKCAL_EXPORT CalendarNull : public Calendar
  {
  public:
    CalendarNull() {}
    ~CalendarNull() {}

    static CalendarNull *self();

    void incidenceUpdated( IncidenceBase * /*incidenceBase*/ ) {}

    void close() {}
    void save() {}

    bool addEvent( Event * /*event*/ )
      { return false; }

    bool deleteEvent( Event * /*event*/ )
      { return false; }

    Event *event( const QString & /*uid*/ )
      { return 0; }

    Event::List rawEvents( EventSortField /*sortField*/,
                           SortDirection /*sortDirection*/ )
      { return Event::List(); }

    Event::List rawEvents( const QDate & /*start*/, const QDate & /*end*/,
                           bool /*inclusive*/ )
      { return Event::List(); }

    Event::List rawEventsForDate( const QDateTime & /*qdt*/ )
      { return Event::List(); }

    Event::List rawEventsForDate(
      const QDate & /*date*/,
      EventSortField /*sortField=EventSortUnsorted*/,
      SortDirection /*sortDirection=SortDirectionAscending*/ )
      { return Event::List(); }

    bool addTodo( Todo * /*todo*/ )
      { return false; }

    bool deleteTodo( Todo * /*todo*/ )
      { return false; }

    Todo *todo( const QString & /*uid*/ )
      { return 0; }

    Todo::List rawTodos( TodoSortField /*sortField*/,
                         SortDirection /*sortDirection*/ )
      { return Todo::List(); }

    Todo::List rawTodosForDate( const QDate & /*date*/ )
      { return Todo::List(); }

    bool addJournal( Journal * /*journal*/ )
      { return false; }

    bool deleteJournal( Journal * /*journal*/ )
      { return false; }

    Journal *journal( const QString & /*uid*/ )
      { return 0; }

    Journal::List rawJournals( JournalSortField /*sortField*/,
                               SortDirection /*sortDirection*/ )
      { return Journal::List(); }

    Journal::List rawJournalsForDate( const QDate & /*date*/ )
      { return Journal::List(); }

    Alarm::List alarms( const QDateTime & /*from*/, const QDateTime & /*to*/ )
      { return Alarm::List(); }

  private:
    static CalendarNull *mSelf;

    class Private;
    Private *d;
  };

}

#endif
