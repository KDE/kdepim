/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include "kcal_resourceopengroupware.h"

#include "kcal_opengroupwareprefsbase.h"
#include "kcal_resourceopengroupwareconfig.h"

#include "libkcal/confirmsavedialog.h"
#include "webdavhandler.h"
#include "folderlister.h"

#include <libkcal/icalformat.h>
#include <libkcal/calendarlocal.h>

#include <assert.h>

#include <qapplication.h>
#include <qdatetime.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qdom.h>

#include <kabc/locknull.h>
#include <kdebug.h>
#include <klocale.h>
#include <kresources/configwidget.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <kurl.h>
#include <libkdepim/kpimprefs.h>
#include <libkdepim/groupwarejob.h>
#include <kio/job.h>
#include <kio/davjob.h>

using namespace KCal;

OpenGroupware::OpenGroupware()
  : ResourceCached( 0 ), mLock( true ),
    mProgress( 0 )
{
  init();

  mPrefs->addGroupPrefix( identifier() );
}

OpenGroupware::OpenGroupware( const KConfig *config )
  : ResourceCached( config ), mLock( true )
{
  init();

  mPrefs->addGroupPrefix( identifier() );

  if ( config ) readConfig( config );
}

OpenGroupware::~OpenGroupware()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

void OpenGroupware::init()
{
  mDownloadJob = 0;
  mProgress = 0;
  
  mIsShowingError = false;

  mPrefs = new OpenGroupwarePrefsBase();
  mFolderLister = new FolderLister( FolderLister::Calendar );
  
  setType( "opengroupware" );

  enableChangeNotification();
}

OpenGroupwarePrefsBase *OpenGroupware::prefs()
{
  return mPrefs;
}

void OpenGroupware::readConfig( const KConfig *config )
{
  kdDebug() << "KCal::OpenGroupware::readConfig()" << endl;

  mPrefs->readConfig();

  ResourceCached::readConfig( config );

  mFolderLister->readConfig( config );

  mBaseUrl = KURL( prefs()->url() );
  mBaseUrl.setUser( prefs()->user() );
  mBaseUrl.setPass( prefs()->password() );
}

void OpenGroupware::writeConfig( KConfig *config )
{
  kdDebug() << "KCal::OpenGroupware::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );

  mFolderLister->writeConfig( config );
}

bool OpenGroupware::doOpen()
{
  return true;
}

void OpenGroupware::doClose()
{
  ResourceCached::doClose();
}

bool OpenGroupware::doLoad()
{
  kdDebug() << "OpenGroupware::load()" << endl;

  if ( mIsShowingError ) {
    kdDebug() << "Still showing error" << endl;
    return true;
  }

  if ( mDownloadJob ) {
    kdWarning() << "Download still in progress" << endl;
    return false;
  }
  
  mCalendar.close();
  clearChanges();
  disableChangeNotification();
  loadCache();
  enableChangeNotification();
  emit resourceChanged( this );
  
  mFoldersForDownload = mFolderLister->activeFolderIds();

  mIncidencesForDownload.clear();
  mCurrentlyOnServer.clear();

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(), i18n("Downloading calendar") );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

  listIncidences();

  return true;
}

void OpenGroupware::listIncidences()
{
  if ( mFoldersForDownload.isEmpty() ) {
    if ( mProgress ) {
      mProgress->setTotalItems( mIncidencesForDownload.count() );
      mProgress->setCompletedItems(1);
      mProgress->updateProgress();
    }

    /* Delete incidences no longer present on the server */
    deleteIncidencesGoneFromServer();
    downloadNextIncidence();
  } else {
    QDomDocument props = WebdavHandler::createItemsAndVersionsPropsRequest();

    //kdDebug(7000) << "getCalendar: " << url.prettyURL() << endl;
    //kdDebug(7000) << "props: " << props.toString() << endl;

    KURL url = mFoldersForDownload.front();
    mFoldersForDownload.pop_front();

    url.setUser( prefs()->user() );
    url.setPass( prefs()->password() );
    url.addPath( "/Calendar" );

    kdDebug() << "OpenGroupware::listIncidences(): " << url << endl;

    mListEventsJob = KIO::davPropFind( url, props, "1", false );

    connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotListJobResult( KIO::Job * ) ) );
  }
}

