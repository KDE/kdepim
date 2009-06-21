/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Volker Krause <vkrause@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <typeinfo>
#include <stdlib.h>

#include <QDateTime>
#include <QString>
#include <QFile>
#include <QRegExp>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/davjob.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <libkdepim/progressmanager.h>

#include <kcal/exceptions.h>
#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/confirmsavedialog.h>

#include <kabc/locknull.h>
#include <kabc/stdaddressbook.h>

#include <kresources/configwidget.h>

#include "webdavhandler.h"
#include "kcalsloxprefs.h"
#include "sloxaccounts.h"

#include "kcalresourceslox.h"

using namespace KCal;

KCalResourceSlox::KCalResourceSlox()
  : ResourceCached(), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

KCalResourceSlox::KCalResourceSlox( const KConfigGroup &group )
  : ResourceCached( group ), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  readConfig( group );
}

KCalResourceSlox::KCalResourceSlox( const KUrl &url )
  : ResourceCached(), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  mPrefs->setUrl( url.url() );
}

KCalResourceSlox::~KCalResourceSlox()
{
  kDebug();

  disableChangeNotification();

  close();

  if ( mLoadEventsJob ) mLoadEventsJob->kill();
  if ( mLoadTodosJob ) mLoadTodosJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;

  kDebug() << "done";
}

void KCalResourceSlox::init()
{
  mPrefs = new SloxPrefs;
  mWebdavHandler.setResource( this );

  mLoadEventsJob = 0;
  mLoadTodosJob = 0;

  mUploadJob = 0;

  mLoadEventsProgress = 0;
  mLoadTodosProgress = 0;

  mAccounts = 0;

  mLock = new KABC::LockNull( true );

  enableChangeNotification();
}

void KCalResourceSlox::readConfig( const KConfigGroup &group )
{
  mPrefs->readConfig();

  mWebdavHandler.setUserId( mPrefs->user() );

  ResourceCached::readConfig( group );

  KUrl url = mPrefs->url();
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  delete mAccounts;
  mAccounts = new SloxAccounts( this, url );
}

void KCalResourceSlox::writeConfig( KConfigGroup &group )
{
  kDebug();

  ResourceCalendar::writeConfig( group );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( group );
}

bool KCalResourceSlox::doLoad( bool syncCache )
{
  kDebug() << long( this );

  Q_UNUSED( syncCache )
  if ( mLoadEventsJob || mLoadTodosJob ) {
    kDebug() << "download still in progress.";
    return true;
  }
  if ( mUploadJob ) {
    kWarning() << "upload still in progress.";
    loadError( "Upload still in progress." );
    return false;
  }

  calendar()->close();

  disableChangeNotification();
  loadFromCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  QString p = KUrl( mPrefs->url() ).protocol();
  if ( p != "http" && p != "https" && p != "webdav" && p != "webdavs" ) {
    QString err = i18n("Non-http protocol: '%1'", p );
    kDebug() << err;
    loadError( err );
    return false;
  }

  // The SLOX contacts are loaded asynchronously, so make sure that they are
  // actually loaded.
  KABC::StdAddressBook::self( true )->asyncLoad();

#if 1
  requestEvents();
#endif
  requestTodos();

  return true;
}

void KCalResourceSlox::requestEvents()
{
  KUrl url = mPrefs->url();
  url.setPath( "/servlet/webdav.calendar/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kDebug() << url;

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
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), lastsync );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->calendarFolderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kDebug() << "REQUEST CALENDAR:" << doc.toString( 2 );

  mLoadEventsJob = KIO::davPropFind( url, doc, "0", KIO::HideProgressInfo );
  connect( mLoadEventsJob, SIGNAL( result( KJob * ) ),
           SLOT( slotLoadEventsResult( KJob * ) ) );
  connect( mLoadEventsJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotEventsProgress( KJob *, unsigned long ) ) );

  mLoadEventsProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading events") );
  connect( mLoadEventsProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoadEvents() ) );

  mPrefs->setLastEventSync( QDateTime::currentDateTime() );
}

