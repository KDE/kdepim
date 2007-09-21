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

#include "uiserver.h"

#include "assuanserverconnection.h"
#include "assuancommand.h"

#include "kleo-assuan.h"

#include "detail_p.h"

#include <ktempdir.h>

#include <QTcpServer>
#include <QTcpSocket>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QEventLoop>
#include <QTimer>
#include <qendian.h>

#include <boost/range/empty.hpp>
#include <boost/bind.hpp>

#include <stdexcept>
#include <cassert>

#ifdef Q_OS_WIN32
# include <windows.h>
# include <io.h>
#else
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/un.h>
# include <cstdio>
# include <cerrno>
# include <cstring>
#endif

using namespace Kleo;
using namespace boost;

namespace {
    template <typename Ex>
    void throw_( const QString & message ) {
        throw Ex( message.toUtf8().constData() );
    }

    static QString tmpDirPrefix() {
        return QDir::temp().absoluteFilePath( "gpg-" );
    }
}

class UiServer::Private : public QTcpServer {
    Q_OBJECT
    friend class ::Kleo::UiServer;
    UiServer * const q;
public:
    explicit Private( UiServer * qq );

private:
    void makeListeningSocket();
    QString makeFileName( const QString & hint=QString() ) const;

protected:
    /* reimp */ void incomingConnection( int fd );

private Q_SLOTS:
    void slotConnectionClosed( Kleo::AssuanServerConnection * conn ) {
        qDebug( "UiServer: connection %p closed", conn );
        connections.erase( std::remove_if( connections.begin(), connections.end(),
                                           bind( &shared_ptr<AssuanServerConnection>::get, _1 ) == conn ),
                           connections.end() );
        if ( q->isStopped() )
            emit q->stopped();
    }

private:
    KTempDir tmpDir;
    QFile file;
    std::vector< shared_ptr<AssuanCommandFactory> > factories;
    std::vector< shared_ptr<AssuanServerConnection> > connections;
    QString socketname;
};

UiServer::Private::Private( UiServer * qq )
    : QTcpServer(),
      q( qq ),
      tmpDir( tmpDirPrefix() ),
      file(),
      factories(),
      connections(),
      socketname()
{
    assuan_set_assuan_err_source( GPG_ERR_SOURCE_DEFAULT );
}


UiServer::UiServer( const QString & socket, QObject * p )
    : QObject( p ), d( new Private( this ) )
{
    d->socketname = d->makeFileName( socket );
}

UiServer::~UiServer() {}

bool UiServer::registerCommandFactory( const shared_ptr<AssuanCommandFactory> & cf ) {
    if ( cf && empty( std::equal_range( d->factories.begin(), d->factories.end(), cf, _detail::ByName<std::less>() ) ) ) {
        d->factories.push_back( cf );
        std::inplace_merge( d->factories.begin(), d->factories.end() - 1, d->factories.end(), _detail::ByName<std::less>() );
        return true;
    } else {
        qWarning( "UiServer::registerCommandFactory( %p ): factory NULL or already registered", cf ? cf.get() : 0 );
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

QString UiServer::socketName() const {
    return d->socketname;
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

QString UiServer::Private::makeFileName( const QString & socket ) const {
    if ( !socket.isEmpty() )
        return socket;
    if ( tmpDir.status() != 0 )
        throw_<std::runtime_error>( tr( "Couldn't create directory %1: %2" ).arg( tmpDirPrefix() + "XXXXXXXX", QString::fromLocal8Bit( strerror(errno) ) ) );
    const QDir dir( tmpDir.name() );
    assert( dir.exists() );
    return dir.absoluteFilePath( "S.uiserver" );
}

#ifndef Q_OS_WIN32

static inline QString system_error_string() {
    return QString::fromLocal8Bit( strerror(errno) );
}

void UiServer::Private::makeListeningSocket() {

    // First, create a file (we do this only for the name, gmpfh)
    const QString fileName = socketname;
    if ( QFile::exists( fileName ) )
        throw_<std::runtime_error>( tr( "Detected another running gnupg UI server listening at %1." ).arg( fileName ) );
    const QByteArray encodedFileName = QFile::encodeName( fileName );

    // Create a Unix Domain Socket:
    const int sock = ::socket( AF_UNIX, SOCK_STREAM, 0 );
    if ( sock < 0 )
        throw_<std::runtime_error>( tr( "Couldn't create socket: %1" ).arg( system_error_string() ) );

    try {
        // Bind
        struct sockaddr_un sa;
        std::memset( &sa, 0, sizeof(sa) );
        sa.sun_family = AF_UNIX;
        std::strncpy( sa.sun_path, encodedFileName.constData(), sizeof( sa.sun_path ) );
        if ( ::bind( sock, (struct sockaddr*)&sa, sizeof( sa ) ) )
            throw_<std::runtime_error>( tr( "Couldn't bind to socket: %1" ).arg( system_error_string() ) );

        // TODO: permissions?
    
        // Listen
        if ( ::listen( sock, SOMAXCONN ) )
            throw_<std::runtime_error>( tr( "Couldn't listen to socket: %1" ).arg( system_error_string() ) );

        if ( !setSocketDescriptor( sock ) )
            throw_<std::runtime_error>( tr( "Couldn't pass socket to Qt: %1. This should not happen, please report this bug." ).arg( errorString() ) );

    } catch ( ... ) {
        ::close( sock );
        throw;
    }
}

#else

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

#endif

void UiServer::Private::incomingConnection( int fd ) {
    try {
        qDebug( "UiServer: client connect on fd %d", fd );
        const shared_ptr<AssuanServerConnection> c( new AssuanServerConnection( (assuan_fd_t)fd, factories ) );
        connect( c.get(), SIGNAL(closed(Kleo::AssuanServerConnection*)),
                 this, SLOT(slotConnectionClosed(Kleo::AssuanServerConnection*)) );
        connections.push_back( c );
        qDebug( "UiServer: client connection %p established successfully", c.get() );
    } catch ( const assuan_exception & e ) {
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

#include "uiserver.moc"
#include "moc_uiserver.cpp"
