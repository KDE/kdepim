/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>


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

public:
    enum Connection { ConnectionUSB=0x1, ConnectionSerial=0x2, ConnectionBluetooth=0x4, ConnectionIrDA=0x8, ConnectionUser=0x10 };

    int at_connections() {
        int r=0;
        if( at_connUSB() ) r|=ATDevicesConfig::ConnectionUSB;
        if( at_connIrdA() ) r|=ATDevicesConfig::ConnectionIrDA;
        if( at_connSerial() ) r|=ATDevicesConfig::ConnectionSerial;
        if( at_connBluetooth() ) r|=ATDevicesConfig::ConnectionBluetooth;
        if( at_connCustom() ) r|=ATDevicesConfig::ConnectionUser;
        return r;
    }
    void setAt_connections(int c) {
        setAt_connUSB(c & ATDevicesConfig::ConnectionUSB);
        setAt_connIrdA(c & ATDevicesConfig::ConnectionIrDA);
        setAt_connSerial(c & ATDevicesConfig::ConnectionSerial);
        setAt_connBluetooth(c & ATDevicesConfig::ConnectionBluetooth);
        setAt_connCustom(c & ATDevicesConfig::ConnectionUser);
    }

