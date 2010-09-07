/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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

#include "configguisyncmlhttp.h"

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>

#include <tqcheckbox.h>
#include <tqdom.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqspinbox.h>
#include <tqtabwidget.h>
#include <tqvbox.h>

ConfigGuiSyncmlHttp::ConfigGuiSyncmlHttp( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent ), mUrl( 0 ), mPort( 0 )
{

  TQTabWidget *tabWidget = new TQTabWidget( this );
  topLayout()->addWidget( tabWidget );

  // Connection
  TQWidget *connectionWidget = new TQWidget( tabWidget );
  TQVBoxLayout *connectionLayout = new TQVBoxLayout( connectionWidget,
                                                   KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( connectionWidget, i18n( "Connection" ) );

  mGridLayout = new TQGridLayout( connectionLayout );

  TQLabel *label = new TQLabel( i18n("Port:"), connectionWidget );
  mGridLayout->addWidget( label, 0, 0 );

  mPort = new TQSpinBox( connectionWidget );
  mPort->setMinValue( 1 );
  mPort->setMaxValue( 65536 );
  mGridLayout->addWidget( mPort, 0, 1 );

  // Database
  TQWidget *databaseWidget = new TQWidget( tabWidget );
  TQVBoxLayout *databaseLayout = new TQVBoxLayout( databaseWidget,
                                                 KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( databaseWidget, i18n( "Databases" ) );

  mGridLayout = new TQGridLayout( databaseLayout );
  addLineEdit( databaseWidget, i18n("Contact Database:"), &mContactDb, 0 );
  addLineEdit( databaseWidget, i18n("Calendar Database:"), &mCalendarDb, 1 );
  addLineEdit( databaseWidget, i18n("Note Database:"), &mNoteDb, 2 );

  mContactDb->insertItem( "addressbook" );
  mContactDb->insertItem( "contacts" );

  mCalendarDb->insertItem( "agenda" );
  mCalendarDb->insertItem( "calendar" );

  mNoteDb->insertItem( "notes" );


  // Options
  TQWidget *optionWidget = new TQWidget( tabWidget );
  TQVBoxLayout *optionLayout = new TQVBoxLayout( optionWidget,
                                               KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( optionWidget, i18n( "Options" ) );

  mGridLayout = new TQGridLayout( optionLayout );

  label = new TQLabel( i18n("User name:"), optionWidget );
  mGridLayout->addWidget( label, 0, 0 );

  mUsername = new KLineEdit( optionWidget );
  mGridLayout->addWidget( mUsername, 0, 1 );

  label = new TQLabel( i18n("Password:"), optionWidget );
  mGridLayout->addWidget( label, 1, 0 );

  mPassword = new KLineEdit( optionWidget );
  mPassword->setEchoMode( TQLineEdit::Password );
  mGridLayout->addWidget( mPassword, 1, 1 );


  mUseStringTable = new TQCheckBox( i18n("Use String Table"), optionWidget );
  mGridLayout->addMultiCellWidget( mUseStringTable, 2, 2, 0, 1 );

  mOnlyReplace = new TQCheckBox( i18n("Only Replace Entries"), optionWidget );
  mGridLayout->addMultiCellWidget( mOnlyReplace, 3, 3, 0, 1 );

  // Url
  label = new TQLabel( i18n("URL:"), optionWidget );
  mGridLayout->addWidget( label, 4, 0 );

  mUrl = new KLineEdit( optionWidget );
  mGridLayout->addWidget( mUrl, 4, 1 );

  // recvLimit
  label = new TQLabel( i18n("Receive Limit:"), optionWidget );
  mGridLayout->addWidget( label, 5, 0 );

  mRecvLimit = new TQSpinBox( optionWidget );
  mRecvLimit->setMinValue( 0 );
  mRecvLimit->setMaxValue( 65536 );
  mGridLayout->addWidget( mRecvLimit, 5, 1 );

  // maxObjSize 
  label = new TQLabel( i18n("Maximum Object Size"), optionWidget );
  mGridLayout->addWidget( label, 6, 0 );

  mMaxObjSize = new TQSpinBox( optionWidget );
  mMaxObjSize->setMinValue( 0 );
  mMaxObjSize->setMaxValue( 65536 );
  mGridLayout->addWidget( mMaxObjSize, 6, 1 );

  topLayout()->addStretch( 1 );
}

void ConfigGuiSyncmlHttp::addLineEdit( TQWidget *parent, const TQString &text, KComboBox **edit, int row )
{
  TQLabel *label = new TQLabel( text, parent);
  mGridLayout->addWidget( label, row, 0 );

  *edit = new KComboBox( true, parent );
  mGridLayout->addWidget( *edit, row, 1 );
}

void ConfigGuiSyncmlHttp::load( const TQString &xml )
{
  TQDomDocument document;
  document.setContent( xml );

  TQDomElement docElement = document.documentElement();
  TQDomNode node;

  for ( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
    if ( element.tagName() == "username" ) {
      mUsername->setText( element.text() );
    } else if ( element.tagName() == "password" ) {
      mPassword->setText( element.text() );
    } else if ( element.tagName() == "url" ) {
      if ( mUrl )
        mUrl->setText( element.text() );
    } else if ( element.tagName() == "port" ) {
      if ( mPort )
        mPort->setValue( element.text().toInt() );
    } else if ( element.tagName() == "recvLimit" ) {
      if ( mRecvLimit )
        mRecvLimit->setValue( element.text().toInt() );
    } else if ( element.tagName() == "maxObjSize" ) {
      if ( mMaxObjSize )
        mMaxObjSize->setValue( element.text().toInt() );
    } else if ( element.tagName() == "usestringtable" ) {
      mUseStringTable->setChecked( element.text() == "1" );
    } else if ( element.tagName() == "onlyreplace" ) {
      mOnlyReplace->setChecked( element.text() == "1" );
    } else if ( element.tagName() == "contact_db" ) {
      mContactDb->setCurrentText( element.text() );
    } else if ( element.tagName() == "calendar_db" ) {
      mCalendarDb->setCurrentText( element.text() );
    } else if ( element.tagName() == "note_db" ) {
      mNoteDb->setCurrentText( element.text() );
    }
  }
}

TQString ConfigGuiSyncmlHttp::save() const
{
  TQString xml;
  xml = "<config>\n";
  xml += "<username>" + mUsername->text() + "</username>\n";
  xml += "<password>" + mPassword->text() + "</password>\n";

  xml += "<url>" + mUrl->text() + "</url>\n";
  xml += "<port>" + TQString::number( mPort->value() ) + "</port>\n";
  // Receive Limit
  xml += "<recvLimit>" + TQString::number( mRecvLimit->value() ) + "</recvLimit>\n";

  // Maximal Object Size
  xml += "<maxObjSize>" + TQString::number( mMaxObjSize->value() ) + "</maxObjSize>\n";

  xml += "<usestringtable>";
  if ( mUseStringTable->isChecked() )
    xml += "1";
  else
    xml += "0";
  xml += "</usestringtable>\n";

  xml += "<onlyreplace>";
  if ( mOnlyReplace->isChecked() )
    xml += "1";
  else
    xml += "0";
  xml += "</onlyreplace>\n";

  xml += "<contact_db>" + mContactDb->currentText() + "</contact_db>\n";
  xml += "<calendar_db>" + mCalendarDb->currentText() + "</calendar_db>\n";
  xml += "<note_db>" + mNoteDb->currentText() + "</note_db>\n";
  xml += "</config>";

  return xml;
}

#include "configguisyncmlhttp.moc"

