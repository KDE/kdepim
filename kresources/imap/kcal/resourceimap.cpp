/*
    This file is part of libkcal.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 - 2004 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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

#include <kabc/locknull.h>
#include <kstandarddirs.h>

#include "resourceimap.h"

using namespace KCal;


ResourceIMAP::ResourceIMAP( const QString &server )
  : ResourceCalendar( 0 ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-libkcal" ),
    mServer( server ),
    mSilent( false )
{
  init();
}

ResourceIMAP::ResourceIMAP( const KConfig* config )
  : ResourceCalendar( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-libkcal" ),
    mSilent( false )
{
  init();

  if ( config ) {
    mServer = config->readEntry( "Servername" );
  }
}

void ResourceIMAP::init()
{
  setType( "imap" );
}

void ResourceIMAP::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "Servername", mServer );
}

ResourceIMAP::~ResourceIMAP()
{
  close();
}

bool ResourceIMAP::doOpen()
{
  // Get the config file
  QString configFile = locateLocal( "config", "kresources/imap/kcalrc" );
  KConfig config( configFile );

  // Read the calendar entries
  QStringList resources;
  if ( !kmailSubresources( resources, "Calendar" ) )
    return false;
  config.setGroup( "Calendar" );
  QStringList::ConstIterator it;
  mEventResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it )
    mEventResources[ *it ] = config.readBoolEntry( *it, true );

  // Read the task entries
  if ( !kmailSubresources( resources, "Task" ) )
    return false;
  config.setGroup( "Task" );
  mTaskResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it )
    mTaskResources[ *it ] = config.readBoolEntry( *it, true );

  // Read the journal entries
  if ( !kmailSubresources( resources, "Journal" ) )
    return false;
  config.setGroup( "Journal" );
  mJournalResources.clear();
  for ( it = resources.begin(); it != resources.end(); ++it )
    mJournalResources[ *it ] = config.readBoolEntry( *it, true );

  return true;
}

void ResourceIMAP::doClose()
{
  // Get the config file
  QString configFile = locateLocal( "config", "kresources/imap/kcalrc" );
  KConfig config( configFile );

  config.setGroup( "Calendar" );
  QMap<QString, bool>::ConstIterator it;
  for ( it = mEventResources.begin(); it != mEventResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );

  config.setGroup( "Task" );
  for ( it = mTaskResources.begin(); it != mTaskResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );

  config.setGroup( "Journal" );
  for ( it = mJournalResources.begin(); it != mJournalResources.end(); ++it )
    config.writeEntry( it.key(), it.data() );
}

bool ResourceIMAP::load()
{
  mUidmap.clear();

  // Load each resource. Note: It's intentional to use & instead of &&
  // so we try all three, even if the first failed
  return loadAllEvents() & loadAllTasks() & loadAllJournals();
}

bool ResourceIMAP::loadAllEvents()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();

  bool silent = mSilent;
  mSilent = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mEventResources.begin(); itR != mEventResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    // Get the list of events
    QStringList lst;
    if ( !kmailIncidences( lst, "Calendar", itR.key() ) ) {
      // The get failed
      mSilent = silent;
      return false;
    }

    // Populate the calendar with the new events
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      Incidence* i = parseIncidence( *it );
      if ( i ) {
        if ( i->type() == "Event" ) {
          addEvent( static_cast<Event*>( i ), itR.key() );
        } else {
          kdDebug(5650) << "Unknown incidence type " << i->type();
          delete i;
        }
      } else
        kdDebug(5650) << "Problem reading: " << *it << endl;
    }
  }
  mSilent = silent;

  emit resourceChanged( this );
  return true;
}

bool ResourceIMAP::loadAllTasks()
{
  // We get a fresh list of todos, so clean out the old ones
  mCalendar.deleteAllTodos();

  bool silent = mSilent;
  mSilent = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mTaskResources.begin(); itR != mTaskResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    // Get the list of todos
    QStringList lst;
    if ( !kmailIncidences( lst, "Task", itR.key() ) )
      // The get failed
      return false;

    // Populate the calendar with the new todos
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      Incidence* i = parseIncidence( *it );
      if ( i ) {
        if ( i->type() == "Todo" ) {
          addTodo( static_cast<Todo*>( i )/*, itR.key()*/ );
        } else {
          kdDebug() << "Unknown incidence type " << i->type();
          delete i;
        }
      }
    }
  }
  mSilent = silent;

  emit resourceChanged( this );
  return true;
}

