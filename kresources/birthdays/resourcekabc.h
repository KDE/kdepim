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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_RESOURCEKABC_H
#define KCAL_RESOURCEKABC_H

#include <QtCore/QString>

#include <kconfig.h>

#include <kcal/incidence.h>
#include <kcal/calendarlocal.h>
#include <kcal/resourcecalendar.h>
#include <kabc/addressbook.h>

#include <kdemacros.h>

namespace KCal {

/**
  Resource providing birthdays and anniversaries as events.
*/
class KDE_EXPORT ResourceKABC : public ResourceCalendar
{
    Q_OBJECT

    friend class ResourceKABCConfig;

  public:
    ResourceKABC();
    ResourceKABC( const KConfigGroup & );
    virtual ~ResourceKABC();

    void readConfig( const KConfigGroup &group );
    void writeConfig( KConfigGroup &group );

    void setAlarm( bool );
    bool alarm();

    void setAlarmDays( int );
    int alarmDays();

    void setCategories( const QStringList &categories );
    QStringList categories() const;

    void setUseCategories( bool useCategories );
    bool useCategories() const;

    bool isSaving();

    KABC::Lock *lock();

    /** Add Event to calendar. */
    bool addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    bool deleteEvent(Event *);
    /** removes all events from this calendar. */
    void deleteAllEvents() {}
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
    Event::List rawEventsForDate( const QDate &date, EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Get unfiltered events for date \a dt.
    */
    Event::List rawEventsForDate( const KDateTime &dt );
    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
    */
    Event::List rawEvents( const QDate &start, const QDate &end,
                           bool inclusive = false );

    /**
      Add a todo to the todolist.
    */
    bool addTodo( Todo *todo );
    /**
      Remove a todo from the todolist.
    */
    bool deleteTodo( Todo * );
    /**
      Removes all todos from the todolist.
    */
    void deleteAllTodos() {}

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
    bool deleteJournal( Journal * );
    /** Removes all journals from the calendar */
    void deleteAllJournals() {}
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

    /** Return all alarms, which occur in the given time interval. */
    Alarm::List alarms( const KDateTime &from, const KDateTime &to );

    /** Return all alarms, which occur before given date. */
    Alarm::List alarmsTo( const KDateTime &to );

    void dump() const;

    virtual void setTimeSpec( const KDateTime::Spec &timeSpec );

    /**
       Get the viewing time specification (time zone etc.) for the calendar.

       @return time specification
    */
    KDateTime::Spec timeSpec() const;

    virtual void setTimeZoneId( const QString &timeZoneId );
    virtual QString timeZoneId() const;

    virtual void shiftTimes(const KDateTime::Spec &oldSpec, const KDateTime::Spec &newSpec);

  protected:
    bool doOpen();
    bool doLoad();
    bool doSave();

  private Q_SLOTS:
    void reload();

  private:
    void init();

    CalendarLocal mCalendar;

    int mAlarmDays;
    bool mAlarm;
    QStringList mCategories;
    bool mUseCategories;
    KABC::AddressBook *mAddressbook;

    KABC::Lock *mLock;

    class Private;
    Private *d;
};

}

#endif
