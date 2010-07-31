/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2006 Daniel Gollub <dgollub@suse.de>

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

#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <tqapplication.h>
#include <tqeventloop.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <tqtabwidget.h>
#include <tqtooltip.h>
#include <tqvbox.h>

#include "connectionwidgets.h"

BluetoothWidget::BluetoothWidget( TQWidget *parent )
  : TQWidget( parent )
{
  TQGridLayout *layout = new TQGridLayout( this );

  mAddress = new KLineEdit( this );
  mAddress->setInputMask( ">NN:NN:NN:NN:NN:NN;" );
  layout->addWidget( mAddress, 1, 0 );

  TQLabel *label = new TQLabel( i18n( "Bluetooth address:" ), this );
  label->setBuddy( mAddress );
  layout->addWidget( label, 0, 0 );

  mChannel = new KLineEdit( this );
  layout->addWidget( mChannel, 1, 1 );

  mChannelLabel = new TQLabel( i18n( "Channel:" ), this );
  mChannelLabel->setBuddy( mChannel );
  layout->addWidget( mChannelLabel, 0, 1 );

  layout->setRowStretch( 2, 1 );
}

void BluetoothWidget::hideChannel()
{
  mChannelLabel->hide();
  mChannel->hide();
}

void BluetoothWidget::showChannel()
{
  mChannelLabel->show();
  mChannel->show();
}

void BluetoothWidget::setAddress( const TQString address )
{
  mAddress->setText( address );
}

void BluetoothWidget::setChannel( const TQString channel )
{
  if ( mChannel )
    mChannel->setText( channel );
}

TQString BluetoothWidget::address() const
{
  return mAddress->text();
}

TQString BluetoothWidget::channel() const
{
  if ( mChannel->text().isEmpty() )
    return TQString();

  return mChannel->text();
}

// FIXME - Only IrMC specific
IRWidget::IRWidget( TQWidget *parent )
  : TQWidget( parent )
{
  TQGridLayout *layout = new TQGridLayout( this, 3, 3, 11, 3 );

  mDevice = new KLineEdit( this );
  mSerialNumber = new KLineEdit( this );

  layout->addWidget( mDevice, 1, 0 );
  layout->addWidget( mSerialNumber, 1, 1 );

  TQLabel *label = new TQLabel( i18n( "Device Name:" ), this );
  label->setBuddy( mDevice );
  layout->addWidget( label, 0, 0 );

  label = new TQLabel( i18n( "Serial Number:" ), this );
  label->setBuddy( mSerialNumber );
  layout->addWidget( label, 0, 1 );

  layout->setRowStretch( 2, 1 );
}

void IRWidget::load( const TQDomElement &parent )
{
  TQDomNode node;
  for ( node = parent.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "irname" )
      mDevice->setText( element.text() );
    else if ( element.tagName() == "irserial" )
      mSerialNumber->setText( element.text() );
  }
}

void IRWidget::save( TQDomDocument &doc, TQDomElement &parent )
{
  TQDomElement element = doc.createElement( "irname" );
  element.appendChild( doc.createTextNode( mDevice->text() ) );
  parent.appendChild( element );

  element = doc.createElement( "irserial" );
  element.appendChild( doc.createTextNode( mSerialNumber->text() ) );
  parent.appendChild( element );
}

// FIXME - Only IrMC specific
CableWidget::CableWidget( TQWidget *parent )
  : TQWidget( parent )
{
  TQGridLayout *layout = new TQGridLayout( this, 3, 2, 11, 3 );

  mManufacturer = new KComboBox( this );
  mDevice = new KComboBox( true, this );

  layout->addWidget( mManufacturer, 0, 1 );
  layout->addWidget( mDevice, 1, 1 );

  TQLabel *label = new TQLabel( i18n( "Device Manufacturer:" ), this );
  label->setBuddy( mManufacturer );
  layout->addWidget( label, 0, 0 );

  label = new TQLabel( i18n( "Device:" ), this );
  label->setBuddy( mDevice );
  layout->addWidget( label, 1, 0 );

  layout->setRowStretch( 2, 1 );

  mManufacturer->insertItem( i18n( "SonyEricsson/Ericsson" ) );
  mManufacturer->insertItem( i18n( "Siemens" ) );

  mDevice->insertItem( "/dev/ttyS0" );
  mDevice->insertItem( "/dev/ttyS1" );
  mDevice->insertItem( "/dev/ttyUSB0" );
  mDevice->insertItem( "/dev/ttyUSB1" );
}

void CableWidget::load( const TQDomElement &parent )
{
  TQDomNode node;
  for ( node = parent.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "cabletype" )
      mManufacturer->setCurrentItem( element.text().toInt() );
    else if ( element.tagName() == "cabledev" )
      mDevice->setCurrentText( element.text() );
  }
}

void CableWidget::save( TQDomDocument &doc, TQDomElement &parent )
{
  TQDomElement element = doc.createElement( "cabletype" );
  element.appendChild( doc.createTextNode( TQString::number( mManufacturer->currentItem() ) ) );
  parent.appendChild( element );

  element = doc.createElement( "cabledev" );
  element.appendChild( doc.createTextNode( mDevice->currentText() ) );
  parent.appendChild( element );
}

UsbWidget::UsbWidget( TQWidget *parent )
  : TQWidget( parent )
{
  TQGridLayout *layout = new TQGridLayout( this, 3, 2, 11, 3);

  mInterface = new TQSpinBox( this );
  layout->addWidget( mInterface, 0, 1 );

  TQLabel *label = new TQLabel( i18n( "USB Interface:" ), this );
  label->setBuddy( mInterface );
  layout->addWidget( label, 0, 0 );

  layout->setRowStretch( 2, 1 );
}

void UsbWidget::setInterface( int interface )
{
  mInterface->setValue( interface );
}

int UsbWidget::interface() const
{
  return mInterface->value();
}

#include "connectionwidgets.moc"
