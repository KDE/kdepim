/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <typeinfo>
#include <stdlib.h>

#include <qdatetime.h>
#include <qstring.h>
#include <qptrlist.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "incidence.h"
#include "event.h"
#include "todo.h"
#include "journal.h"
#include "filestorage.h"

#include <kabc/locknull.h>

#include <kresources/configwidget.h>

#include "resourceremoteconfig.h"

#include "resourceremote.h"

using namespace KCal;

ResourceRemote::ResourceRemote( const KConfig* config )
  : ResourceCached( config ), mLock( 0 )
{
  if ( config ) {
    readConfig( config );
  }

  init();
}

ResourceRemote::ResourceRemote( const KURL &downloadUrl, const KURL &uploadUrl )
  : ResourceCached( 0 )
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

  setType( "remote" );

  mOpen = false;

  mLock = new KABC::LockNull( true );
}

void ResourceRemote::readConfig( const KConfig *config )
{
  QString url = config->readEntry( "DownloadUrl" );
  mDownloadUrl = KURL( url );

  url = config->readEntry( "UploadUrl" );
  mUploadUrl = KURL( url );

  mReloadPolicy = config->readNumEntry( "ReloadPolicy", ReloadNever );
}

void ResourceRemote::writeConfig( KConfig *config )
{
  kdDebug(5800) << "ResourceRemote::writeConfig()" << endl;

  ResourceCalendar::writeConfig( config );

  config->writeEntry( "DownloadUrl", mDownloadUrl.url() );
  config->writeEntry( "UploadUrl", mUploadUrl.url() );

  config->writeEntry( "ReloadPolicy", mReloadPolicy );
}

void ResourceRemote::setDownloadUrl( const KURL &url )
{
  mDownloadUrl = url;
}

KURL ResourceRemote::downloadUrl() const
{
  return mDownloadUrl;
}

void ResourceRemote::setUploadUrl( const KURL &url )
{
  mUploadUrl = url;
}

KURL ResourceRemote::uploadUrl() const
{
  return mUploadUrl;
}

void ResourceRemote::setReloadPolicy( int i )
{
  mReloadPolicy = i;
}

int ResourceRemote::reloadPolicy() const
{
  return mReloadPolicy;
}

QString ResourceRemote::cacheFile()
{
  QString file = locateLocal( "cache", "kcal/kresources/" + identifier() );
  kdDebug(5800) << "ResourceRemote::cacheFile(): " << file << endl;
  return file;
}

bool ResourceRemote::doOpen()
{
  kdDebug(5800) << "ResourceRemote::doOpen()" << endl;

  mOpen = true;

  return true;
}

bool ResourceRemote::load()
{
  kdDebug(5800) << "ResourceRemote::load()" << endl;

  if ( !mOpen ) return true;

  if ( mDownloadJob ) {
    kdWarning() << "ResourceRemote::load(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "ResourceRemote::load(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.close();

  mCalendar.load( cacheFile() );

  mDownloadJob = KIO::file_copy( mDownloadUrl, KURL( cacheFile() ), -1, true );
  connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
           SLOT( slotLoadJobResult( KIO::Job * ) ) );

  return true;
}

void ResourceRemote::slotLoadJobResult( KIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( 0 );
  } else {
    kdDebug(5800) << "ResourceRemote::slotLoadJobResult() success" << endl;

    mCalendar.close();
    mCalendar.load( cacheFile() );

    emit resourceChanged( this );
  }

  mDownloadJob = 0;

  emit resourceLoaded( this );
}

bool ResourceRemote::save()
{
  kdDebug(5800) << "ResourceRemote::save()" << endl;

  if ( !mOpen ) return true;

  if ( readOnly() ) {
    emit resourceSaved( this );
    return true;
  }

  if ( mDownloadJob ) {
    kdWarning() << "ResourceRemote::save(): download still in progress."
                << endl;
    return false;
  }
  if ( mUploadJob ) {
    kdWarning() << "ResourceRemote::save(): upload still in progress."
                << endl;
    return false;
  }

  mCalendar.save( cacheFile() );

  mUploadJob = KIO::file_copy( KURL( cacheFile() ), mUploadUrl, -1, true );
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
    kdDebug(5800) << "ResourceRemote::slotSaveJobResult() success" << endl;
  }
  
  mUploadJob = 0;

  emit resourceSaved( this );
}

void ResourceRemote::doClose()
{
  if ( !mOpen ) return;

  mCalendar.close();
  mOpen = false;
}

KABC::Lock *ResourceRemote::lock()
{
  return mLock;
}

void ResourceRemote::update(IncidenceBase *)
{
}

void ResourceRemote::dump() const
{
  ResourceCalendar::dump();
  kdDebug(5800) << "  DownloadUrl: " << mDownloadUrl.url() << endl;
  kdDebug(5800) << "  UploadUrl: " << mUploadUrl.url() << endl;
  kdDebug(5800) << "  ReloadPolicy: " << mReloadPolicy << endl;
}

#include "resourceremote.moc"
