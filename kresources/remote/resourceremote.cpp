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

#include <QtCore/QDateTime>
#include <QtCore/QString>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <kcal/icalformat.h>
#include <kcal/exceptions.h>
#include <kcal/incidence.h>
#include <kcal/event.h>
#include <kcal/todo.h>
#include <kcal/journal.h>
#include <kcal/filestorage.h>

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include <libkdepim/progressmanager.h>

#include "resourceremoteconfig.h"

#include "resourceremote.h"

using namespace KCal;

ResourceRemote::ResourceRemote()
  : ResourceCached(), mUseProgressManager( true ), mUseCacheFile( true )
{
  init();
}

ResourceRemote::ResourceRemote( const KConfigGroup &group )
  : ResourceCached( group ), mUseProgressManager( true ), mUseCacheFile( true )
{
  readConfig( group );

  init();
}

ResourceRemote::ResourceRemote( const KUrl &downloadUrl, const KUrl &uploadUrl )
  : ResourceCached(), mUseProgressManager( false ), mUseCacheFile( false )
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

  mLock = new KABC::Lock( cacheFile() );

  enableChangeNotification();
}

void ResourceRemote::readConfig( const KConfigGroup &group )
{
  QString url = group.readEntry( "DownloadUrl" );
  mDownloadUrl = KUrl( url );

  url = group.readEntry( "UploadUrl" );
  mUploadUrl = KUrl( url );

  ResourceCached::readConfig( group );
}

void ResourceRemote::writeConfig( KConfigGroup &group )
{
  kDebug(5800) << "ResourceRemote::writeConfig()" << endl;

  ResourceCalendar::writeConfig( group );

  group.writeEntry( "DownloadUrl", mDownloadUrl.url() );
  group.writeEntry( "UploadUrl", mUploadUrl.url() );

  ResourceCached::writeConfig( group );
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

bool ResourceRemote::doLoad( bool )
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

  calendar()->close();

  if ( mUseCacheFile ) {
    disableChangeNotification();
    loadFromCache();
    enableChangeNotification();
  }

  clearChanges();

  emit resourceChanged( this );

  if ( mLock->lock() ) {
    kDebug() << "Download from: " << mDownloadUrl << endl;

    mDownloadJob = KIO::file_copy( mDownloadUrl, KUrl( cacheFile() ), -1, true,
                                   false, !mUseProgressManager );
    connect( mDownloadJob, SIGNAL( result( KJob * ) ),
             SLOT( slotLoadJobResult( KJob * ) ) );
    if ( mUseProgressManager ) {
      connect( mDownloadJob, SIGNAL( percent( KJob *, unsigned long ) ),
               SLOT( slotPercent( KJob *, unsigned long ) ) );
      mProgress = KPIM::ProgressManager::createProgressItem(
        KPIM::ProgressManager::getUniqueID(), i18n("Downloading Calendar") );

      mProgress->setProgress( 0 );
    }
  } else {
    kDebug() << "ResourceRemote::load(): cache file is locked - something else must be loading the file" << endl;
  }
  return true;
}

void ResourceRemote::slotPercent( KJob *, unsigned long percent )
{
  kDebug() << "ResourceRemote::slotPercent(): " << percent << endl;

  mProgress->setProgress( percent );
}

void ResourceRemote::slotLoadJobResult( KJob *job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
  } else {
    kDebug(5800) << "ResourceRemote::slotLoadJobResult() success" << endl;

    calendar()->close();
    disableChangeNotification();
    loadFromCache();
    enableChangeNotification();

    emit resourceChanged( this );
  }

  mDownloadJob = 0;
  if ( mProgress ) {
    mProgress->setComplete();
    mProgress = 0;
  }

  mLock->unlock();
  emit resourceLoaded( this );
}

bool ResourceRemote::doSave( bool )
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

  saveToCache();

  mUploadJob = KIO::file_copy( KUrl( cacheFile() ), mUploadUrl, -1, true );
  connect( mUploadJob, SIGNAL( result( KJob * ) ),
           SLOT( slotSaveJobResult( KJob * ) ) );

  return true;
}

bool ResourceRemote::isSaving()
{
  return mUploadJob;
}

void ResourceRemote::slotSaveJobResult( KJob *job )
{
  if ( job->error() ) {
    static_cast<KIO::Job*>(job)->ui()->showErrorMessage();
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
  txt += i18n("URL: %1", mDownloadUrl.prettyUrl() );
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
