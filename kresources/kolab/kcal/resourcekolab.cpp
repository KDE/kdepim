/*
    This file is part of the kolab resource - the implementation of the
    Kolab storage format. See www.kolab.org for documentation on this.

    Copyright (c) 2004 Bo Thorsen <bo@sonofthor.dk>
                  2004 Till Adam <till@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "resourcekolab.h"
#include "event.h"
#include "task.h"
#include "journal.h"
#include "conflictpreventer.h"

#include <kapplication.h>
#include <dcopclient.h>
#include <libkdepim/kincidencechooser.h>
#include <libkcal/comparisonvisitor.h>
#include <kabc/locknull.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <ktempfile.h>
#include <kmdcodec.h>

#include <qfile.h>
#include <qobject.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qcursor.h>

#include <assert.h>

using namespace KPIM;
using namespace KCal;
using namespace Kolab;

static const char* kmailCalendarContentsType = "Calendar";
static const char* kmailTodoContentsType = "Task";
static const char* kmailJournalContentsType = "Journal";
static const char* eventAttachmentMimeType = "application/x-vnd.kolab.event";
static const char* todoAttachmentMimeType = "application/x-vnd.kolab.task";
static const char* journalAttachmentMimeType = "application/x-vnd.kolab.journal";
static const char* incidenceInlineMimeType = "text/calendar";

int ResourceKolab::sResourceCount = 0;

// flag to determine if a conflict has occurred during the current sync
static bool conflictThisSync = false;

ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceCalendar( config ), ResourceKolabBase( "ResourceKolab-libkcal" ),
    mCalendar( QString::fromLatin1("UTC") ), mOpen( false ),mResourceChangedTimer( 0,
        "mResourceChangedTimer" ), mBatchAddingInProgress( false ),
        mDequeingScheduled( false ), mConflictPreventer( new ConflictPreventer() )
{
  if ( !config ) {
    setResourceName( i18n( "Kolab Server" ) );
  }
  setType( "imap" );
  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
  ++sResourceCount;
}

ResourceKolab::~ResourceKolab()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( mOpen ) {
    close();
  }
  delete mConflictPreventer;
  --sResourceCount;
}

void ResourceKolab::loadSubResourceConfig( KConfig& config,
                                           const QString& name,
                                           const QString& label,
                                           bool writable,
                                           bool alarmRelevant,
                                           ResourceMap& subResource )
{
  KConfigGroup group( &config, name );
  bool active = group.readBoolEntry( "Active", true );
  subResource.insert( name, Kolab::SubResource( active, writable,
                                                alarmRelevant, label ) );
}

bool ResourceKolab::openResource( KConfig& config, const char* contentType,
                                  ResourceMap& map )
{
  // Read the subresource entries from KMail
  QValueList<KMailICalIface::SubResource> subResources;
  if ( !kmailSubresources( subResources, contentType ) )
    return false;
  map.clear();
  QValueList<KMailICalIface::SubResource>::ConstIterator it;
  for ( it = subResources.begin(); it != subResources.end(); ++it )
    loadSubResourceConfig( config, (*it).location, (*it).label, (*it).writable,
                           (*it).alarmRelevant, map );
  return true;
}

bool ResourceKolab::doOpen()
{
  if ( mOpen )
    // Already open
    return true;
  mOpen = true;

  KConfig config( configFile() );

  return openResource( config, kmailCalendarContentsType, mEventSubResources )
    && openResource( config, kmailTodoContentsType, mTodoSubResources )
    && openResource( config, kmailJournalContentsType, mJournalSubResources );
}

static void writeResourceConfig( KConfig& config, ResourceMap& map )
{
  ResourceMap::ConstIterator it;
  for ( it = map.begin(); it != map.end(); ++it ) {
    config.setGroup( it.key() );
    config.writeEntry( "Active", it.data().active() );
  }
}

void ResourceKolab::doClose()
{
  if ( !mOpen )
    // Not open
    return;
  mOpen = false;

  writeConfig();
}

bool ResourceKolab::loadSubResource( const QString& subResource,
                                     const char* mimetype )
{
  const QString labelTxt = !strcmp(mimetype, "application/x-vnd.kolab.task") ? i18n( "Loading tasks..." )
    : !strcmp(mimetype, "application/x-vnd.kolab.journal") ? i18n( "Loading journals..." )
    : i18n( "Loading events..." );

  QMap<Q_UINT32, QString> lst;
  QApplication::setOverrideCursor( QCursor(QCursor::WaitCursor) );
  if ( !kmailIncidences( lst, mimetype, subResource, 0, -1) ) {
    kdError(5650) << "Communication problem in ResourceKolab::load()\n";
    QApplication::restoreOverrideCursor();
    return false;
  }

  { // for RAII scoping below
    TemporarySilencer t( this );
    conflictThisSync = false;
    for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
      addIncidence( mimetype, it.data(), subResource, it.key() );
    }
  }
  QApplication::restoreOverrideCursor();
  return true;
}

bool ResourceKolab::doLoad()
{
  if (!mUidMap.isEmpty() ) {
    emit resourceLoaded( this );
    return true;
  }
  mUidMap.clear();

  bool result = loadAllEvents() & loadAllTodos() & loadAllJournals();
  if ( result ) {
    emit resourceLoaded( this );
  } else {
    // FIXME: anyone know if the resource correctly calls loadError()
    // if it has one?
  }

  return result;
}

bool ResourceKolab::doLoadAll( ResourceMap& map, const char* mimetype )
{
  bool rc = true;
  for ( ResourceMap::ConstIterator it = map.begin(); it != map.end(); ++it ) {
    if ( !it.data().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( it.key(), mimetype );
  }
  return rc;
}

bool ResourceKolab::loadAllEvents()
{
  removeIncidences( "Event" );
  mCalendar.deleteAllEvents();
  bool kolabStyle = doLoadAll( mEventSubResources, eventAttachmentMimeType );
  bool icalStyle = doLoadAll( mEventSubResources, incidenceInlineMimeType );
  return kolabStyle && icalStyle;
}

bool ResourceKolab::loadAllTodos()
{
  removeIncidences( "Todo" );
  mCalendar.deleteAllTodos();
  bool kolabStyle = doLoadAll( mTodoSubResources, todoAttachmentMimeType );
  bool icalStyle = doLoadAll( mTodoSubResources, incidenceInlineMimeType );

  return kolabStyle && icalStyle;
}

bool ResourceKolab::loadAllJournals()
{
  removeIncidences( "Journal" );
  mCalendar.deleteAllJournals();
  bool kolabStyle = doLoadAll( mJournalSubResources, journalAttachmentMimeType );
  bool icalStyle = doLoadAll( mJournalSubResources, incidenceInlineMimeType );

  return kolabStyle && icalStyle;
}

void ResourceKolab::removeIncidences( const QCString& incidenceType )
{
  Kolab::UidMap::Iterator mapIt = mUidMap.begin();
  while ( mapIt != mUidMap.end() )
  {
    Kolab::UidMap::Iterator it = mapIt++;
    // Check the type of this uid: event, todo or journal.
    // Need to look up in mCalendar for that. Given the implementation of incidence(uid),
    // better call event(uid), todo(uid) etc. directly.

    // A  faster but hackish way would probably be to check the type of the resource,
    // like mEventSubResources.find( it.data().resource() ) != mEventSubResources.end() ?
    const QString& uid = it.key();
    if ( incidenceType == "Event" && mCalendar.event( uid ) )
      mUidMap.remove( it );
    else if ( incidenceType == "Todo" && mCalendar.todo( uid ) )
      mUidMap.remove( it );
    else if ( incidenceType == "Journal" && mCalendar.journal( uid ) )
      mUidMap.remove( it );
  }
}

bool ResourceKolab::doSave()
{
  return true;
  /*
  return kmailTriggerSync( kmailCalendarContentsType )
      && kmailTriggerSync( kmailTodoContentsType )
      && kmailTriggerSync( kmailJournalContentsType );
  */
}
void ResourceKolab::incidenceUpdatedSilent( KCal::IncidenceBase* incidencebase )
{
  const QString uid = incidencebase->uid();
  //kdDebug() << k_funcinfo << uid << endl;
  kdDebug() << "incidenceUpdatedSilent " << incidencebase->dtStart() << endl;

  //IncidenceBase doesn't have revision(), downcast needed.
  Incidence *incidence = dynamic_cast<Incidence*>( incidencebase );

  if ( mUidsPendingUpdate.contains( uid ) || mUidsPendingAdding.contains( uid ) ) {
    /* We are currently processing this event ( removing and readding or
     * adding it ). If so, ignore this update. Keep the last of these around
     * and process once we hear back from KMail on this event. */
    mPendingUpdates.remove( uid );
    mPendingUpdates.insert( uid, incidencebase );
    kdDebug() << "incidenceUpdatedSilent(): skipping and updating mPendingUpdates with incidence "
              << incidencebase->dtStart() << endl;

    mConflictPreventer->cleanup( uid );
    return;
  }

  QString subResource;
  Q_UINT32 sernum = 0;
  if ( mUidMap.contains( uid ) ) {
    subResource = mUidMap[ uid ].resource();
    sernum = mUidMap[ uid ].serialNumber();
    if ( kmailMessageReadyForUpdate( subResource, sernum ) == KMailICalIface::Yes ) {
      KCal::ComparisonVisitor v;
      if ( v.compare( incidence, mUidMap[uid].incidenceCopy() ) ) {
        kdDebug() << "incidenceUpdatedSilent(): Skipping redundant change."
                  << "new dtEnd: " << incidence->dtEnd()
                  << " old dtEnd: " << mUidMap[uid].incidenceCopy()->dtEnd()
                  << endl;
        return;
      }
      if ( mConflictPreventer->isRegistered( incidence, subResource ) ) {
        kdDebug() << "incidenceUpdatedSilent(): incidence " << incidence->dtStart()
                  << " is already registered. Cleaning up conflict preventer cache" << endl;
        mConflictPreventer->cleanup( uid );
      }
      mUidsPendingUpdate.append( uid );
      mConflictPreventer->registerOldPayload( mUidMap[uid].incidenceCopy(), subResource );
      sendKMailUpdate( incidencebase, subResource, sernum );
    } else { // It's not ready yet, lets try later
      queueUpdate( incidencebase );
    }
  } else {
    // Why do we call sendKMailUpdate() with no resource? Doesn't look right.
    sendKMailUpdate( incidencebase, subResource, sernum );
  }
}
void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* incidencebase )
{
  if ( incidencebase->isReadOnly() ) {
    return;
  }

  incidencebase->setSyncStatusSilent( KCal::Event::SYNCMOD );
  incidencebase->setLastModified( QDateTime::currentDateTime() );

  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.
  incidenceUpdatedSilent( incidencebase );
}

