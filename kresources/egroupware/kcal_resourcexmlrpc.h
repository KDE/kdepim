 /*
    This file is part of libkcal.

    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

#ifndef KCAL_RESOURCEXMLRPC_H
#define KCAL_RESOURCEXMLRPC_H

#include <qdatetime.h>
#include <qptrlist.h>
#include <qstring.h>

#include <kconfig.h>

#include "libkcal/calendarlocal.h"
#include "libkcal/incidence.h"
#include "libkcal/resourcecalendar.h"

namespace KXMLRPC {
class Server;
}

class QTimer;

namespace KCal {

/**
  This class provides access to php/eGroupware calendar via XML-RPC.
*/
class ResourceXMLRPC : public ResourceCalendar
{
  Q_OBJECT

  public:
    ResourceXMLRPC( const KConfig* );
    ResourceXMLRPC();
    virtual ~ResourceXMLRPC();

    void readConfig( const KConfig* config );
    void writeConfig( KConfig* config );

    void setURL( const KURL& url );
    KURL url() const;

    void setDomain( const QString& domain );
    QString domain() const;

    void setUser( const QString& user );
    QString user() const;

    void setPassword( const QString& password );
    QString password() const;

    void setStartDay( int );
    int startDay() const;

    void setEndDay( int );
    int endDay() const;

    bool load();

    bool save();

    bool isSaving();

    KABC::Lock *lock();

    /**
      Add Event to calendar.
     */
    bool addEvent( Event* event );

    /**
      Deletes an event from this calendar.
     */
    void deleteEvent( Event* );

    /**
      Retrieves an event on the basis of the unique string ID.
     */
    Event *event( const QString& uid );

    /**
      Return unfiltered list of all events in calendar.
     */
    Event::List rawEvents();

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
     */
    Event::List rawEventsForDate( const QDate& date, bool sorted = false );

    /**
      Get unfiltered events for date \a qdt.
     */
    Event::List rawEventsForDate( const QDateTime& qdt );

    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
     */
    Event::List rawEvents( const QDate& start, const QDate& end,
                           bool inclusive = false );


    /**
      Add a todo to the todolist.
     */
    bool addTodo( Todo* todo );

    /**
      Remove a todo from the todolist.
     */
    void deleteTodo( Todo* todo );

    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
     */
    Todo *todo( const QString& uid );

    /**
      Return list of all todos.
     */
    Todo::List rawTodos();

    /**
      Returns list of todos due on the specified date.
     */
    Todo::List todos( const QDate& date );

    /**
      Add a Journal entry to calendar
     */
    virtual bool addJournal( Journal* journal );

    /**
      Remove journal from the calendar.
     */
    void deleteJournal( Journal* journal );

    /**
      Return Journal for given date
     */
    virtual Journal *journal( const QDate& );

    /**
      Return Journal with given UID
     */
    virtual Journal *journal( const QString& uid );

    /**
      Return list of all Journals stored in calendar
     */
    Journal::List journals();

    /**
      Return all alarms, which ocur in the given time interval.
     */
    Alarm::List alarms( const QDateTime& from, const QDateTime& to );

    /**
      Return all alarms, which ocur before given date.
     */
    Alarm::List alarmsTo( const QDateTime& to );

    /**
      Public because needed in MultiCalendar::load()
     */
    bool doOpen();

    void dump() const;

    void setTimeZoneId( const QString& ) {}

  protected slots:
    void loginFinished( const QValueList<QVariant>&, const QVariant& );
    void logoutFinished( const QValueList<QVariant>&, const QVariant& );

    void listEntriesFinished( const QValueList<QVariant>&, const QVariant& );
    void rawDatesFinished( const QValueList<QVariant>&, const QVariant& );
    void addEntryFinished( const QValueList<QVariant>&, const QVariant& );
    void updateEntryFinished( const QValueList<QVariant>&, const QVariant& );
    void deleteEntryFinished( const QValueList<QVariant>&, const QVariant& );

    void fault( int, const QString&, const QVariant& );

  protected:

    void doClose();
    virtual void update( IncidenceBase* incidence );

  private slots:
    void reload();
    void processQueue();

  private:
    void init();
    void writeEvent( Event*, QMap<QString, QVariant>& );
    void readEvent( const QMap<QString, QVariant>&, Event*, QString& );
    void enter_loop();
    void exit_loop();

    void addToQueue( const QDate &date );
    void addToQueue( const QDate &start, const QDate &end );

    CalendarLocal mCalendar;
    KXMLRPC::Server *mServer;

    bool mOpen;

    KURL mURL;
    QString mDomain;
    QString mUser;
    QString mPassword;
    int mStartDay;
    int mEndDay;

    QString mSessionID;
    QString mKp3;
    QString mLastAddUid;
    QMap<QString, QString> mUidMap;
    QMap<QString, int> mCategoryMap;
    QMap<QString, int> mRightsMap;
    QTimer *mQueueTimer;
    QValueList<QDate> mDateQueue;
    QValueList< QPair<QDate, QDate> > mDateRangeQueue;

    bool mSyncComm;

    KABC::Lock *mLock;
};

}

#endif
