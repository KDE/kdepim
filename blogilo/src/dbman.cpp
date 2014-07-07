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
//krazy:excludeall=crashy to skip false positives due to QSqlQuery.exec() usage

#include "dbman.h"
#include "bilboblog.h"
#include "bilbopost.h"

#include <kdebug.h>
#include <KDE/KLocale>
#include <kdatetime.h>
#include <kurl.h>
#include <kwallet.h>
#include <kio/deletejob.h>
#include <kmessagebox.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QSqlDatabase>


class DBMan::Private
{
public:
    Private()
        : mWallet(0),
          useWallet(false)
    {
    }
    KWallet::Wallet* mWallet;
    QString mLastErrorText;
    bool useWallet;
    QMap<int, BilboBlog*> mBlogList;
    QSqlDatabase db;

    static const int DatabaseSchemaVersion = 1;
};

DBMan::DBMan()
    : d(new Private)
{
    d->mWallet = KWallet::Wallet::openWallet( KWallet::Wallet::LocalWallet(), 0 );
    if ( d->mWallet ) {
        d->useWallet = true;
        if ( !d->mWallet->hasFolder( QLatin1String("blogilo") ) ) {
            d->mWallet->createFolder( QLatin1String("blogilo") );
        }
        d->mWallet->setFolder( QLatin1String("blogilo") );
        kDebug() << "Wallet successfully opened.";
    } else {
        d->useWallet = false;
        kDebug() << "Could not use Wallet service, will use database to store passwords";
    }

    if ( !QFile::exists( CONF_DB ) ) {
        if ( !this->createDB() ) {
            KMessageBox::detailedError( 0, i18n( "Cannot create database" ),
                                        i18n( d->db.lastError().text().toUtf8().data() ) );
            kDebug() << "Cannot create database, SQL error: " << d->db.lastError().text() << endl;
            exit ( 1 );
        }
    } else if ( !connectDB() ) {
        kDebug() << d->mLastErrorText;
        exit( 1 );
    }

    if ( !updateDB() ) {
        kDebug() << d->mLastErrorText;
        exit( 1 );
    }

    reloadBlogList();
}

DBMan::~DBMan()
{
    kDebug();
    d->db.close();
    if(d->useWallet) {
        d->mWallet->deleteLater();
        d->mWallet = 0;
    }
    delete d;
    mSelf = 0L;
}


QString DBMan::lastErrorText() const
{
    return d->mLastErrorText;
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
    return d->mBlogList;
}

void DBMan::reloadBlogList()
{
    d->mBlogList.clear();
    QList<BilboBlog*> listBlogs = this->listBlogs();
    const int count = listBlogs.count();
    for ( int i = 0; i < count; ++i ) {
        d->mBlogList [ listBlogs.at(i)->id() ] = listBlogs.at(i);
    }
}