static KCal::Incidence *takeOneByChooser( KIncidenceChooser *ch,
                                          KCal::Incidence *local, KCal::Incidence *remote )
{
  KCal::Incidence *result = 0;
  ch->setIncidences( local ,remote );
  if ( ch->exec() ) {
    result = ch->takeIncidence();
  }
  return result;
}

static KCal::Incidence *takeOneByMode( KIncidenceChooser::TakeMode takeMode,
                                       KCal::Incidence *local, KCal::Incidence *remote )
{
  KCal::Incidence *result = 0;
  switch( takeMode ) {
  case KIncidenceChooser::Newer:
    if ( local->lastModified() == remote->lastModified() ) {
      result = 0;
    } else if ( local->lastModified() >  remote->lastModified() ) {
      result = local;
    } else {
      result = remote;
    }
    break;
  case KIncidenceChooser::Remote:
    result = remote;
    break;
  case KIncidenceChooser::Local:
    result = local;
    break;
  case KIncidenceChooser::Both:
    result = 0;
    break;
  }
  return result;
}

void ResourceKolab::resolveConflict( KCal::Incidence *remoteIncidence,
                                     const QString& subresource,
                                     Q_UINT32 sernum )
{
  if ( !remoteIncidence ) {
    return;
  }

  if ( !mResolveConflict ) {
    // we should do no conflict resolution
    delete remoteIncidence;
    return;
  }

  static bool conflictThisSession = false;

  const QString origUid = remoteIncidence->uid();
  Incidence* localIncidence = mCalendar.incidence( origUid );
  Incidence* addedIncidence = 0;
  Incidence* chosenIncidence = 0;
  if ( localIncidence ) {
    KCal::ComparisonVisitor v;
    if ( v.compare( localIncidence, remoteIncidence ) ) {
      // real duplicate, we keep the second one.
      chosenIncidence = localIncidence; // Just like a "Take Local"
    } else {
      // We have a conflict.
      //1. look in our rc file and see if this folder has a takeMode and askPolicy already
      //2. if not, look in our rc file and see if there is a global takeMode and askPolicy already
      //3. if not, use fallback defaults for takeMode and askPolicy
      KConfig config( configFile() );

      bool folderOnly = true;
      KIncidenceChooser::ConflictAskPolicy askPolicy = KIncidenceChooser::Always;
      KIncidenceChooser::TakeMode takeMode = KIncidenceChooser::Newer;

      config.setGroup( "Conflicts" );
      askPolicy =
        (KIncidenceChooser::ConflictAskPolicy)config.readNumEntry( "AskPolicy", int( askPolicy ) );
      takeMode =
        (KIncidenceChooser::TakeMode)config.readNumEntry( "TakeMode", int( takeMode ) );

      config.setGroup( subresource );
      askPolicy =
        (KIncidenceChooser::ConflictAskPolicy)config.readNumEntry( "AskPolicy", int( askPolicy ) );
      takeMode =
        (KIncidenceChooser::TakeMode)config.readNumEntry( "TakeMode", int( takeMode ) );

      bool chooser = true;
      switch( askPolicy ) {
      case KIncidenceChooser::Always:
        break;

      case KIncidenceChooser::Sync:
        if ( !conflictThisSync ) {
          conflictThisSync = true;
        } else {
          chooser = false;
        }
        break;

      case KIncidenceChooser::Session:
        if ( !conflictThisSession ) {
          conflictThisSession = true;
        } else {
          chooser = false;
        }
        break;

      case KIncidenceChooser::Never:
        chooser = false;
        break;
      }

      if ( chooser ) {
        KIncidenceChooser *ch =
          new KIncidenceChooser( labelForSubresource( subresource ), askPolicy, folderOnly, qApp->mainWidget() );
        chosenIncidence = takeOneByChooser( ch, localIncidence, remoteIncidence );

        if ( ch->folderOnly() ) {
          // write settings for this folder only
          config.setGroup( subresource );
          config.writeEntry( "AskPolicy", int( ch->conflictAskPolicy() ) );
          config.writeEntry( "TakeMode", int( ch->takeMode() ) );
        } else {
          // write settings for global
          config.setGroup( "Conflicts" );
          config.writeEntry( "AskPolicy", int( ch->conflictAskPolicy() ) );
          config.writeEntry( "TakeMode", int( ch->takeMode() ) );
        }
        delete ch;
      } else {
        chosenIncidence = takeOneByMode( takeMode, localIncidence, remoteIncidence );
      }
      conflictThisSession = true;
      conflictThisSync = true;
    }
  } else {
    // nothing there locally, just take the new one. Can't Happen (TM)
    kdWarning() << k_funcinfo << "; local incidence wasn't found in the calendar";
    chosenIncidence = remoteIncidence;
  }

  if ( chosenIncidence == localIncidence ) { // Take Local
    // This code block implements issue4805 and issue4808:
    // To avoid a race between kolab clients that can leed to data loss, we delete both remote
    // and local, and create a new imap object with the same content of the local.
    // This update will trigger an add and a delete, when we hear back from the add, we can
    // safely delete the "Remote"

    //kdDebug() << "DEBUG Local conflict with id " << localIncidence->uid() << origUid
    //          << " and scheduling id " << localIncidence->schedulingID() << endl;
    mPendingDuplicateDeletions.insert( origUid, StorageReference( subresource, sernum, localIncidence ) );
    const bool success = sendKMailUpdate( localIncidence, mUidMap[origUid].resource(),
                                          mUidMap[origUid].serialNumber(), /*force=*/true );
    Q_UNUSED( success );
    //kdDebug()<< "DEBUG Success was " << ( success )<< mUidMap[origUid].resource()
    //         << QString::number( mUidMap[origUid].serialNumber() ) << endl;
    delete remoteIncidence;
  } else { // Take Remote or Take Both
    addedIncidence = remoteIncidence;
    if ( chosenIncidence == remoteIncidence ) { // Take Remote
      if ( localIncidence ) { // Should always be valid
        const bool silent = mSilent;
        mSilent = false;
        deleteIncidence( localIncidence ); // remove local from kmail
        mSilent = silent;
      }
    } else if ( chosenIncidence == 0 ) { // Take Both
      addedIncidence->setSummary( i18n("Copy of: %1").arg( addedIncidence->summary() ) );
      addedIncidence->setUid( CalFormat::createUniqueId() );
      mUidsPendingDeletion.append( origUid );
      sendKMailUpdate( addedIncidence, subresource, sernum, /*force=*/true, /*uidChanged=*/true );
      return;
    }

    mUidsPendingDeletion.append( origUid );
    sendKMailUpdate( addedIncidence, subresource, sernum, /*force=*/true );
  }
}

