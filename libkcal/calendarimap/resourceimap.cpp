/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

void ResourceIMAP::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "Servername", mServer );
}

void ResourceIMAP::init()
{
  mOldestDate = 0L;
  mNewestDate = 0L;

  mRecursList.setAutoDelete(TRUE);
  mTodoList.setAutoDelete(TRUE);
  mEventList.setAutoDelete(TRUE);

  mDCOPClient = new DCOPClient();
  mDCOPClient->attach();

  // TODO: Make sure KMail is running!

  // attach to KMail
  if( !mDCOPClient->connectDCOPSignal( "KMail", "KmailICalIface", "incidenceAdded(QString,QString)",
				       "ResourceIMAP", "addIncidence(QString,QString)", true ) ) {
    kdError() << "DCOP connection to incidenceAdded failed" << endl;
  }
  if( !mDCOPClient->connectDCOPSignal( "KMail", "KmailICalIface", "incidenceDeleted(QString,QString)",
				       "ResourceIMAP", "deleteIncidence(QString,QString)", true ) ) {
    kdError() << "DCOP connection to incidenceDeleted failed" << endl;
  }
}
  

ResourceIMAP::~ResourceIMAP()
{
  close();
  delete mNewestDate;
  delete mOldestDate;
}

bool ResourceIMAP::doOpen()
{
  kdDebug(5800) << "Opening resource " << resourceName() << " on " << mServer << endl;

  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << "Calendar";
  QCString replyType;
  QByteArray reply;
  if( !mDCOPClient->call( "KMail", "KMailICalIface", "incidences(QString)",
			  data, replyType, reply )) {
    kdError() << "DCOP error during addIncicence(QString)" << endl;
  }
  QStringList lst;
  lst << reply;
  kdDebug() << "Got incidences " << lst.join("\n") << endl;
  // NYI: add list of icals to calendar
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
  mEventList.append(anEvent);
  anEvent->registerObserver( this );
  
  // call kmail ...
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << "Calendar";
  arg << anEvent->uid();
  arg << mFormat.toString(anEvent);
  if( !mDCOPClient->send( "KMail", "KMailICalIface", "addIncicence(QString,QString,QString)",
				      data )) {
    kdError() << "DCOP error during addIncicence(QString)" << endl;
  }

  //setModified( true );
}

// probably not really efficient, but...it works for now.
void ResourceIMAP::deleteEvent(Event *event)
{
  kdDebug(5800) << "ResourceIMAP::deleteEvent" << endl;
  mEventList.findRef(event);
  
  // call kmail ...
  QByteArray data;
  QDataStream arg(data, IO_WriteOnly);
  arg << "Calendar";
  arg << event->uid();
  if( !mDCOPClient->send( "KMail", "KMailICalIface", "deleteIncicence(QString,QString)",
				      data )) {
    kdError() << "DCOP error during addIncicence(QString)" << endl;
  }
  
  mEventList.remove();
}


/***********************************************
 * Getting Events
 */

Event *ResourceIMAP::event( const QString &uid )
{
  kdDebug(5800) << "ResourceIMAP::event(): " << uid << endl;
  Event *e;
  for (e = mEventList.first();e;e = mEventList.next())
    if (e->uid() == uid) return e;
  // not found
  return 0;
}

int ResourceIMAP::numEvents(const QDate &qd)
{
  Event *anEvent;
  int count = 0;
  int extraDays, i;

  // first get the simple case from the dictionary.
  for (anEvent=mEventList.first();anEvent;anEvent=mEventList.next()) {
    if (qd == anEvent->dtStart().date()) count++;
  }
  // next, check for repeating events.  Even those that span multiple days...
  for (anEvent = mRecursList.first(); anEvent; anEvent = mRecursList.next()) {
    if (anEvent->isMultiDay()) {
      extraDays = anEvent->dtStart().date().daysTo(anEvent->dtEnd().date());
      //kdDebug(5800) << "multi day event w/" << extraDays << " days" << endl;
      for (i = 0; i <= extraDays; i++) {
        if (anEvent->recursOn(qd.addDays(i))) {
          ++count;
          break;
        }
      }
    } else {
      if (anEvent->recursOn(qd))
        ++count;
    }
  }

  return count;
}

// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
QPtrList<Event> ResourceIMAP::rawEventsForDate(const QDate &qd, bool sorted)
{
  // Search non-recurring events
  QPtrList<Event> eventList;
  // Search recurring events
  QPtrList<Event> eventListSorted;
  return eventListSorted;
}


QPtrList<Event> ResourceIMAP::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
   QPtrList<Event> matchList, *tmpList, tmpList2;
  return matchList;
}

