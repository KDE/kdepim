/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
    02111-1307, USA.
*/

#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qwidgetlist.h>
#include <qwidget.h>

#include <kdebug.h>
#include <kapplication.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/journal.h>

#include <kresources/resourceconfigwidget.h>

#include "dateset.h"
#include "exchangeaccount.h"
#include "exchangeclient.h"
#include "exchangemonitor.h"

#include "resourceexchange.h"
#include "resourceexchangeconfig.h"

using namespace KCal;
using namespace KPIM;

extern "C"
{
  KRES::ResourceConfigWidget *config_widget( QWidget *parent ) {
    return new KCal::ResourceExchangeConfig( parent, "Configure Exchange Calendar" );
  }

  KRES::Resource *resource( const KConfig *config ) {
    kdDebug() << "In researchExchange static resource() function" << endl;
    return new KCal::ResourceExchange( config );
  }
}

class ResourceExchange::EventInfo {
public:
  KCal::Event* event;
  KURL url;
  long updateWatch;
};

ResourceExchange::ResourceExchange( const KConfig *config )
  : ResourceCalendar( config )
{
    mCache = 0L;
  kdDebug() << "Creating ResourceExchange" << endl;
  if (config ) {
    mAccount = new ExchangeAccount(
            config->readEntry( "ExchangeHost" ),
            config->readEntry( "ExchangeAccount" ),
            config->readEntry( "ExchangeMailbox" ),
            decryptStr( config->readEntry( "ExchangePassword" ) ) );
    mCachedSeconds = config->readNumEntry( "ExchangeCacheTimeout", 600 );
  } else {
    mAccount = new ExchangeAccount( "", "", "" );
    mCachedSeconds = 600;
  }
}

ResourceExchange::~ResourceExchange()
{
  kdDebug() << "Destructing ResourceExchange" << endl;
  close();
}

void ResourceExchange::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );
  config->writeEntry( "ExchangeHost", mAccount->host() );
  config->writeEntry( "ExchangeAccount", mAccount->account() );
  config->writeEntry( "ExchangeMailbox", mAccount->mailbox() );
  config->writeEntry( "ExchangePassword", encryptStr( mAccount->password() ) );
  config->writeEntry( "ExchangeCacheTimeout", mCachedSeconds );
}

bool ResourceExchange::doOpen()
{
  mClient = new ExchangeClient( mAccount );
  connect( mClient, SIGNAL( downloadFinished( int, const QString& ) ),
    this, SLOT( slotDownloadFinished( int, const QString& ) ) );
  connect( mClient, SIGNAL( event( KCal::Event*, const KURL& ) ),
    this, SLOT( downloadedEvent( KCal::Event*, const KURL& ) ) );

  kdDebug() << "Creating monitor" << endl;
  QHostAddress ip;
  ip.setAddress( "130.161.216.42" );
  mMonitor = new ExchangeMonitor( mAccount, ExchangeMonitor::CallBack, ip );
  connect( mMonitor, SIGNAL(notify( const QValueList<long>& , const QValueList<KURL>& )), this, SLOT(slotMonitorNotify( const QValueList<long>& , const QValueList<KURL>& )) );
  connect( mMonitor, SIGNAL(error(int , const QString&)), this, SLOT(slotMonitorError(int , const QString&)) );

  mMonitor->addWatch( mAccount->calendarURL(), ExchangeMonitor::UpdateNewMember, 1 );

  QWidgetList* widgets = QApplication::topLevelWidgets();
  if ( !widgets->isEmpty() )
    mClient->setWindow( widgets->first() );
  delete widgets;

  mDates = new DateSet();

  mEventDates = new QMap<Event,QDateTime>();
  mCacheDates = new QMap<QDate, QDateTime>();

  mCache = new CalendarLocal();
  // mOldestDate = 0L;
  // mNewestDate = 0L;

  // FIXME: check if server exists, account is OK, etc.
  return true;
}

void ResourceExchange::doClose()
{
  kdDebug() << "ResourceExchange::doClose()" << endl;
  // delete mNewestDate;
  // delete mOldestDate;
  delete mDates; mDates = 0;
  delete mMonitor; mMonitor = 0;
  delete mClient; mClient = 0;
  delete mAccount; mAccount = 0;
  delete mEventDates; mEventDates = 0;
  delete mCacheDates; mCacheDates = 0;
  if (mCache) {
    mCache->close();
    delete mCache; mCache = 0;
  }
//  setModified( false );
}

bool ResourceExchange::load()
{
  return true;
}

bool ResourceExchange::sync()
{
  return true;
}

