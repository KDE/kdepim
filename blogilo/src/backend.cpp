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

#include "backend.h"
#include "bilboblog.h"
#include "bilbopost.h"
#include "bilbomedia.h"
#include "dbman.h"
#include "settings.h"

#include <kurl.h>
#include <KBlog/blogger1.h>
#include <KBlog/gdata.h>
#include <KBlog/metaweblog.h>
#include <KBlog/movabletype.h>
#include <KBlog/wordpressbuggy.h>
#include <KBlog/blogmedia.h>
#include <kdebug.h>
#include <KLocale>

#include <kio/netaccess.h>
#include <kio/job.h>

const QRegExp splitRX(QLatin1String("((<hr/?>)?<!--split-->)"));

class Backend::Private
{
public:
    Private()
    :kBlog(0), bBlog(0), categoryListNotSet(false)
    {}
    KBlog::Blog *kBlog;
    BilboBlog *bBlog;
    QList<Category> mCreatePostCategories;
    QMap<QString, KBlog::BlogPost *> mSetPostCategoriesMap;
    QMap<KBlog::BlogPost *, BilboPost::Status> mSubmitPostStatusMap;
    QMap<KBlog::BlogMedia *, BilboMedia *> mPublishMediaMap;
    bool categoryListNotSet;
};

Backend::Backend( int blog_id, QObject* parent )
    : QObject( parent ), d(new Private)
{
    kDebug() << "with blog id: " << blog_id;
    d->bBlog = DBMan::self()->blog( blog_id );
    d->kBlog = d->bBlog->blogBackend();

    connect( d->kBlog, SIGNAL(error(KBlog::Blog::ErrorType,QString)),
             this, SLOT(error(KBlog::Blog::ErrorType,QString)) );
    connect( d->kBlog, SIGNAL(errorPost(KBlog::Blog::ErrorType,QString,KBlog::BlogPost*)),
             this, SLOT(error(KBlog::Blog::ErrorType,QString)) );
    connect( d->kBlog, SIGNAL( errorComment( KBlog::Blog::ErrorType, const QString &, KBlog::BlogPost*,
                                           KBlog::BlogComment* ) ),
             this, SLOT(error(KBlog::Blog::ErrorType,QString)) );
    connect( d->kBlog, SIGNAL(errorMedia(KBlog::Blog::ErrorType,QString,KBlog::BlogMedia*)),
             this, SLOT(error(KBlog::Blog::ErrorType,QString)) );
}

Backend::~Backend()
{
    kDebug();
    delete d;
}

void Backend::getCategoryListFromServer()
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    if ( d->bBlog->api() == BilboBlog::METAWEBLOG_API ||
         d->bBlog->api() == BilboBlog::MOVABLETYPE_API ||
         d->bBlog->api() == BilboBlog::WORDPRESSBUGGY_API ) {
        KBlog::MetaWeblog *tmp = static_cast<KBlog::MetaWeblog*>( d->kBlog );
        connect( tmp, SIGNAL(listedCategories(QList<QMap<QString,QString> >)),
                 this, SLOT(categoriesListed(QList<QMap<QString,QString> >)) );
        tmp->listCategories();
    } else {
        error( KBlog::Blog::NotSupported, i18n( "Blog API doesn't support getting Category list." ) );
    }
}

void Backend::categoriesListed( const QList< QMap < QString , QString > > & categories )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    DBMan::self()->clearCategories( d->bBlog->id() );

    const int categoriesCount(categories.count());
    for ( int i = 0; i < categoriesCount; ++i ) {
        const QMap<QString, QString> &category = categories.at( i );

        const QString name = category.value( QLatin1String("name"), QString() );
        const QString description = category.value( QLatin1String("description"), QString() );
        const QString htmlUrl = category.value( QLatin1String("htmlUrl"), QString() );
        const QString rssUrl = category.value( QLatin1String("rssUrl"), QString() );
        QString categoryId = category.value( QLatin1String("categoryId"), QString() );
        const QString parentId = category.value( QLatin1String("parentId"), QString() );

        if(categoryId.isEmpty()) {
            categoryId = QString::number(i);
        }

        DBMan::self()->addCategory( name, description, htmlUrl, rssUrl, categoryId, parentId, d->bBlog->id() );
    }
    kDebug() << "Emitting sigCategoryListFetched...";
    Q_EMIT sigCategoryListFetched( d->bBlog->id() );
}

