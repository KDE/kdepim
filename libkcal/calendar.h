/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001,2003,2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "customproperties.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "kcalversion.h"

#define _TIME_ZONE "-0500" /* hardcoded, overridden in config file. */

class KConfig;

namespace KCal {

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
class Calendar : public QObject, public CustomProperties,
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
      @param anEvent a pointer to the event to add
      
      @return true on success, false on error.
    */
    virtual bool addEvent( Event *anEvent ) = 0;
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
    Event::List events( const QDate &date, bool sorted = false );
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
      Return filtered list of all events in calendar.
    */
    virtual Event::List events();
    /**
      Return unfiltered list of all events in calendar.
    */
    virtual Event::List rawEvents() = 0;

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
      Return filterd list of todos.
    */
    virtual Todo::List todos();
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
    virtual Todo::List rawTodos() = 0;
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
    virtual Journal::List journals() = 0;
    // TODO: Add rawJournals() and rawJournal( QDate )

    /**
      Searches all incidence types for an incidence with this unique
      string identifier, returns a pointer or null.
    */
    Incidence *incidence( const QString &UID );

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
    virtual Event::List rawEventsForDate( const QDate &date,
                                          bool sorted = false ) = 0;  
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
  
    QString mOwner;        // who the calendar belongs to
    QString mOwnerEmail;   // email address of the owner
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
