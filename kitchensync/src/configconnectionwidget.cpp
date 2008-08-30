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
#include <QtGui/QVBoxLayout>

#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>

#include "configconnectionwidget.h"

ConfigConnectionWidget::ConfigConnectionWidget( const QSync::PluginConnection &connection, QWidget *parent )
  : QWidget( parent ),
    mConnection( connection )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QHBoxLayout *typeLayout = new QHBoxLayout;
  layout->addLayout( typeLayout );

  mType = new KComboBox( this );
  mType->addItem( i18n( "Bluetooth" ) );
  mType->addItem( i18n( "USB" ) );
  mType->addItem( i18n( "Network" ) );
  mType->addItem( i18n( "Serial" ) );
  mType->addItem( i18n( "IRDA" ) );

  connect( mType, SIGNAL( activated( int ) ), this, SLOT( typeChanged( int ) ) );

  typeLayout->addWidget( new QLabel( i18n( "Type:" ) ) );
  typeLayout->addWidget( mType );

  mStack = new QStackedLayout;
  layout->addLayout( mStack );

  QFormLayout *pageLayout = 0;

  // bluetooth
  mBluetoothPage = new QWidget;
  pageLayout = new QFormLayout( mBluetoothPage );

  mBluetoothAddress = new KLineEdit( mBluetoothPage );
  mBluetoothChannel = new KLineEdit( mBluetoothPage );
  mBluetoothSdpUuid = new KLineEdit( mBluetoothPage );

  pageLayout->addRow( i18n( "Address:" ), mBluetoothAddress );
  pageLayout->addRow( i18n( "Channel:" ), mBluetoothChannel );
  pageLayout->addRow( i18n( "SDP UUid:" ), mBluetoothSdpUuid );

  mStack->addWidget( mBluetoothPage );

  // usb
  mUsbPage = new QWidget;
  pageLayout = new QFormLayout( mUsbPage );

  mUsbVendorId = new KLineEdit( mUsbPage );
  mUsbProductId = new KLineEdit( mUsbPage );
  mUsbInterface = new KLineEdit( mUsbPage );

  pageLayout->addRow( i18n( "Vendor ID:" ), mUsbVendorId );
  pageLayout->addRow( i18n( "Product ID:" ), mUsbProductId );
  pageLayout->addRow( i18n( "Interface:" ), mUsbInterface );

  mStack->addWidget( mUsbPage );

  // network
  mNetworkPage = new QWidget;
  pageLayout = new QFormLayout( mNetworkPage );

  mNetworkAddress = new KLineEdit( mNetworkPage );
  mNetworkPort = new KLineEdit( mNetworkPage );
  mNetworkProtocol = new KLineEdit( mNetworkPage );
  mNetworkDnsSd = new KLineEdit( mNetworkPage );

  pageLayout->addRow( i18n( "Address:" ), mNetworkAddress );
  pageLayout->addRow( i18n( "Port:" ), mNetworkPort );
  pageLayout->addRow( i18n( "Protocol:" ), mNetworkProtocol );
  pageLayout->addRow( i18n( "DnsSd:" ), mNetworkDnsSd );

  mStack->addWidget( mNetworkPage );

  // serial
  mSerialPage = new QWidget;
  pageLayout = new QFormLayout( mSerialPage );

  mSerialSpeed = new KLineEdit( mSerialPage );
  mSerialDevice = new KLineEdit( mSerialPage );

  pageLayout->addRow( i18n( "Speed:" ), mSerialSpeed );
  pageLayout->addRow( i18n( "Device:" ), mSerialDevice );

  mStack->addWidget( mSerialPage );

  // irda
  mIrdaPage = new QWidget;
  pageLayout = new QFormLayout( mIrdaPage );

  mIrdaService = new KLineEdit( mIrdaPage );

  pageLayout->addRow( i18n( "Service:" ), mIrdaService );

  mStack->addWidget( mIrdaPage );
}

