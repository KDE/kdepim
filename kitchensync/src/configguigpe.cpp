/*
    This file is part of KitchenSync.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "configguigpe.h"

#include <qcheckbox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

ConfigGuiGpe::ConfigGuiGpe( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();

  mConnectionMode->insertItem( i18n( "Local" ) );
  mConnectionMode->insertItem( i18n( "Ssh" ) );
}

void ConfigGuiGpe::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "use_local" ) {
      if ( element.text().toInt() == 1 )
        mConnectionMode->setCurrentItem( 0 );
      else
        mConnectionMode->setCurrentItem( 1 );
    } else if ( element.tagName() == "handheld_ip" ) {
      mIP->setText( element.text() );
    } else if ( element.tagName() == "handheld_port" ) {
      mPort->setValue( element.text().toInt() );
    } else if ( element.tagName() == "handheld_user" ) {
      mUser->setText( element.text() );
    }
  }
}

QString ConfigGuiGpe::save() const
{
  QString config = "<config>";

  config += QString( "<use_local>%1</use_local>" ).arg( mConnectionMode->currentItem() == 0 );
  config += QString( "<use_ssh>%1</use_ssh>" ).arg( mConnectionMode->currentItem() == 1 );
  config += QString( "<handheld_ip>%1</handheld_ip>" ).arg( mIP->text() );
  config += QString( "<handheld_port>%1</handheld_port>" ).arg( mPort->value() );
  config += QString( "<handheld_user>%1</handheld_user>" ).arg( mUser->text() );

  config += "</config>";

  return config;
}

void ConfigGuiGpe::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 4, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new QLabel( i18n( "Connection Mode:" ), this ), 0, 0 );
  mConnectionMode = new KComboBox( this );
  layout->addMultiCellWidget( mConnectionMode, 0, 0, 0, 3 );

  layout->addWidget( new QLabel( i18n( "IP Address:" ), this ), 1, 0 );
  mIP = new KLineEdit( this );
  mIP->setInputMask( "000.000.000.000" );
  layout->addWidget( mIP, 1, 1 );

  layout->addWidget( new QLabel( i18n( "Port:" ), this ), 1, 2, Qt::AlignRight );
  mPort = new QSpinBox( 1, 65536, 1, this );
  layout->addWidget( mPort, 1, 3 );

  layout->addWidget( new QLabel( i18n( "User:" ), this ), 2, 0 );
  mUser = new KLineEdit( this );
  layout->addMultiCellWidget( mUser, 2, 2, 1, 3 );
}
