/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/task.cpp

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

#include "task.h"

#include "kleo-assuan.h"

#include <KIconLoader>
#include <KLocale>

#include <QString>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

namespace {

    class ErrorResult : public Task::Result {
    public:
        ErrorResult( int code, const QString & details )
            : Task::Result(), m_code( code ), m_details( details ) {}

        /* reimp */ QString overview() const { return makeSimpleOverview( m_details, Error ); }
        /* reimp */ QString details() const { return QString(); }
        /* reimp */ int errorCode() const { return m_code; }
        /* reimp */ QString errorString() const { return m_details; }
        
    private:
        int m_code;
        QString m_details;
    };
}

class Task::Private {
    friend class ::Kleo::Task;
    Task * const q;
public:
    explicit Private( Task * qq );

private:
    // ### 
};

Task::Private::Private( Task * qq )
    : q( qq )
{

}

Task::Task( QObject * p )
    : QObject( p ), d( new Private( this ) )
{

}

Task::~Task() {}

void Task::start() {
    try {
        doStart();
    } catch ( const GpgME::Exception & e ) {
        QMetaObject::invokeMethod( this, "emitError", Qt::QueuedConnection, Q_ARG( int, e.error().encodedError() ), Q_ARG( QString, QString::fromLocal8Bit( e.what() ) ) );
    }
}

void Task::emitError( int errCode, const QString& details ) {
    emit result( makeErrorResult( errCode, details ) );
}

boost::shared_ptr<Task::Result> Task::makeErrorResult( int errCode, const QString& details )
{
    return boost::shared_ptr<Task::Result>( new ErrorResult( errCode, details ) );
}

static QString makeNonce() {
    // ### make better
    return QString::number( qrand(), 16 );
}

Task::Result::Result() : m_nonce( makeNonce() ) {}
Task::Result::~Result() {}

QString Task::Result::formatKeyLink( const char * fpr, const QString & content ) const {
    return "<a href=\"key:" + m_nonce + ':' + fpr + "\">" + content + "</a>";
}

bool Task::Result::hasError() const
{
    return errorCode() != 0;
}

static QString image( const char* img ) {
    // ### escape?
    return KIconLoader::global()->iconPath( img, KIconLoader::Small );
}


QString Task::Result::makeSimpleOverview( const QString& desc, ErrorLevel level )
{
    QString img;
    switch ( level ) {
        case Error:
            img = image( "dialog-error" );
            break;
        case NoError:
            img = image( "dialog-ok" );
            break;
        case Warning:
            img = image( "dialog-warning" );
            break;
    }
    
    return QString( "<img src=\"%1\"/><b>%2</b>" ).arg( img, desc );
}

#include "moc_task.cpp"