void ConfigConnectionWidget::load()
{
  switch ( mConnection.type() ) {
    case QSync::PluginConnection::BlueToothConnection: mType->setCurrentIndex( 0 ); break;
    case QSync::PluginConnection::UsbConnection: mType->setCurrentIndex( 1 ); break;
    case QSync::PluginConnection::NetworkConnection: mType->setCurrentIndex( 2 ); break;
    case QSync::PluginConnection::SerialConnection: mType->setCurrentIndex( 3 ); break;
    case QSync::PluginConnection::IrdaConnection: mType->setCurrentIndex( 4 ); break;
  }

  typeChanged( mType->currentIndex() );

  mBluetoothAddress->setText( mConnection.bluetoothAddress() );
  mBluetoothChannel->setText( QString::number( mConnection.bluetoothChannel() ) );
  mBluetoothSdpUuid->setText( mConnection.bluetoothSdpUuid() );

  mUsbVendorId->setText( QString::number( mConnection.usbVendorId() ) );
  mUsbProductId->setText( QString::number( mConnection.usbProductId() ) );
  mUsbInterface->setText( QString::number( mConnection.usbInterface() ) );

  mNetworkAddress->setText( mConnection.networkAddress() );
  mNetworkPort->setText( QString::number( mConnection.networkPort() ) );
  mNetworkProtocol->setText( mConnection.networkProtocol() );
  mNetworkDnsSd->setText( mConnection.networkDnsSd() );

  mSerialSpeed->setText( QString::number( mConnection.serialSpeed() ) );
  mSerialDevice->setText( mConnection.serialDeviceNode() );

  mIrdaService->setText( mConnection.irdaService() );
}

void ConfigConnectionWidget::save()
{
  switch ( mType->currentIndex() ) {
    case 0: mConnection.setType( QSync::PluginConnection::BlueToothConnection ); break;
    case 1: mConnection.setType( QSync::PluginConnection::UsbConnection ); break;
    case 2: mConnection.setType( QSync::PluginConnection::NetworkConnection ); break;
    case 3: mConnection.setType( QSync::PluginConnection::SerialConnection ); break;
    case 4: mConnection.setType( QSync::PluginConnection::IrdaConnection ); break;
  }

  mConnection.setBluetoothAddress( mBluetoothAddress->text() );
  mConnection.setBluetoothChannel( mBluetoothChannel->text().toUInt() );
  mConnection.setBluetoothSdpUuid( mBluetoothSdpUuid->text() );

  mConnection.setUsbVendorId( mUsbVendorId->text().toUInt() );
  mConnection.setUsbProductId( mUsbProductId->text().toUInt() );
  mConnection.setUsbInterface( mUsbInterface->text().toUInt() );

  mConnection.setNetworkAddress( mNetworkAddress->text() );
  mConnection.setNetworkPort( mNetworkPort->text().toUInt() );
  mConnection.setNetworkProtocol( mNetworkProtocol->text() );
  mConnection.setNetworkDnsSd( mNetworkDnsSd->text() );

  mConnection.setSerialSpeed( mSerialSpeed->text().toUInt() );
  mConnection.setSerialDeviceNode( mSerialDevice->text() );

  mConnection.setIrdaService( mIrdaService->text() );
}

void ConfigConnectionWidget::typeChanged( int type )
{
  switch ( type + 1 ) {
    case QSync::PluginConnection::BlueToothConnection: mStack->setCurrentWidget( mBluetoothPage ); break;
    case QSync::PluginConnection::UsbConnection: mStack->setCurrentWidget( mUsbPage ); break;
    case QSync::PluginConnection::NetworkConnection: mStack->setCurrentWidget( mNetworkPage ); break;
    case QSync::PluginConnection::SerialConnection: mStack->setCurrentWidget( mSerialPage ); break;
    case QSync::PluginConnection::IrdaConnection: mStack->setCurrentWidget( mIrdaPage ); break;
  }
}

#include "configconnectionwidget.moc"