void ResourceKolab::addIncidence( const char* mimetype, const QString& data,
                                  const QString& subResource, Q_UINT32 sernum )
{
  // This uses pointer comparison, so it only works if we use the static
  // objects defined in the top of the file
  if ( mimetype == eventAttachmentMimeType )
    addEvent( data, subResource, sernum );
  else if ( mimetype == todoAttachmentMimeType )
    addTodo( data, subResource, sernum );
  else if ( mimetype == journalAttachmentMimeType )
    addJournal( data, subResource, sernum );
  else if ( mimetype == incidenceInlineMimeType ) {
    Incidence *inc = mFormat.fromString( data );
    addIncidence( inc, subResource, sernum );
  }
}


bool ResourceKolab::sendKMailUpdate( KCal::IncidenceBase* incidencebase, const QString& subresource,
                                     Q_UINT32 sernum, bool forceTellKMail, bool uidChanged )
{
  const QString& type = incidencebase->type();
  const char* mimetype = 0;
  QString data;
  bool isXMLStorageFormat = kmailStorageFormat( subresource ) == KMailICalIface::StorageXML;
  if ( type == "Event" ) {
    if( isXMLStorageFormat ) {
      mimetype = eventAttachmentMimeType;
      data = Kolab::Event::eventToXML( static_cast<KCal::Event *>(incidencebase),
          mCalendar.timeZoneId() );
    } else {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Event *>(incidencebase),
          Scheduler::Request );
    }
  } else if ( type == "Todo" ) {
    if( isXMLStorageFormat ) {
      mimetype = todoAttachmentMimeType;
      data = Kolab::Task::taskToXML( static_cast<KCal::Todo *>(incidencebase),
          mCalendar.timeZoneId() );
    } else {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Todo *>(incidencebase),
          Scheduler::Request );
    }
  } else if ( type == "Journal" ) {
    if( isXMLStorageFormat ) {
      mimetype = journalAttachmentMimeType;
      data = Kolab::Journal::journalToXML( static_cast<KCal::Journal *>(incidencebase ),
          mCalendar.timeZoneId() );
    } else {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Journal *>(incidencebase),
          Scheduler::Request );
    }
  } else {
    kdWarning(5006) << "Can't happen: unhandled type=" << type << endl;
  }