void ResourceExchange::slotMonitorNotify( const QValueList<long>& IDs, const QValueList<KURL>& urls )
{
  kdDebug() << "ResourceExchange::slotMonitorNotify()" << endl;

  QString result;
  KPIM::ExchangeMonitor::IDList::ConstIterator it;
  for ( it = IDs.begin(); it != IDs.end(); ++it ) {
    if ( it == IDs.begin() )
      result += QString::number( (*it) );
    else
      result += "," + QString::number( (*it) );
  }
  kdDebug() << "Got signals for " << result << endl;
  QValueList<KURL>::ConstIterator it2;
  for ( it2 = urls.begin(); it2 != urls.end(); ++it2 ) {
    kdDebug() << "URL: " << (*it2).prettyURL() << endl;
  }

  /* Now find out what happened:
   * One or more of the following:
   * 1. Event added in period that we think we have cached
   * 2. Event deleted that we have in cache
   * 3. Event modified that we have in cache
   * 4. Something else happened that isn't relevant to us
   * Update cache, then notify whoever's watching us
   * We may be able to find (1) and (3) by looking at the
   *   DAV:getlastmodified property
   * (2) is trickier: we might have to resort to checking
   * all uids in the cache
   * Or: put monitors on every event in the cache, so that
   * we know when one gets deleted or modified
   * Only look for new events using the global monitor
   */
}

void ResourceExchange::slotMonitorError( int errorCode, const QString& moreInfo )
{
  kdError() << "Ignoring error from Exchange monitor, code=" << errorCode << "; more info: " << moreInfo << endl;
}


void ResourceExchange::addEvent(Event *anEvent)
{
    if( !mCache )
        return;
  kdDebug() << "ResourceExchange::addEvent" << endl;

  // FIXME: first check of upload finished successfully, only then
  // add to cache
  mCache->addEvent( anEvent );

  uploadEvent( anEvent );
//  insertEvent(anEvent);

  anEvent->registerObserver( this );
//  setModified( true );
}

void ResourceExchange::uploadEvent( Event* event )
{
  mClient->uploadSynchronous( event );
}

void ResourceExchange::deleteEvent(Event *event)
{
    if ( !mCache )
        return;
  kdDebug(5800) << "ResourceExchange::deleteEvent" << endl;

  mClient->removeSynchronous( event );

  // This also frees the event
  mCache->deleteEvent( event );

//  setModified( true );
}


Event *ResourceExchange::event( const QString &uid )
{
  kdDebug(5800) << "ResourceExchange::event(): " << uid << endl;

  // FIXME: Look in exchange server for uid!
  return mCache->event( uid );
}

void ResourceExchange::subscribeEvents( const QDate& start, const QDate& end )
{
  kdDebug(5800) << "ResourceExchange::subscribeEvents()" << endl;
  // FIXME: possible race condition if several subscribe events are run close
  // to each other
  mClient->download( start, end, false );
}

void ResourceExchange::downloadedEvent( KCal::Event* event, const KURL& url )
{
  kdDebug() << "Downloaded event: " << event->summary() << " from url " << url.prettyURL() << endl;
    // FIXME: add watches to the monitor for these events
    // KURL url =
    //  mMonitor->addWatch( url, KPIM::ExchangeMonitor::Update, 0 );
//    emit eventsAdded( events );
}

void ResourceExchange::slotDownloadFinished( int result, const QString& moreinfo )
{
  kdDebug() << "ResourceExchange::downloadFinished" << endl;

  if ( result != KPIM::ExchangeClient::ResultOK ) {
    // Do something useful with the error report
    kdError() << "ResourceExchange::slotDownloadFinished(): error " << result << ": " << moreinfo << endl;
  }
}

void ResourceExchange::unsubscribeEvents( const QDate& start, const QDate& end )
{
  kdDebug() << "ResourceExchange::unsubscribeEvents()" << endl;
}

void ResourceExchange::addTodo(Todo *todo)
{
    if( !mCache)
        return;
  mCache->addTodo( todo );

  todo->registerObserver( this );

//  setModified( true );
}

void ResourceExchange::deleteTodo(Todo *todo)
{
    if( !mCache )
        return;
  mCache->deleteTodo( todo );

//  setModified( true );
}

QPtrList<Todo> ResourceExchange::rawTodos() const
{
  return mCache->rawTodos();
}

Todo *ResourceExchange::todo( const QString &uid )
{
  return mCache->todo( uid );
}

QPtrList<Todo> ResourceExchange::todos( const QDate &date )
{
  return mCache->todos( date );
}

int ResourceExchange::numEvents(const QDate &qd)
{
   kdDebug() << "ResourceExchange::numEvents" << endl;
  // FIXME: look in server for number of events
  return mCache->numEvents( qd );
}


Alarm::List ResourceExchange::alarmsTo( const QDateTime &to )
{
  return mCache->alarmsTo( to );
}

Alarm::List ResourceExchange::alarms( const QDateTime &from, const QDateTime &to )
{
  kdDebug(5800) << "ResourceExchange::alarms(" << from.toString() << " - " << to.toString() << ")\n";
  return mCache->alarms( from, to );
}

