/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

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

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqfile.h>
#include <tqregexp.h>

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
#include <libkcal/confirmsavedialog.h>

#include <kabc/locknull.h>
#include <kabc/stdaddressbook.h>

#include <kresources/configwidget.h>

#include "webdavhandler.h"
#include "kcalsloxprefs.h"
#include "sloxaccounts.h"

#include "kcalresourceslox.h"

using namespace KCal;

KCalResourceSlox::KCalResourceSlox( const KConfig *config )
  : ResourceCached( config ), SloxBase( this )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) {
    readConfig( config );
  } else {
    setResourceName( i18n( "OpenXchange Server" ) );
  }
}

KCalResourceSlox::KCalResourceSlox( const KURL &url )
  : ResourceCached( 0 ), SloxBase( this )
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

void KCalResourceSlox::readConfig( const KConfig *config )
{
  mPrefs->readConfig();

  mWebdavHandler.setUserId( mPrefs->user() );

  ResourceCached::readConfig( config );

  KURL url = mPrefs->url();
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  delete mAccounts;
  mAccounts = new SloxAccounts( this, url );
}

void KCalResourceSlox::writeConfig( KConfig *config )
{
  kdDebug() << "KCalResourceSlox::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );
}

bool KCalResourceSlox::doLoad()
{
  kdDebug() << "KCalResourceSlox::load() " << long( this ) << endl;

  if ( mLoadEventsJob || mLoadTodosJob ) {
    kdDebug() << "KCalResourceSlox::load(): download still in progress."
                << endl;
    return true;
  }
  if ( mUploadJob ) {
    kdWarning() << "KCalResourceSlox::load(): upload still in progress."
                << endl;
    loadError( "Upload still in progress." );
    return false;
  }

  mCalendar.close();

  disableChangeNotification();
  loadCache();
  enableChangeNotification();

  emit resourceChanged( this );

  clearChanges();

  TQString p = KURL( mPrefs->url() ).protocol();
  if ( p != "http" && p != "https" && p != "webdav" && p != "webdavs" ) {
    TQString err = i18n("Non-http protocol: '%1'").arg( p );
    kdDebug() << "KCalResourceSlox::load(): " << err << endl;
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
  KURL url = mPrefs->url();
  url.setPath( "/servlet/webdav.calendar/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kdDebug() << "KCalResourceSlox::requestEvents(): " << url << endl;

  TQString lastsync = "0";
  if ( mPrefs->useLastSync() ) {
    TQDateTime dt = mPrefs->lastEventSync();
    if ( dt.isValid() ) {
      lastsync = WebdavHandler::qDateTimeToSlox( dt.addDays( -1 ) );
    }
  }

  TQDomDocument doc;
  TQDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  TQDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), lastsync );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->calendarFolderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kdDebug() << "REQUEST CALENDAR: \n" << doc.toString( 2 ) << endl;

  mLoadEventsJob = KIO::davPropFind( url, doc, "0", false );
  connect( mLoadEventsJob, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotLoadEventsResult( KIO::Job * ) ) );
  connect( mLoadEventsJob, TQT_SIGNAL( percent( KIO::Job *, unsigned long ) ),
           TQT_SLOT( slotEventsProgress( KIO::Job *, unsigned long ) ) );

  mLoadEventsProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading events") );
  connect( mLoadEventsProgress,
           TQT_SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           TQT_SLOT( cancelLoadEvents() ) );

  mPrefs->setLastEventSync( TQDateTime::currentDateTime() );
}

