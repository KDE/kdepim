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
#ifndef KPIM_GROUPWAREDATAADAPTOR_H
#define KPIM_GROUPWAREDATAADAPTOR_H

#include <groupwareresourcejob.h>
#include <kurl.h>
#include <qstring.h>
#include <libkcal/listbase.h>


namespace KIO {
class TransferJob;
class Job;
}

namespace KPIM {

class FolderLister;
class IdMapper;
class GroupwareDataAdaptor;
//class GroupwareJob;

class GroupwareUploadItem
{
  public:
    typedef KCal::ListBase<GroupwareUploadItem> List;
    enum UploadType {
      Added,
      Changed,
      Deleted
    };

    GroupwareUploadItem( UploadType type );
    virtual ~GroupwareUploadItem() {}
    KURL url() const { return mUrl; }
    void setUrl( const KURL &url ) { mUrl = url; }

    QString uid() const { return mUid; }
    void setUid( const QString &uid ) { mUid = uid; }

    virtual QString data() const { return mData; }
    virtual void setData( const QString &data ) { mData = data; }
    virtual KURL adaptNewItemUrl( GroupwareDataAdaptor *adaptor, const KURL &url );

    virtual KIO::TransferJob *createUploadNewJob( GroupwareDataAdaptor *adaptor, const KURL &url );
    virtual KIO::TransferJob *createUploadJob( GroupwareDataAdaptor *adaptor, const KURL &url );
  private:
    KURL mUrl;
    QString mUid;
    QString mData;
    UploadType mType;
};


class GroupwareDataAdaptor
{
  public:
    GroupwareDataAdaptor();
    virtual ~GroupwareDataAdaptor();

    void setFolderLister( FolderLister *folderLister )
    {
      mFolderLister = folderLister;
    }
    FolderLister *folderLister()
    {
      return mFolderLister;
    }

    /**
      Set progress message shown by progress manager during download.
    */
    void setDownloadProgressMessage( const QString &v )
    {
      mDownloadProgressMessage = v;
    }
    /**
      Get download progress message. See setDownloadProgressMessage().
    */
    QString downloadProgressMessage() const
    {
      return mDownloadProgressMessage;
    }

    /**
      Set progress message shown by progress manager during upload.
    */
    void setUploadProgressMessage( const QString &v )
    {
      mUploadProgressMessage = v;
    }
    /**
      Get upload progress message. See setUploadProgressMessage().
    */
    QString uploadProgressMessage() const
    {
      return mUploadProgressMessage;
    }

    /**
      Set user name.
    */
    void setUser( const QString &v )
    {
      mUser = v;
    }
    /**
      Get user. See setUser().
    */
    QString user() const
    {
      return mUser;
    }

    /**
      Set password of user.
    */
    void setPassword( const QString &v )
    {
      mPassword = v;
    }
    /**
      Get password. See setPassword().
    */
    QString password() const
    {
      return mPassword;
    }

    /**
      Set id mapper.
    */
    void setIdMapper( KPIM::IdMapper *v )
    {
      mIdMapper = v;
    }
    /**
      Get idMapper. See setIdMapper().
    */
    KPIM::IdMapper *idMapper() const
    {
      return mIdMapper;
    }

    void setUserPassword( KURL &url );

    /** Adapt the url for downloading. Sets the username and password and calls 
        customAdaptDownloadUrl, which you can reimplement for custom adaptions. */
    virtual void adaptDownloadUrl( KURL &url ) { setUserPassword( url ); customAdaptDownloadUrl( url );}
    /** Adapt the url for uploading. Sets the username and password and calls 
        customAdaptUploadUrl, which you can reimplement for custom adaptions. */
    virtual void adaptUploadUrl( KURL &url ) { setUserPassword( url ); customAdaptUploadUrl( url );}
    /** Apply custom adaptions to the url for downloading. Reimplement this method if you want
        to use webdav:// instead of http:// URLs. */
    virtual void customAdaptDownloadUrl( KURL &/*url*/ ) {}
    /** Apply custom adaptions to the url for uploading. Reimplement this method if you want
        to use webdav:// instead of http:// URLs. */
    virtual void customAdaptUploadUrl( KURL &/*url*/ ) {}
    /** Return the mime-type expected by the resource. */
    virtual QString mimeType() const = 0;
    /** Identifier of the Resource. Used for the custom fields where
        resource-specific information is stored. */
    virtual QCString identifier() const = 0;
    /** Returns whether the item with the given localId exists locally. */
    virtual bool localItemExists( const QString &localId ) = 0;
    /** Returns whether the item was changed locally since the last download
        from the server. */
    virtual bool localItemHasChanged( const QString &localId ) = 0;
    /** Remove the item from the local cache. */
    virtual void deleteItem( const QString &localId ) = 0;
    /** Returns the ginerprint of the item on the server */
    virtual QString extractFingerprint( KIO::TransferJob *job,
           const QString &rawText ) = 0;
    /** Adds the downloaded item to the local cache (job was created by createDownloadItemJob). */
    virtual QString addItem( KIO::TransferJob *job,
           const QString &rawText, QString &fingerprint,
           const QString &localId, const QString &storageLocation ) = 0;
    /** Returns the uid of the item encoded in data. */
    virtual QString extractUid( KIO::TransferJob *job, const QString &data ) = 0;
    /** Removed the changed flag for the item with the given uid. */
    virtual void clearChange( const QString &uid ) = 0;
    /** Creates the KIO::TransferJob for listing all items in the given url. */
    virtual KIO::TransferJob *createListItemsJob( const KURL &url ) = 0;
    /** Creates the KIO::TransferJob for downloading one given item. */
    virtual KIO::TransferJob *createDownloadItemJob( const KURL &url, GroupwareJob::ContentType ctype ) = 0;
    /** Extract the list of items on the server and the list of items to be
        downloaded from the results of the job (the job was created by
        createListitemsJob). */
    virtual bool itemsForDownloadFromList( KIO::Job *job,
           QStringList &currentlyOnServer,
           QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload ) = 0;

    /** Create the job to remove the deletedItems from the server. The base
        URL of the server is passedas uploadurl. */
    virtual KIO::Job *createRemoveItemsJob( const KURL &uploadurl,
           KPIM::GroupwareUploadItem::List deletedItems ) = 0;
    /** Create the job to change the item on the server (at the given URL) */
    virtual KIO::TransferJob *createUploadJob( const KURL &url, GroupwareUploadItem *item );
    /** Create the job to add the item to the server (at the given baseURL) */
    virtual KIO::TransferJob *createUploadNewJob( const KURL &url, GroupwareUploadItem *item );
    virtual void uploadFinished( KIO::TransferJob *trfjob, GroupwareUploadItem *item ) = 0;

    virtual void processDownloadListItem( QStringList &currentlyOnServer,
        QMap<QString,KPIM::GroupwareJob::ContentType> &itemsForDownload,
        const QString &entry, const QString &newFingerprint,
        KPIM::GroupwareJob::ContentType type );
    /** Return the default file name for a new item. */
    virtual QString defaultNewItemName( GroupwareUploadItem */*item*/ ) { return QString::null; }

  private:
    FolderLister *mFolderLister;
    QString mDownloadProgressMessage;
    QString mUploadProgressMessage;
    QString mUser;
    QString mPassword;
    KPIM::IdMapper *mIdMapper;
};

}

#endif
