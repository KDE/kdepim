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

#include "configguisynce.h"

#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klineedit.h>
#include <kdialog.h>
#include <klocale.h>

ConfigGuiSynce::ConfigGuiSynce( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();
}

void ConfigGuiSynce::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "contact" ) {
      mContacts->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "todos" ) {
      mTodos->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "calendar" ) {
      mCalendar->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "file" ) {
      mFile->setText( element.text() );
    }
  }
}

QString ConfigGuiSynce::save() const
{
  QString config = "<config>\n";

  config += QString( "<contact>%1</contact>\n" ).arg( mContacts->isChecked() ? "1" : "0" );
  config += QString( "<todos>%1</todos>\n" ).arg( mTodos->isChecked() ? "1" : "0" );
  config += QString( "<calendar>%1</calendar>\n" ).arg( mCalendar->isChecked() ? "1" : "0" );
  config += QString( "<file>%1</file>\n" ).arg( mFile->text() );

  config += "</config>";

  return config;
}

void ConfigGuiSynce::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 2, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  mContacts = new QCheckBox( this );
  mContacts->setText( "Sync Contacts" );
  layout->addMultiCellWidget( mContacts, 0, 0, 0, 1 );

  mTodos = new QCheckBox( this );
  mTodos->setText( "Sync \'Todo\' items" );
  layout->addMultiCellWidget( mTodos, 1, 1, 0, 1 );

  mCalendar = new QCheckBox( this );
  mCalendar->setText( "Sync Calendar" );
  layout->addMultiCellWidget( mCalendar, 2, 2, 0, 1 );

  layout->addWidget( new QLabel( i18n( "File:" ), this ), 3, 0 );
  mFile = new KLineEdit( this );
  layout->addWidget( mFile, 3, 1 );
}