void KCalResourceSlox::requestTodos()
{
  KUrl url = mPrefs->url();
  url.setPath( "/servlet/webdav.tasks/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kDebug() << url;

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
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), lastsync );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->taskFolderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kDebug() << "REQUEST TASKS:" << doc.toString( 2 );

  mLoadTodosJob = KIO::davPropFind( url, doc, "0", KIO::HideProgressInfo );
  connect( mLoadTodosJob, SIGNAL( result( KJob * ) ),
           SLOT( slotLoadTodosResult( KJob * ) ) );
  connect( mLoadTodosJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotTodosProgress( KJob *, unsigned long ) ) );

  mLoadTodosProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading to-dos") );
  connect( mLoadTodosProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoadTodos() ) );

  mPrefs->setLastTodoSync( QDateTime::currentDateTime() );
}

void KCalResourceSlox::uploadIncidences()
{
  QDomDocument doc;
  QDomElement ms = WebdavHandler::addDavElement( doc, doc, "multistatus" );
  QDomElement pu = WebdavHandler::addDavElement( doc, ms, "propertyupdate" );
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
    kDebug() << "FINISHED";
    emit resourceSaved( this );
    return;
  }

  // Don't try to upload recurring incidences as long as the resource doesn't
  // correctly write them in order to avoid corrupting data on the server.
  // FIXME: Remove when recurrences are correctly written.
  if ( mUploadedIncidence->recurs() && type() == "slox" ) {
    clearChange( mUploadedIncidence );
    uploadIncidences();
    return;
  }

  KUrl url = mPrefs->url();

  QString sloxId = mUploadedIncidence->customProperty( "SLOX", "ID" );
  if ( !sloxId.isEmpty() ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectId ), sloxId );
  } else {
    if ( mUploadIsDelete ) {
      kError() << "Incidence to delete doesn't have a SLOX id";
      clearChange( mUploadedIncidence );
      uploadIncidences();
      return;
    }
  }
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( ClientId ),
                                 mUploadedIncidence->uid() );

  if ( mUploadIsDelete ) {
    if ( mUploadedIncidence->type() == "Event" ) {
      url.setPath( "/servlet/webdav.calendar/" + sloxId );
    } else if ( mUploadedIncidence->type() == "Todo" ) {
      url.setPath( "/servlet/webdav.tasks/" + sloxId );
    } else {
      kWarning() << "Unsupported incidence type:"
                 << mUploadedIncidence->type();
      return;
    }

    if ( type() == "ox" ) {
      WebdavHandler::addSloxElement( this, doc, prop, "method", "DELETE" );
    } else {
      QDomElement remove = WebdavHandler::addElement( doc, pu, "D:remove" );
      QDomElement prop = WebdavHandler::addElement( doc, remove, "D:prop" );
      WebdavHandler::addSloxElement( this, doc, prop, "sloxid", sloxId );
    }
  } else {
    createIncidenceAttributes( doc, prop, mUploadedIncidence );
    // FIXME: Use a visitor
    if ( mUploadedIncidence->type() == "Event" ) {
      url.setPath( "/servlet/webdav.calendar/file.xml" );
      createEventAttributes( doc, prop, static_cast<Event *>( mUploadedIncidence ) );
      // TODO: OX supports recurrences also for tasks
      createRecurrenceAttributes( doc, prop, mUploadedIncidence );
    } else if ( mUploadedIncidence->type() == "Todo" ) {
      url.setPath( "/servlet/webdav.tasks/file.xml" );
      createTodoAttributes( doc, prop, static_cast<Todo *>( mUploadedIncidence ) );
    } else {
      kWarning() << "Unsupported incidence type:"
                 << mUploadedIncidence->type();
      return;
    }
  }

  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kDebug() << url;

  kDebug() << "UPLOAD:" << doc.toString( 2 );

  mUploadJob = KIO::davPropPatch( url, doc, KIO::HideProgressInfo );
  connect( mUploadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotUploadResult( KJob * ) ) );
  connect( mUploadJob, SIGNAL( percent( KJob *, unsigned long ) ),
           SLOT( slotUploadProgress( KJob *, unsigned long ) ) );

  mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Uploading incidence") );
  connect( mUploadProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelUpload() ) );
}

