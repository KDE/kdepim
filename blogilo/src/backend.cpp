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

#include "backend.h"
#include "bilboblog.h"
#include "bilbopost.h"
#include "bilbomedia.h"
#include "dbman.h"

#include <kurl.h>
#include <kblog/blogger1.h>
#include <kblog/gdata.h>
#include <kblog/metaweblog.h>
#include <kblog/movabletype.h>
#include <kblog/wordpressbuggy.h>
#include <kblog/blogmedia.h>
#include <kdebug.h>
#include <KDE/KLocale>
// #include <QMimeData>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <settings.h>

Backend::Backend( int blog_id, QObject* parent ): QObject( parent )
{
    kDebug() << "with blog id: " << blog_id;
    mBBlog = new BilboBlog( DBMan::self()->blog( blog_id ) );
    switch ( mBBlog->api() ) {
        case BilboBlog::BLOGGER1_API:
            mKBlog = new KBlog::Blogger1( KUrl(), this );
            break;
        case BilboBlog::GDATA_API:
            mKBlog = new KBlog::GData( KUrl(), this );
            break;
        case BilboBlog::METAWEBLOG_API:
            mKBlog = new KBlog::MetaWeblog( KUrl(), this );
            break;
        case BilboBlog::MOVABLETYPE_API:
            mKBlog = new KBlog::MovableType( KUrl(), this );
            break;
        case BilboBlog::WORDPRESSBUGGY_API:
            mKBlog = new KBlog::WordpressBuggy( KUrl(), this );
    }
    mKBlog->setUserAgent(APPNAME, VERSION);
    mKBlog->setUsername( mBBlog->username() );
    mKBlog->setPassword( mBBlog->password() );
    mKBlog->setUrl( KUrl( mBBlog->url() ) );
    mKBlog->setBlogId( mBBlog->blogid() );
    categoryListNotSet = false;

    connect( mKBlog, SIGNAL( error( KBlog::Blog::ErrorType, const QString& ) ),
             this, SLOT( error( KBlog::Blog::ErrorType, const QString& ) ) );
    connect( mKBlog, SIGNAL( errorPost( KBlog::Blog::ErrorType, const QString &, KBlog::BlogPost* ) ),
             this, SLOT( error( KBlog::Blog::ErrorType, const QString& ) ) );
    connect( mKBlog, SIGNAL( errorComment( KBlog::Blog::ErrorType, const QString &, KBlog::BlogPost*,
                                           KBlog::BlogComment* ) ),
             this, SLOT( error( KBlog::Blog::ErrorType, const QString& ) ) );
    connect( mKBlog, SIGNAL( errorMedia( KBlog::Blog::ErrorType, const QString &, KBlog::BlogMedia* ) ),
             this, SLOT( error( KBlog::Blog::ErrorType, const QString& ) ) );
}

Backend::~Backend()
{
    kDebug();
    mBBlog->deleteLater();
}

void Backend::getCategoryListFromServer()
{
    kDebug() << "Blog Id: " << mBBlog->id();
    if ( mBBlog->api() == BilboBlog::METAWEBLOG_API || mBBlog->api() == BilboBlog::MOVABLETYPE_API ||
         mBBlog->api() == BilboBlog::WORDPRESSBUGGY_API ) {
        KBlog::MetaWeblog *tmp = dynamic_cast<KBlog::MetaWeblog*>( mKBlog );
        connect( tmp, SIGNAL( listedCategories( const QList< QMap< QString, QString > > & ) ),
                 this, SLOT( categoriesListed( const QList< QMap< QString, QString > > & ) ) );
        tmp->listCategories();
    } else {
        char err[] = "Blog API doesn't support getting Category list.";
        kDebug() << err;
        QString tmp = i18n( err );
        error( KBlog::Blog::NotSupported, tmp );
    }
}

void Backend::categoriesListed( const QList< QMap < QString , QString > > & categories )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    DBMan::self()->clearCategories( mBBlog->id() );

    for ( int i = 0; i < categories.count(); ++i ) {
        QString name, description, htmlUrl, rssUrl, categoryId, parentId;
        const QMap<QString, QString> &category = categories.at( i );

        name = category.value( "name", QString() );
        description = category.value( "description", QString() );
        htmlUrl = category.value( "htmlUrl", QString() );
        rssUrl = category.value( "rssUrl", QString() );
        categoryId = category.value( "categoryId", QString() );
        parentId = category.value( "parentId", QString() );

        if(categoryId.isEmpty()) {
            categoryId = QString::number(i);
        }

        DBMan::self()->addCategory( name, description, htmlUrl, rssUrl, categoryId, parentId, mBBlog->id() );
    }
    kDebug() << "Emitting sigCategoryListFetched...";
    Q_EMIT sigCategoryListFetched( mBBlog->id() );
}