void KCalResourceSlox::requestTodos()
{
  KURL url = mPrefs->url();
  url.setPath( "/servlet/webdav.tasks/" );
  url.setUser( mPrefs->user() );
  url.setPass( mPrefs->password() );

  kdDebug() << "KCalResourceSlox::requestTodos(): " << url << endl;

  TQString lastsync = "0";
  if ( mPrefs->useLastSync() ) {
    TQDateTime dt = mPrefs->lastTodoSync();
    if ( dt.isValid() ) {
      lastsync = WebdavHandler::qDateTimeToSlox( dt.addDays( -1 ) );
    }
  }

  TQDomDocument doc;
  TQDomElement root = WebdavHandler::addDavElement( doc, doc, "propfind" );
  TQDomElement prop = WebdavHandler::addDavElement( doc, root, "prop" );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( LastSync ), lastsync );
  WebdavHandler::addSloxElement( this, doc, prop, fieldName( FolderId ), mPrefs->taskFolderId() );
  if ( type() == "ox" ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "NEW_AND_MODIFIED" );
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "DELETED" );
  } else
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectType ), "all" );

  kdDebug() << "REQUEST TASKS: \n" << doc.toString( 2 ) << endl;

  mLoadTodosJob = KIO::davPropFind( url, doc, "0", false );
  connect( mLoadTodosJob, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotLoadTodosResult( KIO::Job * ) ) );
  connect( mLoadTodosJob, TQT_SIGNAL( percent( KIO::Job *, unsigned long ) ),
           TQT_SLOT( slotTodosProgress( KIO::Job *, unsigned long ) ) );

  mLoadTodosProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Downloading to-dos") );
  connect( mLoadTodosProgress,
           TQT_SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           TQT_SLOT( cancelLoadTodos() ) );

  mPrefs->setLastTodoSync( TQDateTime::currentDateTime() );
}

void KCalResourceSlox::uploadIncidences()
{
  TQDomDocument doc;
  TQDomElement ms = WebdavHandler::addDavElement( doc, doc, "multistatus" );
  TQDomElement pu = WebdavHandler::addDavElement( doc, ms, "propertyupdate" );
  TQDomElement set = WebdavHandler::addElement( doc, pu, "D:set" );
  TQDomElement prop = WebdavHandler::addElement( doc, set, "D:prop" );

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

  // Don't try to upload recurring incidences as long as the resource doesn't
  // correctly write them in order to avoid corrupting data on the server.
  // FIXME: Remove when recurrences are correctly written.
  if ( mUploadedIncidence->doesRecur() && type() == "slox" ) {
    clearChange( mUploadedIncidence );
    uploadIncidences();
    return;
  }

  KURL url = mPrefs->url();

  TQString sloxId = mUploadedIncidence->customProperty( "SLOX", "ID" );
  if ( !sloxId.isEmpty() ) {
    WebdavHandler::addSloxElement( this, doc, prop, fieldName( ObjectId ), sloxId );
  } else {
    if ( mUploadIsDelete ) {
      kdError() << "Incidence to delete doesn't have a SLOX id" << endl;
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
      kdWarning() << "uploadIncidences(): Unsupported incidence type: "
                  << mUploadedIncidence->type() << endl;
      return;
    }

    if ( type() == "ox" ) {
      WebdavHandler::addSloxElement( this, doc, prop, "method", "DELETE" );
    } else {
      TQDomElement remove = WebdavHandler::addElement( doc, pu, "D:remove" );
      TQDomElement prop = WebdavHandler::addElement( doc, remove, "D:prop" );
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
  connect( mUploadJob, TQT_SIGNAL( result( KIO::Job * ) ),
           TQT_SLOT( slotUploadResult( KIO::Job * ) ) );
  connect( mUploadJob, TQT_SIGNAL( percent( KIO::Job *, unsigned long ) ),
           TQT_SLOT( slotUploadProgress( KIO::Job *, unsigned long ) ) );

  mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
      KPIM::ProgressManager::getUniqueID(), i18n("Uploading incidence") );
  connect( mUploadProgress,
           TQT_SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           TQT_SLOT( cancelUpload() ) );
}

void KCalResourceSlox::createIncidenceAttributes( TQDomDocument &doc,
                                                  TQDomElement &parent,
                                                  Incidence *incidence )
{
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( IncidenceTitle ),
                                 incidence->summary() );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Description ),
                                 incidence->description() );

  if ( incidence->attendeeCount() > 0 ) {
    TQDomElement members = WebdavHandler::addSloxElement( this, doc, parent,
        fieldName( Participants ) );
    Attendee::List attendees = incidence->attendees();
    Attendee::List::ConstIterator it;
    for( it = attendees.begin(); it != attendees.end(); ++it ) {
      if ( mAccounts ) {
        TQString userId = mAccounts->lookupId( (*it)->email() );
        TQString status;
        switch ( (*it)->status() ) {
          case Attendee::Accepted: status = "accept"; break;
          case Attendee::Declined: status = "decline"; break;
          default: status = "none"; break;
        }
        TQDomElement el = WebdavHandler::addSloxElement( this, doc, members, fieldName( Participant ), userId );
        el.setAttribute( "confirm", status );
      } else {
        kdError() << "KCalResourceSlox: No accounts set." << endl;
      }
    }
  }

  // set read attributes - if SecrecyPublic, set it to users
  // TODO OX support
  if ( incidence->secrecy() == Incidence::SecrecyPublic && type() != "ox" )
  {
    TQDomElement rights = WebdavHandler::addSloxElement( this, doc, parent, "readrights" );
    WebdavHandler::addSloxElement( this, doc, rights, "group", "users" );
  }

  // set reminder as the number of minutes to the start of the event
  KCal::Alarm::List alarms = incidence->alarms();
  if ( !alarms.isEmpty() && alarms.first()->hasStartOffset() && alarms.first()->enabled() )
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( Reminder ),
                                   TQString::number( (-1) * alarms.first()->startOffset().asSeconds() / 60 ) );
  else
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( Reminder ), "0" );

  // categories
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Categories ), incidence->categories().join( ", " ) );
}

