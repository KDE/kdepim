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
#ifndef KPIM_GROUPWAREUPLOADJOB_H
#define KPIM_GROUPWAREUPLOADJOB_H

#include "groupwareresourcejob.h"

#include <kurl.h>
#include <qstringlist.h>
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

    void slotDeletionResult( KIO::Job *job );
    void slotUploadJobResult( KIO::Job *job );
    void slotUploadNewJobResult( KIO::Job *job );

    void slotItemDeleted( const QString &localID, const QString &remoteURL );
    void slotItemUploaded( const QString &localID, const QString &remoteURL );
    void slotItemUploadedNew( const QString &localID, const QString &remoteURL );
    
    void slotItemDeleteError( const QString &remoteURL, const QString &error );
    void slotItemUploadError( const QString &remoteURL, const QString &error  );
    void slotItemUploadNewError( const QString &localID, const QString &error );

  private:
    KPIM::GroupwareUploadItem::List mAddedItems;
    KPIM::GroupwareUploadItem::List mChangedItems;
    KPIM::GroupwareUploadItem::List mDeletedItems;

    KURL mBaseUrl;

    KIO::TransferJob *mUploadJob;
    KIO::Job *mDeletionJob;
    KPIM::ProgressItem *mUploadProgress;

    KPIM::GroupwareUploadItem::List mItemsUploading;
    KPIM::GroupwareUploadItem::List mItemsUploaded;
    KPIM::GroupwareUploadItem::List mItemsUploadError;
};

}

#endif


