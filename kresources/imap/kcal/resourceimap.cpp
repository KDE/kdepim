/*
    This file is part of libkcal.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 - 2004 Bo Thorsen <bo@sonofthor.dk>

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

#include "resourceimap.h"

using namespace KCal;


ResourceIMAP::ResourceIMAP( const KConfig* config )
  : ResourceCalendar( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-libkcal" )
{
  setType( "imap" );
}

ResourceIMAP::~ResourceIMAP()
{
  if ( isOpen() )
    close();
}

bool ResourceIMAP::doOpen()
{
  KConfig config( configFile() );

  // Read the calendar entries
  QStringList resources;
  if ( !kmailSubresources( resources, "Calendar" ) ) {
    kdError(5650) << "Couldn't talk to KMail\n";
    return false;
  }
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
  KConfig config( configFile() );

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

bool ResourceIMAP::doLoad()
{
  mUidmap.clear();

  // Load each resource. Note: It's intentional to use & instead of &&
  // so we try all three, even if the first failed
  return loadAllEvents() & loadAllTasks() & loadAllJournals();
}

// helper function which parses a list of incidences
void ResourceIMAP::populate( const QStringList& lst, const QString& type,
                             const QString& folder )
{
  const bool silent = mSilent;
  mSilent = true;
  kdDebug(5650) << "Foldertype: " << type << endl;
  for ( QStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if ( i ) {
      if ( i->type() == "Event" && type == "Calendar" ) {
          addEvent( static_cast<Event*>( i ), folder );
      } else if ( i->type() == "Todo" && type == "Task" ) {
          addTodo( static_cast<Todo*>( i ), folder );
      } else if ( i->type() == "Journal" && type == "Journal" ) {
          addJournal( static_cast<Journal*>( i ), folder );
      } else {
        kdDebug(5650) << "Unknown incidence type " << i->type();
        delete i;
      }
    } else
      kdDebug(5650) << "Problem reading: " << *it << endl;
  }
  mSilent = silent;
}

bool ResourceIMAP::loadResource( const QString& type, const QString& folder )
{
  // Get the list of incidences
  QStringList lst;
  if ( !kmailIncidences( lst, type, folder ) )
    // The get failed
    return false;

  // Populate the calendar with the new incidences
  populate( lst, type, folder );
  return true;
}

bool ResourceIMAP::loadAllEvents()
{
  // We get a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();

  bool rc = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mEventResources.begin(); itR != mEventResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    rc &= loadResource( "Calendar", itR.key() );
  }

  emit resourceChanged( this );
  return rc;
}

bool ResourceIMAP::loadAllTasks()
{
  // We get a fresh list of todos, so clean out the old ones
  mCalendar.deleteAllTodos();

  bool rc = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR = mTaskResources.begin(); itR != mTaskResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    rc &= loadResource( "Task", itR.key() );
  }

  emit resourceChanged( this );
  return true;
}

bool ResourceIMAP::loadAllJournals()
{
  // We get a fresh list of journals, so clean out the old ones
  mCalendar.deleteAllJournals();

  bool rc = true;
  QMap<QString, bool>::ConstIterator itR;
  for ( itR=mJournalResources.begin(); itR!=mJournalResources.end(); ++itR ) {
    if ( !itR.data() )
      // This resource is disabled
      continue;

    rc &= loadResource( "Journal", itR.key() );
  }

  emit resourceChanged( this );
  return true;
}

bool ResourceIMAP::doSave()
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
  const QString uid = anEvent->uid();
  mCalendar.addEvent(anEvent);
  anEvent->registerObserver( this );

  // Register for the subresource
  QString resource = subresource;
  if ( subresource.isEmpty() )
    resource = findWritableResource( mEventResources, "Calendar" );
  if ( resource.isEmpty() )
    return false;

  mUidmap[ uid ] = resource;

  if ( mSilent ) return true;

  const QString vCal =
    mFormat.createScheduleMessage( anEvent, Scheduler::Request );
  const bool rc = kmailAddIncidence( "Calendar", resource, uid, vCal );

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addEvent()\n";

  return rc;
}

void ResourceIMAP::deleteEvent(Event *event)
{
  const QString uid = event->uid();
  Q_ASSERT( mUidmap.contains( uid ) );
  kmailDeleteIncidence( "Calendar", mUidmap[ uid ], uid );
  mUidmap.erase( uid );
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

Event::List ResourceIMAP::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection);
}

/***********************************************
 * Adding and removing Todos
 */

bool ResourceIMAP::addTodo(Todo *todo)
{
  return addTodo( todo, QString::null );
}

