/****************************************************************************
** Copyright (C) 2001-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
**********************************************************************/

#ifndef __KDTOOLSCORE_KDPIPEIODEVICE_H__
#define __KDTOOLSCORE_KDPIPEIODEVICE_H__

#include <QIODevice>

#include <utility>

#include "checker.h"

class KDPipeIODevice : public QIODevice {
    Q_OBJECT
    KDAB_MAKE_CHECKABLE( KDPipeIODevice )
public:
    explicit KDPipeIODevice( QObject * parent=0 );
    explicit KDPipeIODevice( int fd, OpenMode=ReadOnly, QObject * parent=0 );
    explicit KDPipeIODevice( Qt::HANDLE handle, OpenMode=ReadOnly, QObject * parent=0 );
    ~KDPipeIODevice();

    static std::pair<KDPipeIODevice*, KDPipeIODevice*> makePairOfConnectedPipes();

    bool open( int fd, OpenMode mode=ReadOnly );
    bool open( Qt::HANDLE handle, OpenMode mode=ReadOnly );

    Qt::HANDLE handle() const;
    int descriptor() const;

    /* reimp */ qint64 bytesAvailable() const;
    /* reimp */ qint64 bytesToWrite() const;
    /* reimp */ bool canReadLine() const;
    /* reimp */ void close();
    /* reimp */ bool isSequential() const;
    /* reimp */ bool atEnd() const;

    /* reimp */ bool waitForBytesWritten( int msecs );
    /* reimp */ bool waitForReadyRead( int msecs );

protected:
    /* reimp */ qint64 readData( char * data, qint64 maxSize );
    /* reimp */ qint64 writeData( const char * data, qint64 maxSize );

private:
    class Private;
    Private * d;
};

#endif /* __KDTOOLSCORE_KDPIPEIODEVICE_H__ */

