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

#include "kcal_resourcegroupwarebase.h"

#include "kcal_groupwareprefs.h"
#include "libkcal/confirmsavedialog.h"
#include "folderlister.h"
#include "calendaradaptor.h"
#include "groupwaredownloadjob.h"
#include "groupwareuploadjob.h"

#include <libkcal/icalformat.h>
#include <klocale.h>

using namespace KCal;

ResourceGroupwareBase::ResourceGroupwareBase()
  : ResourceCached( 0 ), mPrefs(0), mFolderLister(0), 
    mLock( true ), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
}

ResourceGroupwareBase::ResourceGroupwareBase( const KConfig *config )
  : ResourceCached( config ), mPrefs(0), mFolderLister(0), 
    mLock( true ), mAdaptor(0), mDownloadJob(0), mUploadJob(0)
{
  readConfig( config );
}

ResourceGroupwareBase::~ResourceGroupwareBase()
{
  disableChangeNotification();

  delete mPrefs;
  mPrefs = 0;
}

GroupwarePrefsBase *ResourceGroupwareBase::createPrefs()
{
  return new GroupwarePrefsBase();
}



KPIM::GroupwareDownloadJob *ResourceGroupwareBase::createDownloadJob( CalendarAdaptor *adaptor )
{
  return new KPIM::GroupwareDownloadJob( adaptor );
}

KPIM::GroupwareUploadJob *ResourceGroupwareBase::createUploadJob( CalendarAdaptor *adaptor )
{
  return new KPIM::GroupwareUploadJob( adaptor );
}

void ResourceGroupwareBase::setPrefs( GroupwarePrefsBase *newprefs ) 
{
  if ( !newprefs ) return;
  if ( mPrefs ) delete mPrefs;
  mPrefs = newprefs;
  mPrefs->addGroupPrefix( identifier() );
  
  mPrefs->readConfig();
  mBaseUrl = KURL( prefs()->url() );
  mBaseUrl.setUser( prefs()->user() );
  mBaseUrl.setPass( prefs()->password() );
}

void ResourceGroupwareBase::setFolderLister( KPIM::FolderLister *folderLister )
{
  if ( !folderLister ) return;
  if ( mFolderLister ) delete mFolderLister;
  mFolderLister = folderLister;
  if ( adaptor() ) adaptor()->setFolderLister( mFolderLister );
}

void ResourceGroupwareBase::setAdaptor( CalendarAdaptor *adaptor )
{
  if ( !adaptor ) return;
  if ( mAdaptor ) delete mAdaptor;
  mAdaptor = adaptor;
  mAdaptor->setFolderLister( mFolderLister );
  mAdaptor->setDownloadProgressMessage( i18n("Downloading calendar") );
  mAdaptor->setUploadProgressMessage( i18n("Uploading calendar") );
  if ( prefs() ) {
    mAdaptor->setUser( prefs()->user() );
    mAdaptor->setPassword( prefs()->password() );
  }
  mAdaptor->setIdMapper( &idMapper() );
  mAdaptor->setResource( this );
}

void ResourceGroupwareBase::init()
{
  mDownloadJob = 0;
  mIsShowingError = false;

  enableChangeNotification();
}

GroupwarePrefsBase *ResourceGroupwareBase::prefs()
{
  return mPrefs;
}

void ResourceGroupwareBase::readConfig( const KConfig *config )
{
  kdDebug(5800) << "KCal::ResourceGroupwareBase::readConfig()" << endl;
  ResourceCached::readConfig( config );
  if ( mFolderLister ) {
    mFolderLister->readConfig( config );
  }
}

void ResourceGroupwareBase::writeConfig( KConfig *config )
{
  kdDebug(5800) << "KCal::ResourceGroupwareBase::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  mPrefs->writeConfig();

  ResourceCached::writeConfig( config );

  mFolderLister->writeConfig( config );
}

bool ResourceGroupwareBase::doOpen()
{
  return true;
}

