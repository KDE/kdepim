 /*
    This file is part of libkcal.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KCAL_RESOURCECACHED_H
#define KCAL_RESOURCECACHED_H

#include "resourcecalendar.h"

#include "libemailfunctions/idmapper.h"
#include "incidence.h"
#include "calendarlocal.h"

#include <kconfig.h>

#include <qptrlist.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qtimer.h>

namespace KCal {

/**
  This class provides a calendar resource using a local CalendarLocal object to
  cache the calendar data.
*/
class ResourceCached : public ResourceCalendar,
                       public KCal::Calendar::Observer
{
    Q_OBJECT
  public:
    /**
      Reload policy.
      
      @see setReloadPolicy(), reloadPolicy()
    */
    enum { ReloadNever, ReloadOnStartup, ReloadInterval };
    /**
      Save policy.
      
      @see setSavePolicy(), savePolicy()
    */
    enum { SaveNever, SaveOnExit, SaveInterval, SaveDelayed, SaveAlways };
  
    ResourceCached( const KConfig * );
    virtual ~ResourceCached();

    void readConfig( const KConfig *config );
    void writeConfig( KConfig *config );

    /**
      Set reload policy. This controls when the cache is refreshed.

      ReloadNever     never reload
      ReloadOnStartup reload when resource is started
      ReloadInterval  reload regularly after given interval
    */
    void setReloadPolicy( int policy );
    /**
      Return reload policy.
      
      @see setReloadPolicy()
    */
    int reloadPolicy() const;

    /**
      Set reload interval in minutes which is used when reload policy is
      ReloadInterval.
    */
    void setReloadInterval( int minutes );

    /**
      Return reload interval in minutes.
    */
    int reloadInterval() const;

    /**
      Set save policy. This controls when the cache is refreshed.

      SaveNever     never save
      SaveOnExit    save when resource is exited
      SaveInterval  save regularly after given interval
      SaveDelayed   save after small delay
      SaveAlways    save on every change
    */
    void setSavePolicy( int policy );
    /**
      Return save policy.
      
      @see setsavePolicy()
    */
    int savePolicy() const;

    /**
      Set save interval in minutes which is used when save policy is
      SaveInterval.
    */
    void setSaveInterval( int minutes );

    /**
      Return save interval in minutes.
    */
    int saveInterval() const;

    /**
      Set time of last load.
    */
    void setLastLoad( const QDateTime & );
    /**
      Return time of last load.
    */
    QDateTime lastLoad() const;

    /**
      Set time of last save.
    */
    void setLastSave( const QDateTime & );
    /**
      Return time of last save.
    */
    QDateTime lastSave() const;

    /**
      Add event to calendar.
    */
    bool addEvent(Event *anEvent);
    /**
      Deletes an event from this calendar.
    */
    void deleteEvent(Event *);

    /**
      Retrieves an event on the basis of the unique string ID.
    */
    Event *event(const QString &UniqueStr);
    /**
      Return filtered list of all events in calendar.
    */
    Event::List events();
    /**
      Return unfiltered list of all events in calendar.
    */
    Event::List rawEvents();
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
    Todo::List rawTodos();
    /**
      Returns list of todos due on the specified date.
    */
    Todo::List rawTodosForDate( const QDate &date );
    /**
      Add a Journal entry to calendar
    */
    virtual bool addJournal( Journal * );
    /**
      Remove a Journal from the calendar
    */
    virtual void deleteJournal( Journal * );
    /**
      Return Journal for given date.
    */
    virtual Journal *journal( const QDate & );
    /**
      Return Journal with given unique id.
    */
    virtual Journal *journal( const QString &uid );
    /**
      Return list of all Journals stored in calendar
    */
    Journal::List journals();

    /**
      Return all alarms, which ocur in the given time interval.
    */
    Alarm::List alarms( const QDateTime &from, const QDateTime &to );

    /**
      Return all alarms, which ocur before given date.
    */
    Alarm::List alarmsTo( const QDateTime &to );

    /**
      Set id of timezone, e.g. "Europe/Berlin"
    */
    void setTimeZoneId( const QString& tzid );
  
    QString timeZoneId() const;

    void enableChangeNotification();
    void disableChangeNotification();

    void clearChange( Incidence * );

    void clearChanges();

    bool hasChanges() const;
  
    Incidence::List allChanges() const;

    Incidence::List addedIncidences() const;
    Incidence::List changedIncidences() const;
    Incidence::List deletedIncidences() const;

    /**
      Loads the cache, this method should be called on load.
     */
    void loadCache();

    /**
      Saves the cache back.
     */
    void saveCache();

    /**
      Clear cache.
    */
    void clearCache();

    void cleanUpEventCache( const KCal::Event::List &eventList );
    void cleanUpTodoCache( const KCal::Todo::List &todoList );

    /**
      Returns a reference to the id mapper.
     */
    KPIM::IdMapper& idMapper();

  protected:
    // From Calendar::Observer
    void calendarIncidenceAdded( KCal::Incidence * );
    void calendarIncidenceChanged( KCal::Incidence * );
    void calendarIncidenceDeleted( KCal::Incidence * );

    CalendarLocal mCalendar;

    /**
      Virtual method from KRES::Resource, called when the last instace of the 
      resource is closed 
     */
    virtual void doClose();
    /** 
      Opens the resource. Dummy implementation, so child classes don't have to 
      reimplement this method. By default, this does not do anything, but can be reimplemented in child classes 
     */
    virtual bool doOpen();
    /**
      Check if reload required according to reload policy.
    */    
    bool checkForReload();
    /**
      Check if save required according to save policy.
    */    
    bool checkForSave();

    void checkForAutomaticSave();

    void addInfoText( QString & ) const;

    void setupSaveTimer();
    void setupReloadTimer();

    /**
      This method is used by loadCache() and saveCache(), reimplement
      it to change the location of the cache.
     */
    virtual QString cacheFile() const;

  protected slots:
    void slotReload();
    void slotSave();

    void setIdMapperIdentifier();

  private:
    int mReloadPolicy;
    int mReloadInterval;
    QTimer mReloadTimer;
    bool mReloaded;

    int mSavePolicy;
    int mSaveInterval;
    QTimer mSaveTimer;

    QDateTime mLastLoad;
    QDateTime mLastSave;

    QMap<KCal::Incidence *,bool> mAddedIncidences;
    QMap<KCal::Incidence *,bool> mChangedIncidences;
    QMap<KCal::Incidence *,bool> mDeletedIncidences;

    KPIM::IdMapper mIdMapper;

    class Private;
    Private *d;
};

}

#endif
