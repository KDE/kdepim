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

#include "resourcescalix.h"

#include <kapplication.h>
#include <kcal/comparisonvisitor.h>
#include <kcal/icalformat.h>
#include <libkdepim/kincidencechooser.h>
#include <kabc/locknull.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kinputdialog.h>
#include <ktemporaryfile.h>

#include <QObject>
#include <QTimer>
#include <QApplication>

#include <assert.h>

using namespace KCal;
using namespace KPIM;
using namespace Scalix;

static const char* kmailCalendarContentsType = "Calendar";
static const char* kmailTodoContentsType = "Task";
static const char* kmailJournalContentsType = "Journal";
static const char* eventAttachmentMimeType = "application/x-vnd.kolab.event";
static const char* todoAttachmentMimeType = "application/x-vnd.kolab.task";
static const char* journalAttachmentMimeType = "application/x-vnd.kolab.journal";
static const char* incidenceInlineMimeType = "text/calendar";


ResourceScalix::ResourceScalix( const KConfigGroup &config )
  : ResourceCalendar( config ), ResourceScalixBase( "ResourceScalix_libkcal" ),
    mCalendar( QString::fromLatin1("UTC") ), mOpen( false ),
    mResourceChangedTimer( 0 )
{
  setType( "scalix" );
  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
}

ResourceScalix::ResourceScalix()
  : ResourceCalendar(), ResourceScalixBase( "ResourceScalix_libkcal" ),
    mCalendar( QString::fromLatin1("UTC") ), mOpen( false ),
    mResourceChangedTimer( 0 )
{
  setType( "scalix" );
  connect( &mResourceChangedTimer, SIGNAL( timeout() ),
           this, SLOT( slotEmitResourceChanged() ) );
}

ResourceScalix::~ResourceScalix()
{
  // The resource is deleted on exit (StdAddressBook's K3StaticDeleter),
  // and it wasn't closed before that, so close here to save the config.
  if ( mOpen ) {
    close();
  }
}

void ResourceScalix::loadSubResourceConfig( KConfig& config,
                                           const QString& name,
                                           const QString& label,
                                           bool writable,
                                           bool alarmRelevant,
                                           ResourceMap& subResource )
{
  KConfigGroup group( &config, name );
  bool active = group.readEntry( "Active", true );
  subResource.insert( name, Scalix::SubResource( active, writable,
                                                alarmRelevant, label ) );
}

bool ResourceScalix::openResource( KConfig& config, const char* contentType,
                                  ResourceMap& map )
{
  // Read the subresource entries from KMail
  QList<KMail::SubResource> subResources;
  if ( !kmailSubresources( subResources, contentType ) )
    return false;
  map.clear();
  QList<KMail::SubResource>::ConstIterator it;
  for ( it = subResources.constBegin(); it != subResources.constEnd(); ++it )
    loadSubResourceConfig( config, (*it).location, (*it).label, (*it).writable,
                           (*it).alarmRelevant, map );
  return true;
}

bool ResourceScalix::doOpen()
{
  if ( mOpen )
    // Already open
    return true;
  mOpen = true;

  KConfig config( configFile() );
  KConfigGroup group = config.group( "General" );
  mProgressDialogIncidenceLimit = group.readEntry("ProgressDialogIncidenceLimit", 200);

  return openResource( config, kmailCalendarContentsType, mEventSubResources )
    && openResource( config, kmailTodoContentsType, mTodoSubResources )
    && openResource( config, kmailJournalContentsType, mJournalSubResources );
}

static void closeResource( KConfig& config, ResourceMap& map )
{
  ResourceMap::ConstIterator it;
  for ( it = map.constBegin(); it != map.constEnd(); ++it ) {
    KConfigGroup group = config.group( it.key() );
    group.writeEntry( "Active", it.value().active() );
  }
}

void ResourceScalix::doClose()
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

