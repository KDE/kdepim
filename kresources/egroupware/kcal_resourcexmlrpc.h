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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KCAL_RESOURCEXMLRPC_H
#define KCAL_RESOURCEXMLRPC_H

#include <tqdatetime.h>
#include <tqptrlist.h>
#include <tqstring.h>

#include <kconfig.h>
#include <kurl.h>

#include <kdepimmacros.h>

#include "libkcal/calendarlocal.h"
#include "libkcal/incidence.h"
#include "libkcal/resourcecached.h"
#include "todostatemapper.h"

namespace KXMLRPC {
class Server;
}

class Synchronizer;
class QTimer;

namespace KCal {

class EGroupwarePrefs;

/**
  This class provides access to php/eGroupware calendar via XML-RPC.
*/
class KDE_EXPORT ResourceXMLRPC : public ResourceCached
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
    bool deleteEvent( Event* );

    /**
      Retrieves an event on the basis of the unique string ID.
     */
    Event *event( const TQString& uid );

    /**
      Return unfiltered list of all events in calendar.
     */
    Event::List rawEvents();

    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
     */
    Event::List rawEventsForDate(
      const TQDate& date,
      EventSortField sortField=EventSortUnsorted,
      SortDirection sortDirection=SortDirectionAscending );

    /**
      Get unfiltered events for date \a qdt.
     */
    Event::List rawEventsForDate( const TQDateTime& qdt );

    /**
      Get unfiltered events in a range of dates. If inclusive is set to true,
      only events are returned, which are completely included in the range.
     */
    Event::List rawEvents( const TQDate& start, const TQDate& end,
                           bool inclusive = false );


    /**
      Add a todo to the todolist.
     */
    bool addTodo( Todo* todo );

    /**
      Remove a todo from the todolist.
     */
    bool deleteTodo( Todo* todo );

    /**
      Searches todolist for an event with this unique string identifier,
      returns a pointer or null.
     */
    Todo *todo( const TQString& uid );

    /**
      Return list of all todos.
     */
    Todo::List rawTodos();

    /**
      Returns list of todos due on the specified date.
     */
    Todo::List rawTodosForDate( const TQDate& date );

    /**
      Add a Journal entry to calendar
     */
    virtual bool addJournal( Journal* journal );

    /**
      Remove journal from the calendar.
     */
    bool deleteJournal( Journal* journal );

    /**
      Return Journals for given date
     */
    virtual Journal::List journals( const TQDate& );

    /**
      Return Journal with given UID
     */
    virtual Journal *journal( const TQString& uid );

    /**
      Return all alarms, which ocur in the given time interval.
     */
    Alarm::List alarms( const TQDateTime& from, const TQDateTime& to );

    /**
      Return all alarms, which ocur before given date.
     */
    Alarm::List alarmsTo( const TQDateTime& to );

    /**
      Public because needed in MultiCalendar::load()
     */
    bool doOpen();
    void doClose();

    void dump() const;

    void setTimeZoneId( const TQString& ) {}

  protected slots:
    void loginFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void logoutFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void listEventsFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void addEventFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void updateEventFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void deleteEventFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void loadEventCategoriesFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void listTodosFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void addTodoFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void updateTodoFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void deleteTodoFinished( const TQValueList<TQVariant>&, const TQVariant& );
    void loadTodoCategoriesFinished( const TQValueList<TQVariant>&, const TQVariant& );

    void fault( int, const TQString&, const TQVariant& );

  protected:
    bool doLoad();
    bool doSave();

  private slots:
    void reload();

  private:
    void init();
    void initEGroupware();

    void writeEvent( Event*, TQMap<TQString, TQVariant>& );
    void readEvent( const TQMap<TQString, TQVariant>&, Event*, TQString& );

    void writeTodo( Todo*, TQMap<TQString, TQVariant>& );
    void readTodo( const TQMap<TQString, TQVariant>&, Todo*, TQString& );

    void checkLoadingFinished();

    KXMLRPC::Server *mServer;

    EGroupwarePrefs *mPrefs;

    TQString mSessionID;
    TQString mKp3;

    TQMap<TQString, int> mEventCategoryMap;
    TQMap<TQString, int> mTodoCategoryMap;

    TodoStateMapper mTodoStateMapper;

    Synchronizer *mSynchronizer;

    KABC::Lock *mLock;
    int mLoaded;
};

}

#endif
