 /*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCEIMAP_H
#define KCAL_RESOURCEIMAP_H

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>

#include <dcopobject.h>
#include <kconfig.h>

#include <libkcal/incidence.h>
#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include "resourcecalendar.h"

class DCOPClient;

namespace KCal {

/**
  This class provides a calendar stored on an IMAP-server via kmail
*/
  class ResourceIMAP : public ResourceCalendar, public IncidenceBase::Observer, virtual public DCOPObject
{
  K_DCOP

  k_dcop:
    virtual bool addIncidence( const QString& type, const QString& ical );
    virtual void deleteIncidence( const QString& type, const QString& uid );
    virtual void slotRefresh( const QString& type );

  public:
    ResourceIMAP( const KConfig * );
    virtual ~ResourceIMAP();

    virtual void writeConfig( KConfig* config );

    bool load();

    bool save();

    /** Add Event to calendar. */
    bool addEvent(Event *anEvent);
    /** deletes an event from this calendar. */
    void deleteEvent(Event *);

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event(const QString &UniqueStr);
    /**
      Return filtered list of all events in calendar.
    */
//    QPtrList<Event> events();
    /**
      Return unfiltered list of all events in calendar.
    */
    QPtrList<Event> rawEvents();
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    QPtrList<Event> rawEventsForDate( const QDate &date, bool sorted = false );
    /**
      Get unfiltered events for date \a qdt.
    */
    QPtrList<Event> rawEventsForDate( const QDateTime &qdt );
    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
    */
    QPtrList<Event> rawEvents( const QDate &start, const QDate &end,
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
    QPtrList<Todo> rawTodos();
    /**
      Returns list of todos due on the specified date.
    */
    QPtrList<Todo> todos( const QDate &date );
    /** Add a Journal entry to calendar */
    virtual bool addJournal(Journal *);
    /**
      Remove a journal entry from the journal.
    */
    void deleteJournal( Journal * );
    /** Return Journal for given date */
    virtual Journal *journal(const QDate &);
    /** Return Journal with given UID */
    virtual Journal *journal(const QString &UID);
    /** Return list of all Journals stored in calendar */
    QPtrList<Journal> journals();

    /** Return all alarms, which ocur in the given time interval. */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /** Return all alarms, which ocur before given date. */
    Alarm::List alarmsTo( const QDateTime &to );

    
    /** this method should be called whenever a Event is modified directly
     * via it's pointer.  It makes sure that the calendar is internally
     * consistent. */
    void update(IncidenceBase *incidence);
 
    friend class ResourceIMAPConfig;

    // Public because needed in MultiCalendar::load()
    bool doOpen();

  protected:

    /** Notification function of IncidenceBase::Observer. */
    virtual void incidenceUpdated( IncidenceBase *i ) { update( i ); }
    /** Append alarms of incidence in interval to list of alarms. */

  private:
    void init();
    QStringList getIncidenceList( const QString& type );

    bool loadAllEvents();
    bool loadAllTasks();

    KCal::Incidence* parseIncidence( const QString& str );

    QString mServer;
    ICalFormat mFormat;
    CalendarLocal mCalendar;
    DCOPClient* mDCOPClient;
    bool mSilent;
    QString mCurrentUID;
};  

}

#endif
