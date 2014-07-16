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

#include "syncuploader.h"

#include <kdebug.h>
#include "backend.h"
#include "bilbomedia.h"
#include <QEventLoop>

class SyncUploader::Private
{
public:
    Private()
    :mCurrentMedia(0),
      loop(0),
      success(false)
    {}
    BilboMedia *mCurrentMedia;
    QEventLoop *loop;
    QString error;
    bool success;
};

SyncUploader::SyncUploader( QObject *parent )
    :QObject(parent),d(new Private)
{
}

SyncUploader::~SyncUploader()
{
    delete d;
}

QString SyncUploader::errorMessage() const
{
    return d->error;
}

bool SyncUploader::uploadMedia( Backend *backend, BilboMedia *media )
{
    if (!backend || !media){
        kError()<<"Media or Backend is NULL";
        return false;
    }
    d->loop = new QEventLoop(this);
    d->mCurrentMedia = media;
    connect( backend, SIGNAL(sigMediaUploaded(BilboMedia*)), this, SLOT(slotMediaFileUploaded(BilboMedia*)) );
    connect( backend, SIGNAL(sigMediaError(QString,BilboMedia*)),
                this, SLOT(slotMediaError(QString,BilboMedia*)) );

    backend->uploadMedia( media );
    if ( d->loop->exec()==0 )
        return true;
    else
        return false;
}

void SyncUploader::slotMediaFileUploaded( BilboMedia *media )
{
    if (media && media == d->mCurrentMedia){
        kDebug();
        d->success = true;
        d->loop->exit();
    }
}

void SyncUploader::slotMediaError( const QString &errorMessage, BilboMedia* media )
{
    if (media && media == d->mCurrentMedia){
        d->success = false;
        d->error = errorMessage;
        d->loop->exit(1);
    }
}