void Backend::getEntriesListFromServer( int count )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    connect( mKBlog, SIGNAL( listedRecentPosts( const QList<KBlog::BlogPost> & ) ),
             this, SLOT( entriesListed( const QList<KBlog::BlogPost >& ) ) );
    mKBlog->listRecentPosts( count );
}

void Backend::entriesListed( const QList< KBlog::BlogPost > & posts )
{
    kDebug() << "Blog Id: " << mBBlog->id();
//     DBMan::self()->clearPosts( mBBlog->id() );

    for ( int i = 0; i < posts.count(); i++ ) {
        BilboPost tempPost( posts[i] );
        if(Settings::changeNToBreak()) {
            tempPost.setContent( tempPost.content().replace( '\n', "<br/>" ) );
            tempPost.setAdditionalContent( tempPost.additionalContent().replace( '\n', "<br/>" ) );
        }
        DBMan::self()->addPost( tempPost, mBBlog->id() );
    }
    kDebug() << "Emitting sigEntriesListFetched ...";
    Q_EMIT sigEntriesListFetched( mBBlog->id() );
}

void Backend::publishPost( const BilboPost &post )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    BilboPost tmpPost = post;
    if( Settings::addPoweredBy() ) {
        QString poweredStr = "<p>=-=-=-=-=<br/>"
        "<i>Powered by <b><a href='http://blogilo.gnufolks.org/'>Blogilo</a></b></i></p>";
        tmpPost.setContent(post.content() + poweredStr);
    }
    KBlog::BlogPost *bp = preparePost( tmpPost );
    connect( mKBlog, SIGNAL( createdPost( KBlog::BlogPost * ) ),
             this, SLOT( postPublished( KBlog::BlogPost * ) ) );
    mKBlog->createPost( bp );
}

void Backend::postPublished( KBlog::BlogPost *post )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    if ( post->status() == KBlog::BlogPost::Error ) {
        kDebug() << "Publishing/Modifying Failed";
        const QString tmp( i18n( "Publishing/Modifying post failed: %1", post->error() ) );
        kDebug() << "Emitting sigError...";
        Q_EMIT sigError( tmp );
        return;
    }
//     if ( categoryListNotSet ) {
//         categoryListNotSet = false;
//         mSetPostCategoriesMap[ post->postId()] = post;
//         QMap<QString, bool> cats;
//         int count = mCreatePostCategories.count();
//         for ( int i = 0; i < count; ++i ) {
//             cats.insert( mCreatePostCategories[i].categoryId, false );
//         }
//         setPostCategories( post->postId(), cats );
//     } else {
        kDebug()<<"isPrivate: "<<post->isPrivate();
        mSubmitPostStatusMap[ post ] = post->status();
        connect( mKBlog, SIGNAL( fetchedPost(KBlog::BlogPost*)),
                 this, SLOT( savePostInDbAndEmitResult(KBlog::BlogPost*)) );
        mKBlog->fetchPost( post );
//     }
}