void OpenGroupware::slotListJobResult( KIO::Job *job )
{
  kdDebug() << "OpenGroupware::slotListJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
  } else {
    QDomDocument doc = mListEventsJob->response();

    //kdDebug(7000) << " Doc: " << doc.toString() << endl;

    //kdDebug(7000) << idMapper().asString() << endl;
    QDomNodeList entries = doc.elementsByTagNameNS( "DAV:", "href" );
    QDomNodeList fingerprints = doc.elementsByTagNameNS( "DAV:", "getetag" );
    int c = entries.count();
    int i = 0;
    while ( i < c ) {
      QDomNode node = entries.item( i );
      QDomElement e = node.toElement();
      const QString &entry = e.text();
      node = fingerprints.item( i );
      e = node.toElement();
      i++;

      bool download = false;
      KURL url ( entry );
      const QString &location = url.path();
      const QString &newFingerprint = e.text();

      mCurrentlyOnServer << location;
      /* if not locally present, download */
      const QString &localId = idMapper().localId( location );
      Incidence *i = 0;
      if ( !localId.isNull() )
        i = mCalendar.incidence( localId );
      if ( !i ) {
         //kdDebug(7000) << "Not locally present, download: " << location << endl;
        download = true;
      } else {
         //kdDebug(7000) << "Locally present " << endl;
        /* locally present, let's check if it's newer than what we have */
        const QString &oldFingerprint = idMapper().fingerprint( i->uid() );
        if ( oldFingerprint != newFingerprint ) {
          kdDebug(7000) << "Fingerprint changed old: " << oldFingerprint << " new: " << newFingerprint << endl;
          // something changed on the server, let's see if we also changed it locally
          if ( deletedIncidences().find( i ) != deletedIncidences().end()
            || changedIncidences().find( i ) != changedIncidences().end() ) {
            // TODO conflict resolution
             kdDebug(7000) << "TODO conflict resolution" << endl;
            download = true;
          } else {
            download = true;
          }
        } else {
          //kdDebug(7000) << "Fingerprint did not change, don't download this one " << endl;
        }
      }
      if ( download ) {
        mIncidencesForDownload << entry;
      }
    }
      }
  mListEventsJob = 0;

  listIncidences();
}

void OpenGroupware::deleteIncidencesGoneFromServer()
{
  QMap<QString, QString> remoteIds( idMapper().remoteIdMap() );
  QStringList::ConstIterator it = mCurrentlyOnServer.begin();
  while ( it != mCurrentlyOnServer.end() ) {
    remoteIds.remove( (*it) );
    ++it;
  }
  disableChangeNotification();
  QMap<QString, QString>::ConstIterator it2;
  for (it2 = remoteIds.begin(); it2 != remoteIds.end(); ++it2) {
    Incidence *i = mCalendar.incidence( remoteIds[ it2.key() ] );
    if ( i ) mCalendar.deleteIncidence( i );
  }
  enableChangeNotification();
}

void OpenGroupware::downloadNextIncidence()
{
  kdDebug(7000) << " downloadNextIncidence " << endl;
  if ( !mIncidencesForDownload.isEmpty() ) {
    const QString entry = mIncidencesForDownload.front();
    mIncidencesForDownload.pop_front();

    KURL url( entry );
    url.setProtocol( "webdav" );
    mCurrentGetUrl = url.url(); // why can't I ask the job?
    url.setUser( mPrefs->user() );
    url.setPass( mPrefs->password() );

    mJobData = QString::null;

    mDownloadJob = KIO::get( url, false, false );
    mDownloadJob->addMetaData( "accept", "text/calendar" );
    mDownloadJob->addMetaData( "PropagateHttpHeader", "true" );
    connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
        SLOT( slotJobResult( KIO::Job * ) ) );
    connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  } else {
    if ( mProgress ) mProgress->setComplete();
    mProgress = 0;
    loadFinished();
  }
}

void OpenGroupware::slotUploadJobResult( KIO::Job *job )
{
  kdDebug(7000) << " slotUploadJobResult " << endl;
  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    ICalFormat ical;
    Incidence *i = ical.fromString( mIncidencesForUpload.front() );
    clearChange( i );
    mCalendar.deleteIncidence( i );
    saveCache();
    mIncidencesForUpload.pop_front();
  }

  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
  mUploadJob = 0;
  uploadNextIncidence();
}


static const QString getEtagFromHeaders( const QString& headers )
{
  int start = headers.find( "etag:" );
  start += 6;
  return headers.mid( start, headers.find( "\n", start ) - start );
}

void OpenGroupware::slotJobResult( KIO::Job *job )
{
  kdDebug() << "OpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    const QString& headers = job->queryMetaData( "HTTP-Headers" );
    const QString& etag = getEtagFromHeaders( headers );
    CalendarLocal calendar;
    ICalFormat ical;
    if ( !ical.fromString( &calendar, mJobData ) ) {
      loadError( i18n("Error parsing calendar data.") );
    } else {
      Incidence::List incidences = calendar.incidences();
      Incidence::List::ConstIterator it;
      for( it = incidences.begin(); it != incidences.end(); ++it ) {
//        kdDebug() << "INCIDENCE: " << (*it)->summary() << endl;
        Incidence *i = (*it)->clone();
        const QString &remote = KURL( mCurrentGetUrl ).path();
        const QString &local = idMapper().localId( remote );
        if ( local.isEmpty() ) {
          idMapper().setRemoteId( i->uid(), remote );
        } else {
          i->setUid( local );
        }
        idMapper().setFingerprint( i->uid(), etag );
        i->setCustomProperty( "KCalResourceOpengroupware", "storagelocation" , remote );
        // if it's already there, we uploaded or changed it
        if ( !mCalendar.incidence( i->uid() ) ) {
          disableChangeNotification();
          addIncidence( i );
          enableChangeNotification();
        }
        else
          kdDebug(7000) << "Incidence already present locally" << endl;
      }
    }
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mJobData = QString::null;
  mDownloadJob = 0;
  mCurrentGetUrl = QString::null;
  downloadNextIncidence();
}

