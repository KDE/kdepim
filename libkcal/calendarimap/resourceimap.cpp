/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Steffen Hansen <steffen@klaralvdalens-datakonsult.se>

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

#include <libkcal/vcaldrag.h>
#include <libkcal/vcalformat.h>
#include <libkcal/exceptions.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>

#include <kresources/resourceconfigwidget.h>
#include <kresources/resource.h>

#include "resourceimapconfig.h"
#include "resourceimap.h"

using namespace KCal;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new ResourceIMAPConfig( parent, "Configure IMAP-Based Calendar" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    return new ResourceIMAP( config );
  }
}

ResourceIMAP::ResourceIMAP( const KConfig* config )
  : ResourceCalendar( config ),
    DCOPObject("ResourceIMAP")
{
  if ( config ) {
    mServer = config->readEntry( "Servername" );
  }
  init();
}

void ResourceIMAP::writeConfig( KConfig* config ) const
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "Servername", mServer );
}

void ResourceIMAP::init()
{
  kdDebug() << "ResourceIMAP::init()" << endl;

  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();

  // TODO: Make sure KMail is running!

  // attach to KMail
  if( !mDCOPClient->connectDCOPSignal( "kmail", "KmailICalIface", "incidenceAdded(QString,QString)",
				       "ResourceIMAP", "addIncidence(QString,QString)", true ) ) {
    kdError() << "DCOP connection to incidenceAdded failed" << endl;
  }
  if( !mDCOPClient->connectDCOPSignal( "kmail", "KmailICalIface", "incidenceDeleted(QString,QString)",
				       "ResourceIMAP", "deleteIncidence(QString,QString)", true ) ) {
    kdError() << "DCOP connection to incidenceDeleted failed" << endl;
  }
}
  

ResourceIMAP::~ResourceIMAP()
{
  close();
  delete mDCOPClient;
}

QStringList ResourceIMAP::getIncidenceList( const QString& type )
{
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << type;
  QCString replyType = "QStringList";
  QByteArray reply;
  if( !mDCOPClient->call( "kmail", "KMailICalIface", "incidences(QString)",
			  data, replyType, reply ) || replyType != "QStringList" ) {
    kdError() << "DCOP error during incidences(QString)" << endl;
  }
  QStringList lst;
  QDataStream ret(reply, IO_ReadOnly);
  ret >> lst;
  return lst;
}

bool ResourceIMAP::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << " on " << mServer << endl;

  QStringList lst = getIncidenceList( "Calendar" );
  for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if( i ) {
      if( i->type() == "Event" ) {
	mCalendar.addEvent(static_cast<Event*>(i));
	i->registerObserver( this );
      } else {
	kdDebug() << "Unknown incidence type " << i->type();
	delete i;
      }
    }
  }
  lst = getIncidenceList( "Task" );
  for( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    Incidence* i = parseIncidence( *it );
    if( i ) {
      if( i->type() == "Todo" ) {
	mCalendar.addTodo(static_cast<Todo*>(i));
	i->registerObserver( this );
      } else {
	kdDebug() << "Unknown incidence type " << i->type();
	delete i;
      }
    }
  }
  // TODO: complete this for other incidence types

  return true;
}

bool ResourceIMAP::sync()
{
  return false;
}

/***********************************************
 * Adding and removing Events
 */

void ResourceIMAP::addEvent(Event *anEvent)
{
  kdDebug(5800) << "ResourceIMAP::addEvent" << endl;
  mCalendar.addEvent(anEvent);
  anEvent->registerObserver( this );
  
  // call kmail ...
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << QString::fromLatin1("Calendar");
  arg << anEvent->uid();

  // Kind of strange way to store the event
  // but it seems to work, and should be compatible
  // with kroupware_branch
  arg << mFormat.createScheduleMessage(anEvent, Scheduler::Request);
  if( !mDCOPClient->send( "kmail", "KMailICalIface", "addIncidence(QString,QString,QString)",
				      data )) {
    kdError() << "DCOP error during addIncidence(QString)" << endl;
  }
  //setModified( true );
}