void Backend::uploadMedia( BilboMedia * media )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    QString tmp;
    switch ( mBBlog->api() ) {
        case BilboBlog::BLOGGER1_API:
        case BilboBlog::GDATA_API:
            kDebug() << "The Blogger1 and GData API type doesn't support uploading Media files.";
            tmp = i18n( "Uploading media failed: Your Blog API does not support uploading media objects.");
            kDebug() << "Emitting sigError...";
            Q_EMIT sigMediaError( tmp, media );
            return;
            break;
        case BilboBlog::METAWEBLOG_API:
        case BilboBlog::MOVABLETYPE_API:
        case BilboBlog::WORDPRESSBUGGY_API:
            KBlog::BlogMedia *m = new KBlog::BlogMedia() ;
            KBlog::MetaWeblog *MWBlog = qobject_cast<KBlog::MetaWeblog*>( mKBlog );

            m->setMimetype( media->mimeType() );

            QByteArray data;
            KIO::TransferJob *job = KIO::get( media->localUrl(), KIO::Reload, KIO::HideProgressInfo);
            if( !KIO::NetAccess::synchronousRun(job, 0, &data) ){
                kError()<<"Job error: " << job->errorString();
                tmp = i18n( "Uploading media failed: Cannot read the media file,\
 please check if it exists. Path: %1", media->localUrl().pathOrUrl() );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigMediaError( tmp, media );
            }

            if ( data.count() == 0 ) {
                kError() << "Cannot read the media file, please check if it exists.";
                tmp = i18n( "Uploading media failed: Cannot read the media file,\
 please check if it exists. Path: %1", media->localUrl().pathOrUrl() );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigMediaError( tmp, media );
                return;
            }

            m->setData( data );
            m->setName( media->name() );

            media->setCheckSum( qChecksum( data.data(), data.count() ) );

            if ( media->checksum() == 0 ) {
                kError() << "Media file checksum is zero";
                tmp = i18n( "Uploading media failed: Media file checksum is zero, please check file path. Path: %1",
                                         media->localUrl().pathOrUrl() );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigMediaError( tmp, media );
                return;
            }

            if ( !MWBlog ) {
                kError() << "MWBlog is NULL: casting has not worked, this should NEVER happen, has the gui allowed using GDATA?";
                tmp = i18n( "INTERNAL ERROR: MWBlog is NULL: casting has not worked, this should NEVER happen." );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigError( tmp );
                return;
            }
            mPublishMediaMap[ m ] = media;
            connect( MWBlog, SIGNAL( createdMedia( KBlog::BlogMedia* ) ), this, SLOT( mediaUploaded( KBlog::BlogMedia* ) ) );
            connect( MWBlog, SIGNAL( errorMedia( KBlog::Blog::ErrorType, const QString &, KBlog::BlogMedia* ) ),
                     this, SLOT( sltMediaError( KBlog::Blog::ErrorType, const QString &, KBlog::BlogMedia* ) ) );
            MWBlog->createMedia( m );
            return;
            break;
    }
    kError() << "Api type isn't set correctly!";
    tmp = i18n( "API type is not set correctly!" );
    Q_EMIT sigError( tmp );
}

void Backend::mediaUploaded( KBlog::BlogMedia * media )
{
    kDebug() << "Blog Id: " << mBBlog->id() << "Media: "<<media->url();
    if(!media){
        kError()<<"ERROR! Media returned from KBlog is NULL!";
        return;
    }
    BilboMedia * m = mPublishMediaMap.value( media );
    if(!m){
        kError()<<"ERROR! Media returned from KBlog doesn't exist on the Map! Url is:"
                << media->url();
        return;
    }
    mPublishMediaMap.remove( media );
    if ( media->status() == KBlog::BlogMedia::Error ) {
        kError() << "Upload error! with this message: " << media->error();
        const QString tmp( i18n( "Uploading media failed: %1", media->error() ) );
        kDebug() << "Emitting sigMediaError ...";
        Q_EMIT sigMediaError( tmp, m );
        return;
    }
    quint16 newChecksum = qChecksum( media->data().data(), media->data().count() );
    if ( newChecksum != m->checksum() ) {
        kError() << "Check sum error: checksum of sent file: " << m->checksum() <<
                " Checksum of received file: " << newChecksum << "Error: " << media->error() << endl;
        const QString tmp( i18n( "Uploading media failed: Checksum error. Returned error: %1",
                           media->error() ) );
        kDebug() << "Emitting sigMediaError ...";
        Q_EMIT sigMediaError( tmp, m );
        return;
    }
    m->setRemoteUrl( QUrl( media->url().url() ).toString() );
    m->setUploaded( true );
    kDebug() << "Emitting sigMediaUploaded...";
    Q_EMIT sigMediaUploaded( m );
}

void Backend::modifyPost( const BilboPost &post )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    BilboPost tmpPost = post;
    KBlog::BlogPost *bp = preparePost( tmpPost );
    connect( mKBlog, SIGNAL( modifiedPost(KBlog::BlogPost*)),
             this, SLOT( postPublished(KBlog::BlogPost*)) );
    mKBlog->modifyPost( bp );
}

void Backend::removePost( BilboPost &post )
{
    kDebug() << "Blog Id: " << mBBlog->id();

    KBlog::BlogPost *bp = post.toKBlogPost();
    connect( mKBlog, SIGNAL( removedPost(KBlog::BlogPost*)),
             this, SLOT( slotPostRemoved(KBlog::BlogPost*)) );
    mKBlog->removePost( bp );
}

