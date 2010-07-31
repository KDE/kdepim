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

#include "groupwareuploadjob.h"

#include "folderlister.h"
#include "groupwaredataadaptor.h"

#include <libemailfunctions/idmapper.h>
#include <libkdepim/progressmanager.h>

#include <kdebug.h>
#include <kurl.h>
#include <kio/job.h>

#include <tqstringlist.h>
#include <tqtimer.h>
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
    TQT_SIGNAL( progressItemCanceled( KPIM::ProgressItem * ) ),
    TQT_SLOT( cancelSave() ) );

  connect( adaptor(), TQT_SIGNAL( itemDeletionError( const KURL &, const TQString & ) ),
           TQT_SLOT( slotItemDeleteError( const KURL &, const TQString & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemUploadError( const KURL &, const TQString & ) ),
           TQT_SLOT( slotItemUploadError( const KURL &, const TQString & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemUploadNewError( const TQString &, const TQString & ) ),
           TQT_SLOT( slotItemUploadNewError( const TQString &, const TQString & ) ) );

  connect( adaptor(), TQT_SIGNAL( itemDeleted( const TQString &, const KURL & ) ),
           TQT_SLOT( slotItemDeleted( const TQString &, const KURL & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemUploaded( const TQString &, const KURL & ) ),
           TQT_SLOT( slotItemUploaded( const TQString &, const KURL & ) ) );
  connect( adaptor(), TQT_SIGNAL( itemUploadedNew( const TQString &, const KURL& ) ),
           TQT_SLOT( slotItemUploadedNew( const TQString &, const KURL & ) ) );


  mUploadProgress->setTotalItems( mAddedItems.size() + mChangedItems.size() +
                           ((mChangedItems.isEmpty())?0:1) );
  mUploadProgress->updateProgress();
}



void GroupwareUploadJob::deleteItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::deleteItem()"<<endl;
  if ( mDeletedItems.isEmpty() ) {
    TQTimer::singleShot( 0, this, TQT_SLOT( uploadItem() ) );
  } else {
    kdDebug(7000) << " Deleting " << mDeletedItems.size() << " items from the server " << endl;

    KURL url( adaptor()->baseURL() );
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
      mDeletionJobData = TQString::null;
      connect( mDeletionJob, TQT_SIGNAL( result( KIO::Job* ) ),
               TQT_SLOT( slotDeletionJobResult( KIO::Job* ) ) );
//       connect( mDeletionJob, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
//                TQT_SLOT( slotDeletionJobData( KIO::Job *, const TQByteArray & ) ) );
    } else {
      deleteItem();
    }
  }
}



void GroupwareUploadJob::slotDeletionJobData( KIO::Job *, const TQByteArray &data )
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
  TQTimer::singleShot( 0, this, TQT_SLOT( deleteItem() ) );
}



void GroupwareUploadJob::uploadItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadItem()"<<endl;
  if ( mChangedItems.isEmpty() ) {
    TQTimer::singleShot( 0, this, TQT_SLOT( uploadNewItem() ) );
  } else {
    kdDebug(5800)<<"We still have "<<mChangedItems.count()<<" changed items to upload"<<endl;

    KURL url( adaptor()->baseURL() );
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
      mUploadJobData = TQString::null;
      connect( mUploadJob, TQT_SIGNAL( result( KIO::Job* ) ),
               TQT_SLOT( slotUploadJobResult( KIO::Job* ) ) );
      connect( mUploadJob, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
               TQT_SLOT( slotUploadJobData( KIO::Job *, const TQByteArray & ) ) );
    } else {
      uploadItem();
    }
  }
}


void GroupwareUploadJob::slotUploadJobData( KIO::Job *, const TQByteArray &data )
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
  TQTimer::singleShot( 0, this, TQT_SLOT( uploadItem() ) );
}




void GroupwareUploadJob::uploadNewItem()
{
  kdDebug(5800)<<"GroupwareUploadJob::uploadNewItem()"<<endl;
  if ( !mAddedItems.isEmpty() ) {
    
    if ( adaptor()->flags() & KPIM::GroupwareDataAdaptor::GWResBatchCreate ) {
      KURL url( adaptor()->folderLister()->writeDestinationId( FolderLister::All ) );
      adaptor()->adaptUploadUrl( url );
kdDebug() << "Using batch create to " << url.url() << endl;
      mUploadJob = adaptor()->createUploadNewJob( url, mAddedItems );
      mItemsUploading += mAddedItems;
      mAddedItems.clear();
    } else {
      KPIM::GroupwareUploadItem *item = mAddedItems.front();
      KURL url( adaptor()->folderLister()->writeDestinationId( item->itemType() ) );
      adaptor()->adaptUploadUrl( url );
kdDebug() << "Not using batch create to " << url.url() << " for item of type " << item->itemType() << endl;
      if ( !url.isEmpty() ) {
        mUploadJob = adaptor()->createUploadNewJob( url, item );
        mItemsUploading.append( mAddedItems.front() );
      }
      mAddedItems.pop_front();
    }

    if ( mUploadJob ) {
      mUploadNewJobData = TQString::null;
      connect( mUploadJob, TQT_SIGNAL( result( KIO::Job* ) ),
               TQT_SLOT( slotUploadNewJobResult( KIO::Job* ) ) );
      connect( mUploadJob, TQT_SIGNAL( data( KIO::Job *, const TQByteArray & ) ),
               TQT_SLOT( slotUploadNewJobData( KIO::Job *, const TQByteArray & ) ) );
    } else {
      uploadNewItem();
    }

  } else {
    kdDebug(5800)<<"We are finished uploading all items. Setting progress to completed."<<endl;
    uploadCompleted();
  }
}

