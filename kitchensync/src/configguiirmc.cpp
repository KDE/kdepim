/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <kcombobox.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <kdebug.h>

#include <qapplication.h>
#include <qeventloop.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qvbox.h>

#include "configguiirmc.h"

ConfigGuiIRMC::ConfigGuiIRMC( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();

  mConnectionType->insertItem( i18n( "Bluetooth" ) );
  mConnectionType->insertItem( i18n( "InfraRed (IR)" ) );
  mConnectionType->insertItem( i18n( "Cable" ) );

  connect( mConnectionType, SIGNAL( activated( int ) ),
           this, SLOT( connectionTypeChanged( int ) ) );

  connectionTypeChanged( 0 );
}

void ConfigGuiIRMC::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for ( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "connectmedium" ) {
      if ( element.text() == "bluetooth" ) {
        mConnectionType->setCurrentItem( 0 );
        connectionTypeChanged( 0 );
      } else if ( element.text() == "ir" ) {
        mConnectionType->setCurrentItem( 1 );
        connectionTypeChanged( 1 );
      } else if ( element.text() == "cable" ) {
        mConnectionType->setCurrentItem( 2 );
        connectionTypeChanged( 2 );
      }
    } else if (element.tagName() == "btunit" ) {
      mBluetoothWidget->setAddress( element.text() );
    } else if (element.tagName() == "btchannel" ) {
      mBluetoothWidget->setChannel( element.text() );
    } else if (element.tagName() == "donttellsync" ) {
      mDontTellSync->setChecked( element.text() == "true" );
    }


  }

  mIRWidget->load( docElement );
  mCableWidget->load( docElement );
}

QString ConfigGuiIRMC::save() const
{
  QDomDocument doc;
  QDomElement config = doc.createElement( "config" );
  doc.appendChild( config );

  QDomElement element = doc.createElement( "connectmedium" );
  if ( mConnectionType->currentItem() == 0 )
    element.appendChild( doc.createTextNode( "bluetooth" ) );
  if ( mConnectionType->currentItem() == 1 )
    element.appendChild( doc.createTextNode( "ir" ) );
  if ( mConnectionType->currentItem() == 2 )
    element.appendChild( doc.createTextNode( "cable" ) );

  config.appendChild( element );

  if ( mConnectionType->currentItem() == 0 ) {
    QDomElement btunit = doc.createElement( "btunit" );
    if ( !mBluetoothWidget->address().isEmpty() )
      btunit.appendChild( doc.createTextNode( mBluetoothWidget->address() ) );

    QDomElement btchannel = doc.createElement( "btchannel" );
    if ( !mBluetoothWidget->channel().isEmpty() )
      btchannel.appendChild( doc.createTextNode( mBluetoothWidget->channel() ) );

    config.appendChild( btunit );
    config.appendChild( btchannel );
  }

  if ( mDontTellSync->isChecked() ) {
    QDomElement dontellsync = doc.createElement( "donttellsync" );
    dontellsync.appendChild( doc.createTextNode( "true" ) );
    config.appendChild( dontellsync );
  }

  mIRWidget->save( doc, config );
  mCableWidget->save( doc, config );

  return doc.toString();
}

void ConfigGuiIRMC::connectionTypeChanged( int type )
{
  mBluetoothWidget->hide();
  mIRWidget->hide();
  mCableWidget->hide();

  if ( type == 0 )
    mBluetoothWidget->show();
  else if ( type == 1 )
    mIRWidget->show();
  else
    mCableWidget->show();
}

void ConfigGuiIRMC::initGUI()
{
  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout()->addWidget( tabWidget );

  QVBox *connectionWidget = new QVBox( tabWidget );
  connectionWidget->setMargin( KDialog::marginHint() );
  connectionWidget->setSpacing( 5 );

  tabWidget->addTab( connectionWidget, i18n( "Connection" ) );

  mConnectionType = new KComboBox( connectionWidget );
  QToolTip::add( mConnectionType, i18n( "Select your connection type." ) );

  mBluetoothWidget = new BluetoothWidget( connectionWidget );
  mBluetoothWidget->hide();

  mIRWidget = new IRWidget( connectionWidget );
  mIRWidget->hide();

  mCableWidget = new CableWidget( connectionWidget );
  mCableWidget->hide();

  connectionWidget->setStretchFactor( mBluetoothWidget, 1 );
  connectionWidget->setStretchFactor( mIRWidget, 1 );
  connectionWidget->setStretchFactor( mCableWidget, 1 );

  QVBox *optionsWidget = new QVBox( tabWidget );
  optionsWidget->setMargin( KDialog::marginHint() );
  optionsWidget->setSpacing( 5 );

  tabWidget->addTab( optionsWidget, i18n( "Options" ) );

  QHBox *optionBox = new QHBox( optionsWidget );
  optionBox->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Don't send OBEX UUID (IRMC-SYNC)" ), optionBox );
  mDontTellSync = new QCheckBox( optionBox );
  QToolTip::add( mDontTellSync, i18n( "Don't send OBEX UUID while connecting. Needed for older IrMC based mobile phones." ) );
  label->setBuddy( mDontTellSync );

}

#include "configguiirmc.moc"
