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
  This is a null calendar object which does nothing. It can be passed to
  functions which need a calendar object when there actually isn't a real
  calendar yet. CalendarNull can be used to implement the null object design
  pattern. Instead of passing a 0 pointer and checking for 0 with each access
  a CalendarNull object can be passed.
*/
class LIBKCAL_EXPORT CalendarNull : public Calendar
{
  public:
    CalendarNull() {}
    ~CalendarNull() {}

    static CalendarNull *self();

    void incidenceUpdated( IncidenceBase * ) {}

    void close() {}
    void save() {}

    bool addEvent( Event * ) { return false; }
    void deleteEvent( Event * ) {}
    Event *event( const QString & ) { return 0; }
    Event::List rawEvents( EventSortField, SortDirection ) { return Event::List(); }
    Event::List rawEvents( const QDate &, const QDate &, bool ) { return Event::List(); }
    Event::List rawEventsForDate( const QDateTime & ) { return Event::List(); }
    Event::List rawEventsForDate( const QDate &, bool ) { return Event::List(); }

    bool addTodo( Todo * ) { return false; }
    void deleteTodo( Todo * ) {}
    Todo *todo( const QString & ) { return 0; }
    Todo::List rawTodos( TodoSortField, SortDirection ) { return Todo::List(); }
    Todo::List rawTodosForDate( const QDate & ) { return Todo::List(); }

    bool addJournal( Journal * ) { return false; }
    void deleteJournal( Journal * ) {}
    Journal *journal( const QString & ) { return 0; }
    Journal::List rawJournals( JournalSortField, SortDirection ) { return Journal::List(); }
    Journal::List rawJournalsForDate( const QDate & ) { return Journal::List(); }

    Alarm::List alarms( const QDateTime &, const QDateTime & ) { return Alarm::List(); }

  private:
    static CalendarNull *mSelf;

    class Private;
    Private *d;
};

}

#endif