bool ResourceScalix::loadSubResource( const QString& subResource,
                                     const char* mimetype )
{
  int count = 0;
  if ( !kmailIncidencesCount( count, mimetype, subResource ) ) {
    kError(5650) <<"Communication problem in ResourceScalix::load()";
    return false;
  }

  if ( !count )
    return true;

  const int nbMessages = 200; // read 200 mails at a time (see kabc resource)

  const QString labelTxt = !strcmp(mimetype, "application/x-vnd.kolab.task") ? i18n( "Loading tasks..." )
                           : !strcmp(mimetype, "application/x-vnd.kolab.journal") ? i18n( "Loading journals..." )
                           : i18n( "Loading events..." );
  const bool useProgress = qApp && qApp->type() != QApplication::Tty && count > mProgressDialogIncidenceLimit;
//TODO port me kde4
#if 0
  //  if ( useProgress )
  //  (void)::Observer::self(); // ensure kio_uiserver is running
  int progressId = 0;
  QDBusInterface uiserver( "org.kde.kio_uiserver", "/UIServer", QString(), QDBusConnection::sessionBus() );
  if ( useProgress ) {
    QDBusReply<int> ret = uiserver.call( "newJob", );
    progressId = ret;
    <method name="newJob">
      <arg name="appServiceName" type="s" direction="in"/>
      <arg name="capabilities" type="i" direction="in"/>
      <arg name="showProgress" type="b" direction="in"/>
      <arg name="internalAppName" type="s" direction="in"/>
      <arg name="jobIcon" type="s" direction="in"/>
      <arg name="appName" type="s" direction="in"/>
      <arg name="jobId" type="i" direction="out"/>
    </method>
    progressId = uiserver.newJob( kapp->dcopClient()->appId(), true );
    uiserver.call( "totalFiles", progressId, count );
    uiserver.call( "infoMessage", progressId, labelTxt );

    //uiserver.transferring( progressId, labelTxt ); //TODO was removed
  }
#endif
  for ( int startIndex = 0; startIndex < count; startIndex += nbMessages ) {
    KMail::SernumDataPair::List lst;
    if ( !kmailIncidences( lst, mimetype, subResource, startIndex, nbMessages ) ) {
      kError(5650) <<"Communication problem in ResourceScalix::load()";
#if  0
      if ( progressId )
        uiserver.call( "jobFinished",  progressId, errorCode ); //TODO
#endif
      return false;
    }
    { // for RAII scoping below
      TemporarySilencer t( this );
      for( KMail::SernumDataPair::List::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
        addIncidence( mimetype, it->data, subResource, it->sernum );
      }
    }
#if 0
    if ( progressId ) {
      uiserver.call( "processedFiles", progressId, startIndex );
      uiserver.call( "percent", progressId, ( uint )( 100 * startIndex / count ) );
    }
#endif

//    if ( progress.wasCanceled() ) {
//      uiserver.jobFinished( progressId );
//      return false;
//    }
  }
#if 0
  if ( progressId )
    uiserver.call( "jobFinished",  progressId, errorCode ); //TODO
#endif
  return true;
}

bool ResourceScalix::doLoad( bool syncCache )
{
  Q_UNUSED( syncCache );
  if (!mUidMap.isEmpty() ) {
    return true;
  }
  mUidMap.clear();

  return loadAllEvents() & loadAllTodos() & loadAllJournals();
}

bool ResourceScalix::doLoadAll( ResourceMap& map, const char* mimetype )
{
  bool rc = true;
  for ( ResourceMap::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it ) {
    if ( !it.value().active() )
      // This resource is disabled
      continue;

    rc &= loadSubResource( it.key(), mimetype );
  }
  return rc;
}

bool ResourceScalix::loadAllEvents()
{
  removeIncidences( "Event" );
  mCalendar.deleteAllEvents();
  bool kolabStyle = doLoadAll( mEventSubResources, eventAttachmentMimeType );
  bool icalStyle = doLoadAll( mEventSubResources, incidenceInlineMimeType );
  return kolabStyle && icalStyle;
}

