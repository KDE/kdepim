/*
    This file is part of libkcal.

    Copyright (c) 2003 - 2004 Tobias Koenig <tokoe@kde.org>

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

#include <stdlib.h>
#include <typeinfo>

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>

#include "libkcal/vcaldrag.h"
#include "libkcal/vcalformat.h"
#include "libkcal/icalformat.h"
#include "libkcal/exceptions.h"
#include "libkcal/incidence.h"
#include "libkcal/event.h"
#include "libkcal/todo.h"
#include "libkcal/journal.h"
#include "libkcal/filestorage.h"
#include "libkcal/alarm.h"

#include "kcal_resourcexmlrpcconfig.h"
#include "kcal_resourcexmlrpc.h"

#include "xmlrpciface.h"

#define CAL_PRIO_LOW 1
#define CAL_PRIO_NORMAL 2
#define CAL_PRIO_HIGH 3

#define CAL_RECUR_NONE 0
#define CAL_RECUR_DAILY 1
#define CAL_RECUR_WEEKLY 2
#define CAL_RECUR_MONTHLY_MDAY 3
#define CAL_RECUR_MONTHLY_WDAY 4
#define CAL_RECUR_YEARLY 5
#define CAL_SUNDAY 1
#define CAL_MONDAY 2
#define CAL_TUESDAY 4
#define CAL_WEDNESDAY 8
#define CAL_THURSDAY 16
#define CAL_FRIDAY 32
#define CAL_SATURDAY 64
#define CAL_WEEKDAYS 62
#define CAL_WEEKEND 65
#define CAL_ALLDAYS 127

#define CAL_ACCESS_READ 1
#define CAL_ACCESS_ADD 2
#define CAL_ACCESS_EDIT 4
#define CAL_ACCESS_DELETE 8
#define CAL_ACCESS_PRIVATE 9

using namespace KCal;

extern "C"
{
  void *init_kcal_xmlrpc()
  {
    return new KRES::PluginFactory<ResourceXMLRPC, ResourceXMLRPCConfig>();
  }
}

static const QString ReadEntriesObject = "calendar.bocalendar.store_to_cache";
static const QString AddEntryObject = "calendar.bocalendar.add_entry";
static const QString UpdateEntryObject = "calendar.bocalendar.update_entry";
static const QString DeleteEntryObject = "calendar.bocalendar.delete_entry";

ResourceXMLRPC::ResourceXMLRPC( const KConfig* config )
  : ResourceCalendar( config ), mServer( 0 ), mLock( 0 )
{
  if ( config )
    readConfig( config );

  init();
}

ResourceXMLRPC::ResourceXMLRPC( )
  : ResourceCalendar( 0 ), mServer( 0 ), mLock( 0 )
{
  mStartDay = 10;
  mEndDay = 20;

  init();
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  delete mServer;
  delete mLock;
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mSyncComm = false;
  mLock = new KABC::LockNull( true );

  mOpen = false;
}

void ResourceXMLRPC::readConfig( const KConfig* config )
{
  mURL = config->readEntry( "XmlRpcUrl" );
  mDomain = config->readEntry( "XmlRpcDomain", "default" );
  mUser = config->readEntry( "XmlRpcUser" );
  mPassword = KStringHandler::obscure( config->readEntry( "XmlRpcPassword" ) );
  mStartDay = config->readNumEntry( "XmlRpcStartDay", 5 );
  mEndDay = config->readNumEntry( "XmlRpcEndDay", 10 );
}

void ResourceXMLRPC::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );

  config->writeEntry( "XmlRpcUrl", mURL.url() );
  config->writeEntry( "XmlRpcDomain", mDomain );
  config->writeEntry( "XmlRpcUser", mUser );
  config->writeEntry( "XmlRpcPassword", KStringHandler::obscure( mPassword ) );
  config->writeEntry( "XmlRpcStartDay", mStartDay );
  config->writeEntry( "XmlRpcEndDay", mEndDay );
  load();
}


bool ResourceXMLRPC::doOpen()
{
  kdDebug(5800) << "ResourceXMLRPC::doOpen()" << endl;

  mOpen = true;

  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( "", this );
	mServer->setUrl( mURL );
  mServer->setUserAgent( "KDE-Calendar" );

  QMap<QString, QVariant> args;
  args.insert( "domain", mDomain );
  args.insert( "username", mUser );
  args.insert( "password", mPassword );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  enter_loop();

  return true;
}

bool ResourceXMLRPC::load()
{
  kdDebug() << "ResourceXMLRPC::load()" << endl;

  if ( !mOpen ) return true;

  mCalendar.close();

  QDateTime startDate = QDate::currentDate().addDays( mStartDay * -1 );
  QDateTime endDate = QDate::currentDate().addDays( mEndDay );

  QMap<QString, QVariant> args;
  args.insert( "start", startDate );
  args.insert( "end", endDate );

  mServer->call( ReadEntriesObject, args,
                 this, SLOT( listEntriesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  return true;
}

void ResourceXMLRPC::setURL( const KURL& url )
{
  mURL = url;
}

KURL ResourceXMLRPC::url() const
{
  return mURL;
}

void ResourceXMLRPC::setDomain( const QString& domain )
{
  mDomain = domain;
}

QString ResourceXMLRPC::domain() const
{
  return mDomain;
}

void ResourceXMLRPC::setUser( const QString& user )
{
  mUser = user;
}

QString ResourceXMLRPC::user() const
{
  return mUser;
}

void ResourceXMLRPC::setPassword( const QString& password )
{
  mPassword = password;
}

QString ResourceXMLRPC::password() const
{
  return mPassword;
}

void ResourceXMLRPC::setStartDay( int startDay )
{
  mStartDay = startDay;
}

int ResourceXMLRPC::startDay() const
{
  return mStartDay;
}

void ResourceXMLRPC::setEndDay( int endDay )
{
  mEndDay = endDay;
}

int ResourceXMLRPC::endDay() const
{
  return mEndDay;
}

bool ResourceXMLRPC::save()
{
  Event::List events = mCalendar.rawEvents();
  Event::List::Iterator evIt;

  uint counter = 0;
  for ( evIt = events.begin(); evIt != events.end(); ++evIt ) {
    if ( !(*evIt)->isReadOnly() ) {
     QMap<QString, QVariant> args;
     writeEvent( (*evIt), args );

     args.insert( "id", mUidMap[ (*evIt)->uid() ] );
     mServer->call( UpdateEntryObject, QVariant( args ),
                    this, SLOT( updateEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                    this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      counter++;
    }
  }

  if ( counter != 0 )
    enter_loop();

  return true;
}

bool ResourceXMLRPC::isSaving()
{
  return false;
}

KABC::Lock *ResourceXMLRPC::lock()
{
  return mLock;
}

void ResourceXMLRPC::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}


bool ResourceXMLRPC::addEvent( Event* ev )
{
  QMap<QString, QVariant> args;

  Event *oldEvent = mCalendar.event( ev->uid() );
  if ( oldEvent ) { // already exists
    if ( !oldEvent->isReadOnly() ) {
      writeEvent( ev, args );
      args.insert( "id", mUidMap[ ev->uid() ] );
      mServer->call( UpdateEntryObject, QVariant( args ),
                     this, SLOT( updateEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      mCalendar.addEvent( ev );
    }
  } else { // new event
    writeEvent( ev, args );
    mServer->call( AddEntryObject, QVariant( args ),
                   this, SLOT( addEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( ev->uid() ) );

    mCalendar.addEvent( ev );
  }

  return true;
}

void ResourceXMLRPC::deleteEvent( Event* ev )
{
  if ( !(mRightsMap[ ev->uid() ] & CAL_ACCESS_DELETE) )
    return;

  int id = mUidMap[ ev->uid() ].toInt();

  mServer->call( DeleteEntryObject, id,
                 this, SLOT( deleteEntryFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( ev->uid() ) );
}


Event *ResourceXMLRPC::event( const QString& uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceXMLRPC::rawEventsForDate( const QDate& qd, bool sorted )
{
  return mCalendar.rawEventsForDate( qd, sorted );
}


Event::List ResourceXMLRPC::rawEvents( const QDate& start, const QDate& end,
                                       bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceXMLRPC::rawEventsForDate( const QDateTime& qdt )
{
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceXMLRPC::rawEvents()
{
  return mCalendar.rawEvents();
}


bool ResourceXMLRPC::addTodo( Todo* )
{
  return false;
}

void ResourceXMLRPC::deleteTodo( Todo* )
{
}

Todo::List ResourceXMLRPC::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceXMLRPC::todo( const QString& uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceXMLRPC::todos( const QDate& date )
{
  return mCalendar.todos( date );
}


bool ResourceXMLRPC::addJournal( Journal* journal )
{
  return mCalendar.addJournal( journal );
}

void ResourceXMLRPC::deleteJournal( Journal* journal )
{
  mCalendar.deleteJournal( journal );
}

Journal *ResourceXMLRPC::journal( const QDate& date )
{
  return mCalendar.journal( date );
}

Journal *ResourceXMLRPC::journal( const QString& uid )
{
  return mCalendar.journal( uid );
}

Journal::List ResourceXMLRPC::journals()
{
  return mCalendar.journals();
}


Alarm::List ResourceXMLRPC::alarmsTo( const QDateTime& to )
{
  return mCalendar.alarmsTo( to );
}

Alarm::List ResourceXMLRPC::alarms( const QDateTime& from, const QDateTime& to )
{
  return mCalendar.alarms( from, to );
}

void ResourceXMLRPC::update( IncidenceBase* )
{
}

void ResourceXMLRPC::dump() const
{
  ResourceCalendar::dump();
}

void ResourceXMLRPC::reload()
{
  load();
}

void ResourceXMLRPC::loginFinished( const QValueList<QVariant>& variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  KURL url = mURL;
  if ( map[ "GOAWAY" ].toString() == "XOXO" ) { // failed
    mSessionID = mKp3 = "";
  } else {
    mSessionID = map[ "sessionid" ].toString();
    mKp3 = map[ "kp3" ].toString();
  }

  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant>& variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    kdError() << "logout failed" << endl;

  KURL url = mURL;
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  exit_loop();
}

void ResourceXMLRPC::listEntriesFinished( const QValueList<QVariant>& list,
                                          const QVariant& )
{
  QMap<QString, QString>::Iterator uidIt;
  for ( uidIt = mUidMap.begin(); uidIt != mUidMap.end(); ++uidIt ) {
    Event *event = mCalendar.event( uidIt.key() );
    mCalendar.deleteEvent( event );
  }

  mUidMap.clear();
  mRightsMap.clear();

  QMap<QString, QVariant> listMap = list[ 0 ].toMap();
  QMap<QString, QVariant>::Iterator listIt;

  for ( listIt = listMap.begin(); listIt != listMap.end(); ++listIt ) {
    QMap<QString, QVariant> entryMap = (*listIt).toMap();
    QMap<QString, QVariant>::Iterator entryIt;

    for ( entryIt = entryMap.begin(); entryIt != entryMap.end(); ++entryIt ) {
      Event *event = new Event;
      event->setFloats( false );

      readEvent( (*entryIt).toMap(), event );

      mCalendar.addEvent( event );
    }
  }

  exit_loop();

  emit resourceChanged( this );
}

void ResourceXMLRPC::deleteEntryFinished( const QValueList<QVariant>& list,
                                          const QVariant& id )
{
  QMap<QString, QVariant> map = list[ 0 ].toMap();

  if ( map[ "0" ].toInt() == 16 ) {
    mUidMap.erase( id.toString() );
    mRightsMap.erase( id.toString() );

    Event *ev = mCalendar.event( id.toString() );
    mCalendar.deleteEvent( ev );

    emit resourceChanged( this );
  } else
    kdError() << "Unable to delete event!" << endl;

  exit_loop();
}

void ResourceXMLRPC::updateEntryFinished( const QValueList<QVariant>&,
                                          const QVariant& )
{
  exit_loop();
}

void ResourceXMLRPC::addEntryFinished( const QValueList<QVariant>& list,
                                       const QVariant& id )
{
  QMap<QString, QVariant> map = list[ 0 ].toMap();
  QString uid = map[ "0" ].toString();

  mUidMap.insert( id.toString(), uid );

  exit_loop();
}

void ResourceXMLRPC::fault( int error, const QString& errorMsg,
                            const QVariant& )
{
  kdError() << "Server send error " << error << ": " << errorMsg << endl;
  exit_loop();
}

void ResourceXMLRPC::readEvent( const QMap<QString, QVariant> &args, Event *event )
{
  // for recurrence
  int rType = CAL_RECUR_NONE;
  int rInterval = 1;
  int rData = 0;
  int rights = 0;
  QDateTime rEndDate;
  QValueList<QDateTime> rExceptions;

  QMap<QString, QVariant>::ConstIterator it;
  for ( it = args.begin(); it != args.end(); ++it ) {
    if ( it.key() == "id" ) {
      mUidMap.insert( event->uid(), it.data().toString() );
    } else if ( it.key() == "rights" ) {
      rights = it.data().toInt();
    } else if ( it.key() == "start" ) {
      event->setDtStart( it.data().toDateTime() );
    } else if ( it.key() == "end" ) {
      event->setDtEnd( it.data().toDateTime() );
      event->setHasEndDate( true );
    } else if ( it.key() == "modtime" ) {
      event->setLastModified( it.data().toDateTime() );
    } else if ( it.key() == "title" ) {
      event->setSummary( it.data().toString() );
    } else if ( it.key() == "description" ) {
      event->setDescription( it.data().toString() );
    } else if ( it.key() == "location" ) {
      event->setLocation( it.data().toString() );
    } else if ( it.key() == "public" ) {
      if ( it.data().toInt() == 1 )
        event->setSecrecy( Incidence::SecrecyPublic );
      else
        event->setSecrecy( Incidence::SecrecyPrivate );
    } else if ( it.key() == "category" ) {
      QMap<QString, QVariant> categories = it.data().toMap();
      QMap<QString, QVariant>::Iterator catIt;

      for ( catIt = categories.begin(); catIt != categories.end(); ++catIt )
        mCategoryMap.insert( catIt.data().toString(), catIt.key().toInt() );

      event->setCategories( mCategoryMap.keys() );
    } else if ( it.key() == "priority" ) {
      int priority = 0;

      switch( it.data().toInt() ) {
        case CAL_PRIO_LOW:
          priority = 10;
          break;
        case CAL_PRIO_NORMAL:
          priority = 5;
          break;
        case CAL_PRIO_HIGH:
          priority = 1;
      }

      event->setPriority( priority );
    } else if ( it.key() == "recur_type" ) {
      rType = it.data().toInt();
    } else if ( it.key() == "recur_interval" ) {
      rInterval = it.data().toInt();
    } else if ( it.key() == "recur_enddate" ) {
      rEndDate = it.data().toDateTime();
    } else if ( it.key() == "recur_data" ) {
      rData = it.data().toInt();
    } else if ( it.key() == "recur_exception" ) {
      QMap<QString, QVariant> dateList;
      QMap<QString, QVariant>::Iterator dateIt;

      for ( dateIt = dateList.begin(); dateIt != dateList.end(); ++dateIt )
        rExceptions.append( (*dateIt).toDateTime() );
    } else if ( it.key() == "participants" ) {
      QMap<QString, QVariant> persons = it.data().toMap();
      QMap<QString, QVariant>::Iterator personsIt;

      for ( personsIt = persons.begin(); personsIt != persons.end(); ++personsIt ) {
        QMap<QString, QVariant> person = (*personsIt).toMap();
        Attendee::PartStat status = Attendee::InProcess;
        if ( person[ "status" ] == "A" )
          status = Attendee::Accepted;
        else if ( person[ "status" ] == "R" )
          status = Attendee::Declined;
        else if ( person[ "status" ] == "T" )
          status = Attendee::Tentative;
        else if ( person[ "status" ] == "N" )
          status = Attendee::InProcess;

        Attendee *attendee = new Attendee( person[ "name" ].toString(),
                                           person[ "email" ].toString(),
                                           false, status );
        attendee->setUid( personsIt.key() );
        event->addAttendee( attendee );
      }
    } else if ( it.key() == "alarm" ) {
      QMap<QString, QVariant> alarmList = it.data().toMap();
      QMap<QString, QVariant>::Iterator alarmIt;

      for ( alarmIt = alarmList.begin(); alarmIt != alarmList.end(); ++alarmIt ) {
        QMap<QString, QVariant> alarm = (*alarmIt).toMap();

        Alarm *vAlarm = event->newAlarm();
        vAlarm->setText( event->summary() );
        vAlarm->setTime( alarm[ "time" ].toDateTime() );
        vAlarm->setStartOffset( alarm[ "offset" ].toInt() );
        vAlarm->setEnabled( alarm[ "enabled" ].toBool() );
      }
    }
  }

  if ( rType != CAL_RECUR_NONE ) {
    Recurrence *re = event->recurrence();
    re->setRecurStart( event->dtStart() );

    QBitArray weekMask( 7 );
    weekMask.setBit( 0, rData & CAL_MONDAY );
    weekMask.setBit( 1, rData & CAL_TUESDAY );
    weekMask.setBit( 2, rData & CAL_WEDNESDAY );
    weekMask.setBit( 3, rData & CAL_THURSDAY );
    weekMask.setBit( 4, rData & CAL_FRIDAY );
    weekMask.setBit( 5, rData & CAL_SATURDAY );
    weekMask.setBit( 6, rData & CAL_SUNDAY );

    if ( rInterval == 0 ) // libkcal crashes with rInterval == 0
      rInterval = 1;

    switch ( rType ) {
      case CAL_RECUR_DAILY:
        if ( rEndDate.date().isValid() )
          re->setDaily( rInterval, rEndDate.date() );
        else
          re->setDaily( rInterval, -1 );
        break;
      case CAL_RECUR_WEEKLY:
        if ( rEndDate.date().isValid() )
          re->setWeekly( rInterval, weekMask, rEndDate.date() );
        else
          re->setWeekly( rInterval, weekMask, -1 );
        break;
      case CAL_RECUR_MONTHLY_MDAY:
        if ( rEndDate.date().isValid() )
          re->setMonthly( Recurrence::rMonthlyPos, rInterval, rEndDate.date() );
        else
          re->setMonthly( Recurrence::rMonthlyPos, rInterval, -1 );
        break;
      case CAL_RECUR_MONTHLY_WDAY:
        if ( rEndDate.date().isValid() )
          re->setMonthly( Recurrence::rMonthlyDay, rInterval, rEndDate.date() );
        else
          re->setMonthly( Recurrence::rMonthlyDay, rInterval, -1 );
        break;
      case CAL_RECUR_YEARLY:
        if ( rEndDate.date().isValid() )
          re->setYearly( Recurrence::rYearlyDay, rInterval, rEndDate.date() );
        else
          re->setYearly( Recurrence::rYearlyDay, rInterval, -1 );
        break;
    }

    QValueList<QDateTime>::Iterator exIt;
    for ( exIt = rExceptions.begin(); exIt != rExceptions.end(); ++exIt )
      event->addExDateTime( *exIt );
  }

  event->setReadOnly( !(rights & CAL_ACCESS_EDIT) );

  mRightsMap.insert( event->uid(), rights );
}

void ResourceXMLRPC::writeEvent( Event *event, QMap<QString, QVariant> &args )
{
  args.insert( "start", event->dtStart() );
  args.insert( "end", event->dtEnd() );
  args.insert( "modtime", event->lastModified() );
  args.insert( "title", event->summary() );
  args.insert( "description", event->description() );
  args.insert( "location", event->location() );

  // SECRECY
  if ( event->secrecy() == Incidence::SecrecyPublic )
    args.insert( "public", int( 1 ) );
  else
    args.insert( "public", int( 0 ) );
  
  // CATEGORY
  QStringList categories = event->categories();
  QStringList::Iterator catIt;
  QMap<QString, QVariant> catMap;
  int counter = 0;
  for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
    QMap<QString, int>::Iterator it = mCategoryMap.find( *catIt );
    if ( it == mCategoryMap.end() ) // new category
      catMap.insert( QString::number( counter-- ), *catIt );
    else
      catMap.insert( it.key(), *catIt );
  }
  args.insert( "category", catMap );

  // PRIORITY
  int priority = 0;
  if ( event->priority() == 1 )
    priority = CAL_PRIO_HIGH;
  else if ( event->priority() > 1 && event->priority() <= 5 ) 
    priority = CAL_PRIO_NORMAL;
  else
    priority = CAL_PRIO_LOW;

  args.insert( "priority", priority );

  // RECURRENCY
  Recurrence *rec = event->recurrence();
  if ( rec->doesRecur() == Recurrence::rNone ) {
    args.insert( "recur_type", int( 0 ) );
    args.insert( "recur_interval", int( 0 ) );
    args.insert( "recur_enddate", QDateTime() );
    args.insert( "recur_data", int( 0 ) );
    args.insert( "recur_exception", QMap<QString, QVariant>() );
  } else {
    if ( rec->doesRecur() == Recurrence::rDaily ) {
      args.insert( "recur_type", int( CAL_RECUR_DAILY ) );
    } else if ( rec->doesRecur() == Recurrence::rWeekly ) {
      int weekMask = 0;
      if ( rec->days().testBit( 0 ) )
        weekMask += CAL_MONDAY;
      else if ( rec->days().testBit( 1 ) )
        weekMask += CAL_TUESDAY;
      else if ( rec->days().testBit( 2 ) )
        weekMask += CAL_WEDNESDAY;
      else if ( rec->days().testBit( 3 ) )
        weekMask += CAL_THURSDAY;
      else if ( rec->days().testBit( 4 ) )
        weekMask += CAL_FRIDAY;
      else if ( rec->days().testBit( 5 ) )
        weekMask += CAL_SATURDAY;
      else if ( rec->days().testBit( 6 ) )
        weekMask += CAL_SUNDAY;

      args.insert( "recur_data", weekMask );
      args.insert( "recur_type", int( CAL_RECUR_WEEKLY ) );
    } else if ( rec->doesRecur() == Recurrence::rMonthlyPos ) {
      args.insert( "recur_type", int( CAL_RECUR_MONTHLY_MDAY ) );
    } else if ( rec->doesRecur() == Recurrence::rMonthlyDay ) {
      args.insert( "recur_type", int( CAL_RECUR_MONTHLY_WDAY ) );
    } else if ( rec->doesRecur() == Recurrence::rYearlyDay ) {
      args.insert( "recur_type", int( CAL_RECUR_YEARLY ) );
    }

    args.insert( "recur_interval", rec->frequency() );
    args.insert( "recur_enddate", rec->endDateTime() );

    QValueList<QDateTime> dates = event->exDateTimes();
    QValueList<QDateTime>::Iterator dateIt;
    QMap<QString, QVariant> exMap;
    int counter = 0;
    for ( dateIt = dates.begin(); dateIt != dates.end(); ++dateIt, ++counter )
      exMap.insert( QString::number( counter ), *dateIt );

    args.insert( "recur_exception", exMap );
  }

  // PARTICIPANTS
  Attendee::List attendees = event->attendees();
  Attendee::List::Iterator attIt;
  QMap<QString, QVariant> persons;
  for ( attIt = attendees.begin(); attIt != attendees.end(); ++attIt ) {
    QMap<QString, QVariant> person;
    QString status;

    if ( (*attIt)->status() == Attendee::Accepted )
      status = "A";
    else if ( (*attIt)->status() == Attendee::Declined )
      status = "R";
    else if ( (*attIt)->status() == Attendee::Tentative )
      status = "T";
    else
      status = "N";

    person.insert( "status", status );
    person.insert( "name", (*attIt)->name() );
    person.insert( "email", (*attIt)->email() );

    persons.insert( (*attIt)->uid(), person );
  }
  args.insert( "participants", persons );

  // ALARMS
  Alarm::List alarms = event->alarms();
  Alarm::List::Iterator alarmIt;
  QMap<QString, QVariant> alarmMap;
  for ( alarmIt = alarms.begin(); alarmIt != alarms.end(); ++alarmIt ) {
    QMap<QString, QVariant> alarm;
    alarm.insert( "time", (*alarmIt)->time() );
    alarm.insert( "offset", (*alarmIt)->startOffset().asSeconds() );
    alarm.insert( "enabled", ( (*alarmIt)->enabled() ? int( 1 ) : int( 0 ) ) );

    alarmMap.insert( "id", alarm ); // that sucks...
  }

  args.insert( "alarm", alarmMap );
}


void qt_enter_modal( QWidget* widget );
void qt_leave_modal( QWidget* widget );

void ResourceXMLRPC::enter_loop()
{
  QWidget dummy( 0, 0, WType_Dialog | WShowModal );
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal( &dummy );
  mSyncComm = true;
  qApp->enter_loop();
  qt_leave_modal( &dummy );
}

void ResourceXMLRPC::exit_loop()
{
  if ( mSyncComm ) {
    mSyncComm = false;
    qApp->exit_loop();
  }
}

#include "kcal_resourcexmlrpc.moc"
