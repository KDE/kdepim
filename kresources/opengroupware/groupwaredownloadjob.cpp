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

#include "groupwaredownloadjob.h"

#include "webdavhandler.h"
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

  mFoldersForDownload = adaptor()->folderLister()->activeFolderIds();

  mItemsForDownload.clear();
  mCurrentlyOnServer.clear();

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
      mProgress->setTotalItems( mItemsForDownload.count() );
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

    adaptor()->setUserPassword( url );
    adaptor()->adaptDownloadUrl( url );
    kdDebug(5800) << "listItems, after setUserPassword: " << url.url() << endl;

    kdDebug(5800) << "OpenGroupware::listIncidences(): " << url << endl;

    mListEventsJob = adaptor()->createListItemsJob( url );

    connect( mListEventsJob, SIGNAL( result( KIO::Job * ) ),
             SLOT( slotListJobResult( KIO::Job * ) ) );
  }
}

void GroupwareDownloadJob::slotListJobResult( KIO::Job *job )
{
  kdDebug(5800) << "GroupwareDownloadJob::slotListJobResult(): " << endl;

  if ( job->error() ) {
    if ( mProgress ) {
      mProgress->setComplete();
      mProgress = 0;
    }
    error( job->errorString() );
  } else {
    adaptor()->itemsForDownloadFromList( job, mCurrentlyOnServer, mItemsForDownload );
  }

  mListEventsJob = 0;

  listItems();
}

void GroupwareDownloadJob::deleteIncidencesGoneFromServer()
{
  QMap<QString, QString> remoteIds( adaptor()->idMapper()->remoteIdMap() );
  QStringList::ConstIterator it = mCurrentlyOnServer.begin();
  while ( it != mCurrentlyOnServer.end() ) {
    remoteIds.remove( (*it) );
    ++it;
  }
  QMap<QString, QString>::ConstIterator it2;
  for (it2 = remoteIds.begin(); it2 != remoteIds.end(); ++it2) {
    adaptor()->deleteItem( remoteIds[ it2.key() ] );
  }
}

void GroupwareDownloadJob::downloadItem()
{
  kdDebug(7000) << " downloadItem " << endl;
  if ( !mItemsForDownload.isEmpty() ) {
    const QString entry = mItemsForDownload.front();
    mItemsForDownload.pop_front();

    KURL url( entry );
//    url.setProtocol( "webdav" );
    adaptor()->setUserPassword( url );
    adaptor()->adaptDownloadUrl( url );

    mJobData = QString::null;

    mDownloadJob = adaptor()->createDownloadItemJob( url );
    connect( mDownloadJob, SIGNAL( result( KIO::Job * ) ),
        SLOT( slotJobResult( KIO::Job * ) ) );
    connect( mDownloadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
        SLOT( slotJobData( KIO::Job *, const QByteArray & ) ) );
  } else {
    if ( mProgress ) mProgress->setComplete();
    mProgress = 0;
    success();
  }
}

void GroupwareDownloadJob::slotJobResult( KIO::Job *job )
{
  kdDebug(5800) << "OpenGroupware::slotJobResult(): " << endl;

  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  if ( !trfjob ) return;

  if ( job->error() ) {
    error( job->errorString() );
  } else {
    const QString &remote = KURL( trfjob->url() ).path();
    const QString &local = adaptor()->idMapper()->localId( remote );

    // remove old version, we would not have downnloaded 
    // if it were still current
    adaptor()->deleteItem( local );
    
    QString fingerprint; // <- will be set by addItem
    QString id = adaptor()->addItem( trfjob, mJobData, fingerprint, local, remote );

    if ( id.isEmpty() ) {
      error( i18n("Error parsing calendar data.") );
    } else {
      if ( local.isEmpty() ) {
        adaptor()->idMapper()->setRemoteId( id, remote );
      }
      adaptor()->idMapper()->setFingerprint( id, fingerprint );
    }
  }

  if ( mProgress ) {
    mProgress->incCompletedItems();
    mProgress->updateProgress();
  }
  mJobData = QString::null;
  mDownloadJob = 0;

  downloadItem();
}

void GroupwareDownloadJob::slotJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotJobData()" << endl;

  mJobData.append( data.data() );
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
