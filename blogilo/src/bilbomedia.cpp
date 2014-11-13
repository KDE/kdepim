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

#include "bilbomedia.h"

#include <qdebug.h>
#include <QIcon>
#include <QPixmap>
#include <QUrl>
class BilboMedia::Private
{
public:
    Private()
        : mBlogId(-1),
          mMediaId(-1),
          mIsUploaded(false),
          mChecksum(0)
    {

    }

    int mBlogId;
    int mMediaId;
    bool mIsUploaded;
    QUrl mLocalUrl;
    QUrl mRemoteUrl;
    QString mMimeType;
    QString mName;
    quint16 mChecksum;
};
BilboMedia::BilboMedia(QObject *parent)
    : QObject(parent), d(new Private)
{
    d->mChecksum = 0;
}

BilboMedia::~BilboMedia()
{
    delete d;
}

int BilboMedia::blogId() const
{
    return d->mBlogId;
}

void BilboMedia::setBlogId(const int blog_id)
{
    d->mBlogId = blog_id;
}

int BilboMedia::mediaId() const
{
    return d->mMediaId;
}

void BilboMedia::setMediaId(const int media_id)
{
    d->mMediaId = media_id;
}

bool BilboMedia::isUploaded() const
{
    return d->mIsUploaded;
}

void BilboMedia::setUploaded(bool uploaded)
{
    d->mIsUploaded = uploaded;
}

QUrl BilboMedia::localUrl() const
{
    return d->mLocalUrl;
}

void BilboMedia::setLocalUrl(const QUrl &url)
{
    d->mLocalUrl = url;
}

QUrl BilboMedia::remoteUrl() const
{
    return d->mRemoteUrl;
}

void BilboMedia::setRemoteUrl(const QUrl &url)
{
    d->mRemoteUrl = url;
}

QString BilboMedia::mimeType() const
{
    return d->mMimeType;
}

void BilboMedia::setMimeType(const QString &type)
{
    d->mMimeType = type;
}

QString BilboMedia::name() const
{
    return d->mName;
}

void BilboMedia::setName(const QString &name)
{
    d->mName = name;
}

QIcon BilboMedia::icon() const
{
    QPixmap iconPic;
    QString type;
    type = this->mimeType();
    if (type.contains(QLatin1String("image")) && !this->localUrl().isEmpty()) {
        iconPic.load(this->localUrl().toLocalFile());
//   iconPic.scaled(64, 64, Qt::IgnoreAspectRatio);
        iconPic.scaledToHeight(32);
        if (!iconPic.isNull()) {
            return QIcon(iconPic);
        } else {
            qDebug() << "iconPic is Null";
        }
    }

    type.replace(QLatin1Char('/'), QLatin1Char('-'));
    return QIcon(type);
}

quint16 BilboMedia::checksum() const
{
    return d->mChecksum;
}

void BilboMedia::setCheckSum(quint16 sum)
{
    d->mChecksum = sum;
}