void KCalResourceSlox::createIncidenceAttributes( QDomDocument &doc,
                                                  QDomElement &parent,
                                                  Incidence *incidence )
{
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( IncidenceTitle ),
                                 incidence->summary() );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Description ),
                                 incidence->description() );

  if ( incidence->attendeeCount() > 0 ) {
    QDomElement members = WebdavHandler::addSloxElement( this, doc, parent,
        fieldName( Participants ) );
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for( it = attendees.constBegin(); it != attendees.constEnd(); ++it ) {
      if ( mAccounts ) {
        QString userId = mAccounts->lookupId( (*it)->email() );
        QString status;
        switch ( (*it)->status() ) {
          case Attendee::Accepted: status = "accept"; break;
          case Attendee::Declined: status = "decline"; break;
          default: status = "none"; break;
        }
        QDomElement el = WebdavHandler::addSloxElement( this, doc, members, fieldName( Participant ), userId );
        el.setAttribute( "confirm", status );
      } else {
        kError() << "KCalResourceSlox: No accounts set.";
      }
    }
  }

  // set read attributes - if SecrecyPublic, set it to users
  // TODO OX support
  if ( incidence->secrecy() == Incidence::SecrecyPublic && type() != "ox" )
  {
    QDomElement rights = WebdavHandler::addSloxElement( this, doc, parent, "readrights" );
    WebdavHandler::addSloxElement( this, doc, rights, "group", "users" );
  }

  // set reminder as the number of minutes to the start of the event
  KCal::Alarm::List alarms = incidence->alarms();
  if ( !alarms.isEmpty() && alarms.first()->hasStartOffset() && alarms.first()->enabled() )
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( Reminder ),
                                   QString::number( (-1) * alarms.first()->startOffset().asSeconds() / 60 ) );
  else
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( Reminder ), "0" );

  // categories
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Categories ), incidence->categories().join( ", " ) );
}

void KCalResourceSlox::createEventAttributes( QDomDocument &doc,
                                              QDomElement &parent,
                                              Event *event )
{
  QString folderId = mPrefs->calendarFolderId();
  if ( folderId.isEmpty() && type() == "ox" ) // SLOX and OX use diffrent default folders
    folderId = "-1";
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( FolderId ), folderId );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( EventBegin ),
      WebdavHandler::kDateTimeToSlox( event->dtStart() ) );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( EventEnd ),
      WebdavHandler::kDateTimeToSlox( event->dtEnd() ) );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Location ), event->location() );

  if ( event->allDay() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( FullTime ), boolToStr( true ) );
  } else {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( FullTime ), boolToStr( false ) );
  }
}

void KCalResourceSlox::createTodoAttributes( QDomDocument &doc,
                                             QDomElement &parent,
                                             Todo *todo )
{
  QString folderId = mPrefs->taskFolderId();
  if ( folderId.isEmpty() && type() == "ox" ) // SLOX and OX use diffrent default folders
    folderId = "-1";
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( FolderId ), folderId );

  if ( todo->hasStartDate() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( TaskBegin ),
        WebdavHandler::kDateTimeToSlox( todo->dtStart() ) );
  }

  if ( todo->hasDueDate() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( TaskEnd ),
        WebdavHandler::kDateTimeToSlox( todo->dtDue() ) );
  }

  int priority = todo->priority();
  QString txt;
  switch ( priority ) {
    case 9:
    case 8:
      txt = "1";
      break;
    case 2:
    case 1:
      txt = "3";
      break;
    default:
      txt = "2";
      break;
  }
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Priority ), txt );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( PercentComplete ),
                                 QString::number( todo->percentComplete() ) );
}

