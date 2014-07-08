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

#ifndef BACKEND_H
#define BACKEND_H

#include "category.h"
#include "bilbopost.h"

#include <kblog/blog.h>

#include <QObject>
#include <QMap>


class BilboMedia;
/**
Engine of application.
this is heart and brain of app. ;)

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class Backend : public QObject
{
    Q_OBJECT
public:
    explicit Backend( int blog_id, QObject* parent = 0 );

    ~Backend();

    /**
     * Request to Fetch categories list from server.
     * and after it's fetched, categoriesListed() SLOT will insert them to database, and update list in database
     * and emit categoriesFetched() SIGNAL.
     * @param blog_id Id of blog in DB!
     */
    void getCategoryListFromServer();

    /**
     *    retrieve latest posts from server
     * @param count number of post to retrieve.
     */
    void getEntriesListFromServer( int count );

    /**
     *    Use this to publish a post to server.
     * @param post Post to publish.
     */
    void publishPost( BilboPost *post );

    /**
     * Upload a new Media object e.g. image to server.
     * @param  media Media Object to upload.
     */
    void uploadMedia( BilboMedia *media );

    /**
     * Modify an existing post.
     * Note: posiId must sets correctly.
     * @param post post to modify.
     */
    void modifyPost( BilboPost* post );

    /**
     * Remove an existing post from server
     * @param post post to remove.
     */
    void removePost( BilboPost *post );

//     void setPostCategories( const QString postId, const QMap<QString, bool> &categoriesList );

    /**
    * Fetch a blog post from the server with a specific ID.
    * The ID of the existing post must be retrieved using getRecentPosts
    * and then be modified and provided to this method or a new BlogPost
    * created with the existing ID.
    * @param post a blog post with the ID identifying the blog post to fetch.
    * @see sigPostFetched()
    */
    void fetchPost( BilboPost *post );

protected Q_SLOTS:
    void categoriesListed( const QList< QMap< QString, QString > > &   categories );
    void entriesListed( const QList< KBlog::BlogPost > &posts );
    void postPublished( KBlog::BlogPost *post );
    void mediaUploaded( KBlog::BlogMedia *media );
    void error( KBlog::Blog::ErrorType type, const QString &errorMessage );
    void slotMediaError( KBlog::Blog::ErrorType type, const QString &errorMessage, KBlog::BlogMedia *media );
//     void postCategoriesSetted( const QString &postId );
    void slotPostRemoved( KBlog::BlogPost *post );
    void slotPostFetched( KBlog::BlogPost *post );
    /**
     * This function is called after a post published fine, to insert it to DB and emit sigPostPublished.
     */
    void savePostInDbAndEmitResult( KBlog::BlogPost *post );

    void bloggerAuthenticated( const QMap<QString,QString>& authData );

Q_SIGNALS:
    /**
     * emit when a categoriesListed() Done and Categories added to DB
     * @param blog_id id of Blog owner of categories.
     */
    void sigCategoryListFetched( int blog_id );

    /**
     * emit when a entriesListed() Done and Entries added to DB
     * @param blog_id id of Blog owner of Entries.
     */
    void sigEntriesListFetched( int blog_id );

    /**
     * This signal is emitted when a post published/modified and added/edited to Database.
     * @param blog_id blog id.
     * @param post The post has been saved.
     */
    void sigPostPublished( int blog_id, BilboPost *post );

    /**
     * This signal is emitted when a media has been uploaded to the server.
     * @param media Uploaded media Object.
     */
    void sigMediaUploaded( BilboMedia *media );

    /**
     * this signal is emitted when a post removed successfully.
     */
    void sigPostRemoved( int blog_id, const BilboPost &post);

    void sigPostFetched( BilboPost *post );

    /**
     * this signal emitted when an error occurred on current transaction.
     * @param type type of error
     * @param errorMessage error message.
     */
    void sigError( const QString &errorMessage );

    void sigMediaError( const QString &errorMessage, BilboMedia* media );

private:
    KBlog::BlogPost* preparePost( KBlog::BlogPost* post );
    QString errorTypeToString( KBlog::Blog::ErrorType type );

    class Private;
    Private * const d;
};

#endif