void KCalResourceSlox::createEventAttributes( TQDomDocument &doc,
                                              TQDomElement &parent,
                                              Event *event )
{
  TQString folderId = mPrefs->calendarFolderId();
  if ( folderId.isEmpty() && type() == "ox" ) // SLOX and OX use diffrent default folders
    folderId = "-1";
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( FolderId ), folderId );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( EventBegin ),
      WebdavHandler::qDateTimeToSlox( event->dtStart(), timeZoneId() ) );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( EventEnd ),
      WebdavHandler::qDateTimeToSlox( event->dtEnd(), timeZoneId() ) );

  WebdavHandler::addSloxElement( this, doc, parent, fieldName( Location ), event->location() );

  if ( event->doesFloat() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( FullTime ), boolToStr( true ) );
  } else {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( FullTime ), boolToStr( false ) );
  }
}

void KCalResourceSlox::createTodoAttributes( TQDomDocument &doc,
                                             TQDomElement &parent,
                                             Todo *todo )
{
  TQString folderId = mPrefs->taskFolderId();
  if ( folderId.isEmpty() && type() == "ox" ) // SLOX and OX use diffrent default folders
    folderId = "-1";
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( FolderId ), folderId );

  if ( todo->hasStartDate() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( TaskBegin ),
        WebdavHandler::qDateTimeToSlox( todo->dtStart(), timeZoneId() ) );
  }

  if ( todo->hasDueDate() ) {
    WebdavHandler::addSloxElement( this, doc, parent, fieldName( TaskEnd ),
        WebdavHandler::qDateTimeToSlox( todo->dtDue(), timeZoneId() ) );
  }

  int priority = todo->priority();
  TQString txt;
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
                                 TQString::number( todo->percentComplete() ) );
}

void KCalResourceSlox::createRecurrenceAttributes( TQDomDocument &doc,
                                                   TQDomElement &parent,
                                                   KCal::Incidence *incidence )
{
  if ( !incidence->doesRecur() ) {
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
                                     TQString::number( r->frequency() ) );
      break;
    case Recurrence::rWeekly: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "weekly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceWeeklyFreq ),
                                     TQString::number( r->frequency() ) );
      // TODO: SLOX support
      int oxDays = 0;
      for ( int i = 0; i < 7; ++i ) {
        if ( r->days()[i] )
          oxDays += 1 << ( ( i + 1 ) % 7 );
      }
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "days", TQString::number( oxDays ) );
      break; }
    case Recurrence::rMonthlyDay:
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "monthly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthlyFreq ),
                                     TQString::number( r->frequency() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthlyDay ),
                                     TQString::number( r->monthDays().first() ) );
      break;
    case Recurrence::rMonthlyPos: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ),
                                     type() == "ox" ? "monthly" : "monthly2" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Freq ),
                                     TQString::number( r->frequency() ) );
      RecurrenceRule::WDayPos wdp = r->monthPositions().first();
      // TODO: SLOX support
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Day ),
                                     TQString::number( 1 << wdp.day() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceMonthly2Pos ),
                                     TQString::number( wdp.pos() ) );
      break; }
    case Recurrence::rYearlyMonth:
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ), "yearly" );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearlyDay ),
                                     TQString::number( r->yearDates().first() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearlyMonth ),
                                     TQString::number( r->yearMonths().first() + monthOffset ) );
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "interval", "1" );
      break;
    case Recurrence::rYearlyPos: {
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceType ),
                                     type() == "ox" ? "yearly" : "yearly2" );
      RecurrenceRule::WDayPos wdp = r->monthPositions().first();
      // TODO: SLOX support
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Day ),
                                     TQString::number( 1 << wdp.day() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Pos ),
                                     TQString::number( wdp.pos() ) );
      WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceYearly2Month ),
                                     TQString::number( r->yearMonths().first() + monthOffset ) );
      if ( type() == "ox" )
        WebdavHandler::addSloxElement( this, doc, parent, "interval", "1" );
      break; }
    default:
      kdDebug() << k_funcinfo << "unsupported recurrence type: " << r->recurrenceType() << endl;
  }
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceEnd ),
                                 WebdavHandler::qDateTimeToSlox( r->endDateTime() ) );
  // delete exceptions
  DateList exlist = r->exDates();
  TQStringList res;
  for ( DateList::Iterator it = exlist.begin(); it != exlist.end(); ++it )
    res.append( WebdavHandler::qDateTimeToSlox( *it ) );
  WebdavHandler::addSloxElement( this, doc, parent, fieldName( RecurrenceDelEx ), res.join( "," ) );
}

