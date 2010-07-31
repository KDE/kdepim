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

#include <kurl.h>
#include <kio/job.h>
#include <libkcal/journal.h>

#include <tqobject.h>
#include <tqstring.h>
#include <tqvaluelist.h>
#include <tqdatetime.h>

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

  TQString userID() const { return mUserID; }
  void setUserID( const TQString &userID ) { mUserID = userID; }

  TQString blogID() const { return mBlogID; }
  void setBlogID( const TQString &blogID ) { mBlogID = blogID; }

  TQString postID() const { return mPostID; }
  void setPostID( const TQString &postID ) { assignPostID( postID ); mPostID = postID; }

  TQString title() const { return mTitle; }
  void setTitle( const TQString &title ) { mTitle = title; }

  TQString content() const { return mContent; }
  void setContent( const TQString &content ) { mContent = content; }

  TQString category() const { return mCategory; }
  void setCategory( const TQString &category ) { mCategory = category; }

  TQString fingerprint() const { return mFingerprint; }
  void setFingerprint( const TQString &fp ) { mFingerprint = fp; }

  TQDateTime dateTime() const { return mDateTime; }
  void setDateTime( const TQDateTime &datetime ) { mDateTime = datetime; }

  TQDateTime creationDateTime() const { return mCreationDateTime; }
  void setCreationDateTime( const TQDateTime &datetime ) { mCreationDateTime = datetime; }

  TQDateTime modificationDateTime() const { return mModificationDateTime; }
  void setModificationDateTime( const TQDateTime &datetime ) { mModificationDateTime = datetime; }

  virtual void wasDeleted( bool ) {}
  virtual void wasUploaded( bool ) {}
  virtual void error( int /*code*/, const TQString &/*error*/ ) {}

protected:
  // Override this method to detect the new postID assigned when adding a new post
  virtual void assignPostID( const TQString &/*postID*/ ) {}
  TQString mUserID;
  TQString mBlogID;
  TQString mPostID;
  TQString mTitle;
  TQString mContent;
  TQString mCategory;
  TQString mFingerprint;
  TQDateTime mDateTime;
  TQDateTime mCreationDateTime;
  TQDateTime mModificationDateTime;
};


class APIBlog : public QObject
{
    Q_OBJECT
  public:
    APIBlog( const KURL &server, TQObject *parent = 0L, const char *name = 0L );
    virtual ~APIBlog();
    virtual TQString interfaceName() const = 0;

    void setAppID( const TQString &appID ) { mAppID = appID; }
    TQString appID() const { return mAppID; }

    void setPassword( const TQString &pass ) { mPassword = pass; }
    TQString password() const { return mPassword; }

    void setUsername( const TQString &uname ) { mUsername = uname; }
    TQString username() const { return mUsername; }

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

    virtual TQString getFunctionName( blogFunctions type ) = 0;
    virtual TQValueList<TQVariant> defaultArgs( const TQString &id = TQString::null );

    virtual KIO::Job *createUserInfoJob() = 0;
    virtual KIO::Job *createListFoldersJob() = 0;
    virtual KIO::TransferJob *createListItemsJob( const KURL &url ) = 0;
    virtual KIO::TransferJob *createDownloadJob( const KURL &url ) = 0;
    virtual KIO::TransferJob *createUploadJob( const KURL &url, KBlog::BlogPosting *posting ) = 0;
    virtual KIO::TransferJob *createUploadNewJob( KBlog::BlogPosting *posting ) = 0;
    virtual KIO::Job *createRemoveJob( const KURL &url, const TQString &postid ) = 0;

    virtual bool interpretUserInfoJob( KIO::Job *job ) = 0;
    virtual void interpretListFoldersJob( KIO::Job *job ) = 0;
    virtual bool interpretListItemsJob( KIO::Job *job ) = 0;
    virtual bool interpretDownloadItemsJob( KIO::Job *job ) = 0;
    
    static KCal::Journal *journalFromPosting( KBlog::BlogPosting *post );
    static KBlog::BlogPosting *postingFromJournal( KCal::Journal *journal );

  signals:
    // TODO: Connect these
    void userInfoRetrieved( const TQString &nickname, const TQString &userid, const TQString &email );
    void folderInfoRetrieved( const TQString &id, const TQString &name );

    void itemOnServer( const KURL &remoteURL );
    void itemDownloaded( KCal::Incidence *j, const TQString &localID,
                         const KURL &remoteURL, const TQString &fingerprint,
                         const TQString &storageLocation );
    

  protected:

    KURL mServerURL;
    TQString mPassword;
    TQString mUsername;
    TQString mAppID;
    int mDownloadCount;
};

}
#endif
