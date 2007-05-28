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
#include "deviceslist.h"
#include "devicehome.h"
//Added by qt3to4:
#include <Q3PtrList>
DevicesList::DevicesList()
    : Q3PtrList<DeviceHome>()
{
}


DevicesList::~DevicesList()
{
    setAutoDelete(true);
}


/*!
    \fn DevicesList::find(const QString &deviceName)
 */
int DevicesList::find(const QString &deviceName)
{
    int found=0;
    DeviceHome *curDevice=0;
    Q3PtrListIterator<DeviceHome> it( *this);
    while( (curDevice=it.current()) !=0 )
    {
        ++it;
        if(curDevice->objectName() == deviceName)
            return found;
        found++;
    }
    return -1;
}
/*!
    \fn DevicesList::find(QWidget *deviceWidget)
 */
int DevicesList::find(QWidget *deviceWidget)
{
    int found=0;
    DeviceHome *curDevice=0;
    Q3PtrListIterator<DeviceHome> it( *this);
    while( (curDevice=it.current()) !=0 )
    {
        ++it;
        if(curDevice->widget() == deviceWidget)
            return found;
        found++;
    }
    return -1;
}

void DevicesList::dump()
{
    DeviceHome *tempDevice;
    int i=0;
    // Parsing the current DevicesList. If some items are not found in the new one, they'll be removed
    Q3PtrListIterator<DeviceHome> it (*this);
    while( (( tempDevice=it.current()) != 0) && !this->isEmpty()  )
    {
        kDebug() << "DevicesList::dump(): " << QString("%1").arg(i,2) << "|" << tempDevice->objectName() << "|" << tempDevice->friendlyName() << endl;
        ++it; i++;
    }
}
