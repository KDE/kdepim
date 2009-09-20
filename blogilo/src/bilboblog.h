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

#ifndef BILBOBLOG_H
#define BILBOBLOG_H

#include "constants.h"
#include <QObject>
#include <kurl.h>
#include <QHash>
/**
Blog definition class!

 @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
 @author Golnaz Nilieh <g382nilieh@gmail.com>
*/
class BilboBlog : public QObject
{
public:
    enum ApiType {
        BLOGGER1_API = 0, METAWEBLOG_API, MOVABLETYPE_API, WORDPRESSBUGGY_API, GDATA_API
    };

    explicit BilboBlog( QObject *parent = 0 );
    explicit BilboBlog( const BilboBlog &, QObject *parent = 0);
    ~BilboBlog();

    /**
     * returns blog xmlrpc Url!
     * For http://bilbo.wordpress.com :
     * it's url() is http://bilbo.wordpress.com/xmlrpc.php
     * and it's blogUrl() is http://bilbo.wordpress.com/
     * @return url usable for xmlrpc!
     */
    KUrl url() const;
    void setUrl( const KUrl& );
    QString blogid() const;
    void setBlogId( const QString& );
    QString username() const;
    void setUsername( const QString& );
    QString password() const;
    void setPassword( const QString& );
    QString title() const;
    void setTitle( const QString& );
//     QString stylePath() const;
//     void setStylePath( const QString& );
    ApiType api() const;
    void setApi( const ApiType );
    int id() const;//id in DB
    void setId( const int );
    Qt::LayoutDirection direction() const;
    void setDirection( const Qt::LayoutDirection );
    QString localDirectory() const;
    void setLocalDirectory( const QString& );
    bool isError() const;
    void setError(bool isError);

    bool supportUploadMedia() const;
    bool supportCategory() const;
    bool supportTag() const;
//     bool supportComments() const;
    /**
     * return Blog Actual Url!
     * For http://bilbo.wordpress.com :
     * it's url() is http://bilbo.wordpress.com/xmlrpc.php
     * and it's blogUrl() is http://bilbo.wordpress.com/
     * @return Blog actual url.
     */
    QString blogUrl() const;
    void setBlogUrl(const QString &blogUrl);

private:
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
};

#endif
