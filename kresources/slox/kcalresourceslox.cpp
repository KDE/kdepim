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
#include <kabc/stdaddressbook.h>

#include <kresources/configwidget.h>

#include "webdavhandler.h"
#include "kcalsloxprefs.h"
#include "sloxaccounts.h"
#include "confirmsavedialog.h"

#include "kcalresourceslox.h"

using namespace KCal;

KCalResourceSlox::KCalResourceSlox( const KConfig *config )
  : ResourceCached( config )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  }
}

KCalResourceSlox::KCalResourceSlox( const KURL &url )
  : ResourceCached( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
}

KCalResourceSlox::~KCalResourceSlox()
{
  kdDebug() << "~KCalResourceSlox()" << endl;

  disableChangeNotification();

  close();

  if ( mLoadEventsJob ) mLoadEventsJob->kill();
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;

  kdDebug() << "~KCalResourceSlox() done" << endl;
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

  enableChangeNotification();
}

void KCalResourceSlox::readConfig( const KConfig *config )
{
  mPrefs->readConfig();

  ResourceCached::readConfig( config );
}

void KCalResourceSlox::writeConfig( KConfig *config )
{
  kdDebug() << "KCalResourceSlox::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool KCalResourceSlox::doOpen()
{
  kdDebug(5800) << "KCalResourceSlox::doOpen()" << endl;

  mOpen = true;

  return true;
}

bool KCalResourceSlox::doLoad()
{
  kdDebug() << "KCalResourceSlox::load() " << int( this ) << endl;

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

  disableChangeNotification();
  loadCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  QString p = KURL( mPrefs->url() ).protocol();
  if ( p != "http" && p != "https" && p != "webdav" && p != "webdavs" ) {
    QString err = i18n("Non-http protocol: '%1'").arg( p );
    kdDebug() << "KCalResourceSlox::load(): " << err << endl;
    loadError( err );
    return false;
  }

  // The SLOX contacts are loaded asynchronously, so make sure that they are
  // actually loaded.
  KABC::StdAddressBook::self()->asyncLoad();

#if 1
  requestEvents();
#endif
  requestTodos();

  return true;
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
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading events") );
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
  connect( mLoadTodosJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotTodosProgress( KIO::Job *, unsigned long ) ) );

  mLoadTodosProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading to-dos") );
  connect( mLoadTodosProgress,
           SIGNAL( progressItemCanceled( ProgressItem * ) ),
           SLOT( cancelLoadTodos() ) );

  mPrefs->setLastTodoSync( QDateTime::currentDateTime() );
}