//  kdDebug() << k_funcinfo << "Data string:\n" << data << endl;

  KCal::Incidence* incidence = static_cast<KCal::Incidence *>( incidencebase );

  KCal::Attachment::List atts = incidence->attachments();
  QStringList attURLs, attMimeTypes, attNames;
  QValueList<KTempFile*> tmpFiles;
  for ( KCal::Attachment::List::ConstIterator it = atts.constBegin(); it != atts.constEnd(); ++it ) {
    if ( (*it)->isUri() ) {
      continue;
    }
    KTempFile *tempFile = new KTempFile;
    if ( tempFile->status() == 0 ) { // open ok
      const QByteArray decoded = (*it)->decodedData() ;

      tempFile->file()->writeBlock( decoded.data(), decoded.count() );
      KURL url;
      url.setPath( tempFile->name() );
      attURLs.append( url.url() );
      attMimeTypes.append( (*it)->mimeType() );
      attNames.append( (*it)->label() );
      tempFile->close();
      tmpFiles.append( tempFile );
    } else {
      kdWarning(5006) << "Cannot open temporary file for attachment";
    }
  }
  QStringList deletedAtts;
  if ( kmailListAttachments( deletedAtts, subresource, sernum ) ) {
    for ( QStringList::ConstIterator it = attNames.constBegin(); it != attNames.constEnd(); ++it ) {
      deletedAtts.remove( *it );
    }
  }
  CustomHeaderMap customHeaders;
  if ( incidence->schedulingID() != incidence->uid() ) {
    customHeaders.insert( "X-Kolab-SchedulingID", incidence->schedulingID() );
  }

  QString subject = incidencebase->uid();
  if ( !isXMLStorageFormat ) subject.prepend( "iCal " ); // conform to the old style

  if ( mOriginalUID2fakeUID.contains( qMakePair( incidence->schedulingID(), subresource ) ) ) {
    // When we dance the internal UID Tango the UID differs from the subject until the next update
    kdDebug(5006) << "Updating fake uid: " << incidence->uid() << endl
                  << "For incidence: " << incidence->schedulingID() << endl;
    uidChanged = true;
  }

  // behold, sernum is an in-parameter
  //kdDebug() << "DEBUG with " << subject << incidence->uid() << incidence->schedulingID()
  //          << endl;

  if ( uidChanged ) {
    // XXX This is a hack to avoid changing the dcop interface to kmail. We piggy back on the
    // headers to tell Kmail that we actually want to change the UID with this update to
    // avoid a false positive on the subject <> uid safety check.
    customHeaders.insert( "X-KDE-internal-uidChanged", "true" );
  }

  const bool rc = kmailUpdate( forceTellKMail, subresource, sernum, data, mimetype, subject,
                               customHeaders, attURLs, attMimeTypes, attNames, deletedAtts );

  // update the serial number
  if ( mUidMap.contains( incidencebase->uid() ) ) {
    mUidMap[ incidencebase->uid() ].setSerialNumber( sernum );
  }

  for( QValueList<KTempFile *>::Iterator it = tmpFiles.begin(); it != tmpFiles.end(); ++it ) {
    (*it)->setAutoDelete( true );
    delete (*it);
  }

  incidencebase->resetDirtyFields();
  return rc;
}

