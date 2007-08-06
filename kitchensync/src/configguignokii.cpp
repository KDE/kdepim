/*
    This file is part of KitchenSync.

    Copyright (c) 2006 David Förster <david@dfoerster.de>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "configguignokii.h"

#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>

#include <kdebug.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qdom.h>
#include <qvbox.h>

ConfigGuiGnokii::ConfigGuiGnokii( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  QGridLayout *layout = new QGridLayout( topLayout() );

  // Model
  QLabel *label = new QLabel( i18n("Model:"), this );
  layout->addWidget( label, 0, 0 );

  mModel = new KComboBox( true, this );
  layout->addWidget( mModel, 0, 1 );
  mModel->insertItem( "2110" ); 
  mModel->insertItem( "3110" ); 
  mModel->insertItem( "6110" ); 
  mModel->insertItem( "6110" ); 
  mModel->insertItem( "6160" ); 
  mModel->insertItem( "6230" );
  mModel->insertItem( "6230i" );
  mModel->insertItem( "6510" ); 
  mModel->insertItem( "7110" );
  mModel->insertItem( "AT" );
  // This one requires the gnapplet and rfcomm_channel
  mModel->insertItem( "3650" );
  mModel->insertItem( "6600" );
  mModel->insertItem( "gnapplet" );
  mModel->insertItem( "symbian" );
  mModel->insertItem( "sx1" );

  connect( mModel, SIGNAL (activated( int ) ),
    this, SLOT( slotModelChanged () ) );

  // Connection
  label = new QLabel( i18n("Connection:"), this );
  layout->addWidget( label, 1, 0 );

  mConnection = new QComboBox( this );
  layout->addWidget( mConnection, 1, 1 );

  connect( mConnection, SIGNAL (activated( int ) ),
      	    this, SLOT( slotConnectionChanged ( int ) ) );

  // this is a list of all connection types accepted by the gnokii-sync plugin
  mConnectionTypes.append( ConnectionType( "bluetooth",  i18n( "Bluetooth" ) ) );
  mConnectionTypes.append( ConnectionType( "irda", i18n( "IrDA" ) ) );
  mConnectionTypes.append( ConnectionType( "serial", i18n( "Serial" ) ) );
  mConnectionTypes.append( ConnectionType( "infrared", i18n( "Infrared" ) ) );
  mConnectionTypes.append( ConnectionType( "tcp", i18n( "TCP" ) ) );
  mConnectionTypes.append( ConnectionType( "dku2", i18n( "USB (nokia_dku2)" ) ) );
  mConnectionTypes.append( ConnectionType( "dku2libusb", i18n( "USB (libusb)" ) ) );
  mConnectionTypes.append( ConnectionType( "dau9p", i18n( "Serial (DAU9P cable)" ) ) );
  mConnectionTypes.append( ConnectionType( "dlr3p", i18n( "Serial (DLR3P cable)" ) ) );
  mConnectionTypes.append( ConnectionType( "tekram", i18n( "Tekram Ir-Dongle" ) ) );
  mConnectionTypes.append( ConnectionType( "m2bus", i18n( "Serial (M2BUS protocol)" ) ) );

  ConnectionTypeList::ConstIterator it;
  for ( it = mConnectionTypes.begin(); it != mConnectionTypes.end(); it++ ) {
    mConnection->insertItem( (*it).second );
  }

  QVBox *connectionWidget = new QVBox( this );
  connectionWidget->setMargin( KDialog::marginHint() );
  connectionWidget->setSpacing( 5 );

  mBluetooth = new BluetoothWidget( connectionWidget ); 
  mBluetooth->hide();

  layout->addMultiCellWidget( connectionWidget, 2, 2, 0, 1 );

  // Port
  mPortLabel = new QLabel( i18n("Port:"), this );
  layout->addWidget( mPortLabel, 2, 0 );
  mPortLabel->hide();

  mPort = new KComboBox( true, this );
  layout->addWidget( mPort, 2, 1 );
  mPort->hide();

  mPort->insertItem( "/dev/ircomm0" );
  mPort->insertItem( "/dev/ircomm1" );
  mPort->insertItem( "/dev/ttyS0" );
  mPort->insertItem( "/dev/ttyS1" );
  mPort->insertItem( "/dev/ttyUSB0" );
  mPort->insertItem( "/dev/ttyUSB1" );

  layout->setColStretch( 1, 1 );

  topLayout()->addStretch( 1 );
}

void ConfigGuiGnokii::slotConnectionChanged( int nth )
{
  mPort->hide();
  mPortLabel->hide();
  mBluetooth->hide();

  // Bluetooth
  if ( nth == 0 ) {
    mBluetooth->show();
    slotModelChanged();

    if ( !mPort->currentText().isEmpty() )
      mBluetooth->setAddress( mPort->currentText() );

  // dku2libusb
  } else if ( nth == 6 ) {
    // No widget needed.
  } else {
    mPort->show();
    mPortLabel->show();
  }

}

void ConfigGuiGnokii::slotModelChanged()
{
  mBluetooth->hideChannel();

  if ( mModel->currentText() == "gnapplet"
    || mModel->currentText() == "symbian"
    || mModel->currentText() == "3650"
    || mModel->currentText() == "6600"
    || mModel->currentText() == "sx1")
    mBluetooth->showChannel();
  else
    mBluetooth->setChannel("");
}

void ConfigGuiGnokii::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode n;
  for( n = docElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    if ( e.tagName() == "connection" ) {
      for ( uint i = 0; i < mConnectionTypes.count(); i++ ) {
        if ( mConnectionTypes[i].first == e.text()) {
          mConnection->setCurrentItem( i );
          slotConnectionChanged( i );
          break;
        }
      }
    } else if ( e.tagName() == "port" ) {
      mPort->setCurrentText( e.text() );
    } else if ( e.tagName() == "model" ) {
      mModel->setCurrentText( e.text() );
    } else if ( e.tagName() == "rfcomm_channel" ) {
      mBluetooth->setChannel( e.text() );
      mBluetooth->showChannel();
    }
  }
}

QString ConfigGuiGnokii::save() const
{
  QString xml;
  xml = "<config>";

  ConnectionTypeList::ConstIterator it;
  for ( it = mConnectionTypes.begin(); it != mConnectionTypes.end(); it++ ) {
    if ( mConnection->currentText() == (*it).second ) {
      xml += "<connection>" + (*it).first + "</connection>";
      break;
    }
  }

  if ( (*it).first == "bluetooth" )
    xml += "<port>" + mBluetooth->address() + "</port>";
  else if ( (*it).first == "dku2libusb" )
    xml += "<port>" + QString("FF:FF:FF:FF:FF:FF") + "</port>"; // Only place holder for libgnokii
  else
    xml += "<port>" + mPort->currentText() + "</port>";

  // model
  xml += "<model>" + mModel->currentText() + "</model>";

  // rfcomm_channel
  if ( !mBluetooth->channel().isNull() )
    xml += "<rfcomm_channel>" + mBluetooth->channel() + "</rfcomm_channel>";

  xml += "</config>";

  return xml;
}

#include "configguignokii.moc"
