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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// TODO [FIXME] IMPORTANT
// If a cached resource initiates a reload while an event editor is active, or an event is locked for editing,
// a big fat crash will ensue.  The reload subroutine must ABORT if ANY korganizer events are locked for editing!!!

#include <tqdatastream.h>
#include <tqdatetime.h>
#include <tqfile.h>
#include <tqstring.h>
#include <tqptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>

#include "event.h"
#include "exceptions.h"
#include "incidence.h"
#include "journal.h"
#include "todo.h"
#include <unistd.h>


#include "resourcecached.h"

using namespace KCal;

static bool m_editoropen = false;

ResourceCached::ResourceCached( const KConfig* config )
  : ResourceCalendar( config ), mCalendar( TQString::fromLatin1( "UTC" ) ),
    mReloadPolicy( ReloadNever ),  mReloadInterval( 10 ), 
    mReloadTimer( 0, "mReloadTimer" ), mReloaded( false ),
    mSavePolicy( SaveNever ), mSaveInterval( 10 ),
    mSaveTimer( 0, "mSaveTimer" ), mIdMapper( "kcal/uidmaps/" )
{
  connect( &mReloadTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( slotReload() ) );
  connect( &mSaveTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( slotSave() ) );
}

ResourceCached::~ResourceCached()
{
}

void ResourceCached::setReloadPolicy( int i )
{
  mReloadPolicy = i;

  setupReloadTimer();
}

int ResourceCached::reloadPolicy() const
{
  return mReloadPolicy;
}

void ResourceCached::setReloadInterval( int minutes )
{
  mReloadInterval = minutes;
}

int ResourceCached::reloadInterval() const
{
  return mReloadInterval;
}

void ResourceCached::setSavePolicy( int i )
{
  mSavePolicy = i;

  setupSaveTimer();
}

int ResourceCached::savePolicy() const
{
  return mSavePolicy;
}

void ResourceCached::setSaveInterval( int minutes )
{
  mSaveInterval = minutes;
}

int ResourceCached::saveInterval() const
{
  return mSaveInterval;
}

void ResourceCached::readConfig( const KConfig *config )
{
  mReloadPolicy = config->readNumEntry( "ReloadPolicy", ReloadNever );
  mReloadInterval = config->readNumEntry( "ReloadInterval", 10 );

  mSaveInterval = config->readNumEntry( "SaveInterval", 10 );
  mSavePolicy = config->readNumEntry( "SavePolicy", SaveNever );

  mLastLoad = config->readDateTimeEntry( "LastLoad" );
  mLastSave = config->readDateTimeEntry( "LastSave" );

  setupSaveTimer();
  setupReloadTimer();
}

void ResourceCached::setupSaveTimer()
{
  if ( mSavePolicy == SaveInterval ) {
    kdDebug(5800) << "ResourceCached::setSavePolicy(): start save timer (interval "
              << mSaveInterval << " minutes)." << endl;
    mSaveTimer.start( mSaveInterval * 60 * 1000 ); // n minutes
  } else {
    mSaveTimer.stop();
  }
}

void ResourceCached::setupReloadTimer()
{
  if ( mReloadPolicy == ReloadInterval ) {
    kdDebug(5800) << "ResourceCached::setSavePolicy(): start reload timer "
                 "(interval " << mReloadInterval << " minutes)" << endl;
    mReloadTimer.start( mReloadInterval * 60 * 1000 ); // n minutes
  } else {
    mReloadTimer.stop();
  }
}

void ResourceCached::writeConfig( KConfig *config )
{
  config->writeEntry( "ReloadPolicy", mReloadPolicy );
  config->writeEntry( "ReloadInterval", mReloadInterval );

  config->writeEntry( "SavePolicy", mSavePolicy );
  config->writeEntry( "SaveInterval", mSaveInterval );

  config->writeEntry( "LastLoad", mLastLoad );
  config->writeEntry( "LastSave", mLastSave );
}

bool ResourceCached::addEvent(Event *event)
{
  return mCalendar.addEvent( event );
}

// probably not really efficient, but...it works for now.
bool ResourceCached::deleteEvent( Event *event )
{
  kdDebug(5800) << "ResourceCached::deleteEvent" << endl;

  return mCalendar.deleteEvent( event );
}