QPtrList<Event> ResourceIMAP::rawEventsForDate(const QDateTime &qdt)
{
  return rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceIMAP::rawEvents()
{
  QPtrList<Event> eventList;
  return eventList;
}

/***********************************************
 * Adding and removing Todos
 */

void ResourceIMAP::addTodo(Todo *todo)
{
  mTodoList.append(todo);
  todo->registerObserver( this );

  // call kmail ..
  
//  setModified( true );
}

void ResourceIMAP::deleteTodo(Todo *todo)
{
  mTodoList.findRef(todo);

  // call kmail ...
  
  mTodoList.remove();

//  setModified( true );
}

/***********************************************
 * Getting Todos
 */

QPtrList<Todo> ResourceIMAP::rawTodos() const
{
  return mTodoList;
}

Todo *ResourceIMAP::todo( const QString &uid )
{
  Todo *aTodo;
  for (aTodo = mTodoList.first(); aTodo;
       aTodo = mTodoList.next())
    if (aTodo->uid() == uid)
      return aTodo;
  // not found
  return 0;
}

QPtrList<Todo> ResourceIMAP::todos( const QDate &date )
{
  QPtrList<Todo> todos;

  Todo *aTodo;
  for (aTodo = mTodoList.first();aTodo;aTodo = mTodoList.next()) {
    if (aTodo->hasDueDate() && aTodo->dtDue().date() == date) {
      todos.append(aTodo);
    }
  }

  return todos;
}

/***********************************************
 * Journal handling
 */

void ResourceIMAP::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;

  mJournalMap.insert(journal->dtStart().date(),journal);
  journal->registerObserver( this );

  // call kmail ...
  
//  setModified( true );
}

Journal *ResourceIMAP::journal(const QDate &date)
{
  kdDebug(5800) << "ResourceIMAP::journal() " << date.toString() << endl;

  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.find(date);
  if (it == mJournalMap.end()) return 0;
  else {
//    kdDebug(5800) << "  Found" << endl;
    return *it;
  }
}

Journal *ResourceIMAP::journal(const QString &uid)
{
  QMap<QDate,Journal *>::ConstIterator it = mJournalMap.begin();
  QMap<QDate,Journal *>::ConstIterator end = mJournalMap.end();
  for(;it != end; ++it) {
    if ((*it)->uid() == uid) return *it;
  }
  return 0;
}

QPtrList<Journal> ResourceIMAP::journals()
{
  QPtrList<Journal> list;

  QMap<QDate,Journal *>::Iterator it;
  for( it = mJournalMap.begin(); it != mJournalMap.end(); ++it ) {
    list.append(*it);
  }

  return list;
}

/***********************************************
 * Alarm handling
 */

Alarm::List ResourceIMAP::alarmsTo( const QDateTime &to )
{
  if( mOldestDate )
    return alarms( *mOldestDate, to );
  else
    return alarms( QDateTime( QDate( 1900, 1, 1 ) ), to );
}

Alarm::List ResourceIMAP::alarms( const QDateTime &from, const QDateTime &to )
{
  kdDebug(5800) << "ResourceIMAP::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  Alarm::List alarms;

//   // Check all non-recurring events.

//   // Check all recurring events.

//   // Check all todos.

  return alarms;
}

void ResourceIMAP::appendAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  QPtrList<Alarm> alarmList = incidence->alarms();
  Alarm *alarm;
  for( alarm = alarmList.first(); alarm; alarm = alarmList.next() ) {
//    kdDebug(5800) << "ResourceIMAP::appendAlarms() '" << incidence->summary()
//                  << "': " << alarm->time().toString() << " - " << alarm->enabled() << endl;
    if ( alarm->enabled() ) {
      if ( alarm->time() >= from && alarm->time() <= to ) {
        kdDebug(5800) << "ResourceIMAP::appendAlarms() '" << incidence->summary()
                      << "': " << alarm->time().toString() << endl;
        alarms.append( alarm );
      }
    }
  }
}

void ResourceIMAP::appendRecurringAlarms( Alarm::List &alarms, Incidence *incidence,
                                  const QDateTime &from, const QDateTime &to )
{
  QPtrList<Alarm> alarmList = incidence->alarms();
  Alarm *alarm;
  QDateTime qdt;
  for( alarm = alarmList.first(); alarm; alarm = alarmList.next() ) {
    if (incidence->recursOn(from.date())) {
      qdt.setTime(alarm->time().time());
      qdt.setDate(from.date());
    }
    else qdt = alarm->time();
    kdDebug(5800) << "ResourceIMAP::appendAlarms() '" << incidence->summary()
                  << "': " << qdt.toString() << " - " << alarm->enabled() << endl;
    if ( alarm->enabled() ) {
//      kdDebug(5800) << "ResourceIMAP::appendAlarms() '" << incidence->summary()
//                    << "': " << alarm->time().toString() << endl;
      if ( qdt >= from && qdt <= to ) {
        alarms.append( alarm );
      }
    }
  }
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
    Event *anEvent = static_cast<Event *>(incidence);

   // the first thing we do is REMOVE all occurances of the event from
   // both the dictionary and the recurrence list.  Then we reinsert it.
   // We don't bother about optimizations right now.

   // take any instances of it out of the recurrence list

    // ok the event is now GONE.  we want to re-insert it.
    //insertEvent(anEvent);
  }

}


void ResourceIMAP::addIncidence( const QString& type, const QString& ical )
{
  kdDebug() << "ResourceIMAP::addIncidence( " << type << ", " << ical << " )" << endl;
}

void ResourceIMAP::deleteIncidence( const QString& type, const QString& uid )
{
  kdDebug() << "ResourceIMAP::deleteIncidence( " << type << ", " << uid << " )" << endl;
}
