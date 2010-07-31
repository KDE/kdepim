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
#ifndef KPIM_GROUPWAREUPLOADJOB_H
#define KPIM_GROUPWAREUPLOADJOB_H

#include "groupwareresourcejob.h"

#include <kurl.h>
#include <tqstringlist.h>
#include <groupwaredataadaptor.h>

namespace KIO {
  class Job;
  class TransferJob;
  class DeleteJob;
}

namespace KPIM {

class GroupwareDataAdaptor;
class ProgressItem;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class GroupwareUploadJob : public GroupwareJob
{
    Q_OBJECT
  public:
    GroupwareUploadJob( GroupwareDataAdaptor *adaptor );

    KPIM::GroupwareUploadItem::List addedItems() const
    {
      return mAddedItems;
    }
    void setAddedItems( const KPIM::GroupwareUploadItem::List &items )
    {
      mAddedItems = items;
    }
    KPIM::GroupwareUploadItem::List changedItems() const
    {
      return mChangedItems;
    }
    void setChangedItems( const KPIM::GroupwareUploadItem::List &items )
    {
      mChangedItems = items;
    }
    KPIM::GroupwareUploadItem::List deletedItems() const
    {
      return mDeletedItems;
    }
    void setDeletedItems( const KPIM::GroupwareUploadItem::List &items )
    {
      mDeletedItems = items;
    }

    void kill();

  protected slots:
    void deleteItem();
    void uploadItem();
    void uploadNewItem();

  protected slots:
    void run();

    void cancelSave();

    void slotDeletionJobResult( KIO::Job *job );
    void slotDeletionJobData( KIO::Job *, const TQByteArray & );
    void slotUploadJobResult( KIO::Job *job );
    void slotUploadJobData( KIO::Job *, const TQByteArray & );
    void slotUploadNewJobResult( KIO::Job *job );
    void slotUploadNewJobData( KIO::Job *, const TQByteArray & );

    void slotItemDeleted( const TQString &localID, const KURL &remoteURL );
    void slotItemUploaded( const TQString &localID, const KURL &remoteURL );
    void slotItemUploadedNew( const TQString &localID, const KURL &remoteURL );

    void slotItemDeleteError( const KURL &remoteURL, const TQString &error );
    void slotItemUploadError( const KURL &remoteURL, const TQString &error  );
    void slotItemUploadNewError( const TQString &localID, const TQString &error );

    void uploadCompleted();

  private:
    KPIM::GroupwareUploadItem::List mAddedItems;
    KPIM::GroupwareUploadItem::List mChangedItems;
    KPIM::GroupwareUploadItem::List mDeletedItems;

    KPIM::GroupwareUploadItem::List mItemsUploading;
    KPIM::GroupwareUploadItem::List mItemsUploaded;
    KPIM::GroupwareUploadItem::List mItemsUploadError;

    TQString mDeletionJobData;
    TQString mUploadJobData;
    TQString mUploadNewJobData;

    KIO::TransferJob *mUploadJob;
    KIO::Job *mDeletionJob;
    KPIM::ProgressItem *mUploadProgress;

};

}

#endif


