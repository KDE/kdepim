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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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

#include "kcal_egroupwareprefs.h"
#include "kcal_resourcexmlrpcconfig.h"
#include "kcal_resourcexmlrpc.h"

#include "access.h"
#include "synchronizer.h"
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

using namespace KCal;

typedef KRES::PluginFactory<ResourceXMLRPC, ResourceXMLRPCConfig> XMLRPCFactory;
K_EXPORT_COMPONENT_FACTORY( kcal_xmlrpc, XMLRPCFactory )


static const QString SearchEventsCommand = "calendar.bocalendar.search";
static const QString AddEventCommand = "calendar.bocalendar.write";
static const QString DeleteEventCommand = "calendar.bocalendar.delete";
static const QString LoadEventCategoriesCommand = "calendar.bocalendar.categories";

static const QString SearchTodosCommand = "infolog.boinfolog.search";
static const QString AddTodoCommand = "infolog.boinfolog.write";
static const QString DeleteTodoCommand = "infolog.boinfolog.delete";
static const QString LoadTodoCategoriesCommand = "infolog.boinfolog.categories";

static void setRights( Incidence *incidence, int rights )
{
  incidence->setCustomProperty( "EGWRESOURCE", "RIGHTS", QString::number( rights ) );
}

static int rights( Incidence *incidence )
{
  return incidence->customProperty( "EGWRESOURCE", "RIGHTS" ).toInt();
}

ResourceXMLRPC::ResourceXMLRPC( const KConfig* config )
  : ResourceCached( config ), mServer( 0 ), mLock( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config )
    readConfig( config );

  initEGroupware();
}

ResourceXMLRPC::ResourceXMLRPC( )
  : ResourceCached( 0 ), mServer( 0 ), mLock( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  initEGroupware();
}

ResourceXMLRPC::~ResourceXMLRPC()
{
  disableChangeNotification();

  delete mServer;
  mServer = 0;

  delete mLock;
  mLock = 0;

  delete mPrefs;
  mPrefs = 0;

  delete mSynchronizer;
  mSynchronizer = 0;
}

void ResourceXMLRPC::init()
{
  setType( "xmlrpc" );

  mTodoStateMapper.setPath( "kcal/todostatemap/" );

  mPrefs = new EGroupwarePrefs;
  mLoaded = 0;

  mLock = new KABC::LockNull( true );
  mSynchronizer = new Synchronizer();
}

void ResourceXMLRPC::initEGroupware()
{
  KURL url( mPrefs->url() );
}

void ResourceXMLRPC::readConfig( const KConfig* config )
{
  mPrefs->readConfig();

  ResourceCached::readConfig( config );
}

