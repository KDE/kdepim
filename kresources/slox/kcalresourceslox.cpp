/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <typeinfo>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>
#include <qfile.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <libkcal/vcaldrag.h>
#include <libkcal/vcalformat.h>
#include <libkcal/icalformat.h>
#include <libkcal/exceptions.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>
#include <libkcal/filestorage.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include "webdavhandler.h"

#include "kcalresourceslox.h"

KCalResourceSlox::KCalResourceSlox( const KConfig *config )
  : ResourceCached( config )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

KCalResourceSlox::KCalResourceSlox( const KURL &url )
  : ResourceCached( 0 )
{
  mDownloadUrl = url;

  init();
}

KCalResourceSlox::~KCalResourceSlox()
{
  close();

  if ( mLoadEventsJob ) mLoadEventsJob->kill();
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;
}

void KCalResourceSlox::init()
{
  mLoadEventsJob = 0;
  mLoadTodosJob = 0;
  mUploadJob = 0;

  setType( "slox" );

  mOpen = false;

  mLock = new KABC::LockNull( true );
}

void KCalResourceSlox::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "DownloadUrl" );
  mDownloadUrl = KURL( url );

  url = config->readEntry( "UploadUrl" );
  mUploadUrl = KURL( url );

  mReloadPolicy = config->readNumEntry( "ReloadPolicy", ReloadNever );

  mLastEventSync = config->readDateTimeEntry( "LastEventSync" );
  mLastTodoSync = config->readDateTimeEntry( "LastEventSync" );
}

void KCalResourceSlox::writeConfig( KConfig *config )
{
  kdDebug() << "KCalResourceSlox::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "DownloadUrl", mDownloadUrl.url() );
  config->writeEntry( "UploadUrl", mUploadUrl.url() );

  config->writeEntry( "ReloadPolicy", mReloadPolicy );

  config->writeEntry( "LastEventSync", mLastEventSync );
  config->writeEntry( "LastTodoSync", mLastTodoSync );
}

QString KCalResourceSlox::cacheFile()
{
  QString file = locateLocal( "cache", "kcal/kresources/" + identifier() );
  kdDebug() << "KCalResourceSlox::cacheFile(): " << file << endl;
  return file;
}

bool KCalResourceSlox::doOpen()
{
  kdDebug(5800) << "KCalResourceSlox::doOpen()" << endl;

  mOpen = true;

  return true;
}

bool KCalResourceSlox::load()
{
  kdDebug() << "KCalResourceSlox::load()" << endl;

  if ( !mOpen ) {
    kdWarning() << "Warning: resource not open." << endl;
    return true;
  }

  if ( mLoadEventsJob || mLoadTodosJob ) {
    kdWarning() << "KCalResourceSlox::load(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "KCalResourceSlox::load(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.close();

  mCalendar.load( cacheFile() );

  QString p = mDownloadUrl.protocol();
  if ( p != "http" && p != "https" && p != "webdav" && p != "webdavs" ) {
    mErrorMessage = i18n("Non-http protcol: '%1'").arg( p );
    kdDebug() << mErrorMessage << endl;
    return false;
  }

#if 1
  requestEvents();
#endif
  requestTodos();

  return true;
}

QString KCalResourceSlox::errorMessage()
{
  return mErrorMessage;
}

void KCalResourceSlox::requestEvents()
{
  QString url = mDownloadUrl.url() + "/servlet/webdav.calendar/";

  QString lastsync;
  if ( mLastEventSync.isValid() ) {
    lastsync = WebdavHandler::qDateTimeToSlox( mLastEventSync.addDays( -1 ) );
  } else {
    lastsync = "0";
  }

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( doc, prop, "lastsync", lastsync );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

  kdDebug() << "REQUEST CALENDAR: \n" << doc.toString( 2 ) << endl;

  mLoadEventsJob = KIO::davPropFind( url, doc, "0" );
  connect( mLoadEventsJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadEventsResult( KIO::Job * ) ) );

  mLastEventSync = QDateTime::currentDateTime();
}

void KCalResourceSlox::requestTodos()
{
  QString url = mDownloadUrl.url() + "/servlet/webdav.tasks/";

  QString lastsync;
  if ( mLastEventSync.isValid() ) {
    lastsync = WebdavHandler::qDateTimeToSlox( mLastTodoSync.addDays( -1 ) );
  } else {
    lastsync = "0";
  }

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( doc, prop, "lastsync", lastsync );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

  kdDebug() << "REQUEST TASKS: \n" << doc.toString( 2 ) << endl;

  mLoadTodosJob = KIO::davPropFind( url, doc, "0" );
  connect( mLoadTodosJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadTodosResult( KIO::Job * ) ) );

  mLastTodoSync = QDateTime::currentDateTime();
}

void KCalResourceSlox::parseIncidenceAttribute( const QDomElement &e,
                                                KCal::Incidence *incidence )
{
  if ( e.tagName() == "title" ) {
    incidence->setSummary( e.text() );
  } else if ( e.tagName() == "description" ) {
    incidence->setDescription( e.text() );
  }
}

