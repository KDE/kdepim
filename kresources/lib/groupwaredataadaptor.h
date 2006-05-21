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
#ifndef KPIM_GROUPWAREDATAADAPTOR_H
#define KPIM_GROUPWAREDATAADAPTOR_H

#include <groupwareresourcejob.h>
#include <folderlister.h>
#include <kurl.h>
#include <qstring.h>
#include <libkcal/listbase.h>
#include <qobject.h>
#include <kdepimmacros.h>


namespace KIO {
class TransferJob;
class Job;
}

namespace KPIM {

class FolderLister;
class IdMapper;
class GroupwareDataAdaptor;
//class GroupwareJob;

class KDE_EXPORT GroupwareUploadItem
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

    QString fingerprint() const { return mFingerprint; }
    void setFingerprint( const QString &fpr ) { mFingerprint = fpr; }

    virtual QString data() const { return mData; }
    virtual void setData( const QString &data ) { mData = data; }
    virtual KURL adaptNewItemUrl( GroupwareDataAdaptor *adaptor, 
                                  const KURL &url );

    virtual KIO::TransferJob *createRawUploadJob( 
                               GroupwareDataAdaptor *adaptor, const KURL &url );
    virtual KIO::TransferJob *createUploadNewJob( 
                               GroupwareDataAdaptor *adaptor, const KURL &url );
    virtual KIO::TransferJob *createUploadJob( GroupwareDataAdaptor *adaptor, 
                                               const KURL &url );
    virtual KPIM::FolderLister::ContentType itemType() { return mItemType; }
  protected:
    KPIM::FolderLister::ContentType mItemType;
  private:
    KURL mUrl;
    QString mUid;
    QString mData;
    QString mFingerprint;
    UploadType mType;
};