void Backend::getEntriesListFromServer( int count )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    connect( d->kBlog, SIGNAL(listedRecentPosts(QList<KBlog::BlogPost>)),
             this, SLOT(entriesListed(QList<KBlog::BlogPost>)) );
    d->kBlog->listRecentPosts( count );
}

void Backend::entriesListed( const QList< KBlog::BlogPost > & posts )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
//     DBMan::self()->clearPosts( d->bBlog->id() );

    const int postCount(posts.count());
    for ( int i = 0; i < postCount; ++i ) {
        BilboPost tempPost( posts.at(i) );
        if(Settings::changeNToBreak()) {
            tempPost.setContent( tempPost.content().replace( QLatin1Char('\n'), QLatin1String("<br/>") ) );
            tempPost.setAdditionalContent( tempPost.additionalContent().replace( QLatin1Char('\n'), QLatin1String("<br/>") ) );
        }
        DBMan::self()->addPost( tempPost, d->bBlog->id() );
    }
    kDebug() << "Emitting sigEntriesListFetched ...";
    Q_EMIT sigEntriesListFetched( d->bBlog->id() );
}

void Backend::publishPost( BilboPost* post )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
//     BilboPost tmpPost = post;
    if( Settings::addPoweredBy() ) {
        QString poweredStr = QLatin1String("<p>=-=-=-=-=<br/>"
        "<i>Powered by <b><a href='http://blogilo.gnufolks.org/'>Blogilo</a></b></i></p>");
        post->setContent(post->content() + poweredStr);
    }
    preparePost( post );
    connect( d->kBlog, SIGNAL(createdPost(KBlog::BlogPost*)),
             this, SLOT(postPublished(KBlog::BlogPost*)) );
    d->kBlog->createPost( post );
}

void Backend::postPublished( KBlog::BlogPost *post )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    if ( post->status() == KBlog::BlogPost::Error ) {
        kDebug() << "Publishing/Modifying Failed";
        const QString tmp( i18n( "Publishing/Modifying post failed: %1", post->error() ) );
        kDebug() << "Emitting sigError...";
        Q_EMIT sigError( tmp );
        return;
    }
    kDebug()<<"isPrivate: "<<post->isPrivate();
    if(post->isPrivate() && d->bBlog->api() == BilboBlog::GDATA_API){
        //GData do not support fetching drafts!
        savePostInDbAndEmitResult(post);
        return;
    }
    d->mSubmitPostStatusMap[ post ] = post->status();
    connect( d->kBlog, SIGNAL(fetchedPost(KBlog::BlogPost*)),
             this, SLOT(savePostInDbAndEmitResult(KBlog::BlogPost*)) );
    d->kBlog->fetchPost( post );
}