bool ResourceIMAP::loadAllJournals()
{
  // We get a fresh list of journals, so clean out the old ones
  mCalendar.deleteAllJournals();

  bool silent = mSilent;
  mSilent = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR=mJournalResources.begin(); itR!=mJournalResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    // Get the list of journals
    QStringList lst;
    if ( !kmailIncidences( lst, "Journal", itR.key() ) )
      // The get failed
      return false;

    // Populate the calendar with the new journals
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
      Incidence* i = parseIncidence( *it );
      if ( i ) {
        if ( i->type() == "Journal" ) {
          addJournal( static_cast<Journal*>( i )/*, itR.key()*/ );
        } else {
          kdDebug() << "Unknown incidence type " << i->type();
          delete i;
        }
      }
    }
  }
  mSilent = silent;

  emit resourceChanged( this );
  return true;
}

bool ResourceIMAP::save()
{
  // The KMail folders are always up to date with the state of the folders
  return true;
}

KABC::Lock *ResourceIMAP::lock()
{
  return new KABC::LockNull( true );
}

/***********************************************
 * Adding and removing Events
 */
bool ResourceIMAP::addEvent( Event *anEvent )
{
  return addEvent( anEvent, QString::null );
}

bool ResourceIMAP::addEvent( Event *anEvent, const QString& subresource )
{
  kdDebug(5800) << "ResourceIMAP::addEvent" << endl;
  mCalendar.addEvent(anEvent);
  anEvent->registerObserver( this );

  // Register for the subresource
  QString resource = subresource;
  if ( subresource.isEmpty() )
    // TODO: Do something a bit more clever
    resource = mEventResources.begin().key();
  mUidmap[ anEvent->uid() ] = resource;

  if ( mSilent ) return true;

  mCurrentUID = anEvent->uid();
  QString vCal = mFormat.createScheduleMessage( anEvent, Scheduler::Request );
  bool rc = kmailAddIncidence( "Calendar", resource, mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addEvent()\n";

  return rc;
}

// probably not really efficient, but...it works for now.
void ResourceIMAP::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceIMAP::deleteEvent" << endl;
  Q_ASSERT( mUidmap.contains( mCurrentUID ) );

  // Call kmail ...
  if ( !mSilent ) {
    mCurrentUID = event->uid();
    kmailDeleteIncidence( "Calendar", mUidmap[ mCurrentUID ], mCurrentUID );
    mCurrentUID = QString::null;
  }

  mUidmap.erase( event->uid() );
  mCalendar.deleteEvent(event);
}


/***********************************************
 * Getting Events
 */

Event *ResourceIMAP::event( const QString &uid )
{
  return mCalendar.event(uid);
}

// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
Event::List ResourceIMAP::rawEventsForDate( const QDate &qd, bool sorted )
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


Event::List ResourceIMAP::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceIMAP::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt );
}

Event::List ResourceIMAP::rawEvents()
{
  return mCalendar.rawEvents();
}

/***********************************************
 * Adding and removing Todos
 */