void ResourceXMLRPC::writeConfig( KConfig* config )
{
  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool ResourceXMLRPC::doOpen()
{
  kdDebug(5800) << "ResourceXMLRPC::doOpen()" << endl;

  if ( mServer )
    delete mServer;

  mServer = new KXMLRPC::Server( KURL(), this );
  mServer->setUrl( KURL( mPrefs->url() ) );
  mServer->setUserAgent( "KDE-Calendar" );

  QMap<QString, QVariant> args;
  args.insert( "domain", mPrefs->domain() );
  args.insert( "username", mPrefs->user() );
  args.insert( "password", mPrefs->password() );

  mServer->call( "system.login", QVariant( args ),
                 this, SLOT( loginFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mSynchronizer->start();

  return true;
}

void ResourceXMLRPC::doClose()
{
  kdDebug(5800) << "ResourceXMLRPC::doClose()" << endl;

  QMap<QString, QVariant> args;
  args.insert( "sessionid", mSessionID );
  args.insert( "kp3", mKp3 );

  mServer->call( "system.logout", QVariant( args ),
                 this, SLOT( logoutFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );

  mSynchronizer->start();
}

bool ResourceXMLRPC::doLoad()
{
  kdDebug() << "ResourceXMLRPC::load()" << endl;

  mCalendar.close();

  disableChangeNotification();
  loadCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  loadCache();
  mTodoStateMapper.setIdentifier( type() + "_" + identifier() );
  mTodoStateMapper.load();

  QMap<QString, QVariant> args, columns;
  args.insert( "start", QDateTime( QDate::currentDate().addDays( -12 ) ) );
  args.insert( "end", QDateTime( QDate::currentDate().addDays( 2000 ) ) );

  mServer->call( SearchEventsCommand, args,
                 this, SLOT( listEventsFinished( const QValueList<QVariant>&, const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ) );
  args.clear();

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

bool ResourceXMLRPC::doSave()
{
  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  saveCache();

  const Event::List events = mCalendar.rawEvents();
  Event::List::ConstIterator evIt;

  uint counter = 0;
  for ( evIt = events.begin(); evIt != events.end(); ++evIt ) {
    if ( !(*evIt)->isReadOnly() ) {
      QMap<QString, QVariant> args;
      writeEvent( (*evIt), args );

      args.insert( "id", idMapper().remoteId( (*evIt)->uid() ).toInt() );
      mServer->call( AddEventCommand, QVariant( args ),
                     this, SLOT( updateEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      counter++;
    }
  }

  const Todo::List todos = mCalendar.rawTodos();
  Todo::List::ConstIterator todoIt;

  for ( todoIt = todos.begin(); todoIt != todos.end(); ++todoIt ) {
    if ( !(*todoIt)->isReadOnly() ) {
      QMap<QString, QVariant> args;
      writeTodo( (*todoIt), args );

      args.insert( "id", idMapper().remoteId( (*todoIt)->uid() ).toInt() );
      mServer->call( AddTodoCommand, QVariant( args ),
                     this, SLOT( updateTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );
      counter++;
    }
  }

  if ( counter != 0 )
    mSynchronizer->start();

  mTodoStateMapper.save();

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


bool ResourceXMLRPC::addEvent( Event* ev )
{
  QMap<QString, QVariant> args;

  disableChangeNotification();

  setRights( ev, EGW_ACCESS_ALL );
  Event *oldEvent = mCalendar.event( ev->uid() );
  if ( oldEvent ) { // already exists
    if ( !oldEvent->isReadOnly() ) {
      writeEvent( ev, args );
      args.insert( "id", idMapper().remoteId( ev->uid() ).toInt() );
      mServer->call( AddEventCommand, QVariant( args ),
                     this, SLOT( updateEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );

      mCalendar.deleteIncidence( oldEvent );
      mCalendar.addIncidence( ev );
      saveCache();
    }
  } else { // new event
    writeEvent( ev, args );
    mServer->call( AddEventCommand, QVariant( args ),
                   this, SLOT( addEventFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( ev->uid() ) );

    mCalendar.addEvent( ev );
    saveCache();
  }

  enableChangeNotification();

  return true;
}

bool ResourceXMLRPC::deleteEvent( Event* ev )
{
  if ( !(rights( ev ) & EGW_ACCESS_DELETE) && rights( ev ) != -1 )
    return false;

  mServer->call( DeleteEventCommand, idMapper().remoteId( ev->uid() ).toInt(),
                 this, SLOT( deleteEventFinished( const QValueList<QVariant>&,
                                                  const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( ev->uid() ) );
  return true;
}


Event *ResourceXMLRPC::event( const QString& uid )
{
  return mCalendar.event( uid );
}

Event::List ResourceXMLRPC::rawEventsForDate( const QDate& qd,
                                              EventSortField sortField,
                                              SortDirection sortDirection )
{
  return mCalendar.rawEventsForDate( qd, sortField, sortDirection );
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


bool ResourceXMLRPC::addTodo( Todo *todo )
{
  QMap<QString, QVariant> args;

  disableChangeNotification();

  setRights( todo, EGW_ACCESS_ALL );
  Todo *oldTodo = mCalendar.todo( todo->uid() );
  if ( oldTodo ) { // already exists
    if ( !oldTodo->isReadOnly() ) {
      writeTodo( todo, args );
      args.insert( "id", idMapper().remoteId( todo->uid() ).toInt() );
      mServer->call( AddTodoCommand, QVariant( args ),
                     this, SLOT( updateTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                     this, SLOT( fault( int, const QString&, const QVariant& ) ) );

      mCalendar.deleteIncidence( oldTodo );
      mCalendar.addIncidence( todo );
      saveCache();
    }
  } else { // new todo
    writeTodo( todo, args );
    mServer->call( AddTodoCommand, QVariant( args ),
                   this, SLOT( addTodoFinished( const QValueList<QVariant>&, const QVariant& ) ),
                   this, SLOT( fault( int, const QString&, const QVariant& ) ),
                   QVariant( todo->uid() ) );

    mCalendar.addTodo( todo );
    saveCache();
  }

  enableChangeNotification();

  return true;
}

bool ResourceXMLRPC::deleteTodo( Todo *todo )
{
  if ( !(rights( todo ) & EGW_ACCESS_DELETE) && rights( todo ) != -1 )
    return false;

  mServer->call( DeleteTodoCommand, idMapper().remoteId( todo->uid() ).toInt(),
                 this, SLOT( deleteTodoFinished( const QValueList<QVariant>&,
                                                 const QVariant& ) ),
                 this, SLOT( fault( int, const QString&, const QVariant& ) ),
                 QVariant( todo->uid() ) );
  return true;
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

bool ResourceXMLRPC::addJournal( Journal* journal )
{
  return mCalendar.addJournal( journal );
}

bool ResourceXMLRPC::deleteJournal( Journal* journal )
{
  return mCalendar.deleteJournal( journal );
}

Journal::List ResourceXMLRPC::journals( const QDate& date )
{
  return mCalendar.journals( date );
}

Journal *ResourceXMLRPC::journal( const QString& uid )
{
  return mCalendar.journal( uid );
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


void ResourceXMLRPC::loginFinished( const QValueList<QVariant>& variant,
                                    const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  KURL url = KURL( mPrefs->url() );
  if ( map[ "GOAWAY" ].toString() == "XOXO" ) { // failed
    mSessionID = mKp3 = "";
  } else {
    mSessionID = map[ "sessionid" ].toString();
    mKp3 = map[ "kp3" ].toString();
  }

  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  mSynchronizer->stop();
}

void ResourceXMLRPC::logoutFinished( const QValueList<QVariant>& variant,
                                     const QVariant& )
{
  QMap<QString, QVariant> map = variant[ 0 ].toMap();

  if ( map[ "GOODBYE" ].toString() != "XOXO" )
    kdError() << "logout failed" << endl;

  KURL url = KURL( mPrefs->url() );
  mSessionID = mKp3 = "";
  url.setUser( mSessionID );
  url.setPass( mKp3 );
  mServer->setUrl( url );

  mSynchronizer->stop();
}

void ResourceXMLRPC::listEventsFinished( const QValueList<QVariant>& list,
                                         const QVariant& )
{
  const QValueList<QVariant> eventList = list[ 0 ].toList();
  QValueList<QVariant>::ConstIterator eventIt;

  disableChangeNotification();

  Event::List retrievedEvents;

  bool changed = false;
  for ( eventIt = eventList.begin(); eventIt != eventList.end(); ++eventIt ) {
    QMap<QString, QVariant> map = (*eventIt).toMap();

    Event *event = new Event;
    event->setFloats( false );

    QString uid;
    readEvent( map, event, uid );

    // do we already have this event?
    Event *oldEvent = 0;
    QString localUid = idMapper().localId( uid );
    if ( !localUid.isEmpty() )
      oldEvent = mCalendar.event( localUid );

    if ( oldEvent ) {
      event->setUid( oldEvent->uid() );
      event->setCreated( oldEvent->created() );

      if ( !(*oldEvent == *event) ) {
        mCalendar.deleteEvent( oldEvent );
        mCalendar.addEvent( event );
        retrievedEvents.append( event );
        changed = true;
      } else
        delete event;
    } else {
      if ( !localUid.isEmpty() )
        event->setUid( localUid );
      idMapper().setRemoteId( event->uid(), uid );
      mCalendar.addEvent( event );
      retrievedEvents.append( event );
      changed = true;
    }
  }

  enableChangeNotification();

  clearChanges();


  if ( changed ) {
    cleanUpEventCache( retrievedEvents );
    saveCache();
    emit resourceChanged( this );
  }

  checkLoadingFinished();
}

void ResourceXMLRPC::deleteEventFinished( const QValueList<QVariant>&,
                                          const QVariant& id )
{
  idMapper().removeRemoteId( idMapper().remoteId( id.toString() ) );

  Event *ev = mCalendar.event( id.toString() );

  disableChangeNotification();
  mCalendar.deleteEvent( ev );
  saveCache();
  enableChangeNotification();

  emit resourceChanged( this );
}

void ResourceXMLRPC::updateEventFinished( const QValueList<QVariant>&,
                                          const QVariant& )
{
  mSynchronizer->stop();
}

void ResourceXMLRPC::addEventFinished( const QValueList<QVariant>& list,
                                       const QVariant& id )
{
  idMapper().setRemoteId( id.toString(), list[ 0 ].toString() );

  emit resourceChanged( this );
}

void ResourceXMLRPC::loadEventCategoriesFinished( const QValueList<QVariant> &mapList, const QVariant& )
{
  mEventCategoryMap.clear();

  const QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::ConstIterator it;

  KPimPrefs prefs( "korganizerrc" );
  for ( it = map.begin(); it != map.end(); ++it ) {
    mEventCategoryMap.insert( it.data().toString(), it.key().toInt() );

    if ( prefs.mCustomCategories.find( it.data().toString() ) == prefs.mCustomCategories.end() )
      prefs.mCustomCategories.append( it.data().toString() );
  }

  prefs.usrWriteConfig();
  prefs.config()->sync();

  checkLoadingFinished();
}

void ResourceXMLRPC::listTodosFinished( const QValueList<QVariant>& list,
                                        const QVariant& )
{
  const QValueList<QVariant> todoList = list[ 0 ].toList();
  QValueList<QVariant>::ConstIterator todoIt;

  disableChangeNotification();

  Todo::List retrievedTodos;

  bool changed = false;
  for ( todoIt = todoList.begin(); todoIt != todoList.end(); ++todoIt ) {
    QMap<QString, QVariant> map = (*todoIt).toMap();

    Todo *todo = new Todo;

    QString uid;
    readTodo( map, todo, uid );

    // do we already have this todo?
    Todo *oldTodo = 0;
    QString localUid = idMapper().localId( uid );
    if ( !localUid.isEmpty() )
      oldTodo = mCalendar.todo( localUid );

    if ( oldTodo ) {
      todo->setUid( oldTodo->uid() );
      todo->setCreated( oldTodo->created() );

      if ( !(*oldTodo == *todo) ) {
        mCalendar.deleteTodo( oldTodo );
        mCalendar.addTodo( todo );
        retrievedTodos.append( todo );
        changed = true;
      } else
        delete todo;
    } else {
      idMapper().setRemoteId( todo->uid(), uid );
      mCalendar.addTodo( todo );
      retrievedTodos.append( todo );
      changed = true;
    }
  }

  enableChangeNotification();

  if ( changed ) {
    cleanUpTodoCache( retrievedTodos );
    saveCache();
    emit resourceChanged( this );
  }

  checkLoadingFinished();
}

void ResourceXMLRPC::deleteTodoFinished( const QValueList<QVariant>&,
                                         const QVariant& id )
{
  idMapper().removeRemoteId( idMapper().remoteId( id.toString() ) );
  mTodoStateMapper.remove( idMapper().remoteId( id.toString() ) );

  Todo *todo = mCalendar.todo( id.toString() );
  disableChangeNotification();
  mCalendar.deleteTodo( todo );
  saveCache();
  enableChangeNotification();

  emit resourceChanged( this );
}

void ResourceXMLRPC::addTodoFinished( const QValueList<QVariant>& list,
                                      const QVariant& id )
{
  idMapper().setRemoteId( id.toString(), list[ 0 ].toString() );

  emit resourceChanged( this );
}

void ResourceXMLRPC::updateTodoFinished( const QValueList<QVariant>&,
                                         const QVariant& )
{
  mSynchronizer->stop();
}

void ResourceXMLRPC::loadTodoCategoriesFinished( const QValueList<QVariant> &mapList, const QVariant& )
{
  mTodoCategoryMap.clear();

  const QMap<QString, QVariant> map = mapList[ 0 ].toMap();
  QMap<QString, QVariant>::ConstIterator it;

  KPimPrefs prefs( "korganizerrc" );
  for ( it = map.begin(); it != map.end(); ++it ) {
    mTodoCategoryMap.insert( it.data().toString(), it.key().toInt() );

    if ( prefs.mCustomCategories.find( it.data().toString() ) == prefs.mCustomCategories.end() )
      prefs.mCustomCategories.append( it.data().toString() );
  }

  prefs.usrWriteConfig();
  prefs.config()->sync();

  checkLoadingFinished();
}

void ResourceXMLRPC::fault( int error, const QString& errorMsg,
                            const QVariant& )
{
  kdError() << "Server send error " << error << ": " << errorMsg << endl;
  mSynchronizer->stop();
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
      QDateTime start = args[ "start" ].toDateTime();
      QDateTime end = it.data().toDateTime();
      if ( start.time() == end.time() &&
           start.time().hour() == 0 && start.time().minute() == 0 &&
           start.time().second() == 0 ) {
        event->setDtEnd( end.addDays( -1 ) );
        event->setFloats( true );
      } else {
        event->setDtEnd( end );
        event->setHasEndDate( true );
      }
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
      const QMap<QString, QVariant> categories = it.data().toMap();
      QMap<QString, QVariant>::ConstIterator catIt;

      QStringList eventCategories;
      for ( catIt = categories.begin(); catIt != categories.end(); ++catIt ) {
        mEventCategoryMap.insert( catIt.data().toString(), catIt.key().toInt() );
        eventCategories.append( catIt.data().toString() );
      }

      event->setCategories( eventCategories );
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
      const QMap<QString, QVariant> dateList = it.data().toMap();
      QMap<QString, QVariant>::ConstIterator dateIt;

      for ( dateIt = dateList.begin(); dateIt != dateList.end(); ++dateIt )
        rExceptions.append( (*dateIt).toDateTime() );
    } else if ( it.key() == "participants" ) {
      const QMap<QString, QVariant> persons = it.data().toMap();
      QMap<QString, QVariant>::ConstIterator personsIt;

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
      const QMap<QString, QVariant> alarmList = it.data().toMap();
      QMap<QString, QVariant>::ConstIterator alarmIt;

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

  if ( rType != CAL_RECUR_NONE && rInterval > 0 ) {
    Recurrence *re = event->recurrence();
//    re->setRecurStart( event->dtStart() );


    if ( rInterval == 0 ) // libkcal crashes with rInterval == 0
      rInterval = 1;

    switch ( rType ) {
      case CAL_RECUR_DAILY:
        re->setDaily( rInterval );
        break;
      case CAL_RECUR_WEEKLY: {
        QBitArray weekMask( 7 );
        weekMask.setBit( 0, rData & CAL_MONDAY );
        weekMask.setBit( 1, rData & CAL_TUESDAY );
        weekMask.setBit( 2, rData & CAL_WEDNESDAY );
        weekMask.setBit( 3, rData & CAL_THURSDAY );
        weekMask.setBit( 4, rData & CAL_FRIDAY );
        weekMask.setBit( 5, rData & CAL_SATURDAY );
        weekMask.setBit( 6, rData & CAL_SUNDAY );

        re->setWeekly( rInterval, weekMask );
        break; }
      case CAL_RECUR_MONTHLY_MDAY:
        re->setMonthly( rInterval );
        break;
      case CAL_RECUR_MONTHLY_WDAY:
        re->setMonthly( rInterval );
        // TODO: Set the correct monthly pos
        break;
      case CAL_RECUR_YEARLY:
        re->setYearly( rInterval );
        break;
    }
    if ( rEndDate.date().isValid() )
      re->setEndDate( rEndDate.date() );

    QValueList<QDateTime>::ConstIterator exIt;
    for ( exIt = rExceptions.begin(); exIt != rExceptions.end(); ++exIt )
      re->addExDateTime( *exIt );
  }

  event->setReadOnly( !(rights & EGW_ACCESS_EDIT) );
  setRights( event, rights );
}

void ResourceXMLRPC::writeEvent( Event *event, QMap<QString, QVariant> &args )
{
  args.insert( "start", event->dtStart() );

  // handle all day events
  if ( event->doesFloat() )
    args.insert( "end", event->dtEnd().addDays( 1 ) );
  else
    args.insert( "end", event->dtEnd() );

  args.insert( "modtime", event->lastModified() );
  args.insert( "title", event->summary() );
  args.insert( "description", event->description() );
  args.insert( "location", event->location() );

  // SECRECY
  args.insert( "access", (event->secrecy() == Incidence::SecrecyPublic ? "public" : "private") );

  // CATEGORY
  const QStringList categories = event->categories();
  QStringList::ConstIterator catIt;
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

  // RECURRENCE
  Recurrence *rec = event->recurrence();
  if ( !rec->doesRecur() ) {
    args.insert( "recur_type", int( 0 ) );
    args.insert( "recur_interval", int( 0 ) );
    args.insert( "recur_enddate", QDateTime() );
    args.insert( "recur_data", int( 0 ) );
    args.insert( "recur_exception", QMap<QString, QVariant>() );
  } else {
    switch ( rec->recurrenceType() ) {
      case Recurrence::rDaily:
          args.insert( "recur_type", int( CAL_RECUR_DAILY ) );
          break;
      case Recurrence::rWeekly:  {
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
          }
          break;
      case Recurrence::rMonthlyPos:
          args.insert( "recur_type", int( CAL_RECUR_MONTHLY_MDAY ) );
          break;
      case Recurrence::rMonthlyDay:
          args.insert( "recur_type", int( CAL_RECUR_MONTHLY_WDAY ) );
          break;
      case Recurrence::rYearlyDay:
          args.insert( "recur_type", int( CAL_RECUR_YEARLY ) );
          break;
      default:
          break;
    }

    args.insert( "recur_interval", rec->frequency() );
    args.insert( "recur_enddate", rec->endDateTime() );

    //  TODO: Also use exception dates!
    const QValueList<QDateTime> dates = event->recurrence()->exDateTimes();
    QValueList<QDateTime>::ConstIterator dateIt;
    QMap<QString, QVariant> exMap;
    int counter = 0;
    for ( dateIt = dates.begin(); dateIt != dates.end(); ++dateIt, ++counter )
      exMap.insert( QString::number( counter ), *dateIt );

    args.insert( "recur_exception", exMap );
  }

  // PARTICIPANTS
  const Attendee::List attendees = event->attendees();
  Attendee::List::ConstIterator attIt;
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
  const Alarm::List alarms = event->alarms();
  Alarm::List::ConstIterator alarmIt;
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

  const QStringList categories = todo->categories();
  QStringList::ConstIterator catIt;
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
    QString parentUid = idMapper().remoteId( inc->uid() );
    args.insert( "id_parent", parentUid );
  }

  // STATE
  QString remoteId = idMapper().remoteId( todo->uid() );
  QString status = mTodoStateMapper.remoteState( remoteId, todo->percentComplete() );
  args.insert( "status", status );
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
  const QMap<QString, QVariant> categories = args[ "category" ].toMap();
  QMap<QString, QVariant>::ConstIterator it;

  QStringList todoCategories;
  for ( it = categories.begin(); it != categories.end(); ++it ) {
    mTodoCategoryMap.insert( it.data().toString(), it.key().toInt() );
    todoCategories.append( it.data().toString() );
  }

  todo->setCategories( todoCategories );

  todo->setLastModified( args[ "datemodified" ].toDateTime() );

  todo->setFloats( true );
  QDateTime dateTime = args[ "startdate" ].toDateTime();
  if ( dateTime.isValid() ) {
    todo->setDtStart( dateTime );
    todo->setHasStartDate( true );
    if ( !dateTime.time().isNull() )
      todo->setFloats( false );
  }

  dateTime = args[ "enddate" ].toDateTime();
  if ( dateTime.isValid() ) {
    todo->setDtDue( dateTime );
    todo->setHasDueDate( true );
    if ( !dateTime.time().isNull() )
      todo->setFloats( false );
  }

  // SUBTODO
  QString parentId = args[ "id_parent" ].toString();
  if ( parentId != "0" ) { // we are a sub todo
    QString localParentUid = idMapper().localId( parentId );
    if ( !localParentUid.isEmpty() ) { // found parent todo
      Todo *parent = mCalendar.todo( localParentUid );
      if ( parent )
        todo->setRelatedTo( parent );
    }
  }

  // STATE
  QString status = args[ "status" ].toString();
  int state = TodoStateMapper::toLocal( status );

  mTodoStateMapper.addTodoState( uid, state, status );
  todo->setPercentComplete( state );

  int rights = args[ "rights" ].toInt();
  todo->setReadOnly( !(rights & EGW_ACCESS_EDIT) );
  setRights( todo, rights );
}

void ResourceXMLRPC::checkLoadingFinished()
{
  mLoaded++;
  if ( mLoaded == 4 ) {
    mLoaded = 0;
    emit resourceLoaded( this );
  }
}

#include "kcal_resourcexmlrpc.moc"