bool ResourceScalix::loadAllTodos()
{
  removeIncidences( "Todo" );
  mCalendar.deleteAllTodos();
  bool kolabStyle = doLoadAll( mTodoSubResources, todoAttachmentMimeType );
  bool icalStyle = doLoadAll( mTodoSubResources, incidenceInlineMimeType );

  return kolabStyle && icalStyle;
}

bool ResourceScalix::loadAllJournals()
{
  removeIncidences( "Journal" );
  mCalendar.deleteAllJournals();
  bool kolabStyle = doLoadAll( mJournalSubResources, journalAttachmentMimeType );
  bool icalStyle = doLoadAll( mJournalSubResources, incidenceInlineMimeType );

  return kolabStyle && icalStyle;
}

void ResourceScalix::removeIncidences( const QByteArray& incidenceType )
{
  Scalix::UidMap::Iterator mapIt = mUidMap.begin();
  while ( mapIt != mUidMap.end() )
  {
    Scalix::UidMap::Iterator it = mapIt++;
    // Check the type of this uid: event, todo or journal.
    // Need to look up in mCalendar for that. Given the implementation of incidence(uid),
    // better call event(uid), todo(uid) etc. directly.

    // A  faster but hackish way would probably be to check the type of the resource,
    // like mEventSubResources.find( it.value().resource() ) != mEventSubResources.constEnd() ?
    const QString& uid = it.key();
    if ( incidenceType == "Event" && mCalendar.event( uid ) )
      mUidMap.erase( it );
    else if ( incidenceType == "Todo" && mCalendar.todo( uid ) )
      mUidMap.erase( it );
    else if ( incidenceType == "Journal" && mCalendar.journal( uid ) )
      mUidMap.erase( it );
  }
}

bool ResourceScalix::doSave( bool syncCache )
{
  Q_UNUSED( syncCache );
  return true;
  /*
  return kmailTriggerSync( kmailCalendarContentsType )
      && kmailTriggerSync( kmailTodoContentsType )
      && kmailTriggerSync( kmailJournalContentsType );
  */
}

bool ResourceScalix::doSave( bool syncCache, KCal::Incidence *incidence )
{
  Q_UNUSED( syncCache );
  Q_UNUSED( incidence );
  return true;
}

void ResourceScalix::incidenceUpdatedSilent( KCal::IncidenceBase *incidencebase)
{
  if ( incidencebase->isReadOnly() ) return; // Should not happen (TM)
  const QString uid = incidencebase->uid();
  //kDebug() << uid;

  if ( mUidsPendingUpdate.contains( uid ) || mUidsPendingAdding.contains( uid ) ) {
    /* We are currently processing this event ( removing and readding or
     * adding it ). If so, ignore this update. Keep the last of these around
     * and process once we hear back from KMail on this event. */
    mPendingUpdates.remove( uid );
    mPendingUpdates.insert( uid, incidencebase );
    return;
  }

  QString subResource;
  quint32 sernum = 0;
  if ( mUidMap.contains( uid ) ) {
    subResource = mUidMap[ uid ].resource();
    sernum = mUidMap[ uid ].serialNumber();
    mUidsPendingUpdate.append( uid );
  }
  sendKMailUpdate( incidencebase, subResource, sernum );

}
void ResourceScalix::incidenceUpdated( KCal::IncidenceBase* incidencebase )
{
  if ( incidencebase->isReadOnly() ) return;
//TODO port me kde4
  //incidencebase->setSyncStatus( KCal::Event::SYNCMOD );
  incidencebase->setLastModified( KDateTime::currentUtcDateTime() );
  // we should probably update the revision number here,
  // or internally in the Event itself when certain things change.
  // need to verify with ical documentation.
  incidenceUpdatedSilent( incidencebase );
}