void KCalResourceSlox::parseMembersAttribute( const TQDomElement &e,
                                              Incidence *incidence )
{
  incidence->clearAttendees();

  TQDomNode n;
  for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement memberElement = n.toElement();
    if ( memberElement.tagName() == fieldName( Participant ) ) {
      TQString member = memberElement.text();
      KABC::Addressee account;
      if ( mAccounts ) account = mAccounts->lookupUser( member );
      else kdError() << "KCalResourceSlox: no accounts set" << endl;
      TQString name;
      TQString email;
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
      TQString status = memberElement.attribute( "confirm" );
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
      kdDebug() << "Unknown tag in members attribute: "
                << memberElement.tagName() << endl;
    }
  }
}

void KCalResourceSlox::parseReadRightsAttribute( const TQDomElement &e,
                                              Incidence *incidence )
{
  TQDomNode n;
  for( n = e.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement rightElement = n.toElement();
    if ( rightElement.tagName() == "group" ) {
      TQString groupName = rightElement.text();
      if ( groupName == "users" )
        incidence->setSecrecy( Incidence::SecrecyPublic );
    }
  }
}

void KCalResourceSlox::parseIncidenceAttribute( const TQDomElement &e,
                                                Incidence *incidence )
{
  TQString tag = e.tagName();
  TQString text = decodeText( e.text() );
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
    else kdDebug() << "KCalResourceSlox: no accounts set" << endl;
    incidence->setOrganizer( Person( a.formattedName(), a.preferredEmail() ) );
  } else if ( tag == fieldName( Participants ) ) {
    parseMembersAttribute( e, incidence );
  } else if ( tag == "readrights" ) {
    parseReadRightsAttribute( e, incidence );
  } else if ( tag == fieldName( Categories ) ) {
    incidence->setCategories( TQStringList::split( TQRegExp(",\\s*"), text ) );
  }
}

void KCalResourceSlox::parseEventAttribute( const TQDomElement &e,
                                            Event *event )
{
  TQString tag = e.tagName();
  TQString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;

  if ( tag == fieldName( EventBegin ) ) {
    TQDateTime dt;
    if ( event->doesFloat() ) {
      if ( type() == "ox" )
        dt = WebdavHandler::sloxToQDateTime( text, timeZoneId() );
      else
        dt = WebdavHandler::sloxToQDateTime( text ); // ### is this really correct for SLOX?
    } else
      dt = WebdavHandler::sloxToQDateTime( text, timeZoneId() );
    event->setDtStart( dt );
  } else if ( tag == fieldName( EventEnd ) ) {
    TQDateTime dt;
    if ( event->doesFloat() ) {
      dt = WebdavHandler::sloxToQDateTime( text );
      dt = dt.addSecs( -1 );
    }
    else dt = WebdavHandler::sloxToQDateTime( text, timeZoneId() );
    event->setDtEnd( dt );
  } else if ( tag == fieldName( Location ) ) {
    event->setLocation( text );
  }
}