bool ResourceIMAP::addTodo(Todo *todo)
{
  mCalendar.addTodo(todo);
  todo->registerObserver( this );

  if ( mSilent ) return true;

  mCurrentUID = todo->uid();
  QString vCal = mFormat.createScheduleMessage( todo, Scheduler::Request );
  bool rc = kmailAddIncidence( "Task", "FIXME", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addTodo()\n";

  return rc;
}

void ResourceIMAP::deleteTodo(Todo *todo)
{
  // call kmail ...
  if ( !mSilent ) {
    mCurrentUID = todo->uid();
    kmailDeleteIncidence( "Task", "FIXME", mCurrentUID );
    mCurrentUID = QString::null;
  }
  mCalendar.deleteTodo(todo);
}

/***********************************************
 * Getting Todos
 */

Todo::List ResourceIMAP::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceIMAP::todo( const QString &uid )
{
  return mCalendar.todo(uid);
}

Todo::List ResourceIMAP::rawTodosForDate( const QDate &date )
{
  return mCalendar.rawTodosForDate(date);
}

/***********************************************
 * Journal handling
 */

bool ResourceIMAP::addJournal(Journal *journal)
{
  // kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString()
  //               << endl;
  mCalendar.addJournal(journal);
  journal->registerObserver( this );

  if ( mSilent ) return true;

  // call kmail ..
  mCurrentUID = journal->uid();
  QString vCal = mFormat.createScheduleMessage( journal, Scheduler::Request );
  bool rc = kmailAddIncidence( "Journal", "FIXME", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addJournal()\n";

  return rc;
}

void ResourceIMAP::deleteJournal(Journal *journal)
{
  if( !journal )
    return;

  if ( !mSilent ) {
    mCurrentUID = journal->uid();
    kmailDeleteIncidence( "Journal", "FIXME", mCurrentUID );
    mCurrentUID = QString::null;
  }
  mCalendar.deleteJournal(journal);
}


Journal *ResourceIMAP::journal(const QDate &date)
{
  return mCalendar.journal(date);
}

Journal *ResourceIMAP::journal(const QString &uid)
{
  return mCalendar.journal(uid);
}

Journal::List ResourceIMAP::journals()
{
  return mCalendar.journals();
}

/***********************************************
 * Alarm handling
 */

Alarm::List ResourceIMAP::alarmsTo( const QDateTime &to )
{
  return mCalendar.alarmsTo(to);
}

Alarm::List ResourceIMAP::alarms( const QDateTime &from, const QDateTime &to )
{
  return mCalendar.alarms( from, to );
}

/***********************************************
 * update() (kind of slot)
 */

// after changes are made to an event, this should be called.
void ResourceIMAP::update(IncidenceBase *incidencebase)
{
  QString type = incidencebase->type();
  if ( type == "Event" ) type = "Calendar";
  else if ( type == "Todo" ) type = "Task";
  else if ( type != "Journal" ) return;

  incidencebase->setSyncStatus(Event::SYNCMOD);
  incidencebase->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  mCurrentUID = incidencebase->uid();
  Q_ASSERT( mUidmap.contains( mCurrentUID ) );

  // Update the iCal
  QString iCal = mFormat.createScheduleMessage( incidencebase,
                                                Scheduler::Request );
  bool rc = kmailUpdate( type, mUidmap[ mCurrentUID ], mCurrentUID, iCal );
  mCurrentUID = QString::null;

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::update()\n";
}

KCal::Incidence* ResourceIMAP::parseIncidence( const QString& str )
{
 Incidence* i = mFormat.fromString( str );
 return i;
}

bool ResourceIMAP::addIncidence( const QString& type, const QString& ical )
{
  if( type != "Calendar" && type != "Task" && type != "Journal" )
    // Not an ical for us
    return false;

  // kdDebug() << "ResourceIMAP::addIncidence( " << type << ", "
  //           << /*ical*/"..." << " )" << endl;
  Incidence* i = parseIncidence( ical );
  if ( !i ) return false;
  // Ignore events that come from us
  if ( !mCurrentUID.isNull() && mCurrentUID == i->uid() ) return true;

  mSilent = true;
  if ( type == "Calendar" && i->type() == "Event" ) {
    addEvent( static_cast<Event*>(i) );
    emit resourceChanged( this );
  } else if ( type == "Task" && i->type() == "Todo" ) {
    addTodo( static_cast<Todo*>(i) );
    emit resourceChanged( this );
  } else if ( type == "Journal" && i->type() == "Journal" ) {
    addJournal( static_cast<Journal*>(i) );
    emit resourceChanged( this );
  }
  mSilent = false;

  return true;
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid )
{
  if( type != "Calendar" && type != "Task" && type != "Journal" )
    // Not an ical for us
    return;

  // kdDebug() << "ResourceIMAP::deleteIncidence( " << type << ", " << uid
  //           << " )" << endl;
  // Ignore events that come from us
  if ( !mCurrentUID.isNull() && mCurrentUID == uid ) return;

  mSilent = true;
  if ( type == "Calendar" ) {
    Event* e = event( uid );
    if( e ) {
      deleteEvent( e );
      emit resourceChanged( this );
    }
  } else if ( type == "Task" ) {
    Todo* t = todo( uid );
    if( t ) {
      deleteTodo( t );
      emit resourceChanged( this );
    }
  } else if ( type == "Journal" ) {
    Journal* j = journal( uid );
    if( j ) {
      deleteJournal( j );
      emit resourceChanged( this );
    }
  }
  mSilent = false;
}

void ResourceIMAP::slotRefresh( const QString& type )
{
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTasks();
  else if ( type == "Journal" )
    loadAllJournals();
}

QStringList ResourceIMAP::subresources() const
{
  QStringList calendar, tasks, journal;
  if ( kmailSubresources( calendar, "Calendar" ) )
    if ( kmailSubresources( tasks, "Task" ) )
      kmailSubresources( journal, "Journal" );
  return calendar + tasks + journal;
}

void ResourceIMAP::setSubresourceActive( const QString& subresource,
                                         bool active )
{
  // Get the config file
  QString configFile = locateLocal( "config", "kresources/imaprc" );
  KConfig config( configFile );

  if ( mEventResources.contains( subresource ) ) {
    config.setGroup( "Calendar" );
    config.writeEntry( subresource, active );
    slotRefresh( "Calendar" );
  } else if ( mTaskResources.contains( subresource ) ) {
    config.setGroup( "Task" );
    config.writeEntry( subresource, active );
    slotRefresh( "Task" );
  } else if ( mJournalResources.contains( subresource ) ) {
    config.setGroup( "Journal" );
    config.writeEntry( subresource, active );
    slotRefresh( "Journal" );
  }
}

void ResourceIMAP::subresourceAdded( const QString& type, const QString& )
{
  // TODO: Optimize this
  slotRefresh( type );
}

void ResourceIMAP::subresourceDeleted( const QString& type, const QString& )
{
  // TODO: Optimize this
  slotRefresh( type );
}

void ResourceIMAP::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include "resourceimap.moc"