Event *ResourceCached::event( const TQString &uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceCached::rawEventsForDate( const TQDate &qd,
                                              EventSortField sortField,
                                              SortDirection sortDirection )
{
  Event::List list = mCalendar.rawEventsForDate( qd, sortField, sortDirection );

  return list;
}

Event::List ResourceCached::rawEvents( const TQDate &start, const TQDate &end,
                                       bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceCached::rawEventsForDate( const TQDateTime &qdt )
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceCached::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection );
}

bool ResourceCached::addTodo( Todo *todo )
{
  return mCalendar.addTodo( todo );
}

bool ResourceCached::deleteTodo( Todo *todo )
{
  return mCalendar.deleteTodo( todo );
}

bool ResourceCached::deleteJournal( Journal *journal )
{
  return mCalendar.deleteJournal( journal );
}


Todo::List ResourceCached::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
}

Todo *ResourceCached::todo( const TQString &uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceCached::rawTodosForDate( const TQDate &date )
{
  return mCalendar.rawTodosForDate( date );
}


bool ResourceCached::addJournal( Journal *journal )
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceCached::journal( const TQString &uid )
{
  return mCalendar.journal( uid );
}

Journal::List ResourceCached::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

Journal::List ResourceCached::rawJournalsForDate( const TQDate &date )
{
  return mCalendar.rawJournalsForDate( date );
}


Alarm::List ResourceCached::alarmsTo( const TQDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceCached::alarms( const TQDateTime &from, const TQDateTime &to )
{
  // kdDebug(5800) << "ResourceCached::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  return mCalendar.alarms( from, to );
}


void ResourceCached::setTimeZoneId( const TQString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

TQString ResourceCached::timeZoneId() const
{
  return mCalendar.timeZoneId();
}

void ResourceCached::clearChanges()
{
  mAddedIncidences.clear();
  mChangedIncidences.clear();
  mDeletedIncidences.clear();
}

void ResourceCached::loadCache()
{
  setIdMapperIdentifier();
  mIdMapper.load();

  if ( KStandardDirs::exists( cacheFile() ) ) {
    mCalendar.load( cacheFile() );
    if ( readOnly() ) {
      Incidence::List incidences( rawIncidences() );
      Incidence::List::Iterator it;
      for ( it = incidences.begin(); it != incidences.end(); ++it ) {
        (*it)->setReadOnly( true );
      }
    }
  }
}

void ResourceCached::saveCache()
{
  kdDebug(5800) << "ResourceCached::saveCache(): " << cacheFile() << endl;

  setIdMapperIdentifier();
  mIdMapper.save();

  mCalendar.save( cacheFile() );
}

void ResourceCached::setIdMapperIdentifier()
{
  mIdMapper.setIdentifier( type() + "_" + identifier() );
}

void ResourceCached::clearCache()
{
  mCalendar.close();
}

void ResourceCached::clearEventsCache()
{
  mCalendar.closeEvents();
}

void ResourceCached::clearTodosCache()
{
  mCalendar.closeTodos();
}

void ResourceCached::clearJournalsCache()
{
  mCalendar.closeJournals();
}

void ResourceCached::cleanUpEventCache( const Event::List &eventList )
{
  CalendarLocal calendar ( TQString::fromLatin1( "UTC" ) );

  if ( KStandardDirs::exists( cacheFile() ) )
    calendar.load( cacheFile() );
  else
    return;

  Event::List list = calendar.events();
  Event::List::ConstIterator cacheIt, it;
  for ( cacheIt = list.begin(); cacheIt != list.end(); ++cacheIt ) {
    bool found = false;
    for ( it = eventList.begin(); it != eventList.end(); ++it ) {
      if ( (*it)->uid() == (*cacheIt)->uid() )
        found = true;
    }

    if ( !found ) {
      mIdMapper.removeRemoteId( mIdMapper.remoteId( (*cacheIt)->uid() ) );
      Event *event = mCalendar.event( (*cacheIt)->uid() );
      if ( event )
        mCalendar.deleteEvent( event );
    }
  }

  calendar.close();
}

void ResourceCached::cleanUpTodoCache( const Todo::List &todoList )
{
  CalendarLocal calendar ( TQString::fromLatin1( "UTC" ) );

  if ( KStandardDirs::exists( cacheFile() ) )
    calendar.load( cacheFile() );
  else
    return;

  Todo::List list = calendar.todos();
  Todo::List::ConstIterator cacheIt, it;
  for ( cacheIt = list.begin(); cacheIt != list.end(); ++cacheIt ) {

    bool found = false;
    for ( it = todoList.begin(); it != todoList.end(); ++it ) {
      if ( (*it)->uid() == (*cacheIt)->uid() )
        found = true;
    }

    if ( !found ) {
      mIdMapper.removeRemoteId( mIdMapper.remoteId( (*cacheIt)->uid() ) );
      Todo *todo = mCalendar.todo( (*cacheIt)->uid() );
      if ( todo )
        mCalendar.deleteTodo( todo );
    }
  }

  calendar.close();
}

KPIM::IdMapper& ResourceCached::idMapper()
{
  return mIdMapper;
}

TQString ResourceCached::cacheFile() const
{
  return locateLocal( "cache", "kcal/kresources/" + identifier() );
}

TQString ResourceCached::changesCacheFile( const TQString &type ) const
{
  return locateLocal( "cache", "kcal/changescache/" + identifier() + "_" + type );
}

void ResourceCached::saveChangesCache( const TQMap<Incidence*, bool> &map, const TQString &type )
{
  CalendarLocal calendar ( TQString::fromLatin1( "UTC" ) );

  bool isEmpty = true;
  TQMap<Incidence *,bool>::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it ) {
    isEmpty = false;
    calendar.addIncidence( it.key()->clone() );
  }

  if ( !isEmpty ) {
    calendar.save( changesCacheFile( type ) );
  } else {
    TQFile file( changesCacheFile( type ) );
    file.remove();
  }

  calendar.close();
}

void ResourceCached::saveChangesCache()
{
  saveChangesCache( mAddedIncidences, "added" );
  saveChangesCache( mDeletedIncidences, "deleted" );
  saveChangesCache( mChangedIncidences, "changed" );
}

void ResourceCached::loadChangesCache( TQMap<Incidence*, bool> &map, const TQString &type )
{
  CalendarLocal calendar ( TQString::fromLatin1( "UTC" ) );

  if ( KStandardDirs::exists( changesCacheFile( type ) ) )
    calendar.load( changesCacheFile( type ) );
  else
    return;

  const Incidence::List list = calendar.incidences();
  Incidence::List::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    map.insert( (*it)->clone(), true );

  calendar.close();
}

void ResourceCached::loadChangesCache()
{
  loadChangesCache( mAddedIncidences, "added" );
  loadChangesCache( mDeletedIncidences, "deleted" );
  loadChangesCache( mChangedIncidences, "changed" );
}

void ResourceCached::calendarIncidenceAdded( Incidence *i )
{
#if 1
  kdDebug(5800) << "ResourceCached::calendarIncidenceAdded(): "
            << i->uid() << endl;
#endif

  TQMap<Incidence *,bool>::ConstIterator it;
  it = mAddedIncidences.find( i );
  if ( it == mAddedIncidences.end() ) {
    mAddedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

void ResourceCached::calendarIncidenceChanged( Incidence *i )
{
#if 1
  kdDebug(5800) << "ResourceCached::calendarIncidenceChanged(): "
            << i->uid() << endl;
#endif

  TQMap<Incidence *,bool>::ConstIterator it;
  it = mChangedIncidences.find( i );
  // FIXME: If you modify an added incidence, there's no need to add it to mChangedIncidences!
  if ( it == mChangedIncidences.end() ) {
    mChangedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

void ResourceCached::calendarIncidenceDeleted( Incidence *i )
{
#if 1
  kdDebug(5800) << "ResourceCached::calendarIncidenceDeleted(): "
            << i->uid() << endl;
#endif

  if (i->hasRecurrenceID()) {
    // This incidence has a parent; notify the parent of the child's death and do not destroy the parent!
    // Get the parent
    IncidenceList il = i->childIncidences();
    IncidenceListIterator it;
    it = il.begin();
    Incidence *parentIncidence;
    parentIncidence = this->incidence(*it);
    // Remove the child
    calendarIncidenceChanged(parentIncidence);
  }
  else {
    TQMap<Incidence *,bool>::ConstIterator it;
    it = mDeletedIncidences.find( i );
    if ( it == mDeletedIncidences.end() ) {
      mDeletedIncidences.insert( i, true );
    }
  }
  checkForAutomaticSave();
}

Incidence::List ResourceCached::addedIncidences() const
{
  Incidence::List added;
  TQMap<Incidence *,bool>::ConstIterator it;
  for( it = mAddedIncidences.begin(); it != mAddedIncidences.end(); ++it ) {
    added.append( it.key() );
  }
  return added;
}

Incidence::List ResourceCached::changedIncidences() const
{
  Incidence::List changed;
  TQMap<Incidence *,bool>::ConstIterator it;
  for( it = mChangedIncidences.begin(); it != mChangedIncidences.end(); ++it ) {
    changed.append( it.key() );
  }
  return changed;
}

Incidence::List ResourceCached::deletedIncidences() const
{
  Incidence::List deleted;
  TQMap<Incidence *,bool>::ConstIterator it;
  for( it = mDeletedIncidences.begin(); it != mDeletedIncidences.end(); ++it ) {
    deleted.append( it.key() );
  }
  return deleted;
}

Incidence::List ResourceCached::allChanges() const
{
  Incidence::List changes;
  TQMap<Incidence *,bool>::ConstIterator it;
  for( it = mAddedIncidences.begin(); it != mAddedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  for( it = mChangedIncidences.begin(); it != mChangedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  for( it = mDeletedIncidences.begin(); it != mDeletedIncidences.end(); ++it ) {
    changes.append( it.key() );
  }
  return changes;
}

bool ResourceCached::hasChanges() const
{
  return !( mAddedIncidences.isEmpty() && mChangedIncidences.isEmpty() &&
            mDeletedIncidences.isEmpty() );
}

void ResourceCached::clearChange( Incidence *incidence )
{
  clearChange( incidence->uid() );
}

void ResourceCached::clearChange( const TQString &uid )
{
  TQMap<Incidence*, bool>::Iterator it;

  for ( it = mAddedIncidences.begin(); it != mAddedIncidences.end(); ++it )
    if ( it.key()->uid() == uid ) {
      mAddedIncidences.remove( it );
      break;
    }

  for ( it = mChangedIncidences.begin(); it != mChangedIncidences.end(); ++it )
    if ( it.key()->uid() == uid ) {
      mChangedIncidences.remove( it );
      break;
    }

  for ( it = mDeletedIncidences.begin(); it != mDeletedIncidences.end(); ++it )
    if ( it.key()->uid() == uid ) {
      mDeletedIncidences.remove( it );
      break;
    }
}

void ResourceCached::enableChangeNotification()
{
  mCalendar.registerObserver( this );
}

void ResourceCached::disableChangeNotification()
{
  mCalendar.unregisterObserver( this );
}

bool ResourceCached::editorWindowOpen()
{
  return m_editoropen;
}

void ResourceCached::setEditorWindowOpen(bool open)
{
  m_editoropen = open;
}

void ResourceCached::slotReload()
{
  if ( !isActive() ) return;

  // Make sure no editor windows are open
  if (editorWindowOpen() == true) return;

  kdDebug(5800) << "ResourceCached::slotReload()" << endl;

  load();
}

void ResourceCached::slotSave()
{
  if ( !isActive() ) return;

  kdDebug(5800) << "ResourceCached::slotSave()" << endl;

  save();
}

void ResourceCached::checkForAutomaticSave()
{
  if ( mSavePolicy == SaveAlways )  {
    kdDebug(5800) << "ResourceCached::checkForAutomaticSave(): save now" << endl;
    mSaveTimer.start( 1 * 1000, true ); // 1 second
  } else if ( mSavePolicy == SaveDelayed ) {
    kdDebug(5800) << "ResourceCached::checkForAutomaticSave(): save delayed"
              << endl;
    mSaveTimer.start( 15 * 1000, true ); // 15 seconds
  }
}

bool ResourceCached::checkForReload()
{
  if ( mReloadPolicy == ReloadNever ) return false;
  if ( mReloadPolicy == ReloadOnStartup ) return !mReloaded;
  return true;
}

bool ResourceCached::checkForSave()
{
  if ( mSavePolicy == SaveNever ) return false;
  return true;
}

void ResourceCached::addInfoText( TQString &txt ) const
{
  if ( mLastLoad.isValid() ) {
    txt += "<br>";
    txt += i18n("Last loaded: %1")
           .arg( KGlobal::locale()->formatDateTime( mLastLoad ) );
  }
  if ( mLastSave.isValid() ) {
    txt += "<br>";
    txt += i18n("Last saved: %1")
           .arg( KGlobal::locale()->formatDateTime( mLastSave ) );
  }
}

void ResourceCached::doClose()
{
  mCalendar.close();
}

bool ResourceCached::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << endl;
  return true;
}

void KCal::ResourceCached::setOwner( const Person &owner )
{
  mCalendar.setOwner( owner );
}

const Person & KCal::ResourceCached::getOwner() const
{
  return mCalendar.getOwner();
}

#include "resourcecached.moc"