void KCalResourceSlox::createRecurrenceAttributes( QDomDocument &doc,
                                                   QDomElement &parent,
                                                   KCal::Incidence *incidence )
{
  if ( !incidence->recurs() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ),
                                   type() == "ox" ? "none" : "no" );
    return;
  }
  Recurrence *r = incidence->recurrence();
  int monthOffset = ( type() == "ox" ? -1 : 0 );
  switch ( r->recurrenceType() ) {
    case Recurrence::rDaily:
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "daily" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceDailyFreq ),
                                     QString::number( r->frequency() ) );
      break;
    case Recurrence::rWeekly: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "weekly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceWeeklyFreq ),
                                     QString::number( r->frequency() ) );
      // TODO: SLOX support
      int oxDays = 0;
      for ( int i = 0; i < 7; ++i ) {
        if ( r->days()[i] )
          oxDays += 1 << ( ( i + 1 ) % 7 );
      }
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "days", QString::number( oxDays ) );
      break; }
    case Recurrence::rMonthlyDay:
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "monthly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthlyFreq ),
                                     QString::number( r->frequency() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthlyDay ),
                                     QString::number( r->monthDays().first() ) );
      break;
    case Recurrence::rMonthlyPos: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ),
                                     type() == "ox" ? "monthly" : "monthly2" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Freq ),
                                     QString::number( r->frequency() ) );
      RecurrenceRule::WDayPos wdp = r->monthPositions().first();
      // TODO: SLOX support
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Day ),
                                     QString::number( 1 << wdp.day() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Pos ),
                                     QString::number( wdp.pos() ) );
      break; }
    case Recurrence::rYearlyMonth:
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "yearly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearlyDay ),
                                     QString::number( r->yearDates().first() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearlyMonth ),
                                     QString::number( r->yearMonths().first() + monthOffset ) );
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "interval", "1" );
      break;
    case Recurrence::rYearlyPos: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ),
                                     type() == "ox" ? "yearly" : "yearly2" );
      RecurrenceRule::WDayPos wdp = r->monthPositions().first();
      // TODO: SLOX support
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Day ),
                                     QString::number( 1 << wdp.day() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Pos ),
                                     QString::number( wdp.pos() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Month ),
                                     QString::number( r->yearMonths().first() + monthOffset ) );
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "interval", "1" );
      break; }
    default:
      kDebug() << "unsupported recurrence type:" << r->recurrenceType();
  }
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceEnd ),
                                 WebdavHandler::kDateTimeToSlox( r->endDateTime() ) );
  // delete exceptions
  DateList exlist = r->exDates();
  QStringList res;
  for ( DateList::ConstIterator it = exlist.constBegin(); it != exlist.constEnd(); ++it )
    res.append( WebdavHandler::qDateTimeToSlox( QDateTime( *it ) ) );
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceDelEx ), res.join( "," ) );
}

void KCalResourceSlox::parseMembersAttribute( const QDomElement &e,
                                              Incidence *incidence )
{
  incidence->clearAttendees();

  QDomNode n;
  for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement memberElement = n.toElement();
    if ( memberElement.tagName() == fieldName( Participant ) ) {
      QString member = memberElement.text();
      KABC::Addressee account;
      if ( mAccounts ) account = mAccounts->lookupUser( member );
      else kError() << "KCalResourceSlox: no accounts set";
      QString name;
      QString email;
      Attendee *a = incidence->attendeeByUid( member );
      if ( account.isEmpty() ) {
        if ( a ) continue;

        name = member;
        email = member + '@' + KUrl( mPrefs->url() ).host();
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
      QString status = memberElement.attribute( "confirm" );
      if ( !status.isEmpty() ) {
        if ( status == "accept" ) {
          a->setStatus( Attendee::Accepted );
        } else if ( status == "decline" ) {
          a->setStatus( Attendee::Declined );
        } else {
          a->setStatus( Attendee::NeedsAction );
        }
      }
    } else {
      kDebug() << "Unknown tag in members attribute:"
               << memberElement.tagName();
    }
  }
}

void KCalResourceSlox::parseReadRightsAttribute( const QDomElement &e,
                                              Incidence *incidence )
{
  QDomNode n;
  for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement rightElement = n.toElement();
    if ( rightElement.tagName() == "group" ) {
      QString groupName = rightElement.text();
      if ( groupName == "users" )
        incidence->setSecrecy( Incidence::SecrecyPublic );
    }
  }
}

