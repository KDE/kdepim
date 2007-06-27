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

#include <kio/observer.h>
#include <kio/uiserver_stub.h>
#include <kapplication.h>
#include <dcopclient.h>
#include <libkcal/icalformat.h>
#include <libkdepim/kincidencechooser.h>
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

#include <assert.h>

using namespace KCal;
using namespace Kolab;

static const char* kmailCalendarContentsType = "Calendar";
static const char* kmailTodoContentsType = "Task";
static const char* kmailJournalContentsType = "Journal";
static const char* eventAttachmentMimeType = "application/x-vnd.kolab.event";
static const char* todoAttachmentMimeType = "application/x-vnd.kolab.task";
static const char* journalAttachmentMimeType = "application/x-vnd.kolab.journal";
static const char* incidenceInlineMimeType = "text/calendar";


ResourceKolab::ResourceKolab( const KConfig *config )
  : ResourceCalendar( config ), ResourceKolabBase( "ResourceKolab-libkcal" ),
    mCalendar( QString::fromLatin1("UTC") ), mOpen( false ),mResourceChangedTimer( 0,
        "mResourceChangedTimer" )
{
  setType( "imap" );
  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
}

ResourceKolab::~ResourceKolab()
{
  // The resource is deleted on exit (StdAddressBook's KStaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( mOpen ) {
    close();
  }
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
  config.setGroup( "General" );
  mProgressDialogIncidenceLimit = config.readNumEntry("ProgressDialogIncidenceLimit", 200);

  return openResource( config, kmailCalendarContentsType, mEventSubResources )
    && openResource( config, kmailTodoContentsType, mTodoSubResources )
    && openResource( config, kmailJournalContentsType, mJournalSubResources );
}

static void closeResource( KConfig& config, ResourceMap& map )
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

  KConfig config( configFile() );
  closeResource( config, mEventSubResources );
  closeResource( config, mTodoSubResources );
  closeResource( config, mJournalSubResources );
}

bool ResourceKolab::loadSubResource( const QString& subResource,
                                     const char* mimetype )
{
  int count = 0;
  if ( !kmailIncidencesCount( count, mimetype, subResource ) ) {
    kdError(5650) << "Communication problem in ResourceKolab::load()\n";
    return false;
  }

  if ( !count )
    return true;

  const int nbMessages = 200; // read 200 mails at a time (see kabc resource)

  const QString labelTxt = !strcmp(mimetype, "application/x-vnd.kolab.task") ? i18n( "Loading tasks..." )
                           : !strcmp(mimetype, "application/x-vnd.kolab.journal") ? i18n( "Loading journals..." )
                           : i18n( "Loading events..." );
  const bool useProgress = qApp && qApp->type() != QApplication::Tty && count > mProgressDialogIncidenceLimit;
  if ( useProgress )
    (void)::Observer::self(); // ensure kio_uiserver is running
  UIServer_stub uiserver( "kio_uiserver", "UIServer" );
  int progressId = 0;
  if ( useProgress ) {
    progressId = uiserver.newJob( kapp->dcopClient()->appId(), true );
    uiserver.totalFiles( progressId, count );
    uiserver.infoMessage( progressId, labelTxt );
    uiserver.transferring( progressId, labelTxt );
  }

  for ( int startIndex = 0; startIndex < count; startIndex += nbMessages ) {
    QMap<Q_UINT32, QString> lst;
    if ( !kmailIncidences( lst, mimetype, subResource, startIndex, nbMessages ) ) {
      kdError(5650) << "Communication problem in ResourceKolab::load()\n";
      if ( progressId )
        uiserver.jobFinished( progressId );
      return false;
    }

    { // for RAII scoping below
      TemporarySilencer t( this );
      for( QMap<Q_UINT32, QString>::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
        addIncidence( mimetype, it.data(), subResource, it.key() );
      }
    }
    if ( progressId ) {
      uiserver.processedFiles( progressId, startIndex );
      uiserver.percent( progressId, 100 * startIndex / count );
    }

//    if ( progress.wasCanceled() ) {
//      uiserver.jobFinished( progressId );
//      return false;
//    }
  }

  if ( progressId )
    uiserver.jobFinished( progressId );
  return true;
}

