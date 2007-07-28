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
#ifndef DEVICESLIST_H
#define DEVICESLIST_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QWidget>

class DeviceHome;
/**
    @author Marco Gulino <marco@kmobiletools.org>
    @author Matthias Lechner <matthias@lmme.de>
*/
class DevicesList : public QList<DeviceHome*>
{
public:
    DevicesList();

    ~DevicesList();

    /**
     * Returns the index that contains the part with the specified @p deviceName
     *
     * @param deviceName the device name to search for
     */
    int find( const QString &deviceName );

    /**
     * Returns the index that contains the part with the specified @p deviceWidget
     *
     * @param deviceWidget the widget 
     */
    int find( const QWidget *deviceWidget );
};

#endif
