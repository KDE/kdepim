/*
    This file is part of libkcal.

    Copyright (c) 2006 David Jarvie <software@astrojar.org.uk>
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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KCAL_RESOURCECACHED_H
#define KCAL_RESOURCECACHED_H

#include "resourcecalendar.h"

#include <kresources/idmapper.h>
#include "incidence.h"
#include "calendarlocal.h"

#include <kconfig.h>

#include <QString>
#include <QDateTime>
#include <QTimer>

#include <kdepimmacros.h>

namespace KCal {

/**
  This class provides a calendar resource using a local CalendarLocal object to
  cache the calendar data.
*/
class KDE_EXPORT ResourceCached : public ResourceCalendar,
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

    /**
      Whether to update the cache file when loading a resource, or whether to
      upload the cache file after saving the resource.
      Only applicable to genuinely cached resources.
     */
    enum CacheAction {
        DefaultCache,    // use the default action set by setReloadPolicy() or setSavePolicy()
        NoSyncCache,     // perform a cache-only operation, without downloading or uploading
        SyncCache        // update the cache file before loading, or upload cache after saving
    };

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
      Inhibit or allow cache reloads when using load(DefaultCache). If inhibited,
      this overrides the policy set by setReloadPolicy(), preventing any non-explicit
      reloads from being performed. If not inhibited, reloads take place according
      to the policy set by setReloadPolicy().

      @param inhibit  true to inhibit reloads, false to allow them
    */
    bool inhibitDefaultReload( bool inhibit );
    bool defaultReloadInhibited() const   { return mInhibitReload; }

    /**
      Return whether the resource cache has been reloaded since startup.
     */
    bool reloaded() const   { return mReloaded; }

    /**
      Set save policy. This controls when the cache is refreshed.

      SaveNever     never save
      SaveOnExit    save when resource is exited
      SaveInterval  save regularly after given interval
      SaveDelayed   save on every change, after small delay
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
      Load resource data, specifying whether to refresh the cache file first.
      For a non-cached resource, this method has the same effect as load().
     */
    bool load( CacheAction );

    /**
      Load resource data.
     */
    virtual bool load()   { return load( SyncCache ); }

    /**
      Save the resource data to cache, and optionally upload the cache file afterwards.
      For a non-cached resource, this method has the same effect as save().

      @param incidence if given as 0, doSave(bool) is called to save all incidences,
             else doSave(bool, incidence) is called to save only the given one
    */
    bool save( CacheAction, Incidence *incidence = 0 );

    /**
      Save resource data.
     */
    virtual bool save( Incidence *incidence = 0 )   { return save( SyncCache, incidence ); }

    /**
      Add event to calendar.
    */
    bool addEvent(Event *anEvent);
    /**
      Deletes an event from this calendar.
    */
    bool deleteEvent(Event *);

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
    Event::List rawEvents( EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );
    /**
      Builds and then returns a list of all events that match for the
      date specified. useful for dayView, etc. etc.
    */
    Event::List rawEventsForDate( const QDate &date, EventSortField sortField = EventSortUnsorted, SortDirection sortDirection = SortDirectionAscending );

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
    bool deleteTodo( Todo * );
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
    /**
      Add a Journal entry to calendar
    */
    virtual bool addJournal( Journal * );
    /**
      Remove a Journal from the calendar
    */
    virtual bool deleteJournal( Journal * );
    /**
      Return Journal with given unique id.
    */
    virtual Journal *journal( const QString &uid );
    /**
      Return list of all journals.
    */
    Journal::List rawJournals( JournalSortField sortField = JournalSortUnsorted,SortDirection sortDirection = SortDirectionAscending );
    /**
      Return list of journals for the given date.
    */
    Journal::List rawJournalsForDate( const QDate &date );

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
    void setTimeZoneId( const QString &timeZoneId );

    QString timeZoneId() const;

    /**
      Return the owner of the calendar's full name.
    */
    const Person &getOwner() const;
    /**
      Set the owner of the calendar. Should be owner's full name.
    */
    void setOwner( const Person &owner );

    void enableChangeNotification();
    void disableChangeNotification();

    void clearChange( Incidence * );
    void clearChange( const QString &uid );

    void clearChanges();

    bool hasChanges() const;

    Incidence::List allChanges() const;

    Incidence::List addedIncidences() const;
    Incidence::List changedIncidences() const;
    Incidence::List deletedIncidences() const;

    /**
      Load the resource from the cache.
      @return true if the cache file exists, false if not
     */
    bool loadFromCache();

    /**
      Save the resource back to the cache.
     */
    void saveToCache();

    /**
      Clear cache.
    */
    void clearCache();

    void cleanUpEventCache( const KCal::Event::List &eventList );
    void cleanUpTodoCache( const KCal::Todo::List &todoList );

    /**
      Returns a reference to the id mapper.
     */
    KRES::IdMapper& idMapper();

  protected:
    // From Calendar::Observer
    void calendarIncidenceAdded( KCal::Incidence *incidence );
    void calendarIncidenceChanged( KCal::Incidence *incidence );
    void calendarIncidenceDeleted( KCal::Incidence *incidence );

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
      Do the actual loading of the resource data. Called by load(CacheAction).
    */
    virtual bool doLoad( bool syncCache ) = 0;
    /**
      Set the cache-reloaded status.
      Non-local resources must set this true once the cache has been downloaded successfully.
     */
    void setReloaded( bool done )   { mReloaded = done; }
    /**
      Do the actual saving of the resource data. Called by save(CacheAction). Saves
      the resource data to the cache and optionally uploads (if a remote resource).

      @param syncCache if true, the cache will be uploaded to the remote resource. If false,
                       only the cache will be updated.
    */
    virtual bool doSave( bool syncCache ) = 0;
    /**
      Do the actual saving of the resource data. Called by save(CacheAction).
      Save one Incidence. The default implementation calls doSave(bool) to save everything
    */
    virtual bool doSave( bool syncCache, Incidence * );

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
      This method is used by loadFromCache() and saveToCache(), reimplement
      it to change the location of the cache.
     */
    virtual QString cacheFile() const;

    /**
      Functions for keeping the changes persistent.
     */
    virtual QString changesCacheFile( const QString& ) const;
    void loadChangesCache( QMap<Incidence*, bool>&, const QString& );
    void loadChangesCache();
    void saveChangesCache( const QMap<Incidence*, bool>&, const QString& );
    void saveChangesCache();

  protected slots:
    void slotReload();
    void slotSave();

    void setIdMapperIdentifier();

  private:
    // Virtual methods which should not be used by derived classes.
    virtual bool doLoad()   { return doLoad( true ); }
    virtual bool doSave()   { return doSave( true ); }

    int mReloadPolicy;
    int mReloadInterval;
    QTimer mReloadTimer;
    bool mInhibitReload;   // true to prevent downloads by load(DefaultCache)
    bool mReloaded;        // true once it has been downloaded
    bool mSavePending;     // true if a save of changes has been scheduled on the timer

    int mSavePolicy;
    int mSaveInterval;
    QTimer mSaveTimer;

    QDateTime mLastLoad;
    QDateTime mLastSave;

    QMap<KCal::Incidence *,bool> mAddedIncidences;
    QMap<KCal::Incidence *,bool> mChangedIncidences;
    QMap<KCal::Incidence *,bool> mDeletedIncidences;

    KRES::IdMapper mIdMapper;

    class Private;
    Private *d;
};

}

#endif