bool ResourceKolab::doLoad()
{
  if (!mUidMap.isEmpty() ) {
    return true;
  }
  mUidMap.clear();

  return loadAllEvents() & loadAllTodos() & loadAllJournals();
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
void ResourceKolab::incidenceUpdatedSilent( KCal::IncidenceBase* incidencebase)
{
 const QString uid = incidencebase->uid();
  //kdDebug() << k_funcinfo << uid << endl;

  if ( mUidsPendingUpdate.contains( uid ) || mUidsPendingAdding.contains( uid ) ) {
    /* We are currently processing this event ( removing and readding or
     * adding it ). If so, ignore this update. Keep the last of these around
     * and process once we hear back from KMail on this event. */
    mPendingUpdates.replace( uid, incidencebase );
    return;
  }

  QString subResource;
  Q_UINT32 sernum = 0;
  if ( mUidMap.contains( uid ) ) {
    subResource = mUidMap[ uid ].resource();
    sernum = mUidMap[ uid ].serialNumber();
    mUidsPendingUpdate.append( uid );
  }
  sendKMailUpdate( incidencebase, subResource, sernum );

}
void ResourceKolab::incidenceUpdated( KCal::IncidenceBase* incidencebase )
{
  if ( incidencebase->isReadOnly() ) return;
  incidencebase->setSyncStatusSilent( KCal::Event::SYNCMOD );
  incidencebase->setLastModified( QDateTime::currentDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.
  incidenceUpdatedSilent( incidencebase );

}

void ResourceKolab::resolveConflict( KCal::Incidence* inc, const QString& subresource, Q_UINT32 sernum )
{
    if ( ! inc )
        return;
    if ( ! mResolveConflict ) {
        // we should do no conflict resolution
        delete inc;
        return;
    }
    const QString origUid = inc->uid();
    Incidence* local = mCalendar.incidence( origUid );
    Incidence* localIncidence = 0;
    Incidence* addedIncidence = 0;
    Incidence*  result = 0;
    if ( local ) {
      if (*local == *inc) {
        // real duplicate, remove the second one
        result = local;
      } else {
        KIncidenceChooser* ch = new KIncidenceChooser();
        ch->setIncidence( local ,inc );
        if ( KIncidenceChooser::chooseMode == KIncidenceChooser::ask ) {
          connect ( this, SIGNAL( useGlobalMode() ), ch, SLOT (  useGlobalMode() ) );
          if ( ch->exec() )
            if ( KIncidenceChooser::chooseMode != KIncidenceChooser::ask )
              emit useGlobalMode() ;
        }
        result = ch->getIncidence();
        delete ch;
      }
    } else {
      // nothing there locally, just take the new one. Can't Happen (TM)
      result = inc;
    }
    if ( result == local ) {
        delete inc;
        localIncidence = local;
    } else  if ( result == inc ) {
        addedIncidence = inc;
    } else if ( result == 0 ) { // take both
        addedIncidence = inc;
        addedIncidence->setSummary( i18n("Copy of: %1").arg( addedIncidence->summary() ) );
        addedIncidence->setUid( CalFormat::createUniqueId() );
        localIncidence = local;
    }
    bool silent = mSilent;
    mSilent = false;
    if ( !localIncidence ) {
        deleteIncidence( local ); // remove local from kmail
    }
    mUidsPendingDeletion.append( origUid );
    if ( addedIncidence  ) {
        sendKMailUpdate( addedIncidence, subresource, sernum );
    } else {
        kmailDeleteIncidence( subresource, sernum );// remove new from kmail
    }
    mSilent = silent;
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
                                     Q_UINT32 sernum )
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
    KTempFile* tempFile = new KTempFile;
    QCString decoded = KCodecs::base64Decode( QCString( (*it)->data() ) );
    tempFile->file()->writeBlock( decoded.data(), decoded.length() );
    tempFile->close();
    KURL url;
    url.setPath( tempFile->name() );
    attURLs.append( url.url() );
    attMimeTypes.append( (*it)->mimeType() );
    attNames.append( (*it)->label() );
  }
  QStringList deletedAtts;
  if ( kmailListAttachments( deletedAtts, subresource, sernum ) ) {
    for ( QStringList::ConstIterator it = attNames.constBegin(); it != attNames.constEnd(); ++it ) {
      deletedAtts.remove( *it );
    }
  }
  CustomHeaderMap customHeaders;
  if ( incidence->schedulingID() != incidence->uid() )
    customHeaders.insert( "X-Kolab-SchedulingID", incidence->schedulingID() );

  QString subject = incidencebase->uid();
  if ( !isXMLStorageFormat ) subject.prepend( "iCal " ); // conform to the old style

  // behold, sernum is an in-parameter
  const bool rc = kmailUpdate( subresource, sernum, data, mimetype, subject, customHeaders, attURLs, attMimeTypes, attNames, deletedAtts );
  // update the serial number
  if ( mUidMap.contains( incidencebase->uid() ) ) {
    mUidMap[ incidencebase->uid() ].setSerialNumber( sernum );
  }

  for( QValueList<KTempFile *>::Iterator it = tmpFiles.begin(); it != tmpFiles.end(); ++it ) {
    (*it)->setAutoDelete( true );
    delete (*it);
  }

  return rc;
}

