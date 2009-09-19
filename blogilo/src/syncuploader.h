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

#ifndef SYNCUPLOADER_H
#define SYNCUPLOADER_H

#include <QObject>
#include <QString>

class Backend;
class BilboMedia;
class QEventLoop;
/**
Synchronous uploader.
This class has just one function, that will upload a media object in synchronous way. (KBlog upload media asynchronous)
and return True on success and false on failure.

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
@author Golnaz Nilieh \<g382nilieh@gmail.com\>
*/
class SyncUploader : public QObject
{
Q_OBJECT
public:
    SyncUploader( QObject *parent=0 );
    ~SyncUploader();

    /**
    Synchronous Media file uploader!
    @return True on success, and false on failure
    */
    bool uploadMedia( Backend *backend, BilboMedia *media );
    QString errorMessage() const;

private slots:
    void slotMediaFileUploaded( BilboMedia *media );
//     void slotError( const QString& errMsg );
    void slotMediaError( const QString &errorMessage, BilboMedia* media );

private:
    BilboMedia *mCurrentMedia;
    QEventLoop *loop;
    QString error;
    bool success;
};

#endif // SYNCUPLOADER_H