bool ResourceIMAP::addTodo( Todo *todo, const QString& subresource )
{
  const QString uid = todo->uid();
  // Don't add todo's twice, CalendarLocal happily accepts duplicates :(
  if ( mCalendar.todo( uid ) ) return true;
  mCalendar.addTodo( todo );
  todo->registerObserver( this );

  // Register for the subresource
  QString resource = subresource;
  if ( subresource.isEmpty() )
    resource = findWritableResource( mTaskResources, "Task" );
  if ( resource.isEmpty() )
    return false;

  mUidmap[ uid ] = resource;

  if ( mSilent ) return true;
  const QString vCal =
    mFormat.createScheduleMessage( todo, Scheduler::Request );
  const bool rc = kmailAddIncidence( "Task", resource, uid, vCal );

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addTodo()\n";

  return rc;
}

void ResourceIMAP::deleteTodo(Todo *todo)
{
  const QString uid = todo->uid();
  Q_ASSERT( mUidmap.contains( uid ) );
  kmailDeleteIncidence( "Task", mUidmap[ uid ], uid );
  mUidmap.erase( uid );
  mCalendar.deleteTodo(todo);
}

/***********************************************
 * Getting Todos
 */

Todo::List ResourceIMAP::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
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

bool ResourceIMAP::addJournal( Journal *journal )
{
  return addJournal( journal, QString::null );
}

bool ResourceIMAP::addJournal( Journal *journal, const QString& subresource )
{
  const QString uid = journal->uid();
  mCalendar.addJournal(journal);
  journal->registerObserver( this );

  // Register for the subresource
  QString resource = subresource;
  if ( subresource.isEmpty() )
    resource = findWritableResource( mJournalResources, "Journal" );
  if ( resource.isEmpty() )
    return false;

  mUidmap[ uid ] = resource;

  if ( mSilent ) return true;

  // Call kmail ..
  const QString vCal =
    mFormat.createScheduleMessage( journal, Scheduler::Request );
  const bool rc = kmailAddIncidence( "Journal", resource, uid, vCal );

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addJournal()\n";

  return rc;
}