void OpenGroupware::slotJobData( KIO::Job *, const QByteArray &data )
{
//  kdDebug() << "OpenGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
}

void OpenGroupware::loadFinished()
{
  clearChanges();
  saveCache();
  enableChangeNotification();

  emit resourceChanged( this );
  emit resourceLoaded( this );
}

bool OpenGroupware::doSave()
{
  kdDebug() << "KCal::OpenGroupware::doSave()" << endl;

  saveCache();

  if ( !hasChanges() ) {
    kdDebug() << "No changes" << endl;
    return true;
  }
  if ( !confirmSave() ) return false;

  Incidence::List::ConstIterator it;
  mIncidencesForUpload.clear();
  ICalFormat format;

  Incidence::List deleted = deletedIncidences();
  for( it = deleted.begin(); it != deleted.end(); ++it ) {
    kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    KURL url( mBaseUrl );
    url.setPath( (*it)->customProperty( "KCalResourceOpengroupware", "storagelocation" ) );
    mIncidencesForDeletion << url.url();
  }
  
  Incidence::List added = addedIncidences();
  for( it = added.begin(); it != added.end(); ++it ) {
    mIncidencesForUpload << format.toICalString(*it);
  }
  // add the changed incidences as well
  // TODO check if the version we based our change on is still current 
  // on the server
  Incidence::List changed = changedIncidences();
  for( it = changed.begin(); it != changed.end(); ++it ) {
    mIncidencesForUpload << format.toICalString(*it);
  }

  // the delete will trigger the uploads, once it is finished
  if ( !mIncidencesForDeletion.isEmpty() )
    doDeletions();
  else
    uploadNextIncidence();

  return true;
}

void OpenGroupware::uploadNextIncidence()
{
  if ( !mIncidencesForUpload.isEmpty() ) {
    KURL url( mBaseUrl );
    url.setPath( url.path() + "/Calendar/new.ics" );
    const QString inc = mIncidencesForUpload.front();
    kdDebug(7000) << "Uploading to: " << inc << endl;
    kdDebug(7000) << "Uploading: " << url.prettyURL() << endl;
    mUploadJob = KIO::storedPut( inc.utf8(), url, -1, true, false, false );
    mUploadJob->addMetaData( "content-type", "text/calendar" );
    connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
        this, SLOT( slotUploadJobResult( KIO::Job * ) ) );

    mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
        KPIM::ProgressManager::getUniqueID(), i18n("Downloading calendar") );
    connect( mUploadProgress,
        SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
        SLOT( cancelSave() ) );
  } else {
    if ( mUploadProgress ) {
      mUploadProgress->setComplete();
      mUploadProgress = 0;
    }
    /* 
     * After the put the server might have expanded recurring events and will
     * also change the uids of the uploaded events. Remove them from the cache
     * and get the fresh delta and download. 
     */
    doLoad();
  }
}

// FIXME: Put this into ResourceCached
bool OpenGroupware::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *OpenGroupware::lock()
{
  return &mLock;
}

void OpenGroupware::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mListEventsJob ) mListEventsJob->kill();
  mListEventsJob = 0;
  if ( mProgress ) mProgress->setComplete();
  mProgress = 0;
}

void OpenGroupware::cancelSave()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
  mUploadProgress = 0;
}

void OpenGroupware::doDeletions()
{
  kdDebug(7000) << " URLs for deletion: " << mIncidencesForDeletion << endl;
  mDeletionJob = KIO::del( mIncidencesForDeletion, false, false );
  connect( mDeletionJob, SIGNAL( result( KIO::Job* ) ),
           this, SLOT( slotDeletionResult( KIO::Job* ) ) );
}

void OpenGroupware::slotDeletionResult( KIO::Job *job )
{
  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
    kdDebug(5006) << "slotDeletionResult failed " << endl;
  } else {
    kdDebug(5006) << "slotDeletionResult successfull " << endl;
    
    QStringList::Iterator it = mIncidencesForDeletion.begin();
    for ( ; it != mIncidencesForDeletion.end(); ++it ) {
      KURL url( *it );
      const QString &remote = url.path();
      const QString &local = idMapper().localId( remote );
      if ( !local.isEmpty() ) {
        Incidence *i = mCalendar.incidence( local  );
        clearChange( i );
      }
    }
    mIncidencesForDeletion.clear();
    uploadNextIncidence();
  }
}

#include "kcal_resourceopengroupware.moc"
