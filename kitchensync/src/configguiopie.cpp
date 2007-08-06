/*
    This file is part of KitchenSync.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

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

#include "configguiopie.h"

#include <klocale.h>

#include <qcombobox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>

ConfigGuiOpie::ConfigGuiOpie( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  QGridLayout *layout = new QGridLayout( topLayout() );

  QLabel *label = new QLabel( i18n("Device IP:"), this );
  layout->addWidget( label, 0, 0 );

  mDeviceIP = new QLineEdit( this );
  mDeviceIP->setInputMask( "000.000.000.000" );
  label->setBuddy( mDeviceIP );
  layout->addWidget( mDeviceIP, 0, 1 );

  label = new QLabel( i18n("Device Type:"), this );
  layout->addWidget( label, 1, 0 );

  mDeviceType = new QComboBox( this );
  label->setBuddy( mDeviceType );
  layout->addWidget( mDeviceType, 1, 1 );

  label = new QLabel( i18n("Username:"), this );
  layout->addWidget( label, 2, 0 );

  mUserName = new QLineEdit( this );
  label->setBuddy( mUserName );
  layout->addWidget( mUserName, 2, 1 );

  label = new QLabel( i18n("Password:"), this );
  layout->addWidget( label, 3, 0 );

  mPassword = new QLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );
  label->setBuddy( mPassword );
  layout->addWidget( mPassword, 3, 1 );

  label = new QLabel( i18n("Protocol:"), this );
  layout->addWidget( label, 4, 0 );

  mConnectionType = new QComboBox( this );
  label->setBuddy( mConnectionType );
  layout->addWidget( mConnectionType, 4, 1 );

  label = new QLabel( i18n("Port:"), this );
  layout->addWidget( label, 5, 0 );

  mPort = new QSpinBox( this );
  mPort->setRange( 0, 65335 );
  label->setBuddy( mPort );
  layout->addWidget( mPort, 5, 1 );

  mDeviceType->insertItem( i18n("Opie/OpenZaurus") );
  mDeviceType->insertItem( i18n("Qtopia2") );

  mConnectionType->insertItem( i18n("SCP") );
  mConnectionType->insertItem( i18n("FTP") );

  topLayout()->addStretch( 1 );
}

void ConfigGuiOpie::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode n;
  for( n = docElement.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    QDomElement e = n.toElement();
    if ( e.tagName() == "username" ) {
      mUserName->setText( e.text() );
    } else if ( e.tagName() == "password" ) {
      mPassword->setText( e.text() );
    } else if ( e.tagName() == "url" ) {
      mDeviceIP->setText( e.text() );
    } else if ( e.tagName() == "port" ) {
      mPort->setValue( e.text().toInt() );
    } else if ( e.tagName() == "device" ) {
      if ( e.text() == "opie" )
        mDeviceType->setCurrentItem( 0 );
      else
        mDeviceType->setCurrentItem( 1 );
    } else if ( e.tagName() == "conntype" ) {
      if ( e.text() == "scp" )
        mConnectionType->setCurrentItem( 0 );
      else
        mConnectionType->setCurrentItem( 1 );
    }
  }
}

QString ConfigGuiOpie::save() const
{
  QString xml;
  xml = "<config>";
  xml += "<username>" + mUserName->text() + "</username>";
  xml += "<password>" + mPassword->text() + "</password>";
  xml += "<url>" + mDeviceIP->text() + "</url>";
  xml += "<device>" + QString( mDeviceType->currentItem() == 0 ? "opie" : "qtopia2" ) + "</device>";
  xml += "<port>" + QString::number( mPort->value() ) + "</port>";
  xml += "<conntype>" + QString( mConnectionType->currentItem() == 0 ? "scp" : "ftp" ) + "</conntype>";
  xml += "</config>";

  return xml;
}