void KCalResourceSlox::parseEventAttribute( const QDomElement &e,
                                            KCal::Event *event )
{
  if ( e.tagName() == "begins" ) {
    event->setDtStart( WebdavHandler::sloxToQDateTime( e.text() ) );
  } else if ( e.tagName() == "ends" ) {
    event->setDtEnd( WebdavHandler::sloxToQDateTime( e.text() ) );
  }
}

void KCalResourceSlox::parseTodoAttribute( const QDomElement &e,
                                           KCal::Todo *todo )
{
  if ( e.tagName() == "startdate" ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( e.text() );
    if ( dt.isValid() ) {
      todo->setDtStart( dt );
      todo->setHasStartDate( true );
    }
  } else if ( e.tagName() == "deadline" ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( e.text() );
    if ( dt.isValid() ) {
      todo->setDtDue( dt );
      todo->setHasDueDate( true );
    }
  } else if ( e.tagName() == "priority" ) {
    int p = e.text().toInt();
    if ( p < 1 || p > 3 ) {
      kdError() << "Unknown priority: " << e.text() << endl;
    } else {
      int priority;
      switch ( p ) {
        case 1:
          priority = 5;
          break;
        default:
        case 2:
          priority = 3;
          break;
        case 3:
          priority = 1;
          break;
      }
      todo->setPriority( priority );
    }
  } else if ( e.tagName() == "status" ) {
    int completed = e.text().toInt();
    todo->setPercentComplete( completed );
  }
}

void KCalResourceSlox::slotLoadTodosResult( KIO::Job *job )
{
  kdDebug() << "KCalResourceSlox::slotLoadJobResult()" << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadJobResult() success" << endl;

    QDomDocument doc = mLoadTodosJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

    bool changed = false;

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      if ( item.status == SloxItem::Delete ) {
        KCal::Todo *todo = mCalendar.todo( item.uid );
        if ( todo ) { 
          mCalendar.deleteTodo( todo );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        KCal::Todo *newTodo = 0;
        KCal::Todo *todo = mCalendar.todo( item.uid );
        if ( !todo ) {
          newTodo = new KCal::Todo;
          todo = newTodo;
          todo->setUid( item.uid );
        }

        QDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          parseIncidenceAttribute( e, todo );
          parseTodoAttribute( e, todo );
        }

        if ( newTodo ) mCalendar.addTodo( todo );

        changed = true;
      }
    }
    
    if ( changed ) emit resourceChanged( this );
  }

  mLoadTodosJob = 0;

  emit resourceLoaded( this );
}

void KCalResourceSlox::slotLoadEventsResult( KIO::Job *job )
{
  kdDebug() << "KCalResourceSlox::slotLoadEventsResult()" << endl;

  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadEventsResult() success" << endl;

    QDomDocument doc = mLoadEventsJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

    bool changed = false;

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      if ( item.status == SloxItem::Delete ) {
        KCal::Event *event = mCalendar.event( item.uid );
        if ( event ) {
          mCalendar.deleteEvent( event );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        KCal::Event *newEvent = 0;
        KCal::Event *event = mCalendar.event( item.uid );
        if ( !event ) {
          newEvent = new KCal::Event;
          event = newEvent;
          event->setUid( item.uid );
        }

        event->setFloats( false );

        QDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          parseIncidenceAttribute( e, event );
          parseEventAttribute( e, event );
        }

        kdDebug() << "EVENT " << item.uid << " " << event->summary() << endl;

        if ( newEvent ) mCalendar.addEvent( event );

        changed = true;
      }
    }
    
    if ( changed ) emit resourceChanged( this );
  }

  mLoadEventsJob = 0;

  emit resourceLoaded( this );
}

bool KCalResourceSlox::save()
{
  kdDebug() << "KCalResourceSlox::save()" << endl;

  if ( !mOpen ) return true;

  if ( readOnly() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mLoadEventsJob || mLoadTodosJob ) {
    kdWarning() << "KCalResourceSlox::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "KCalResourceSlox::save(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.save( cacheFile() );

#if 0
  mUploadJob = KIO::file_copy( KURL( cacheFile() ), mUploadUrl, -1, true );
  connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSaveJobResult( KIO::Job * ) ) );
#endif

  return true;
}

bool KCalResourceSlox::isSaving()
{
  return mUploadJob;
}

void KCalResourceSlox::slotSaveJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug() << "KCalResourceSlox::slotSaveJobResult() success" << endl;
  }
  
  mUploadJob = 0;

  emit resourceSaved( this );
}

void KCalResourceSlox::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}

KABC::Lock *KCalResourceSlox::lock()
{
  return mLock;
}

void KCalResourceSlox::update( KCal::IncidenceBase * )
{
}

void KCalResourceSlox::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  DownloadUrl: " << mDownloadUrl.url() << endl;
  kdDebug(5800) << "  UploadUrl: " << mUploadUrl.url() << endl;
  kdDebug(5800) << "  ReloadPolicy: " << mReloadPolicy << endl;
}

#include "kcalresourceslox.moc"
