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

#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <libemailfunctions/idmapper.h>
#include <libkdepim/progressmanager.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>

#include <qstringlist.h>
#include <qtimer.h>
#include <klocale.h>

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

  connect( adaptor(), SIGNAL( itemDeletionError( const QString &, const QString & ) ),
           SLOT( slotItemDeleteError( const QString &, const QString & ) ) );
  connect( adaptor(), SIGNAL( itemUploadError( const QString &, const QString & ) ),
           SLOT( slotItemUploadError( const QString &, const QString & ) ) );
  connect( adaptor(), SIGNAL( itemUploadNewError( const QString &, const QString & ) ),
           SLOT( slotItemUploadNewError( const QString &, const QString & ) ) );

  connect( adaptor(), SIGNAL( itemDeleted( const QString &, const QString & ) ),
           SLOT( slotItemDeleted( const QString &, const QString & ) ) );
  connect( adaptor(), SIGNAL( itemUploaded( const QString &, const QString & ) ),
           SLOT( slotItemUploaded( const QString &, const QString & ) ) );
  connect( adaptor(), SIGNAL( itemUploadedNew( const QString &, const QString & ) ),
           SLOT( slotItemUploadedNew( const QString &, const QString & ) ) );


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
    adaptor()->adaptUploadUrl( url );
    if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResBatchDelete ) {
kdDebug() << "Using batch delete " << endl;
      mDeletionJob = adaptor()->createRemoveJob( url, mDeletedItems );
      mItemsUploading += mDeletedItems;
      mDeletedItems.clear();
    } else {
kdDebug() << "Not using batch delete " << endl;
      KPIM::GroupwareUploadItem *item = mDeletedItems.front();
      mDeletionJob = adaptor()->createRemoveJob( url, item );
      mItemsUploading.append( mDeletedItems.front() );
      mDeletedItems.pop_front();
    }

    if ( mDeletionJob ) {
      mDeletionJobData = QString::null;
      connect( mDeletionJob, SIGNAL( result( KIO::Job* ) ),
               SLOT( slotDeletionJobResult( KIO::Job* ) ) );
//       connect( mDeletionJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
//                SLOT( slotDeletionJobData( KIO::Job *, const QByteArray & ) ) );
    } else {
      deleteItem();
    }
  }
}



void GroupwareUploadJob::slotDeletionJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotDeletionData()" << endl;

  mDeletionJobData.append( data.data() );
}

    

void GroupwareUploadJob::slotDeletionJobResult( KIO::Job *job )
{
  if ( job  && adaptor() ) {
    adaptor()->interpretRemoveJob( job, mDeletionJobData );
  }
  mDeletionJob = 0;
  QTimer::singleShot( 0, this, SLOT( deleteItem() ) );
}



void GroupwareUploadJob::uploadItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadItem()"<<endl;
  if ( mChangedItems.isEmpty() ) {
    QTimer::singleShot( 0, this, SLOT( uploadNewItem() ) );
  } else {
    kdDebug(5800)<<"We still have "<<mChangedItems.count()<<" changed items to upload"<<endl;

    KURL url( mBaseUrl );
    adaptor()->adaptUploadUrl( url );
    if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResBatchModify ) {
kdDebug() << "Using batch upload " << endl;
      mUploadJob = adaptor()->createUploadJob( url, mChangedItems );
      mItemsUploading += mChangedItems;
      mChangedItems.clear();
    } else {
kdDebug() << "Not using batch upload " << endl;
      KPIM::GroupwareUploadItem *item = mChangedItems.front();
      mUploadJob = adaptor()->createUploadJob( url, item );
      mItemsUploading.append( mChangedItems.front() );
      mChangedItems.pop_front();
    }

    if ( mUploadJob ) {
      mUploadJobData = QString::null;
      connect( mUploadJob, SIGNAL( result( KIO::Job* ) ),
               SLOT( slotUploadJobResult( KIO::Job* ) ) );
      connect( mUploadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
               SLOT( slotUploadJobData( KIO::Job *, const QByteArray & ) ) );
    } else {
      uploadItem();
    }
  }
}


