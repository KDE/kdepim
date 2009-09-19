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

#include "bilbomedia.h"
#include "kicon.h"
#include "kdebug.h"
//#include <QMimeData>
#include <QPixmap>

BilboMedia::BilboMedia( QObject *parent )
        : QObject( parent )
{
    mChecksum = 0;
}

BilboMedia::~BilboMedia()
{
}

int BilboMedia::blogId() const
{
    return mBlogId;
}


void BilboMedia::setBlogId( const int blog_id )
{
    mBlogId = blog_id;
}

int BilboMedia::mediaId() const
{
    return mMediaId;
}

void BilboMedia::setMediaId( const int media_id )
{
    mMediaId = media_id;
}

bool BilboMedia::isUploaded() const
// bool BilboMedia::isLocal() const
{
    return mIsUploaded;
//     return mIsLocal;
}

void BilboMedia::setUploaded( bool uploaded )
// void BilboMedia::setLocal( bool is_local )
{
    mIsUploaded = uploaded;
//     mIsLocal = is_local;
}

KUrl BilboMedia::localUrl() const
{
    return mLocalUrl;
}

void BilboMedia::setLocalUrl( const KUrl& url )
{
    mLocalUrl = url;
}

KUrl BilboMedia::remoteUrl() const
{
    return mRemoteUrl;
}

void BilboMedia::setRemoteUrl( const KUrl& url )
{
    mRemoteUrl = url;
}

QString BilboMedia::mimeType() const
{
    return mMimeType;
}

void BilboMedia::setMimeType( const QString &type )
{
    mMimeType = type;
}

QString BilboMedia::name() const
{
    return mName;
}

void BilboMedia::setName( const QString &name )
{
    mName = name;
}

KIcon BilboMedia::icon() const
{
    QPixmap iconPic;
    QString type;
    type = this->mimeType();
    if ( type.contains( "image" ) && !this->localUrl().isEmpty() ) {
        iconPic.load( this->localUrl().toLocalFile() );
//   iconPic.scaled(64, 64, Qt::IgnoreAspectRatio);
        iconPic.scaledToHeight( 32 );
        if ( !iconPic.isNull() ) {
            return KIcon( iconPic );
        } else {
            kDebug() << "iconPic is Null";
        }
    }

    type.replace( QChar( '/' ), QChar( '-' ) );
    return KIcon( type );
}

quint16 BilboMedia::checksum()
{
    return mChecksum;
}

void BilboMedia::setCheckSum( quint16 sum )
{
    mChecksum = sum;
}

#include <bilbomedia.moc>
