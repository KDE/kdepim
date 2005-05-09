/*
    This file is part of kdepim.

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
#include <qtimer.h>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>

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

#include "uidmapper.h"

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
#define CAL_ACCESS_ALL 15

using namespace KCal;

extern "C"
{
  void *init_kcal_xmlrpc()
  {
    return new KRES::PluginFactory<ResourceXMLRPC, ResourceXMLRPCConfig>();
  }
}

static const QString SearchEventsCommand = "calendar.bocalendar.search";
static const QString AddEventCommand = "calendar.bocalendar.write";
static const QString DeleteEventCommand = "calendar.bocalendar.delete";
static const QString LoadEventCategoriesCommand = "calendar.bocalendar.categories";

static const QString SearchTodosCommand = "infolog.boinfolog.search";
static const QString AddTodoCommand = "infolog.boinfolog.write";
static const QString DeleteTodoCommand = "infolog.boinfolog.delete";
static const QString LoadTodoCategoriesCommand = "infolog.boinfolog.categories";

ResourceXMLRPC::ResourceXMLRPC( const KConfig* config )
  : ResourceCalendar( config ), mServer( 0 ), mLock( 0 )
{
  if ( config )
    readConfig( config );
  else
    mDomain = "default";

  init();
}

ResourceXMLRPC::ResourceXMLRPC( )
  : ResourceCalendar( 0 ), mServer( 0 ), mLock( 0 )
{
  init();
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  mEventUidMapper->store();
  delete mEventUidMapper;
  mEventUidMapper = 0;

  mTodoUidMapper->store();
  delete mTodoUidMapper;
  mTodoUidMapper = 0;

  delete mServer;
  delete mLock;
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mEventUidMapper = new UIDMapper( locateLocal( "data", "kcal/egroupware_cache/" + mURL.host() + "/event_cache.dat" ) );
  mEventUidMapper->load();

  mTodoUidMapper = new UIDMapper( locateLocal( "data", "kcal/egroupware_cache/" + mURL.host() + "/todo_cache.dat" ) );
  mTodoUidMapper->load();

  mSyncComm = false;
  mLock = new KABC::LockNull( true );

  mOpen = false;

  mQueueTimer = new QTimer( this );
}

void ResourceXMLRPC::readConfig( const KConfig* config )
{
  mURL = config->readEntry( "XmlRpcUrl" );
  mDomain = config->readEntry( "XmlRpcDomain", "default" );
  mUser = config->readEntry( "XmlRpcUser" );
  mPassword = KStringHandler::obscure( config->readEntry( "XmlRpcPassword" ) );
}

void ResourceXMLRPC::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );

  config->writeEntry( "XmlRpcUrl", mURL.url() );
  config->writeEntry( "XmlRpcDomain", mDomain );
  config->writeEntry( "XmlRpcUser", mUser );
  config->writeEntry( "XmlRpcPassword", KStringHandler::obscure( mPassword ) );
}


bool ResourceXMLRPC::doOpen()
{
  kdDebug(5800) << "ResourceXMLRPC::doOpen()" << endl;

  mOpen = true;

  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( KURL(), this );
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

  mQueueTimer->start( 3000 );
  connect( mQueueTimer, SIGNAL( timeout() ), this, SLOT( processQueue() ) );

  return true;
}

bool ResourceXMLRPC::doLoad()
{
  kdDebug() << "ResourceXMLRPC::load()" << endl;

  if ( !mOpen ) return true;

  mCalendar.close();

  QMap<QString, QVariant> args, columns;
  columns.insert( "type", "task" );
  args.insert( "filter", "none" );
  args.insert( "col_filter", columns );
  args.insert( "order", "id_parent" );

  mServer->call( SearchTodosCommand, args,
                 this, SLOT( listTodosFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mServer->call( LoadEventCategoriesCommand, QVariant( QMap<QString, QVariant>() ),
                 this, SLOT( loadEventCategoriesFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mServer->call( LoadTodoCategoriesCommand, QVariant( false, 0 ),
                 this, SLOT( loadTodoCategoriesFinished( const QValueList<QVariant>&, const QVariant& ) ),
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

bool ResourceXMLRPC::doSave()
{
  Event::List events = mCalendar.rawEvents();
  Event::List::Iterator evIt;

  uint counter = 0;
  for ( evIt = events.begin(); evIt != events.end(); ++evIt ) {
    if ( !(*evIt)->isReadOnly() ) {
      QMap<QString, QVariant> args;
      writeEvent( (*evIt), args );

      args.insert( "id", mEventUidMapper->remoteUid( (*evIt)->uid() ) );
      mServer->call( AddEventCommand, QVariant( args ),
                     this, SLOT( updateEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
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

  bool added = false;
  Event *oldEvent = mCalendar.event( ev->uid() );
  if ( oldEvent ) { // already exists
    if ( !oldEvent->isReadOnly() ) {
      writeEvent( ev, args );
      args.insert( "id", mEventUidMapper->remoteUid( ev->uid() ) );
      mServer->call( AddEventCommand, QVariant( args ),
                     this, SLOT( updateEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      mCalendar.addEvent( ev );
      added = true;
    }
  } else { // new event
    writeEvent( ev, args );
    mServer->call( AddEventCommand, QVariant( args ),
                   this, SLOT( addEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( ev->uid() ) );

    mCalendar.addEvent( ev );
    added = true;
  }

  if ( added )
    enter_loop();

  return true;
}

void ResourceXMLRPC::deleteEvent( Event* ev )
{
  if ( !(mRightsMap[ ev->uid() ] & CAL_ACCESS_DELETE) )
    return;

  QString id = mEventUidMapper->remoteUid( ev->uid() );

  mServer->call( DeleteEventCommand, id,
                 this, SLOT( deleteEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( ev->uid() ) );
  enter_loop();
}


Event *ResourceXMLRPC::event( const QString& uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceXMLRPC::rawEventsForDate( const QDate& qd, bool sorted )
{
  addToQueue( qd );
  return mCalendar.rawEventsForDate( qd, sorted );
}


Event::List ResourceXMLRPC::rawEvents( const QDate& start, const QDate& end,
                                       bool inclusive )
{
  addToQueue( start, end );
  return mCalendar.rawEvents( start, end, inclusive );
}

Event::List ResourceXMLRPC::rawEventsForDate( const QDateTime& qdt )
{
  addToQueue( qdt.date() );
  return mCalendar.rawEventsForDate( qdt.date() );
}

Event::List ResourceXMLRPC::rawEvents()
{
  return mCalendar.rawEvents();
}


bool ResourceXMLRPC::addTodo( Todo *todo )
{
  QMap<QString, QVariant> args;

  writeTodo( todo, args );
  mServer->call( AddTodoCommand, QVariant( args ),
                 this, SLOT( addTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( todo->uid() ) );

  mCalendar.addTodo( todo );

  enter_loop();

  return true;
}

void ResourceXMLRPC::deleteTodo( Todo *todo )
{
  QString id = mTodoUidMapper->remoteUid( todo->uid() );

  mServer->call( DeleteTodoCommand, id,
                 this, SLOT( deleteTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( todo->uid() ) );
  enter_loop();
}

Todo::List ResourceXMLRPC::rawTodos()
{
  return mCalendar.rawTodos();
}

Todo *ResourceXMLRPC::todo( const QString& uid )
{
  return mCalendar.todo( uid );
}

Todo::List ResourceXMLRPC::rawTodosForDate( const QDate& date )
{
  return mCalendar.rawTodosForDate( date );
}

// FIXME: This function isn't called anymore. Use an Observer instead, or write
// the data in the save() function.
void ResourceXMLRPC::changeIncidence( Incidence *incidence )
{
  if ( incidence->type() != "Todo" )
    return;

  Todo *todo = dynamic_cast<Todo*>( incidence );
  if ( !todo )
    return;

  QMap<QString, QVariant> args;

  Todo *oldTodo = mCalendar.todo( todo->uid() );
  if ( !oldTodo->isReadOnly() ) {
    writeTodo( todo, args );
    args.insert( "id", mTodoUidMapper->remoteUid( todo->uid() ) );
    mServer->call( AddTodoCommand, QVariant( args ),
                   this, SLOT( updateTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ) );
    mCalendar.addTodo( todo );
    enter_loop();
  }
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

void ResourceXMLRPC::dump() const
{
  ResourceCalendar::dump();
}

void ResourceXMLRPC::reload()
{
  load();
}

void ResourceXMLRPC::processQueue()
{
  if ( mDateQueue.isEmpty() && mDateRangeQueue.isEmpty() )
    return;

  mQueueTimer->stop();

  QValueList< QPair<QDate, QDate> >::Iterator it;
  for ( it = mDateRangeQueue.begin(); it != mDateRangeQueue.end(); ++it ) {
    QMap<QString, QVariant> args;
    args.insert( "start", QDateTime( (*it).first ) );
    args.insert( "end", QDateTime( (*it).second ) );

    mServer->call( SearchEventsCommand, args,
                   this, SLOT( listEventsFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ) );
  }

  QValueList<QDate>::Iterator dateIt;
  for ( dateIt = mDateQueue.begin(); dateIt != mDateQueue.end(); ++dateIt ) {
    QMap<QString, QVariant> args;
    args.insert( "start", QDateTime( *dateIt ) );
    args.insert( "end", QDateTime( *dateIt ) );

    mServer->call( SearchEventsCommand, args,
                   this, SLOT( listEventsFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ) );
  }

  mDateRangeQueue.clear();
  mDateQueue.clear();

  mQueueTimer->start( 1000 * 10 ); // process queue every 10 second
}

void ResourceXMLRPC::addToQueue( const QDate &date )
{
  QValueList< QPair<QDate, QDate> >::Iterator it;
  for ( it = mDateRangeQueue.begin(); it != mDateRangeQueue.end(); ++it ) {
    if ( date >= (*it).first && date <= (*it).second ) // alread in existing range
      return;

    if ( date.addDays( 1 ) == (*it).first ) {
      (*it).first = date;
      return;
    } else if ( (*it).second.addDays( 1 ) == date ) {
      (*it).second = date;
      return;
    }
  }

  if ( mDateQueue.find( date ) != mDateQueue.end() )
    return;

  QValueList<QDate>::Iterator dateIt;
  for ( dateIt = mDateQueue.begin(); dateIt != mDateQueue.end(); ++dateIt ) {
    if ( date < (*dateIt ) ) {
      if ( date.addDays( 8 ) >= (*dateIt) ) {
        addToQueue( date, *dateIt );
        mDateQueue.remove( dateIt );
        return;
      }
    } else {
      if ( date.addDays( -8 ) <= (*dateIt) ) {
        addToQueue( *dateIt, date );
        mDateQueue.remove( dateIt );
        return;
      }
    }
  }

  mDateQueue.append( date );
}

void ResourceXMLRPC::addToQueue( const QDate &start, const QDate &end )
{
  QValueList< QPair<QDate, QDate> >::Iterator it;
  for ( it = mDateRangeQueue.begin(); it != mDateRangeQueue.end(); ++it ) {
    if ( (start >= (*it).first && start <= (*it).second) &&
         (end >= (*it).first && end <= (*it).second) ) { // already included
      return;
    } else if ( (start >= (*it).first && start <= (*it).second) &&
                (end > (*it).second) ) { // overlaps
      (*it).second = end;
      return;
    } else if ( (start < (*it).first) &&
                (end >= (*it).first && end <= (*it).second) ) { // overlaps
      (*it).first = start;
      return;
    } else if ( ((*it).first >= start && (*it).first <= end) &&
                ((*it).second >= start && (*it).second <= end) ) { // absorbs
      (*it).first = start;
      (*it).second = end;
      return;
    }
  }

  mDateRangeQueue.append( qMakePair( start, end ) );
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

void ResourceXMLRPC::listEventsFinished( const QValueList<QVariant>& list,
                                         const QVariant& )
{
  QValueList<QVariant> eventList = list[ 0 ].toList();
  QValueList<QVariant>::Iterator eventIt;

  bool changed = false;
  for ( eventIt = eventList.begin(); eventIt != eventList.end(); ++eventIt ) {
    QMap<QString, QVariant> map = (*eventIt).toMap();

    Event *event = new Event;
    event->setFloats( false );

    QString uid;
    readEvent( map, event, uid );

    // do we already have this event?
    Event *oldEvent = 0;
    QString localUid = mEventUidMapper->localUid( uid );
    if ( !localUid.isEmpty() )
      oldEvent = mCalendar.event( localUid );

    if ( oldEvent ) {
      event->setUid( oldEvent->uid() );
      event->setCreated( oldEvent->created() );

      if ( !(*oldEvent == *event) ) {
        mCalendar.deleteEvent( oldEvent );
        mCalendar.addEvent( event );
        changed = true;
      } else
        delete event;
    } else {
      if ( !localUid.isEmpty() )
        event->setUid( localUid );
      mEventUidMapper->add( event->uid(), uid );
      mCalendar.addEvent( event );
      changed = true;
    }
  }

  if ( changed )
    emit resourceChanged( this );
}

void ResourceXMLRPC::deleteEventFinished( const QValueList<QVariant>&,
                                          const QVariant& id )
{
  mEventUidMapper->removeByLocal( id.toString() );
  mRightsMap.erase( id.toString() );

  Event *ev = mCalendar.event( id.toString() );
  mCalendar.deleteEvent( ev );

  exit_loop();
}

void ResourceXMLRPC::updateEventFinished( const QValueList<QVariant>&,
                                          const QVariant& )
{
  exit_loop();
}

void ResourceXMLRPC::addEventFinished( const QValueList<QVariant>& list,
                                       const QVariant& id )
{
  mEventUidMapper->add( id.toString(), list[ 0 ].toString() );
  mRightsMap[ id.toString() ] = CAL_ACCESS_ALL;

  exit_loop();
}

void ResourceXMLRPC::loadEventCategoriesFinished( const QValueList<QVariant> &mapList, const QVariant& )
{
  mEventCategoryMap.clear();

  QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::Iterator it;

  KPimPrefs prefs( "korganizerrc" );
  for ( it = map.begin(); it != map.end(); ++it ) {
    mEventCategoryMap.insert( it.data().toString(), it.key().toInt() );

    if ( prefs.mCustomCategories.find( it.data().toString() ) == prefs.mCustomCategories.end() )
      prefs.mCustomCategories.append( it.data().toString() );
  }

  prefs.usrWriteConfig();
  prefs.config()->sync();
}

void ResourceXMLRPC::listTodosFinished( const QValueList<QVariant>& list,
                                        const QVariant& )
{
  QValueList<QVariant> todoList = list[ 0 ].toList();
  QValueList<QVariant>::Iterator todoIt;

  bool changed = false;
  for ( todoIt = todoList.begin(); todoIt != todoList.end(); ++todoIt ) {
    QMap<QString, QVariant> map = (*todoIt).toMap();

    Todo *todo = new Todo;

    QString uid;
    readTodo( map, todo, uid );

    // do we already have this todo?
    Todo *oldTodo = 0;
    QString localUid = mTodoUidMapper->localUid( uid );
    if ( !localUid.isEmpty() )
      oldTodo = mCalendar.todo( localUid );

    if ( oldTodo ) {
      todo->setUid( oldTodo->uid() );
      todo->setCreated( oldTodo->created() );

      if ( !(*oldTodo == *todo) ) {
        mCalendar.deleteTodo( oldTodo );
        mCalendar.addTodo( todo );
        changed = true;
      } else
        delete todo;
    } else {
      mTodoUidMapper->add( todo->uid(), uid );
      mCalendar.addTodo( todo );
      changed = true;
    }
  }

  exit_loop();

  if ( changed )
    emit resourceChanged( this );
}

void ResourceXMLRPC::deleteTodoFinished( const QValueList<QVariant>&,
                                         const QVariant& id )
{
  mTodoUidMapper->removeByLocal( id.toString() );

  Todo *todo = mCalendar.todo( id.toString() );
  mCalendar.deleteTodo( todo );

  exit_loop();

  emit resourceChanged( this );
}

void ResourceXMLRPC::updateTodoFinished( const QValueList<QVariant>&,
                                         const QVariant& )
{
  exit_loop();
}

void ResourceXMLRPC::addTodoFinished( const QValueList<QVariant>& list,
                                      const QVariant& id )
{
  mTodoUidMapper->add( id.toString(), list[ 0 ].toString() );

  exit_loop();
}

void ResourceXMLRPC::loadTodoCategoriesFinished( const QValueList<QVariant> &mapList, const QVariant& )
{
  mTodoCategoryMap.clear();

  QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::Iterator it;

  KPimPrefs prefs( "korganizerrc" );
  for ( it = map.begin(); it != map.end(); ++it ) {
    mTodoCategoryMap.insert( it.data().toString(), it.key().toInt() );

    if ( prefs.mCustomCategories.find( it.data().toString() ) == prefs.mCustomCategories.end() )
      prefs.mCustomCategories.append( it.data().toString() );
  }

  prefs.usrWriteConfig();
  prefs.config()->sync();
}

void ResourceXMLRPC::fault( int error, const QString& errorMsg,
                            const QVariant& )
{
  kdError() << "Server send error " << error << ": " << errorMsg << endl;
  exit_loop();
}

void ResourceXMLRPC::readEvent( const QMap<QString, QVariant> &args, Event *event,
                                QString &uid )
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
      uid = it.data().toString();
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
    } else if ( it.key() == "access" ) {
      event->setSecrecy( (it.data().toString() == "public" ?
                          Incidence::SecrecyPublic : Incidence::SecrecyPrivate) );
    } else if ( it.key() == "category" ) {
      QMap<QString, QVariant> categories = it.data().toMap();
      QMap<QString, QVariant>::Iterator catIt;

      for ( catIt = categories.begin(); catIt != categories.end(); ++catIt )
        mEventCategoryMap.insert( catIt.data().toString(), catIt.key().toInt() );

      event->setCategories( mEventCategoryMap.keys() );
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
  args.insert( "access", (event->secrecy() == Incidence::SecrecyPublic ? "public" : "private") );

  // CATEGORY
  QStringList categories = event->categories();
  QStringList::Iterator catIt;
  QMap<QString, QVariant> catMap;
  int counter = 0;
  for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
    QMap<QString, int>::Iterator it = mEventCategoryMap.find( *catIt );
    if ( it == mEventCategoryMap.end() ) // new category
      catMap.insert( QString::number( counter-- ), *catIt );
    else
      catMap.insert( QString::number( it.data() ), *catIt );
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
      if ( rec->days().testBit( 1 ) )
        weekMask += CAL_TUESDAY;
      if ( rec->days().testBit( 2 ) )
        weekMask += CAL_WEDNESDAY;
      if ( rec->days().testBit( 3 ) )
        weekMask += CAL_THURSDAY;
      if ( rec->days().testBit( 4 ) )
        weekMask += CAL_FRIDAY;
      if ( rec->days().testBit( 5 ) )
        weekMask += CAL_SATURDAY;
      if ( rec->days().testBit( 6 ) )
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

void ResourceXMLRPC::writeTodo( Todo* todo, QMap<QString, QVariant>& args )
{
  args.insert( "subject", todo->summary() );
  args.insert( "des", todo->description() );
  args.insert( "access",
               (todo->secrecy() == Todo::SecrecyPublic ? "public" : "private" ) );

  // CATEGORIES
  QMap<QString, QVariant> catMap;

  QStringList categories = todo->categories();
  QStringList::Iterator catIt;
  int counter = 0;
  for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
    QMap<QString, int>::Iterator it = mTodoCategoryMap.find( *catIt );
    if ( it == mTodoCategoryMap.end() )
      catMap.insert( QString::number( counter-- ), *catIt );
    else
      catMap.insert( QString::number( it.data() ), *catIt );
  }
  args.insert( "category", catMap );

  args.insert( "datemodified", todo->lastModified() );
  args.insert( "startdate", todo->dtStart() );
  args.insert( "enddate", todo->dtDue() );

  // SUBTODO
  Incidence *inc = todo->relatedTo();
  if ( inc ) {
    QString parentUid = mTodoUidMapper->remoteUid( inc->uid() );
    args.insert( "id_parent", parentUid );
  }

  // STATE
  QString statusDesc = mTodoStateMap[ todo->uid() ];
  int status = todo->percentComplete();
  if ( status == 0 )
    statusDesc = (statusDesc == "offer" ? "offer" : "0%");
  else if ( status == 50 )
    statusDesc = (statusDesc == "ongoing" ? "ongoing" : "50%");
  else if ( status == 100 )
    if ( statusDesc != "done" && statusDesc != "billed" )
      statusDesc = "100%";
  else
    statusDesc = QString::number( status ) + "%";

  args.insert( "status", statusDesc );
}

void ResourceXMLRPC::readTodo( const QMap<QString, QVariant>& args, Todo *todo, QString &uid )
{
  uid = args[ "id" ].toString();

/*
  info_from
  info_addr
  info_owner
  info_responsible
  info_modifier
*/

  todo->setSummary( args[ "subject" ].toString() );
  todo->setDescription( args[ "des" ].toString() );
  todo->setSecrecy( args[ "access" ].toString() == "public" ? Todo::SecrecyPublic : Todo::SecrecyPrivate );

  // CATEGORIES
  QMap<QString, QVariant> categories = args[ "category" ].toMap();
  QMap<QString, QVariant>::Iterator it;

  for ( it = categories.begin(); it != categories.end(); ++it )
    mTodoCategoryMap.insert( it.data().toString(), it.key().toInt() );

  todo->setCategories( mTodoCategoryMap.keys() );

  todo->setLastModified( args[ "datemodified" ].toDateTime() );
  todo->setDtStart( args[ "startdate" ].toDateTime() );
  todo->setDtDue( args[ "enddate" ].toDateTime() );

  // SUBTODO
  QString parentId = args[ "id_parent" ].toString();
  if ( parentId != "0" ) { // we are a sub todo
    QString localParentUid = mTodoUidMapper->localUid( parentId );
    if ( !localParentUid.isEmpty() ) { // found parent todo
      Todo *parent = mCalendar.todo( localParentUid );
      if ( parent )
        todo->setRelatedTo( parent );
    }
  }

  // STATE
  QString status = args[ "status" ].toString();
  mTodoStateMap.insert( uid, status );

  if ( status == "offer" )
    todo->setPercentComplete( 0 );
  else if ( status == "ongoing" )
    todo->setPercentComplete( 50 );
  else if ( status == "done" || status == "billed" )
    todo->setPercentComplete( 100 );
  else {
    QString number = status.replace( "%", "" );
    todo->setPercentComplete( number.toInt() );
  }

  int rights = args[ "rights" ].toInt();
  todo->setReadOnly( !(rights & CAL_ACCESS_EDIT) );
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
