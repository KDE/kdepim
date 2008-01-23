/*
    This file is part of KitchenSync.

    Copyright (c) 2007 Anirudh Ramesh <abattoir@abattoir.in>

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

#include "configguijescs.h"

#include <qcheckbox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiJescs::ConfigGuiJescs( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiJescs::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "url" ) {
      mUrl->setText( element.text() );
    } else if ( element.tagName() == "username" ) {
      mUsername->setText( element.text() );
    } else if ( element.tagName() == "password" ) {
      mPassword->setText( element.text() );
    } else if ( element.tagName() == "del_notify" ) {
      mDelNotify->setChecked( element.text() == "1" );
    }
  }
}

QString ConfigGuiJescs::save() const
{
  int delNotifyState;
  QString config = "<config>\n";

  config += QString( "<url>%1</url>\n" ).arg( mUrl->text() );
  config += QString( "<username>%1</username>\n" ).arg( mUsername->text() );
  config += QString( "<password>%1</password>\n" ).arg( mPassword->text() );
  if ( mDelNotify->isChecked() ) { delNotifyState = 1;
  }  else { delNotifyState = 0;
  }
  config += QString( "<del_notify>%1</del_notify>\n" ).arg( delNotifyState );

  config += "</config>";

  return config;
}

void ConfigGuiJescs::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 3, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new QLabel( i18n( "URL:" ), this ), 0, 0 );
  mUrl = new KLineEdit( this );
  layout->addMultiCellWidget( mUrl, 0, 0, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Username:" ), this ), 1, 0 );
  mUsername = new KLineEdit( this );
  layout->addMultiCellWidget( mUsername, 1, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Password:" ), this ), 2, 0 );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( KLineEdit::Password );
  layout->addMultiCellWidget( mPassword, 2, 2, 1, 2 );

  mDelNotify = new QCheckBox( this );
  mDelNotify->setText( "Notify attendees about event/task deletion" );
  layout->addMultiCellWidget( mDelNotify, 3, 3, 0, 2 );
}