bool ResourceKolab::addIncidence( KCal::Incidence* incidence, const QString& _subresource,
                                  Q_UINT32 sernum )
{
  Q_ASSERT( incidence );
  if ( !incidence ) {
    return false;
  }

  kdDebug() << "Resourcekolab, adding incidence "
            << incidence->summary()
            << "; subresource = " << _subresource
            << "; sernum = " << sernum
            << "; mBatchAddingInProgress = " << mBatchAddingInProgress
            << "; dtStart() = " << incidence->dtStart()
            << "; revision = " << incidence->revision()
            << "; uid = " << incidence->uid()
            << "; schedulingID = " << incidence->schedulingID()
            << "; mSilent = " << mSilent
            << endl
            << "; mPendingDuplicateDeletions.count() = " << mPendingDuplicateDeletions.count()
            << endl;

  QString uid = incidence->uid();
  QString subResource = _subresource;

  Kolab::ResourceMap *map = &mEventSubResources; // don't use a ref here!

  const QString& type = incidence->type();
  if ( type == "Event" ) {
    map = &mEventSubResources;
  } else if ( type == "Todo" ) {
    map = &mTodoSubResources;
  } else if ( type == "Journal" ) {
    map = &mJournalSubResources;
  } else {
    kdWarning() << "unknown type " << type << endl;
  }

  if ( !mSilent ) { /* We got this one from the user, tell KMail. */
    // Find out if this event was previously stored in KMail
    bool newIncidence = _subresource.isEmpty();
    if ( newIncidence ) {
      ResourceType type = Incidences;
      // Add a description of the incidence
      QString text = "<b><font size=\"+1\">";
      if ( incidence->type() == "Event" ) {
        type = Events;
        text += i18n( "Choose the folder where you want to store this event" );
      } else if ( incidence->type() == "Todo" ) {
        type = Tasks;
        text += i18n( "Choose the folder where you want to store this task" );
      } else {
        text += i18n( "Choose the folder where you want to store this incidence" );
      }
      text += "<font></b><br>";
      if ( !incidence->summary().isEmpty() )
        text += i18n( "<b>Summary:</b> %1" ).arg( incidence->summary() ) + "<br>";
      if ( !incidence->location().isEmpty() )
        text += i18n( "<b>Location:</b> %1" ).arg( incidence->location() );
      text += "<br>";
      if ( !incidence->doesFloat() )
        text += i18n( "<b>Start:</b> %1, %2" )
                .arg( incidence->dtStartDateStr(), incidence->dtStartTimeStr() );
      else
        text += i18n( "<b>Start:</b> %1" ).arg( incidence->dtStartDateStr() );
      text += "<br>";
      if ( incidence->type() == "Event" ) {
        Event* event = static_cast<Event*>( incidence );
        if ( event->hasEndDate() ) {
          if ( !event->doesFloat() ) {
            text += i18n( "<b>End:</b> %1, %2" )
                    .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
          } else {
            text += i18n( "<b>End:</b> %1" ).arg( event->dtEndDateStr() );
          }
        }
        text += "<br>";
      }

      // Lets not warn the user 100 times that there's no writable resource
      // and not ask 100 times which resource to use
      if ( !mBatchAddingInProgress || !mLastUsedResources.contains( type ) ) {
        subResource = findWritableResource( type, *map, text );
        mLastUsedResources[type] = subResource;
      } else {
        subResource = mLastUsedResources[type];
      }

      if ( subResource.isEmpty() ) {
        switch( mErrorCode ) {
        case NoWritableFound:
          setException( new ErrorFormat( ErrorFormat::NoWritableFound ) );
          break;
        case UserCancel:
          setException( new ErrorFormat( ErrorFormat::UserCancel ) );
          break;
        case NoError:
          break;
        }
      }
    }

    if ( subResource.isEmpty() ) {
      endAddingIncidences(); // cleanup
      kdDebug(5650) << "ResourceKolab: subResource is empty" << endl;
      return false;
    }

    mNewIncidencesMap.insert( uid, subResource );

    if ( !sendKMailUpdate( incidence, subResource, sernum ) ) {
      kdError(5650) << "Communication problem in ResourceKolab::addIncidence()\n";
      endAddingIncidences(); // cleanup
      return false;
    } else {
      // KMail is doing it's best to add the event now, put a sticker on it,
      // so we know it's one of our transient ones
      mUidsPendingAdding.append( uid );

      /* Add to the cache immediately if this is a new event coming from
       * KOrganizer. It relies on the incidence being in the calendar when
       * addIncidence returns. */
      if ( newIncidence || sernum == 0 ) {
        mCalendar.addIncidence( incidence );
       incidence->registerObserver( this );
      }
    }
  } else { /* KMail told us */

    const bool ourOwnUpdate = mUidsPendingUpdate.contains(  uid );
    kdDebug( 5650 ) << "addIncidence: ourOwnUpdate " << ourOwnUpdate << endl;

    { // This block fixes issue4826. Checks if the addition is a false positive conflict
      if ( mUidMap.contains( uid ) &&
           mConflictPreventer->processNewPayload( incidence, _subresource, sernum ) ) {
        if ( (mUidMap[uid].serialNumber() == sernum ) ) {
          kdWarning() << "Two incidences with the same SerialNumber. Ignoring: "
                      << mUidMap[uid].serialNumber() << endl;
          if ( ourOwnUpdate ) {
            // It can happen that we get here issue4871 got us here.
            // In that case we are about to process the same update twice
            // (triggering the conflict preventer) we also have the
            // correct serial number. Instead of understanding why
            // this is the case (No more time and my brain hurts)
            // We now remove the pending update as it is already processed.
            // This fixes the corrupted state and should be done anyway because
            // otherwise this uid is locked agains further modification.
            kdDebug() << "Removing uid from pending update list" << endl;
            mUidsPendingUpdate.remove( uid );
          }
        } else {
          const bool success = kmailDeleteIncidence( _subresource, sernum, /*force=*/true );
          if ( !success ) {
            // What do to in case if unsucess? log message
            kdWarning() << "Error deleting false positive conflict" << endl;
          }
        }
        return false;
      }
    }

    /* Check if we updated this one, which means kmail deleted and added it.
     * We know the new state, so lets just not do much at all. The old incidence
     * in the calendar remains valid, but the serial number changed, so we need to
     * update that */
    //kdDebug() << "DEBUG addIncidence: ourOwnUpdate=" << ourOwnUpdate << "; mPendingDuplicateDeletions.contains()= " << mPendingDuplicateDeletions.contains( uid ) << endl;
    if ( ourOwnUpdate ) {
      mUidsPendingUpdate.remove( uid );
      mUidMap.remove( uid );
      mUidMap[ uid ] = StorageReference( subResource, sernum, incidence );
    } else {
      /* This is a real add, from KMail, we didn't trigger this ourselves.
       * If this uid already exists in this folder, do conflict resolution,
       * unless the folder is read-only, in which case the user should not be
       * offered a means of putting mails in a folder she'll later be unable to
       * upload. Skip the incidence, in this case. */
      if ( mPendingDuplicateDeletions.contains( uid ) ) {
        mUidsPendingDeletion.append( uid );
        StorageReference toDelete = mPendingDuplicateDeletions[uid];
        const bool success = kmailDeleteIncidence( toDelete.resource(), toDelete.serialNumber(),
                                                   /*force=*/true );
        Q_UNUSED( success );
        mPendingDuplicateDeletions.remove( uid );
      }

      if ( mUidMap.contains( uid ) ) {
        if ( mUidMap[ uid ].resource() == subResource ) {
          if ( (*map)[ subResource ].writable() ) {
            kdDebug( 5650 ) << "lets resolve the conflict " << endl;
            resolveConflict( incidence, subResource, sernum );
          } else {
            kdWarning( 5650 ) << "Duplicate event in a read-only folder detected! "
              "Please inform the owner of the folder. " << endl;
          }
          return true;
        } else {
          // duplicate uid in a different folder, do the internal-uid tango
          kdDebug(5650 ) << "Doing internal-uid tango for incidence " << uid
                         << "; resource=" << subResource << endl;
          incidence->setSchedulingID( uid );

          incidence->setUid( CalFormat::createUniqueId( ) );
          uid = incidence->uid();

          /* Will be needed when kmail triggers a delete, so we don't delete the inocent
           * incidence that's sharing the uid with this one */
          mOriginalUID2fakeUID[qMakePair( incidence->schedulingID(), subResource )] = uid;
        }
      }
      /* Add to the cache if the add didn't come from KOrganizer, in which case
       * we've already added it, and listen to updates from KOrganizer for it. */
      if ( !mUidsPendingAdding.contains( uid ) ) {
        mCalendar.addIncidence( incidence );
        incidence->registerObserver( this );
      }
      if ( !subResource.isEmpty() && sernum != 0 ) {
        mUidMap.insert( uid, StorageReference( subResource, sernum, incidence ) );
        incidence->setReadOnly( !(*map)[ subResource ].writable() );
      }
    }
    /* Check if there are updates for this uid pending and if so process them. */
    if ( KCal::IncidenceBase *update = mPendingUpdates.find( uid ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( uid );
      mUidsPendingAdding.remove( uid );
      kdDebug() << "sendKMailUpdate(): firing pending update " << update->dtStart() << endl;
      incidenceUpdated( update );
    } else {
      /* If the uid was added by KMail, KOrganizer needs to be told, so
       * schedule emitting of the resourceChanged signal. */
      if ( !mUidsPendingAdding.contains( uid ) ) {
        if ( !ourOwnUpdate ) mResourceChangedTimer.changeInterval( 100 );
      } else {
        mUidsPendingAdding.remove( uid );
      }
    }

    mNewIncidencesMap.remove( uid );
  }
  return true;
}

bool ResourceKolab::addEvent( KCal::Event *event )
{
  return addEvent( event, QString() );
}

bool ResourceKolab::addEvent( KCal::Event *event, const QString &subResource )
{
  if ( mUidMap.contains( event->uid() ) ) {
    return true; //noop
  } else {
    return addIncidence( event, subResource, 0 );
  }
}

void ResourceKolab::addEvent( const QString& xml, const QString& subresource,
                              Q_UINT32 sernum )
{
  KCal::Event* event = Kolab::Event::xmlToEvent( xml, mCalendar.timeZoneId(), this, subresource, sernum );
  Q_ASSERT( event );
  if ( event ) {
      addIncidence( event, subresource, sernum );
  }
}

bool ResourceKolab::deleteIncidence( KCal::Incidence* incidence )
{
  if ( incidence->isReadOnly() ) {
    return false;
  }
  const int count = mQueuedIncidenceUpdates.count();
  for ( int i=0; i<count; ++i ) {
    KCal::IncidenceBase *queuedIncidence = mQueuedIncidenceUpdates[i];
    if ( queuedIncidence && queuedIncidence->uid() == incidence->uid() ) {
      mQueuedIncidenceUpdates.remove( queuedIncidence );
    }
  }

  mConflictPreventer->cleanup( incidence->uid() );

  const QString uid = incidence->uid();
  if( !mUidMap.contains( uid ) ) return false; // Odd
  /* The user told us to delete, tell KMail */
  if ( !mSilent ) {
    kmailDeleteIncidence( mUidMap[ uid ].resource(),
                          mUidMap[ uid ].serialNumber() );
    mUidsPendingDeletion.append( uid );
    incidence->unRegisterObserver( this );
    mCalendar.deleteIncidence( incidence );
    mUidMap.remove( uid );
  } else {
    assert( false ); // If this still happens, something is very wrong
  }
  return true;
}

bool ResourceKolab::deleteEvent( KCal::Event* event )
{
  return deleteIncidence( event );
}

KCal::Event* ResourceKolab::event( const QString& uid )
{
  return mCalendar.event(uid);
}

KCal::Event::List ResourceKolab::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection );
}

