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

#include "bilboblog.h"
#include "dbman.h"
#include <kblog/wordpressbuggy.h>
#include <kblog/gdata.h>
#include <KDebug>

#include <QApplication>


class BilboBlog::Private
{
public:
    Private()
    : kblog(0)
    {}
    KUrl mUrl;
    QString mBlogUrl;
    QString mBlogid;
    QString mUsername;
    QString mPassword;
    QString mTitle;
    QString mStylePath;
    ApiType mApi;
    int mId;//id in DB
    Qt::LayoutDirection mDir;
    QString mLocalDirectory;
    bool mError;
    QHash<QString, bool> mSupportedFeatures;
    KBlog::Blog *kblog;
    QMap<QString, QString> mAuthData;
};

BilboBlog::BilboBlog( QObject *parent )
        : QObject( parent ), d(new Private)
{
    d->mError = false;
    setApi(BLOGGER1_API);
}

BilboBlog::BilboBlog( const BilboBlog &blog)
        : QObject( qApp ), d(new Private)
{
    d->mUrl = blog.url();
    d->mBlogUrl = blog.blogUrl();
    d->mBlogid = blog.blogid();
    d->mUsername = blog.username();
    d->mPassword = blog.password();
    d->mTitle = blog.title();
    setApi( blog.api() );
    d->mId = blog.id();
    d->mDir = blog.direction();
    d->mLocalDirectory = blog.localDirectory();
    d->mError = blog.isError();
}

BilboBlog::~BilboBlog()
{
    delete d;
}

KBlog::Blog* BilboBlog::blogBackend()
{
    if(!d->kblog){
        switch ( api() ) {
        case BilboBlog::BLOGGER1_API:
            d->kblog = new KBlog::Blogger1( url(), this );
            break;
        case BilboBlog::METAWEBLOG_API:
            d->kblog = new KBlog::MetaWeblog( url(), this );
            break;
        case BilboBlog::MOVABLETYPE_API:
            d->kblog = new KBlog::MovableType( url(), this );
            break;
        case BilboBlog::WORDPRESSBUGGY_API:
            d->kblog = new KBlog::WordpressBuggy( url(), this );
            break;
        case BilboBlog::BLOGGER_API:
            d->kblog = new KBlog::Blogger( url(), this );
            qobject_cast<KBlog::Blogger*>(d->kblog)->setApiKey( QLatin1String("508396175529-icqp62q8t6st41gjv1du100fol6renq4.apps.googleusercontent.com") );
            qobject_cast<KBlog::Blogger*>(d->kblog)->setSecretKey( QLatin1String("JFPDXYmGIuM601vhgVGv0Dlx") );
            break;
        }
        d->kblog->setUserAgent( QLatin1String(APPNAME), QLatin1String(VERSION) );
        d->kblog->setUsername( username() );
        d->kblog->setPassword( password() );
        d->kblog->setBlogId( blogid() );
    }
    return d->kblog;
}

bool BilboBlog::isError() const
{
    return d->mError;
}

void BilboBlog::setError(bool isError)
{
    d->mError = isError;
}

KUrl BilboBlog::url() const
{
    return d->mUrl;
}

void BilboBlog::setUrl( const KUrl &url )
{
    d->mUrl = url;
}

QString BilboBlog::blogid() const
{
    return d->mBlogid;
}

void BilboBlog::setBlogId( const QString &url )
{
    d->mBlogid = url;
}

QString BilboBlog::username() const
{
    return d->mUsername;
}

void BilboBlog::setUsername( const QString &username )
{
    d->mUsername = username;
}

QString BilboBlog::password() const
{
    return d->mPassword;
}

void BilboBlog::setPassword( const QString &password )
{
    d->mPassword = password;
}

QString BilboBlog::title() const
{
    return d->mTitle;
}

void BilboBlog::setTitle( const QString &title )
{
    d->mTitle = title;
}

BilboBlog::ApiType BilboBlog::api() const
{
    return d->mApi;
}

void BilboBlog::setApi( const ApiType api )
{
    d->mApi = api;
    switch(api) {
        case BLOGGER1_API:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = false;
            d->mSupportedFeatures[QLatin1String("category")] = false;
            d->mSupportedFeatures[QLatin1String("tag")] = false;
            break;
        case METAWEBLOG_API:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = true;
            d->mSupportedFeatures[QLatin1String("category")] = true;
            d->mSupportedFeatures[QLatin1String("tag")] = false;
            break;
        case MOVABLETYPE_API:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = true;
            d->mSupportedFeatures[QLatin1String("category")] = true;
            d->mSupportedFeatures[QLatin1String("tag")] = true;
            break;
        case WORDPRESSBUGGY_API:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = true;
            d->mSupportedFeatures[QLatin1String("category")] = true;
            d->mSupportedFeatures[QLatin1String("tag")] = true;
            break;
        case BLOGGER_API:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = false;
            d->mSupportedFeatures[QLatin1String("category")] = false;
            d->mSupportedFeatures[QLatin1String("tag")] = true;
            break;
        default:
            d->mSupportedFeatures[QLatin1String("uploadMedia")] = false;
            d->mSupportedFeatures[QLatin1String("category")] = false;
            d->mSupportedFeatures[QLatin1String("tag")] = false;
            break;
    }
}

int BilboBlog::id() const
{
    return d->mId;
}

void BilboBlog::setId( const int id )
{
    d->mId = id;
}

Qt::LayoutDirection BilboBlog::direction() const
{
    return d->mDir;
}

void BilboBlog::setDirection( const Qt::LayoutDirection dir )
{
    d->mDir = dir;
}

QString BilboBlog::localDirectory() const
{
    return d->mLocalDirectory;
}

void BilboBlog::setLocalDirectory( const QString &directory )
{
    d->mLocalDirectory = directory;
}

QString BilboBlog::blogUrl() const
{
    if(d->mBlogUrl.isEmpty())
        return d->mUrl.prettyUrl();
    else
        return d->mBlogUrl;
}

void BilboBlog::setBlogUrl(const QString &blogUrl)
{
    d->mBlogUrl = blogUrl;
}

bool BilboBlog::supportUploadMedia() const
{
    return d->mSupportedFeatures[QLatin1String("uploadMedia")];
}

bool BilboBlog::supportCategory() const
{
    return d->mSupportedFeatures[QLatin1String("category")];
}

bool BilboBlog::supportTag() const
{
    return d->mSupportedFeatures[QLatin1String("tag")];
}

void BilboBlog::setAuthData( const QMap<QString, QString>& authData )
{
    d->mAuthData = authData;
}

QMap<QString, QString> BilboBlog::authData() const
{
    return d->mAuthData;
}