void GroupwareUploadJob::slotUploadNewJobData( KIO::Job *, const TQByteArray &data )
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
  TQTimer::singleShot( 0, this, TQT_SLOT( uploadNewItem() ) );
}


void GroupwareUploadJob::kill()
{
  cancelSave();
}

void GroupwareUploadJob::slotItemDeleted( const TQString &/*localId*/, const KURL &remoteURL )
{
  kdDebug() << "GroupwareUploadJob::slotItemDeleted, removal successful: "<< remoteURL.url() << endl;

  const TQString &remote = remoteURL.path();
  const TQString &local = adaptor()->idMapper()->localId( remote );
  if ( !local.isEmpty() ) {
    // TODO: Is the deleted status reset in the resource?
    adaptor()->deleteItem( local );
  }

  KPIM::GroupwareUploadItem::List allit( mDeletedItems );
  allit += mItemsUploading;
  allit += mItemsUploadError;

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == remoteURL.path() ) {
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



void GroupwareUploadJob::slotItemUploaded( const TQString &/*localId*/, const KURL &remoteURL )
{
  kdDebug() << "GroupwareUploadJob::slotItemUploaded, upload successful: "<< remoteURL.url() << endl;

  const TQString &remote = remoteURL.path();
  const TQString &local = adaptor()->idMapper()->localId( remote );
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
    if ( (*it)->url().path() == remoteURL.path() ) {
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


void GroupwareUploadJob::slotItemUploadedNew( const TQString &localId, const KURL &remoteURL )
{
  kdDebug() << "GroupwareUploadJob::slotItemUploadedNew, upload successful: "<< remoteURL.url() << endl;

  const TQString &remote = remoteURL.path();
  // TODO: For a new item this won't return anything, so we need to insert the
  // local<=>remote id map when creating the upload job... And maybe
  const TQString &local = adaptor()->idMapper()->localId( remote );
  if ( !localId.isEmpty() ) {
    adaptor()->deleteItem( localId );
  }
  if ( !local.isEmpty() ) {
//     adaptor()->itemUploadedNew( local, remoteURL );
  }

  KPIM::GroupwareUploadItem::List allit( mChangedItems );
  allit += mAddedItems;
  allit += mItemsUploading;
  allit += mItemsUploadError;


  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == remoteURL.path() ) {
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

void GroupwareUploadJob::slotItemDeleteError( const KURL &remoteURL, const TQString &/*error*/ )
{
  // TODO: Add to error list, remove from uploading and toUpload list
  kdDebug() << "GroupwareUploadJob::slotItemDeleteError, removal not successful: "<< remoteURL.url() << endl;
  KPIM::GroupwareUploadItem::List allit( mDeletedItems );
  allit += mItemsUploading;
  allit += mItemsUploaded;

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == remoteURL.path() ) {
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

void GroupwareUploadJob::slotItemUploadError( const KURL &remoteURL, const TQString &/*error*/ )
{
  // TODO: Add to error list, remove from uploading and toUpload list
  kdDebug() << "GroupwareUploadJob::slotItemUploadError, removal not successful: "<< remoteURL.url() << endl;
  KPIM::GroupwareUploadItem::List allit( mChangedItems );
  allit += mItemsUploading;
  allit += mItemsUploaded;

  KPIM::GroupwareUploadItem::List::Iterator it = allit.begin();
  for ( ; it != allit.end(); ++it ) {
    if ( (*it)->url().path() == remoteURL.path() ) {
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

void GroupwareUploadJob::slotItemUploadNewError( const TQString &/*localID*/, const TQString &remoteURL )
{
  kdDebug(5006) << "GroupwareUploadJob::slotItemUploadNewError, removal not successful: "<< remoteURL << endl;
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
    error( i18n("1 item could not be uploaded.", "%n items could not be uploaded.",  mItemsUploadError.count() ) );
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