KCal::Event::List ResourceKolab::rawEventsForDate( const QDate& date,
                                                   EventSortField sortField,
                                                   SortDirection sortDirection )
{
  return mCalendar.rawEventsForDate( date, sortField, sortDirection );
}

KCal::Event::List ResourceKolab::rawEventsForDate( const QDateTime& qdt )
{
  return mCalendar.rawEventsForDate( qdt );
}

KCal::Event::List ResourceKolab::rawEvents( const QDate& start,
                                            const QDate& end,
                                            bool inclusive )
{
  return mCalendar.rawEvents( start, end, inclusive );
}

bool ResourceKolab::addTodo( KCal::Todo *todo )
{
  return addTodo( todo, QString() );
}

bool ResourceKolab::addTodo( KCal::Todo *todo, const QString &subResource )
{
  if ( mUidMap.contains( todo->uid() ) ) {
    return true; //noop
  } else {
    return addIncidence( todo, subResource, 0 );
  }
}

void ResourceKolab::addTodo( const QString& xml, const QString& subresource,
                             Q_UINT32 sernum )
{
  KCal::Todo* todo = Kolab::Task::xmlToTask( xml, mCalendar.timeZoneId(), this, subresource, sernum );
  Q_ASSERT( todo );
  if ( todo ) {
    addIncidence( todo, subresource, sernum );
  }
}

bool ResourceKolab::deleteTodo( KCal::Todo* todo )
{
  return deleteIncidence( todo );
}

KCal::Todo* ResourceKolab::todo( const QString& uid )
{
  return mCalendar.todo( uid );
}

KCal::Todo::List ResourceKolab::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
}

KCal::Todo::List ResourceKolab::rawTodosForDate( const QDate& date )
{
  return mCalendar.rawTodosForDate( date );
}

bool ResourceKolab::addJournal( KCal::Journal *journal )
{
  return addJournal( journal, QString() );
}

bool ResourceKolab::addJournal( KCal::Journal *journal, const QString &subResource )
{
  if ( mUidMap.contains( journal->uid() ) )
    return true; //noop
  else
    return addIncidence( journal, subResource, 0 );
}

void ResourceKolab::addJournal( const QString& xml, const QString& subresource,
                                Q_UINT32 sernum )
{
  KCal::Journal* journal =
    Kolab::Journal::xmlToJournal( xml, mCalendar.timeZoneId() );
  Q_ASSERT( journal );
  if( journal ) {
      addIncidence( journal, subresource, sernum );
  }
}

bool ResourceKolab::deleteJournal( KCal::Journal* journal )
{
  return deleteIncidence( journal );
}

KCal::Journal* ResourceKolab::journal( const QString& uid )
{
  return mCalendar.journal(uid);
}

KCal::Journal::List ResourceKolab::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

KCal::Journal::List ResourceKolab::rawJournalsForDate( const QDate &date )
{
  return mCalendar.rawJournalsForDate( date );
}

KCal::Alarm::List ResourceKolab::relevantAlarms( const KCal::Alarm::List &alarms )
{
  KCal::Alarm::List relevantAlarms;
  KCal::Alarm::List::ConstIterator it( alarms.begin() );
  while ( it != alarms.end() ) {
    KCal::Alarm *a = (*it);
    ++it;
    const QString &uid = a->parent()->uid();
     if ( mUidMap.contains( uid ) ) {
       const QString &sr = mUidMap[ uid ].resource();
       Kolab::SubResource *subResource = 0;
       if ( mEventSubResources.contains( sr ) )
          subResource = &( mEventSubResources[ sr ] );
       else if ( mTodoSubResources.contains( sr ) )
          subResource = &( mTodoSubResources[ sr ] );
       assert( subResource );
       if ( subResource->alarmRelevant() )
           relevantAlarms.append ( a );
       else {
         kdDebug(5650) << "Alarm skipped, not relevant." << endl;
       }
     }
  }
  return relevantAlarms;
}



KCal::Alarm::List ResourceKolab::alarms( const QDateTime& from,
                                         const QDateTime& to )
{
  return relevantAlarms( mCalendar.alarms( from, to ) );
}

KCal::Alarm::List ResourceKolab::alarmsTo( const QDateTime& to )
{
  return relevantAlarms( mCalendar.alarmsTo(to) );
}

void ResourceKolab::setTimeZoneId( const QString& tzid )
{
  mCalendar.setTimeZoneId( tzid );
  mFormat.setTimeZone( mCalendar.timeZoneId(), !mCalendar.isLocalTime() );
}

