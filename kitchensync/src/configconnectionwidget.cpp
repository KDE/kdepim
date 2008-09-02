/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <QtGui/QFormLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QStackedLayout>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>

#include <stdio.h>

#include "configconnectionwidget.h"

static unsigned int hexStringToInt( const QString &txt )
{
  unsigned int value = 0;

  if ( ::sscanf( txt.toLatin1(), "%x", &value ) != 1 )
    value = 0;

  return value;
}

static QString intToHexString( unsigned int value )
{
  QString txt;
  txt.sprintf( "%x", value );

  return txt;
}


ConfigConnectionWidget::ConfigConnectionWidget( const QSync::PluginConnection &connection, QWidget *parent )
  : QWidget( parent ),
    mConnection( connection ),
    mBluetoothPage( 0 ), mUsbPage( 0 ), mNetworkPage( 0 ), mSerialPage( 0 ), mIrdaPage( 0 ),
    mBluetoothAddress( 0 ), mBluetoothChannel( 0 ), mBluetoothSdpUuid( 0 ),
    mUsbVendorId( 0 ), mUsbProductId( 0 ), mUsbInterface( 0 ),
    mNetworkAddress( 0 ), mNetworkPort( 0 ), mNetworkProtocol( 0 ), mNetworkDnsSd( 0 ),
    mSerialSpeed( 0 ), mSerialDevice( 0 ),
    mIrdaService( 0 )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QHBoxLayout *typeLayout = new QHBoxLayout;
  layout->addLayout( typeLayout );

  mType = new KComboBox( this );
  connect( mType, SIGNAL( activated( int ) ), this, SLOT( typeChanged( int ) ) );

  typeLayout->addWidget( new QLabel( i18n( "Type:" ) ) );
  typeLayout->addWidget( mType );

  mStack = new QStackedLayout;
  layout->addLayout( mStack );

  QFormLayout *pageLayout = 0;

  // bluetooth
  if ( connection.isTypeSupported( QSync::PluginConnection::BlueToothConnection ) ) {
    mType->addItem( i18n( "Bluetooth" ), QSync::PluginConnection::BlueToothConnection );

    mBluetoothPage = new QWidget;
    pageLayout = new QFormLayout( mBluetoothPage );

    if ( connection.isOptionSupported( QSync::PluginConnection::BluetoothAddressOption ) ) {
      mBluetoothAddress = new KLineEdit( mBluetoothPage );
      mBluetoothAddress->setInputMask( ">HH:HH:HH:HH:HH:HH;" );
      pageLayout->addRow( i18n( "Address:" ), mBluetoothAddress );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::BluetoothChannelOption ) ) {
      mBluetoothChannel = new QSpinBox( mBluetoothPage );
      mBluetoothChannel->setRange( 1, 100 );
      pageLayout->addRow( i18n( "Channel:" ), mBluetoothChannel );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::BluetoothSdpUuidOption ) ) {
      mBluetoothSdpUuid = new KLineEdit( mBluetoothPage );
      pageLayout->addRow( i18n( "SDP UUid:" ), mBluetoothSdpUuid );
    }

    mStack->addWidget( mBluetoothPage );
  }

  // usb
  if ( connection.isTypeSupported( QSync::PluginConnection::UsbConnection ) ) {
    mType->addItem( i18n( "USB" ), QSync::PluginConnection::UsbConnection );

    mUsbPage = new QWidget;
    pageLayout = new QFormLayout( mUsbPage );

    if ( connection.isOptionSupported( QSync::PluginConnection::UsbVendorIdOption ) ) {
      mUsbVendorId = new KLineEdit( mUsbPage );
      mUsbVendorId->setInputMask( "\\0\\xHHHH" );
      pageLayout->addRow( i18n( "Vendor ID:" ), mUsbVendorId );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::UsbProductIdOption ) ) {
      mUsbProductId = new KLineEdit( mUsbPage );
      mUsbProductId->setInputMask( "\\0\\xHHHH" );
      pageLayout->addRow( i18n( "Product ID:" ), mUsbProductId );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::UsbInterfaceOption ) ) {
      mUsbInterface = new QSpinBox( mUsbPage );
      pageLayout->addRow( i18n( "Interface:" ), mUsbInterface );
    }

    mStack->addWidget( mUsbPage );
  }

  // network
  if ( connection.isTypeSupported( QSync::PluginConnection::NetworkConnection ) ) {
    mType->addItem( i18n( "Network" ), QSync::PluginConnection::NetworkConnection );

    mNetworkPage = new QWidget;
    pageLayout = new QFormLayout( mNetworkPage );

    if ( connection.isOptionSupported( QSync::PluginConnection::NetworkAddressOption ) ) {
      mNetworkAddress = new KLineEdit( mNetworkPage );
      pageLayout->addRow( i18n( "Address:" ), mNetworkAddress );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::NetworkPortOption ) ) {
      mNetworkPort = new QSpinBox( mNetworkPage );
      mNetworkPort->setRange( 1, 65535 );
      pageLayout->addRow( i18n( "Port:" ), mNetworkPort );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::NetworkProtocolOption ) ) {
      mNetworkProtocol = new KLineEdit( mNetworkPage );
      pageLayout->addRow( i18n( "Protocol:" ), mNetworkProtocol );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::NetworkDnsSdOption ) ) {
      mNetworkDnsSd = new KLineEdit( mNetworkPage );
      pageLayout->addRow( i18n( "DnsSd:" ), mNetworkDnsSd );
    }

    mStack->addWidget( mNetworkPage );
  }

  // serial
  if ( connection.isTypeSupported( QSync::PluginConnection::SerialConnection ) ) {
    mType->addItem( i18n( "Serial" ), QSync::PluginConnection::SerialConnection );

    mSerialPage = new QWidget;
    pageLayout = new QFormLayout( mSerialPage );

    if ( connection.isOptionSupported( QSync::PluginConnection::SerialSpeedOption ) ) {
      mSerialSpeed = new KLineEdit( mSerialPage );
      pageLayout->addRow( i18n( "Speed:" ), mSerialSpeed );
    }

    if ( connection.isOptionSupported( QSync::PluginConnection::SerialDeviceNodeOption ) ) {
      mSerialDevice = new KComboBox( mSerialPage );
      mSerialDevice->setEditable( true );
      mSerialDevice->addItem( "/dev/ttyS0" );
      mSerialDevice->addItem( "/dev/ttyS1" );
      mSerialDevice->addItem( "/dev/ttyUSB0" );
      mSerialDevice->addItem( "/dev/ttyUSB1" );
      pageLayout->addRow( i18n( "Device:" ), mSerialDevice );
    }

    mStack->addWidget( mSerialPage );
  }

  // irda
  if ( connection.isTypeSupported( QSync::PluginConnection::IrdaConnection ) ) {
    mType->addItem( i18n( "IRDA" ), QSync::PluginConnection::IrdaConnection );

    mIrdaPage = new QWidget;
    pageLayout = new QFormLayout( mIrdaPage );

    if ( connection.isOptionSupported( QSync::PluginConnection::IrdaServiceOption ) ) {
      mIrdaService = new KLineEdit( mIrdaPage );
      pageLayout->addRow( i18n( "Service:" ), mIrdaService );
    }

    mStack->addWidget( mIrdaPage );
  }
}

