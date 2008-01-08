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

#include <KLocale>

#include <QString>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;
using namespace GpgME;

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
        QMetaObject::invokeMethod( this, "error", Qt::QueuedConnection, Q_ARG( int, e.error().encodedError() ), Q_ARG( QString, QString::fromLocal8Bit( e.what() ) ) );
    }
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


#include "moc_task.cpp"


