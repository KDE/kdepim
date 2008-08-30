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

#ifndef QSYNC_PLUGINCONNECTION_H
#define QSYNC_PLUGINCONNECTION_H

#include <libqopensync/qopensync_export.h>

#include <QtCore/QString>

class OSyncPluginConnection;

namespace QSync {

class QSYNC_EXPORT PluginConnection
{
    friend class PluginConfig;

  public:
    enum ConnectionType
    {
      UnknownConnection = 0,
      BlueToothConnection,
      UsbConnection,
      NetworkConnection,
      SerialConnection,
      IrdaConnection
    };

    PluginConnection();
    ~PluginConnection();

    /**
      Returns whether the object is a valid plugin connection.
     */
    bool isValid() const;

    /**
      Returns the type of the plugin connection.
     */
    ConnectionType type() const;

    /**
      Sets the bluetooth address of the connection.
     */
    void setBluetoothAddress( const QString &address );

    /**
      Returns the bluetooth address of the connection.
     */
    QString bluetoothAddress() const;

    /**
      Sets the bluetooth channel of the connection.
     */
    void setBluetoothChannel( unsigned int channel );

    /**
      Returns the bluetooth channel of the connection.
     */
    unsigned int bluetoothChannel() const;

    /**
      Sets the bluetooth sdp uuid of the connection.
     */
    void setBluetoothSdpUuid( const QString &sdpuuid );

    /**
      Returns the bluetooth spd uuid of the connection.
     */
    QString bluetoothSdpUuid() const;

    /**
      Sets the usb vendor id of the connection.
     */
    void setUsbVendorId( unsigned int vendorId );

    /**
      Returns the usb vendor id of the connection.
     */
    unsigned int usbVendorId() const;

    /**
      Sets the usb product id of the connection.
     */
    void setUsbProductId( unsigned int productId );

    /**
      Returns the usb product id of the connection.
     */
    unsigned int usbProductId() const;

    /**
      Sets the usb interface of the connection.
     */
    void setUsbInterface( unsigned int interface );

    /**
      Returns the usb interface of the connection.
     */
    unsigned int usbInterface() const;

    /**
      Sets the network address of the connection.
     */
    void setNetworkAddress( const QString &address );

    /**
      Returns the network address of the connection.
     */
    QString networkAddress() const;

    /**
      Sets the network port of the connection.
     */
    void setNetworkPort( unsigned int port );

    /**
      Returns the network port of the connection.
     */
    unsigned int networkPort() const;

    /**
      Sets the network protocol of the connection.
     */
    void setNetworkProtocol( const QString &protocol );

    /**
      Returns the network protocol of the connection.
     */
    QString networkProtocol() const;

    /**
      Sets the network DNS SD of the connection.
     */
    void setNetworkDnsSd( const QString &dnssd );

    /**
      Returns the network DNS SD of the connection.
     */
    QString networkDnsSd() const;

    /**
      Sets the serial speed of the connection.
     */
    void setSerialSpeed( unsigned int speed );

    /**
      Returns the serial speed of the connection.
     */
    unsigned int serialSpeed() const;

    /**
      Sets the serial device node of the connection.
     */
    void setSerialDeviceNode( const QString &device );

    /**
      Returns the serial device node of the connection.
     */
    QString serialDeviceNode() const;

    /**
      Sets the IRDA service of the connection.
     */
    void setIrdaService( const QString &service );

    /**
      Returns the IRDA service of the connection.
     */
    QString irdaService() const;

  private:
    OSyncPluginConnection *mPluginConnection;
};

}

#endif