void KCalResourceSlox::parseRecurrence( const TQDomNode &node, Event *event )
{
  TQString type;

  int dailyValue = -1;
  TQDateTime end;

  int weeklyValue = -1;
  TQBitArray days( 7 ); // days, starting with monday
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

  TQDomNode n;

  for( n = node.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    TQDomElement e = n.toElement();
    TQString tag = e.tagName();
    TQString text = decodeText( e.text() );
    kdDebug() << k_funcinfo << tag << ": " << text << endl;

    if ( tag == fieldName( RecurrenceType ) ) {
      type = text;
    } else if ( tag == "daily_value" ) {
      dailyValue = text.toInt();
    } else if ( tag == fieldName( RecurrenceEnd ) ) {
      end = WebdavHandler::sloxToQDateTime( text );
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
      TQStringList exdates = TQStringList::split( ",", text );
      TQStringList::Iterator it;
      for ( it = exdates.begin(); it != exdates.end(); ++it )
        deleteExceptions.append( WebdavHandler::sloxToQDateTime( *it ).date() );
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
    TQBitArray _days( 7 );
    if ( daysSet )
      _days = days;
    else
      _days.setBit( event->dtStart().date().dayOfWeek() );
    r->addMonthlyPos( monthly2Recurrency, _days );
  } else if ( type == "yearly2" ) {
    r->setYearly( 1 );
    r->addYearlyMonth( yearly2Month );
    TQBitArray _days( 7 );
    if ( daysSet )
      _days = days;
    else
      _days.setBit( ( yearly2Day + 5 ) % 7 );
    r->addYearlyPos( yearly2Recurrency, _days );
  }
  r->setEndDate( end.date() );
  r->setExDates( deleteExceptions );
}

void KCalResourceSlox::parseTodoAttribute( const TQDomElement &e,
                                           Todo *todo )
{
  TQString tag = e.tagName();
  TQString text = decodeText( e.text() );
  if ( text.isEmpty() ) return;

  if ( tag == fieldName( TaskBegin ) ) {
    TQDateTime dt = WebdavHandler::sloxToQDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtStart( dt );
      todo->setHasStartDate( true );
    }
  } else if ( tag == fieldName( TaskEnd ) ) {
    TQDateTime dt = WebdavHandler::sloxToQDateTime( text );
    if ( dt.isValid() ) {
      todo->setDtDue( dt );
      todo->setHasDueDate( true );
    }
  } else if ( tag == fieldName( Priority ) ) {
    int p = text.toInt();
    if ( p < 1 || p > 3 ) {
      kdError() << "Unknown priority: " << text << endl;
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

void KCalResourceSlox::slotLoadTodosResult( KIO::Job *job )
{
  kdDebug() << "KCalResourceSlox::slotLoadTodosJobResult()" << endl;

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadTodosJobResult() success" << endl;

    TQDomDocument doc = mLoadTodosJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    TQValueList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    disableChangeNotification();

    TQValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      TQString uid = sloxIdToTodoUid( item.sloxId );
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
          todo->setSecrecy( Incidence::SecrecyPrivate );
        }

        todo->setCustomProperty( "SLOX", "ID", item.sloxId );

        mWebdavHandler.clearSloxAttributeStatus();

        TQDomNode n;
        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          TQDomElement e = n.toElement();
          mWebdavHandler.parseSloxAttribute( e );
          parseIncidenceAttribute( e, todo );
          parseTodoAttribute( e, todo );
        }

        mWebdavHandler.setSloxAttributes( todo );

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
  kdDebug() << "KCalResourceSlox::slotLoadEventsResult() " << long( this ) << endl;

  if ( job->error() ) {
    loadError( job->errorString() );
  } else {
    kdDebug() << "KCalResourceSlox::slotLoadEventsResult() success" << endl;

    TQDomDocument doc = mLoadEventsJob->response();

    mWebdavHandler.log( doc.toString( 2 ) );

    TQValueList<SloxItem> items = WebdavHandler::getSloxItems( this, doc );

    bool changed = false;

    disableChangeNotification();

    TQValueList<SloxItem>::ConstIterator it;
    for( it = items.begin(); it != items.end(); ++it ) {
      SloxItem item = *it;
      TQString uid = sloxIdToEventUid( item.sloxId );
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
          event->setSecrecy( Incidence::SecrecyPrivate );
        }

        event->setCustomProperty( "SLOX", "ID", item.sloxId );

        TQDomNode n = item.domNode.namedItem( fieldName( FullTime ) );
        event->setFloats( n.toElement().text() == boolToStr( true ) );

        bool doesRecur = false;

        mWebdavHandler.clearSloxAttributeStatus();

        for( n = item.domNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
          TQDomElement e = n.toElement();
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

//        kdDebug() << "EVENT " << item.uid << " " << event->summary() << endl;

        if ( newEvent ) mCalendar.addEvent( event );

        changed = true;
      }
    }

    enableChangeNotification();

    saveCache();

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

    if ( !mUploadJob )
    {
        kdDebug() << "KCalResourceSlox::slotUploadResult() - mUploadJob was 0" << endl;
        return;
    }

    TQDomDocument doc = mUploadJob->response();

    kdDebug() << "UPLOAD RESULT:" << endl;
    kdDebug() << doc.toString( 2 ) << endl;

    TQDomElement docElement = doc.documentElement();

    TQDomNode responseNode;
    for( responseNode = docElement.firstChild(); !responseNode.isNull();
         responseNode = responseNode.nextSibling() ) {
      TQDomElement responseElement = responseNode.toElement();
      if ( responseElement.tagName() == "response" ) {
        TQDomNode propstat = responseElement.namedItem( "propstat" );
        if ( propstat.isNull() ) {
          kdError() << "Unable to find propstat tag." << endl;
          continue;
        }

        TQDomNode status = propstat.namedItem( "status" );
        if ( !status.isNull() ) {
          TQDomElement statusElement = status.toElement();
          TQString response = statusElement.text();
        if ( !response.contains( "200" ) ) {
            TQString error = "'" + mUploadedIncidence->summary() + "'\n";
            error += response;
            TQDomNode dn = propstat.namedItem( "responsedescription" );
            TQString d = dn.toElement().text();
            if ( !d.isEmpty() ) error += "\n" + d;
            saveError( error );
            continue;
          }
        }

        TQDomNode prop = propstat.namedItem( "prop" );
        if ( prop.isNull() ) {
          kdError() << "Unable to find WebDAV property" << endl;
          continue;
        }

        TQDomNode sloxIdNode = prop.namedItem( fieldName( ObjectId ) );
        if ( sloxIdNode.isNull() ) {
          kdError() << "Unable to find SLOX id." << endl;
          continue;
        }
        TQDomElement sloxIdElement = sloxIdNode.toElement();
        TQString sloxId = sloxIdElement.text();
        kdDebug() << "SLOXID: " << sloxId << endl;

        if ( mUploadIsDelete ) {
          kdDebug() << "Incidence deleted" << endl;
        } else {
          TQDomNode clientIdNode = prop.namedItem( fieldName( ClientId ) );
          if ( clientIdNode.isNull() ) {
            kdError() << "Unable to find client id." << endl;
            continue;
          }
          TQDomElement clientidElement = clientIdNode.toElement();
          TQString clientId = clientidElement.text();

          kdDebug() << "CLIENTID: " << clientId << endl;

          Incidence *i = mUploadedIncidence->clone();
          TQString uid;
          if ( i->type() == "Event" ) uid = sloxIdToEventUid( sloxId );
          else if ( i->type() == "Todo" ) uid = sloxIdToTodoUid( sloxId );
          else {
            kdError() << "KCalResourceSlox::slotUploadResult(): Unknown type: "
                      << i->type() << endl;
          }
          i->setUid( uid );
          i->setCustomProperty( "SLOX", "ID", sloxId );

          disableChangeNotification();
          mCalendar.deleteIncidence( mUploadedIncidence );
          mCalendar.addIncidence( i );
          saveCache();
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
  return result == TQDialog::Accepted;
}

bool KCalResourceSlox::doSave()
{
  kdDebug() << "KCalResourceSlox::save()" << endl;

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

  cancelLoadEvents();
  cancelLoadTodos();

  if ( mUploadJob ) {
    kdError() << "KCalResourceSlox::doClose() Still saving" << endl;
  } else {
    mCalendar.close();
  }
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

TQString KCalResourceSlox::sloxIdToEventUid( const TQString &sloxId )
{
  return "KResources_SLOX_Event_" + sloxId;
}

TQString KCalResourceSlox::sloxIdToTodoUid( const TQString &sloxId )
{
  return "KResources_SLOX_Todo_" + sloxId;
}

#include "kcalresourceslox.moc"
