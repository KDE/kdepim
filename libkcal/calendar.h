/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_CALENDAR_H
#define KCAL_CALENDAR_H

#include <qobject.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qdict.h>
#include <kdepimmacros.h>

#include "customproperties.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "kcalversion.h"
#include "person.h"

#define _TIME_ZONE "-0500" /* hardcoded, overridden in config file. */

class KConfig;

namespace KCal {

/**
   Sort direction.
*/
enum SortDirection {
  SortDirectionAscending,
  SortDirectionDescending
};

/**
   How events are to be sorted.
*/
enum EventSortField {
  EventSortUnsorted,
  EventSortStartDate,
  EventSortEndDate,
  EventSortSummary
};

/**
   How todos are to be sorted.
*/
enum TodoSortField {
  TodoSortUnsorted,
  TodoSortStartDate,
  TodoSortDueDate,
  TodoSortPriority,
  TodoSortPercentComplete,
  TodoSortSummary
};

/**
   How journals are to be sorted.
*/
enum JournalSortField {
  JournalSortUnsorted,
  JournalSortDate,
  JournalSortSummary
};

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
class KDE_EXPORT Calendar : public QObject, public CustomProperties,
                 public IncidenceBase::Observer
{
    Q_OBJECT
  public:
    Calendar();
    Calendar(const QString &timeZoneId);
    virtual ~Calendar();

    /**
      Clears out the current calendar, freeing all used memory etc.
    */
    virtual void close() = 0;

    /**
      Sync changes in memory to persistant storage.
    */
    virtual void save() = 0;

    virtual bool isSaving() { return false; }

    /**
      Return the owner of the calendar's full name.
    */
    const Person &getOwner() const;
    /**
      Set the owner of the calendar. Should be owner's full name.
    */
    void setOwner( const Person &owner );

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
       Sort eventList according to sortField
    */
    static Event::List sortEvents( Event::List *eventList,
                            EventSortField sortField,
                            SortDirection sortDirection );

    /**
       Sort todoList according to sortField
    */
    static Todo::List sortTodos( Todo::List *todoList,
                          TodoSortField sortField,
                          SortDirection sortDirection );

    /**
       Sort journalList according to sortField
    */
    static Journal::List sortJournals( Journal::List *journalList,
                                JournalSortField sortField,
                                SortDirection sortDirection );

    /**
      Add an incidence to calendar.

      @return true on success, false on error.
    */
    virtual bool addIncidence( Incidence * );
    /**
      Delete an incidence from calendar.

      @return true on success, false on error.
    */
    virtual bool deleteIncidence( Incidence * );
    /**
      Return filtered list of all incidences of this calendar.
    */
    virtual Incidence::List incidences();
    virtual Incidence::List incidences( const QDate &qdt );

    /**
      Return unfiltered list of all incidences of this calendar.
    */
    virtual Incidence::List rawIncidences();

    /**
      Return a list of all categories used in this calendar.
    */
    QStringList incidenceCategories();

    /**
      Adds a Event to this calendar object.
      @return true on success, false on error.
    */
    virtual bool addEvent( Event * ) = 0;
    /**
      Delete event from calendar.
    */
    virtual void deleteEvent( Event * ) = 0;
    /**
      Retrieves an event on the basis of the unique string ID.
    */
    virtual Event *event( const QString &UniqueStr ) = 0;
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
      The calendar filter is applied.
    */
    //TODO: Deprecate
    Event::List events( const QDate &date, bool sorted = false );
    //Event::List events( const QDate &date, EventSorted sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Get events, which occur on the given date.
      The calendar filter is applied.
    */
    Event::List events( const QDateTime &qdt );
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
      The calendar filter is applied.
    */
    Event::List events( const QDate &start, const QDate &end,
                        bool inclusive = false);
    /**
       Return filtered list of all events sorted according to sortField.
    */
    virtual Event::List events( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Return unfiltered list of all events in calendar.
    */
    virtual Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending ) = 0;

    /**
      Add a todo to the todolist.

      @return true on success, false on error.
    */
    virtual bool addTodo( Todo *todo ) = 0;
    /**
      Remove a todo from the todolist.
    */
    virtual void deleteTodo( Todo * ) = 0;
    /**
      Return filtered list of todos.
    */
    virtual Todo::List todos( TodoSortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
    */
    virtual Todo *todo( const QString &uid ) = 0;
    /**
      Returns list of todos due on the specified date.
    */
    virtual Todo::List todos( const QDate &date );
    /**
      Return unfiltered list of todos.
    */
    virtual Todo::List rawTodos( TodoSortField sortField = TodoSortUnsorted, SortDirection sortDirection = SortDirectionAscending ) = 0;
    /**
      Return unfiltered list of todos.
    */
    virtual Todo::List rawTodosForDate( const QDate &date ) = 0;

    /**
      Add a Journal entry to calendar.

      @return true on success, false on error.
    */
    virtual bool addJournal( Journal * ) = 0;
    /**
      Remove a journal entry from the calendar.
    */
    virtual void deleteJournal( Journal * ) = 0;
    /**
      Return Journal with given UID.
    */
    virtual Journal *journal( const QString &uid ) = 0;
    /**
      Return list of all Journal entries.
    */
    virtual Journal::List journals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Returns list of journals for the specified date.
    */
    virtual Journal::List journals( const QDate &date );
    /**
       Return unfiltered list of all journals in calendar.
    */
    virtual Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted, SortDirection sortDirection = SortDirectionAscending ) = 0;
    /**
      Return unfiltered list of journals for a given date.
    */
    virtual Journal::List rawJournalsForDate( const QDate &date ) = 0;

    /**
      Searches all incidence types for an incidence with this unique
      string identifier, returns a pointer or null.
    */
    Incidence *incidence( const QString &uid );

    /**
      Searches all events and todos for an incidence with this
      scheduling ID. Returns a pointer or null.
    */
    Incidence *incidenceFromSchedulingID( const QString &uid );

    /**
      Setup relations for an incidence.
    */
    virtual void setupRelations( Incidence * );
    /**
      Remove all relations to an incidence
    */
    virtual void removeRelations( Incidence * );

    /**
      Set calendar filter, which filters events for the events() functions.
      The Filter object is owned by the caller.
    */
    void setFilter( CalFilter * );
    /**
      Return calendar filter.
    */
    CalFilter *filter();

    /**
      Return all alarms, which ocur in the given time interval.
    */
    virtual Alarm::List alarms( const QDateTime &from,
                                const QDateTime &to ) = 0;

    class Observer {
      public:
        virtual void calendarModified( bool, Calendar * ) {};

        virtual void calendarIncidenceAdded( Incidence * ) {}
        virtual void calendarIncidenceChanged( Incidence * ) {}
        virtual void calendarIncidenceDeleted( Incidence * ) {}
    };

    void registerObserver( Observer * );
    void unregisterObserver( Observer * );

    void setModified( bool );
    /**
      Return whether the calendar was modified since opening / saving
     */
    bool isModified() const { return mModified; }

    /**
      Set product id returned by loadedProductId(). This function is only
      useful for the calendar loading code.
    */
    void setLoadedProductId( const QString & );
    /**
      Return product id taken from file that has been loaded. Returns
      QString::null, if no calendar has been loaded.
    */
    QString loadedProductId();

    /**
      Merge lists of events, todos and journals to a list of incidences.
    */
    static Incidence::List mergeIncidenceList( const Event::List &,
                                               const Todo::List &,
                                               const Journal::List & );

    virtual bool beginChange( Incidence * );
    virtual bool endChange( Incidence * );

    /**
      Dissociate an incidence from a recurring incidence. By default, only one
      single event for the given date will be dissociated and returned.
      If single == false, the recurrence will be split at date, the
      old incidence will have its recurrence ending at date and the
      new incidence (return value) will have all recurrences past the date.
    */
    Incidence *dissociateOccurrence( Incidence *incidence, QDate date,
                                     bool single = true );

  signals:
    void calendarChanged();
    void calendarSaved();
    void calendarLoaded();

  protected:
    /**
      The observer interface. So far not implemented.
    */
    void incidenceUpdated( IncidenceBase * );
  public:
    /**
      Get unfiltered events, which occur on the given date.
    */
    virtual Event::List rawEventsForDate( const QDateTime &qdt ) = 0;
    /**
      Get unfiltered events, which occur on the given date.
    */
    //TODO: Deprecate
    virtual Event::List rawEventsForDate( const QDate &date,
                                          bool sorted = false ) = 0;
    //virtual Event::List rawEventsForDate( const QDate &date, EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending ) = 0;
    /**
      Get events in a range of dates. If inclusive is set to true, only events
      are returned, which are completely included in the range.
    */
    virtual Event::List rawEvents( const QDate &start, const QDate &end,
                                   bool inclusive = false ) = 0;

  protected:
    /**
      let the subclasses of KCal::Calendar set the time zone
    */
    virtual void doSetTimeZoneId( const QString & ) {}

    void notifyIncidenceAdded( Incidence * );
    void notifyIncidenceChanged( Incidence * );
    void notifyIncidenceDeleted( Incidence * );

    void setObserversEnabled( bool enabled );

  private:
    void init();

    Person mOwner;         // who the calendar belongs to

    int mTimeZone;         // timezone OFFSET from GMT (MINUTES)
    bool mLocalTime;       // use local time, not UTC or a time zone

    CalFilter *mFilter;
    CalFilter *mDefaultFilter;

    QString mTimeZoneId;

    QPtrList<Observer> mObservers;
    bool mNewObserver;
    bool mObserversEnabled;

    bool mModified;

    QString mLoadedProductId;

    // This list is used to put together related todos
    QDict<Incidence> mOrphans;
    QDict<Incidence> mOrphanUids;

    class Private;
    Private *d;
};

}

#endif
