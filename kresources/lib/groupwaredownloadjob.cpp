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
#include <kresources/idmapper.h>
#include <libkdepim/progressmanager.h>

using namespace KPIM;

GroupwareDownloadJob::GroupwareDownloadJob( GroupwareDataAdaptor *adaptor )
  : GroupwareJob( adaptor ), mProgress(0),
    mDownloadJob(0), mListEventsJob(0)
{
}

void GroupwareDownloadJob::run()
{
  kDebug(5800) <<"GroupwareDownloadJob::run()";

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
  connect( adaptor(), SIGNAL( itemToDownload( const KUrl &, KPIM::FolderLister::ContentType ) ),
           SLOT( slotItemToDownload( const KUrl &, KPIM::FolderLister::ContentType ) ) );
  connect( adaptor(), SIGNAL( itemOnServer( const KUrl & ) ),
           SLOT( slotItemOnServer( const KUrl & ) ) );
  connect( adaptor(), SIGNAL( itemDownloaded( const QString &, const KUrl &, const QString & ) ),
           SLOT( slotItemDownloaded( const QString &, const KUrl &, const QString & ) ) );
  connect( adaptor(), SIGNAL( itemDownloadError( const KUrl &, const QString & ) ),
           SLOT( slotItemDownloadError( const KUrl &, const QString & ) ) );

  mProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(),
    adaptor()->downloadProgressMessage() );
  connect( mProgress,
           SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
           SLOT( cancelLoad() ) );

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

    //kDebug(7000) <<"props:" << props.toString();
    KUrl url = mFoldersForDownload.front();
    mFoldersForDownload.pop_front();
    kDebug(5800) <<"listItems:" << url.url();

    adaptor()->adaptDownloadUrl( url );
    kDebug(5800) <<"listItems, after adaptDownloadUrl:" << url.url();

    kDebug(5800) <<"OpenGroupware::listIncidences():" << url;

    mListItemsData.clear();
    mListEventsJob = adaptor()->createListItemsJob( url );

    connect( mListEventsJob, SIGNAL( result( KJob * ) ),
             SLOT( slotListItemsResult( KJob * ) ) );
    connect( mListEventsJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotListItemsData( KIO::Job *, const QByteArray & ) ) );
  }
}


void GroupwareDownloadJob::slotListItemsData( KIO::Job *, const QByteArray &data )
{
  kDebug(5800) <<"OpenGroupware::slotListItemsData()";

  mListItemsData.append( data.data() );
}


void GroupwareDownloadJob::slotListItemsResult( KJob *job )
{
  kDebug(5800) <<"GroupwareDownloadJob::slotListItemsResult():";

  if ( job->error() ) {
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
    error( job->errorString() );
  } else {
    adaptor()->interpretListItemsJob( static_cast<KIO::Job*>(job), mListItemsData );
  }

  mListItemsData.clear();
  mListEventsJob = 0;

  listItems();
}

void GroupwareDownloadJob::deleteIncidencesGoneFromServer()
{
  QMap<QString, QString> remoteIds( adaptor()->idMapper()->remoteIdMap() );
  KUrl::List::ConstIterator it = mCurrentlyOnServer.begin();
  while ( it != mCurrentlyOnServer.end() ) {
    remoteIds.remove( (*it).path() );
    ++it;
  }
  QMap<QString, QString>::ConstIterator it2;
  for (it2 = remoteIds.begin(); it2 != remoteIds.end(); ++it2) {
    adaptor()->deleteItem( remoteIds[ it2.key() ] );
  }
}

void GroupwareDownloadJob::downloadItem()
{
  kDebug(7000) <<" downloadItem";
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
      mDownloadItemsData.clear();
      mDownloadJob = adaptor()->createDownloadJob( mItemsForDownload );
      mItemsDownloading = mItemsForDownload;
      mItemsForDownload.clear();
    } else {
      // Download the first item of the list
      QMap<KUrl,KPIM::FolderLister::ContentType>::Iterator it = mItemsForDownload.begin();
      KUrl href( it.key() );
      KPIM::FolderLister::ContentType ctype = it.value();
      mItemsDownloading.insert( it.key(), it.value() );
      mItemsForDownload.remove( it.key() );
 
      adaptor()->adaptDownloadUrl( href );
      mDownloadItemsData.clear();

      mDownloadJob = adaptor()->createDownloadJob( href, ctype );
    }
    connect( mDownloadJob, SIGNAL( result( KJob * ) ),
        SLOT( slotDownloadItemResult( KJob * ) ) );
    connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotDownloadItemData( KIO::Job *, const QByteArray & ) ) );
  }
}

void GroupwareDownloadJob::slotDownloadItemResult( KJob *job )
{
  kDebug(5800) <<"GroupwareDownloadJob::slotDownloadItemResult():";

  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  if ( !trfjob ) return;

  if ( job->error() ) {
    error( job->errorString() );
  } else {
    adaptor()->interpretDownloadItemsJob( static_cast<KIO::Job*>(job), mDownloadItemsData );
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mDownloadItemsData.clear();
  mDownloadJob = 0;

  downloadItem();
}

void GroupwareDownloadJob::slotDownloadItemData( KIO::Job *, const QByteArray &data )
{
  kDebug(5800) <<"OpenGroupware::slotDownloadItemData()";

  mDownloadItemsData.append( QString::fromUtf8( data.data(), data.size() ) );
}

void GroupwareDownloadJob::slotItemToDownload( const KUrl &remoteURL,
                         KPIM::FolderLister::ContentType type )
{
  KUrl url( remoteURL );
  adaptor()->adaptDownloadUrl( url );
  if ( !mItemsForDownload.contains( url ) &&
       !mItemsDownloading.contains( url ) &&
       !mItemsDownloaded.contains( url ) ) {
    mItemsForDownload.insert( url, type );
  }
}


void GroupwareDownloadJob::slotItemOnServer( const KUrl &remoteURL )
{
kDebug()<<"GroupwareDownloadJob::slotItemOnServer(" << remoteURL.url() <<")";
  if ( !mCurrentlyOnServer.contains( remoteURL ) ) {
    mCurrentlyOnServer.append( remoteURL );
  }
}


void GroupwareDownloadJob::slotItemDownloadError( const KUrl &remoteURL, const QString &/*error*/ )
{
  // TODO: Error handling!
  if ( mItemsDownloading.contains( remoteURL ) ) {
    mItemsDownloadError[ remoteURL ] = mItemsDownloading[ remoteURL ];
  } else if ( mItemsForDownload.contains( remoteURL ) ) {
    mItemsDownloadError[ remoteURL ] = mItemsForDownload[ remoteURL ];
  }
}


void GroupwareDownloadJob::slotItemDownloaded( const QString &localID,
         const KUrl &remoteURL, const QString &fingerprint )
{
kDebug()<<"GroupwareDownloadJob::slotItemDownloaded(" << localID <<"," << remoteURL.url() <<"," << fingerprint <<")";
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
