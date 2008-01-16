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

#include "log.h"

#include "iodevicelogger.h"
#include "config-kleopatra.h"

#ifdef HAVE_ASSUAN
#include <uiserver/kleo-assuan.h>
#endif

#include <KLocalizedString>
#include <KRandom>

#include <QDateTime>
#include <QFile>
#include <QString>
#include <QDebug>

#include <boost/weak_ptr.hpp>
#include <exception>

using namespace boost;
using namespace Kleo;

class Log::Private {
    Log* const q;
public:
    explicit Private( Log* qq ) : q( qq ), m_ioLoggingEnabled( false ) {}
    
    bool m_ioLoggingEnabled;
    QString m_outputDirectory;
};


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
    d->m_outputDirectory = path;
}

shared_ptr<QIODevice> Log::createIOLogger( const shared_ptr<QIODevice>& io, const QString& prefix, OpenMode mode ) const
{
    if ( !d->m_ioLoggingEnabled )
        return io;
    
    shared_ptr<IODeviceLogger> logger( new IODeviceLogger( io ) );

    const QString timestamp = QDateTime::currentDateTime().toString( "yyMMdd-hhmmss" );
    
    const QString fn = d->m_outputDirectory + '/' + prefix + '-' + timestamp + '-' + KRandom::randomString( 4 );
    shared_ptr<QFile> file( new QFile( fn ) );

    if ( !file->open( QIODevice::WriteOnly ) )
    {
        QString errorMessage = i18n( "Log Error: Couldn't open log file \"%1\" for write", fn );
#ifdef HAVE_ASSUAN
        throw assuan_exception( gpg_error( GPG_ERR_EIO ), errorMessage );
#else
        qDebug()<<errorMessage;
        return io;
#endif
    }
    if ( mode & Read )    
        logger->setReadLogDevice( file );
    else // Write
        logger->setWriteLogDevice( file );
    
    return logger;
}