void ResourceGroupwareBase::doClose()
{
  ResourceCached::doClose();
	if ( mDownloadJob ) mDownloadJob->kill();
}

bool ResourceGroupwareBase::doLoad()
{
  kdDebug(5800) << "ResourceGroupwareBase::load()" << endl;

  if ( mIsShowingError ) {
    kdDebug(5800) << "Still showing error" << endl;
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

  mDownloadJob = createDownloadJob( adaptor() );
  connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );

  return true;
}

void ResourceGroupwareBase::slotDownloadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug(5800) << "ResourceGroupwareBase::slotJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kdDebug(5800) << "Successfully downloaded data" << endl;
  
    clearChanges();
    saveCache();
    enableChangeNotification();

    emit resourceChanged( this );
    emit resourceLoaded( this );
  }

  mDownloadJob = 0;
}

bool ResourceGroupwareBase::doSave()
{
  kdDebug(5800) << "KCal::ResourceGroupwareBase::doSave()" << endl;

  saveCache();

  if ( !hasChanges() ) {
    kdDebug(5800) << "No changes" << endl;
    return true;
  }
  if ( !confirmSave() ) return false;
  
  mUploadJob = createUploadJob( adaptor() );
  connect( mUploadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
    SLOT( slotUploadJobResult( KPIM::GroupwareJob * ) ) );
  mUploadJob->setBaseUrl( mBaseUrl );
    
  Incidence::List inc;
  Incidence::List::Iterator it;
  KPIM::GroupwareUploadItem::List addedItems, changedItems, deletedItems;

  inc = addedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    addedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Added ) );
  }
  // TODO: Check if the item has changed on the server...
  // In particular, check if the version we based our change on is still current 
  // on the server
  inc = changedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    changedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Changed ) );
  }
  inc = deletedIncidences();
  for( it = inc.begin(); it != inc.end(); ++it ) {
    deletedItems.append( adaptor()->newUploadItem( *it, KPIM::GroupwareUploadItem::Deleted ) );
  }

  mUploadJob->setAddedItems( addedItems );
  mUploadJob->setChangedItems( changedItems );
  mUploadJob->setDeletedItems( deletedItems );

  return true;
}

void ResourceGroupwareBase::slotUploadJobResult( KPIM::GroupwareJob *job )
{
  kdDebug(5800) << "ResourceGroupwareBase::slotUploadJobResult(): " << endl;

  if ( job->error() ) {
    mIsShowingError = true;
    loadError( job->errorString() );
    mIsShowingError = false;
  } else {
    kdDebug(5800) << "Successfully uploaded data" << endl;
    /* 
     * After the put the server might have expanded recurring events and will
     * also change the uids of the uploaded events. Remove them from the cache
     * and get the fresh delta and download. 
     */

    if ( !mDownloadJob ) {
      mDownloadJob = createDownloadJob( adaptor() );
      connect( mDownloadJob, SIGNAL( result( KPIM::GroupwareJob * ) ),
          SLOT( slotDownloadJobResult( KPIM::GroupwareJob * ) ) );
    } else {
      kdWarning() << k_funcinfo << "Download still in progress. Can't happen. (TM)" << endl;
    }
  }

  mDownloadJob = 0;
}

// FIXME: Put this into ResourceCached
bool ResourceGroupwareBase::confirmSave()
{
  if ( !hasChanges() ) return true;

  ConfirmSaveDialog dlg( resourceName(), 0 );

  dlg.addIncidences( addedIncidences(), i18n("Added") );
  dlg.addIncidences( changedIncidences(), i18n("Changed") );
  dlg.addIncidences( deletedIncidences(), i18n("Deleted") );

  int result = dlg.exec();
  return result == QDialog::Accepted;
}

KABC::Lock *ResourceGroupwareBase::lock()
{
  return &mLock;
}

#include "kcal_resourcegroupwarebase.moc"
