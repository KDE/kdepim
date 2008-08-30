/*
    This file is part of libqopensync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

#include "pluginconnection.h"

using namespace QSync;

PluginConnection::PluginConnection()
  : mPluginConnection( 0 )
{
}

PluginConnection::~PluginConnection()
{
}

bool PluginConnection::isValid() const
{
  return (mPluginConnection != 0);
}

PluginConnection::ConnectionType PluginConnection::type() const
{
  Q_ASSERT( mPluginConnection );

  const OSyncPluginConnectionType type = osync_plugin_connection_get_type( mPluginConnection );

  ConnectionType connType = UnknownConnection;
  switch ( type ) {
    case OSYNC_PLUGIN_CONNECTION_UNKNOWN: connType = UnknownConnection; break;
    case OSYNC_PLUGIN_CONNECTION_BLUETOOTH: connType = BlueToothConnection; break;
    case OSYNC_PLUGIN_CONNECTION_USB: connType = UsbConnection; break;
    case OSYNC_PLUGIN_CONNECTION_NETWORK: connType = NetworkConnection; break;
    case OSYNC_PLUGIN_CONNECTION_SERIAL: connType = SerialConnection; break;
    case OSYNC_PLUGIN_CONNECTION_IRDA: connType = IrdaConnection; break;
  }

  return connType;
}

void PluginConnection::setBluetoothAddress( const QString &address )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_bt_set_addr( mPluginConnection, address.toLatin1().data() );
}

QString PluginConnection::bluetoothAddress() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_bt_get_addr( mPluginConnection ) );
}

void PluginConnection::setBluetoothChannel( unsigned int channel )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_bt_set_channel( mPluginConnection, channel );
}

unsigned int PluginConnection::bluetoothChannel() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_bt_get_channel( mPluginConnection );
}

void PluginConnection::setBluetoothSdpUuid( const QString &sdpuuid )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_bt_set_sdpuuid( mPluginConnection, sdpuuid.toLatin1().data() );
}

QString PluginConnection::bluetoothSdpUuid() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_bt_get_sdpuuid( mPluginConnection ) );
}

void PluginConnection::setUsbVendorId( unsigned int vendorId )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_usb_set_vendorid( mPluginConnection, vendorId );
}

unsigned int PluginConnection::usbVendorId() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_usb_get_vendorid( mPluginConnection );
}

void PluginConnection::setUsbProductId( unsigned int productId )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_usb_set_productid( mPluginConnection, productId );
}

unsigned int PluginConnection::usbProductId() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_usb_get_productid( mPluginConnection );
}

void PluginConnection::setUsbInterface( unsigned int interface )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_usb_set_interface( mPluginConnection, interface );
}

unsigned int PluginConnection::usbInterface() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_usb_get_interface( mPluginConnection );
}

void PluginConnection::setNetworkAddress( const QString &address )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_net_set_address( mPluginConnection, address.toLatin1().data() );
}

QString PluginConnection::networkAddress() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_net_get_address( mPluginConnection ) );
}

void PluginConnection::setNetworkPort( unsigned int port )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_net_set_port( mPluginConnection, port );
}

unsigned int PluginConnection::networkPort() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_net_get_port( mPluginConnection );
}

void PluginConnection::setNetworkProtocol( const QString &protocol )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_net_set_protocol( mPluginConnection, protocol.toLatin1().data() );
}

QString PluginConnection::networkProtocol() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_net_get_protocol( mPluginConnection ) );
}

void PluginConnection::setNetworkDnsSd( const QString &dnssd )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_net_set_dnssd( mPluginConnection, dnssd.toLatin1().data() );
}

QString PluginConnection::networkDnsSd() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_net_get_dnssd( mPluginConnection ) );
}

void PluginConnection::setSerialSpeed( unsigned int speed )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_serial_set_speed( mPluginConnection, speed );
}

unsigned int PluginConnection::serialSpeed() const
{
  Q_ASSERT( mPluginConnection );

  return osync_plugin_connection_serial_get_speed( mPluginConnection );
}

void PluginConnection::setSerialDeviceNode( const QString &device )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_serial_set_devicenode( mPluginConnection, device.toLatin1().data() );
}

QString PluginConnection::serialDeviceNode() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_serial_get_devicenode( mPluginConnection ) );
}

void PluginConnection::setIrdaService( const QString &service )
{
  Q_ASSERT( mPluginConnection );

  osync_plugin_connection_irda_set_service( mPluginConnection, service.toLatin1().data() );
}

QString PluginConnection::irdaService() const
{
  Q_ASSERT( mPluginConnection );

  return QString::fromLatin1( osync_plugin_connection_irda_get_service( mPluginConnection ) );
}
