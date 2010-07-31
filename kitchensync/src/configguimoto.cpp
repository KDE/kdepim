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

#include "configguimoto.h"

#include <tqdom.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiMoto::ConfigGuiMoto( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiMoto::load( const TQString &xml )
{
  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();
  TQDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "device" ) {
      mDeviceString->setText( element.text() );
    }
  }
}

TQString ConfigGuiMoto::save() const
{
  TQString config = "<config>\n";

  config += TQString( "<device>%1</device>\n" ).arg( mDeviceString->text() );

  config += "</config>";

  return config;
}

void ConfigGuiMoto::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( topLayout(), 12, 3, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  layout->addWidget( new TQLabel( i18n( "Device String:" ), this ), 0, 0 );
  mDeviceString = new KLineEdit( this );
  layout->addMultiCellWidget( mDeviceString, 0, 0, 1, 2 );
}
