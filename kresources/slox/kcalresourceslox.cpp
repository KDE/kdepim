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

#include <libkdepim/progressmanager.h>

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
#include "kcalsloxprefs.h"
#include "sloxaccounts.h"

#include "kcalresourceslox.h"

using namespace KCal;

KCalResourceSlox::KCalResourceSlox( const KConfig *config )
  : ResourceCached( config )
{
  init();

  if ( config ) {
    readConfig( config );
  }
}

KCalResourceSlox::KCalResourceSlox( const KURL &url )
  : ResourceCached( 0 )
{
  init();

  mPrefs->setUrl( url.url() );
}

KCalResourceSlox::~KCalResourceSlox()
{
  kdDebug() << "~KCalResourceSlox()" << endl;

  close();

  if ( mLoadEventsJob ) mLoadEventsJob->kill();
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;
}

void KCalResourceSlox::init()
{
  mPrefs = new SloxPrefs;

  mLoadEventsJob = 0;
  mLoadTodosJob = 0;

  mUploadJob = 0;

  mLoadEventsProgress = 0;
  mLoadTodosProgress = 0;

  setType( "slox" );

  mOpen = false;

  mLock = new KABC::LockNull( true );
}

void KCalResourceSlox::readConfig( const KConfig * )
{
  mPrefs->readConfig();
}

void KCalResourceSlox::writeConfig( KConfig *config )
{
  kdDebug() << "KCalResourceSlox::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();
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

  QString p = KURL( mPrefs->url() ).protocol();
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
  KURL url = mPrefs->url();
  url.setPath( "/servlet/webdav.calendar/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kdDebug() << "KCalResourceSlox::requestEvents(): " << url << endl;

  QString lastsync = "0";
  if ( mPrefs->useLastSync() ) {
    QDateTime dt = mPrefs->lastEventSync();
    if ( dt.isValid() ) {
      lastsync = WebdavHandler::qDateTimeToSlox( dt.addDays( -1 ) );
    }
  }

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( doc, prop, "lastsync", lastsync );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

  kdDebug() << "REQUEST CALENDAR: \n" << doc.toString( 2 ) << endl;

  mLoadEventsJob = KIO::davPropFind( url, doc, "0", false );
  connect( mLoadEventsJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadEventsResult( KIO::Job * ) ) );
  connect( mLoadEventsJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotEventsProgress( KIO::Job *, unsigned long ) ) );

  mLoadEventsProgress = KPIM::ProgressManager::instance()->createProgressItem(
      "sloxkcalevents", i18n("Downloading events") );
  connect( mLoadEventsProgress,
           SIGNAL( progressItemCanceled( ProgressItem * ) ),
           SLOT( cancelLoadEvents() ) );

  mPrefs->setLastEventSync( QDateTime::currentDateTime() );
}

void KCalResourceSlox::requestTodos()
{
  KURL url = mPrefs->url();
  url.setPath( "/servlet/webdav.tasks/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kdDebug() << "KCalResourceSlox::requestTodos(): " << url << endl;

  QString lastsync = "0";
  if ( mPrefs->useLastSync() ) {
    QDateTime dt = mPrefs->lastTodoSync();
    if ( dt.isValid() ) {
      lastsync = WebdavHandler::qDateTimeToSlox( dt.addDays( -1 ) );
    }
  }

  QDomDocument doc;
  QDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  QDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( doc, prop, "lastsync", lastsync );
  WebdavHandler::addSloxElement( doc, prop, "folderid" );
  WebdavHandler::addSloxElement( doc, prop, "objecttype", "all" );

  kdDebug() << "REQUEST TASKS: \n" << doc.toString( 2 ) << endl;

  mLoadTodosJob = KIO::davPropFind( url, doc, "0", false );
  connect( mLoadTodosJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadTodosResult( KIO::Job * ) ) );
  connect( mLoadEventsJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotTodosProgress( KIO::Job *, unsigned long ) ) );

  mLoadTodosProgress = KPIM::ProgressManager::instance()->createProgressItem(
      "sloxkcaltodos", i18n("Downloading todos") );
  connect( mLoadTodosProgress,
           SIGNAL( progressItemCanceled( ProgressItem * ) ),
           SLOT( cancelLoadTodos() ) );

  mPrefs->setLastTodoSync( QDateTime::currentDateTime() );
}

void KCalResourceSlox::parseMembersAttribute( const QDomElement &e,
                                              Incidence *incidence )
{
  QDomNode n;
  for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement memberElement = n.toElement();
    if ( memberElement.tagName() == "member" ) {
      QString member = memberElement.text();
      KABC::Addressee account = SloxAccounts::self()->lookupUser( member );
      QString name;
      QString email;
      Attendee *a = incidence->attendeeByUid( member );
      if ( account.isEmpty() ) {
        if ( a ) continue;

        name = member;
        email = member + "@" + KURL( mPrefs->url() ).host();
      } else {
        name = account.realName();
        email = account.preferredEmail();
      }
      if ( a ) {
        a->setName( name );
        a->setEmail( email );
      } else {
        a = new Attendee( name, email );
        a->setUid( member );
        incidence->addAttendee( a );
      }
    } else {
      kdDebug() << "Unknown tag in members attribute: "
                << memberElement.tagName() << endl;
    }
  }
}