bool DBMan::connectDB()
{
    kDebug();
    if( d->db.isOpen() )
        return true;
    d->db = QSqlDatabase::addDatabase( QLatin1String("QSQLITE") );
    d->db.setDatabaseName( QString(CONF_DB) );

    if ( !d->db.open() ) {
        KMessageBox::detailedError( 0, i18n( "Cannot connect to database" ),
                                    i18n( d->db.lastError().text().toUtf8().data() ) );
        kDebug() << "Cannot connect to database, SQL error: " << d->db.lastError().text();
        return false;
    }
    return true;
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
    if ( !q.exec( QLatin1String("CREATE TABLE blog (id INTEGER PRIMARY KEY, blogid TEXT, blog_url TEXT, username TEXT,\
                  password TEXT, style_url TEXT, api_type TEXT, title TEXT, direction TEXT,\
                  local_directory TEXT, icon_url TEXT)") ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///posts table!
    if ( !q.exec( QLatin1String("CREATE TABLE post (id INTEGER PRIMARY KEY, postid TEXT NOT NULL, blog_id NUMERIC NOT NULL,\
                  author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
                  c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
                  is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
                  status NUMERIC, trackback_urls TEXT, UNIQUE(postid, blog_id));") ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///comments table!
    if ( !q.exec( QLatin1String("CREATE TABLE comment (id INTEGER PRIMARY KEY, commentid TEXT NOT NULL, blog_id NUMERIC NOT NULL,\
        postId TEXT, author_name TEXT, author_url TEXT, author_email TEXT, title TEXT, content TEXT,\
        c_time TEXT, m_time TEXT, link TEXT, password TEXT,\
        status NUMERIC, UNIQUE(commentid, blog_id));" ) ) ){
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///categories table!
    if ( !q.exec( QLatin1String("CREATE TABLE category (catid INTEGER PRIMARY KEY, name TEXT NOT NULL,\
                  description TEXT, htmlUrl TEXT, rssUrl TEXT, categoryId TEXT, parentId TEXT,\
                  blog_id NUMERIC NOT NULL, UNIQUE(name,blog_id));" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///files table
    if( !q.exec( QLatin1String("CREATE TABLE file (fileid INTEGER PRIMARY KEY, name TEXT, blog_id NUMERIC, is_uploaded NUMERIC,\
        local_url TEXT, remote_url TEXT, mime_type TEXT);" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///connection bethween posts and categories
    if ( !q.exec( QLatin1String("CREATE TABLE post_cat (blogId TEXT NOT NULL, postId TEXT NOT NULL,\
        categoryId TEXT NOT NULL, UNIQUE(blogId,postId,categoryId));" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///connection bethween posts and media files
    if ( !q.exec( QLatin1String("CREATE TABLE post_file (post_id INTEGER, file_id INTEGER);" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///local posts table
    if( !q.exec( QLatin1String("CREATE TABLE local_post (local_id INTEGER PRIMARY KEY, id INTEGER UNIQUE, postid TEXT, blog_id NUMERIC,\
        author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
        c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
        is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
        status NUMERIC);" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///Connection between local_posts and categories
    if( !q.exec( QLatin1String("CREATE TABLE local_post_cat (local_id INT, categoryId TEXT);" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///temporary posts table
    if( !q.exec( QLatin1String("CREATE TABLE temp_post (local_id INTEGER PRIMARY KEY, id INTEGER UNIQUE, postid TEXT, blog_id NUMERIC,\
                 author TEXT, slug TEXT, post_password TEXT, title TEXT, content TEXT, text_more TEXT,\
                 c_time TEXT, m_time TEXT, is_private NUMERIC, is_comment_allowed NUMERIC,\
                 is_trackback_allowed NUMERIC, link TEXT, perma_link TEXT, summary TEXT, tags TEXT,\
                 status NUMERIC);" ) ) ){
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///Connection between temp_posts and categories
    if( !q.exec( QLatin1String("CREATE TABLE temp_post_cat (local_id INT, categoryId TEXT);" ) ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    ///Auth data
    if ( !q.exec( QLatin1String("CREATE TABLE auth_data (blog_id INT, key TEXT NOT NULL, values TEXT NOT NULL, UNIQUE(blog_id,key))") ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    if ( !q.exec( QLatin1String("CREATE TABLE schema_version (version INT);") ) ) {
        ret = false;
        d->mLastErrorText = q.lastError().text();
    }

    {
        QSqlQuery vq;
        vq.prepare( QLatin1String("INSERT INTO schema_version (version) VALUES (?);") );
        vq.addBindValue( DBMan::Private::DatabaseSchemaVersion );
        if ( !vq.exec() ) {
            ret = false;
            d->mLastErrorText = q.lastError().text();
        }
    }

    ///delete related information on DB, On removing a post or a blog
    q.exec( QLatin1String("CREATE TRIGGER delete_post AFTER DELETE ON post\
    BEGIN\
    DELETE FROM post_cat WHERE post_cat.postId=OLD.postid;\
    DELETE FROM post_file WHERE post_file.post_id=OLD.id;\
    DELETE FROM comment WHERE comment.postId=OLD.postid;\
    END" ) );
    q.exec( QLatin1String("CREATE TRIGGER delete_blog AFTER DELETE ON blog \
    BEGIN\
    DELETE FROM category WHERE category.blog_id=OLD.id;\
    DELETE FROM file WHERE file.blog_id=OLD.id;\
    DELETE FROM post WHERE post.blog_id=OLD.id;\
    DELETE FROM comment WHERE comment.blog_id=OLD.id;\
    DELETE FROM auth_data WHERE auth_data.blog_id=OLD.id;\
    END" ) );
    q.exec( QLatin1String("CREATE TRIGGER delete_temp_post AFTER DELETE ON temp_post \
    BEGIN\
    DELETE FROM temp_post_cat WHERE local_id=OLD.local_id;\
    END" ));
    q.exec( QLatin1String("CREATE TRIGGER delete_local_post AFTER DELETE ON local_post \
    BEGIN\
    DELETE FROM local_post_cat WHERE local_id=OLD.local_id;\
    END" ));

    return ret;
}

bool DBMan::updateDB()
{
    uint dbVersion = 0;

    /// Check whether schema_version exists
    if ( !d->db.tables(QSql::Tables).contains( QLatin1String("schema_version") ) ) {
        QSqlQuery q;
        if ( !q.exec( QLatin1String("CREATE TABLE schema_version (version INT);") ) ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }
        if ( !q.exec( QLatin1String("INSERT INTO schema_version (version) VALUES (0);") ) ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }
    }

    QSqlQuery q;
    if ( !q.exec( QLatin1String("SELECT version FROM schema_version;") ) ) {
        d->mLastErrorText = q.lastError().text();
        return false;
    }
    q.next();
    dbVersion = q.value(0).toUInt();

    if ( dbVersion < 1 ) {
        QSqlQuery q;
        if ( !q.exec( QLatin1String("CREATE TABLE auth_data (blog_id INT, key TEXT NOT NULL, value TEXT NOT NULL, UNIQUE(blog_id, key));") ) ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }

        if ( !q.exec( QLatin1String("DROP TRIGGER delete_blog") ) ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }

        if ( !q.exec( QLatin1String("CREATE TRIGGER delete_blog AFTER DELETE ON blog \
              BEGIN\
              DELETE FROM category WHERE category.blog_id=OLD.id;\
              DELETE FROM file WHERE file.blog_id=OLD.id;\
              DELETE FROM post WHERE post.blog_id=OLD.id;\
              DELETE FROM comment WHERE comment.blog_id=OLD.id;\
              DELETE FROM auth_data WHERE auth_data.blog_id=OLD.id;\
              END" ) ) ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }

        dbVersion = 1;
    }


    /// Update schema_version table
    {
        QSqlQuery q;
        q.prepare( QLatin1String("UPDATE schema_version SET version = ?") );
        q.addBindValue( dbVersion );
        if ( !q.exec() ) {
            d->mLastErrorText = q.lastError().text();
            return false;
        }
    }

    return true;
}

int DBMan::addBlog( const BilboBlog & blog )
{
    QSqlQuery q;
    if( d->useWallet ) {
        q.prepare( QLatin1String("INSERT INTO blog (blogid, blog_url, username, style_url, api_type, title,\
               direction, local_directory) VALUES(?, ?, ?, ?, ?, ?, ?, ?)" ) );
        if ( d->mWallet && d->mWallet->writePassword( blog.url().url() + QLatin1Char('_') + blog.username(), blog.password() ) == 0 )
            kDebug() << "Password stored to kde wallet";
        else
            return -1;
    } else {
        q.prepare( QLatin1String("INSERT INTO blog (password, blogid, blog_url, username, style_url, api_type, title,\
               direction, local_directory) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)" ) );
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
        d->mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::editBlog( const BilboBlog & blog )
{
    QSqlQuery q;
    if( d->useWallet ) {
        q.prepare( QLatin1String("UPDATE blog SET blogid=?, blog_url=?, username=? , style_url=? , api_type=?, \
                   title=?, direction=?, local_directory=? WHERE id=?" ) );
        if ( d->mWallet && d->mWallet->writePassword( blog.url().url() + QLatin1Char('_') + blog.username(), blog.password() ) == 0 )
            kDebug() << "Password stored to kde wallet";
        else
            return false;
    } else {
        q.prepare( QLatin1String("UPDATE blog SET password=?, blogid=?, blog_url=?, username=? , style_url=? , api_type=?, \
                    title=?, direction=?, local_directory=? WHERE id=?" ));
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
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
        return res;
    }
    reloadBlogList();
    return res;
}

bool DBMan::removeBlog( int blog_id )
{
    BilboBlog *tmp = d->mBlogList[ blog_id ];
    if( d->useWallet ) {
        if ( d->mWallet && d->mWallet->removeEntry( tmp->url().url() + QLatin1Char('_') + tmp->username() ) == 0
                        && d->mWallet->removeEntry( QString::number( blog_id ) ) == 0 )
            kDebug() << "Password removed to kde wallet";
    }
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM blog WHERE id=?" ));
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
        return res;
    }
    QString path = KStandardDirs::locateLocal( "data", QString::fromLatin1( "blogilo/%1/" ).arg( blog_id ) , false );
    KIO::del(KUrl(path), KIO::HideProgressInfo);
    reloadBlogList();
    return res;
}

int DBMan::addPost( const BilboPost & post, int blog_id )
{
    kDebug() << "Adding post with title: " << post.title() << " to Blog " << blog_id;
    QSqlQuery q;
    q.prepare( QLatin1String("INSERT OR REPLACE INTO post (postid, blog_id, author, title, content, text_more, c_time, m_time,\
               is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, slug,\
               tags, status) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" ) );
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
    q.addBindValue( post.tags().join(QLatin1String(",")) );
    q.addBindValue( post.status() );

    int ret;
    if ( q.exec() ) {
        ret = q.lastInsertId().toInt();

        ///Delete previouse Categories (if there are any!) :
        QSqlQuery qd;
        qd.prepare( QLatin1String("DELETE FROM post_cat WHERE postId=? AND blogId=(SELECT blogid FROM blog where id=?)" ));
        qd.addBindValue(post.postId());
        qd.addBindValue(blog_id);
        if ( !qd.exec() ) {
            d->mLastErrorText = qd.lastError().text();
            kError() << "Cannot delete previous categories.";
        }

        int cat_count = post.categories().count();
        if( cat_count > 0 ) {
//             kDebug()<< "Adding "<<cat_count<<" category to post.";
            QSqlQuery q2;
            q2.prepare( QLatin1String("INSERT OR REPLACE INTO post_cat (blogId, postId, categoryId)\
            VALUES((SELECT blogid FROM blog where id=?), ?, \
            (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" ) );
            for ( int i = 0; i < cat_count; ++i ) {
                q2.addBindValue(blog_id);
                q2.addBindValue(post.postId());
                q2.addBindValue(post.categories()[i]);
                q2.addBindValue(blog_id);
                if ( !q2.exec() ) {
                    kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
                    d->mLastErrorText = q.lastError().text();
                }
            }
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot Add post to database!\n\tSQL Error: " << q.lastError().text();
        ret = -1;
    }

    return ret;
}

bool DBMan::editPost( const BilboPost & post, int blog_id )
{
    kDebug();
    QSqlQuery q;
    q.prepare( QLatin1String("UPDATE post SET author=?, title=?, content=?, text_more=?, c_time=?, m_time=?,\
               is_private=?, is_comment_allowed=?, is_trackback_allowed=?, link=?, perma_link=?, summary=?,\
               slug=?, tags=?, status=? WHERE postid=? AND blog_id=?" ) );
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
    q.addBindValue( post.tags().join(QLatin1String(",")) );
    q.addBindValue( post.status() );

    q.addBindValue( post.postId() );
    q.addBindValue( blog_id );

    if ( !q.exec() ) {
        d->mLastErrorText = q.lastError().text();
        kDebug()<<"Modifying post failed, SQL ERROR: "<< d->mLastErrorText;
        return false;
    }

    ///Delete previouse Categories:
    QSqlQuery qd;
    qd.prepare( QLatin1String("DELETE FROM post_cat WHERE postId=? AND blogId=(SELECT blogid FROM blog where id=?)" ));
    qd.addBindValue(post.postId());
    qd.addBindValue(blog_id);
    if ( !qd.exec() ) {
        d->mLastErrorText = qd.lastError().text();
        kDebug() << "Cannot delete previous categories.";
    }

    ///Add new Categories:

    int cat_count = post.categories().count();
    if( cat_count > 0 ) {
//             kDebug()<< "Adding "<<cat_count<<" category to post.";
        QSqlQuery q2;
        q2.prepare( QLatin1String("INSERT OR REPLACE INTO post_cat (blogId, postId, categoryId)\
        VALUES((SELECT blogid FROM blog where id=?), ?, \
        (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" ) );
        for ( int i = 0; i < cat_count; ++i ) {
            q2.addBindValue(blog_id);
            q2.addBindValue(post.postId());
            q2.addBindValue(post.categories()[i]);
            q2.addBindValue(blog_id);
            if ( !q2.exec() ) {
                d->mLastErrorText = q2.lastError().text();
                kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
            }
        }
    }

    return true;
}

bool DBMan::removePost( int id )
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM post WHERE id=?" ));
    q.addBindValue( id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << d->mLastErrorText;
    }
    return res;
}

bool DBMan::removePost( int blog_id, const QString &postId)
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM post WHERE blog_id=? AND postId=?" ));
    q.addBindValue( blog_id );
    q.addBindValue( postId );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << d->mLastErrorText;
    }
    return res;
}

bool DBMan::clearPosts( int blog_id )
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM post WHERE blog_id=?") );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

int DBMan::addCategory( const QString &name, const QString &description, const QString &htmlUrl,
                        const QString &rssUrl, const QString &categoryId, const QString &parentId, int blog_id )
{
    QSqlQuery q;
    q.prepare( QLatin1String("INSERT OR REPLACE INTO category (name, description, htmlUrl, rssUrl, categoryId, parentId, blog_id)\
               VALUES(?, ?, ?, ?, ?, ?, ?)" ) );
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
        d->mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::clearCategories( int blog_id )
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM category WHERE blog_id=?") );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

int DBMan::addFile( const QString& name, int blog_id, bool isUploaded, const QString& localUrl, const QString& remoteUrl )
// int DBMan::addFile( const QString &name, int blog_id, bool isLocal, const QString &localUrl, const QString &remoteUrl )
{
    QSqlQuery q;
//  q.prepare("INSERT INTO file(name, blog_id, is_uploaded, local_url, remote_url) VALUES(?, ?, ?, ?, ?)");
    q.prepare( QLatin1String("INSERT INTO file(name, blog_id, is_local, local_url, remote_url) VALUES(?, ?, ?, ?, ?)") );
    q.addBindValue( name );
    q.addBindValue( blog_id );
    q.addBindValue( isUploaded );
//     q.addBindValue( isLocal );
    q.addBindValue( localUrl );
    q.addBindValue( remoteUrl );

    if ( q.exec() )
        return q.lastInsertId().toInt();
    else {
        d->mLastErrorText = q.lastError().text();
        return -1;
    }
}

int DBMan::addFile(const BilboMedia & file)
{
    QSqlQuery q;
    q.prepare( QLatin1String("INSERT INTO file(name, blog_id, is_local, local_url, remote_url) VALUES(?, ?, ?, ?, ?)") );
    q.addBindValue( file.name() );
    q.addBindValue( file.blogId() );
    q.addBindValue( file.isUploaded() );
    q.addBindValue( file.localUrl() );
    q.addBindValue( file.remoteUrl() );

    if ( q.exec() )
        return q.lastInsertId().toInt();
    else {
        d->mLastErrorText = q.lastError().text();
        return -1;
    }
}

bool DBMan::removeFile( int fileid )
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM file WHERE fileid=?") );
    q.addBindValue( fileid );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

bool DBMan::clearFiles( int blog_id )
{
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM file WHERE blog_id=?") );
    q.addBindValue( blog_id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
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
        postTable = QLatin1String("local_post");
        postCatTable = QLatin1String("local_post_cat");
    } else {
        postTable = QLatin1String("temp_post");
        postCatTable = QLatin1String("temp_post_cat");
    }
    int localId = post.localId();
//    if(post.status() == KBlog::BlogPost::New) {///Post is new!
//         kDebug()<<"Post is new!";
        if(post.localId() == -1){
            ///Add new post to temp_post
            kDebug()<<"Add new post to temp_post";
            q.prepare( QLatin1String("INSERT OR REPLACE INTO ")+ postTable +QLatin1String(" (postid, blog_id,\
            author, title, content, text_more, c_time, m_time, is_private, is_comment_allowed,\
            is_trackback_allowed, link, perma_link, summary, slug, tags, status)\
            VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)" ));
//             q.addBindValue( post.id() == -1 ? QVariant(QVariant::Int) : post.id() );
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
            q.addBindValue( post.tags().join(QLatin1String(",")) );
            q.addBindValue( post.status() );

            if ( q.exec() ) {
                localId = q.lastInsertId().toInt();
            } else {
                d->mLastErrorText = q.lastError().text();
                kDebug() << "Cannot Add new local post to database!\n\tSQL Error: " << q.lastError().text();
                return -1;
            }
        } else {
            ///Update post, with id!
            kDebug()<<"Update post, with id!";
            q.prepare( QLatin1String("UPDATE ")+ postTable +QLatin1String(" SET postid=?, blog_id=?,\
            author=?, title=?, content=?, text_more=?, c_time=?, m_time=?, is_private=?, is_comment_allowed=?,\
            is_trackback_allowed=?, link=?, perma_link=?, summary=?, slug=?, tags=?, status=?\
            WHERE local_id=?" ));
//             q.addBindValue( post.id() == -1 ? QVariant(QVariant::Int) : post.id() );
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
            q.addBindValue( post.tags().join(QLatin1String(",")) );
            q.addBindValue( post.status() );
            q.addBindValue( post.localId() );

            if ( !q.exec() ) {
                d->mLastErrorText = q.lastError().text();
                kDebug() << "Cannot Add new local post to database!\n\tSQL Error: " << q.lastError().text();
                return -1;
            }
        }
    /*} else {///Post is already created at "Post" table and has a valid id, postId and blog_id. So, local_id is useless here
        kDebug()<<"Post is already created at \"Post\" table and has a valid id, postId and blog_id. So, local_id is useless here";
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
            d->mLastErrorText = q.lastError().text();
            kDebug() << "Cannot Add or Edit local post to database!\n\tSQL Error: " << q.lastError().text();
            return -1;
        }
    }*/

    ///Delete previouse Categories:
    QSqlQuery qd;
    qd.prepare( QLatin1String("DELETE FROM ") + postCatTable + QLatin1String(" WHERE local_id=?") );
    qd.addBindValue( localId );
    if ( !qd.exec() ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot delete previouse categories.";
    }

    ///Add new Categories:
    ///Using post_id or if it's empty local_id for postId
    int cat_count = post.categories().count();
    if( cat_count > 0 ) {
        //kDebug()<< "Adding "<<cat_count<<" category to post.";
        QSqlQuery q2;
        q2.prepare( QLatin1String("INSERT OR REPLACE INTO ") + postCatTable + QLatin1String(" (local_id, categoryId)\
        VALUES(?, (SELECT categoryId FROM category WHERE name = ? AND blog_id= ?))" ) );
        for ( int i = 0; i < cat_count; ++i ) {
            q2.addBindValue(localId);
            q2.addBindValue(post.categories()[i]);
            q2.addBindValue(blog_id);
            if ( !q2.exec() ) {
                d->mLastErrorText = q.lastError().text();
                kDebug() << "Cannot add one of categories to Post, SQL Error: " << q2.lastError().text();
            }
//             kDebug()<<"category "<<post.categories()[i] <<" added.";
        }
    }
    return localId;
}

bool DBMan::removeLocalEntry( const BilboPost &post )
{
    kDebug();
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM local_post WHERE local_id=?") );
    q.addBindValue( post.localId() );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << d->mLastErrorText;
    }
    return res;
}

bool DBMan::removeLocalEntry( int local_id )
{
    kDebug();
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM local_post WHERE local_id=?") );
    q.addBindValue( local_id );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << d->mLastErrorText;
    }
    return res;
}

bool DBMan::removeTempEntry( const BilboPost &post )
{
    kDebug();
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM temp_post WHERE local_id=?") );
    q.addBindValue( post.localId() );
    bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    kDebug()<<"Id: "<<post.id()<<"\tStatus: "<<post.status();
    return res;
}

bool DBMan::clearTempEntries()
{
    kDebug();
    QSqlQuery q;
    bool res = q.exec( QLatin1String("DELETE FROM temp_post") );
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

QMap<QString, QString> DBMan::getAuthData(int blog_id)
{
    kDebug() << blog_id;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT key, value FROM auth_data WHERE blog_id = ?") );
    q.addBindValue( blog_id );
    if ( !q.exec() ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
        return QMap<QString, QString>();
    }

    QMap<QString, QString> result;
    while ( q.next() ) {
        result[q.value(0).toString() ] = q.value(1).toString();
    }

    kDebug() << blog_id << result;
    return result;
}

bool DBMan::saveAuthData( const QMap<QString, QString> &authData, int blog_id )
{
    kDebug() << blog_id;
    QSqlQuery q;
    q.prepare( QLatin1String("INSERT OR REPLACE INTO auth_data (blog_id, key, value) VALUES (?, ?, ?)") );
    QMap<QString, QString>::ConstIterator iter, end = authData.constEnd();
    QList<QVariant> ids, keys, values;
    for ( iter = authData.constBegin(); iter != end; ++iter ) {
        ids << blog_id;
        keys << iter.key();
        values << iter.value();
    }
    q.addBindValue( ids );
    q.addBindValue( keys );
    q.addBindValue( values );
    const bool res = q.execBatch();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}

bool DBMan::clearAuthData( int blog_id )
{
    kDebug() << blog_id;
    QSqlQuery q;
    q.prepare( QLatin1String("DELETE FROM auth_data WHERE blog_id = ?") );
    q.addBindValue( blog_id );
    const bool res = q.exec();
    if ( !res ) {
        d->mLastErrorText = q.lastError().text();
        kDebug() << q.lastError().text();
    }
    return res;
}



BilboBlog *DBMan::blog(int blog_id)
{
    return blogList().value(blog_id);
}

QList< BilboBlog *> DBMan::listBlogs()
{
    QList<BilboBlog *> list;
    QSqlQuery q;
    if (q.exec( QLatin1String("SELECT id, blogid, blog_url, username, style_url, api_type, title,\
            direction, local_directory, password FROM blog") ) ) {
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
            if( d->useWallet ) {
                QString buffer;
                if ( d->mWallet && d->mWallet->readPassword( tmp->url().url() + QLatin1Char('_') + tmp->username() , buffer )
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
        d->mLastErrorText = q.lastError().text();
    }
    return list;
}

QMap< QString, int > DBMan::listBlogsTitle()
{
    QMap< QString, int > list;
    QSqlQuery q;
    if( q.exec( QLatin1String("SELECT title, id FROM blog") ) ) {
        while ( q.next() ) {
            list[q.value( 0 ).toString()] = q.value( 1 ).toInt();
        }
    } else {
        d->mLastErrorText = q.lastError().text();
    }
    return list;
}

QList< BilboPost* > DBMan::listPosts( int blog_id )
{
    QList<BilboPost *> list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT id, postid, author, title, content, c_time, m_time, is_private, is_comment_allowed,\
               is_trackback_allowed, link, perma_link, summary, tags, status, text_more, slug\
               FROM post WHERE blog_id = ? ORDER BY c_time DESC") );
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
            tmp->setTags( q.value( 13 ).toString().split( QLatin1Char(','), QString::SkipEmptyParts ) );
            tmp->setStatus(( KBlog::BlogPost::Status ) q.value( 14 ).toInt() );
            tmp->setAdditionalContent( q.value( 15 ).toString() );
            tmp->setSlug( q.value( 16 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( QLatin1String("SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
                        category.categoryId, category.parentId\
                        FROM category JOIN post_cat ON category.categoryId=post_cat.categoryId\
                        WHERE post_cat.postId = ? AND post_cat.blogId = (SELECT blogid FROM blog where id=?)" ) );
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
                d->mLastErrorText = q2.lastError().text();
            }
            tmp->setCategoryList( catList );
            list.append( tmp );
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id << "error: "<< d->mLastErrorText;
    }
    return list;
}

BilboPost DBMan::getPostInfo( int post_id )
{
    QSqlQuery q;
    BilboPost tmp;
    q.prepare( QLatin1String("SELECT id, postid, author, title, content, c_time, m_time, is_private, is_comment_allowed,\
               is_trackback_allowed, link, perma_link, summary, tags, status, blog_id, text_more, slug\
               FROM post WHERE id = ?") );
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
            tmp.setTags( q.value( 13 ).toString().split( QLatin1Char(','), QString::SkipEmptyParts ) );
            tmp.setStatus(( KBlog::BlogPost::Status ) q.value( 14 ).toInt() );
            int blog_id = q.value( 15 ).toInt();
            tmp.setAdditionalContent(  q.value( 16 ).toString() );
            tmp.setSlug(  q.value( 17 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( QLatin1String("SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
                        category.categoryId, category.parentId, category.blog_id\
                        FROM category JOIN post_cat ON category.categoryId=post_cat.categoryId\
                        WHERE post_cat.postId = ? AND post_cat.blogId = (SELECT blogid FROM blog where id=?)") );
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
                d->mLastErrorText = q2.lastError().text();
            }
            tmp.setCategoryList( catList );
        } else {
            d->mLastErrorText = i18n( "There is no post with the requested ID" );
            kDebug() << "There isn't any post with id: " << post_id;
            tmp.setStatus(KBlog::BlogPost::Error);
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get post with id " << post_id;
        tmp.setStatus(KBlog::BlogPost::Error);
    }

    return tmp;
}

QMap< int, QString > DBMan::listPostsTitle( int blog_id )
{
    QMap< int, QString >list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT title, id FROM post WHERE blog_id = ? ORDER BY c_time DESC") );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list.insert( q.value( 1 ).toInt(), q.value( 0 ).toString() );
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id;
    }
    return list;
}

QList<QVariantMap> DBMan::listPostsInfo( int blog_id )
{
    QList<QVariantMap> list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT title, id, c_time, is_private FROM post WHERE blog_id = ? ORDER BY c_time DESC") );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            QVariantMap entry;
            entry[ QLatin1String("title") ] = q.value( 0 ).toString();
            entry[ QLatin1String("id") ] = q.value( 1 ).toInt();
            entry[ QLatin1String("c_time") ] = KDateTime::fromString( q.value( 2 ).toString() ).dateTime();
            entry[ QLatin1String("is_private") ] = q.value( 3 ).toBool();
            list.append(entry);
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of posts for blog with id " << blog_id;
    }
    return list;
}

QMap< QString, int > DBMan::listCategoriesName( int blog_id )
{
    QMap< QString, int > list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT name, catid FROM category WHERE blog_id = ?") );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list[q.value( 0 ).toString()] = q.value( 1 ).toInt();
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QList< Category > DBMan::listCategories( int blog_id )
{
    QList< Category > list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT catid, name, description, htmlUrl, rssUrl, categoryId, parentId FROM category\
               WHERE blog_id = ?") );
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
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QMap< QString, bool > DBMan::listCategoriesId( int blog_id )
{
    QMap< QString, bool > list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT categoryId FROM category\
               WHERE blog_id = ?") );
    q.addBindValue( blog_id );
    if ( q.exec() ) {
        while ( q.next() ) {
            list.insert( q.value( 0 ).toString(), false );
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of categories for blog with id " << blog_id;
    }
    return list;
}

QMap<BilboPost*, int> DBMan::listTempPosts()
{
    QMap<BilboPost*, int> list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT local_id, id, postid, blog_id, author, title, content, text_more, c_time,\
    m_time, is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, tags, status,\
    slug FROM temp_post ORDER BY m_time DESC") );
    if ( q.exec() ) {
        while ( q.next() ) {
            BilboPost *tmp = new BilboPost();
            tmp->setLocalId( q.value( 0 ).toInt() );
            tmp->setId( q.value( 1 ).toInt() );
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
            tmp->setTags( q.value( 16 ).toString().split( QLatin1Char(','), QString::SkipEmptyParts ) );
            tmp->setStatus(( KBlog::BlogPost::Status ) q.value( 17 ).toInt() );
            tmp->setSlug( q.value( 18 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( QLatin1String("SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
            category.categoryId, category.parentId\
            FROM category JOIN temp_post_cat ON category.categoryId=temp_post_cat.categoryId\
            WHERE temp_post_cat.local_id = ?") );
            q2.addBindValue( tmp->localId() );
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
                d->mLastErrorText = q2.lastError().text();
                kDebug()<<"Cannot get categories list of a post. SQL Error: "<< q2.lastError().text();
            }
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of temporary posts, SQL Error: "<< q.lastError().text();
    }
    return list;
}

QList<QVariantMap> DBMan::listLocalPosts()
{
    kDebug();
    QList<QVariantMap> list;
    QSqlQuery q;
    q.prepare( QLatin1String("SELECT local_post.local_id, local_post.title, local_post.blog_id, blog.title\
    FROM local_post LEFT JOIN blog ON local_post.blog_id = blog.id ORDER BY m_time DESC") );
    if ( q.exec() ) {
        while ( q.next() ) {
            QVariantMap entry;
            entry[ QLatin1String("local_id") ] = q.value( 0 ).toInt();
            entry[ QLatin1String("post_title") ] = q.value( 1 ).toString();
            entry[ QLatin1String("blog_id") ] = q.value( 2 ).toInt();
            entry[ QLatin1String("blog_title") ] = q.value( 3 ).toString();
            list.append(entry);
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get list of local posts. SQL Error: "<< q.lastError().text();
    }
    return list;
}

BilboPost DBMan::localPost(int local_id)
{
    QSqlQuery q;
    BilboPost tmp;
    q.prepare( QLatin1String("SELECT id, local_id, postid, blog_id, author, title, content, text_more, c_time,\
    m_time, is_private, is_comment_allowed, is_trackback_allowed, link, perma_link, summary, tags, status,\
    slug FROM local_post WHERE local_id=?") );
    q.addBindValue(local_id);
    if ( q.exec() ) {
        if ( q.next() ) {
            tmp.setId( q.value( 0 ).toInt() );
            tmp.setLocalId( q.value( 1 ).toInt() );
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
            tmp.setTags( q.value( 16 ).toString().split( QLatin1Char(','), QString::SkipEmptyParts ) );
            tmp.setStatus(( KBlog::BlogPost::Status ) q.value( 17 ).toInt() );
            tmp.setSlug( q.value( 18 ).toString() );

            ///get Category list:
            QList<Category> catList;
            QSqlQuery q2;
            q2.prepare( QLatin1String("SELECT category.name, category.description, category.htmlUrl, category.rssUrl,\
            category.categoryId, category.parentId\
            FROM category JOIN local_post_cat ON category.categoryId=local_post_cat.categoryId\
            WHERE local_post_cat.local_id = ?") );
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
                d->mLastErrorText = q2.lastError().text();
                kDebug()<<"Cannot get categories list of local post. SQL Error: "<< q2.lastError().text();
            }
        } else {
            d->mLastErrorText = i18n( "There is no local post with the requested ID " );
            kDebug()<<"there isn't any local post with local_id "<<local_id;
        }
    } else {
        d->mLastErrorText = q.lastError().text();
        kDebug() << "Cannot get local post. SQL Error: "<< q.lastError().text();
    }
    return tmp;
}
