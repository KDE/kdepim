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
#ifndef KCAL_CALENDARRESOURCES_H
#define KCAL_CALENDARRESOURCES_H

#include <qintdict.h>
#include <qmap.h>

#include "calendar.h"
#include "resourcecalendar.h"

#include <kresources/manager.h>

class QWidget;

namespace KCal {

class CalFormat;

/**
  This class provides a calendar composed of several calendar resources.
*/
class CalendarResources : public Calendar,
                          public KRES::ManagerObserver<ResourceCalendar>
{
    Q_OBJECT
  public:
    class DestinationPolicy
    {
      public:
        DestinationPolicy( CalendarResourceManager *manager )
          : mManager( manager ) {}

        virtual ResourceCalendar *destination( Incidence * ) = 0;

      protected:
        CalendarResourceManager *resourceManager() { return mManager; }

      private:
        CalendarResourceManager *mManager;
    };

    class StandardDestinationPolicy : public DestinationPolicy
    {
      public:
        StandardDestinationPolicy( CalendarResourceManager *manager )
          : DestinationPolicy( manager ) {}

        ResourceCalendar *destination( Incidence * );

      private:
        class Private;
        Private *d;
    };

    class AskDestinationPolicy : public DestinationPolicy
    {
      public:
        AskDestinationPolicy( CalendarResourceManager *manager,
                              QWidget *parent = 0 )
          : DestinationPolicy( manager ), mParent( parent ) {}

        ResourceCalendar *destination( Incidence * );

      private:
        QWidget *mParent;

        class Private;
        Private *d;
    };

    class Ticket
    {
        friend class CalendarResources;
      public:
        ResourceCalendar *resource() const { return mResource; }
        
      private:
        Ticket( ResourceCalendar *r ) : mResource( r ) {}
    
        ResourceCalendar *mResource;

        class Private;
        Private *d;
    };

    /** constructs a new calendar that uses the ResourceManager for "calendar" */
    CalendarResources();
    /** constructs a new calendar, with variables initialized to sane values. */
    CalendarResources( const QString &timeZoneId );
    ~CalendarResources();

    /**
      Read the resources settings from a config file. You have to call this
      method before load().

      @param config The KConfig object which points to the config file.
                    If no object is given (null pointer) the standard config
                    file is used.
     */
    void readConfig( KConfig *config = 0 );

    /**
      Loads all events from the resources. You have to add the resources first
      or call readConfig() to load the system resources.
     */
    void load();

    /**
      Return ResourceManager used by this calendar.
    */
    CalendarResourceManager *resourceManager() const
    {
      return mManager;
    }

    /**
      Set the destinatinpolicy to add incidences always to the standard resource
    */
    void setStandardDestinationPolicy();
    /**
      Set the destinatinpolicy to ask to which resource incidences are added
    */
    void setAskDestinationPolicy();

    /** clears out the current calendar, freeing all used memory etc. etc. */
    void close();

    /**
      Request ticket for saving the calendar. If a ticket is returned the
      calendar is locked for write access until save() or releaseSaveTicket() is
      called.
    */
    Ticket *requestSaveTicket( ResourceCalendar * );
    /**
      Save calendar. If save is successfull, the ticket is deleted. Otherwise
      the caller has to release the ticket with releaseSaveTicket() to abandon
      the save operation or to call save() to try to save again.
    */
    virtual bool save( Ticket * );
    /**
      Release the save ticket. The calendar is unlocked without saving.
    */
    virtual void releaseSaveTicket( Ticket *ticket );

    void save();

    bool isSaving();

    bool addIncidence( Incidence * );

    /** Add Event to calendar. */
    bool addEvent(Event *anEvent);
    /** Add Event to a resource. */
    bool addEvent(Event *anEvent, ResourceCalendar *resource);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event(const QString &UniqueStr);
    /**
      Return filtered list of all events in calendar.
    */
//    Event::List events();
    /**
      Return unfiltered list of all events in calendar.
    */
    Event::List rawEvents();

    /*
      Returns a QString with the text of the holiday (if any) that falls
      on the specified date.
    */
    QString getHolidayForDate(const QDate &qd);

    /**
      Add a todo to the todolist.
    */
    bool addTodo( Todo *todo );
    /** Add Todo to a resource. */
    bool addTodo(Todo *todo, ResourceCalendar *resource);
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
    Todo::List rawTodos();
    /**
      Returns list of todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );

    /** Add a Journal entry to calendar */
    bool addJournal(Journal *);
    /** Remove journal entry. */
    void deleteJournal( Journal * );
    /** Add Event to a resource. */
    bool addJournal(Journal *journal, ResourceCalendar *resource);
    /** Return Journal for given date */
    Journal *journal(const QDate &);
    /** Return Journal with given UID */
    Journal *journal(const QString &UID);
    /** Return list of all Journals stored in calendar */
    Journal::List journals();

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /** Return all alarms, which ocur before given date. */
    Alarm::List alarmsTo( const QDateTime &to );

    /** Return Resource for given uid */
    ResourceCalendar *resource(Incidence *);

    bool beginChange( Incidence * );
    bool endChange( Incidence * );

  signals:
    void signalResourceAdded( ResourceCalendar * );
    void signalResourceModified( ResourceCalendar * );
    void signalResourceDeleted( ResourceCalendar * );

    void signalErrorMessage( const QString & );

    /**
      This signal is emitted to indicate progress of actions the resource
      executes, e.g. while downloading data. The id is unique to the action, the
      percent value shows how much of the action is complete. A percent value of
      -1 indicates that there is progress, but the percent value is unknown.
    */
    void progress( ResourceCalendar *, const QString &id, int percent );

  protected:
    /**
      The observer interface. So far not implemented.
    */
    void incidenceUpdated( IncidenceBase * );

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    Event::List rawEventsForDate( const QDate &date, bool sorted = false );
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

    void connectResource( ResourceCalendar * );

    void resourceAdded( ResourceCalendar *resource );
    void resourceModified( ResourceCalendar *resource );
    void resourceDeleted( ResourceCalendar *resource );

    virtual void doSetTimeZoneId( const QString& tzid );

    int incrementChangeCount( ResourceCalendar * );
    int decrementChangeCount( ResourceCalendar * );

  private:
    void init();

    bool mOpen;

    KRES::Manager<ResourceCalendar>* mManager;
    QMap <Incidence*, ResourceCalendar*> mResourceMap;

    DestinationPolicy *mDestinationPolicy;
    StandardDestinationPolicy *mStandardPolicy;
    AskDestinationPolicy *mAskPolicy;

    QMap<ResourceCalendar *, Ticket *> mTickets;
    QMap<ResourceCalendar *, int> mChangeCounts;

    class Private;
    Private *d;
};

}

#endif
