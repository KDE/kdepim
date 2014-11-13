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

#ifndef BILBOMEDIA_H
#define BILBOMEDIA_H

#include <QObject>

/**
Contains needed properties of each media object, like images and videos.

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
 */
class BilboMedia : public QObject
{
    Q_OBJECT
public:
    ///BilboMedia constructor
    explicit BilboMedia(QObject *parent = 0);

    ///BilboMedia destructor
    ~BilboMedia();

    int blogId() const;
    void setBlogId(const int blog_id);

    int mediaId() const;
    void setMediaId(const int media_id);

    bool isUploaded() const;
    void setUploaded(bool uploaded);

//     bool isLocal() const;
//     void setLocal( bool is_local );

    QUrl localUrl() const;
    void setLocalUrl(const QUrl &url);

    QUrl remoteUrl() const;
    void setRemoteUrl(const QUrl &url);

    QString mimeType() const;
    void setMimeType(const QString &type);

    QString name() const;
    void setName(const QString &name);

    QIcon icon() const;

    quint16 checksum() const;
    void setCheckSum(quint16 sum);

private:
    class Private;
    Private *const d;
};

#endif
