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

#include <qcheckbox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>

ConfigGuiSyncmlHttp::ConfigGuiSyncmlHttp( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent ), mUrl( 0 ), mPort( 0 )
{

  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout()->addWidget( tabWidget );

  // Connection
  QWidget *connectionWidget = new QWidget( tabWidget );
  QVBoxLayout *connectionLayout = new QVBoxLayout( connectionWidget,
                                                   KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( connectionWidget, i18n( "Connection" ) );

  mGridLayout = new QGridLayout( connectionLayout );

  QLabel *label = new QLabel( i18n("Port:"), connectionWidget );
  mGridLayout->addWidget( label, 0, 0 );

  mPort = new QSpinBox( connectionWidget );
  mPort->setMinValue( 1 );
  mPort->setMaxValue( 65536 );
  mGridLayout->addWidget( mPort, 0, 1 );

  // Database
  QWidget *databaseWidget = new QWidget( tabWidget );
  QVBoxLayout *databaseLayout = new QVBoxLayout( databaseWidget,
                                                 KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( databaseWidget, i18n( "Databases" ) );

  mGridLayout = new QGridLayout( databaseLayout );
  addLineEdit( databaseWidget, i18n("Contact Database:"), &mContactDb, 0 );
  addLineEdit( databaseWidget, i18n("Calendar Database:"), &mCalendarDb, 1 );
  addLineEdit( databaseWidget, i18n("Note Database:"), &mNoteDb, 2 );

  mContactDb->insertItem( "addressbook" );
  mContactDb->insertItem( "contacts" );

  mCalendarDb->insertItem( "agenda" );
  mCalendarDb->insertItem( "calendar" );

  mNoteDb->insertItem( "notes" );


  // Options
  QWidget *optionWidget = new QWidget( tabWidget );
  QVBoxLayout *optionLayout = new QVBoxLayout( optionWidget,
                                               KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( optionWidget, i18n( "Options" ) );

  mGridLayout = new QGridLayout( optionLayout );

  label = new QLabel( i18n("User name:"), optionWidget );
  mGridLayout->addWidget( label, 0, 0 );

  mUsername = new KLineEdit( optionWidget );
  mGridLayout->addWidget( mUsername, 0, 1 );

  label = new QLabel( i18n("Password:"), optionWidget );
  mGridLayout->addWidget( label, 1, 0 );

  mPassword = new KLineEdit( optionWidget );
  mPassword->setEchoMode( QLineEdit::Password );
  mGridLayout->addWidget( mPassword, 1, 1 );


  mUseStringTable = new QCheckBox( i18n("Use String Table"), optionWidget );
  mGridLayout->addMultiCellWidget( mUseStringTable, 2, 2, 0, 1 );

  mOnlyReplace = new QCheckBox( i18n("Only Replace Entries"), optionWidget );
  mGridLayout->addMultiCellWidget( mOnlyReplace, 3, 3, 0, 1 );

  // Url
  label = new QLabel( i18n("URL:"), optionWidget );
  mGridLayout->addWidget( label, 4, 0 );

  mUrl = new KLineEdit( optionWidget );
  mGridLayout->addWidget( mUrl, 4, 1 );

  // recvLimit
  label = new QLabel( i18n("Receive Limit:"), optionWidget );
  mGridLayout->addWidget( label, 5, 0 );

  mRecvLimit = new QSpinBox( optionWidget );
  mRecvLimit->setMinValue( 1 );
  mRecvLimit->setMaxValue( 65536 );
  mGridLayout->addWidget( mRecvLimit, 5, 1 );

  // maxObjSize 
  label = new QLabel( i18n("Maximum Object Size"), optionWidget );
  mGridLayout->addWidget( label, 6, 0 );

  mMaxObjSize = new QSpinBox( optionWidget );
  mMaxObjSize->setMinValue( 1 );
  mMaxObjSize->setMaxValue( 65536 );
  mGridLayout->addWidget( mMaxObjSize, 6, 1 );

  topLayout()->addStretch( 1 );
}

void ConfigGuiSyncmlHttp::addLineEdit( QWidget *parent, const QString &text, KComboBox **edit, int row )
{
  QLabel *label = new QLabel( text, parent);
  mGridLayout->addWidget( label, row, 0 );

  *edit = new KComboBox( true, parent );
  mGridLayout->addWidget( *edit, row, 1 );
}

void ConfigGuiSyncmlHttp::load( const QString &xml )
{
  QDomDocument document;
  document.setContent( xml );

  QDomElement docElement = document.documentElement();
  QDomNode node;

  for ( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
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

QString ConfigGuiSyncmlHttp::save() const
{
  QString xml;
  xml = "<config>\n";
  xml += "<username>" + mUsername->text() + "</username>\n";
  xml += "<password>" + mPassword->text() + "</password>\n";

  xml += "<url>" + mUrl->text() + "</url>\n";
  xml += "<port>" + QString::number( mPort->value() ) + "</port>\n";
  // Receive Limit
  xml += "<recvLimit>" + QString::number( mRecvLimit->value() ) + "</recvLimit>\n";

  // Maximal Object Size
  xml += "<maxObjSize>" + QString::number( mMaxObjSize->value() ) + "</maxObjSize>\n";

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