void ResourceScalix::resolveConflict( KCal::Incidence* inc, const QString& subresource, quint32 sernum )
{
    if ( ! inc )
        return;
    if ( ! isResolveConflictSet() ) {
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
      ComparisonVisitor visitor;
      if ( visitor.compare( local, inc ) ) {
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
        addedIncidence->setSummary( i18n("Copy of: %1", addedIncidence->summary() ) );
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
void ResourceScalix::addIncidence( const char* mimetype, const QString& data,
                                  const QString& subResource, quint32 sernum )
{
  // This uses pointer comparison, so it only works if we use the static
  // objects defined in the top of the file
  Incidence *inc = mFormat.fromString( data );
  addIncidence( inc, subResource, sernum );
}


bool ResourceScalix::sendKMailUpdate( KCal::IncidenceBase* incidencebase, const QString& subresource,
                                     quint32 sernum )
{
  const QString& type = incidencebase->type();
  const char* mimetype = 0;
  QString data;
  if ( type == "Event" ) {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Event *>(incidencebase),
          iTIPPublish );
  } else if ( type == "Todo" ) {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Todo *>(incidencebase),
          iTIPPublish );
  } else if ( type == "Journal" ) {
      mimetype = incidenceInlineMimeType;
      data = mFormat.createScheduleMessage( static_cast<KCal::Journal *>(incidencebase),
          iTIPPublish );
  } else {
    kWarning(5006) <<"Can't happen: unhandled type=" << type;
  }

//  kDebug() <<"Data string:" << data;

  KCal::Incidence* incidence = static_cast<KCal::Incidence *>( incidencebase );

  KCal::Attachment::List atts = incidence->attachments();
  QStringList attURLs, attMimeTypes, attNames;
  QList<KTemporaryFile*> tmpFiles;
  for ( KCal::Attachment::List::ConstIterator it = atts.constBegin(); it != atts.constEnd(); ++it ) {
    KTemporaryFile* tempFile = new KTemporaryFile;
    QByteArray decoded = QByteArray::fromBase64( (*it)->data() );
    tempFile->write( decoded );
    KUrl url;
    url.setPath( tempFile->fileName() );
    attURLs.append( url.url() );
    attMimeTypes.append( (*it)->mimeType() );
    attNames.append( (*it)->label() );
    tempFile->close();
  }
  QStringList deletedAtts;
  if ( kmailListAttachments( deletedAtts, subresource, sernum ) ) {
    for ( QStringList::ConstIterator it = attNames.constBegin(); it != attNames.constEnd(); ++it ) {
      deletedAtts.removeAll( *it );
    }
  }
  KMail::CustomHeader::List customHeaders;
  if ( type == "Event" )
    customHeaders << KMail::CustomHeader( "X-Scalix-Class", "IPM.Appointment" );
  else if ( type == "Todo" )
    customHeaders << KMail::CustomHeader( "X-Scalix-Class", "IPM.Task" );

  QString subject = incidence->summary();

  // behold, sernum is an in-parameter
  const bool rc = kmailUpdate( subresource, sernum, data, mimetype, subject, customHeaders, attURLs, attMimeTypes, attNames, deletedAtts );
  // update the serial number
  if ( mUidMap.contains( incidencebase->uid() ) ) {
    mUidMap[ incidencebase->uid() ].setSerialNumber( sernum );
  }

  for( QList<KTemporaryFile *>::Iterator it = tmpFiles.begin(); it != tmpFiles.end(); ++it ) {
    (*it)->setAutoRemove( true );
    delete (*it);
  }

  return rc;
}

bool ResourceScalix::addIncidence( KCal::Incidence* incidence, const QString& _subresource,
                                  quint32 sernum )
{
  Q_ASSERT( incidence );
  if ( !incidence ) return false;
  QString uid = incidence->uid();
  QString subResource = _subresource;

  Scalix::ResourceMap *map = &mEventSubResources; // don't use a ref here!

  const QString& type = incidence->type();
  if ( type == "Event" )
    map = &mEventSubResources;
  else if ( type == "Todo" )
    map = &mTodoSubResources;
  else if ( type == "Journal" )
    map = &mJournalSubResources;
  else
    kWarning() <<"unknown type" << type;

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
        text += i18n( "<b>Summary:</b> %1", incidence->summary() ) + "<br>";
      if ( !incidence->location().isEmpty() )
        text += i18n( "<b>Location:</b> %1", incidence->location() );
      text += "<br>";
      if ( !incidence->recurrence()->allDay() )
        text += i18n( "<b>Start:</b> %1, %2" )
                .arg( incidence->dtStartDateStr(), incidence->dtStartTimeStr() );
      else
        text += i18n( "<b>Start:</b> %1", incidence->dtStartDateStr() );
      text += "<br>";
      if ( incidence->type() == "Event" ) {
        Event* event = static_cast<Event*>( incidence );
        if ( event->hasEndDate() ) {
          if ( !event->recurrence()->allDay() ) {
            text += i18n( "<b>End:</b> %1, %2" )
                    .arg( event->dtEndDateStr(), event->dtEndTimeStr() );
          } else {
            text += i18n( "<b>End:</b> %1", event->dtEndDateStr() );
          }
        }
        text += "<br>";
      }
      subResource = findWritableResource( *map, text );
    }

    if ( subResource.isEmpty() )
      return false;

    mNewIncidencesMap.insert( uid, subResource );

    if ( !sendKMailUpdate( incidence, subResource, sernum ) ) {
      kError(5650) <<"Communication problem in ResourceScalix::addIncidence()";
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
      mUidsPendingUpdate.removeAll( uid );
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
            kWarning( 5650 ) <<"Duplicate event in a read-only folder detected!"
              "Please inform the owner of the folder.";
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
    if ( KCal::IncidenceBase *update = mPendingUpdates.value( uid ) ) {
      mSilent = false; // we do want to tell KMail
      mPendingUpdates.remove( uid );
      incidenceUpdated( update );
    } else {
      /* If the uid was added by KMail, KOrganizer needs to be told, so
       * schedule emitting of the resourceChanged signal. */
      if ( !mUidsPendingAdding.contains( uid ) ) {
        if ( !ourOwnUpdate ) mResourceChangedTimer.start( 100 );
      } else {
        mUidsPendingAdding.removeAll( uid );
      }
    }

    mNewIncidencesMap.remove( uid );
  }
  return true;
}