void KCalResourceSlox::uploadIncidences()
{
  QDomDocument doc;
  QDomElement ms = WebdavHandler::addDavElement( doc, doc, "D:multistatus" );
  QDomElement pu = WebdavHandler::addDavElement( doc, ms, "D:propertyupdate" );
  QDomElement set = WebdavHandler::addElement( doc, pu, "D:set" );
  QDomElement prop = WebdavHandler::addElement( doc, set, "D:prop" );

  mUploadIsDelete = false;
  Incidence::List added = addedIncidences();
  Incidence::List changed = changedIncidences();
  Incidence::List deleted = deletedIncidences();
  if ( !added.isEmpty() ) {
    mUploadedIncidence = added.first();
  } else if ( !changed.isEmpty() ) {
    mUploadedIncidence = changed.first();
  } else if ( !deleted.isEmpty() ) {
    mUploadedIncidence = deleted.first();
    mUploadIsDelete = true;
  } else {
    mUploadedIncidence = 0;
    kdDebug() << "uploadIncidences(): FINISHED" << endl;
    emit resourceSaved( this );
    return;
  }

  KURL url = mPrefs->url();

  QString sloxId = mUploadedIncidence->customProperty( "SLOX", "ID" );
  if ( !sloxId.isEmpty() ) {
    WebdavHandler::addSloxElement( doc, prop, "S:sloxid", sloxId );
  } else {
    if ( mUploadIsDelete ) {
      kdError() << "Incidence to delete doesn't have a SLOX id" << endl;
      clearChange( mUploadedIncidence );
      uploadIncidences();
      return;
    }
  }
  WebdavHandler::addSloxElement( doc, prop, "S:clientid",
                                 mUploadedIncidence->uid() );

  if ( mUploadIsDelete ) {
    if ( mUploadedIncidence->type() == "Event" ) {
      url.setPath( "/servlet/webdav.calendar/" + sloxId );
    } else if ( mUploadedIncidence->type() == "Todo" ) {
      url.setPath( "/servlet/webdav.tasks/" + sloxId );
    } else {
      kdWarning() << "uploadIncidences(): Unsupported incidence type: "
                  << mUploadedIncidence->type() << endl;
      return;
    }

    QDomElement remove = WebdavHandler::addElement( doc, pu, "D:remove" );
    QDomElement prop = WebdavHandler::addElement( doc, remove, "D:prop" );
    WebdavHandler::addSloxElement( doc, prop, "S:sloxid", sloxId );
  } else {
    createIncidenceAttributes( doc, prop, mUploadedIncidence );
    // FIXME: Use a visitor
    if ( mUploadedIncidence->type() == "Event" ) {
      url.setPath( "/servlet/webdav.calendar/file.xml" );
      createEventAttributes( doc, prop, static_cast<Event *>( mUploadedIncidence ) );
    } else if ( mUploadedIncidence->type() == "Todo" ) {
      url.setPath( "/servlet/webdav.tasks/file.xml" );
      createTodoAttributes( doc, prop, static_cast<Todo *>( mUploadedIncidence ) );
    } else {
      kdWarning() << "uploadIncidences(): Unsupported incidence type: "
                  << mUploadedIncidence->type() << endl;
      return;
    }
  }

  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kdDebug() << "KCalResourceSlox::uploadIncidences(): " << url << endl;

  kdDebug() << "UPLOAD: \n" << doc.toString( 2 ) << endl;

  mUploadJob = KIO::davPropPatch( url, doc, false );
  connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotUploadResult( KIO::Job * ) ) );
  connect( mUploadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
           SLOT( slotUploadProgress( KIO::Job *, unsigned long ) ) );

  mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Uploading incidence") );
  connect( mUploadProgress,
           SIGNAL( progressItemCanceled( ProgressItem * ) ),
           SLOT( cancelUpload() ) );
}

void KCalResourceSlox::createIncidenceAttributes( QDomDocument &doc,
                                                  QDomElement &parent,
                                                  Incidence *incidence )
{
  WebdavHandler::addSloxElement( doc, parent, "S:title",
                                 incidence->summary() );

  WebdavHandler::addSloxElement( doc, parent, "S:description",
                                 incidence->description() );

  if ( incidence->attendeeCount() > 0 ) {
    QDomElement members = WebdavHandler::addSloxElement( doc, parent,
                                                          "S:members" );
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      QString userId = SloxAccounts::self()->lookupId( (*it)->email() );
      WebdavHandler::addSloxElement( doc, members, "S:member", userId );
    }
  }

  // FIXME: create reminder
}

void KCalResourceSlox::createEventAttributes( QDomDocument &doc,
                                              QDomElement &parent,
                                              Event *event )
{
  WebdavHandler::addSloxElement( doc, parent, "S:begins",
      WebdavHandler::qDateTimeToSlox( event->dtStart(), timeZoneId() ) );

  WebdavHandler::addSloxElement( doc, parent, "S:ends",
      WebdavHandler::qDateTimeToSlox( event->dtEnd(), timeZoneId() ) );

  WebdavHandler::addSloxElement( doc, parent, "S:location", event->location() );

  if ( event->doesFloat() ) {
    WebdavHandler::addSloxElement( doc, parent, "S:full_time", "yes" );
  }
}

