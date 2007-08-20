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

#include "enginexp.h"

namespace KMobileTools {

EngineXP::EngineXP( QObject *parent, const QString& deviceName )
 : QObject( parent )
{
    m_deviceName = deviceName;
    m_connected = false;
    connect( this, SIGNAL(deviceConnected()), this, SLOT(setDeviceConnected()) );
    connect( this, SIGNAL(deviceDisconnected()), this, SLOT(setDeviceDisconnected()) );
}


EngineXP::~EngineXP()
{
}


bool EngineXP::implements( const QString& interfaceName ) {
    QString qualifiedInterfaceName = QString( "KMobileTools::Ifaces::%1" ).arg( interfaceName );
    if( inherits( qualifiedInterfaceName.toUtf8() ) )
        return true;

    return false;
}

bool EngineXP::connected() const {
    return m_connected;
}

void EngineXP::setDeviceConnected() {
    m_connected = true;
}

void EngineXP::setDeviceDisconnected() {
    m_connected = false;
}

QString EngineXP::deviceName() const {
    return m_deviceName;
}

}

#include "enginexp.moc"

