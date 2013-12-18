/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/uiserver_win.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "uiserver_p.h"

#include "utils/gnupg-helper.h"

#include <KLocalizedString>

#include <qendian.h>

#include <stdexcept>
#include <cassert>

#include <windows.h>
#include <io.h>
#include <winsock2.h>

#include <cstring>
#include <cstdlib>

using namespace Kleo;
using namespace boost;


QString UiServer::Private::systemErrorString() {
#ifndef _WIN32_WCE
    return QString::fromLocal8Bit( strerror(errno) );
#else
    return QString();
#endif
}

void UiServer::Private::doMakeListeningSocket( const QByteArray & encodedFileName ) {
    // Create a Unix Domain Socket:
    const assuan_fd_t sock = assuan_sock_new( AF_UNIX, SOCK_STREAM, 0 );
    if ( sock == ASSUAN_INVALID_FD )
        throw_<std::runtime_error>( i18n( "Could not create socket: %1", systemErrorString() ) );

    try {
        // Bind
        struct sockaddr_un sa;
        std::memset( &sa, 0, sizeof(sa) );
        sa.sun_family = AF_UNIX;
        std::strncpy( sa.sun_path, encodedFileName.constData(), sizeof( sa.sun_path ) - 1 );
        if ( assuan_sock_bind( sock, (struct sockaddr*)&sa, sizeof( sa ) ) )
            throw_<std::runtime_error>( i18n( "Could not bind to socket: %1", systemErrorString() ) );

        if ( assuan_sock_get_nonce( (struct sockaddr*)&sa, sizeof( sa ), &nonce ) )
            throw_<std::runtime_error>( i18n("Could not get socket nonce: %1", systemErrorString() ) );

        // Listen
        if ( ::listen( (SOCKET)sock, SOMAXCONN ) )
            throw_<std::runtime_error>( i18n( "Could not listen to socket: %1", systemErrorString() ) );

        if ( !setSocketDescriptor( (intptr_t)sock  ) )
            throw_<std::runtime_error>( i18n( "Could not pass socket to Qt: %1. This should not happen, please report this bug.", errorString() ) );

    } catch ( ... ) {
        assuan_sock_close( sock );
        throw;
    }
}

