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

#include "uiserver_p.h"

#include "utils/gnupg-registry.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <qendian.h>

#include <stdexcept>
#include <cassert>

#include <windows.h>
#include <io.h>

using namespace Kleo;
using namespace boost;

QString UiServer::Private::makeFileName( const QString & socket ) const {
    if ( !socket.isEmpty() )
        return socket;
    if ( tmpDir.status() != 0 )
        throw_<std::runtime_error>( tr( "Couldn't create directory %1: %2" ).arg( tmpDirPrefix() + "XXXXXXXX", QString::fromLocal8Bit( strerror(errno) ) ) );
    const char * const gnupg_home = default_homedir();
    const QDir dir( gnupg_home ? QFile::decodeName( gnupg_home ) : tmpDir.name() );
    assert( dir.exists() );
    return dir.absoluteFilePath( "S.uiserver" );
}

// The Windows case is simpler, because we use a TCP socket here, so
// we use vanilla QTcpServer:
void UiServer::Private::makeListeningSocket() {

    // First, create a tempfile that will contain the port we're
    // listening on:
    file.setFileName( socketname );
    if ( !file.open( QIODevice::WriteOnly ) )
        throw_<std::runtime_error>( tr( "Couldn't create temporary file %1: %2" ).arg( file.fileName(), file.errorString() ) );

    // now, start listening to the host:
    if ( !listen( QHostAddress::LocalHost ) )
        throw_<std::runtime_error>( tr( "UiServer: listen failed: %1" ).arg( errorString() ) );

    const quint16 port = serverPort();
    QTextStream( &file ) << qToBigEndian( port ) << " # == htons( " << port << " )" << endl;
    file.close();
}

