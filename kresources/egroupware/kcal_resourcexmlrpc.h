 /*
    This file is part of kdepim.

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
#include <kurl.h>

#include "libkcal/calendarlocal.h"
#include "libkcal/incidence.h"
#include "libkcal/resourcecached.h"

namespace KXMLRPC {
class Server;
}

class UIDMapper;
class QTimer;

namespace KCal {

class EGroupwarePrefs;

/**
  This class provides access to php/eGroupware calendar via XML-RPC.
*/
class ResourceXMLRPC : public ResourceCached
{
  Q_OBJECT

  public:
    ResourceXMLRPC( const KConfig* );
    ResourceXMLRPC();
    virtual ~ResourceXMLRPC();

    void readConfig( const KConfig* config );
    void writeConfig( KConfig* config );

    EGroupwarePrefs *prefs() const { return mPrefs; }

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
    Todo::List rawTodosForDate( const QDate& date );

    /**
      Add a Journal entry to calendar
     */
    virtual bool addJournal( Journal* journal );

    /**
      Remove journal from the calendar.
     */
    void deleteJournal( Journal* journal );

    /**
      Return Journals for given date
     */
    virtual Journal::List journals( const QDate& );

    /**
      Return Journal with given UID
     */
    virtual Journal *journal( const QString& uid );

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
    void doClose();

    void dump() const;

    void setTimeZoneId( const QString& ) {}

  protected slots:
    void loginFinished( const QValueList<QVariant>&, const QVariant& );
    void logoutFinished( const QValueList<QVariant>&, const QVariant& );

    void listEventsFinished( const QValueList<QVariant>&, const QVariant& );
    void addEventFinished( const QValueList<QVariant>&, const QVariant& );
    void updateEventFinished( const QValueList<QVariant>&, const QVariant& );
    void deleteEventFinished( const QValueList<QVariant>&, const QVariant& );
    void loadEventCategoriesFinished( const QValueList<QVariant>&, const QVariant& );

    void listTodosFinished( const QValueList<QVariant>&, const QVariant& );
    void addTodoFinished( const QValueList<QVariant>&, const QVariant& );
    void updateTodoFinished( const QValueList<QVariant>&, const QVariant& );
    void deleteTodoFinished( const QValueList<QVariant>&, const QVariant& );
    void loadTodoCategoriesFinished( const QValueList<QVariant>&, const QVariant& );

    void fault( int, const QString&, const QVariant& );

  protected:
    bool doLoad();
    bool doSave();

  private slots:
    void reload();

  private:
    void init();
    void initEGroupware();

    void writeEvent( Event*, QMap<QString, QVariant>& );
    void readEvent( const QMap<QString, QVariant>&, Event*, QString& );

    void writeTodo( Todo*, QMap<QString, QVariant>& );
    void readTodo( const QMap<QString, QVariant>&, Todo*, QString& );

    void checkLoadingFinished();

    void enter_loop();
    void exit_loop();

    KXMLRPC::Server *mServer;

    EGroupwarePrefs *mPrefs;

    QString mSessionID;
    QString mKp3;

    QMap<QString, int> mEventCategoryMap;
    QMap<QString, int> mTodoCategoryMap;
    QMap<QString, QString> mTodoStateMap;

    bool mSyncComm;

    KABC::Lock *mLock;
    int mLoaded;
};

}

#endif
