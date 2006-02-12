/*
    This file is part of libkcal.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

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
*/

#include <typeinfo>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <q3ptrlist.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <libkcal/icalformat.h>
#include <libkcal/exceptions.h>
#include <libkcal/incidence.h>
#include <libkcal/event.h>
#include <libkcal/todo.h>
#include <libkcal/journal.h>
#include <libkcal/filestorage.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include "resourceremoteconfig.h"

#include "resourceremote.h"

using namespace KCal;

ResourceRemote::ResourceRemote( const KConfig *config )
  : ResourceCached( config ), mUseProgressManager( true ), mUseCacheFile( true )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceRemote::ResourceRemote( const KUrl &downloadUrl, const KUrl &uploadUrl )
  : ResourceCached( 0 ), mUseProgressManager( false ), mUseCacheFile( false )
{
  mDownloadUrl = downloadUrl;

  if ( uploadUrl.isEmpty() ) {
    mUploadUrl = mDownloadUrl;
  } else {
    mUploadUrl = uploadUrl;
  }

  init();
}

ResourceRemote::~ResourceRemote()
{
  close();

  if ( mDownloadJob ) mDownloadJob->kill();
  if ( mUploadJob ) mUploadJob->kill();

  delete mLock;
}

void ResourceRemote::init()
{
  mDownloadJob = 0;
  mUploadJob = 0;

  mProgress = 0;

  setType( "remote" );

  mLock = new KABC::LockNull( true );

  enableChangeNotification();
}

void ResourceRemote::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "DownloadUrl" );
  mDownloadUrl = KUrl( url );

  url = config->readEntry( "UploadUrl" );
  mUploadUrl = KUrl( url );

  ResourceCached::readConfig( config );
}

void ResourceRemote::writeConfig( KConfig *config )
{
  kDebug(5800) << "ResourceRemote::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "DownloadUrl", mDownloadUrl.url() );
  config->writeEntry( "UploadUrl", mUploadUrl.url() );

  ResourceCached::writeConfig( config );
}

void ResourceRemote::setDownloadUrl( const KUrl &url )
{
  mDownloadUrl = url;
}

KUrl ResourceRemote::downloadUrl() const
{
  return mDownloadUrl;
}

void ResourceRemote::setUploadUrl( const KUrl &url )
{
  mUploadUrl = url;
}

KUrl ResourceRemote::uploadUrl() const
{
  return mUploadUrl;
}

void ResourceRemote::setUseProgressManager( bool useProgressManager )
{
  mUseProgressManager = useProgressManager;
}

bool ResourceRemote::useProgressManager() const
{
  return mUseProgressManager;
}

void ResourceRemote::setUseCacheFile( bool useCacheFile )
{
  mUseCacheFile = useCacheFile;
}

bool ResourceRemote::useCacheFile() const
{
  return mUseCacheFile;
}

bool ResourceRemote::doLoad()
{
  kDebug(5800) << "ResourceRemote::load()" << endl;

  if ( mDownloadJob ) {
    kWarning() << "ResourceRemote::load(): download still in progress."
                << endl;
    return true;
  }
  if ( mUploadJob ) {
    kWarning() << "ResourceRemote::load(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.close();

  if ( mUseCacheFile ) {
    disableChangeNotification();
    loadCache();
    enableChangeNotification();
  }

  clearChanges();

  emit resourceChanged( this );

  kDebug() << "Download from: " << mDownloadUrl << endl;

  mDownloadJob = KIO::file_copy( mDownloadUrl, KUrl( cacheFile() ), -1, true,
                                 false, !mUseProgressManager );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadJobResult( KIO::Job * ) ) );
  if ( mUseProgressManager ) {
    connect( mDownloadJob, SIGNAL( percent( KIO::Job *, unsigned long ) ),
             SLOT( slotPercent( KIO::Job *, unsigned long ) ) );
    mProgress = KPIM::ProgressManager::createProgressItem(
        KPIM::ProgressManager::getUniqueID(), i18n("Downloading Calendar") );

    mProgress->setProgress( 0 );
  }

  return true;
}

void ResourceRemote::slotPercent( KIO::Job *, unsigned long percent )
{
  kDebug() << "ResourceRemote::slotPercent(): " << percent << endl;

  mProgress->setProgress( percent );
}

void ResourceRemote::slotLoadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kDebug(5800) << "ResourceRemote::slotLoadJobResult() success" << endl;

    mCalendar.close();
    disableChangeNotification();
    loadCache();
    enableChangeNotification();

    emit resourceChanged( this );
  }

  mDownloadJob = 0;
  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
  }

  emit resourceLoaded( this );
}

bool ResourceRemote::doSave()
{
  kDebug(5800) << "ResourceRemote::save()" << endl;

  if ( readOnly() || !hasChanges() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mDownloadJob ) {
    kWarning() << "ResourceRemote::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kWarning() << "ResourceRemote::save(): upload still in progress."
                << endl;
    return false;
  }

  mChangedIncidences = allChanges();

  saveCache();

  mUploadJob = KIO::file_copy( KUrl( cacheFile() ), mUploadUrl, -1, true );
  connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotSaveJobResult( KIO::Job * ) ) );

  return true;
}

bool ResourceRemote::isSaving()
{
  return mUploadJob;
}

void ResourceRemote::slotSaveJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kDebug(5800) << "ResourceRemote::slotSaveJobResult() success" << endl;
  
    Incidence::List::ConstIterator it;
    for( it = mChangedIncidences.begin(); it != mChangedIncidences.end();
         ++it ) {
      clearChange( *it );
    }
    mChangedIncidences.clear();
  }
  
  mUploadJob = 0;

  emit resourceSaved( this );
}

KABC::Lock *ResourceRemote::lock()
{
  return mLock;
}

void ResourceRemote::dump() const
{
  ResourceCalendar::dump();
  kDebug(5800) << "  DownloadUrl: " << mDownloadUrl.url() << endl;
  kDebug(5800) << "  UploadUrl: " << mUploadUrl.url() << endl;
  kDebug(5800) << "  ReloadPolicy: " << reloadPolicy() << endl;
}

void ResourceRemote::addInfoText( QString &txt ) const
{
  txt += "<br>";
  txt += i18n("URL: %1").arg( mDownloadUrl.prettyURL() );
}

bool ResourceRemote::setValue( const QString &key, const QString &value )
{
  if ( key == "URL" ) {
    setUploadUrl( KUrl( value ) );
    setDownloadUrl( KUrl( value ) );
    return true;
  } else  if ( key == "DownloadURL" ) {
    setDownloadUrl( KUrl( value ) );
    return true;
  } else if ( key == "UploadURL" ) {
    setUploadUrl( KUrl( value ) );
    return true;
  } else
    return ResourceCached::setValue( key, value );
}

#include "resourceremote.moc"