/****************************** PROTECTED METHODS ****************************/

// after changes are made to an event, this should be called.
void ResourceExchange::update(IncidenceBase *incidence)
{
  Event* event = dynamic_cast<Event *>( incidence );
  if ( event ) {
    kdDebug() << "Event updated, resubmit to server..." << endl;
    uploadEvent( event );
  }
//  setModified( true );
}

// this function will take a VEvent and insert it into the event
// dictionary for the ResourceExchange.  If there is no list of events for that
// particular location in the dictionary, a new one will be created.
/*
void ResourceExchange::insertEvent(const Event *anEvent)
{
  kdDebug() << "ResourceExchange::insertEvent" << endl;

}
*/
// taking a QDate, this function will look for an eventlist in the dict
// with that date attached -
QPtrList<Event> ResourceExchange::rawEventsForDate(const QDate &qd, bool sorted)
{
  // kdDebug() << "ResourceExchange::rawEventsForDate(" << qd.toString() << "," << sorted << ")" << endl;

  // If the events for this date are not in the cache, or if they are old,
  // get them again
  QDateTime now = QDateTime::currentDateTime();
  // kdDebug() << "Now is " << now.toString() << endl;
  // kdDebug() << "mDates: " << mDates << endl;
  // kdDebug() << "mDates->contains(qd) is " << mDates->contains( qd ) << endl;
  QDate start = QDate( qd.year(), qd.month(), 1 ); // First day of month
  if ( !mDates->contains( start ) || (*mCacheDates)[start].secsTo( now ) > mCachedSeconds ) {
    QDate end = start.addMonths( 1 ).addDays( -1 ); // Last day of month
    // Get events that occur in this period from the cache
    QPtrList<Event> oldEvents = mCache->rawEvents( start, end, false );
    // And remove them all
    QPtrListIterator<Event> it( oldEvents );
    Event *event;
    while ( (event = it.current()) != 0 ) {
      ++it;
      mCache->deleteEvent( event );
    }

    kdDebug() << "Reading events for month of " << start.toString() << endl;
    mClient->downloadSynchronous( mCache, start, end, true ); // Show progress dialog
    mDates->add( start );
    mCacheDates->insert( start, now );
  }

  // Events are safely in the cache now, return them from cache
  QPtrList<Event> events = mCache->rawEventsForDate( qd, sorted );
  // kdDebug() << "Found " << events.count() << " events." << endl;
  return events;

/*
  if (!sorted) {
    return eventList;
  }

  //  kdDebug(5800) << "Sorting events for date\n" << endl;
  // now, we have to sort it based on getDtStart.time()
  QPtrList<Event> eventListSorted;
  for (anEvent = eventList.first(); anEvent; anEvent = eventList.next()) {
    if (!eventListSorted.isEmpty() &&
	anEvent->dtStart().time() < eventListSorted.at(0)->dtStart().time()) {
      eventListSorted.insert(0,anEvent);
      goto nextToInsert;
    }
    for (i = 0; (uint) i+1 < eventListSorted.count(); i++) {
      if (anEvent->dtStart().time() > eventListSorted.at(i)->dtStart().time() &&
	  anEvent->dtStart().time() <= eventListSorted.at(i+1)->dtStart().time()) {
	eventListSorted.insert(i+1,anEvent);
	goto nextToInsert;
      }
    }
    eventListSorted.append(anEvent);
  nextToInsert:
    continue;
  }
  return eventListSorted;
*/
}


QPtrList<Event> ResourceExchange::rawEvents( const QDate &start, const QDate &end,
                                          bool inclusive )
{
   kdDebug() << "ResourceExchange::rawEvents(start,end,inclusive)" << endl;
 return mCache->rawEvents( start, end, inclusive );
}

QPtrList<Event> ResourceExchange::rawEventsForDate(const QDateTime &qdt)
{
   kdDebug() << "ResourceExchange::rawEventsForDate(qdt)" << endl;
 return rawEventsForDate( qdt.date() );
}

QPtrList<Event> ResourceExchange::rawEvents()
{
   kdDebug() << "ResourceExchange::rawEvents()" << endl;
 return mCache->rawEvents();
}

void ResourceExchange::addJournal(Journal *journal)
{
  kdDebug(5800) << "Adding Journal on " << journal->dtStart().toString() << endl;
  mCache->addJournal( journal );

  journal->registerObserver( this );

//  setModified( true );
}

Journal *ResourceExchange::journal(const QDate &date)
{
    if( !mCache)
        return 0;
//  kdDebug(5800) << "ResourceExchange::journal() " << date.toString() << endl;
    return mCache->journal( date );
}

Journal *ResourceExchange::journal(const QString &uid)
{
    if( !mCache)
        return 0;
    return mCache->journal( uid );
}

QPtrList<Journal> ResourceExchange::journals()
{
  return mCache->journals();
}

#include "resourceexchange.moc"