void KCalResourceSlox::parseIncidenceAttribute( const QDomElement &e,
                                                Incidence *incidence )
{
  QString tag = e.tagName();
  QString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;

  if ( tag == fieldName( IncidenceTitle ) ) {
    incidence->setSummary( text );
  } else if ( e.tagName() == fieldName( Description ) ) {
    incidence->setDescription( text );
  } else if ( tag == fieldName( Reminder ) ) {
    int minutes = text.toInt();
    if ( minutes != 0 ) {
      Alarm::List alarms = incidence->alarms();
      Alarm *alarm;
      if ( alarms.isEmpty() ) alarm = incidence->newAlarm();
      else alarm = alarms.first();
      if ( alarm->type() == Alarm::Invalid ) {
        alarm->setType( Alarm::Display );
      }
      Duration d( minutes * -60 );
      alarm->setStartOffset( d );
      alarm->setEnabled( true );
    } else {
      // 0 reminder -> disable alarm
      incidence->clearAlarms();
    }
  } else if ( tag == fieldName( CreatedBy ) ) {
    KABC::Addressee a;
    if ( mAccounts ) a = mAccounts->lookupUser( text );
    else kDebug() << "KCalResourceSlox: no accounts set";
    incidence->setOrganizer( Person( a.formattedName(), a.preferredEmail() ) );
  } else if ( tag == fieldName( Participants ) ) {
    parseMembersAttribute( e, incidence );
  } else if ( tag == "readrights" ) {
    parseReadRightsAttribute( e, incidence );
  } else if ( tag == fieldName( Categories ) ) {
    incidence->setCategories( text.split( QRegExp(",\\s*") ) );
  }
}

void KCalResourceSlox::parseEventAttribute( const QDomElement &e,
                                            Event *event )
{
  QString tag = e.tagName();
  QString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;

  if ( tag == fieldName( EventBegin ) ) {
    KDateTime dt;
    if ( event->allDay() ) {
      if ( type() == "ox" )
        dt = WebdavHandler::sloxToKDateTime( text, timeSpec() );
      else
        dt = WebdavHandler::sloxToKDateTime( text ); // ### is this really correct for SLOX?
      dt.setDateOnly( true );
    } else
      dt = WebdavHandler::sloxToKDateTime( text );
    event->setDtStart( dt );
  } else if ( tag == fieldName( EventEnd ) ) {
    KDateTime dt;
    if ( event->allDay() ) {
      dt = WebdavHandler::sloxToKDateTime( text );
      dt = dt.addSecs( -1 );
    }
    else dt = WebdavHandler::sloxToKDateTime( text );
    event->setDtEnd( dt );
  } else if ( tag == fieldName( Location ) ) {
    event->setLocation( text );
  }
}