void Backend::slotPostRemoved( KBlog::BlogPost *post )
{
    if(!post) {
        kDebug()<<"post returned from server is NULL";
        return;
    }
    if( !DBMan::self()->removePost(mBBlog->id(), post->postId()) ) {
        kDebug()<<"cannot remove post from database, error: "<<DBMan::self()->lastErrorText();
    }
    emit sigPostRemoved(mBBlog->id(), BilboPost(*post));
}

void Backend::fetchPost( BilboPost &post )
{
    KBlog::BlogPost *bp = post.toKBlogPost();
    connect( mKBlog, SIGNAL( fetchedPost(KBlog::BlogPost*)),
             this, SLOT( slotPostFetched(KBlog::BlogPost*)) );
    mKBlog->fetchPost( bp );
}

void Backend::slotPostFetched( KBlog::BlogPost *post )
{
    emit sigPostFetched( new BilboPost(*post) );
    delete post;
}

void Backend::error( KBlog::Blog::ErrorType type, const QString & errorMessage )
{
    kDebug() << "Blog Id: " << mBBlog->id();
    QString errType = errorTypeToString( type );
    errType += errorMessage;
    kDebug() << errType;
    kDebug() << "Emitting sigError";
    Q_EMIT sigError( errType );
}

void Backend::sltMediaError( KBlog::Blog::ErrorType type, const QString & errorMessage, KBlog::BlogMedia * media )
{
    kDebug();
    QString errType = errorTypeToString( type );
    errType += errorMessage;
    kDebug() << errType;
    kDebug() << "Emitting sigMediaError ...";
    emit sigMediaError( errorMessage, mPublishMediaMap[ media ] );
    mPublishMediaMap.remove( media );
}

QString Backend::errorTypeToString( KBlog::Blog::ErrorType type )
{
    QString errType;
    switch ( type ) {
        case KBlog::Blog::XmlRpc:
            errType = i18n( "Server (XMLRPC) error: " );
            break;
        case KBlog::Blog::Atom:
            errType = i18n( "Server (Atom) error: " );
            break;
        case KBlog::Blog::ParsingError:
            errType = i18n( "Parsing error: " );
            break;
        case KBlog::Blog::AuthenticationError:
            errType = i18n( "Authentication error: " );
            break;
        case KBlog::Blog::NotSupported:
            errType = i18n( "Not supported error: " );
            break;
        default:
            errType = i18n( "Unknown error: " );
    };
    return errType;
}

void Backend::savePostInDbAndEmitResult( KBlog::BlogPost *post )
{
    if(!post) {
        kError()<<"ERROR: post is NULL ";
        Q_EMIT sigError( "post is NULL" );
        return;
    }
    kDebug()<<"isPrivate: "<<post->isPrivate();
    BilboPost *pp = new BilboPost( *post );
    int post_id;
    if( mSubmitPostStatusMap[ post ] == KBlog::BlogPost::Modified) {
        post_id = DBMan::self()->editPost( *pp, mBBlog->id() );
    } else {
        post_id = DBMan::self()->addPost( *pp, mBBlog->id() );
    }
    mSubmitPostStatusMap.remove(post);
    if ( post_id != -1 ) {
        pp->setPrivate( post->isPrivate() );
        pp->setId( post_id );
        kDebug() << "Emitting sigPostPublished ...";
        Q_EMIT sigPostPublished( mBBlog->id(), pp );
    }
    delete post;
}

KBlog::BlogPost * Backend::preparePost( BilboPost &post )
{
    post.setContent( post.content().remove('\n') );
    post.setAdditionalContent( post.additionalContent().remove( '\n' ) );
    if ( mBBlog->api() == BilboBlog::MOVABLETYPE_API || mBBlog->api() == BilboBlog::WORDPRESSBUGGY_API ) {
        QStringList content = post.content().split("<!--split-->");
        if( content.count() == 2 ) {
            post.setContent(content[0]);
            post.setAdditionalContent( content[1] );
        }
    }
//     if( mBBlog->api() == BilboBlog::MOVABLETYPE_API && post.categoryList().count() > 0 ) {
//         mCreatePostCategories = post.categoryList();
//         categoryListNotSet = true;
//     }
    return post.toKBlogPost();
}

#include "backend.moc"
