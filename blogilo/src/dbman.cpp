/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "dbman.h"
#include <kmessagebox.h>
#include "bilboblog.h"
#include "bilbopost.h"
#include <kdebug.h>
#include <KDE/KLocale>
#include <kdatetime.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kwallet.h>
#include <kio/deletejob.h>

DBMan::DBMan()
{
    kDebug();
    mWallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
    if ( mWallet ) {
        useWallet = true;
        if ( !mWallet->setFolder( "blogilo" ) ) {
            mWallet->createFolder( "blogilo" );
            mWallet->setFolder( "blogilo" );
        }
        kDebug() << "Wallet successfully opened.";
    } else {
        useWallet = false;
        kDebug() << "Could not use Wallet service, will use database to store passwords";
    }

    if ( !QFile::exists( CONF_DB ) ) {
        if ( !this->createDB() ) {
            KMessageBox::detailedError( 0, i18n( "Cannot create database" ),
                                        i18n( db.lastError().text().toUtf8().data() ) );
            kDebug() << "Cannot create database, SQL error: " << db.lastError().text() << endl;
            exit ( 1 );
        }
    } else if ( !connectDB() )
        exit( 1 );

    reloadBlogList();
}

QString DBMan::lastErrorText()
{
    return mLastErrorText;
}

DBMan * DBMan::mSelf = 0L;

DBMan * DBMan::self()
{
    if ( !mSelf )
        mSelf = new DBMan;
    return mSelf;
}

const QMap<int, BilboBlog*> & DBMan::blogList() const
{
    return mBlogList;
}

void DBMan::reloadBlogList()
{
    mBlogList.clear();
    QList<BilboBlog*> listBlogs = this->listBlogs();
    int count = listBlogs.count();
    for ( int i = 0; i < count; ++i ) {
        mBlogList [ listBlogs[i]->id() ] = listBlogs[i];
    }
}

bool DBMan::connectDB()
{
    kDebug();
    if( db.isOpen() )
        return true;
    db = QSqlDatabase::addDatabase( "QSQLITE" );
    db.setDatabaseName( CONF_DB );

    if ( !db.open() ) {
        KMessageBox::detailedError( 0, i18n( "Cannot connect to database" ),
                                    i18n( db.lastError().text().toUtf8().data() ) );
        kDebug() << "Cannot connect to database, SQL error: " << db.lastError().text();
        return false;
    }
    return true;
}

DBMan::~DBMan()
{
    kDebug();
    db.close();
    mSelf = 0L;
}