void KCalResourceSlox::parseRecurrence( const QDomNode &node, Event *event )
{
  QString type;

  int dailyValue = -1;
  KDateTime end;

  int weeklyValue = -1;
  QBitArray days( 7 ); // days, starting with monday
  bool daysSet = false;

  int monthlyValueDay = -1;
  int monthlyValueMonth = -1;

  int yearlyValueDay = -1;
  int yearlyMonth = -1;

  int monthly2Recurrency = 0;
  int monthly2Day = 0;
  int monthly2ValueMonth = -1;

  int yearly2Recurrency = 0;
  int yearly2Day = 0;
  int yearly2Month = -1;

  DateList deleteExceptions;

  QDomNode n;

  for( n = node.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    QString tag = e.tagName();
    QString text = decodeText( e.text() );
    kDebug() << tag << ":" << text;

    if ( tag == fieldName( RecurrenceType ) ) {
      type = text;
    } else if ( tag == "daily_value" ) {
      dailyValue = text.toInt();
    } else if ( tag == fieldName( RecurrenceEnd ) ) {
      end = WebdavHandler::sloxToKDateTime( text );
    } else if ( tag == "weekly_value" ) {
      weeklyValue = text.toInt();
    } else if ( tag.left( 11 ) == "weekly_day_" ) {
      int day = tag.mid( 11, 1 ).toInt();
      int index;
      if ( day == 1 ) index = 0;
      else index = day - 2;
      days.setBit( index );
    } else if ( tag == "monthly_value_day" ) {
      monthlyValueDay = text.toInt();
    } else if ( tag == "monthly_value_month" ) {
      monthlyValueMonth = text.toInt();
    } else if ( tag == "yearly_value_day" ) {
      yearlyValueDay = text.toInt();
    } else if ( tag == "yearly_month" ) {
      yearlyMonth = text.toInt();
    } else if ( tag == "monthly2_recurrency" ) {
      monthly2Recurrency = text.toInt();
    } else if ( tag == "monthly2_day" ) {
      monthly2Day = text.toInt();
    } else if ( tag == "monthly2_value_month" ) {
      monthly2ValueMonth = text.toInt();
    } else if ( tag == "yearly2_reccurency" ) { // this is not a typo, this is what SLOX really sends!
      yearly2Recurrency = text.toInt();
    } else if ( tag == "yearly2_day" ) {
      yearly2Day = text.toInt();
    } else if ( tag == "yearly2_month" ) {
      yearly2Month = text.toInt() + 1;
    // OX recurrence fields
    } else if ( tag == "interval" ) {
      dailyValue = text.toInt();
      weeklyValue = text.toInt();
      monthlyValueMonth = text.toInt();
      monthly2ValueMonth = text.toInt();
    } else if ( tag == "days" ) {
      int tmp = text.toInt();  // OX encodes days binary: 1=Su, 2=Mo, 4=Tu, ...
      for ( int i = 0; i < 7; ++i ) {
        if ( tmp & (1 << i) )
          days.setBit( (i + 6) % 7 );
      }
      daysSet = true;
    } else if ( tag == "day_in_month" ) {
      monthlyValueDay = text.toInt();
      monthly2Recurrency = text.toInt();
      yearlyValueDay = text.toInt();
      yearly2Recurrency = text.toInt();
    } else if ( tag == "month" ) {
      yearlyMonth = text.toInt() + 1; // starts at 0
      yearly2Month = text.toInt() + 1;
    } else if ( tag == fieldName( RecurrenceDelEx ) ) {
      QStringList exdates = text.split( "," );
      QStringList::ConstIterator it;
      for ( it = exdates.constBegin(); it != exdates.constEnd(); ++it )
        deleteExceptions.append( WebdavHandler::sloxToKDateTime( *it ).date() );
    }
  }

  if ( daysSet && type == "monthly" )
    type = "monthly2"; // HACK: OX doesn't cleanly distinguish between monthly and monthly2
  if ( daysSet && type == "yearly" )
    type = "yearly2";

  Recurrence *r = event->recurrence();

  if ( type == "daily" ) {
    r->setDaily( dailyValue );
  } else if ( type == "weekly" ) {
    r->setWeekly( weeklyValue, days );
  } else if ( type == "monthly" ) {
    r->setMonthly( monthlyValueMonth );
    r->addMonthlyDate( monthlyValueDay );
  } else if ( type == "yearly" ) {
    r->setYearly( 1 );
    r->addYearlyDate( yearlyValueDay );
    r->addYearlyMonth( yearlyMonth );
  } else if ( type == "monthly2" ) {
    r->setMonthly( monthly2ValueMonth );
    QBitArray _days( 7 );
    if ( daysSet )
      _days = days;
    else
      _days.setBit( event->dtStart().date().dayOfWeek() );
    r->addMonthlyPos( monthly2Recurrency, _days );
  } else if ( type == "yearly2" ) {
    r->setYearly( 1 );
    r->addYearlyMonth( yearly2Month );
    QBitArray _days( 7 );
    if ( daysSet )
      _days = days;
    else
      _days.setBit( ( yearly2Day + 5 ) % 7 );
    r->addYearlyPos( yearly2Recurrency, _days );
  }
  r->setEndDate( end.date() );
  r->setExDates( deleteExceptions );
}

void KCalResourceSlox::parseTodoAttribute( const QDomElement &e,
                                           Todo *todo )
{
  QString tag = e.tagName();
  QString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;

  if ( tag == fieldName( TaskBegin ) ) {
    KDateTime dt = WebdavHandler::sloxToKDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtStart( dt );
      todo->setHasStartDate( true );
    }
  } else if ( tag == fieldName( TaskEnd ) ) {
    KDateTime dt = WebdavHandler::sloxToKDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtDue( dt );
      todo->setHasDueDate( true );
    }
  } else if ( tag == fieldName( Priority ) ) {
    int p = text.toInt();
    if ( p < 1 || p > 3 ) {
      kError() << "Unknown priority:" << text;
    } else {
      int priority;
      switch ( p ) {
        case 1:
          priority = 9;
          break;
        default:
        case 2:
          priority = 5;
          break;
        case 3:
          priority = 1;
          break;
      }
      todo->setPriority( priority );
    }
  } else if ( tag == fieldName( PercentComplete ) ) {
    int completed = text.toInt();
    todo->setPercentComplete( completed );
  }
}

