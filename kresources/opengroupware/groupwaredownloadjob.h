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
#ifndef KPIM_GROUPWAREDOWNLOADJOB_H
#define KPIM_GROUPWAREDOWNLOADJOB_H

#include "groupwarejob.h"

#include <kurl.h>

#include <libkdepim/progressmanager.h>

#include <kconfig.h>

namespace KIO {
  class Job;
  class TransferJob;
  class DavJob;
}

namespace KPIM {

class GroupwareDataAdaptor;

/**
  This class provides a resource for accessing a Groupware kioslave-based
  calendar.
*/
class GroupwareDownloadJob : public GroupwareJob
{
    Q_OBJECT
  public:  
    GroupwareDownloadJob( GroupwareDataAdaptor *adaptor );

    void kill();

  protected:
    void listItems();
    void deleteIncidencesGoneFromServer();
    void downloadItem();

  protected slots:
    void run();

    void cancelLoad();

    void slotListJobResult( KIO::Job * );
    void slotJobResult( KIO::Job * );
    void slotJobData( KIO::Job *, const QByteArray & );
  
  private:
    GroupwareDataAdaptor *mAdaptor;

    QStringList mFoldersForDownload;
    QStringList mCurrentlyOnServer;

    QStringList mItemsForDownload;

    KPIM::ProgressItem *mProgress;

    KIO::TransferJob *mDownloadJob;
    KIO::DavJob *mListEventsJob;
    QString mJobData;

    QString mCurrentGetUrl;
};

}

#endif
