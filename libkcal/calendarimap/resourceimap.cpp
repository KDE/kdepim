/*
    This file is part of libkcal.

    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>
    Copyright (c) 2003 Bo Thorsen <bo@klaralvdalens-datakonsult.se>

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
#include <dcopclient.h>
#include <kapplication.h>
#include <kdcopservicestarter.h>
#include <klocale.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/vcalformat.h>
#include <libkcal/exceptions.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>

#include <kresources/configwidget.h>
#include <kresources/resource.h>

#include "kmailicalIface_stub.h"

#include "resourceimapconfig.h"
#include "resourceimap.h"

using namespace KCal;

static const QCString dcopObjectId = "KMailICalIface";

extern "C"
{
  void *init_kcal_imap()
  {
    return new KRES::PluginFactory<ResourceIMAP,ResourceIMAPConfig>();
  }
}


ResourceIMAP::ResourceIMAP( const KConfig* config )
  : DCOPObject("ResourceIMAP"), ResourceCalendar( config )
{
  if ( config ) {
    mServer = config->readEntry( "Servername" );
  }
  init();

  // Make the connection to KMail ready
  mKMailIcalIfaceStub = 0;
  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString& ) ),
           this, SLOT( unregisteredFromDCOP( const QCString& ) ) );
}

void ResourceIMAP::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "Servername", mServer );
}

void ResourceIMAP::init()
{
  kdDebug(5800) << "ResourceIMAP::init()" << endl;

  mSilent = false;

  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();
  mDCOPClient->registerAs( "resourceimap", true );
}


ResourceIMAP::~ResourceIMAP()
{
  kapp->dcopClient()->setNotifications( false );
  delete mKMailIcalIfaceStub;
  close();
  delete mDCOPClient;
}

bool ResourceIMAP::getIncidenceList( QStringList& lst, const QString& type )
{
  if ( !connectToKMail() ) {
    kdError() << "DCOP error during incidences(QString)\n";
    return false;
  }

  lst = mKMailIcalIfaceStub->incidences( type );
  if ( !mKMailIcalIfaceStub->ok() ) {
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

/***********************************************
 * Adding and removing Events
 */

bool ResourceIMAP::addEvent(Event *anEvent)
{
  kdDebug(5800) << "ResourceIMAP::addEvent" << endl;
  mCalendar.addEvent(anEvent);
  anEvent->registerObserver( this );

  if ( mSilent ) return true;

  // Call kmail ...
  if ( !connectToKMail() ) {
    kdError() << "DCOP error during addIncidence(QString)\n";
    return false;
  }

  mCurrentUID = anEvent->uid();
  QString vCal = mFormat.createScheduleMessage( anEvent, Scheduler::Request );
  bool rc = mKMailIcalIfaceStub->addIncidence( "Calendar", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::addEvent()\n";
    return false;
  }

  return rc;
}

// probably not really efficient, but...it works for now.
void ResourceIMAP::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceIMAP::deleteEvent" << endl;

  // Call kmail ...
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during "
                << "ResourceIMAP::deleteIncidence(QString)\n";
    } else {
      mCurrentUID = event->uid();
      mKMailIcalIfaceStub->deleteIncidence( "Calendar", mCurrentUID );
    }
  }

  mCalendar.deleteEvent(event);
  mCurrentUID = QString::null;
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
QPtrList<Event> ResourceIMAP::rawEventsForDate( const QDate &qd, bool sorted )
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


QPtrList<Event> ResourceIMAP::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceIMAP::rawEventsForDate(const QDateTime &qdt)
{
  return mCalendar.rawEventsForDate( qdt );
}

QPtrList<Event> ResourceIMAP::rawEvents()
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

  if ( !connectToKMail() ) {
    kdError() << "DCOP error during addTodo(QString)\n";
    return false;
  }

  mCurrentUID = todo->uid();
  QString vCal = mFormat.createScheduleMessage( todo, Scheduler::Request );
  bool rc = mKMailIcalIfaceStub->addIncidence( "Task", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::addTodo()\n";
    return false;
  }

  return rc;
}

void ResourceIMAP::deleteTodo(Todo *todo)
{
  // call kmail ...
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during ResourceIMAP::deleteTodo(QString)\n";
    } else {
      mCurrentUID = todo->uid();
      mKMailIcalIfaceStub->deleteIncidence( "Task", mCurrentUID );
      mCurrentUID = QString::null;
    }
  }
  mCalendar.deleteTodo(todo);
}

/***********************************************
 * Getting Todos
 */

QPtrList<Todo> ResourceIMAP::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceIMAP::todo( const QString &uid )
{
  return mCalendar.todo(uid);
}