void KCalResourceSlox::parseIncidenceAttribute( const QDomElement &e,
                                                Incidence *incidence )
{
  QString tag = e.tagName();
  QString text = QString::fromUtf8( e.text().latin1() );
  if ( text.isEmpty() ) return;

  if ( tag == "title" ) {
    incidence->setSummary( text );
  } else if ( e.tagName() == "description" ) {
    incidence->setDescription( text );
  } else if ( tag == "reminder" ) {
    int minutes = text.toInt();
    // FIXME: What exactly means a "0" reminder?
    if ( minutes != 0 ) {
      Alarm::List alarms = incidence->alarms();
      Alarm *alarm;
      if ( alarms.isEmpty() ) alarm = incidence->newAlarm();
      else alarm = alarms.first(); 
      if ( alarm->type() == Alarm::Invalid ) {
        alarm->setType( Alarm::Display );
      }
      Duration d( minutes * 60 );
      alarm->setStartOffset( d );
      alarm->setEnabled( true );
    }
  } else if ( tag == "members" ) {
    parseMembersAttribute( e, incidence );
  }
}

void KCalResourceSlox::parseEventAttribute( const QDomElement &e,
                                            Event *event )
{
  QString tag = e.tagName();
  QString text = QString::fromUtf8( e.text().latin1() );
  if ( text.isEmpty() ) return;

  if ( tag == "begins" ) {
    QDateTime dt;
    if ( event->doesFloat() ) dt = WebdavHandler::sloxToQDateTime( text );
    else dt = WebdavHandler::sloxToQDateTime( text, timeZoneId() );
    event->setDtStart( dt );
  } else if ( tag == "ends" ) {
    QDateTime dt;
    if ( event->doesFloat() ) {
      dt = WebdavHandler::sloxToQDateTime( text );
      dt = dt.addSecs( -1 );
    }
    else dt = WebdavHandler::sloxToQDateTime( text, timeZoneId() );
    event->setDtEnd( dt );
  } else if ( tag == "location" ) {
    event->setLocation( text );
  }
}

void KCalResourceSlox::parseTodoAttribute( const QDomElement &e,
                                           Todo *todo )
{
  QString tag = e.tagName();
  QString text = QString::fromUtf8( e.text().latin1() );
  if ( text.isEmpty() ) return;

  if ( tag == "startdate" ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtStart( dt );
      todo->setHasStartDate( true );
    }
  } else if ( tag == "deadline" ) {
    QDateTime dt = WebdavHandler::sloxToQDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtDue( dt );
      todo->setHasDueDate( true );
    }
  } else if ( tag == "priority" ) {
    int p = text.toInt();
    if ( p < 1 || p > 3 ) {
      kdError() << "Unknown priority: " << text << endl;
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
  } else if ( tag == "status" ) {
    int completed = text.toInt();
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
        Todo *todo = mCalendar.todo( item.uid );
        if ( todo ) { 
          mCalendar.deleteTodo( todo );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Todo *newTodo = 0;
        Todo *todo = mCalendar.todo( item.uid );
        if ( !todo ) {
          newTodo = new Todo;
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

  mLoadTodosProgress->setComplete();
  mLoadTodosProgress = 0;

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
        Event *event = mCalendar.event( item.uid );
        if ( event ) {
          mCalendar.deleteEvent( event );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Event *newEvent = 0;
        Event *event = mCalendar.event( item.uid );
        if ( !event ) {
          newEvent = new Event;
          event = newEvent;
          event->setUid( item.uid );
        }

        QDomNode n = item.domNode.namedItem( "full_time" );
        event->setFloats( n.toElement().text() == "yes" );

        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          parseIncidenceAttribute( e, event );
          parseEventAttribute( e, event );
        }

//        kdDebug() << "EVENT " << item.uid << " " << event->summary() << endl;

        if ( newEvent ) mCalendar.addEvent( event );

        changed = true;
      }
    }
    
    if ( changed ) emit resourceChanged( this );
  }

  mLoadEventsJob = 0;

  mLoadEventsProgress->setComplete();
  mLoadEventsProgress = 0;

  emit resourceLoaded( this );
}

void KCalResourceSlox::slotEventsProgress( KIO::Job *job,
                                           unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: sloxkcal " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadEventsProgress ) mLoadEventsProgress->setProgress( percent );
}

void KCalResourceSlox::slotTodosProgress( KIO::Job *job, unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: sloxkcal " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadTodosProgress ) mLoadTodosProgress->setProgress( percent );
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

  cancelLoadEvents();
  cancelLoadTodos();

  mCalendar.close();
  mOpen = false;
}

KABC::Lock *KCalResourceSlox::lock()
{
  return mLock;
}

void KCalResourceSlox::update( IncidenceBase * )
{
}

void KCalResourceSlox::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  Url: " << mPrefs->url() << endl;
}

void KCalResourceSlox::cancelLoadEvents()
{
  if ( mLoadEventsJob ) mLoadEventsJob->kill();
  mLoadEventsJob = 0;
  if ( mLoadEventsProgress ) mLoadEventsProgress->setComplete();
}

void KCalResourceSlox::cancelLoadTodos()
{
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  mLoadTodosJob = 0;
  if ( mLoadTodosProgress ) mLoadTodosProgress->setComplete();
}

#include "kcalresourceslox.moc"
