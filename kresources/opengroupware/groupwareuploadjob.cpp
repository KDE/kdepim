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

#include "groupwareuploadjob.h"

#include "webdavhandler.h"
#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <libemailfunctions/idmapper.h>
#include <libkdepim/progressmanager.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>

#include <qstringlist.h>
#include <qtimer.h>

using namespace KPIM;

GroupwareUploadJob::GroupwareUploadJob( GroupwareDataAdaptor *adaptor )
  : GroupwareJob( adaptor ), mUploadJob(0), 
    mDeletionJob(0), mUploadProgress(0)
{
}

void GroupwareUploadJob::run()
{
  deleteItem();
  mUploadProgress = KPIM::ProgressManager::instance()->createProgressItem(
    KPIM::ProgressManager::getUniqueID(),
    adaptor()->uploadProgressMessage() );
  connect( mUploadProgress,
    SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
    SLOT( cancelSave() ) );

  mUploadProgress->setTotalItems( mAddedItems.size() + mChangedItems.size() +
                           ((mChangedItems.isEmpty())?0:1) );
  mUploadProgress->updateProgress();

}

void GroupwareUploadJob::deleteItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::deleteItem()"<<endl;
  if ( mDeletedItems.isEmpty() ) {
    QTimer::singleShot( 0, this, SLOT( uploadItem() ) );
  } else {
    kdDebug(7000) << " Deleting " << mDeletedItems.size() << " items from the server " << endl;

    KURL url( mBaseUrl );
    url = WebdavHandler::toDAV( url );
    
    // TODO: What to do with servers that don't allow you to remove all incidences at once?
    mDeletionJob = adaptor()->createRemoveItemsJob( url, mDeletedItems );
    connect( mDeletionJob, SIGNAL( result( KIO::Job* ) ),
             SLOT( slotDeletionResult( KIO::Job* ) ) );
  }
}

void GroupwareUploadJob::slotDeletionResult( KIO::Job *job )
{
  KIO::DeleteJob *deljob = dynamic_cast<KIO::DeleteJob*>(job);
  if ( job->error() ) {
    kdDebug(5006) << "slotDeletionResult failed " << endl;
    error( job->errorString() );
  } else if ( deljob ) {
    kdDebug(5006) << "slotDeletionResult successfull " << endl;
    
    KURL::List urls( deljob->urls() );
    for ( KURL::List::Iterator it = urls.begin(); it != urls.end(); ++it ) {
      const QString &remote = (*it).path();
      const QString &local = adaptor()->idMapper()->localId( remote );
      if ( !local.isEmpty() ) {
        adaptor()->deleteItem( local );
      }
    }
  }
  KPIM::GroupwareUploadItem::List::Iterator it = mDeletedItems.begin();
  for ( ; it != mDeletedItems.end(); ++it ) {
    delete (*it);  
  }
  mDeletedItems.clear();
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
  QTimer::singleShot( 0, this, SLOT( uploadItem() ) );
}

void GroupwareUploadJob::uploadItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadItem()"<<endl;
  if ( mChangedItems.isEmpty() ) {
    QTimer::singleShot( 0, this, SLOT( uploadNewItem() ) );
  } else {
    kdDebug(5800)<<"We still have "<<mAddedItems.count()<<" changed items to upload"<<endl;
    GroupwareUploadItem *item = mChangedItems.front();
    if ( !item ) {
      mChangedItems.pop_front();
      emit QTimer::singleShot( 0, this, SLOT( uploadItem() ) );
      return;
    }
    QString uid = item->uid();
    const QString remote = adaptor()->idMapper()->remoteId( uid );
    if ( remote.isEmpty() ) {
      mAddedItems.append( item );
      emit QTimer::singleShot( 0, this, SLOT( uploadItem() ) );
      return;
    }
    KURL url( mBaseUrl );
    url.setPath( remote );
    adaptor()->setUserPassword( url );
    mUploadJob = adaptor()->createUploadJob( url, item );
    connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
      SLOT( slotUploadJobResult( KIO::Job * ) ) );
  }
}

void GroupwareUploadJob::slotUploadJobResult( KIO::Job *job )
{
  kdDebug(7000) << " slotUploadJobResult " << endl;
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  if ( !trfjob ) return;

  if ( job->error() ) {
    error( job->errorString() );
  } else {
    adaptor()->uploadFinished( trfjob, mChangedItems.front() );
    delete mChangedItems.front();
    mChangedItems.pop_front();
  }

  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
  mUploadJob = 0;

  uploadItem();
}

void GroupwareUploadJob::uploadNewItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadNewItem()"<<endl;
  if ( !mAddedItems.isEmpty() ) {
    kdDebug(5800)<<"We still have "<<mAddedItems.count()<<" new items to upload"<<endl;
    GroupwareUploadItem *item = mAddedItems.front();
    if ( !item ) {
      delete mAddedItems.front();
      mAddedItems.pop_front();
      emit QTimer::singleShot( 0, this, SLOT( uploadNewItem() ) );
      return;
    }
    QString uid = item->uid();
    
    KURL url( adaptor()->folderLister()->writeDestinationId() );
    adaptor()->setUserPassword( url );
    adaptor()->adaptUploadUrl( url );
    kdDebug(5800) << "Put new URL: " << url.url() << endl;
    
    mUploadJob = adaptor()->createUploadNewJob( url, item );
    
    connect( mUploadJob, SIGNAL( result( KIO::Job * ) ),
      SLOT( slotUploadNewJobResult( KIO::Job * ) ) );
  } else {
    kdDebug(5800)<<"We are finished uploading all items. Setting progress to completed."<<endl;
    if ( mUploadProgress ) {
      mUploadProgress->setComplete();
      mUploadProgress = 0;
    }
    success();
  }
}

void GroupwareUploadJob::slotUploadNewJobResult( KIO::Job *job )
{
  kdDebug(7000) << " slotUploadNewJobResult " << endl;
  KIO::TransferJob *trfjob = dynamic_cast<KIO::TransferJob*>(job);
  if ( !trfjob ) return;

  if ( job->error() ) {
kdDebug(7000) << "   error!!!, string="<<job->errorString()<<endl;
    error( job->errorString() );
  } else {
//     TODO: Don't update the etag, but instead let the download job download that new
//     item. Otherwise we won't know the url of the item!
    adaptor()->uploadFinished( trfjob, mAddedItems.front() );
    delete mAddedItems.front();
    mAddedItems.pop_front();
  }

  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
  mUploadJob = 0;

  uploadNewItem();
}

void GroupwareUploadJob::kill()
{
  cancelSave();
}

void GroupwareUploadJob::cancelSave()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
  mUploadProgress = 0;
}

#include "groupwareuploadjob.moc"