void ResourceIMAP::deleteJournal(Journal *journal)
{
  if( !journal )
    return;

  const QString uid = journal->uid();
  Q_ASSERT( mUidmap.contains( journal->uid() ) );
  kmailDeleteIncidence( "Journal", mUidmap[ uid ], uid );
  mUidmap.erase( uid );
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

Journal::List ResourceIMAP::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

Journal *ResourceIMAP::rawJournalForDate( const QDate &date )
{
  return mCalendar.rawJournalForDate( date );
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

// after changes are made to an incidence, this is called via the Observer.
void ResourceIMAP::incidenceUpdated( IncidenceBase *incidencebase )
{
  QString type = incidencebase->type();
  if ( type == "Event" ) type = "Calendar";
  else if ( type == "Todo" ) type = "Task";
  else if ( type != "Journal" ) return;

  incidencebase->setSyncStatus( Event::SYNCMOD );
  incidencebase->setLastModified( QDateTime::currentDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  const QString uid = incidencebase->uid();
  Q_ASSERT( mUidmap.contains( uid ) );

  // Update the iCal
  const QString iCal = mFormat.createScheduleMessage( incidencebase,
                                                      Scheduler::Request );
  const bool rc = kmailUpdate( type, mUidmap[ uid ], uid, iCal );

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::update()\n";
}

KCal::Incidence* ResourceIMAP::parseIncidence( const QString& str )
{
  return mFormat.fromString( str );
}

bool ResourceIMAP::addIncidence( const QString& type, const QString& resource,
                                 const QString& ical )
{
  if( type != "Calendar" && type != "Task" && type != "Journal" )
    // Not an ical for us
    return false;

  // kdDebug() << "ResourceIMAP::addIncidence( " << type << ", "
  //           << /*ical*/"..." << " )" << endl;
  Incidence* i = parseIncidence( ical );
  if ( !i ) return false;

  const bool silent = mSilent;
  mSilent = true;
  if ( type == "Calendar" && i->type() == "Event" ) {
    addEvent( static_cast<Event*>(i), resource );
    emit resourceChanged( this );
  } else if ( type == "Task" && i->type() == "Todo" ) {
    addTodo( static_cast<Todo*>(i), resource );
    emit resourceChanged( this );
  } else if ( type == "Journal" && i->type() == "Journal" ) {
    addJournal( static_cast<Journal*>(i), resource );
    emit resourceChanged( this );
  }
  mSilent = silent;

  return true;
}

void ResourceIMAP::deleteIncidence( const QString& type,
                                    const QString& /*resource*/,
                                    const QString& uid )
{
  deleteIncidence( type, uid, true );
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid,
                                    bool silence )
{
  if( type != "Calendar" && type != "Task" && type != "Journal" )
    // Not an ical for us
    return;

  const bool silent = mSilent;
  mSilent = silence;
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
  mSilent = silent;
}

void ResourceIMAP::slotRefresh( const QString& type,
                                const QString& /*resource*/ )
{
  // TODO: Only load the specified resource
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTasks();
  else if ( type == "Journal" )
    loadAllJournals();
}

QStringList ResourceIMAP::subresources() const
{
  // TODO: This is not needed - make the list out of the mXResources instead
  QStringList calendar, tasks, journal;
  if ( kmailSubresources( calendar, "Calendar" ) )
    if ( kmailSubresources( tasks, "Task" ) )
      kmailSubresources( journal, "Journal" );
  return calendar + tasks + journal;
}

void ResourceIMAP::setSubresourceActive( const QString& subresource,
                                         bool active )
{
  kdDebug(5650) << "setSubresourceActive( " << subresource << ", "
                << active << " )\n";
  KConfig config( configFile() );

  if ( mEventResources.contains( subresource ) ) {
    kdDebug(5650) << "Calendar\n";
    config.setGroup( "Calendar" );
    config.writeEntry( subresource, active );
    mEventResources[ subresource ] = active;
    slotRefresh( "Calendar", subresource );
  } else if ( mTaskResources.contains( subresource ) ) {
    config.setGroup( "Task" );
    config.writeEntry( subresource, active );
    mTaskResources[ subresource ] = active;
    slotRefresh( "Task", subresource );
  } else if ( mJournalResources.contains( subresource ) ) {
    config.setGroup( "Journal" );
    config.writeEntry( subresource, active );
    mJournalResources[ subresource ] = active;
    slotRefresh( "Journal", subresource );
  }

  config.sync();
}

// Add the new subresource entries
void ResourceIMAP::subresourceAdded( const QString& type,
                                     const QString& subresource )
{
  KConfig config( configFile() );
  config.setGroup( type );

  if ( type == "Calendar" ) {
    if ( !mEventResources.contains( subresource ) ) {
      mEventResources[ subresource ] =
        config.readBoolEntry( subresource, true );
      loadResource( "Calendar", subresource );
      emit resourceChanged( this );
    }
  } else if ( type == "Task" ) {
    if ( !mTaskResources.contains( subresource ) ) {
      mTaskResources[ subresource ] =
        config.readBoolEntry( subresource, true );
      loadResource( "Task", subresource );
      emit resourceChanged( this );
    }
  }
  else if ( type == "Journal" ) {
    if ( !mJournalResources.contains( subresource ) ) {
      mJournalResources[ subresource ] =
        config.readBoolEntry( subresource, true );
      loadResource( "Journal", subresource );
      emit resourceChanged( this );
    }
  }

  emit signalSubresourceAdded( this, type, subresource );
}

void ResourceIMAP::subresourceDeleted( const QString& type,
                                       const QString& subresource )
{
  if ( type == "Calendar" ) {
    if ( !mEventResources.contains( subresource ) )
      // Not registered
      return;
    mEventResources.erase( subresource );
  } else if ( type == "Task" ) {
    if ( !mTaskResources.contains( subresource ) )
      // Not registered
      return;
    mTaskResources.erase( subresource );
  } else if ( type == "Journal" ) {
    if ( !mJournalResources.contains( subresource ) )
      // Not registered
      return;
    mJournalResources.erase( subresource );
  } else
    // Unknown type
    return;

  KConfig config( configFile() );
  config.setGroup( type );
  config.deleteEntry( subresource );
  config.sync();

  // Make a list of all uids to remove
  QMap<QString, QString>::ConstIterator mapIt;
  QStringList uids;
  for ( mapIt = mUidmap.begin(); mapIt != mUidmap.end(); ++mapIt )
    if ( mapIt.data() == subresource )
      // We have a match
      uids << mapIt.key();

  // Finally delete all the incidences
  if ( !uids.isEmpty() ) {
    QStringList::ConstIterator it;
    for ( it = uids.begin(); it != uids.end(); ++it )
      deleteIncidence( type, *it, false );

    emit resourceChanged( this );
  }

  emit signalSubresourceRemoved( this, type, subresource );
}

bool ResourceIMAP::subresourceActive( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceIMAP*>( this )->doOpen();

  if ( mEventResources.contains( subresource ) )
    return mEventResources[ subresource ];
  if ( mTaskResources.contains( subresource ) )
    return mTaskResources[ subresource ];
  if ( mJournalResources.contains( subresource ) )
    return mJournalResources[ subresource ];

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

void ResourceIMAP::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
  mFormat.setTimeZone( tzid, true );
}

void ResourceIMAP::asyncLoadResult( const QStringList& lst, const QString& type,
                                    const QString& folder )
{
  populate( lst, type, folder );
  emit resourceChanged( this );
}

#include "resourceimap.moc"