bool ResourceKolab::addIncidence( KCal::Incidence* incidence, const QString& _subresource,
                                  Q_UINT32 sernum )
{
  Q_ASSERT( incidence );
  if ( !incidence ) return false;
  QString uid = incidence->uid();
  QString subResource = _subresource;

  Kolab::ResourceMap *map = &mEventSubResources; // don't use a ref here!

  const QString& type = incidence->type();
  if ( type == "Event" )
    map = &mEventSubResources;
  else if ( type == "Todo" )
    map = &mTodoSubResources;
  else if ( type == "Journal" )
    map = &mJournalSubResources;
  else
    kdWarning() << "unknown type " << type << endl;

  if ( !mSilent ) { /* We got this one from the user, tell KMail. */
    // Find out if this event was previously stored in KMail
    bool newIncidence = _subresource.isEmpty();
    if ( newIncidence ) {
      // Add a description of the incidence
      QString text = "<b><font size=\"+1\">";
      if ( incidence->type() == "Event" )
        text += i18n( "Choose the folder where you want to store this event" );
      else if ( incidence->type() == "Todo" )
        text += i18n( "Choose the folder where you want to store this task" );
      else
        text += i18n( "Choose the folder where you want to store this incidence" );
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
        if ( event->hasEndDate() )
          if ( !event->doesFloat() )
            text += i18n( "<b>End:</b> %1, %2" )
                    .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
          else
            text += i18n( "<b>End:</b> %1" ).arg( event->dtEndDateStr() );
        text += "<br>";
      }
      subResource = findWritableResource( *map, text );
    }

    if ( subResource.isEmpty() )
      return false;

    mNewIncidencesMap.insert( uid, subResource );

    if ( !sendKMailUpdate( incidence, subResource, sernum ) ) {
      kdError(5650) << "Communication problem in ResourceKolab::addIncidence()\n";
      return false;
    } else {
      // KMail is doing it's best to add the event now, put a sticker on it,
      // so we know it's one of our transient ones
      mUidsPendingAdding.append( uid );

      /* Add to the cache immediately if this is a new event coming from
       * KOrganizer. It relies on the incidence being in the calendar when
       * addIncidence returns. */
      if ( newIncidence ) {
        mCalendar.addIncidence( incidence );
        incidence->registerObserver( this );
      }
    }
  } else { /* KMail told us */
    bool ourOwnUpdate = mUidsPendingUpdate.contains(  uid );
    /* Check if we updated this one, which means kmail deleted and added it.
     * We know the new state, so lets just not do much at all. The old incidence
     * in the calendar remains valid, but the serial number changed, so we need to
     * update that */
    if ( ourOwnUpdate ) {
      mUidsPendingUpdate.remove( uid );
      mUidMap.remove( uid );
      mUidMap[ uid ] = StorageReference( subResource, sernum );
    } else {
      /* This is a real add, from KMail, we didn't trigger this ourselves.
       * If this uid already exists in this folder, do conflict resolution,
       * unless the folder is read-only, in which case the user should not be
       * offered a means of putting mails in a folder she'll later be unable to
       * upload. Skip the incidence, in this case. */
      if ( mUidMap.contains( uid ) ) {
        if ( mUidMap[ uid ].resource() == subResource ) {
          if ( (*map)[ subResource ].writable() ) {
            resolveConflict( incidence, subResource, sernum );
          } else {
            kdWarning( 5650 ) << "Duplicate event in a read-only folder detected! "
              "Please inform the owner of the folder. " << endl;
          }
          return true;
        } else {
          // duplicate uid in a different folder, do the internal-uid tango
          incidence->setSchedulingID( uid );
          incidence->setUid(CalFormat::createUniqueId( ) );
          uid = incidence->uid();
        }
      }
      /* Add to the cache if the add didn't come from KOrganizer, in which case
       * we've already added it, and listen to updates from KOrganizer for it. */
      if ( !mUidsPendingAdding.contains( uid ) ) {
        mCalendar.addIncidence( incidence );
        incidence->registerObserver( this );
      }
      if ( !subResource.isEmpty() && sernum != 0 ) {
        mUidMap[ uid ] = StorageReference( subResource, sernum );
        incidence->setReadOnly( !(*map)[ subResource ].writable() );
      }
    }
    /* Check if there are updates for this uid pending and if so process them. */
    if ( KCal::IncidenceBase *update = mPendingUpdates.find( uid ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( uid );
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


bool ResourceKolab::addEvent( KCal::Event* event )
{
  if ( mUidMap.contains( event->uid() ) )
    return true; //noop
  else
    return addIncidence( event, QString::null, 0 );
}

void ResourceKolab::addEvent( const QString& xml, const QString& subresource,
                              Q_UINT32 sernum )
{
  KCal::Event* event = Kolab::Event::xmlToEvent( xml, mCalendar.timeZoneId(), this, subresource, sernum );
  Q_ASSERT( event );
  if( event ) {
      addIncidence( event, subresource, sernum );
  }
}

bool ResourceKolab::deleteIncidence( KCal::Incidence* incidence )
{
  if ( incidence->isReadOnly() ) return false;

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

bool ResourceKolab::addTodo( KCal::Todo* todo )
{
  if ( mUidMap.contains( todo->uid() ) )
    return true; //noop
  else
    return addIncidence( todo, QString::null, 0 );
}

void ResourceKolab::addTodo( const QString& xml, const QString& subresource,
                             Q_UINT32 sernum )
{
  KCal::Todo* todo = Kolab::Task::xmlToTask( xml, mCalendar.timeZoneId(), this, subresource, sernum );
  Q_ASSERT( todo );
  if( todo )
      addIncidence( todo, subresource, sernum );
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

bool ResourceKolab::addJournal( KCal::Journal* journal )
{
  if ( mUidMap.contains( journal->uid() ) )
    return true; //noop
  else
    return addIncidence( journal, QString::null, 0 );
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
  bool rc = true;
  TemporarySilencer t( this ); // RAII
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return false;
  if ( !subresourceActive( subResource ) ) return true;

  if ( format == KMailICalIface::StorageXML ) {
    // If this data file is one of ours, load it here
    if ( type == kmailCalendarContentsType )
      addEvent( data, subResource, sernum );
    else if ( type == kmailTodoContentsType )
      addTodo( data, subResource, sernum );
    else if ( type == kmailJournalContentsType )
      addJournal( data, subResource, sernum );
    else
      rc = false;
  } else {
    Incidence *inc = mFormat.fromString( data );
    if ( !inc )
      rc = false;
    else
      addIncidence( inc, subResource, sernum );
  }
  return rc;
}

void ResourceKolab::fromKMailDelIncidence( const QString& type,
                                           const QString& subResource,
                                           const QString& uid )
{
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return;
  if ( !subresourceActive( subResource ) ) return;

  // Can't be in both, by contract
  if ( mUidsPendingDeletion.find( uid ) != mUidsPendingDeletion.end() ) {
    mUidsPendingDeletion.remove( mUidsPendingDeletion.find( uid ) );
  } else if ( mUidsPendingUpdate.contains( uid ) ) {
    // It's good to know if was deleted, but we are waiting on a new one to
    // replace it, so let's just sit tight.
  } else {
    // We didn't trigger this, so KMail did, remove the reference to the uid
    KCal::Incidence* incidence = mCalendar.incidence( uid );
    if( incidence ) {
      incidence->unRegisterObserver( this );
      mCalendar.deleteIncidence( incidence );
    }
    mUidMap.remove( uid );
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
    while ( mapIt != mUidMap.end() )
    {
        Kolab::UidMap::Iterator it = mapIt++;
        const StorageReference ref = it.data();
        if ( ref.resource() != subResource ) continue;
        // FIXME incidence() is expensive
        KCal::Incidence* incidence = mCalendar.incidence( it.key() );
        if( incidence ) {
            incidence->unRegisterObserver( this );
            mCalendar.deleteIncidence( incidence );
        }
        mUidMap.remove( it );
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

#include "resourcekolab.moc"
