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

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/vcalformat.h>
#include <libkcal/exceptions.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>
#include <kresources/resource.h>

#include "resourceimapconfig.h"
#include "resourceimap.h"

using namespace KCal;


extern "C"
{
  void *init_kcal_imap()
  {
    return new KRES::PluginFactory<ResourceIMAP,ResourceIMAPConfig>();
  }
}


ResourceIMAP::ResourceIMAP( const KConfig* config )
  : ResourceCalendar( config ),
    ResourceIMAPBase::ResourceIMAPShared( "ResourceIMAP-libkcal" ),
    mSilent( false )
{
  if ( config ) {
    mServer = config->readEntry( "Servername" );
  }
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

bool ResourceIMAP::getIncidenceList( QStringList& lst, const QString& type )
{
  if( !kmailIncidences( lst, type ) ) {
    kdError() << "Communication problem in ResourceIMAP::getIncidenceList()\n";
    return false;
  }

  return true;
}

bool ResourceIMAP::doOpen()
{
  return true;
}

bool ResourceIMAP::load()
{
  kdDebug(5800) << "Loading resource " << resourceName() << " on "
                << mServer << endl;

  // Load each resource. Note: It's intentional to use & instead of &&
  // so we try all three, even if the first failed
  return loadAllEvents() & loadAllTasks() & loadAllJournals();
}

bool ResourceIMAP::loadAllEvents()
{
  // Get the list of events
  QStringList lst;
  if ( !getIncidenceList( lst, "Calendar" ) )
    // The get failed
    return false;

  // We got a fresh list of events, so clean out the old ones
  mCalendar.deleteAllEvents();

  // Populate the calendar with the new events
  for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if ( i ) {
      if ( i->type() == "Event" ) {
        mCalendar.addEvent(static_cast<Event*>(i));
        i->registerObserver( this );
      } else {
        kdDebug() << "Unknown incidence type " << i->type();
        delete i;
      }
    }
  }

  return true;
}

bool ResourceIMAP::loadAllTasks()
{
  // Get the list of todos
  QStringList lst;
  if ( !getIncidenceList( lst, "Task" ) )
    // The get failed
    return false;

  // We got a fresh list of todos, so clean out the old ones
  mCalendar.deleteAllTodos();

  // Populate the calendar with the new todos
  for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if ( i ) {
      if ( i->type() == "Todo" ) {
        mCalendar.addTodo(static_cast<Todo*>(i));
        i->registerObserver( this );
      } else {
        kdDebug() << "Unknown incidence type " << i->type();
        delete i;
      }
    }
  }

  return true;
}

bool ResourceIMAP::loadAllJournals()
{
  // Get the list of journals
  QStringList lst;
  if ( !getIncidenceList( lst, "Journal" ) )
    // The get failed
    return false;

  // We got a fresh list of journals, so clean out the old ones
  mCalendar.deleteAllJournals();

  // Populate the calendar with the new journals
  for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if ( i ) {
      if ( i->type() == "Journal" ) {
        mCalendar.addJournal(static_cast<Journal*>(i));
        i->registerObserver( this );
      } else {
        kdDebug() << "Unknown incidence type " << i->type();
        delete i;
      }
    }
  }

  return true;
}

bool ResourceIMAP::save()
{
  return false;
}

KABC::Lock *ResourceIMAP::lock()
{
  return new KABC::LockNull( true );
}

/***********************************************
 * Adding and removing Events
 */

bool ResourceIMAP::addEvent(Event *anEvent)
{
  kdDebug(5800) << "ResourceIMAP::addEvent" << endl;
  mCalendar.addEvent(anEvent);
  anEvent->registerObserver( this );

  if ( mSilent ) return true;

  mCurrentUID = anEvent->uid();
  QString vCal = mFormat.createScheduleMessage( anEvent, Scheduler::Request );
  bool rc = kmailAddIncidence( "Calendar", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !rc )
    kdError() << "Communication problem in ResourceIMAP::addEvent()\n";

  return rc;
}

// probably not really efficient, but...it works for now.
void ResourceIMAP::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceIMAP::deleteEvent" << endl;

  // Call kmail ...
  if ( !mSilent ) {
    mCurrentUID = event->uid();
    kmailDeleteIncidence( "Calendar", mCurrentUID );
    mCurrentUID = QString::null;
  }

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
  bool rc = kmailAddIncidence( "Task", mCurrentUID, vCal );
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
    kmailDeleteIncidence( "Task", mCurrentUID );
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

Todo::List ResourceIMAP::todos( const QDate &date )
{
  return mCalendar.todos(date);
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
  bool rc = kmailAddIncidence( "Journal", mCurrentUID, vCal );
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
    kmailDeleteIncidence( "Journal", mCurrentUID );
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

  // Delete the old one and add the new version
  mCurrentUID = incidencebase->uid();
  QString vCal = mFormat.createScheduleMessage( incidencebase,
                                                Scheduler::Request );
  bool rc = kmailDeleteIncidence( type, mCurrentUID );
  rc &= kmailAddIncidence( type, mCurrentUID, vCal );
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

void ResourceIMAP::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
}

#include "resourceimap.moc"

