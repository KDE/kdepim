/**************************************************************************
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef API_BLOG_H
#define API_BLOG_H

// #include <groupwareresourcejob.h>
// #include <folderlister.h>

#include <kurl.h>
#include <kio/job.h>
#include <libkcal/journal.h>

#include <qobject.h>
#include <qstring.h>
#include <qvaluelist.h>
#include <qdatetime.h>

/**
This is the main interface for blog backends
@author ian reinhart geiser, Reinhold Kainhofer
*/

namespace KBlog {

class BlogPosting
{
public:
  BlogPosting() {}
  virtual ~BlogPosting() {}

  QString userID() const { return mUserID; }
  void setUserID( const QString &userID ) { mUserID = userID; }

  QString blogID() const { return mBlogID; }
  void setBlogID( const QString &blogID ) { mBlogID = blogID; }

  QString postID() const { return mPostID; }
  void setPostID( const QString &postID ) { assignPostID( postID ); mPostID = postID; }

  QString title() const { return mTitle; }
  void setTitle( const QString &title ) { mTitle = title; }

  QString content() const { return mContent; }
  void setContent( const QString &content ) { mContent = content; }

  QString category() const { return mCategory; }
  void setCategory( const QString &category ) { mCategory = category; }

  QString fingerprint() const { return mFingerprint; }
  void setFingerprint( const QString &fp ) { mFingerprint = fp; }

  QDateTime dateTime() const { return mDateTime; }
  void setDateTime( const QDateTime &datetime ) { mDateTime = datetime; }

  QDateTime creationDateTime() const { return mCreationDateTime; }
  void setCreationDateTime( const QDateTime &datetime ) { mCreationDateTime = datetime; }

  QDateTime modificationDateTime() const { return mModificationDateTime; }
  void setModificationDateTime( const QDateTime &datetime ) { mModificationDateTime = datetime; }

  virtual void wasDeleted( bool ) {}
  virtual void wasUploaded( bool ) {}
  virtual void error( int /*code*/, const QString &/*error*/ ) {}

protected:
  // Override this method to detect the new postID assigned when adding a new post
  virtual void assignPostID( const QString &/*postID*/ ) {}
  QString mUserID;
  QString mBlogID;
  QString mPostID;
  QString mTitle;
  QString mContent;
  QString mCategory;
  QString mFingerprint;
  QDateTime mDateTime;
  QDateTime mCreationDateTime;
  QDateTime mModificationDateTime;
};


class APIBlog : public QObject
{
    Q_OBJECT
  public:
    APIBlog( const KURL &server, QObject *parent = 0L, const char *name = 0L );
    virtual ~APIBlog();
    virtual QString interfaceName() const = 0;

    void setAppID( const QString &appID ) { mAppID = appID; }
    QString appID() const { return mAppID; }

    void setPassword( const QString &pass ) { mPassword = pass; }
    QString password() const { return mPassword; }

    void setUsername( const QString &uname ) { mUsername = uname; }
    QString username() const { return mUsername; }

    void setURL( const KURL& url ) { mServerURL = url; }
    KURL url() const { return mServerURL; }

    void setDownloadCount( int nr ) { mDownloadCount = nr; }
    int downloadCount() const { return mDownloadCount; }

    static void dumpBlog( BlogPosting *blog );


    enum blogFunctions {
      bloggerGetUserInfo,
      bloggerGetUsersBlogs,
      bloggerGetRecentPosts,
      bloggerNewPost,
      bloggerEditPost,
      bloggerDeletePost,
      bloggerGetPost,
      bloggerGetTemplate,
      bloggerSetTemplate
    };

    virtual QString getFunctionName( blogFunctions type ) = 0;
    virtual QValueList<QVariant> defaultArgs( const QString &id = QString::null );

    virtual KIO::Job *createUserInfoJob() = 0;
    virtual KIO::Job *createListFoldersJob() = 0;
    virtual KIO::TransferJob *createListItemsJob( const KURL &url ) = 0;
    virtual KIO::TransferJob *createDownloadJob( const KURL &url ) = 0;
//     virtual KIO::Job *createRemoveJob( const KURL &url, KPIM::GroupwareUploadItem::List deletedItems ) = 0;

    virtual bool interpretUserInfoJob( KIO::Job *job ) = 0;
    virtual void interpretListFoldersJob( KIO::Job *job ) = 0;
    virtual bool interpretListItemsJob( KIO::Job *job ) = 0;
    virtual bool interpretDownloadItemsJob( KIO::Job *job ) = 0;

  signals:
    // TODO: Connect these
    void userInfoRetrieved( const QString &nickname, const QString &userid, const QString &email );
    void folderInfoRetrieved( const QString &id, const QString &name );

    void itemOnServer( const QString &remoteURL );
    void itemDownloaded( KCal::Incidence *j, const QString &localID,
                         const QString &remoteURL, const QString &fingerprint,
                         const QString &storageLocation );
    

  protected:
    KCal::Journal *journalFromPosting( KBlog::BlogPosting *post );
    KBlog::BlogPosting *postingFromJournal( KCal::Journal *journal );

    KURL mServerURL;
    QString mPassword;
    QString mUsername;
    QString mAppID;
    int mDownloadCount;
};

}
#endif
