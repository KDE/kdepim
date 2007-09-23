/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "jobsignalmapper.h"
#include <KDE/KDebug>

JobSignalMapper::JobSignalMapper( QObject* parent )
    : QObject( parent )
{
}

JobSignalMapper::~JobSignalMapper()
{
}

void JobSignalMapper::setMapping( QObject* signalOrigin, const QString& deviceName ) {
    kDebug() << "setMapping(): " << signalOrigin << ", " << deviceName;
    m_signals[signalOrigin] = deviceName;
    connect( signalOrigin, SIGNAL(destroyed()), this, SLOT(signalOriginDestroyed()) );
}

void JobSignalMapper::removeMapping( const QString& deviceName ) {
    kDebug() << "removeMapping:" << deviceName;
    QObject* signalOrigin = m_signals.key( deviceName, 0 );
    if( signalOrigin )
        m_signals.remove( signalOrigin );
}

QString JobSignalMapper::mapping( QObject* signalOrigin ) const {
    kDebug() << "mapping(): " << signalOrigin;
    return m_signals.value( signalOrigin );
}

void JobSignalMapper::signalOriginDestroyed() {
    kDebug() << "signalOriginDestroyed()";
    if( m_signals.contains( sender() ) ) {
        QString deviceName = m_signals.value( sender() );
        removeMapping( deviceName );
    }
}

void JobSignalMapper::map( KMobileTools::JobXP* job ) {
    if( m_signals.contains( sender() ) )
        emit mapped( m_signals.value( sender() ), job );
}

#include "jobsignalmapper.moc"