void Backend::uploadMedia( BilboMedia * media )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    QString tmp;
    switch ( d->bBlog->api() ) {
        case BilboBlog::BLOGGER1_API:
        case BilboBlog::GDATA_API:
            kDebug() << "The Blogger1 and GData API type doesn't support uploading Media files.";
            tmp = i18n( "Uploading media failed: Your Blog API does not support uploading media objects.");
            kDebug() << "Emitting sigError...";
            Q_EMIT sigMediaError( tmp, media );
            return;
        case BilboBlog::METAWEBLOG_API:
        case BilboBlog::MOVABLETYPE_API:
        case BilboBlog::WORDPRESSBUGGY_API:
            KBlog::BlogMedia *m = new KBlog::BlogMedia() ;
            KBlog::MetaWeblog *MWBlog = qobject_cast<KBlog::MetaWeblog*>( d->kBlog );

            m->setMimetype( media->mimeType() );

            QByteArray data;
            KIO::TransferJob *job = KIO::get( media->localUrl(), KIO::Reload, KIO::HideProgressInfo);
            if( !KIO::NetAccess::synchronousRun(job, 0, &data) ){
                kError()<<"Job error: " << job->errorString();
                tmp = i18n( "Uploading media failed: Cannot read the media file, please check if it exists. Path: %1", media->localUrl().pathOrUrl() );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigMediaError( tmp, media );
            }

            if ( data.count() == 0 ) {
                kError() << "Cannot read the media file, please check if it exists.";
                tmp = i18n( "Uploading media failed: Cannot read the media file, please check if it exists. Path: %1", media->localUrl().pathOrUrl() );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigMediaError( tmp, media );
                delete m;
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
                delete m;
                return;
            }

            if ( !MWBlog ) {
                kError() << "MWBlog is NULL: casting has not worked, this should NEVER happen, has the gui allowed using GDATA?";
                tmp = i18n( "INTERNAL ERROR: MWBlog is NULL: casting has not worked, this should NEVER happen." );
                kDebug() << "Emitting sigError...";
                Q_EMIT sigError( tmp );
                delete m;
                return;
            }
            d->mPublishMediaMap[ m ] = media;
            connect( MWBlog, SIGNAL(createdMedia(KBlog::BlogMedia*)), this, SLOT(mediaUploaded(KBlog::BlogMedia*)) );
            connect( MWBlog, SIGNAL(errorMedia(KBlog::Blog::ErrorType,QString,KBlog::BlogMedia*)),
                     this, SLOT(slotMediaError(KBlog::Blog::ErrorType,QString,KBlog::BlogMedia*)) );
            MWBlog->createMedia( m );
            return;
    }
    kError() << "Api type isn't set correctly!";
    tmp = i18n( "API type is not set correctly." );
    Q_EMIT sigError( tmp );
}

void Backend::mediaUploaded( KBlog::BlogMedia * media )
{
    kDebug() << "Blog Id: " << d->bBlog->id() << "Media: "<<media->url();
    if(!media){
        kError()<<"ERROR! Media returned from KBlog is NULL!";
        return;
    }
    BilboMedia * m = d->mPublishMediaMap.value( media );
    if(!m){
        kError()<<"ERROR! Media returned from KBlog doesn't exist on the Map! Url is:"
                << media->url();
        return;
    }
    d->mPublishMediaMap.remove( media );
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

void Backend::modifyPost( BilboPost* post )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
//     BilboPost tmpPost = post;
    preparePost( post );
    connect( d->kBlog, SIGNAL(modifiedPost(KBlog::BlogPost*)),
             this, SLOT(postPublished(KBlog::BlogPost*)) );
    d->kBlog->modifyPost( post );
}

void Backend::removePost( BilboPost* post )
{
    kDebug() << "Blog Id: " << d->bBlog->id();

//     KBlog::BlogPost *bp = post.toKBlogPost();
    connect( d->kBlog, SIGNAL(removedPost(KBlog::BlogPost*)),
             this, SLOT(slotPostRemoved(KBlog::BlogPost*)) );
    d->kBlog->removePost( post );
}

void Backend::slotPostRemoved( KBlog::BlogPost *post )
{
    if(!post) {
        kDebug()<<"post returned from server is NULL";
        return;
    }
    if( !DBMan::self()->removePost(d->bBlog->id(), post->postId()) ) {
        kDebug()<<"cannot remove post from database, error: "<<DBMan::self()->lastErrorText();
    }
    emit sigPostRemoved(d->bBlog->id(), BilboPost(*post));
}

void Backend::fetchPost( BilboPost* post )
{
//     KBlog::BlogPost *bp = post.toKBlogPost();
    connect( d->kBlog, SIGNAL(fetchedPost(KBlog::BlogPost*)),
             this, SLOT(slotPostFetched(KBlog::BlogPost*)) );
    d->kBlog->fetchPost( post );
}

void Backend::slotPostFetched( KBlog::BlogPost *post )
{
    emit sigPostFetched( new BilboPost(*post) );
//     delete post;
}

void Backend::error( KBlog::Blog::ErrorType type, const QString & errorMessage )
{
    kDebug() << "Blog Id: " << d->bBlog->id();
    QString errType = errorTypeToString( type );
    errType += errorMessage;
    kDebug() << errType;
    kDebug() << "Emitting sigError";
    Q_EMIT sigError( errType );
}