bool ResourceScalix::addEvent( KCal::Event* event )
{
  if ( mUidMap.contains( event->uid() ) )
    return true; //noop
  else
    return addIncidence( event, QString(), 0 );
}

bool ResourceScalix::deleteIncidence( KCal::Incidence* incidence )
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

bool ResourceScalix::deleteEvent( KCal::Event* event )
{
  return deleteIncidence( event );
}

KCal::Event* ResourceScalix::event( const QString& uid )
{
  return mCalendar.event(uid);
}

KCal::Event::List ResourceScalix::rawEvents( EventSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawEvents( sortField, sortDirection );
}

KCal::Event::List ResourceScalix::rawEventsForDate( const QDate& date,
                                                   const KDateTime::Spec& timespec,
                                                   EventSortField sortField,
                                                   SortDirection sortDirection )
{
  return mCalendar.rawEventsForDate( date, timespec, sortField, sortDirection );
}

KCal::Event::List ResourceScalix::rawEventsForDate( const KDateTime& dt )
{
  return mCalendar.rawEventsForDate( dt );
}

KCal::Event::List ResourceScalix::rawEvents( const QDate& start,
                                            const QDate& end,
                                            const KDateTime::Spec& timespec,
                                            bool inclusive )
{
  return mCalendar.rawEvents( start, end, timespec, inclusive );
}

bool ResourceScalix::addTodo( KCal::Todo* todo )
{
  if ( mUidMap.contains( todo->uid() ) )
    return true; //noop
  else
    return addIncidence( todo, QString(), 0 );
}

bool ResourceScalix::deleteTodo( KCal::Todo* todo )
{
  return deleteIncidence( todo );
}