void GroupwareUploadJob::slotUploadJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotUploadData()" << endl;

  mUploadJobData.append( data.data() );
}



void GroupwareUploadJob::slotUploadJobResult( KIO::Job *job )
{
  if ( job  && adaptor() ) {
    adaptor()->interpretUploadJob( job, mUploadJobData );
  }
  mUploadJob = 0;
  QTimer::singleShot( 0, this, SLOT( uploadItem() ) );
}




void GroupwareUploadJob::uploadNewItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadNewItem()"<<endl;
  if ( !mAddedItems.isEmpty() ) {
    KURL url( mBaseUrl );
    adaptor()->adaptUploadUrl( url );
    if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResBatchCreate ) {
kdDebug() << "Using batch create" << endl;
      mUploadJob = adaptor()->createUploadNewJob( url, mAddedItems );
      mItemsUploading += mAddedItems;
      mAddedItems.clear();
    } else {
kdDebug() << "Not using batch create" << endl;
      KPIM::GroupwareUploadItem *item = mAddedItems.front();
      mUploadJob = adaptor()->createUploadNewJob( url, item );
      mItemsUploading.append( mAddedItems.front() );
      mAddedItems.pop_front();
    }

    if ( mUploadJob ) {
      mUploadNewJobData = QString::null;
      connect( mUploadJob, SIGNAL( result( KIO::Job* ) ),
               SLOT( slotUploadNewJobResult( KIO::Job* ) ) );
      connect( mUploadJob, SIGNAL( data( KIO::Job *, const QByteArray & ) ),
               SLOT( slotUploadNewJobData( KIO::Job *, const QByteArray & ) ) );
    } else {
      uploadNewItem();
    }
    
  } else {
    kdDebug(5800)<<"We are finished uploading all items. Setting progress to completed."<<endl;
    uploadCompleted();
  }
}

void GroupwareUploadJob::slotUploadNewJobData( KIO::Job *, const QByteArray &data )
{
  kdDebug(5800) << "OpenGroupware::slotUploadNewJobData()" << endl;

  mUploadNewJobData.append( data.data() );
}



void GroupwareUploadJob::slotUploadNewJobResult( KIO::Job *job )
{
  if ( job  && adaptor() ) {
    adaptor()->interpretUploadNewJob( job, mUploadNewJobData );
  }
  mUploadJob = 0;
  QTimer::singleShot( 0, this, SLOT( uploadNewItem() ) );
}


void GroupwareUploadJob::kill()
{
  cancelSave();
}

void GroupwareUploadJob::slotItemDeleted( const QString &/*localId*/, const QString &remoteURL )
{
  kdDebug(5006) << "GroupwareUploadJob::slotItemDeleted, removal successful: "<< remoteURL << endl;
  const KURL &url( remoteURL );

  const QString &remote = url.path();
  const QString &local = adaptor()->idMapper()->localId( remote );
  if ( !local.isEmpty() ) {
    // TODO: Is the deleted status reset in the resource?
    adaptor()->deleteItem( local );
  }

  KPIM::GroupwareUploadItem::List allit( mDeletedItems );
  allit += mItemsUploading;
  allit += mItemsUploadError;

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mDeletedItems.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.remove( item );
      mItemsUploaded.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}



void GroupwareUploadJob::slotItemUploaded( const QString &/*localId*/, const QString &remoteURL )
{
  kdDebug(5006) << "GroupwareUploadJob::slotItemUploaded, upload successful: "<< remoteURL << endl;
  const KURL &url( remoteURL );

  const QString &remote = url.path();
  const QString &local = adaptor()->idMapper()->localId( remote );
  if ( !local.isEmpty() ) {
    // TODO: Is the deleted status reset in the resource?
//     adaptor()->itemUploaded( local, remoteURL );
  }

  KPIM::GroupwareUploadItem::List allit( mChangedItems );
  allit += mAddedItems;
  allit += mItemsUploading;
  allit += mItemsUploadError;
  

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mChangedItems.remove( item );
      mAddedItems.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.remove( item );
      mItemsUploaded.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}


