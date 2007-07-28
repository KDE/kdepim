/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>
   by Matthias Lechner <matthias@lmme.de>

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
#include "deviceslist.h"
#include "devicehome.h"

DevicesList::DevicesList()
    : QList<DeviceHome*>()
{
}


DevicesList::~DevicesList()
{
    qDeleteAll( begin(), end() );
}


int DevicesList::find( const QString &deviceName )
{
    for( int i = 0; i < size(); ++i ) {
        if( at(i)->objectName() == deviceName )
            return i;
    }

    return -1;
}


int DevicesList::find( const QWidget *deviceWidget )
{
    for( int i = 0; i < size(); ++i ) {
        if( at(i)->widget() == deviceWidget )
            return i;
    }

    return -1;
}
