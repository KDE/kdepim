/**************************************************************************
*   Copyright (C) 2003 by ian reinhart geiser <geiseri@kde.org>           *
*   Copyright (C) 2004 by Reinhold Kainhofer <reinhold@kainhofer.com>     *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef BLOGGERAPI_H
#define BLOGGERAPI_H

#include <bloginterface.h>

/**
Implementation for BloggerAPI

@author ian reinhart geiser, Reinhold Kainhofer
*/
namespace KXMLRPC
{
  class Server;
};

namespace KBlog {

class BloggerPostingWrapper;

class bloggerAPI : public blogInterface
{
    Q_OBJECT
  public:
    bloggerAPI( const KURL &server, QObject *parent = 0L, const char *name = 0L );
    ~bloggerAPI();
    QString interfaceName() const { return "Blogger API 1.0"; }
    QString defaultBlogID() const { return mDefaultBlogID; }

  public slots:
    void initServer();
    void getBlogs();
    void post( BlogPosting *post, bool publish = false );
    void editPost( BlogPosting *post, bool publish = false );
    void fetchPosts( const QString &blogID, int maxPosts );
    void fetchPost( const QString &postID );
    // void fetchTemplates();
    void deletePost( BlogPosting *posting );
    
    void setDefaultBlogID( const QString &blogID );

  private slots:
    void userInfoFinished( const QValueList<QVariant> & );
    void listFinished( const QValueList<QVariant> & );
    void blogListFinished( const QValueList<QVariant> & );
//     void deleteFinished( const QValueList<QVariant> & );
    void getFinished( const QValueList<QVariant> & );
//     void postFinished( const QValueList<QVariant> & );
    void fault( int, const QString& );

  protected:
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

    QValueList<QVariant> defaultArgs( const QString &id = QString::null );
    void warningNotInitialized();
    
    static QString escapeContent( const QString &content );
    QString formatContents( BlogPosting *blog );
    bool readPostingFromMap( BlogPosting *post, 
        const QMap<QString, QVariant> &postInfo );
    bool readBlogInfoFromMap( KBlog::BlogListItem &blog, 
        const QMap<QString, QVariant> &postInfo );

    BloggerPostingWrapper *createWrapper( BlogPosting *posting );
    virtual QString getFunctionName( blogFunctions type );
    
  private:
    KXMLRPC::Server *mXMLRPCServer;
    bool isValid;
    QString mDefaultBlogID;
};

};
#endif
