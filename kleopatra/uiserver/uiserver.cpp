/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/uiserver.cpp

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

#include "uiserver.h"
#include "uiserver_p.h"

#include <utils/detail_p.h>
#include "utils/gnupg-helper.h"
#include <utils/exception.h>
#include <utils/stl_util.h>

#include <KLocalizedString>

#include <QTcpSocket>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
#include <QFile>

#include <boost/range/empty.hpp>
#include <boost/bind.hpp>

#include <algorithm>
#include <cassert>
#include <cerrno>

using namespace Kleo;
using namespace boost;

// static
void UiServer::setLogStream( FILE * stream ) {
    assuan_set_assuan_log_stream( stream );
}

UiServer::Private::Private( UiServer * qq )
    : QTcpServer(),
      q( qq ),
      file(),
      factories(),
      connections(),
      suggestedSocketName(),
      actualSocketName(),
      cryptoCommandsEnabled( false )
{
    assuan_set_assuan_err_source( GPG_ERR_SOURCE_DEFAULT );
}

bool UiServer::Private::isStaleAssuanSocket( const QString& fileName )
{
    assuan_context_t ctx = 0;
    const bool error = assuan_socket_connect_ext( &ctx, QFile::encodeName( fileName ).constData(), -1, 0 );
    if ( !error )
        assuan_disconnect( ctx );
    return error;
}

UiServer::UiServer( const QString & socket, QObject * p )
    : QObject( p ), d( new Private( this ) )
{
    d->suggestedSocketName = d->makeFileName( socket );
}

UiServer::~UiServer() {
    if ( QFile::exists( d->actualSocketName ) )
        QFile::remove( d->actualSocketName );
}

bool UiServer::registerCommandFactory( const shared_ptr<AssuanCommandFactory> & cf ) {
    if ( cf && empty( std::equal_range( d->factories.begin(), d->factories.end(), cf, _detail::ByName<std::less>() ) ) ) {
        d->factories.push_back( cf );
        std::inplace_merge( d->factories.begin(), d->factories.end() - 1, d->factories.end(), _detail::ByName<std::less>() );
        return true;
    } else {
        qWarning( "UiServer::registerCommandFactory( %p ): factory NULL or already registered", ( void* )( cf ? cf.get() : 0 ) );
        return false;
    }
}

void UiServer::start() {
    d->makeListeningSocket();
}

void UiServer::stop() {

    d->close();

    if ( d->file.exists() )
        d->file.remove();

}

void UiServer::enableCryptoCommands( bool on ) {
    if ( on == d->cryptoCommandsEnabled )
        return;
    d->cryptoCommandsEnabled = on;
    kdtools::for_each( d->connections,
                       bind( &AssuanServerConnection::enableCryptoCommands, _1, on ) );
}

QString UiServer::socketName() const {
    return d->actualSocketName;
}

bool UiServer::waitForStopped( unsigned int ms ) {
    if ( isStopped() )
        return true;
    QEventLoop loop;
    QTimer timer;
    timer.setInterval( ms );
    timer.setSingleShot( true );
    connect( &timer, SIGNAL(timeout()), &loop, SLOT(quit()) );
    connect( this,   SIGNAL(stopped()), &loop, SLOT(quit()) );
    loop.exec();
    return !timer.isActive();
}

bool UiServer::isStopped() const {
    return d->connections.empty() && !d->isListening() ;
}

bool UiServer::isStopping() const {
    return !d->connections.empty() && !d->isListening() ;
}

void UiServer::Private::slotConnectionClosed( Kleo::AssuanServerConnection * conn ) {
    qDebug( "UiServer: connection %p closed", ( void* )conn );
    connections.erase( std::remove_if( connections.begin(), connections.end(),
                                       boost::bind( &boost::shared_ptr<AssuanServerConnection>::get, _1 ) == conn ),
                       connections.end() );
    if ( q->isStopped() )
        emit q->stopped();
}


void UiServer::Private::incomingConnection( int fd ) {
    try {
        qDebug( "UiServer: client connect on fd %d", fd );
#ifdef HAVE_ASSUAN_SOCK_GET_NONCE
        if ( assuan_sock_check_nonce( (assuan_fd_t)fd, &nonce ) ) {
            qDebug( "UiServer: nonce check failed" );
            assuan_sock_close( (assuan_fd_t)fd );
            return;
        }
#endif
        const shared_ptr<AssuanServerConnection> c( new AssuanServerConnection( (assuan_fd_t)fd, factories ) );
        connect( c.get(), SIGNAL(closed(Kleo::AssuanServerConnection*)),
                 this, SLOT(slotConnectionClosed(Kleo::AssuanServerConnection*)) );
        connect( c.get(), SIGNAL(startKeyManagerRequested()),
                 q, SIGNAL(startKeyManagerRequested()), Qt::QueuedConnection );
        connect( c.get(), SIGNAL(startConfigDialogRequested()),
                 q, SIGNAL(startConfigDialogRequested()), Qt::QueuedConnection );
        c->enableCryptoCommands( cryptoCommandsEnabled );
        connections.push_back( c );
        qDebug( "UiServer: client connection %p established successfully", ( void* )c.get() );
    } catch ( const Exception & e ) {
        qDebug( "UiServer: client connection failed: %s", e.what() );
        QTcpSocket s;
        s.setSocketDescriptor( fd );
        QTextStream( &s ) << "ERR " << e.error_code() << " " << e.what() << "\r\n";
        s.waitForBytesWritten();
        s.close();
    } catch ( ... ) {
        qDebug( "UiServer: client connection failed: unknown exception caught" );
        // this should never happen...
        QTcpSocket s;
        s.setSocketDescriptor( fd );
        QTextStream( &s ) << "ERR 63 unknown exception caught\r\n";
        s.waitForBytesWritten();
        s.close();
    }
}

QString UiServer::Private::makeFileName( const QString & socket ) const {
    if ( !socket.isEmpty() )
        return socket;
    const QString gnupgHome = gnupgHomeDirectory();
    if ( gnupgHome.isEmpty() )
        throw_<std::runtime_error>( i18n( "Could not determine the GnuPG home directory. Consider setting the GNUPGHOME environment variable." ) );
    ensureDirectoryExists( gnupgHome );
    const QDir dir( gnupgHome );
    assert( dir.exists() );
    return dir.absoluteFilePath( "S.uiserver" );
}

void UiServer::Private::ensureDirectoryExists( const QString& path ) const {
    const QFileInfo info( path );
    if ( info.exists() && !info.isDir() )
        throw_<std::runtime_error>( i18n( "Cannot determine the GnuPG home directory: %1 exists but is no directory.", path ) );
    if ( info.exists() )
        return;
    const QDir dummy; //there is no static QDir::mkpath()...
    errno = 0;
    if ( !dummy.mkpath( path ) )
        throw_<std::runtime_error>( i18n( "Could not create GnuPG home directory %1: %2", path, systemErrorString() ) );
}

void UiServer::Private::makeListeningSocket() {

    // First, create a file (we do this only for the name, gmpfh)
    const QString fileName = suggestedSocketName;

    if ( QFile::exists( fileName ) ) {
        if ( isStaleAssuanSocket( fileName ) ) {
            QFile::remove( fileName );
        } else {
            throw_<std::runtime_error>( i18n( "Detected another running gnupg UI server listening at %1.", fileName ) );
        }
    }

    doMakeListeningSocket( QFile::encodeName( fileName ) );

    actualSocketName = suggestedSocketName;
}

#include "moc_uiserver_p.cpp"
#include "moc_uiserver.cpp"