KCal::Todo* ResourceScalix::todo( const QString& uid )
{
  return mCalendar.todo( uid );
}

KCal::Todo::List ResourceScalix::rawTodos( TodoSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawTodos( sortField, sortDirection );
}

KCal::Todo::List ResourceScalix::rawTodosForDate( const QDate& date )
{
  return mCalendar.rawTodosForDate( date );
}

bool ResourceScalix::addJournal( KCal::Journal* journal )
{
  if ( mUidMap.contains( journal->uid() ) )
    return true; //noop
  else
    return addIncidence( journal, QString(), 0 );
}

bool ResourceScalix::deleteJournal( KCal::Journal* journal )
{
  return deleteIncidence( journal );
}

KCal::Journal* ResourceScalix::journal( const QString& uid )
{
  return mCalendar.journal(uid);
}

KCal::Journal::List ResourceScalix::rawJournals( JournalSortField sortField, SortDirection sortDirection )
{
  return mCalendar.rawJournals( sortField, sortDirection );
}

KCal::Journal::List ResourceScalix::rawJournalsForDate( const QDate &date )
{
  return mCalendar.rawJournalsForDate( date );
}

KCal::Alarm::List ResourceScalix::relevantAlarms( const KCal::Alarm::List &alarms )
{
  KCal::Alarm::List relevantAlarms;
  KCal::Alarm::List::ConstIterator it( alarms.constBegin() );
  while ( it != alarms.constEnd() ) {
    KCal::Alarm *a = (*it);
    ++it;
    const QString &uid = a->parent()->uid();
     if ( mUidMap.contains( uid ) ) {
       const QString &sr = mUidMap[ uid ].resource();
       Scalix::SubResource *subResource = 0;
       if ( mEventSubResources.contains( sr ) )
          subResource = &( mEventSubResources[ sr ] );
       else if ( mTodoSubResources.contains( sr ) )
          subResource = &( mTodoSubResources[ sr ] );
       assert( subResource );
       if ( subResource->alarmRelevant() )
           relevantAlarms.append ( a );
       else {
         kDebug(5650) <<"Alarm skipped, not relevant.";
       }
     }
  }
  return relevantAlarms;
}



KCal::Alarm::List ResourceScalix::alarms( const KDateTime& from,
                                         const KDateTime& to )
{
  return relevantAlarms( mCalendar.alarms( from, to ) );
}

KCal::Alarm::List ResourceScalix::alarmsTo( const KDateTime& to )
{
  return relevantAlarms( mCalendar.alarmsTo(to) );
}

void ResourceScalix::setTimeZoneId( const QString& tzid )
{
  //TODO port me kde4
#if 0
  mCalendar.setTimeZoneId( tzid );
  mFormat.setTimeZone( mCalendar.timeZoneId(), !mCalendar.isLocalTime() );
#endif
}

bool ResourceScalix::fromKMailAddIncidence( const QString& type,
                                           const QString& subResource,
                                           quint32 sernum,
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

    Incidence *inc = mFormat.fromString( data );
    if ( !inc )
      rc = false;
    else
      addIncidence( inc, subResource, sernum );

  return rc;
}

void ResourceScalix::fromKMailDelIncidence( const QString& type,
                                           const QString& subResource,
                                           const QString& uid )
{
  if ( type != kmailCalendarContentsType && type != kmailTodoContentsType
       && type != kmailJournalContentsType )
    // Not ours
    return;
  if ( !subresourceActive( subResource ) ) return;

  // Can't be in both, by contract
  if ( mUidsPendingDeletion.contains( uid ) ) {
    mUidsPendingDeletion.removeAll( uid );
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
    mResourceChangedTimer.start( 100 );
  }
}

void ResourceScalix::fromKMailRefresh( const QString& type,
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
    kWarning(5006) <<"KCal Kolab resource: fromKMailRefresh: unknown type" << type;
  mResourceChangedTimer.start( 100 );
}

