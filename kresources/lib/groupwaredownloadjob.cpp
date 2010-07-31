/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "groupwaredownloadjob.h"

#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <kdebug.h>
#include <klocale.h>
#include <kurl.h>
#include <kio/job.h>
#include <libemailfunctions/idmapper.h>
#include <libkdepim/progressmanager.h>

using namespace KPIM;

GroupwareDownloadJob::GroupwareDownloadJob( GroupwareDataAdaptor *adaptor )
  : GroupwareJob( adaptor ), mProgress(0),
    mDownloadJob(0), mListEventsJob(0)
{
}

void GroupwareDownloadJob::run()
{
  kdDebug(5800) << "GroupwareDownloadJob::run()" << endl;

  if ( !adaptor() ) {
    error( i18n("Unable to initialize the download job.") );
    return;
  }
  
  if ( adaptor()->folderLister() ){
    mFoldersForDownload = adaptor()->folderLister()->activeFolderIds();
  } else {
    // TODO: If we don't have a folder lister, use the base URL (e.g. if all
    // communication goes through one script on the server
  }

  mItemsForDownload.clear();
  mCurrentlyOnServer.clear();
  connect( adaptor(), TQT_SIGNAL( itemToDownload( const KURL &, KPIM::FolderLister::ContentType ) ),
           TQT_SLOT( slotItemToDownload( const KURL &, KPIM::FolderLister::ContentType ) ) );
  connect( adaptor(), TQT_SIGNAL( itemOnServer( const KURL & ) ),
           TQT_SLOT( slotItemOnServer( const KURL & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemDownloaded( const TQString &, const KURL &, const TQString & ) ),
           TQT_SLOT( slotItemDownloaded( const TQString &, const KURL &, const TQString & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemDownloadError( const KURL &, const TQString & ) ),
           TQT_SLOT( slotItemDownloadError( const KURL &, const TQString & ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(),
    adaptor()->downloadProgressMessage() );
  connect( mProgress,
           TQT_SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           TQT_SLOT( cancelLoad() ) );

  listItems();
}

void GroupwareDownloadJob::listItems()
{
  if ( mFoldersForDownload.isEmpty() ) {
    if ( mProgress ) {
      mProgress->setTotalItems( mItemsForDownload.count() + 1 );
      mProgress->setCompletedItems( 1 );
      mProgress->updateProgress();
    }

    /* Delete incidences no longer present on the server */
    deleteIncidencesGoneFromServer();
    downloadItem();
  } else {

    //kdDebug(7000) << "props: " << props.toString() << endl;
    KURL url = mFoldersForDownload.front();
    mFoldersForDownload.pop_front();
    kdDebug(5800) << "listItems: " << url.url() << endl;

    adaptor()->adaptDownloadUrl( url );
    kdDebug(5800) << "listItems, after adaptDownloadUrl: " << url.url() << endl;

    kdDebug(5800) << "OpenGroupware::listIncidences(): " << url << endl;

    mListItemsData = TQString::null;
    mListEventsJob = adaptor()->createListItemsJob( url );

    connect( mListEventsJob, TQT_SIGNAL( result( KIO::Job * ) ),
             TQT_SLOT( slotListItemsResult( KIO::Job * ) ) );
    connect( mListEventsJob, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
        TQT_SLOT( slotListItemsData( KIO::Job *, const TQByteArray & ) ) );
  }
}


void GroupwareDownloadJob::slotListItemsData( KIO::Job *, const TQByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotListItemsData()" << endl;

  mListItemsData.append( data.data() );
}


void GroupwareDownloadJob::slotListItemsResult( KIO::Job *job )
{
  kdDebug(5800) << "GroupwareDownloadJob::slotListItemsResult(): " << endl;

  if ( job->error() ) {
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
    error( job->errorString() );
  } else {
    adaptor()->interpretListItemsJob( job, mListItemsData );
  }

  mListItemsData = TQString::null;
  mListEventsJob = 0;

  listItems();
}

void GroupwareDownloadJob::deleteIncidencesGoneFromServer()
{
  TQMap<TQString, TQString> remoteIds( adaptor()->idMapper()->remoteIdMap() );
  KURL::List::ConstIterator it = mCurrentlyOnServer.begin();
  while ( it != mCurrentlyOnServer.end() ) {
    remoteIds.remove( (*it).path() );
    ++it;
  }
  TQMap<TQString, TQString>::ConstIterator it2;
  for (it2 = remoteIds.begin(); it2 != remoteIds.end(); ++it2) {
    adaptor()->deleteItem( remoteIds[ it2.key() ] );
  }
}

void GroupwareDownloadJob::downloadItem()
{
  kdDebug(7000) << " downloadItem " << endl;
  if ( mItemsForDownload.isEmpty() ) { 
    if ( mProgress ) mProgress->setComplete();
    
    mItemsForDownload.clear();
    mItemsDownloading.clear();
    mItemsDownloaded.clear();
    mItemsDownloadError.clear();
    
    mProgress = 0;
    success();
  } else {
    if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResBatchRequest ) {
      mDownloadItemsData = TQString::null;
      mDownloadJob = adaptor()->createDownloadJob( mItemsForDownload );
      mItemsDownloading = mItemsForDownload;
      mItemsForDownload.clear();
    } else {
      // Download the first item of the list
      TQMap<KURL,KPIM::FolderLister::ContentType>::Iterator it = mItemsForDownload.begin();
      KURL href( it.key() );
      KPIM::FolderLister::ContentType ctype = it.data();
      mItemsDownloading.insert( it.key(), it.data() );
      mItemsForDownload.remove( it.key() );
 
      adaptor()->adaptDownloadUrl( href );
      mDownloadItemsData = TQString::null;

      mDownloadJob = adaptor()->createDownloadJob( href, ctype );
    }
    connect( mDownloadJob, TQT_SIGNAL( result( KIO::Job * ) ),
        TQT_SLOT( slotDownloadItemResult( KIO::Job * ) ) );
    connect( mDownloadJob, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
        TQT_SLOT( slotDownloadItemData( KIO::Job *, const TQByteArray & ) ) );
  }
}

void GroupwareDownloadJob::slotDownloadItemResult( KIO::Job *job )
{
  kdDebug(5800) << "GroupwareDownloadJob::slotDownloadItemResult(): " << endl;

  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  if ( !trfjob ) return;

  if ( job->error() ) {
    error( job->errorString() );
  } else {
    adaptor()->interpretDownloadItemsJob( job, mDownloadItemsData );
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mDownloadItemsData = TQString::null;
  mDownloadJob = 0;

  downloadItem();
}

void GroupwareDownloadJob::slotDownloadItemData( KIO::Job *, const TQByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotDownloadItemData()" << endl;

  mDownloadItemsData.append( TQString::fromUtf8( data.data(), data.size() ) );
}

void GroupwareDownloadJob::slotItemToDownload( const KURL &remoteURL,
                         KPIM::FolderLister::ContentType type )
{
  KURL url( remoteURL );
  adaptor()->adaptDownloadUrl( url );
  if ( !mItemsForDownload.contains( url ) &&
       !mItemsDownloading.contains( url ) &&
       !mItemsDownloaded.contains( url ) ) {
    mItemsForDownload.insert( url, type );
  }
}


void GroupwareDownloadJob::slotItemOnServer( const KURL &remoteURL )
{
kdDebug()<<"GroupwareDownloadJob::slotItemOnServer( " << remoteURL.url() << ")" << endl;
  if ( !mCurrentlyOnServer.contains( remoteURL ) ) {
    mCurrentlyOnServer.append( remoteURL );
  }
}


void GroupwareDownloadJob::slotItemDownloadError( const KURL &remoteURL, const TQString &/*error*/ )
{
  // TODO: Error handling!
  if ( mItemsDownloading.contains( remoteURL ) ) {
    mItemsDownloadError[ remoteURL ] = mItemsDownloading[ remoteURL ];
  } else if ( mItemsForDownload.contains( remoteURL ) ) {
    mItemsDownloadError[ remoteURL ] = mItemsForDownload[ remoteURL ];
  }
}


void GroupwareDownloadJob::slotItemDownloaded( const TQString &localID,
         const KURL &remoteURL, const TQString &fingerprint )
{
kdDebug()<<"GroupwareDownloadJob::slotItemDownloaded( " << localID << ", " << remoteURL.url() << ", " << fingerprint << ")" << endl;
  if ( mItemsForDownload.contains( remoteURL ) ) {
    mItemsDownloaded[ remoteURL ] = mItemsForDownload[ remoteURL ];
    mItemsForDownload.remove( remoteURL );
  }
  if ( mItemsDownloading.contains( remoteURL ) ) {
    mItemsDownloaded[ remoteURL ] = mItemsDownloading[ remoteURL ];
    mItemsDownloading.remove( remoteURL );
  }
  if ( !mItemsDownloaded.contains( remoteURL ) ) {
    mItemsDownloaded[ remoteURL ] = KPIM::FolderLister::Unknown;
  }
  adaptor()->idMapper()->setRemoteId( localID, remoteURL.path() );
  adaptor()->idMapper()->setFingerprint( localID, fingerprint );
}



void GroupwareDownloadJob::kill()
{
  cancelLoad();
}



void GroupwareDownloadJob::cancelLoad()
{
  if ( mDownloadJob ) mDownloadJob->kill();
  mDownloadJob = 0;
  if ( mListEventsJob ) mListEventsJob->kill();
  mListEventsJob = 0;
  if ( mProgress ) mProgress->setComplete();
  

  mProgress = 0;
}

#include "groupwaredownloadjob.moc"