void KCalResourceSlox::createTodoAttributes( QDomDocument &doc,
                                             QDomElement &parent,
                                             Todo *todo )
{
  if ( todo->hasStartDate() ) {
    WebdavHandler::addSloxElement( doc, parent, "S:startdate",
        WebdavHandler::qDateTimeToSlox( todo->dtStart(), timeZoneId() ) );
  }

  if ( todo->hasDueDate() ) {
    WebdavHandler::addSloxElement( doc, parent, "S:deadline",
        WebdavHandler::qDateTimeToSlox( todo->dtDue(), timeZoneId() ) );
  }

  int priority = todo->priority();
  QString txt;
  switch ( priority ) {
    case 5:
    case 4:
      txt = "1";
      break;
    default:
    case 3:
      txt = "2";
      break;
    case 2:
    case 1:
      txt = "3";
      break;
  }
  WebdavHandler::addSloxElement( doc, parent, "S:priority", txt );

  WebdavHandler::addSloxElement( doc, parent, "S:status",
                                 QString::number( todo->percentComplete() ) );
}

void KCalResourceSlox::parseMembersAttribute( const QDomElement &e,
                                              Incidence *incidence )
{
  incidence->clearAttendees();

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
  QString text = e.text();
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
  QString text = e.text();
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
  QString text = e.text();
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
  kdDebug() << "KCalResourceSlox::slotLoadTodosJobResult()" << endl;

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadTodosJobResult() success" << endl;

    QDomDocument doc = mLoadTodosJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

    bool changed = false;

    disableChangeNotification();

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      QString uid = sloxIdToTodoUid( item.sloxId );
      if ( item.status == SloxItem::Delete ) {
        Todo *todo = mCalendar.todo( uid );
        if ( todo ) {
          mCalendar.deleteTodo( todo );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Todo *newTodo = 0;
        Todo *todo = mCalendar.todo( uid );
        if ( !todo ) {
          newTodo = new Todo;
          todo = newTodo;
          todo->setUid( uid );
        }

        todo->setCustomProperty( "SLOX", "ID", item.sloxId );

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

    enableChangeNotification();

    clearChanges();

    if ( changed ) emit resourceChanged( this );

    emit resourceLoaded( this );
  }

  mLoadTodosJob = 0;

  if ( mLoadTodosProgress ) mLoadTodosProgress->setComplete();
  mLoadTodosProgress = 0;
}

void KCalResourceSlox::slotLoadEventsResult( KIO::Job *job )
{
  kdDebug() << "KCalResourceSlox::slotLoadEventsResult() " << int( this ) << endl;

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadEventsResult() success" << endl;

    QDomDocument doc = mLoadEventsJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QValueList<SloxItem> items = WebdavHandler::getSloxItems( doc );

    bool changed = false;

    disableChangeNotification();

    QValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      QString uid = sloxIdToEventUid( item.sloxId );
      if ( item.status == SloxItem::Delete ) {
        Event *event = mCalendar.event( uid );
        if ( event ) {
          mCalendar.deleteEvent( event );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Event *newEvent = 0;
        Event *event = mCalendar.event( uid );
        if ( !event ) {
          newEvent = new Event;
          event = newEvent;
          event->setUid( uid );
        }

        event->setCustomProperty( "SLOX", "ID", item.sloxId );

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

    enableChangeNotification();

    clearChanges();

    if ( changed ) emit resourceChanged( this );

    emit resourceLoaded( this );
  }

  mLoadEventsJob = 0;

  if ( mLoadEventsProgress ) mLoadEventsProgress->setComplete();
  mLoadEventsProgress = 0;
}

void KCalResourceSlox::slotUploadResult( KIO::Job *job )
{
  kdDebug() << "KCalResourceSlox::slotUploadResult()" << endl;

  if ( job->error() ) {
    saveError( job->errorString() );
  } else {
    kdDebug() << "KCalResourceSlox::slotUploadResult() success" << endl;

    QDomDocument doc = mUploadJob->response();

    kdDebug() << "UPLOAD RESULT:" << endl;
    kdDebug() << doc.toString( 2 ) << endl;

    QDomElement docElement = doc.documentElement();

    QDomNode responseNode;
    for( responseNode = docElement.firstChild(); !responseNode.isNull();
         responseNode = responseNode.nextSibling() ) {
      QDomElement responseElement = responseNode.toElement();
      if ( responseElement.tagName() == "response" ) {
        QDomNode propstat = responseElement.namedItem( "propstat" );
        if ( propstat.isNull() ) {
          kdError() << "Unable to find propstat tag." << endl;
          continue;
        }

        QDomNode prop = propstat.namedItem( "prop" );
        if ( prop.isNull() ) {
          kdError() << "Unable to find WebDAV property" << endl;
          continue;
        }

        QDomNode sloxIdNode = prop.namedItem( "sloxid" );
        if ( sloxIdNode.isNull() ) {
          kdError() << "Unable to find SLOX id." << endl;
          continue;
        }
        QDomElement sloxIdElement = sloxIdNode.toElement();
        QString sloxId = sloxIdElement.text();
        kdDebug() << "SLOXID: " << sloxId << endl;

        if ( mUploadIsDelete ) {
          kdDebug() << "Incidence deleted" << endl;
        } else {
          QDomNode clientIdNode = prop.namedItem( "clientid" );
          if ( clientIdNode.isNull() ) {
            kdError() << "Unable to find client id." << endl;
            continue;
          }
          QDomElement clientidElement = clientIdNode.toElement();
          QString clientId = clientidElement.text();

          kdDebug() << "CLIENTID: " << clientId << endl;

          Incidence *i = mUploadedIncidence->clone();
          QString uid;
          if ( i->type() == "Event" ) uid = sloxIdToEventUid( sloxId );
          else if ( i->type() == "Todo" ) uid = sloxIdToTodoUid( sloxId );
          else {
            kdError() << "KCalResourceSlox::slotUploadResult(): Unknown type: "
                      << i->type() << endl;
          }
          i->setUid( uid );
          i->setCustomProperty( "SLOX", "ID", uid );

          disableChangeNotification();
          mCalendar.deleteIncidence( mUploadedIncidence );
          mCalendar.addIncidence( i );
          saveCache();
          enableChangeNotification();
        }
      }
    }
  }

  mUploadJob = 0;

  mUploadProgress->setComplete();
  mUploadProgress = 0;

  clearChange( mUploadedIncidence );

  uploadIncidences();
}

void KCalResourceSlox::slotEventsProgress( KIO::Job *job,
                                           unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: events " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadEventsProgress ) mLoadEventsProgress->setProgress( percent );
}

void KCalResourceSlox::slotTodosProgress( KIO::Job *job, unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: todos " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadTodosProgress ) mLoadTodosProgress->setProgress( percent );
}

void KCalResourceSlox::slotUploadProgress( KIO::Job *job, unsigned long percent )
{
#if 0
  kdDebug() << "PROGRESS: upload " << int( job ) << ": " << percent << endl;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mUploadProgress ) mUploadProgress->setProgress( percent );
}

bool KCalResourceSlox::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

bool KCalResourceSlox::doSave()
{
  kdDebug() << "KCalResourceSlox::save()" << endl;

  if ( !mOpen ) return true;

  if ( readOnly() || !hasChanges() ) {
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

  if ( !confirmSave() ) return false;

  saveCache();

  uploadIncidences();

  return true;
}

bool KCalResourceSlox::isSaving()
{
  return mUploadJob;
}

void KCalResourceSlox::doClose()
{
  kdDebug() << "KCalResourceSlox::doClose()" << endl;

  if ( !mOpen ) return;

  cancelLoadEvents();
  cancelLoadTodos();

  if ( mUploadJob ) {
    kdError() << "KCalResourceSlox::doClose() Still saving" << endl;
  } else {
    mCalendar.close();
  }

  mOpen = false;
}

KABC::Lock *KCalResourceSlox::lock()
{
  return mLock;
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
  mLoadEventsProgress = 0;
}

void KCalResourceSlox::cancelLoadTodos()
{
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  mLoadTodosJob = 0;
  if ( mLoadTodosProgress ) mLoadTodosProgress->setComplete();
  mLoadTodosProgress = 0;
}

void KCalResourceSlox::cancelUpload()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
}

QString KCalResourceSlox::sloxIdToEventUid( const QString &sloxId )
{
  return "KResources_SLOX_Event_" + sloxId;
}

QString KCalResourceSlox::sloxIdToTodoUid( const QString &sloxId )
{
  return "KResources_SLOX_Todo_" + sloxId;
}

#include "kcalresourceslox.moc"
