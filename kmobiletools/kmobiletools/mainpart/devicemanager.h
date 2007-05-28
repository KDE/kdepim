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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "ui_deviceList.h"

#include <kdialog.h>

/**
@author Marco Gulino
*/
class K3IconView;
class Q3ListViewItem;
class QStringList;
class deviceList;

class DeviceManager : public KDialog
{
Q_OBJECT
public:
    explicit DeviceManager(QWidget *parent = 0, const char *name = 0);

    ~DeviceManager();
    int showDeviceConfigDialog(const QString &deviceName, bool newdevice=false);

public slots:
    void updateView();
    void slotRemoveDevice();
    void slotDeviceProperties();
    void slotNewDevice();
    void doubleClickedItem(Q3ListViewItem *item);
    void slotItemRenamed ( Q3ListViewItem * item, int col, const QString & text );
    void selectionChanged ();
    void deviceToggled(bool);
    void deviceChanged(const QString &);

    private:
    Ui::deviceList ui; QWidget w_ui;
signals:
    void deviceAdded(const QString&);
    void deviceRemoved(const QString&);
    void loadDevice(const QString&);
    void unloadDevice(const QString&);
};


#endif
