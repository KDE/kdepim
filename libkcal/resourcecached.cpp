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

#include <qdatastream.h>
#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kstandarddirs.h>

#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"

#include "resourcecached.h"

using namespace KCal;

ResourceCached::ResourceCached( const KConfig* config )
  : ResourceCalendar( config ), mReloadPolicy( ReloadNever ),
    mReloadInterval( 10 ), mReloaded( false ), mSavePolicy( SaveNever ),
    mSaveInterval( 10 )
{
  connect( &mReloadTimer, SIGNAL( timeout() ), SLOT( slotReload() ) );
  connect( &mSaveTimer, SIGNAL( timeout() ), SLOT( slotSave() ) );
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
void ResourceCached::deleteEvent( Event *event )
{
  kdDebug(5800) << "ResourceCached::deleteEvent" << endl;

  mCalendar.deleteEvent( event );
}


Event *ResourceCached::event( const QString &uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceCached::rawEventsForDate( const QDate &qd, bool sorted )
{
  Event::List list = mCalendar.rawEventsForDate( qd, sorted );

  return list;
}


Event::List ResourceCached::rawEvents( const QDate &start, const QDate &end,
                                       bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceCached::rawEventsForDate( const QDateTime &qdt )
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceCached::rawEvents()
{
  return mCalendar.rawEvents();
}

bool ResourceCached::addTodo( Todo *todo )
{
  return mCalendar.addTodo( todo );
}

void ResourceCached::deleteTodo( Todo *todo )
{
  mCalendar.deleteTodo( todo );
}

void ResourceCached::deleteJournal( Journal *journal )
{
  mCalendar.deleteJournal( journal );
}


Todo::List ResourceCached::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceCached::todo( const QString &uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceCached::rawTodosForDate( const QDate &date )
{
  return mCalendar.rawTodosForDate( date );
}


bool ResourceCached::addJournal( Journal *journal )
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  return mCalendar.addJournal( journal );
}

Journal *ResourceCached::journal( const QDate &date )
{
//  kdDebug(5800) << "ResourceCached::journal() " << date.toString() << endl;

  return mCalendar.journal( date );
}

Journal *ResourceCached::journal( const QString &uid )
{
  return mCalendar.journal( uid );
}

Journal::List ResourceCached::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceCached::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceCached::alarms( const QDateTime &from, const QDateTime &to )
{
//  kdDebug(5800) << "ResourceCached::alarms(" << from.toString() << " - " << to.toString() << ")\n";

  return mCalendar.alarms( from, to );
}


void ResourceCached::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

QString ResourceCached::timeZoneId() const
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
  if ( !mIdMapper.load( uidMapFile() ) )
    kdError(5800) << "Can't read uid map file '" << uidMapFile() << "'" << endl;

  if ( KStandardDirs::exists( cacheFile() ) )
    mCalendar.load( cacheFile() );
}

void ResourceCached::saveCache()
{
  kdDebug(5800) << "ResourceCached::saveCache(): " << cacheFile() << endl;

  if ( !mIdMapper.save( uidMapFile() ) )
    kdError(5800) << "Can't write uid map file '" << uidMapFile() << "'" << endl;

  mCalendar.save( cacheFile() );
}

void ResourceCached::clearCache()
{
  mCalendar.close();
}

void ResourceCached::cleanUpEventCache( const Event::List &eventList )
{
  CalendarLocal calendar;

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
  CalendarLocal calendar;

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

QString ResourceCached::cacheFile() const
{
  return locateLocal( "cache", "kcal/kresources/" + identifier() );
}

QString ResourceCached::uidMapFile() const
{
  return locateLocal( "data", "kcal/uidmaps/" + type() + "/" + identifier() );
}

void ResourceCached::calendarIncidenceAdded( Incidence *i )
{
#if 1
  kdDebug(5800) << "ResourceCached::calendarIncidenceAdded(): "
            << i->uid() << endl;
#endif

  QMap<Incidence *,bool>::ConstIterator it;
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

  QMap<Incidence *,bool>::ConstIterator it;
  it = mChangedIncidences.find( i );
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

  QMap<Incidence *,bool>::ConstIterator it;
  it = mDeletedIncidences.find( i );
  if ( it == mDeletedIncidences.end() ) {
    mDeletedIncidences.insert( i, true );
  }

  checkForAutomaticSave();
}

Incidence::List ResourceCached::addedIncidences() const
{
  Incidence::List added;
  QMap<Incidence *,bool>::ConstIterator it;
  for( it = mAddedIncidences.begin(); it != mAddedIncidences.end(); ++it ) {
    added.append( it.key() );
  }
  return added;
}

Incidence::List ResourceCached::changedIncidences() const
{
  Incidence::List changed;
  QMap<Incidence *,bool>::ConstIterator it;
  for( it = mChangedIncidences.begin(); it != mChangedIncidences.end(); ++it ) {
    changed.append( it.key() );
  }
  return changed;
}

Incidence::List ResourceCached::deletedIncidences() const
{
  Incidence::List deleted;
  QMap<Incidence *,bool>::ConstIterator it;
  for( it = mDeletedIncidences.begin(); it != mDeletedIncidences.end(); ++it ) {
    deleted.append( it.key() );
  }
  return deleted;
}

Incidence::List ResourceCached::allChanges() const
{
  Incidence::List changes;
  QMap<Incidence *,bool>::ConstIterator it;
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
  mAddedIncidences.remove( incidence );
  mChangedIncidences.remove( incidence );
  mDeletedIncidences.remove( incidence );
}

void ResourceCached::enableChangeNotification()
{
  mCalendar.registerObserver( this );
}

void ResourceCached::disableChangeNotification()
{
  mCalendar.unregisterObserver( this );
}

void ResourceCached::slotReload()
{
  kdDebug(5800) << "ResourceCached::slotReload()" << endl;

  load();
}

void ResourceCached::slotSave()
{
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

void ResourceCached::addInfoText( QString &txt ) const
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


#include "resourcecached.moc"