// probably not really efficient, but...it works for now.
void ResourceIMAP::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceIMAP::deleteEvent" << endl;
  // call kmail ...
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << QString::fromLatin1("Calendar");
  arg << event->uid();
  if( !mDCOPClient->send( "kmail", "KMailICalIface", "deleteIncidence(QString,QString)",
				      data )) {
    kdError() << "DCOP error during deleteIncidence(QString)" << endl;
  }
  mCalendar.deleteEvent(event);
}


/***********************************************
 * Getting Events
 */

Event *ResourceIMAP::event( const QString &uid )
{
  kdDebug(5800) << "ResourceIMAP::event(): " << uid << endl;
  return mCalendar.event(uid);
}

int ResourceIMAP::numEvents(const QDate &qd)
{
  return mCalendar.numEvents(qd);
}

// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
QPtrList<Event> ResourceIMAP::rawEventsForDate(const QDate &qd, bool sorted)
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

void ResourceIMAP::addTodo(Todo *todo)
{
  mCalendar.addTodo(todo);
  todo->registerObserver( this );

  // call kmail ..
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << QString::fromLatin1("Task");
  arg << todo->uid();

  // Kind of strange way to store the event
  // but it seems to work, and should be compatible
  // with kroupware_branch
  arg << mFormat.createScheduleMessage(todo, Scheduler::Request);
  if( !mDCOPClient->send( "kmail", "KMailICalIface", "addIncidence(QString,QString,QString)",
				      data )) {
    kdError() << "DCOP error during addIncidence(QString)" << endl;
  }
  
//  setModified( true );
}

void ResourceIMAP::deleteTodo(Todo *todo)
{
  // call kmail ...
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << QString::fromLatin1("Task");
  arg << todo->uid();
  if( !mDCOPClient->send( "kmail", "KMailICalIface", "deleteIncidence(QString,QString)",
				      data )) {
    kdError() << "DCOP error during deleteIncidence(QString)" << endl;
  }
  mCalendar.deleteTodo(todo);
 
//  setModified( true );
}

/***********************************************
 * Getting Todos
 */

QPtrList<Todo> ResourceIMAP::rawTodos() const
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

void ResourceIMAP::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  mCalendar.addJournal(journal);
  journal->registerObserver( this );

  // call kmail ...
  
//  setModified( true );
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
  kdDebug(5800) << "ResourceIMAP::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  return mCalendar.alarms( from, to );
}

/***********************************************
 * update() (kind of slot)
 */

// after changes are made to an event, this should be called.
void ResourceIMAP::update(IncidenceBase *incidence)
{
  incidence->setSyncStatus(Event::SYNCMOD);
  incidence->setLastModified(QDateTime::currentDateTime());
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.

  if ( incidence->type() == "Event" ) {
    // Just get delete the event and add it again!
    Event *anEvent = static_cast<Event *>(incidence);
    Event *newEvent = static_cast<Event *>(anEvent->clone());
    deleteEvent( anEvent );
    addEvent( newEvent );

   // the first thing we do is REMOVE all occurances of the event from
   // both the dictionary and the recurrence list.  Then we reinsert it.
   // We don't bother about optimizations right now.

   // take any instances of it out of the recurrence list

    // ok the event is now GONE.  we want to re-insert it.
    //insertEvent(anEvent);
  }

}

KCal::Incidence* ResourceIMAP::parseIncidence( const QString& str )
{
  KCal::ScheduleMessage *message = mFormat.parseScheduleMessage( &mCalendar,
								 str );
  if( message ) {
    return dynamic_cast<KCal::Incidence*>( message->event() );
  } else {
    QString errorMessage;
    if( mFormat.exception() ) {
      errorMessage = mFormat.exception()->message();
    }
    kdDebug() << "ResourceIMAP::parseIncidence( " << str << ") Error parsing \""
	      << str << "\""
	      << "Message: " << errorMessage << endl;
    return 0;
  }
}

void ResourceIMAP::addIncidence( const QString& type, const QString& ical )
{
  kdDebug() << "ResourceIMAP::addIncidence( " << type << ", " << ical << " )" << endl;
  if( type == "Calendar" ) {
    Incidence* i = parseIncidence( ical );
    if( i && i->type() == "Event" ) {
      addEvent( static_cast<Event*>(i) );
    }
  }
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid )
{
  kdDebug() << "ResourceIMAP::deleteIncidence( " << type << ", " << uid << " )" << endl;
}
