/**************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser <geiseri@kde.org>           *
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef BLOGINTERFACE_H
#define BLOGINTERFACE_H

#include <qobject.h>
#include <qdatetime.h>
#include <qstring.h>
#include <kurl.h>
/**
This is the main interface for blog backends

@author ian reinhart geiser, Reinhold Kainhofer
*/

namespace KBlog {

enum blogAPITypes {
  blogAPIBlogger = 1,
  blogAPIBlogger2,
  blogAPIFile,
  blogAPIDrupal,
  blogAPIMetaWeblog,
  blogAPIMovableType,
  blogAPIUnknown
};

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
  QDateTime mDateTime;
  QDateTime mCreationDateTime;
  QDateTime mModificationDateTime;
};


class BlogListItem
{
public:
  BlogListItem() {}
  ~BlogListItem() {}  

  QString id() const { return mId; }
  void setId( const QString &id ) { mId = id; }
  
  QString name() const { return mName; }
  void setName( const QString &name ) { mName = name; }

  QString url() const { return mUrl; }
  void setUrl( const QString &url ) { mUrl = url; }

protected:
  QString mId;
  QString mName;
  QString mUrl;
};

class BlogTemplate
{
public:
  BlogTemplate() {}
  ~BlogTemplate() {}
  
  QString titleTagOpen() const { return mTitleTagOpen; }
  void setTitleTagOpen( const QString &titleTagOpen ) 
             { mTitleTagOpen = titleTagOpen; }

  QString titleTagClose() const { return mTitleTagClose; }
  void setTitleTagClose( const QString &titleTagClose ) 
             { mTitleTagClose = titleTagClose; }

  QString categoryTagOpen() const { return mCategoryTagOpen; }
  void setCategoryTagOpen( const QString &categoryTagOpen ) 
             { mCategoryTagOpen = categoryTagOpen; }
  
  QString categoryTagClose() const { return mCategoryTagClose; }
  void setCategoryTagClose( const QString &categoryTagClose ) 
             { mCategoryTagClose = categoryTagClose; }
  
protected:
  QString mTitleTagOpen;
  QString mTitleTagClose;
  QString mCategoryTagOpen;
  QString mCategoryTagClose;
};


class blogInterface : public QObject
{
    Q_OBJECT
  public:
    blogInterface( const KURL &server, QObject *parent = 0L, const char *name = 0L );
    virtual ~blogInterface();
    virtual QString interfaceName() const = 0;
    
    void setAppID( const QString &appID ) { mAppID = appID; }
    QString appID() const { return mAppID; }

    void setPassword( const QString &pass );
    QString password() const;

    void setUsername( const QString &uname );
    QString username() const;

    void setURL( const KURL& url );
    KURL url() const;

    void setTemplateTags( const BlogTemplate& Template );
    BlogTemplate templateTags() const;
    
    static void dumpBlog( BlogPosting *blog );


  public slots:
    virtual void initServer() = 0;
    virtual void getBlogs() = 0;
    virtual void post( BlogPosting *data, bool publish = false ) = 0;
    virtual void editPost( BlogPosting *data, bool publish = false ) = 0;
    virtual void fetchPosts( const QString &blogID, int maxPosts ) = 0;
    virtual void fetchPost( const QString &postID ) = 0;
    // void fetchTemplates() = 0;
    virtual void deletePost( const QString &postID );
    virtual void deletePost( BlogPosting *posting ) = 0;

  signals:
    void serverInfoSignal( const QString &/*nickname*/, 
                           const QString & /*m_userid*/,
                           const QString & /*email*/ );
    void blogListSignal( QValueList<KBlog::BlogListItem> /*blogs*/ );
    void recentPostsSignal( QStringList /*postIDs*/ );
    void recentPostsSignal( const QValueList<KBlog::BlogPosting*> &/*blogs*/ );
    //void post( const blogPost &post );
    void postFinishedSignal( bool /*success*/ );
    void publishFinishedSignal( bool /*success*/ );
    void editFinishedSignal( bool /*success*/ );
    void deleteFinishedSignal( bool /*success*/ );
    void newPostSignal( KBlog::BlogPosting */*post*/ );

    // Error message
    void error( const QString &/*faultMessage*/ );

  protected:
    KURL mServerURL;
    BlogTemplate mTemplate;
    QString mPassword;
    QString mUsername;
    QString mAppID;
};

};
#endif
