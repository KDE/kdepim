/***************************************************************************
*   Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
***************************************************************************/
#ifndef FILEAPI_H
#define FILEAPI_H

#include <bloginterface.h>

namespace KBlog {

/**
Implementation for FileAPI

@author Reinhold Kainhofer
*/

class fileAPI : public blogInterface
{
    Q_OBJECT
  public:
    fileAPI( const KURL &server, QObject *parent = 0L, const char *name = 0L );
    ~fileAPI();
    QString interfaceName() { return "File API"; }

  public slots:
    void initServer();
    void getBlogs();
    void post( const BlogPosting& post, bool publish = false );
    void editPost( const BlogPosting& post, bool publish = false );
    void fetchPosts( const QString &blogID, int maxPosts );
    void fetchPost( const QString &postID );
    // void fetchTemplates();
    void deletePost( const QString &postID );

  private slots:
/*    void userInfoFinished( const QValueList<QVariant> & );
    void listFinished( const QValueList<QVariant> & );
    void blogListFinished( const QValueList<QVariant> & );
    void deleteFinished( const QValueList<QVariant> & );
    void getFinished( const QValueList<QVariant> & );
    void postFinished( const QValueList<QVariant> & );
    void fault( int, const QString& );
*/

  private:
    void dumpBlog( const BlogPosting &blog );
    bool isValid;
    
/*    KIO::Job *mDownloadJob;
    KIO::Job *mUploadJob;
    QString mTempFile;*/
    
    BlogListItem mBlogInfo;
    QValueList<BlogPosting> mBlogList;
};

};
#endif
