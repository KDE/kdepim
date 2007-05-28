/***************************************************************************
*   Copyright (C) 2006 by                                                 *
*   Marcin Przylucki <marcin.przylucki@kdemail.net>                       *
*   Marco Gulino <marco@kmobiletools.org>                                 *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,       *
*  Boston, MA 02110-1301, USA.                                            *
***************************************************************************/

#ifndef KIO_OBEXPROTOCOL_H
#define KIO_OBEXPROTOCOL_H

#include <kio/slavebase.h>

#include "obeximpl.h"
//Added by qt3to4:
#include <Q3CString>


/**
@author Marcin Przylucki
*/
class kio_obexProtocol : public KIO::SlaveBase
{
public:
    kio_obexProtocol(const Q3CString &pool_socket, const Q3CString &app_socket);
    ~kio_obexProtocol();

    void listDir(const KUrl &url);
    void stat(const KUrl &url);
    void get(const KUrl &url);
    void del(const KUrl &url, bool isfile);
    void special(const QByteArray &data);
    void mkdir( const KUrl &url, int permissions );
//     void finished();

    private:
        void listRoot(const KUrl &url);
        void put(const KUrl &url, int permissions, bool overwrite, bool resume);
        void disconnectTimeout();
        void setHost(const QString &host, int port, const QString &user, const QString &pass );
        OBEXImpl m_impl;
        QString s_host;
};

#endif
