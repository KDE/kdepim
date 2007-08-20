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

#include "deviceitem.h"

#include <QtGui/QAction>

#include <KLocale>
#include <KIcon>

#include <libkmobiletools/deviceloader.h>

DeviceItem::DeviceItem( const QString& name, TreeItem* parent )
: TreeItem( name, parent )
{
    m_engine = KMobileTools::DeviceLoader::instance()->engine( name );

    m_connectDeviceAction = new QAction( i18n( "Connect device" ), this );
    m_disconnectDeviceAction = new QAction( i18n( "Disconnect device" ), this );

    m_connectDeviceAction->setEnabled( !m_engine->connected() );
    m_disconnectDeviceAction->setEnabled( m_engine->connected() );

    connect( m_connectDeviceAction, SIGNAL(triggered()), this, SLOT(connectDevice()) );
    connect( m_disconnectDeviceAction, SIGNAL(triggered()), this, SLOT(disconnectDevice()) );

    connect( m_engine, SIGNAL(deviceConnected()), this, SLOT(deviceConnected()) );
    connect( m_engine, SIGNAL(deviceDisconnected()), this, SLOT(deviceDisconnected()) );

    m_actionList.append( m_connectDeviceAction );
    m_actionList.append( m_disconnectDeviceAction );

    // set icon for deviceItem
    KPluginInfo deviceInformation = KMobileTools::DeviceLoader::instance()->engineInformation( name );
    setIcon( KIcon( deviceInformation.icon() ) );

    /// @todo use KIconEffect to display the connection state (convert to gray-scale on disconnection)
}


DeviceItem::~DeviceItem()
{
}

QList<QAction*> DeviceItem::actionList() const {
    kDebug() << "action list requested" << endl;
    return m_actionList;
}

void DeviceItem::connectDevice() {
    /// @todo add a timer or something if connecting fails...
    m_connectDeviceAction->setEnabled( false );
    m_engine->connectDevice();
}

void DeviceItem::disconnectDevice() {
    m_disconnectDeviceAction->setEnabled( false );
    m_engine->disconnectDevice();
}

void DeviceItem::deviceConnected() {
    m_connectDeviceAction->setEnabled( false );
    m_disconnectDeviceAction->setEnabled( true );
}

void DeviceItem::deviceDisconnected() {
    m_connectDeviceAction->setEnabled( true );
    m_disconnectDeviceAction->setEnabled( false );
}

#include "deviceitem.moc"