void ResourceScalix::fromKMailAddSubresource( const QString& type,
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
  KConfigGroup group = config.group( subResource );

  bool active = group.readEntry( subResource, true );
  (*map)[ subResource ] = Scalix::SubResource( active, writable,
                                              alarmRelevant, label );
  loadSubResource( subResource, mimetype );
  emit signalSubresourceAdded( this, type, subResource, label );
}

void ResourceScalix::fromKMailDelSubresource( const QString& type,
                                             const QString& subResource )
{
  ResourceMap* map = subResourceMap( type );
  if ( !map ) // not ours
    return;
  if ( map->contains( subResource ) )
    map->remove( subResource );
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

QStringList ResourceScalix::subresources() const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceScalix*>( this )->doOpen();
  return ( mEventSubResources.keys()
         + mTodoSubResources.keys()
         + mJournalSubResources.keys() );
}

QString ResourceScalix::labelForSubresource( const QString& subresource ) const
{
  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].label();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].label();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].label();
  return subresource;
}

void ResourceScalix::fromKMailAsyncLoadResult( const QMap<quint32, QString>& map,
                                              const QString& type,
                                              const QString& folder )
{
  TemporarySilencer t( this );
  for( QMap<quint32, QString>::ConstIterator it = map.constBegin(); it != map.constEnd(); ++it )
    addIncidence( type.toLatin1(), it.value(), folder, it.key() );
}

bool ResourceScalix::subresourceActive( const QString& subresource ) const
{
  // Workaround: The ResourceView in KOrganizer wants to know this
  // before it opens the resource :-( Make sure we are open
  const_cast<ResourceScalix*>( this )->doOpen();

  if ( mEventSubResources.contains( subresource ) )
    return mEventSubResources[ subresource ].active();
  if ( mTodoSubResources.contains( subresource ) )
    return mTodoSubResources[ subresource ].active();
  if ( mJournalSubResources.contains( subresource ) )
    return mJournalSubResources[ subresource ].active();

  // Safe default bet:
  kDebug(5650) <<"subresourceActive(" << subresource <<" ): Safe bet";

  return true;
}

void ResourceScalix::setSubresourceActive( const QString &subresource, bool v )
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
    mResourceChangedTimer.start( 100 );
  }
}

void ResourceScalix::slotEmitResourceChanged()
{
   kDebug(5650) <<"KCal Kolab resource: emitting resource changed";
   mResourceChangedTimer.stop();
   emit resourceChanged( this );
}

KABC::Lock* ResourceScalix::lock()
{
  return new KABC::LockNull( true );
}


Scalix::ResourceMap* ResourceScalix::subResourceMap( const QString& contentsType )
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
bool ResourceScalix::addSubresource( const QString& resource, const QString& parent )
{
   kDebug(5650) <<"KCal Kolab resource - adding subresource:" << resource;
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
     const QString choice = KInputDialog::getItem( caption, QString(), contentTypeChoices );
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
bool ResourceScalix::removeSubresource( const QString& resource )
{
   kDebug(5650) <<"KCal Kolab resource - removing subresource:" << resource;
   return kmailRemoveSubresource( resource );
}

/*virtual*/
QString ResourceScalix::subresourceIdentifier( KCal::Incidence *incidence )
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


bool ResourceScalix::unloadSubResource( const QString& subResource )
{
    const bool silent = mSilent;
    mSilent = true;
    Scalix::UidMap::Iterator mapIt = mUidMap.begin();
    while ( mapIt != mUidMap.end() )
    {
        Scalix::UidMap::Iterator it = mapIt++;
        const StorageReference ref = it.value();
        if ( ref.resource() != subResource ) continue;
        // FIXME incidence() is expensive
        KCal::Incidence* incidence = mCalendar.incidence( it.key() );
        if( incidence ) {
            incidence->unRegisterObserver( this );
            mCalendar.deleteIncidence( incidence );
        }
        mUidMap.remove( it.key() );
    }
    mSilent = silent;
    return true;
}

#include "resourcescalix.moc"