bool ResourceKolab::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           Q_UINT32 sernum,
                                           int format,
                                           const QString& data )
{
  kdDebug() << "fromKMailAddIncidence()" << subResource << endl;
  bool rc = true;
  TemporarySilencer t( this ); // RAII
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType ) {
    // Not ours
    return false;
  }

  //kdDebug() << "DEBUG fromKMailAddIncidence " << sernum << data << endl;

  if ( !subresourceActive( subResource ) ) {
    return true;
  }

  if ( format == KMailICalIface::StorageXML ) {
    // If this data file is one of ours, load it here
    if ( type == kmailCalendarContentsType ) {
      addEvent( data, subResource, sernum );
    } else if ( type == kmailTodoContentsType ) {
      addTodo( data, subResource, sernum );
    } else if ( type == kmailJournalContentsType ) {
      addJournal( data, subResource, sernum );
    } else {
      rc = false;
    }
  } else {
    Incidence *inc = mFormat.fromString( data );
    if ( inc ) {
      //kdDebug() << "DEBUG fromKMailAddIncidence: Received incidence with uid " << inc->uid() << endl;
      addIncidence( inc, subResource, sernum );
    } else {
      rc = false;
    }
  }
  return rc;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& subResource,
                                           const QString& uid,
                                           Q_INT32 sernum )
{
  kdDebug() << "fromKMailDelIncidence()" << endl;
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return;
  if ( !subresourceActive( subResource ) ) return;

  //kdDebug() << "DEBUG fromKMailDelIncidence " << uid << endl;

  if ( mConflictPreventer->isFalsePositive( subResource, sernum ) ) {
    mConflictPreventer->cleanup( uid, subResource, sernum );
    return;
  }

  // Can't be in both, by contract
  if ( mUidsPendingDeletion.find( uid ) != mUidsPendingDeletion.end() ) {
    mUidsPendingDeletion.remove( mUidsPendingDeletion.find( uid ) );
  } else if ( mUidsPendingUpdate.contains( uid ) ) {
    // It's good to know if was deleted, but we are waiting on a new one to
    // replace it, so let's just sit tight.
  } else {
    QString uidToUse;

    QPair<QString, QString> p( uid, subResource );
    if ( mOriginalUID2fakeUID.contains( p ) ) {
      // Incidence with the same uid in a different folder...
      // use the UID that addIncidence(...) generated
      uidToUse = mOriginalUID2fakeUID[p];
    } else {
      uidToUse = uid;
    }

    // We didn't trigger this, so KMail did, remove the reference to the uid
    KCal::Incidence* incidence = mCalendar.incidence( uidToUse );
    if( incidence ) {
      incidence->unRegisterObserver( this );
      mCalendar.deleteIncidence( incidence );
    }
    mUidMap.remove( uidToUse );
    mOriginalUID2fakeUID.remove( p );
    mResourceChangedTimer.changeInterval( 100 );
  }
}

void ResourceKolab::fromKMailRefresh( const QString& type,
                                      const QString& /*subResource*/ )
{
  // TODO: Only load the specified subResource
  if ( type == "Calendar" )
    loadAllEvents();
  else if ( type == "Task" )
    loadAllTodos();
  else if ( type == "Journal" )
    loadAllJournals();
  else
    kdWarning(5006) << "KCal Kolab resource: fromKMailRefresh: unknown type " << type << endl;
  mResourceChangedTimer.changeInterval( 100 );
}

void ResourceKolab::fromKMailAddSubresource( const QString& type,
                                             const QString& subResource,
                                             const QString& label,
                                             bool writable, bool alarmRelevant )
{
  ResourceMap* map = 0;
  const char* mimetype = 0;
  if ( type == kmailCalendarContentsType ) {
    map = &mEventSubResources;
    mimetype = eventAttachmentMimeType;
  } else if ( type == kmailTodoContentsType ) {
    map = &mTodoSubResources;
    mimetype = todoAttachmentMimeType;
  } else if ( type == kmailJournalContentsType ) {
    map = &mJournalSubResources;
    mimetype = journalAttachmentMimeType;
  } else
    // Not ours
    return;

  if ( map->contains( subResource ) )
    // Already registered
    return;

  KConfig config( configFile() );
  config.setGroup( subResource );

  bool active = config.readBoolEntry( subResource, true );
  (*map)[ subResource ] = Kolab::SubResource( active, writable,
                                              alarmRelevant, label );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource, label );
}

void ResourceKolab::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
  ResourceMap* map = subResourceMap( type );
  if ( !map ) // not ours
    return;
  if ( map->contains( subResource ) )
    map->erase( subResource );
  else
    // Not registered
    return;

  // Delete from the config file
  KConfig config( configFile() );
  config.deleteGroup( subResource );
  config.sync();

  unloadSubResource( subResource );

  emit signalSubresourceRemoved( this, type, subResource );
}

QStringList ResourceKolab::subresources() const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();
  return ( mEventSubResources.keys()
         + mTodoSubResources.keys()
         + mJournalSubResources.keys() );
}

const QString
ResourceKolab::labelForSubresource( const QString& subresource ) const
{
  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].label();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].label();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].label();
  return subresource;
}

void ResourceKolab::fromKMailAsyncLoadResult( const QMap<Q_UINT32, QString>& map,
                                              const QString& type,
                                              const QString& folder )
{
  TemporarySilencer t( this );
  conflictThisSync = false;
  for( QMap<Q_UINT32, QString>::ConstIterator it = map.begin(); it != map.end(); ++it )
    addIncidence( type.latin1(), it.data(), folder, it.key() );
}

bool ResourceKolab::subresourceActive( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();

  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].active();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].active();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].active();

  // Safe default bet:
  kdDebug(5650) << "subresourceActive( " << subresource << " ): Safe bet\n";

  return true;
}

void ResourceKolab::setSubresourceActive( const QString &subresource, bool v )
{
  ResourceMap *map = 0;
  const char* mimeType = 0;
  if ( mEventSubResources.contains( subresource ) ) {
     map = &mEventSubResources;
     mimeType = eventAttachmentMimeType;
  }
  if ( mTodoSubResources.contains( subresource ) ) {
     map = &mTodoSubResources;
     mimeType = todoAttachmentMimeType;
  }
  if ( mJournalSubResources.contains( subresource ) ) {
     map = &mJournalSubResources;
     mimeType = journalAttachmentMimeType;
  }

  if ( map && ( ( *map )[ subresource ].active() != v ) ) {
    ( *map )[ subresource ].setActive( v );
    if ( v ) {
        loadSubResource( subresource, mimeType );
    } else {
        unloadSubResource( subresource );
    }
    mResourceChangedTimer.changeInterval( 100 );
  }
  QTimer::singleShot( 0, this, SLOT(writeConfig()) );
}

