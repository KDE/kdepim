/* -*- mode: c++; c-basic-offset:4 -*-
    utils/log.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#include "log.h"
#include "iodevicelogger.h"

#include <kleo/exception.h>

#include <KLocalizedString>
#include <KRandom>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QString>
#include <QDebug>

#include <boost/weak_ptr.hpp>
#include <cassert>
#include <cstdio>

using namespace boost;
using namespace Kleo;

class Log::Private {
    Log* const q;
public:
    explicit Private( Log* qq ) : q( qq ), m_ioLoggingEnabled( false ), m_logFile( 0 ) {}
    ~Private();
    bool m_ioLoggingEnabled;
    QString m_outputDirectory;
    FILE* m_logFile;
};

Log::Private::~Private()
{
    if ( m_logFile )
        fclose( m_logFile );
}

void Log::messageHandler( QtMsgType type, const char* msg )
{
    Q_UNUSED( type )
    FILE* const file = Log::instance()->logFile();
    if ( !file ) {
        fprintf( stderr, "Log::messageHandler[!file]: %s", msg );
        return;
    }
    
    qint64 toWrite = strlen( msg );
    while ( toWrite > 0 )
    {
        const qint64 written = fprintf( file, "%s", msg );
        if ( written == -1 )
            return;
        toWrite -= written;
    }
    //append newline:
    while ( fprintf( file, "\n" ) == 0 ) ;
    fflush( file );
}

shared_ptr<const Log> Log::instance() {
    return mutableInstance();
}

shared_ptr<Log> Log::mutableInstance() {
    static weak_ptr<Log> self;
    try {
        return shared_ptr<Log>( self );
    } catch ( const bad_weak_ptr & ) {
        const shared_ptr<Log> s( new Log );
        self = s;
        return s;
    }
}

Log::Log() : d( new Private( this ) )
{    
}

Log::~Log()
{
}

FILE* Log::logFile() const
{
    return d->m_logFile;
}

void Log::setIOLoggingEnabled( bool enabled )
{
    d->m_ioLoggingEnabled = enabled;
}

bool Log::ioLoggingEnabled() const
{
    return d->m_ioLoggingEnabled;
}

QString Log::outputDirectory() const
{
    return d->m_outputDirectory;
}

void Log::setOutputDirectory( const QString& path )
{
    if ( d->m_outputDirectory == path )
        return;
    d->m_outputDirectory = path;
    assert( !d->m_logFile );
    const QString lfn = path + QLatin1String("/kleo-log");
    d->m_logFile = fopen( QDir::toNativeSeparators( lfn ).toLocal8Bit().constData(), "a" );
    assert( d->m_logFile );
}

shared_ptr<QIODevice> Log::createIOLogger( const shared_ptr<QIODevice>& io, const QString& prefix, OpenMode mode ) const
{
    if ( !d->m_ioLoggingEnabled )
        return io;
    
    shared_ptr<IODeviceLogger> logger( new IODeviceLogger( io ) );

    const QString timestamp = QDateTime::currentDateTime().toString( QLatin1String("yyMMdd-hhmmss") );
    
    const QString fn = d->m_outputDirectory + QLatin1Char('/') + prefix + QLatin1Char('-') + timestamp + QLatin1Char('-') + KRandom::randomString( 4 );
    shared_ptr<QFile> file( new QFile( fn ) );

    if ( !file->open( QIODevice::WriteOnly ) )
        throw Exception( gpg_error( GPG_ERR_EIO ), i18n( "Log Error: Could not open log file \"%1\" for writing.", fn ) );
 
    if ( mode & Read )    
        logger->setReadLogDevice( file );
    else // Write
        logger->setWriteLogDevice( file );
    
    return logger;
}

