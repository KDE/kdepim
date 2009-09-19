/***************************************************************************
 *   This file is part of the Bilbo Blogger.                               *
 *   Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>     *
 *   Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "bilboblog.h"

BilboBlog::BilboBlog( QObject *parent )
        : QObject( parent )
{
    mError = false;
    setApi(BLOGGER1_API);
}

BilboBlog::BilboBlog( const BilboBlog &blog, QObject *parent )
        : QObject( parent )
{
    mUrl = blog.url();
    mBlogUrl = blog.blogUrl();
    mBlogid = blog.blogid();
    mUsername = blog.username();
    mPassword = blog.password();
    mTitle = blog.title();
    setApi( blog.api() );
    mId = blog.id();
    mDir = blog.direction();
    mLocalDirectory = blog.localDirectory();
    mError = blog.isError();
}

BilboBlog::~BilboBlog()
{
}

bool BilboBlog::isError() const
{
    return mError;
}

void BilboBlog::setError(bool isError)
{
    mError = isError;
}

KUrl BilboBlog::url() const
{
    return mUrl;
}

void BilboBlog::setUrl( const KUrl &url )
{
    mUrl = url;
}

QString BilboBlog::blogid() const
{
    return mBlogid;
}

void BilboBlog::setBlogId( const QString &url )
{
    mBlogid = url;
}

QString BilboBlog::username() const
{
    return mUsername;
}

void BilboBlog::setUsername( const QString &username )
{
    mUsername = username;
}

QString BilboBlog::password() const
{
    return mPassword;
}

void BilboBlog::setPassword( const QString &password )
{
    mPassword = password;
}

QString BilboBlog::title() const
{
    return mTitle;
}

void BilboBlog::setTitle( const QString &title )
{
    mTitle = title;
}

/*
QString BilboBlog::stylePath() const
{
    return mStylePath;
}

void BilboBlog::setStylePath( const QString &path )
{
    mStylePath = path;
}
*/

BilboBlog::ApiType BilboBlog::api() const
{
    return mApi;
}

void BilboBlog::setApi( const ApiType api )
{
    mApi = api;
    switch(api) {
        case BLOGGER1_API:
            mSupportedFeatures["uploadMedia"] = false;
            mSupportedFeatures["category"] = false;
            mSupportedFeatures["tag"] = false;
            break;
        case METAWEBLOG_API:
            mSupportedFeatures["uploadMedia"] = true;
            mSupportedFeatures["category"] = true;
            mSupportedFeatures["tag"] = false;
            break;
        case MOVABLETYPE_API:
            mSupportedFeatures["uploadMedia"] = true;
            mSupportedFeatures["category"] = true;
            mSupportedFeatures["tag"] = true;
            break;
        case WORDPRESSBUGGY_API:
            mSupportedFeatures["uploadMedia"] = true;
            mSupportedFeatures["category"] = true;
            mSupportedFeatures["tag"] = true;
            break;
        case GDATA_API:
            mSupportedFeatures["uploadMedia"] = false;
            mSupportedFeatures["category"] = false;
            mSupportedFeatures["tag"] = true;
            break;
        default:
            mSupportedFeatures["uploadMedia"] = false;
            mSupportedFeatures["category"] = false;
            mSupportedFeatures["tag"] = false;
            break;
    }
}

int BilboBlog::id() const
{
    return mId;
}

void BilboBlog::setId( const int id )
{
    mId = id;
}

Qt::LayoutDirection BilboBlog::direction() const
{
    return mDir;
}

void BilboBlog::setDirection( const Qt::LayoutDirection dir )
{
    mDir = dir;
}

QString BilboBlog::localDirectory() const
{
    return mLocalDirectory;
}

void BilboBlog::setLocalDirectory( const QString &directory )
{
    mLocalDirectory = directory;
}

QString BilboBlog::blogUrl() const
{
    if(mBlogUrl.isEmpty())
        return mUrl.prettyUrl();
    else
        return mBlogUrl;
}

void BilboBlog::setBlogUrl(const QString &blogUrl)
{
    mBlogUrl = blogUrl;
}

bool BilboBlog::supportUploadMedia() const
{
    return mSupportedFeatures["uploadMedia"];
}

bool BilboBlog::supportCategory() const
{
    return mSupportedFeatures["category"];
}

bool BilboBlog::supportTag() const
{
    return mSupportedFeatures["tag"];
}
