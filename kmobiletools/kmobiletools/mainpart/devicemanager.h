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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <KDE/KDialog>

class KListWidget;
class KPushButton;
/**
 * @author Marco Gulino
 * @author Matthias Lechner
 */
class DeviceManager : public KDialog
{
Q_OBJECT
public:
    DeviceManager( QWidget *parent = 0 );
    ~DeviceManager();

private Q_SLOTS:
    void populateDeviceList();
    void checkEnableButtons();

    void addDevice();
    void removeDevice();
    void deviceProperties();

    void addDeviceItem( const QString& deviceName );
    void removeDeviceItem( const QString& deviceName );

public slots:
    void slotRemoveDevice();
    void slotDeviceProperties();
    void slotNewDevice();
    //void doubleClickedItem(Q3ListViewItem *item);
    //void slotItemRenamed ( Q3ListViewItem * item, int col, const QString & text );

signals:
    void deviceAdded(const QString&);
    void deviceRemoved(const QString&);
    void loadDevice(const QString&);
    void unloadDevice(const QString&);

private:
    KListWidget* m_deviceList;
    KPushButton* m_addDevice;
    KPushButton* m_removeDevice;
    KPushButton* m_deviceProperties;

    void setupGUI();
};


#endif