QPtrList<Todo> ResourceIMAP::todos( const QDate &date )
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
  if ( !connectToKMail() ) {
    kdError() << "DCOP error during addTodo(QString)\n";
    return false;
  }

  mCurrentUID = journal->uid();
  QString vCal = mFormat.createScheduleMessage( journal, Scheduler::Request );
  bool rc = mKMailIcalIfaceStub->addIncidence( "Journal", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::addJournal()\n";
    return false;
  }

  return rc;
}

void ResourceIMAP::deleteJournal(Journal *journal)
{
  if ( !mSilent ) {
    if ( !connectToKMail() ) {
      kdError() << "DCOP error during ResourceIMAP::deleteJournal(QString)\n";
    } else {
      mCurrentUID = journal->uid();
      mKMailIcalIfaceStub->deleteIncidence( "Journal", mCurrentUID );
      mCurrentUID = QString::null;
    }
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

QPtrList<Journal> ResourceIMAP::journals()
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
  if ( !connectToKMail() ) {
    kdError() << "DCOP error during ResourceIMAP::update(QString)\n";
    return;
  }

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
  mKMailIcalIfaceStub->deleteIncidence( type, mCurrentUID );
  mKMailIcalIfaceStub->addIncidence( "Journal", mCurrentUID, vCal );
  mCurrentUID = QString::null;

  if ( !mKMailIcalIfaceStub->ok() ) {
    kdError() << "Communication problem in ResourceIMAP::addJournal()\n";
  }
}

KCal::Incidence* ResourceIMAP::parseIncidence( const QString& str )
{
 Incidence* i = mFormat.fromString( str );
 return i;
}

bool ResourceIMAP::addIncidence( const QString& type, const QString& ical )
{
  // kdDebug() << "ResourceIMAP::addIncidence( " << type << ", "
  //           << /*ical*/"..." << " )" << endl;
  Incidence* i = parseIncidence( ical );
  if ( !i ) return false;
  // Ignore events that come from us
  if ( !mCurrentUID.isNull() && mCurrentUID == i->uid() ) return true;

  if ( type == "Calendar" ) {
    if ( i && i->type() == "Event" ) {
      mSilent = true;
      addEvent( static_cast<Event*>(i) );
      mSilent = false;
    }
  }

  return true;
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid )
{
  // kdDebug() << "ResourceIMAP::deleteIncidence( " << type << ", " << uid
  //           << " )" << endl;
  // Ignore events that come from us
  if ( !mCurrentUID.isNull() && mCurrentUID == uid ) return;

  mSilent = true;
  if ( type == "Calendar" ) {
    Event* e = event(uid);
    deleteEvent(e);
  } else if ( type == "Task" ) {
    Todo* t = todo(uid);
    deleteTodo(t);
  } else if ( type == "Journal" ) {
    Journal* j = journal(uid);
    deleteJournal(j);
  }
  mSilent = false;
}

void ResourceIMAP::slotRefresh( const QString& type )
{
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTasks();
  else
    kdDebug(5800) << "ResourceIMAP::slotRefresh called with wrong type " << type << endl;
}

bool ResourceIMAP::connectToKMail() const
{
  if ( !mKMailIcalIfaceStub ) {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->
      findServiceFor( "DCOP/ResourceBackend/IMAP", QString::null,
                      QString::null, &error, &dcopService );
    if ( result != 0 ) {
      kdDebug(5800) << "Couldn't connect to the IMAP resource backend\n";
      // TODO: You might want to show "error" (if not empty) here, using e.g. KMessageBox
      return false;
    }

    mKMailIcalIfaceStub = new KMailICalIface_stub( kapp->dcopClient(),
                                                   dcopService, dcopObjectId );

    // Attach to the KMail signals
    if ( !connectKMailSignal( "incidenceAdded(QString,QString)",
                              "addIncidence(QString,QString)" ) ) {
      kdError() << "DCOP connection to incidenceAdded failed" << endl;
    }
    if ( !connectKMailSignal( "incidenceDeleted(QString,QString)",
                              "deleteIncidence(QString,QString)" ) ) {
      kdError() << "DCOP connection to incidenceDeleted failed" << endl;
    }
    if ( !connectKMailSignal( "signalRefresh(QString)",
                              "slotRefresh(QString)" ) ) {
      kdError() << "DCOP connection to signalRefresh failed" << endl;
    }
  }

  return ( mKMailIcalIfaceStub != 0 );
}

bool ResourceIMAP::connectKMailSignal( const QCString& signal,
                                       const QCString& method ) const
{
  ResourceIMAP* _this = const_cast<ResourceIMAP*>( this );
  return _this->connectDCOPSignal( "kmail", dcopObjectId, signal, method,
                                   false );
}

void ResourceIMAP::unregisteredFromDCOP( const QCString& appId )
{
  if ( mKMailIcalIfaceStub && mKMailIcalIfaceStub->app() == appId ) {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete mKMailIcalIfaceStub;
    mKMailIcalIfaceStub = 0L;
  }
}


#include "resourceimap.moc"

