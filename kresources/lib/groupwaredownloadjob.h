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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KPIM_GROUPWAREDOWNLOADJOB_H
#define KPIM_GROUPWAREDOWNLOADJOB_H

#include "groupwareresourcejob.h"
#include <qstringlist.h>

namespace KIO {
  class Job;
  class TransferJob;
}

namespace KPIM {

class GroupwareDataAdaptor;
class ProgressItem;

/**
  This class provides a resource for accessing a Groupware calendar with local caching.
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

    void slotListItemsResult( KIO::Job * );
    void slotListItemsData( KIO::Job *, const QByteArray & );
    void slotDownloadItemResult( KIO::Job * );
    void slotDownloadItemData( KIO::Job *, const QByteArray & );
    
    void slotItemToDownload( const KURL &remoteURL,
                             KPIM::FolderLister::ContentType type );
    void slotItemOnServer( const KURL &remoteURL );
    void slotItemDownloaded( const QString &localID, const KURL &remoteURL,
                             const QString &fingerprint );
    void slotItemDownloadError( const KURL &remoteURL, const QString &error );

  private:
    KURL::List mFoldersForDownload;
    /** mCurrentlyOnServer is the list of pathes of all items on the server.
        These pathes don't contain the server name! */
    KURL::List mCurrentlyOnServer;

    QMap<KURL,KPIM::FolderLister::ContentType> mItemsForDownload;
    QMap<KURL,KPIM::FolderLister::ContentType> mItemsDownloading;
    QMap<KURL,KPIM::FolderLister::ContentType> mItemsDownloaded;
    QMap<KURL,KPIM::FolderLister::ContentType> mItemsDownloadError;

    KPIM::ProgressItem *mProgress;

    KIO::TransferJob *mDownloadJob;
    KIO::TransferJob *mListEventsJob;
    
    QString mListItemsData;
    QString mDownloadItemsData;
};

}

#endif
