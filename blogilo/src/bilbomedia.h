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

#ifndef BILBOMEDIA_H
#define BILBOMEDIA_H

#include <QObject>
#include <KUrl>

/**
Contains needed properties of each media object, like images and videos.

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
 */
class QMimeData;
class KIcon;
class BilboMedia : public QObject
{
    Q_OBJECT
public:
    ///BilboMedia constructor
    BilboMedia( QObject *parent = 0 );

    ///BilboMedia destructor
    ~BilboMedia();

    int blogId() const;
    void setBlogId( const int blog_id );

    int mediaId() const;
    void setMediaId( const int media_id );

    bool isUploaded() const;
    void setUploaded( bool uploaded );

//     bool isLocal() const;
//     void setLocal( bool is_local );

    KUrl localUrl() const;
    void setLocalUrl( const KUrl &url );

    KUrl remoteUrl() const;
    void setRemoteUrl( const KUrl &url );

    QString mimeType() const;
    void setMimeType( const QString &type );

    QString name() const;
    void setName( const QString &name );

    KIcon icon() const;

    quint16 checksum();
    void setCheckSum( quint16 sum );

private:
    int mBlogId;
    int mMediaId;
    bool mIsUploaded;
//     bool mIsLocal;
    KUrl mLocalUrl;
    KUrl mRemoteUrl;
    QString mMimeType;
    QString mName;
    quint16 mChecksum;
};

#endif
