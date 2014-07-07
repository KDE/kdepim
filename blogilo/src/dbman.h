/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#ifndef DBMAN_H
#define DBMAN_H
#include "bilbomedia.h"
#include "constants.h"
#include "category.h"
class BilboBlog;
class BilboPost;
namespace KWallet
{
    class Wallet;
}

/**
DataBase Manager class. this class implement Low level Database operations. and any object of App.
have to use this API to store or retrive information and settings from Database.

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/

class DBMan
{
public:
    DBMan();

    ~DBMan();
    const QMap<int, BilboBlog*> & blogList() const;

    QString lastErrorText() const;
    /**
     * \brief Retrieve the instance of DataBase Manager.
     *
     * The Database manager (DBMan) is a singleton class of which only a single
     * instance will exist. If no manager exists yet this function will
     * create one for you.
     *
     * \return the instance of the DBMan
      */
    static DBMan* self();

    ///(BEGIN) Data retrieveing Functions:

    /**
     *    return List of blogs in Database.
     * @return
     */
    QMap<QString, int> listBlogsTitle();///QString as Title, and int as blog_id
    BilboBlog *blog(int blog_id);

    QList<BilboPost*> listPosts( int blog_id );
    QMap< int, QString > listPostsTitle( int blog_id );///QString as Title, and int as post_id
    QList<QVariantMap> listPostsInfo( int blog_id );
    BilboPost getPostInfo( int post_id );

    QMap<QString, int> listCategoriesName( int blog_id );
    QList<Category> listCategories( int blog_id );
    QMap<QString, bool> listCategoriesId( int blog_id );

    /**
    Returns list of temporary posts, e.g. posts saved intervally or at application quit.
    Map value (e.g. int) is blog id.
    */
    QMap<BilboPost*, int> listTempPosts();
    /**
    Returns list of locally saved posts.
    Map value (e.g. int) is blog id.
    */
    QList<QVariantMap> listLocalPosts();
    BilboPost localPost(int local_id);
    ///END

    ///(BEGIN) Data Manipulation Functions:

    ///Blog:

    int addBlog( const BilboBlog& blog );

    bool editBlog( const BilboBlog& blog );

    bool removeBlog( int blog_id );

    ///Post:

    /**
     *
     * @param post
     * @param blog_id
     * @return return post id in database (deffer with postid)
     */
    int addPost( const BilboPost& post, int blog_id );

    bool editPost( const BilboPost& post, int blog_id );

    bool removePost( int id );
    bool removePost(int blog_id, const QString &postId);

    bool clearPosts( int blog_id );

    ///Category:
    int addCategory( const QString &name, const QString &description, const QString &htmlUrl,
                     const QString &rssUrl, const QString &categoryId, const QString &parentId, int blog_id );
    bool clearCategories( int blog_id );

    ///File:
    int addFile(const QString &name, int blog_id, bool isUploaded, const QString &localUrl, const QString &remoteUrl );
    int addFile( const BilboMedia& file );
    int addFile();
    bool removeFile( int fileid );
    bool clearFiles( int blog_id );

    int saveLocalEntry( const BilboPost& post, int blog_id );
    int saveTempEntry( const BilboPost& post, int blog_id );
    bool removeLocalEntry( const BilboPost &post );
    bool removeLocalEntry( int local_id );
    bool removeTempEntry( const BilboPost &post );
    bool clearTempEntries();

    ///Auth Data:
    QMap<QString,QString> getAuthData( int blog_id );
    bool saveAuthData( const QMap<QString, QString>& authData, int blog_id );
    bool clearAuthData( int blog_id );

    ///END

private:
    enum LocalPostState {Local=0, Temp=1};
    int saveTemp_LocalEntry( const BilboPost& post, int blog_id, LocalPostState state );
    QList<BilboBlog*> listBlogs();
    bool createDB();
    bool connectDB();
    bool updateDB();
    void reloadBlogList();

    static DBMan* mSelf;

    class Private;
    Private * const d;
};

#endif