void GroupwareUploadJob::slotItemUploadedNew( const QString &/*localId*/, const QString &remoteURL )
{
  kdDebug(5006) << "GroupwareUploadJob::slotItemUploadedNew, upload successful: "<< remoteURL << endl;
  const KURL &url( remoteURL );

  const QString &remote = url.path();
  // TODO: For a new item this won't return anything, so we need to insert the
  // local<=>remote id map when creating the upload job... And maybe 
  const QString &local = adaptor()->idMapper()->localId( remote );
  if ( !local.isEmpty() ) {
//     adaptor()->itemUploadedNew( local, remoteURL );
  }

  KPIM::GroupwareUploadItem::List allit( mChangedItems );
  allit += mAddedItems;
  allit += mItemsUploading;
  allit += mItemsUploadError;
  

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mChangedItems.remove( item );
      mAddedItems.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.remove( item );
      mItemsUploaded.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}

void GroupwareUploadJob::slotItemDeleteError( const QString &/*localID*/, const QString &remoteURL )
{
  // TODO: Add to error list, remove from uploading and toUpload list
  kdDebug(5006) << "GroupwareUploadJob::slotItemDeleted, removal successful: "<< remoteURL << endl;
  KPIM::GroupwareUploadItem::List allit( mDeletedItems );
  allit += mItemsUploading;
  allit += mItemsUploaded;
  const KURL &url( remoteURL );

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mDeletedItems.remove( item );
      mItemsUploaded.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}

void GroupwareUploadJob::slotItemUploadError( const QString &/*localID*/, const QString &remoteURL )
{
  // TODO: Add to error list, remove from uploading and toUpload list
  kdDebug(5006) << "GroupwareUploadJob::slotItemDeleted, removal successful: "<< remoteURL << endl;
  KPIM::GroupwareUploadItem::List allit( mChangedItems );
  allit += mItemsUploading;
  allit += mItemsUploaded;
  const KURL &url( remoteURL );

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mChangedItems.remove( item );
      mItemsUploaded.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}

void GroupwareUploadJob::slotItemUploadNewError( const QString &/*localID*/, const QString &remoteURL )
{
  kdDebug(5006) << "GroupwareUploadJob::slotItemDeleted, removal successful: "<< remoteURL << endl;
  KPIM::GroupwareUploadItem::List allit( mAddedItems );
  allit += mItemsUploading;
  allit += mItemsUploaded;
  const KURL &url( remoteURL );

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == url.path() ) {
kdDebug()<<"Found it in the list!"<<endl;
      KPIM::GroupwareUploadItem *item = (*it);
      mAddedItems.remove( item );
      mItemsUploaded.remove( item );
      mItemsUploading.remove( item );
      mItemsUploadError.append( item );
    }
  }
  
  if ( mUploadProgress ) {
    mUploadProgress->incCompletedItems();
    mUploadProgress->updateProgress();
  }
}


void GroupwareUploadJob::cancelSave()
{
  if ( mUploadJob ) mUploadJob->kill();
  mUploadJob = 0;
  if ( mUploadProgress ) mUploadProgress->setComplete();
  mUploadProgress = 0;
}

void GroupwareUploadJob::uploadCompleted()
{
  if ( !mItemsUploadError.isEmpty() ) {
    error( i18n("%1 items could not be uploaded.", "1 item could not be uploaded.", mItemsUploadError.count() ) );
  }
  KPIM::GroupwareUploadItem::List items( mAddedItems );
  items += mChangedItems;
  items += mDeletedItems;
  items += mItemsUploading;
  items += mItemsUploaded;
  items += mItemsUploadError;

  mAddedItems.clear();
  mChangedItems.clear();
  mDeletedItems.clear();
  mItemsUploading.clear();
  mItemsUploaded.clear();
  mItemsUploadError.clear();
  items.setAutoDelete( true );
  items.clear();

  if ( mUploadProgress ) {
    mUploadProgress->setComplete();
    mUploadProgress = 0;
  }
  success();

}
#include "groupwareuploadjob.moc"
