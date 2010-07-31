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
#include <klineedit.h>
#include <klocale.h>

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqdom.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqspinbox.h>
#include <tqtabwidget.h>

#include "configguipalm.h"

ConfigGuiPalm::ConfigGuiPalm( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();

  mDevice->insertItem( "/dev/pilot" );
  mDevice->insertItem( "/dev/ttyUSB0" );
  mDevice->insertItem( "/dev/ttyUSB1" );
  mDevice->insertItem( "/dev/ttyUSB2" );
  mDevice->insertItem( "/dev/ttyUSB3" );

  mSpeed->insertItem( "9600" );
  mSpeed->insertItem( "19200" );
  mSpeed->insertItem( "38400" );
  mSpeed->insertItem( "57600" );
  mSpeed->insertItem( "115200" );
}

void ConfigGuiPalm::load( const TQString &xml )
{
  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();
  TQDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "sockaddr" ) {
      mDevice->setCurrentText( element.text() );
    } else if ( element.tagName() == "speed" ) {
      mSpeed->setCurrentText( element.text() );
    } else if ( element.tagName() == "timeout" ) {
      mTimeout->setValue( element.text().toInt() );
    } else if ( element.tagName() == "username" ) {
      mUserName->setText( element.text() );
    } else if ( element.tagName() == "mismatch" ) {
      switch ( element.text().toInt() ) {
        case 0:
          mSyncAlways->setChecked( true );
          break;
        case 2:
          mSyncAbort->setChecked( true );
          break;
        case 1:
        default:
          mSyncAsk->setChecked( true );
          break;
      }
    } else if ( element.tagName() == "popup" ) {
      mPopup->setChecked( element.text() == "1" );
    }
  }
}

TQString ConfigGuiPalm::save() const
{
  TQString config = "<config>";

  config += "<sockaddr>" + mDevice->currentText() + "</sockaddr>";
  config += "<username>" + mUserName->text() + "</username>";
  config += "<timeout>" + TQString::number( mTimeout->value() ) + "</timeout>";
  config += "<type>0</type>";
  config += "<speed>" + mSpeed->currentText() + "</speed>";
  config += "<id>0</id>";
  config += "<codepage>cp1252</codepage>";
  config += "<popup>" + TQString( mPopup->isChecked() ? "1" : "0" ) + "</popup>";

  TQString popup;
  if ( mSyncAlways->isChecked() )
    popup = "0";
  else if ( mSyncAsk->isChecked() )
    popup = "1";
  else if ( mSyncAbort->isChecked() )
    popup = "2";

  config += "<mismatch>" + popup + "</mismatch>";

  config += "</config>";

  return config;
}

void ConfigGuiPalm::initGUI()
{
  TQFont boldFont = font();
  boldFont.setBold( true );

  TQTabWidget *tabWidget = new TQTabWidget( this );

  TQWidget *connectionWidget = new TQWidget( tabWidget );
  TQVBoxLayout *connectionLayout = new TQVBoxLayout( connectionWidget,
                                                   KDialog::marginHint(), KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Connection" ), connectionWidget );
  label->setFont( boldFont );
  connectionLayout->addWidget( label );

  TQGridLayout *gridLayout = new TQGridLayout( connectionLayout, 3, 2, KDialog::spacingHint() );
  gridLayout->setMargin( KDialog::marginHint() );

  gridLayout->addWidget( new TQLabel( i18n( "Port:" ), connectionWidget ), 0, 0 );
  gridLayout->addWidget( new TQLabel( i18n( "Speed:" ), connectionWidget ), 1, 0 );
  gridLayout->addWidget( new TQLabel( i18n( "Timeout:" ), connectionWidget ), 2, 0 );

  mDevice = new KComboBox( true, connectionWidget );
  mSpeed = new KComboBox( connectionWidget );
  mTimeout = new TQSpinBox( 1, 60, 1, connectionWidget );
  mTimeout->setSuffix( i18n( " sec" ) );

  gridLayout->addWidget( mDevice, 0, 1 );
  gridLayout->addWidget( mSpeed, 1, 1 );
  gridLayout->addWidget( mTimeout, 2, 1 );
  gridLayout->setColStretch( 1, 1 );

  label = new TQLabel( i18n( "User" ), connectionWidget );
  label->setFont( boldFont );
  connectionLayout->addWidget( label );

  gridLayout = new TQGridLayout( connectionLayout, 1, 2, KDialog::spacingHint() );
  gridLayout->setMargin( KDialog::marginHint() );

  gridLayout->addWidget( new TQLabel( i18n( "Username:" ), connectionWidget ), 0, 0 );

  mUserName = new KLineEdit( connectionWidget );
  gridLayout->addWidget( mUserName, 0, 1 );

  label = new TQLabel( i18n( "What to do if Username does not match" ), connectionWidget );
  label->setFont( boldFont );
  connectionLayout->addWidget( label );

  gridLayout = new TQGridLayout( connectionLayout, 1, 2, KDialog::spacingHint() );
  gridLayout->setMargin( KDialog::marginHint() );

  TQButtonGroup *buttonGroup = new TQButtonGroup( 1, Qt::Horizontal, connectionWidget );
  buttonGroup->setExclusive( true );
  buttonGroup->setFrameStyle( TQFrame::NoFrame );
  mSyncAlways = new TQRadioButton( i18n( "Sync Anyway" ), buttonGroup );
  mSyncAsk = new TQRadioButton( i18n( "Ask What To Do" ), buttonGroup );
  mSyncAbort = new TQRadioButton( i18n( "Abort Sync" ), buttonGroup );

  gridLayout->addMultiCellWidget( buttonGroup, 0, 0, 0, 1 );

  connectionLayout->addStretch( 1 );
  tabWidget->addTab( connectionWidget, i18n( "Connection" ) );

  TQWidget *optionWidget = new TQWidget( tabWidget );
  TQVBoxLayout *optionLayout = new TQVBoxLayout( optionWidget,
                                               KDialog::marginHint(), KDialog::spacingHint() );

  label = new TQLabel( i18n( "Hotsync Notification" ), optionWidget );
  label->setFont( boldFont );
  optionLayout->addWidget( label );

  gridLayout = new TQGridLayout( optionLayout, 1, 2, KDialog::spacingHint() );
  gridLayout->setMargin( KDialog::marginHint() );

  mPopup = new TQCheckBox( i18n( "Popup when interaction is required" ), optionWidget );
  gridLayout->addMultiCellWidget( mPopup, 0, 0, 0, 1 );

  optionLayout->addStretch( 1 );
  tabWidget->addTab( optionWidget, i18n( "Options" ) );

  topLayout()->addWidget( tabWidget );
}