bool ResourceKolab::subresourceWritable( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceKolab*>( this )->doOpen();

  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].writable();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].writable();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].writable();

  return false; //better a safe default
}

void ResourceKolab::slotEmitResourceChanged()
{
   kdDebug(5650) << "KCal Kolab resource: emitting resource changed " << endl;
   mResourceChangedTimer.stop();
   emit resourceChanged( this );
}

KABC::Lock* ResourceKolab::lock()
{
  return new KABC::LockNull( true );
}

Kolab::ResourceMap* ResourceKolab::subResourceMap( const QString& contentsType )
{
  if ( contentsType == kmailCalendarContentsType ) {
    return &mEventSubResources;
  } else if ( contentsType == kmailTodoContentsType ) {
    return &mTodoSubResources;
  } else if ( contentsType == kmailJournalContentsType ) {
    return &mJournalSubResources;
  }
  // Not ours
  return 0;
}


/*virtual*/
bool ResourceKolab::addSubresource( const QString& resource, const QString& parent )
{
   kdDebug(5650) << "KCal Kolab resource - adding subresource: " << resource << endl;
   QString contentsType = kmailCalendarContentsType;
   if ( !parent.isEmpty() ) {
     if ( mEventSubResources.contains( parent ) )
       contentsType = kmailCalendarContentsType;
     else if ( mTodoSubResources.contains( parent ) )
       contentsType = kmailTodoContentsType;
     else if ( mJournalSubResources.contains( parent ) )
       contentsType = kmailJournalContentsType;
   } else {
     QStringList contentTypeChoices;
     contentTypeChoices << i18n("Calendar") << i18n("Tasks") << i18n("Journals");
     const QString caption = i18n("Which kind of subresource should this be?");
     const QString choice = KInputDialog::getItem( caption, QString::null, contentTypeChoices );
     if ( choice == contentTypeChoices[0] )
       contentsType = kmailCalendarContentsType;
     else if ( choice == contentTypeChoices[1] )
       contentsType = kmailTodoContentsType;
     else if ( choice == contentTypeChoices[2] )
       contentsType = kmailJournalContentsType;
   }

   return kmailAddSubresource( resource, parent, contentsType );
}

/*virtual*/
bool ResourceKolab::removeSubresource( const QString& resource )
{
   kdDebug(5650) << "KCal Kolab resource - removing subresource: " << resource << endl;
   return kmailRemoveSubresource( resource );
}

/*virtual*/
QString ResourceKolab::subresourceIdentifier( Incidence *incidence )
{
  QString uid = incidence->uid();
  if ( mUidMap.contains( uid ) )
    return mUidMap[ uid ].resource();
  else
    if ( mNewIncidencesMap.contains( uid ) )
      return mNewIncidencesMap[ uid ];
    else
      return QString();
}


bool ResourceKolab::unloadSubResource( const QString& subResource )
{
    const bool silent = mSilent;
    mSilent = true;
    Kolab::UidMap::Iterator mapIt = mUidMap.begin();
    QPtrList<KCal::Incidence> incidences;
    while ( mapIt != mUidMap.end() )
    {
        Kolab::UidMap::Iterator it = mapIt++;
        const StorageReference ref = it.data();
        if ( ref.resource() != subResource ) continue;
        // FIXME incidence() is expensive
        KCal::Incidence* incidence = mCalendar.incidence( it.key() );
        if( incidence ) {
          // register all observers first before actually deleting them
          // in case of inter-incidence relations the other part will get
          // the change notification otherwise
          incidence->unRegisterObserver( this );
          incidences.append( incidence );
        }
        mUidMap.remove( it );
    }
    QPtrListIterator<KCal::Incidence> it( incidences );
    for ( ; it.current(); ++it ) {
      mCalendar.deleteIncidence( it.current() );
    }
    mSilent = silent;
    return true;
}

QString ResourceKolab::subresourceType( const QString &resource )
{
  if ( mEventSubResources.contains( resource ) )
    return "event";
  if ( mTodoSubResources.contains( resource ) )
    return "todo";
  if ( mJournalSubResources.contains( resource ) )
    return "journal";
  return QString();
}

void ResourceKolab::writeConfig()
{
  KConfig config( configFile() );
  writeResourceConfig( config, mEventSubResources );
  writeResourceConfig( config, mTodoSubResources );
  writeResourceConfig( config, mJournalSubResources );
}

void ResourceKolab::beginAddingIncidences()
{
  mBatchAddingInProgress = true;
}

void ResourceKolab::endAddingIncidences()
{
  mBatchAddingInProgress = false;
  mLastUsedResources.clear();
}

void ResourceKolab::queueUpdate( IncidenceBase *incidence )
{
  kdDebug() << "ResourceKolab::queueUpdate()" << endl;
  Q_ASSERT( incidence );
  const int count = mQueuedIncidenceUpdates.count();
  bool found = false;
  for ( int i=0; i<count; ++i ) {
    // Do some compression, we can discard older updates, they don't need to be sent to kmail
    IncidenceBase *queuedIncidence = mQueuedIncidenceUpdates[i];
    if ( queuedIncidence && queuedIncidence->uid() == incidence->uid() ) {
      mQueuedIncidenceUpdates[i] = incidence;
      found = true;
      break;
    }
  }

  if ( !found ) {
    mQueuedIncidenceUpdates << incidence;
  }

  if ( !mDequeingScheduled ) {
    mDequeingScheduled = true;
    QTimer::singleShot( 1000, this, SLOT(dequeueUpdates()));
  }
}

void ResourceKolab::dequeueUpdates()
{
  kdDebug() << "ResourceKolab::dequeueUpdates()" << endl;
  mDequeingScheduled = false;
  const int count = mQueuedIncidenceUpdates.count();
  // queueUpdate() will be called while we are still in the for loop below, so use a copy
  const QValueList<IncidenceBase*> listCopy = mQueuedIncidenceUpdates;
  mQueuedIncidenceUpdates.clear();

  for ( int i=0; i<count; ++i ) {
    if ( listCopy[i] && mUidMap.contains( listCopy[i]->uid() ) ) {
     incidenceUpdatedSilent( listCopy[i] );
    }
  }
}

/**static*/
int ResourceKolab::instanceCount()
{
  return sResourceCount;
}

#include "resourcekolab.moc"
