/* -*- mode: c++; c-basic-offset:4 -*-
    command.cpp

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <config-kleopatra.h>

#include "command.h"
#include "command_p.h"

#include <QtGlobal> // Q_OS_WIN

#ifdef Q_OS_WIN // HACK: AllowSetForegroundWindow needs _WIN32_WINDOWS >= 0x0490 set
# ifndef _WIN32_WINDOWS
#  define _WIN32_WINDOWS 0x0500
#  define _WIN32_WINNT 0x0500 // good enough for Vista too
# endif
# include <utils/gnupg-registry.h>
# include <windows.h>
#endif

#include <QMutexLocker>
#include <QFile>
#include <QDebug>
#include <QDir>

#include <assuan.h>
#include <gpg-error.h>

#include <boost/shared_ptr.hpp>
#include <boost/type_traits/remove_pointer.hpp>

#include <algorithm>
#include <string>

using namespace KLEOPATRACLIENT_NAMESPACE;
using namespace boost;

// copied from kleopatra/utils/hex.cpp
static std::string hexencode( const std::string & in ) {
    std::string result;
    result.reserve( 3 * in.size() );

    static const char hex[] = "0123456789ABCDEF";

    for ( std::string::const_iterator it = in.begin(), end = in.end() ; it != end ; ++it )
        switch ( const unsigned char ch = *it ) {
        default:
            if ( ch >= '!' && ch <= '~' || ch > 0xA0 ) {
                result += ch;
                break;
            }
            // else fall through
        case ' ':
            result += '+';
            break;
        case '"':
        case '#':
        case '$':
        case '%':
        case '\'':
        case '+':
        case '=':
            result += '%';
            result += hex[ (ch & 0xF0) >> 4 ];
            result += hex[ (ch & 0x0F)      ];
            break;
        }
    
    return result;
}

#ifdef UNUSED
static std::string hexencode( const char * in ) {
    if ( !in )
        return std::string();
    return hexencode( std::string( in ) );
}
#endif

static QByteArray hexencode( const QByteArray & in ) {
    if ( in.isNull() )
        return QByteArray();
    const std::string result = hexencode( std::string( in.constData() ) );
    return QByteArray( result.data(), result.size() );
}
// end copied from kleopatra/utils/hex.cpp

Command::Command( QObject * p )
    : QObject( p ), d( new Private( this ) )
{
    d->init();
}

Command::Command( Private * pp, QObject * p )
    : QObject( p ), d( pp )
{
    d->init();
}

Command::~Command() {
    delete d; d = 0;
}

void Command::Private::init() {
    connect( this, SIGNAL(started()),  q, SIGNAL(started())  );
    connect( this, SIGNAL(finished()), q, SIGNAL(finished()) );
}

void Command::setParentWId( WId wid ) {
    const QMutexLocker locker( &d->mutex );
    d->inputs.parentWId = wid;
}

WId Command::parentWId() const {
    const QMutexLocker locker( &d->mutex );
    return d->inputs.parentWId;
}


void Command::setServerLocation( const QString & location ) {
    const QMutexLocker locker( &d->mutex );
    d->outputs.serverLocation = location;
}

QString Command::serverLocation() const {
    const QMutexLocker locker( &d->mutex );
    return d->outputs.serverLocation;
}


bool Command::waitForFinished() {
    return d->wait();
}

bool Command::waitForFinished( unsigned long ms ) {
    return d->wait( ms );
}


bool Command::error() const {
    const QMutexLocker locker( &d->mutex );
    return !d->outputs.errorString.isEmpty();
}

bool Command::wasCanceled() const {
    const QMutexLocker locker( &d->mutex );
    return d->outputs.canceled;
}

QString Command::errorString() const {
    const QMutexLocker locker( &d->mutex );
    return d->outputs.errorString;
}


qint64 Command::serverPid() const {
    const QMutexLocker locker( &d->mutex );
    return d->outputs.serverPid;
}


void Command::start() {
    d->start();
}

void Command::cancel() {
    qDebug( "Sorry, not implemented: KleopatraClient::Command::Cancel" );
}


void Command::setOptionValue( const char * name, const QVariant & value, bool critical ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );
    const Private::Option opt = {
        value,
        true,
        critical
    };
    d->inputs.options[name] = opt;
}

QVariant Command::optionValue( const char * name ) const {
    if ( !name || !*name )
        return QVariant();
    const QMutexLocker locker( &d->mutex );

    const std::map<std::string,Private::Option>::const_iterator it = d->inputs.options.find( name );
    if ( it == d->inputs.options.end() )
        return QVariant();
    else
        return it->second.value;
}


void Command::setOption( const char * name, bool critical ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );

    if ( isOptionSet( name ) )
        unsetOption( name );

    const Private::Option opt = {
        QVariant(),
        false,
        critical
    };

    d->inputs.options[name] = opt;
}

void Command::unsetOption( const char * name ) {
    if ( !name || !*name )
        return;
    const QMutexLocker locker( &d->mutex );
    d->inputs.options.erase( name );
}

bool Command::isOptionSet( const char * name ) const {
    if ( !name || !*name )
        return false;
    const QMutexLocker locker( &d->mutex );
    return d->inputs.options.count( name );
}

bool Command::isOptionCritical( const char * name ) const {
    if ( !name || !*name )
        return false;
    const QMutexLocker locker( &d->mutex );
    const std::map<std::string,Private::Option>::const_iterator it = d->inputs.options.find( name );
    return it != d->inputs.options.end() && it->second.isCritical;
}

void Command::setFilePaths( const QStringList & filePaths ) {
    const QMutexLocker locker( &d->mutex );
    d->inputs.filePaths = filePaths;
}

QStringList Command::filePaths() const {
    const QMutexLocker locker( &d->mutex );
    return d->inputs.filePaths;
}

QByteArray Command::receivedData() const {
    const QMutexLocker locker( &d->mutex );
    return d->outputs.data;
}


void Command::setCommand( const char * command ) {
    const QMutexLocker locker( &d->mutex );
    d->inputs.command = command;
}

QByteArray Command::command() const {
    const QMutexLocker locker( &d->mutex );
    return d->inputs.command;
}

//
// here comes the ugly part
//

typedef shared_ptr< remove_pointer<assuan_context_t>::type > AssuanContextBase;
namespace {
    struct AssuanClientContext : AssuanContextBase {
        AssuanClientContext() : AssuanContextBase() {}
        explicit AssuanClientContext( assuan_context_t ctx ) : AssuanContextBase( ctx, &assuan_disconnect ) {}
        void reset( assuan_context_t ctx=0 ) { AssuanContextBase::reset( ctx, &assuan_disconnect ); }
    };
}

static assuan_error_t
my_assuan_transact( const AssuanClientContext & ctx,
                    const char *command,
                    int (*data_cb)( void *, const void *, size_t )=0,
                    void * data_cb_arg=0,
                    int (*inquire_cb)( void *, const char * )=0,
                    void * inquire_cb_arg=0,
                    int (*status_cb)( void *, const char * )=0,
                    void * status_cb_arg=0)
{
    return assuan_transact( ctx.get(), command, data_cb, data_cb_arg, inquire_cb, inquire_cb_arg, status_cb, status_cb_arg );
}

static QString to_error_string( int err ) {
    char buffer[1024];
    gpg_strerror_r( static_cast<gpg_error_t>(err), 
                    buffer, sizeof buffer );
    buffer[sizeof buffer - 1] = '\0';
    return QString::fromLocal8Bit( buffer );
}

static QString gnupg_home_directory() {
#ifdef Q_OS_WIN
    return QFile::decodeName( default_homedir() );
#else
    const QByteArray gnupgHome = qgetenv( "GNUPGHOME" );
    if ( !gnupgHome.isEmpty() )
        return QFile::decodeName( gnupgHome );
    else
        return QDir::homePath() + QLatin1String( "/.gnupg" );
#endif
}

static QString get_default_socket_name() {
    const QString homeDir = gnupg_home_directory();
    if ( homeDir.isEmpty() )
        return QString();
    return QDir( homeDir ).absoluteFilePath( QLatin1String( "S.uiserver" ) );
}

static QString default_socket_name() {
    static QString name = get_default_socket_name();
    return name;
}

static QString start_uiserver() {
    return Command::tr("start_uiserver: not yet implemented");
}

static int getinfo_pid_cb( void * opaque, const void * buffer, size_t length ) {
    qint64 & pid = *static_cast<qint64*>( opaque );
    pid = QByteArray( static_cast<const char*>( buffer ), length ).toLongLong();
    return 0;
}

static int command_data_cb( void * opaque, const void * buffer, size_t length ) {
    QByteArray & ba = *static_cast<QByteArray*>( opaque );
    ba.append( QByteArray( static_cast<const char*>(buffer), length ) );
    return 0;
}

static int send_option( const AssuanClientContext & ctx, const char * name, const QVariant & value ) {
    if ( value.isValid() )
        return my_assuan_transact( ctx, QString().sprintf( "OPTION %s=%s", name, value.toString().toUtf8().constData() ).toUtf8().constData() );
    else
        return my_assuan_transact( ctx, QString().sprintf( "OPTION %s", name ).toUtf8().constData() );
}

static int send_file( const AssuanClientContext & ctx, const QString & file ) {
    return my_assuan_transact( ctx, QString().sprintf( "FILE %s", hexencode( QFile::encodeName( file ) ).constData() ).toUtf8().constData() );
}

void Command::Private::run() {

    // Take a snapshot of the input data, and clear the output data:
    Inputs in;
    Outputs out;
    {
        const QMutexLocker locker( &mutex );
        in = inputs;
        outputs = out;
    }

    out.canceled = false;

    int err = 0;

    assuan_context_t naked_ctx = 0;
    AssuanClientContext ctx;

    if ( out.serverLocation.isEmpty() )
        out.serverLocation = default_socket_name();

    const QString socketName = out.serverLocation;
    if ( socketName.isEmpty() ) {
        out.errorString = tr("Invalid socket name!");
        goto leave;
    }

    err = assuan_socket_connect( &naked_ctx, QFile::encodeName( socketName ).constData(), -1 );
    if ( err ) {
        qDebug( "UI server not running, starting it" );
        
        const QString errorString = start_uiserver();
        if ( !errorString.isEmpty() ) {
            out.errorString = errorString;
            goto leave;
        }

        // give it a bit of time to start up and try a couple of times
        for ( int i = 0 ; err && i < 20 ; ++i ) {
            msleep( 500 );
            err = assuan_socket_connect( &naked_ctx, QFile::encodeName( socketName ).constData(), -1 );
        }
    }

    if ( err ) {
        out.errorString = tr( "Could not connect to Kleopatra UI server at %1: %2" )
            .arg( socketName, to_error_string( err ) );
        goto leave;
    }

    ctx.reset( naked_ctx );
    naked_ctx = 0;

    out.serverPid = -1;
    err = my_assuan_transact( ctx, "GETINFO pid", &getinfo_pid_cb, &out.serverPid );
    if ( err || out.serverPid <= 0 ) {
        out.errorString = tr( "Could not get the process-id of the Kleopatra UI server at %1: %2" )
            .arg( socketName, to_error_string( err ) );
        goto leave;
    }

    qDebug() << "Server PID =" << out.serverPid;

#ifdef Q_OS_WIN
    if ( !AllowSetForegroundWindow( (pid_t)out.serverPid ) )
        qDebug() << "AllowSetForegroundWindow(" << out.serverPid << ") failed: " << GetLastError();
#endif

    if ( in.command.isEmpty() )
        goto leave;

    if ( in.parentWId ) {
#ifdef Q_OS_WIN32
        err = send_option( ctx, "window-id", QString().sprintf( "%lx", reinterpret_cast<unsigned long>( in.parentWId ) ) );
#else
        err = send_option( ctx, "window-id", QString().sprintf( "%lx", static_cast<unsigned long>( in.parentWId ) ) );
#endif
        if ( err )
            qDebug( "sending option window-id failed - ignoring" );
    }

    for ( std::map<std::string,Option>::const_iterator it = in.options.begin(), end = in.options.end() ; it != end ; ++it )
        if ( ( err = send_option( ctx, it->first.c_str(), it->second.hasValue ? it->second.value.toString() : QVariant() ) ) )
            if ( it->second.isCritical ) {
                out.errorString = tr("Failed to send critical option %1: %2")
                    .arg( QString::fromLatin1( it->first.c_str() ), to_error_string( err ) );
                goto leave;
            } else {
                qDebug() << "Failed to send non-critical option" << it->first.c_str() << ":" << to_error_string( err );
            }

    Q_FOREACH( const QString & filePath, in.filePaths )
        if ( ( err = send_file( ctx, filePath ) ) ) {
            out.errorString = tr("Failed to send file path %1: %2")
                .arg( filePath, to_error_string( err ) );
            goto leave;
        }

#if 0
    setup I/O;
#endif

    err = my_assuan_transact( ctx, in.command.constData(), &command_data_cb, &out.data );
    if ( err ) {
        if ( gpg_err_code( err ) == GPG_ERR_CANCELED )
            out.canceled = true;
        else
            out.errorString = tr( "Command (%1) failed: %2" )
                .arg( QString::fromLatin1( in.command.constData() ) ).arg( to_error_string( err ) );
        goto leave;
    }
                                    

 leave:
    const QMutexLocker locker( &mutex );
    // copy outputs to where Command can see them:
    outputs = out;
}

#include "moc_command_p.cpp"
#include "moc_command.cpp"
