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
#ifndef KCAL_RESOURCEKABC_H
#define KCAL_RESOURCEKABC_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include <kconfig.h>

#include "incidence.h"
#include "calendarlocal.h"
#include <kabc/addressbook.h>

#include "resourcecalendar.h"

#include <kdepimmacros.h>

namespace KIO {
class FileCopyJob;
class Job;
}

namespace KCal {

/**
  Resource providing birthdays and anniversaries as events.
*/
class KDE_EXPORT ResourceKABC : public ResourceCalendar
{
    Q_OBJECT

    friend class ResourceKABCConfig;

  public:
    ResourceKABC( const KConfig * );
    ResourceKABC( );
    virtual ~ResourceKABC();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig* config );

    void setAlarm( bool );
    bool alarm();

    void setAlarmDays( int );
    int alarmDays();

    bool isSaving();

    KABC::Lock *lock();

    /** Add Event to calendar. */
    bool addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event(const QString &UniqueStr);
    /**
      Return unfiltered list of all events in calendar.
    */
    Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    //TODO: Deprecate
    Event::List rawEventsForDate( const QDate &date, bool sorted = false );
    //Event::List rawEventsForDate( const QDate &date, EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Get unfiltered events for date \a qdt.
    */
    Event::List rawEventsForDate( const QDateTime &qdt );
    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           bool inclusive = false );

    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    // QString getHolidayForDate(const QDate &qd);

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
    Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Returns list of todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );
    /** Add a Journal entry to calendar */
    virtual bool addJournal(Journal *);
    /** Remove journal from the calendar. */
    void deleteJournal( Journal * );
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &uid);
    /**
      Return list of all journals.
    */
    Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Returns list of journals for the given date.
    */
    Journal::List rawJournalsForDate( const QDate &date );

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /** Return all alarms, which ocur before given date. */
    Alarm::List alarmsTo( const QDateTime &to );

    void dump() const;

    void setTimeZoneId( const QString& tzid );

  protected:
    bool doOpen();
    bool doLoad();
    bool doSave();

  private slots:
    void reload();

  private:
    void init();

    CalendarLocal mCalendar;

    int mAlarmDays;
    bool mAlarm;
    KABC::AddressBook *mAddressbook;

    KABC::Lock *mLock;

    class Private;
    Private *d;
};

}

#endif