void Backend::slotMediaError( KBlog::Blog::ErrorType type, const QString & errorMessage,
                              KBlog::BlogMedia * media )
{
    kDebug();
    QString errType = errorTypeToString( type );
    errType += errorMessage;
    kDebug() << errType;
    kDebug() << "Emitting sigMediaError ...";
    emit sigMediaError( errorMessage, d->mPublishMediaMap[ media ] );
    d->mPublishMediaMap.remove( media );
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
        Q_EMIT sigError( i18n("post is NULL") );
        return;
    }
    kDebug()<<"isPrivate: "<<post->isPrivate();
    BilboPost *pp = new BilboPost( *post );
    int post_id;
    if( d->mSubmitPostStatusMap[ post ] == KBlog::BlogPost::Modified) {
        post_id = DBMan::self()->editPost( *pp, d->bBlog->id() );
    } else {
        post_id = DBMan::self()->addPost( *pp, d->bBlog->id() );
    }
    d->mSubmitPostStatusMap.remove(post);
    if ( post_id != -1 ) {
        pp->setPrivate( post->isPrivate() );
        pp->setId( post_id );
        kDebug() << "Emitting sigPostPublished ...";
        Q_EMIT sigPostPublished( d->bBlog->id(), pp );
    }
    // TODO crashes stylegetter on GData. Somehow the post gets deleted before
    // slotFetchedPost as it seems. Don't get all the pointer copies done here.
    //delete post;
}

KBlog::BlogPost* Backend::preparePost( KBlog::BlogPost* post )
{
    QString content = post->content();
    QString html1;
    int i = 0;
    int found = content.indexOf(QLatin1String("<pre>"), i, Qt::CaseInsensitive);
    while ( found != -1 )
    {
        html1 += content.mid( i, found-i).remove(QLatin1Char('\n'));
        i = found;
        found = content.indexOf(QLatin1String("</pre>"), i, Qt::CaseInsensitive);
        if ( found != -1 ) {
            html1 += content.mid( i, found+5-i);
            i = found + 5;
            found = content.indexOf(QLatin1String("<pre>"), i, Qt::CaseInsensitive);
        } else {
            html1 += content.mid( i, content.length()-i );
            i = -1;
        }
    }
    if ( i != -1 )
        html1 += content.mid( i, content.length()-i).remove(QLatin1Char('\n'));
    post->setContent( html1 );

    content = post->additionalContent();
    QString html2 = QString();
    i= 0;
    found = content.indexOf(QLatin1String("<pre>"), i, Qt::CaseInsensitive);
    while ( found != -1 )
    {
        html2 += content.mid( i, found-i).remove(QLatin1Char('\n'));
        i = found;
        found = content.indexOf(QLatin1String("</pre>"), i, Qt::CaseInsensitive);
        if ( found != -1 ) {
            html2 += content.mid( i, found+5-i);
            i = found + 5;
            found = content.indexOf(QLatin1String("<pre>"), i, Qt::CaseInsensitive);
        } else {
            html2 += content.mid( i, content.length()-i );
            i = -1;
        }
    }
    if ( i != -1 )
        html2 += content.mid( i, content.length()-i).remove(QLatin1Char('\n'));
    post->setAdditionalContent( html2 );

    //the following two lines are replaced by the above code, because '\n' characters shouldn't
    //be omitted inside <pre> blocks.

    //post.setContent( post.content().remove('\n') );
    //post.setAdditionalContent( post.additionalContent().remove( '\n' ) );
    if ( d->bBlog->api() == BilboBlog::MOVABLETYPE_API || d->bBlog->api() == BilboBlog::WORDPRESSBUGGY_API ) {
        QStringList content = post->content().split(splitRX);
        if( content.count() == 2 ) {
            post->setContent(content.at(0));
            post->setAdditionalContent( content.at(1) );
        }
    }
    //     if( d->bBlog->api() == BilboBlog::MOVABLETYPE_API && post.categoryList().count() > 0 ) {
    //         mCreatePostCategories = post.categoryList();
    //         categoryListNotSet = true;
    //     }
    return post;//.toKBlogPost();
}