void KCalResourceSlox::slotLoadTodosResult( KJob *job )
{
  kDebug();

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kDebug() << "success";

    QDomDocument doc = mLoadTodosJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    disableChangeNotification();

    QList<SloxItem>::ConstIterator it;
    for( it = items.constBegin(); it != items.constEnd(); ++it ) {
      SloxItem item = *it;
      QString uid = sloxIdToTodoUid( item.sloxId );
      if ( item.status == SloxItem::Delete ) {
        Todo *todo = calendar()->todo( uid );
        if ( todo ) {
          calendar()->deleteTodo( todo );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Todo *newTodo = 0;
        Todo *todo = calendar()->todo( uid );
        if ( !todo ) {
          newTodo = new Todo;
          todo = newTodo;
          todo->setUid( uid );
          todo->setSecrecy( Incidence::SecrecyPrivate );
        }

        todo->setCustomProperty( "SLOX", "ID", item.sloxId );

        mWebdavHandler.clearSloxAttributeStatus();

        QDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          mWebdavHandler.parseSloxAttribute( e );
          parseIncidenceAttribute( e, todo );
          parseTodoAttribute( e, todo );
        }

        mWebdavHandler.setSloxAttributes( todo );

        if ( newTodo ) calendar()->addTodo( todo );

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

void KCalResourceSlox::slotLoadEventsResult( KJob *job )
{
  kDebug() << long( this );

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kDebug() << "success";

    QDomDocument doc = mLoadEventsJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    QList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    disableChangeNotification();

    QList<SloxItem>::ConstIterator it;
    for( it = items.constBegin(); it != items.constEnd(); ++it ) {
      SloxItem item = *it;
      QString uid = sloxIdToEventUid( item.sloxId );
      if ( item.status == SloxItem::Delete ) {
        Event *event = calendar()->event( uid );
        if ( event ) {
          calendar()->deleteEvent( event );
          changed = true;
        }
      } else if ( item.status == SloxItem::Create ) {
        Event *newEvent = 0;
        Event *event = calendar()->event( uid );
        if ( !event ) {
          newEvent = new Event;
          event = newEvent;
          event->setUid( uid );
          event->setSecrecy( Incidence::SecrecyPrivate );
        }

        event->setCustomProperty( "SLOX", "ID", item.sloxId );

        QDomNode n = item.domNode.namedItem( fieldName( FullTime ) );
        event->setAllDay( n.toElement().text() == boolToStr( true ) );

        bool doesRecur = false;

        mWebdavHandler.clearSloxAttributeStatus();

        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          QDomElement e = n.toElement();
          mWebdavHandler.parseSloxAttribute( e );
          parseIncidenceAttribute( e, event );
          parseEventAttribute( e, event );
          if ( e.tagName() == fieldName( RecurrenceType ) && e.text() != "no" ) {
            doesRecur = true;
          }
        }

        if ( doesRecur )
          parseRecurrence( item.domNode, event );
        else
          event->recurrence()->unsetRecurs();

        mWebdavHandler.setSloxAttributes( event );

//        kDebug() << "EVENT" << item.uid << event->summary();

        if ( newEvent ) calendar()->addEvent( event );

        changed = true;
      }
    }

    enableChangeNotification();

    saveToCache();

    clearChanges();

    if ( changed ) emit resourceChanged( this );

    emit resourceLoaded( this );
  }

  mLoadEventsJob = 0;

  if ( mLoadEventsProgress ) mLoadEventsProgress->setComplete();
  mLoadEventsProgress = 0;
}

void KCalResourceSlox::slotUploadResult( KJob *job )
{
  kDebug();

  if ( job->error() ) {
    saveError( job->errorString() );
  } else {
    kDebug() << "success";

    if ( !mUploadJob )
    {
        kDebug() << "mUploadJob was 0";
        return;
    }

    QDomDocument doc = mUploadJob->response();

    kDebug() << "UPLOAD RESULT:";
    kDebug() << doc.toString( 2 );

    QDomElement docElement = doc.documentElement();

    QDomNode responseNode;
    for( responseNode = docElement.firstChild(); !responseNode.isNull();
         responseNode = responseNode.nextSibling() ) {
      QDomElement responseElement = responseNode.toElement();
      if ( responseElement.tagName() == "response" ) {
        QDomNode propstat = responseElement.namedItem( "propstat" );
        if ( propstat.isNull() ) {
          kError() << "Unable to find propstat tag.";
          continue;
        }

        QDomNode status = propstat.namedItem( "status" );
        if ( !status.isNull() ) {
          QDomElement statusElement = status.toElement();
          QString response = statusElement.text();
        if ( !response.contains( "200" ) ) {
            QString error = '\'' + mUploadedIncidence->summary() + "'\n";
            error += response;
            QDomNode dn = propstat.namedItem( "responsedescription" );
            QString d = dn.toElement().text();
            if ( !d.isEmpty() ) error += '\n' + d;
            saveError( error );
            continue;
          }
        }

        QDomNode prop = propstat.namedItem( "prop" );
        if ( prop.isNull() ) {
          kError() << "Unable to find WebDAV property";
          continue;
        }

        QDomNode sloxIdNode = prop.namedItem( fieldName( ObjectId ) );
        if ( sloxIdNode.isNull() ) {
          kError() << "Unable to find SLOX id.";
          continue;
        }
        QDomElement sloxIdElement = sloxIdNode.toElement();
        QString sloxId = sloxIdElement.text();
        kDebug() << "SLOXID:" << sloxId;

        if ( mUploadIsDelete ) {
          kDebug() << "Incidence deleted";
        } else {
          QDomNode clientIdNode = prop.namedItem( fieldName( ClientId ) );
          if ( clientIdNode.isNull() ) {
            kError() << "Unable to find client id.";
            continue;
          }
          QDomElement clientidElement = clientIdNode.toElement();
          QString clientId = clientidElement.text();

          kDebug() << "CLIENTID:" << clientId;

          Incidence *i = mUploadedIncidence->clone();
          QString uid;
          if ( i->type() == "Event" ) uid = sloxIdToEventUid( sloxId );
          else if ( i->type() == "Todo" ) uid = sloxIdToTodoUid( sloxId );
          else {
            kError() << "Unknown type:"
                     << i->type();
          }
          i->setUid( uid );
          i->setCustomProperty( "SLOX", "ID", sloxId );

          disableChangeNotification();
          calendar()->deleteIncidence( mUploadedIncidence );
          calendar()->addIncidence( i );
          saveToCache();
          enableChangeNotification();

          emit resourceChanged( this );
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

void KCalResourceSlox::slotEventsProgress( KJob *job,
                                           unsigned long percent )
{
#if 0
  kDebug() << "PROGRESS: events" << int( job ) << ":" << percent;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadEventsProgress ) mLoadEventsProgress->setProgress( percent );
}

void KCalResourceSlox::slotTodosProgress( KJob *job, unsigned long percent )
{
#if 0
  kDebug() << "PROGRESS: todos" << int( job ) << ":" << percent;
#else
  Q_UNUSED( job );
  Q_UNUSED( percent );
#endif
  if ( mLoadTodosProgress ) mLoadTodosProgress->setProgress( percent );
}

void KCalResourceSlox::slotUploadProgress( KJob *job, unsigned long percent )
{
#if 0
  kDebug() << "PROGRESS: upload" << int( job ) << ":" << percent;
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

bool KCalResourceSlox::doSave( bool syncCache )
{
  kDebug();

  Q_UNUSED( syncCache );
  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mLoadEventsJob || mLoadTodosJob ) {
    kWarning() << "download still in progress.";
    return false;
  }
  if ( mUploadJob ) {
    kWarning() << "upload still in progress.";
    return false;
  }

  if ( !confirmSave() ) return false;

  saveToCache();

  uploadIncidences();

  return true;
}

bool KCalResourceSlox::doSave( bool syncCache, KCal::Incidence *incidence )
{
  Q_UNUSED( syncCache );
  Q_UNUSED( incidence );
  return true;
}

bool KCalResourceSlox::isSaving()
{
  return mUploadJob;
}

void KCalResourceSlox::doClose()
{
  kDebug();

  cancelLoadEvents();
  cancelLoadTodos();

  if ( mUploadJob ) {
    kError() << "Still saving";
  } else {
    calendar()->close();
  }
}

KABC::Lock *KCalResourceSlox::lock()
{
  return mLock;
}

void KCalResourceSlox::dump() const
{
  ResourceCalendar::dump();
  kDebug(5800) << "  Url:" << mPrefs->url();
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
