/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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
#include "calendaradaptor.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"

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
  : ResourceCached( 0 ), mLock( true )
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
  
  mIsShowingError = false;

  mPrefs = new OpenGroupwarePrefsBase();
  mFolderLister = new FolderLister( FolderLister::Calendar );
  
  setType( "opengroupware" );

  enableChangeNotification();

  mAdaptor = new KPIM::CalendarAdaptor();
  mAdaptor->setFolderLister( mFolderLister );
  mAdaptor->setDownloadProgressMessage( i18n("Downloading calendar") );
  mAdaptor->setUploadProgressMessage( i18n("Uploading calendar") );
  mAdaptor->setUser( prefs()->user() );
  mAdaptor->setPassword( prefs()->password() );
  mAdaptor->setIdMapper( &idMapper() );
  mAdaptor->setResource( this );
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

  mDownloadJob = new KPIM::GroupwareDownloadJob( mAdaptor );
  connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );

  return true;
}

void OpenGroupware::slotDownloadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug() << "OpenGroupware::slotJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kdDebug() << "Successfully downloaded data" << endl;
  
    clearChanges();
    saveCache();
    enableChangeNotification();

    emit resourceChanged( this );
    emit resourceLoaded( this );
  }

  mDownloadJob = 0;
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
  
  QStringList incidencesForUpload;
  QStringList incidencesForDeletion;

  ICalFormat format;

  Incidence::List deleted = deletedIncidences();
  for( it = deleted.begin(); it != deleted.end(); ++it ) {
    //kdDebug(7000) << "Delete: " << endl << format.toICalString(*it) << endl;
    KURL url( mBaseUrl );
    url.setPath( (*it)->customProperty( "KCalResourceOpengroupware", "storagelocation" ) );
    incidencesForDeletion << url.url();
  }
  
  Incidence::List added = addedIncidences();
  for( it = added.begin(); it != added.end(); ++it ) {
    incidencesForUpload << format.toICalString(*it);
  }
  // add the changed incidences as well
  // TODO check if the version we based our change on is still current 
  // on the server
  Incidence::List changed = changedIncidences();
  for( it = changed.begin(); it != changed.end(); ++it ) {
    incidencesForUpload << format.toICalString(*it);
  }

  mUploadJob = new KPIM::GroupwareUploadJob( mAdaptor );
  connect( mUploadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotUploadJobResult( KPIM::GroupwareJob * ) ) );

  mUploadJob->setUrlsForDeletion( incidencesForDeletion );
  mUploadJob->setDataForUpload( incidencesForUpload );
  mUploadJob->setBaseUrl( mBaseUrl );

  return true;
}

void OpenGroupware::slotUploadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug() << "OpenGroupware::slotUploadJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kdDebug() << "Successfully uploaded data" << endl;
    /* 
     * After the put the server might have expanded recurring events and will
     * also change the uids of the uploaded events. Remove them from the cache
     * and get the fresh delta and download. 
     */

    if ( !mDownloadJob ) {
      mDownloadJob = new KPIM::GroupwareDownloadJob( mAdaptor );
      connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
          SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );
    } else {
      kdWarning() << k_funcinfo << "Download still in progress. Can't happen. (TM)" << endl;
    }
  }

  mDownloadJob = 0;
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

#include "kcal_resourceopengroupware.moc"
