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

#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include <QtCore/QString>
#include <QtCore/QList>
#include <libkmobiletools/enginexp.h>

#include "treeitem.h"

class QAction;
/**
 * @author Matthias Lechner <matthias@lmme.de>
 */
class DeviceItem : public TreeItem {
    Q_OBJECT
public:
    DeviceItem( const QString& name, TreeItem* parent );
    ~DeviceItem();

    QList<QAction*> actionList() const;

private Q_SLOTS:
    void connectDevice();
    void disconnectDevice();

    void deviceConnected();
    void deviceDisconnected();

private:
    QList<QAction*> m_actionList;
    QAction* m_connectDeviceAction;
    QAction* m_disconnectDeviceAction;

    KMobileTools::EngineXP* m_engine;
};

#endif
