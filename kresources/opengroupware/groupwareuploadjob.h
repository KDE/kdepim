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

#include "groupwarejob.h"

#include <kurl.h>

#include <libkdepim/progressmanager.h>
#include <libkcal/resourcecached.h>

#include <kconfig.h>

namespace KIO {
  class Job;
  class TransferJob;
  class DeleteJob;
}

namespace KPIM {

class GroupwareDataAdaptor;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class GroupwareUploadJob : public GroupwareJob
{
    Q_OBJECT
  public:  
    GroupwareUploadJob( GroupwareDataAdaptor *adaptor );

    /**
      Set urls of objects to be deleted.
    */
    void setUrlsForDeletion( const QStringList &v )
    {
      mUrlsForDeletion = v;
    }
    /**
      Get urls scheduled for deletion. See setUrlsForDeletion().
    */
    QStringList urlsForDeletion() const
    {
      return mUrlsForDeletion;
    }

    /**
      Set data to be uploaded.
    */
    void setDataForUpload( const QStringList &v )
    {
      mDataForUpload = v;
    }
    /**
      Get dataForUpload. See setDataForUpload().
    */
    QStringList dataForUpload() const
    {
      return mDataForUpload;
    }

    /**
      Set base URL.
    */
    void setBaseUrl( const KURL &v )
    {
      mBaseUrl = v;
    }
    /**
      Get baseUrl. See setBaseUrl().
    */
    KURL baseUrl() const
    {
      return mBaseUrl;
    }

    void kill();

  protected:
    void deleteItems();
    void uploadItem();

  protected slots:
    void run();

    void cancelSave();

    void slotDeletionResult( KIO::Job *job );
    void slotUploadJobResult( KIO::Job *job );
  
  private:
    QStringList mUrlsForDeletion;
    QStringList mDataForUpload;
    KURL mBaseUrl;

    KIO::TransferJob *mUploadJob;
    KIO::DeleteJob *mDeletionJob;
    KPIM::ProgressItem *mUploadProgress;

    QStringList mIncidencesForUpload;
    QStringList mIncidencesForDeletion;
    
    KURL mCurrentPutUrl;
};

}

#endif