/**
Will create configuration database!

    Notes about database:
        * blog.style_url will use for BilboBlog::BlogUrl.
*/
bool DBMan::createDB()
{
    kDebug();
    bool ret = true;
    if ( !connectDB() )
        exit( 1 );

    QSqlQuery q;
    ///Blog table!
    if ( !q.exec( "CREATE TABLE blog (id INTEGER PRIMARY KEY, blogid TEXT, blog_url TEXT, username TEXT,\
                  password TEXT, style_url TEXT, api_type TEXT, title TEXT, direction TEXT,\
                  local_directory TEXT, icon_url TEXT)" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///posts table!
    if ( !q.exec( "CREATE TABLE post (id INTEGER PRIMARY KEY, postid TEXT NOT NULL, blog_id NUMERIC NOT NULL,\
                  author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
                  c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
                  is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
                  status NUMERIC, trackback_urls TEXT, UNIQUE(postid, blog_id));" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///comments table!
    if ( !q.exec( "CREATE TABLE comment (id INTEGER PRIMARY KEY, commentid TEXT NOT NULL, blog_id NUMERIC NOT NULL,\
        postId TEXT, author_name TEXT, author_url TEXT, author_email TEXT, title TEXT, content TEXT,\
        c_time TEXT, m_time TEXT, link TEXT, password TEXT,\
        status NUMERIC, UNIQUE(commentid, blog_id));" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///categories table!
    if ( !q.exec( "CREATE TABLE category (catid INTEGER PRIMARY KEY, name TEXT NOT NULL,\
                  description TEXT, htmlUrl TEXT, rssUrl TEXT, categoryId TEXT, parentId TEXT,\
                  blog_id NUMERIC NOT NULL, UNIQUE(name,blog_id));" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///files table
    if( !q.exec( "CREATE TABLE file (fileid INTEGER PRIMARY KEY, name TEXT, blog_id NUMERIC, is_uploaded NUMERIC,\
        local_url TEXT, remote_url TEXT, mime_type TEXT);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///connection bethween posts and categories
    if ( !q.exec( "CREATE TABLE post_cat (blogId TEXT NOT NULL, postId TEXT NOT NULL,\
        categoryId TEXT NOT NULL, UNIQUE(blogId,postId,categoryId));" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///connection bethween posts and media files
    if ( !q.exec( "CREATE TABLE post_file (post_id INTEGER, file_id INTEGER);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///local posts table
    if( !q.exec( "CREATE TABLE local_post (local_id INTEGER PRIMARY KEY, id INTEGER UNIQUE, postid TEXT, blog_id NUMERIC,\
        author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
        c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
        is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
        status NUMERIC);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///Connection between local_posts and categories
    if( !q.exec( "CREATE TABLE local_post_cat (local_id INT, categoryId TEXT);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///temporary posts table
    if( !q.exec( "CREATE TABLE temp_post (local_id INTEGER PRIMARY KEY, id INTEGER UNIQUE, postid TEXT, blog_id NUMERIC,\
                 author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
                 c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
                 is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
                 status NUMERIC);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///Connection between temp_posts and categories
    if( !q.exec( "CREATE TABLE temp_post_cat (local_id INT, categoryId TEXT);" ) ) {
        ret = false;
        mLastErrorText = q.lastError().text();
    }

    ///delete related informations on DB, On removing a post or a blog
    q.exec( "CREATE TRIGGER delete_post AFTER DELETE ON post\
    BEGIN\
    DELETE FROM post_cat WHERE post_cat.postId=OLD.postid;\
    DELETE FROM post_file WHERE post_file.post_id=OLD.id;\
    DELETE FROM comment WHERE comment.postId=OLD.postid;\
    END" );
    q.exec( "CREATE TRIGGER delete_blog AFTER DELETE ON blog \
    BEGIN\
    DELETE FROM category WHERE category.blog_id=OLD.id;\
    DELETE FROM file WHERE file.blog_id=OLD.id;\
    DELETE FROM post WHERE post.blog_id=OLD.id;\
    DELETE FROM comment WHERE comment.blog_id=OLD.id;\
    END" );
    q.exec( "CREATE TRIGGER delete_temp_post AFTER DELETE ON temp_post \
    BEGIN\
    DELETE FROM temp_post_cat WHERE local_id=OLD.local_id;\
    END" );
    q.exec( "CREATE TRIGGER delete_local_post AFTER DELETE ON local_post \
    BEGIN\
    DELETE FROM local_post_cat WHERE local_id=OLD.local_id;\
    END" );

    return ret;
}

int DBMan::addBlog( const BilboBlog & blog )
{
    QSqlQuery q;
    if( useWallet ) {
        q.prepare( "INSERT INTO blog (blogid, blog_url, username, style_url, api_type, title,\
               direction, local_directory) VALUES(?, ?, ?, ?, ?, ?, ?, ?)" );
        if ( mWallet && mWallet->writePassword( blog.url().url() + '_' + blog.username(), blog.password() ) == 0 )
            kDebug() << "Password stored to kde wallet";
        else
            return -1;
    } else {
        q.prepare( "INSERT INTO blog (password, blogid, blog_url, username, style_url, api_type, title,\
               direction, local_directory) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)" );
        q.addBindValue( blog.password() );
    }
    q.addBindValue( blog.blogid() );
    q.addBindValue( blog.url().url() );
    q.addBindValue( blog.username() );
    q.addBindValue( blog.blogUrl() );
    q.addBindValue( blog.api() );
    q.addBindValue( blog.title() );
    q.addBindValue( blog.direction() );
    q.addBindValue( blog.localDirectory() );

    if ( q.exec() ) {
        reloadBlogList();
        return q.lastInsertId().toInt();
    } else {
        mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::editBlog( const BilboBlog & blog )
{
    QSqlQuery q;
    if( useWallet ) {
        q.prepare( "UPDATE blog SET blogid=?, blog_url=?, username=? , style_url=? , api_type=?, \
                   title=?, direction=?, local_directory=? WHERE id=?" );
        if ( mWallet && mWallet->writePassword( blog.url().url() + '_' + blog.username(), blog.password() ) == 0 )
            kDebug() << "Password stored to kde wallet";
        else
            return false;
    } else {
        q.prepare( "UPDATE blog SET password=?, blogid=?, blog_url=?, username=? , style_url=? , api_type=?, \
                    title=?, direction=?, local_directory=? WHERE id=?" );
        q.addBindValue( blog.password() );
    }
    q.addBindValue( blog.blogid() );
    q.addBindValue( blog.url().url() );
    q.addBindValue( blog.username() );
    q.addBindValue( blog.blogUrl() );
    q.addBindValue( blog.api() );
    q.addBindValue( blog.title() );
    q.addBindValue( blog.direction() );
    q.addBindValue( blog.localDirectory() );
    q.addBindValue( blog.id() );

    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
        return res;
    }
    reloadBlogList();
    return res;
}

bool DBMan::removeBlog( int blog_id )
{
    BilboBlog *tmp = mBlogList[ blog_id ];
    if( useWallet ) {
        if ( mWallet && mWallet->removeEntry( tmp->url().url() + '_' + tmp->username() ) == 0 )
            kDebug() << "Password removed to kde wallet";
    }
    QSqlQuery q;
    q.prepare( "DELETE FROM blog WHERE id=?" );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
        return res;
    }
    QString path = KStandardDirs::locateLocal( "data", QString( "blogilo/%1/" ).arg( blog_id ) , false );
    KIO::del(KUrl(path), KIO::HideProgressInfo);
    reloadBlogList();
    return res;
}

int DBMan::addPost( const BilboPost & post, int blog_id )
{
    kDebug() << "Adding post with title: " << post.title() << " to Blog " << blog_id;
    QSqlQuery q;
    q.prepare( "INSERT OR REPLACE INTO post (postid, blog_id, author, title, content, text_more, c_time, m_time,\
               is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, slug,\
               tags, status) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" );
    q.addBindValue( post.postId() );
    q.addBindValue( blog_id );
    q.addBindValue( post.author() );
    q.addBindValue( post.title() );
    q.addBindValue( post.content() );
    q.addBindValue( post.additionalContent() );
    q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
    q.addBindValue( ( post.modificationDateTime().isNull() ? post.creationDateTime().toString( KDateTime::ISODate ) :
                      post.modificationDateTime().toString( KDateTime::ISODate )) );
    q.addBindValue( post.isPrivate() );
    q.addBindValue( post.isCommentAllowed() );
    q.addBindValue( post.isTrackBackAllowed() );
    q.addBindValue( post.link().url() );
    q.addBindValue( post.permaLink().url() );
    q.addBindValue( post.summary() );
    q.addBindValue( post.slug() );
    q.addBindValue( post.tags().join(QString(',')) );
    q.addBindValue( post.status() );

    int ret;
    if ( q.exec() ) {
        ret = q.lastInsertId().toInt();

        ///Delete previouse Categories (if there are any!) :
        QSqlQuery qd;
        qd.prepare( "DELETE FROM post_cat WHERE postId=? AND blogId=(SELECT blogid FROM blog where id=?)" );
        qd.addBindValue(post.postId());
        qd.addBindValue(blog_id);
        if ( !qd.exec() ) {
            mLastErrorText = qd.lastError().text();
            kError() << "Cannot delete previous categories.";
        }

        int cat_count = post.categories().count();
        if( cat_count > 0 ) {
//             kDebug()<< "Adding "<<cat_count<<" category to post.";
            QSqlQuery q2;
            q2.prepare( "INSERT OR REPLACE INTO post_cat (blogId, postId, categoryId)\
            VALUES((SELECT blogid FROM blog where id=?), ?, \
            (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" );
            for ( int i = 0; i < cat_count; ++i ) {
                q2.addBindValue(blog_id);
                q2.addBindValue(post.postId());
                q2.addBindValue(post.categories()[i]);
                q2.addBindValue(blog_id);
                if ( !q2.exec() ) {
                    kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
                    mLastErrorText = q.lastError().text();
                }
            }
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot Add post to database!\n\tSQL Error: " << q.lastError().text();
        ret = -1;
    }

    return ret;
}

bool DBMan::editPost( const BilboPost & post, int blog_id )
{
    kDebug();
    QSqlQuery q;
    q.prepare( "UPDATE post SET author=?, title=?, content=?, text_more=?, c_time=?, m_time=?,\
               is_private=?, is_comment_allowed=?, is_trackback_allowed=?, link=?, perma_link=?, summary=?,\
               slug=?, tags=?, status=? WHERE postid=? AND blog_id=?" );
    q.addBindValue( post.author() );
    q.addBindValue( post.title() );
    q.addBindValue( post.content() );
    q.addBindValue( post.additionalContent() );
    q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
    q.addBindValue( post.modificationDateTime().toString( KDateTime::ISODate ) );
    q.addBindValue( post.isPrivate() );
    q.addBindValue( post.isCommentAllowed() );
    q.addBindValue( post.isTrackBackAllowed() );
    q.addBindValue( post.link().url() );
    q.addBindValue( post.permaLink().url() );
    q.addBindValue( post.summary() );
    q.addBindValue( post.slug() );
    q.addBindValue( post.tags().join(QString(',')) );
    q.addBindValue( post.status() );

    q.addBindValue( post.postId() );
    q.addBindValue( blog_id );

    if ( !q.exec() ) {
        mLastErrorText = q.lastError().text();
        kDebug()<<"Modifying post failed, SQL ERROR: "<< mLastErrorText;
        return false;
    }

    ///Delete previouse Categories:
    QSqlQuery qd;
    qd.prepare( "DELETE FROM post_cat WHERE postId=? AND blogId=(SELECT blogid FROM blog where id=?)" );
    qd.addBindValue(post.postId());
    qd.addBindValue(blog_id);
    if ( !qd.exec() ) {
        mLastErrorText = qd.lastError().text();
        kDebug() << "Cannot delete previous categories.";
    }

    ///Add new Categories:

    int cat_count = post.categories().count();
    if( cat_count > 0 ) {
//             kDebug()<< "Adding "<<cat_count<<" category to post.";
        QSqlQuery q2;
        q2.prepare( "INSERT OR REPLACE INTO post_cat (blogId, postId, categoryId)\
        VALUES((SELECT blogid FROM blog where id=?), ?, \
        (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" );
        for ( int i = 0; i < cat_count; ++i ) {
            q2.addBindValue(blog_id);
            q2.addBindValue(post.postId());
            q2.addBindValue(post.categories()[i]);
            q2.addBindValue(blog_id);
            if ( !q2.exec() ) {
                mLastErrorText = q2.lastError().text();
                kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
            }
        }
    }

    return true;
}

bool DBMan::removePost( int id )
{
    QSqlQuery q;
    q.prepare( "DELETE FROM post WHERE id=?" );
    q.addBindValue( id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << mLastErrorText;
    }
    return res;
}

bool DBMan::removePost( int blog_id, QString postId)
{
    QSqlQuery q;
    q.prepare( "DELETE FROM post WHERE blog_id=? AND postId=?" );
    q.addBindValue( blog_id );
    q.addBindValue( postId );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << mLastErrorText;
    }
    return res;
}

bool DBMan::clearPosts( int blog_id )
{
    QSqlQuery q;
    q.prepare( "DELETE FROM post WHERE blog_id=?" );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

int DBMan::addCategory( const QString &name, const QString &description, const QString &htmlUrl,
                        const QString &rssUrl, const QString &categoryId, const QString &parentId, int blog_id )
{
    QSqlQuery q;
    q.prepare( "INSERT OR REPLACE INTO category (name, description, htmlUrl, rssUrl, categoryId, parentId, blog_id)\
               VALUES(?, ?, ?, ?, ?, ?, ?)" );
    q.addBindValue( name );
    q.addBindValue( description );
    q.addBindValue( htmlUrl );
    q.addBindValue( rssUrl );
    q.addBindValue( categoryId );
    q.addBindValue( parentId );
    q.addBindValue( blog_id );

    if ( q.exec() )
        return q.lastInsertId().toInt();
    else {
        mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::clearCategories( int blog_id )
{
    QSqlQuery q;
    q.prepare( "DELETE FROM category WHERE blog_id=?" );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

int DBMan::addFile( QString name, int blog_id, bool isUploaded, QString localUrl, QString remoteUrl )
// int DBMan::addFile( const QString &name, int blog_id, bool isLocal, const QString &localUrl, const QString &remoteUrl )
{
    QSqlQuery q;
//  q.prepare("INSERT INTO file(name, blog_id, is_uploaded, local_url, remote_url) VALUES(?, ?, ?, ?, ?)");
    q.prepare( "INSERT INTO file(name, blog_id, is_local, local_url, remote_url) VALUES(?, ?, ?, ?, ?)" );
    q.addBindValue( name );
    q.addBindValue( blog_id );
    q.addBindValue( isUploaded );
//     q.addBindValue( isLocal );
    q.addBindValue( localUrl );
    q.addBindValue( remoteUrl );

    if ( q.exec() )
        return q.lastInsertId().toInt();
    else {
        mLastErrorText = q.lastError().text();
        return -1;
    }
}

int DBMan::addFile(const BilboMedia & file)
{
    QSqlQuery q;
    q.prepare( "INSERT INTO file(name, blog_id, is_local, local_url, remote_url) VALUES(?, ?, ?, ?, ?)" );
    q.addBindValue( file.name() );
    q.addBindValue( file.blogId() );
    q.addBindValue( file.isUploaded() );
    q.addBindValue( file.localUrl() );
    q.addBindValue( file.remoteUrl() );

    if ( q.exec() )
        return q.lastInsertId().toInt();
    else {
        mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::removeFile( int fileid )
{
    QSqlQuery q;
    q.prepare( "DELETE FROM file WHERE fileid=?" );
    q.addBindValue( fileid );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

bool DBMan::clearFiles( int blog_id )
{
    QSqlQuery q;
    q.prepare( "DELETE FROM file WHERE blog_id=?" );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

int DBMan::saveLocalEntry( const BilboPost& post, int blog_id )
{
    return saveTemp_LocalEntry(post, blog_id, Local);
}

int DBMan::saveTempEntry( const BilboPost& post, int blog_id )
{
    return saveTemp_LocalEntry(post, blog_id, Temp);
}

int DBMan::saveTemp_LocalEntry( const BilboPost& basePost, int blog_id, LocalPostState state )
{
    kDebug();
    QSqlQuery q;
    BilboPost post = basePost;
//     kDebug()<<"postId: "<<post.postId();
    QString postTable, postCatTable;
    if(state == Local) {
        postTable = "local_post";
        postCatTable = "local_post_cat";
    } else {
        postTable = "temp_post";
        postCatTable = "temp_post_cat";
    }
    int postId = -1, localId=-1;
    if(post.status() == KBlog::BlogPost::New) {///Post is new!
        if(post.id() == -1){
            ///Add new post to temp_post
            q.prepare( "INSERT OR REPLACE INTO "+ postTable +" (postid, blog_id, author, title, content,\
            text_more, c_time, m_time, is_private, is_comment_allowed, is_trackback_allowed, link, perma_link,\
            summary, slug, tags, status) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" );
            q.addBindValue( post.postId() );
            q.addBindValue( blog_id );
            q.addBindValue( post.author() );
            q.addBindValue( post.title() );
            q.addBindValue( post.content() );
            q.addBindValue( post.additionalContent() );
            q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
            q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
            q.addBindValue( post.isPrivate() );
            q.addBindValue( post.isCommentAllowed() );
            q.addBindValue( post.isTrackBackAllowed() );
            q.addBindValue( post.link().url() );
            q.addBindValue( post.permaLink().url() );
            q.addBindValue( post.summary() );
            q.addBindValue( post.slug() );
            q.addBindValue( post.tags().join(QString(',')) );
            q.addBindValue( post.status() );

            if ( q.exec() ) {
                localId = postId = q.lastInsertId().toInt();
            } else {
                mLastErrorText = q.lastError().text();
                kDebug() << "Cannot Add new local post to database!\n\tSQL Error: " << q.lastError().text();
                return -1;
            }
        } else {
            ///Update post, with id!
            q.prepare( "INSERT OR REPLACE INTO "+ postTable +" (local_id, postid, blog_id, author, title,\
            content, text_more, c_time, m_time, is_private, is_comment_allowed, is_trackback_allowed, link,\
            perma_link, summary, slug, tags, status)\
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" );
            q.addBindValue( post.id() );
            q.addBindValue( post.postId() );
            q.addBindValue( blog_id );
            q.addBindValue( post.author() );
            q.addBindValue( post.title() );
            q.addBindValue( post.content() );
            q.addBindValue( post.additionalContent() );
            q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
            q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
            q.addBindValue( post.isPrivate() );
            q.addBindValue( post.isCommentAllowed() );
            q.addBindValue( post.isTrackBackAllowed() );
            q.addBindValue( post.link().url() );
            q.addBindValue( post.permaLink().url() );
            q.addBindValue( post.summary() );
            q.addBindValue( post.slug() );
            q.addBindValue( post.tags().join(QString(',')) );
            q.addBindValue( post.status() );

            if ( q.exec() ) {
                localId = postId = q.lastInsertId().toInt();
            } else {
                mLastErrorText = q.lastError().text();
                kDebug() << "Cannot Add new local post to database!\n\tSQL Error: " << q.lastError().text();
                return -1;
            }
        }
    } else {///Post is already created at "Post" table and has a valid id, postId and blog_id. So, local_id is useless here
        q.prepare( "INSERT OR REPLACE INTO "+ postTable +" (id, postid, blog_id, author, title,\
        content, text_more, c_time, m_time, is_private,\
        is_comment_allowed, is_trackback_allowed, link, perma_link, summary, slug, tags, status)\
        VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" );
        q.addBindValue( post.id() );
        q.addBindValue( post.postId() );
        q.addBindValue( blog_id );
        q.addBindValue( post.author() );
        q.addBindValue( post.title() );
        q.addBindValue( post.content() );
        q.addBindValue( post.additionalContent() );
        q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
        q.addBindValue( post.creationDateTime().toString( KDateTime::ISODate ) );
        q.addBindValue( post.isPrivate() );
        q.addBindValue( post.isCommentAllowed() );
        q.addBindValue( post.isTrackBackAllowed() );
        q.addBindValue( post.link().url() );
        q.addBindValue( post.permaLink().url() );
        q.addBindValue( post.summary() );
        q.addBindValue( post.slug() );
        q.addBindValue( post.tags().join(QString(',')) );
        q.addBindValue( post.status() );

        if ( q.exec() ) {
            postId = post.id();
            localId = q.lastInsertId().toInt();
        } else {
            mLastErrorText = q.lastError().text();
            kDebug() << "Cannot Add or Edit local post to database!\n\tSQL Error: " << q.lastError().text();
            return -1;
        }
    }

    ///Delete previouse Categories:
    QSqlQuery qd;
    qd.prepare( "DELETE FROM " + postCatTable + " WHERE local_id=?" );
    qd.addBindValue( localId );
    if ( !qd.exec() ) {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot delete previouse categories.";
    }

    ///Add new Categories:
    ///Using post_id or if it's empty local_id for postId
    int cat_count = post.categories().count();
    if( cat_count > 0 ) {
        //kDebug()<< "Adding "<<cat_count<<" category to post.";
        QSqlQuery q2;
        q2.prepare( "INSERT OR REPLACE INTO " + postCatTable + " (local_id, categoryId)\
        VALUES(?, (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" );
        for ( int i = 0; i < cat_count; ++i ) {
            q2.addBindValue(localId);
            q2.addBindValue(post.categories()[i]);
            q2.addBindValue(blog_id);
            if ( !q2.exec() ) {
                mLastErrorText = q.lastError().text();
                kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
            }
//             kDebug()<<"category "<<post.categories()[i] <<" added.";
        }
    }
    return postId;
}

bool DBMan::removeLocalEntry( const BilboPost &post )
{
    kDebug();
    QSqlQuery q;
    if(post.status() == KBlog::BlogPost::New) {
        q.prepare( "DELETE FROM local_post WHERE local_id=?" );
    } else {
        q.prepare( "DELETE FROM local_post WHERE id=?" );
    }
    q.addBindValue( post.id() );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << mLastErrorText;
    }
    return res;
}

bool DBMan::removeLocalEntry( int local_id )
{
    kDebug();
    QSqlQuery q;
    q.prepare( "DELETE FROM local_post WHERE local_id=?" );
    q.addBindValue( local_id );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << mLastErrorText;
    }
    return res;
}

bool DBMan::removeTempEntry( const BilboPost &post )
{
    kDebug();
    QSqlQuery q;
    if(post.status() == KBlog::BlogPost::New) {
        q.prepare( "DELETE FROM temp_post WHERE local_id=?" );
    } else {
        q.prepare( "DELETE FROM temp_post WHERE id=?" );
    }
    q.addBindValue( post.id() );
    bool res = q.exec();
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    kDebug()<<"Id: "<<post.id()<<"\tStatus: "<<post.status();
    return res;
}

bool DBMan::clearTempEntries()
{
    kDebug();
    QSqlQuery q;
    bool res = q.exec( "DELETE FROM temp_post" );
    if ( !res ) {
        mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

const BilboBlog &DBMan::blog(int blog_id)
{
    return *blogList().value(blog_id);
}

QList< BilboBlog *> DBMan::listBlogs()
{
    QList<BilboBlog *> list;
    QSqlQuery q;
    if (q.exec( "SELECT id, blogid, blog_url, username, style_url, api_type, title,\
            direction, local_directory, password FROM blog" ) ) {
        while ( q.next() ) {
            BilboBlog *tmp = new BilboBlog;
            tmp->setId( q.value( 0 ).toInt() );
            tmp->setBlogId( q.value( 1 ).toString() );
            tmp->setUrl( QUrl( q.value( 2 ).toString() ) );
            tmp->setUsername( q.value( 3 ).toString() );
            tmp->setBlogUrl( q.value( 4 ).toString() );
            tmp->setApi(( BilboBlog::ApiType )q.value( 5 ).toInt() );
            tmp->setTitle( q.value( 6 ).toString() );
            tmp->setDirection(( Qt::LayoutDirection )q.value( 7 ).toInt() );
            tmp->setLocalDirectory( q.value( 8 ).toString() );
            if( useWallet ) {
                QString buffer;
                if ( mWallet && mWallet->readPassword( tmp->url().url() + '_' + tmp->username() , buffer )
                    == 0 && !buffer.isEmpty() ) {
                    tmp->setPassword( buffer );
                    kDebug() << "Password loaded from kde wallet.";
                }
            } else {
                tmp->setPassword( q.value( 9 ).toString() );
            }
            list.append( tmp );
        }
    } else {
        mLastErrorText = q.lastError().text();
    }
    return list;
}

QMap< QString, int > DBMan::listBlogsTitle()
{
    QMap< QString, int > list;
    QSqlQuery q;
    if( q.exec( "SELECT title, id FROM blog" ) ) {
        while ( q.next() ) {
            list[q.value( 0 ).toString()] = q.value( 1 ).toInt();
        }
    } else {
        mLastErrorText = q.lastError().text();
    }
    return list;
}

QList< BilboPost* > DBMan::listPosts( int blog_id )
{
    QList<BilboPost *> list;
    QSqlQuery q;
    q.prepare( "SELECT id, postid, author, title, content, c_time, m_time, is_private, is_comment_allowed,\
               is_trackback_allowed, link, perma_link, summary, tags, status, text_more, slug\
               FROM post WHERE blog_id = ? ORDER BY c_time DESC" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            BilboPost *tmp = new BilboPost();
            tmp->setId( q.value( 0 ).toInt() );
            tmp->setAuthor( q.value( 2 ).toString() );
            tmp->setPostId( q.value( 1 ).toString() );
            tmp->setTitle( q.value( 3 ).toString() );
            tmp->setContent( q.value( 4 ).toString() );
            tmp->setCreationDateTime( KDateTime::fromString( q.value( 5 ).toString(), KDateTime::ISODate ) );
            tmp->setModificationDateTime( KDateTime::fromString( q.value( 6 ).toString(), KDateTime::ISODate ) );
            tmp->setPrivate( q.value( 7 ).toBool() );
            tmp->setCommentAllowed( q.value( 8 ).toBool() );
            tmp->setTrackBackAllowed( q.value( 9 ).toBool() );
            tmp->setLink( KUrl( q.value( 10 ).toString() ) );
            tmp->setPermaLink( KUrl( q.value( 11 ).toString() ) );
            tmp->setSummary( q.value( 12 ).toString() );
            tmp->setTags( q.value( 13 ).toString().split( ',', QString::SkipEmptyParts ) );
            tmp->setStatus(( KBlog::BlogPost::Status ) q.value( 14 ).toInt() );
            tmp->setAdditionalContent( q.value( 15 ).toString() );
            tmp->setSlug( q.value( 16 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( "SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
                        category.categoryId, category.parentId\
                        FROM category JOIN post_cat ON category.categoryId=post_cat.categoryId\
                        WHERE post_cat.postId = ? AND post_cat.blogId = (SELECT blogid FROM blog where id=?)" );
            q2.addBindValue( tmp->postId() );
            q2.addBindValue( blog_id );
            if ( q2.exec() ) {
                while ( q2.next() ) {
                    Category cat;
                    cat.blog_id = blog_id;
                    cat.name = q2.value( 0 ).toString();
                    cat.description = q2.value( 1 ).toString();
                    cat.htmlUrl = q2.value( 2 ).toString();
                    cat.rssUrl = q2.value( 3 ).toString();
                    cat.categoryId = q2.value( 4 ).toString();
                    cat.parentId = q2.value( 5 ).toString();
                    catList.append( cat );
                }
            } else {
                mLastErrorText = q2.lastError().text();
            }
            tmp->setCategoryList( catList );
            list.append( tmp );
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id << "error: "<< mLastErrorText;
    }
    return list;
}

BilboPost DBMan::getPostInfo( int post_id )
{
    QSqlQuery q;
    BilboPost tmp;
    q.prepare( "SELECT id, postid, author, title, content, c_time, m_time, is_private, is_comment_allowed,\
               is_trackback_allowed, link, perma_link, summary, tags, status, blog_id, text_more, slug\
               FROM post WHERE id = ?" );
    q.addBindValue( post_id );
    if ( q.exec() ) {
        if ( q.next() ) {
            tmp.setId( q.value( 0 ).toInt() );
            tmp.setAuthor( q.value( 2 ).toString() );
            tmp.setPostId( q.value( 1 ).toString() );
            tmp.setTitle( q.value( 3 ).toString() );
            tmp.setContent( q.value( 4 ).toString() );
            tmp.setCreationDateTime( KDateTime::fromString( q.value( 5 ).toString(), KDateTime::ISODate ) );
            tmp.setModificationDateTime( KDateTime::fromString( q.value( 6 ).toString(), KDateTime::ISODate ) );
            tmp.setPrivate( q.value( 7 ).toBool() );
            tmp.setCommentAllowed( q.value( 8 ).toBool() );
            tmp.setTrackBackAllowed( q.value( 9 ).toBool() );
            QUrl u( q.value( 10 ).toString() );
            tmp.setLink( u );
            QUrl pu( q.value( 11 ).toString() );
            tmp.setPermaLink( pu );
            tmp.setSummary( q.value( 12 ).toString() );
            tmp.setTags( q.value( 13 ).toString().split( ',', QString::SkipEmptyParts ) );
            tmp.setStatus(( KBlog::BlogPost::Status ) q.value( 14 ).toInt() );
            int blog_id = q.value( 15 ).toInt();
            tmp.setAdditionalContent(  q.value( 16 ).toString() );
            tmp.setSlug(  q.value( 17 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( "SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
                        category.categoryId, category.parentId, category.blog_id\
                        FROM category JOIN post_cat ON category.categoryId=post_cat.categoryId\
                        WHERE post_cat.postId = ? AND post_cat.blogId = (SELECT blogid FROM blog where id=?)" );
            q2.addBindValue( tmp.postId() );
            q2.addBindValue( blog_id );
            if ( q2.exec() ) {
                while ( q2.next() ) {
                    Category cat;
                    cat.blog_id = q2.value( 6 ).toInt();
                    cat.name = q2.value( 0 ).toString();
                    cat.description = q2.value( 1 ).toString();
                    cat.htmlUrl = q2.value( 2 ).toString();
                    cat.rssUrl = q2.value( 3 ).toString();
                    cat.categoryId = q2.value( 4 ).toString();
                    cat.parentId = q2.value( 5 ).toString();
                    catList.append( cat );
                }
            } else {
                mLastErrorText = q2.lastError().text();
            }
            tmp.setCategoryList( catList );
        } else {
            mLastErrorText = i18n( "There is no post with the requested ID" );
            kDebug() << "There isn't any post with id: " << post_id;
            tmp.setStatus(KBlog::BlogPost::Error);
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get post with id " << post_id;
        tmp.setStatus(KBlog::BlogPost::Error);
    }

    return tmp;
}

QMap< int, QString > DBMan::listPostsTitle( int blog_id )
{
    QMap< int, QString >list;
    QSqlQuery q;
    q.prepare( "SELECT title, id FROM post WHERE blog_id = ? ORDER BY c_time DESC" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list.insert( q.value( 1 ).toInt(), q.value( 0 ).toString() );
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id;
    }
    return list;
}

QList<QVariantMap> DBMan::listPostsInfo( int blog_id )
{
    QList<QVariantMap> list;
    QSqlQuery q;
    q.prepare( "SELECT title, id, c_time, is_private FROM post WHERE blog_id = ? ORDER BY c_time DESC" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            QVariantMap entry;
            entry[ "title" ] = q.value( 0 ).toString();
            entry[ "id" ] = q.value( 1 ).toInt();
            entry[ "c_time" ] = KDateTime::fromString( q.value( 2 ).toString() ).dateTime();
            entry[ "is_private" ] = q.value( 3 ).toBool();
            list.append(entry);
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id;
    }
    return list;
}

QMap< QString, int > DBMan::listCategoriesName( int blog_id )
{
    QMap< QString, int > list;
    QSqlQuery q;
    q.prepare( "SELECT name, catid FROM category WHERE blog_id = ?" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list[q.value( 0 ).toString()] = q.value( 1 ).toInt();
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QList< Category > DBMan::listCategories( int blog_id )
{
    QList< Category > list;
    QSqlQuery q;
    q.prepare( "SELECT catid, name, description, htmlUrl, rssUrl, categoryId, parentId FROM category\
               WHERE blog_id = ?" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            Category c;
            c.blog_id = blog_id;
            c.id = q.value( 0 ).toInt();
            c.name = q.value( 1 ).toString();
            c.description = q.value( 2 ).toString();
            c.htmlUrl = q.value( 3 ).toString();
            c.rssUrl = q.value( 4 ).toString();
            c.categoryId = q.value( 5 ).toString();
            c.parentId = q.value( 6 ).toString();
            list.append( c );
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QMap< QString, bool > DBMan::listCategoriesId( int blog_id )
{
    QMap< QString, bool > list;
    QSqlQuery q;
    q.prepare( "SELECT categoryId FROM category\
               WHERE blog_id = ?" );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list.insert( q.value( 0 ).toString(), false );
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QMap<BilboPost*, int> DBMan::listTempPosts()
{
    QMap<BilboPost*, int> list;
    QSqlQuery q;
    q.prepare( "SELECT id, local_id, postid, blog_id, author, title, content, text_more, c_time,\
    m_time, is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, tags, status,\
    slug FROM temp_post ORDER BY m_time DESC" );
    if ( q.exec() ) {
        while ( q.next() ) {
            BilboPost *tmp = new BilboPost();
            int id = q.value( 0 ).toInt();
            int local_id = q.value( 1 ).toInt();
            tmp->setPostId( q.value( 2 ).toString() );
            int blog_id = q.value( 3 ).toInt();
            tmp->setAuthor( q.value( 4 ).toString() );
            tmp->setTitle( q.value( 5 ).toString() );
            tmp->setContent( q.value( 6 ).toString() );
            tmp->setCreationDateTime( KDateTime::fromString( q.value( 8 ).toString(), KDateTime::ISODate ) );
            tmp->setModificationDateTime( KDateTime::fromString( q.value( 9 ).toString(), KDateTime::ISODate ) );
            tmp->setPrivate( q.value( 10 ).toBool() );
            tmp->setCommentAllowed( q.value( 11 ).toBool() );
            tmp->setTrackBackAllowed( q.value( 12 ).toBool() );
            tmp->setLink( KUrl( q.value( 13 ).toString() ) );
            tmp->setPermaLink( KUrl( q.value( 14 ).toString() ) );
            tmp->setSummary( q.value( 15 ).toString() );
            tmp->setTags( q.value( 16 ).toString().split( ',', QString::SkipEmptyParts ) );
            tmp->setStatus(( KBlog::BlogPost::Status ) q.value( 17 ).toInt() );
            tmp->setSlug( q.value( 18 ).toString() );

            if(tmp->status() == KBlog::BlogPost::New){
                tmp->setId(local_id);
            } else {
                tmp->setId(id);
            }
            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( "SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
            category.categoryId, category.parentId\
            FROM category JOIN temp_post_cat ON category.categoryId=temp_post_cat.categoryId\
            WHERE temp_post_cat.local_id = ?" );
            q2.addBindValue( local_id );
//             q2.addBindValue( blog_id );
            if ( q2.exec() ) {
                while ( q2.next() ) {
                    Category cat;
                    cat.blog_id = blog_id;
                    cat.name = q2.value( 0 ).toString();
                    cat.description = q2.value( 1 ).toString();
                    cat.htmlUrl = q2.value( 2 ).toString();
                    cat.rssUrl = q2.value( 3 ).toString();
                    cat.categoryId = q2.value( 4 ).toString();
                    cat.parentId = q2.value( 5 ).toString();
                    catList.append( cat );
                }
                tmp->setCategoryList( catList );
                list.insert( tmp, blog_id);
            } else {
                mLastErrorText = q2.lastError().text();
                kDebug()<<"Cannot get categories list of a post. SQL Error: "<< q2.lastError().text();
            }
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of temporary posts, SQL Error: "<< q.lastError().text();
    }
    return list;
}

QList<QVariantMap> DBMan::listLocalPosts()
{
    kDebug();
    QList<QVariantMap> list;
    QSqlQuery q;
    q.prepare( "SELECT local_post.local_id, local_post.title, local_post.blog_id, blog.title\
    FROM local_post LEFT JOIN blog ON local_post.blog_id = blog.id ORDER BY m_time DESC" );
    if ( q.exec() ) {
        while ( q.next() ) {
            QVariantMap entry;
            entry[ "local_id" ] = q.value( 0 ).toInt();
            entry[ "post_title" ] = q.value( 1 ).toString();
            entry[ "blog_id" ] = q.value( 2 ).toInt();
            entry[ "blog_title" ] = q.value( 3 ).toString();
            list.append(entry);
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of local posts. SQL Error: "<< q.lastError().text();
    }
    return list;
}

BilboPost DBMan::localPost(int local_id)
{
    QSqlQuery q;
    BilboPost tmp;
    q.prepare( "SELECT id, local_id, postid, blog_id, author, title, content, text_more, c_time,\
    m_time, is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, tags, status,\
    slug FROM local_post WHERE local_id=?" );
    q.addBindValue(local_id);
    if ( q.exec() ) {
        if ( q.next() ) {
            int id = q.value( 0 ).toInt();
            tmp.setPostId( q.value( 2 ).toString() );
            int blog_id = q.value( 3 ).toInt();
            tmp.setAuthor( q.value( 4 ).toString() );
            tmp.setTitle( q.value( 5 ).toString() );
            tmp.setContent( q.value( 6 ).toString() );
            tmp.setAdditionalContent( q.value( 7 ).toString() );
            tmp.setCreationDateTime( KDateTime::fromString( q.value( 8 ).toString(), KDateTime::ISODate ) );
            tmp.setModificationDateTime( KDateTime::fromString( q.value( 9 ).toString(), KDateTime::ISODate ) );
            tmp.setPrivate( q.value( 10 ).toBool() );
            tmp.setCommentAllowed( q.value( 11 ).toBool() );
            tmp.setTrackBackAllowed( q.value( 12 ).toBool() );
            tmp.setLink( KUrl( q.value( 13 ).toString() ) );
            tmp.setPermaLink( KUrl( q.value( 14 ).toString() ) );
            tmp.setSummary( q.value( 15 ).toString() );
            tmp.setTags( q.value( 16 ).toString().split( ',', QString::SkipEmptyParts ) );
            tmp.setStatus(( KBlog::BlogPost::Status ) q.value( 17 ).toInt() );
            tmp.setSlug( q.value( 18 ).toString() );

            if(tmp.status() == KBlog::BlogPost::New){
                tmp.setId(local_id);
            } else {
                tmp.setId(id);
            }
            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( "SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
            category.categoryId, category.parentId\
            FROM category JOIN local_post_cat ON category.categoryId=local_post_cat.categoryId\
            WHERE local_post_cat.local_id = ?" );
            q2.addBindValue( local_id );
            if ( q2.exec() ) {
                while ( q2.next() ) {
                    Category cat;
                    cat.blog_id = blog_id;
                    cat.name = q2.value( 0 ).toString();
                    cat.description = q2.value( 1 ).toString();
                    cat.htmlUrl = q2.value( 2 ).toString();
                    cat.rssUrl = q2.value( 3 ).toString();
                    cat.categoryId = q2.value( 4 ).toString();
                    cat.parentId = q2.value( 5 ).toString();
                    catList.append( cat );
                }
                tmp.setCategoryList( catList );
            } else {
                mLastErrorText = q2.lastError().text();
                kDebug()<<"Cannot get categories list of local post. SQL Error: "<< q2.lastError().text();
            }
        } else {
            mLastErrorText = i18n( "There is no local post with the requested ID " );
            kDebug()<<"there isn't any local post with local_id "<<local_id;
        }
    } else {
        mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get local post. SQL Error: "<< q.lastError().text();
    }
    return tmp;
}