void ConfigConnectionWidget::load()
{
  mType->setCurrentIndex( mType->findData( mConnection.type() ) );
  typeChanged( mType->currentIndex() );

  if ( mBluetoothAddress )
    mBluetoothAddress->setText( mConnection.bluetoothAddress() );
  if ( mBluetoothChannel )
    mBluetoothChannel->setValue( mConnection.bluetoothChannel() );
  if ( mBluetoothSdpUuid )
    mBluetoothSdpUuid->setText( mConnection.bluetoothSdpUuid() );

  if ( mUsbVendorId )
    mUsbVendorId->setText( intToHexString( mConnection.usbVendorId() ) );
  if ( mUsbProductId )
    mUsbProductId->setText( intToHexString( mConnection.usbProductId() ) );
  if ( mUsbInterface )
    mUsbInterface->setValue( mConnection.usbInterface() );

  if ( mNetworkAddress )
    mNetworkAddress->setText( mConnection.networkAddress() );
  if ( mNetworkPort )
    mNetworkPort->setValue( mConnection.networkPort() );
  if ( mNetworkProtocol )
    mNetworkProtocol->setText( mConnection.networkProtocol() );
  if ( mNetworkDnsSd )
    mNetworkDnsSd->setText( mConnection.networkDnsSd() );

  if ( mSerialSpeed )
    mSerialSpeed->setText( QString::number( mConnection.serialSpeed() ) );
  if ( mSerialDevice )
    mSerialDevice->setEditText( mConnection.serialDeviceNode() );

  if ( mIrdaService )
    mIrdaService->setText( mConnection.irdaService() );
}

void ConfigConnectionWidget::save()
{
  mConnection.setType( (QSync::PluginConnection::ConnectionType)mType->itemData( mType->currentIndex() ).toInt() );

  if ( mBluetoothAddress )
    mConnection.setBluetoothAddress( mBluetoothAddress->text() );
  if ( mBluetoothChannel )
    mConnection.setBluetoothChannel( mBluetoothChannel->value() );
  if ( mBluetoothSdpUuid )
    mConnection.setBluetoothSdpUuid( mBluetoothSdpUuid->text() );

  if ( mUsbVendorId )
    mConnection.setUsbVendorId( hexStringToInt( mUsbVendorId->text() ) );
  if ( mUsbProductId )
    mConnection.setUsbProductId( hexStringToInt( mUsbProductId->text() ) );
  if ( mUsbInterface )
    mConnection.setUsbInterface( mUsbInterface->value() );

  if ( mNetworkAddress )
    mConnection.setNetworkAddress( mNetworkAddress->text() );
  if ( mNetworkPort )
    mConnection.setNetworkPort( mNetworkPort->value() );
  if ( mNetworkProtocol )
    mConnection.setNetworkProtocol( mNetworkProtocol->text() );
  if ( mNetworkDnsSd )
    mConnection.setNetworkDnsSd( mNetworkDnsSd->text() );

  if ( mSerialSpeed )
    mConnection.setSerialSpeed( mSerialSpeed->text().toUInt() );
  if ( mSerialDevice )
    mConnection.setSerialDeviceNode( mSerialDevice->currentText() );

  if ( mIrdaService )
    mConnection.setIrdaService( mIrdaService->text() );
}

void ConfigConnectionWidget::typeChanged( int type )
{
  int i = mType->itemData( type ).toInt();

  switch ( i ) {
    case QSync::PluginConnection::BlueToothConnection: mStack->setCurrentWidget( mBluetoothPage ); break;
    case QSync::PluginConnection::UsbConnection: mStack->setCurrentWidget( mUsbPage ); break;
    case QSync::PluginConnection::NetworkConnection: mStack->setCurrentWidget( mNetworkPage ); break;
    case QSync::PluginConnection::SerialConnection: mStack->setCurrentWidget( mSerialPage ); break;
    case QSync::PluginConnection::IrdaConnection: mStack->setCurrentWidget( mIrdaPage ); break;
  }
}

#include "configconnectionwidget.moc"