class KDE_EXPORT GroupwareDataAdaptor : public QObject
{
Q_OBJECT
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
      Set progress message shown by progress manager during download. It will
      be interpreted as rich text, so it might have to be escaped.
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
      Set progress message shown by progress manager during upload. It will
      be interpreted as rich text, so it might have to be escaped.
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
      Set base URL.
    */
    void setBaseURL( const KURL &url )
    {
      mBaseURL = url;
    }
    /**
      Get base URL. See setBaseURL().
    */
    KURL baseURL() const
    {
      return mBaseURL;
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
     *  customAdaptDownloadUrl, which you can reimplement for custom adaptions. 
     */
    virtual void adaptDownloadUrl( KURL &url ) 
    {
      setUserPassword( url ); 
      customAdaptDownloadUrl( url );
    }
    /** Adapt the url for uploading. Sets the username and password and calls
     *  customAdaptUploadUrl, which you can reimplement for custom adaptions. */
    virtual void adaptUploadUrl( KURL &url ) 
    {
      setUserPassword( url ); 
      customAdaptUploadUrl( url );
    }
    /** Apply custom adaptions to the url for downloading. Reimplement this 
     *  method if you want to use webdav:// instead of http:// URLs. */
    virtual void customAdaptDownloadUrl( KURL &/*url*/ ) {}
    /** Apply custom adaptions to the url for uploading. Reimplement this 
     *  method if you want to use webdav:// instead of http:// URLs. */
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

    /** Removed the changed flag for the item with the given uid. */
    virtual void clearChange( const QString &uid ) = 0;

    virtual FolderLister::Entry::List defaultFolders();
    virtual QValueList<FolderLister::ContentType> supportedTypes() = 0;
    virtual bool supports( FolderLister::ContentType type ) {
      return supportedTypes().contains( type );
    }


    // Creating jobs
    /** Creates the KIO::Job for logging in to the server. This is only
     *  called if the GroupwareDataAdaptor::GWResNeedsLogin flag is set
     *  for the resource. */
    virtual KIO::Job *createLoginJob( const KURL &, const QString &/*user*/,
                                      const QString &/*password*/ ) { return 0; }
    /** Creates the KIO::Job for logging off the server. This is only
     *  called if the GroupwareDataAdaptor::GWResNeedsLogoff flag is set
     *  for the resource. */
    virtual KIO::Job *createLogoffJob( const KURL &, const QString &/*user*/,
                                      const QString &/*password*/ ) { return 0; }
    /** Creates the KIO::Job for listing all subfolders of the given url. */
    virtual KIO::Job *createListFoldersJob ( const KURL & ) = 0;
    /** Creates the KIO::TransferJob for listing all items in the given url. */
    virtual KIO::TransferJob *createListItemsJob( const KURL & ) = 0;
    /** Creates the KIO::TransferJob for downloading one given item. */
    virtual KIO::TransferJob *createDownloadJob( const KURL &,
                                          FolderLister::ContentType ) = 0;
    /** Creates the KIO::TransferJob for downloading a list of items items. */
    virtual KIO::TransferJob *createDownloadJob( const QMap<KURL,FolderLister::ContentType> & ) { return 0; }
    /** Create the job to remove the deletedItems from the server. The base
        URL of the server is passed as uploadurl.  */
    virtual KIO::Job *createRemoveJob( const KURL &,
                             const KPIM::GroupwareUploadItem::List &/*deletedItems*/ ) { return 0; }
    /** Create the job to remove the item from the server. The base
        URL of the server is passed as uploadurl.  */
    virtual KIO::Job *createRemoveJob( const KURL &, 
                             KPIM::GroupwareUploadItem */*deletedItem*/ ) { return 0; }
    /** Create the job to change the item on the server (at the given URL) */
    virtual KIO::TransferJob *createUploadJob( const KURL &,
                                               GroupwareUploadItem *item );
    /** Create the job to change the items on the server (at the given URL) */
    virtual KIO::TransferJob *createUploadJob( const KURL &, const GroupwareUploadItem::List &/*items*/ ) { return 0; }
    /** Create the job to add the item to the server (at the given baseURL) */
    virtual KIO::TransferJob *createUploadNewJob( const KURL &,
                                                  GroupwareUploadItem *item );
    /** Create the job to add the items to the server (at the given baseURL) */
    virtual KIO::TransferJob *createUploadNewJob( const KURL &, const GroupwareUploadItem::List &/*items*/ ) { return 0; }


    // Interpreting the result of the jobs
    /** Extract the success information from the login job, created by
     *  createLoginJob. Return true, if the login was successfull, or false
     *  if the user could not be authenticated or something else went wrong.
     *  In that case the resource will not be marked as open. */
    virtual bool interpretLoginJobResult( KIO::Job * ) { return true; }
    /** Extract the success information from the logoff job, created by
     *  createLogoffJob. */
    virtual bool interpretLogoffJobResult( KIO::Job * ) { return true; }
    
    virtual void interpretListFoldersJob( KIO::Job *, FolderLister *) = 0;
    /** Extract the list of items on the server and the list of items to be
        downloaded from the results of the job (the job was created by
        createListitemsJob). */
    virtual bool interpretListItemsJob( KIO::Job *, const QString &jobData ) = 0;
    virtual bool interpretDownloadItemsJob( KIO::Job *job, const QString &jobData ) = 0;
    virtual bool interpretRemoveJob( KIO::Job *job, const QString &jobData );
    virtual bool interpretUploadJob( KIO::Job *job, const QString &/*jobData*/ );
    virtual bool interpretUploadNewJob( KIO::Job *job, const QString &/*jobData*/ );

    virtual void processDownloadListItem(  const KURL &entry,
        const QString &newFingerprint, KPIM::FolderLister::ContentType type );
    /** Return the default file name for a new item. */
    virtual QString defaultNewItemName( GroupwareUploadItem * )
    {
      return QString::null; 
    }
    virtual QString uidFromJob( KIO::Job *job ) const;
    virtual void setUidForJob( KIO::Job *job, const QString &uid );


    enum {
      GWResBatchCreate = 0x0001,
      GWResBatchModify = 0x0002,
      GWResBatchDelete = 0x0004,
      GWResBatchRequest = 0x0008,
      GWResHandleSingleIncidences = 0x0020,
      GWResNeedsLogon = 0x0040,
      GWResNeedsLogoff = 0x0080
    };
    virtual long flags() const = 0;


  signals:
    void folderInfoRetrieved( const KURL &href, const QString &name,
                                     KPIM::FolderLister::ContentType );
    void folderSubitemRetrieved( const KURL &, bool isFolder );

    void itemToDownload( const KURL &remoteURL, KPIM::FolderLister::ContentType type );
    /** passed the whole remote url (including hostname) to the receiver */
    void itemOnServer( const KURL &remoteURL );
    
    void itemDownloaded( const QString &localID, const KURL &remoteURL,
                         const QString &fingerprint );
    void itemDeleted( const QString &localID, const KURL &remoteURL );
    void itemUploaded( const QString &localID, const KURL &remoteURL );
    void itemUploadedNew( const QString &localID, const KURL &remoteURL );

    void itemDownloadError( const KURL &remoteURL, const QString &error );
    void itemDeletionError( const KURL &remoteURL, const QString &error );
    void itemUploadError( const KURL &remoteURL, const QString &error );
    void itemUploadNewError( const QString &localID, const QString &error );

  private:
    FolderLister *mFolderLister;
    QString mDownloadProgressMessage;
    QString mUploadProgressMessage;
    KURL mBaseURL;
    QString mUser;
    QString mPassword;
    KPIM::IdMapper *mIdMapper;
    QMap<KIO::Job*,QString> mJobUIDMap;
};

}

#endif
