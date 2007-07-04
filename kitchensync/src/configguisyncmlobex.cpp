/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "configguisyncmlobex.h"

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>

#include <qlayout.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qdom.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <qvbox.h>

ConfigGuiSyncmlObex::ConfigGuiSyncmlObex( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout()->addWidget( tabWidget );

  // Connection
  QVBox *connectionWidget = new QVBox( this );
  connectionWidget->setMargin( KDialog::marginHint() );
  connectionWidget->setSpacing( 5 );

  tabWidget->addTab( connectionWidget, i18n( "Connection" ) );

  mConnection = new KComboBox( connectionWidget );

  connect( mConnection, SIGNAL (activated( int ) ),
           this, SLOT( slotConnectionChanged ( int ) ) );

  mConnectionTypes.append( ConnectionType( 2, i18n( "Bluetooth" ) ) );
  mConnectionTypes.append( ConnectionType( 5, i18n( "USB" ) ) );

  ConnectionTypeList::ConstIterator it;
  for ( it = mConnectionTypes.begin(); it != mConnectionTypes.end(); it++ )
    mConnection->insertItem( (*it).second );

  mBluetooth = new BluetoothWidget( connectionWidget );
  mBluetooth->hide();

  mUsb = new UsbWidget( connectionWidget );
  mUsb->hide();

  connectionWidget->setStretchFactor( mBluetooth, 1 );
  connectionWidget->setStretchFactor( mUsb, 1 );

  // Databases
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
  QWidget *optionsWidget = new QWidget( tabWidget );
  QVBoxLayout *optionsLayout = new QVBoxLayout( optionsWidget,
                                                KDialog::marginHint(), KDialog::spacingHint() );

  tabWidget->addTab( optionsWidget, i18n( "Options" ) );

  mGridLayout = new QGridLayout( optionsLayout );

  QLabel *label = new QLabel( i18n("User name:"), optionsWidget );
  mGridLayout->addWidget( label, 0, 0 );

  mUsername = new KLineEdit( optionsWidget );
  mGridLayout->addWidget( mUsername, 0, 1 );

  label = new QLabel( i18n("Password:"), optionsWidget );
  mGridLayout->addWidget( label, 1, 0 );

  mPassword = new KLineEdit( optionsWidget );
  mPassword->setEchoMode( QLineEdit::Password );
  mGridLayout->addWidget( mPassword, 1, 1 );

  mUseStringTable = new QCheckBox( i18n("Use String Table"), optionsWidget );
  mGridLayout->addMultiCellWidget( mUseStringTable, 2, 2, 0, 1 );

  mOnlyReplace = new QCheckBox( i18n("Only Replace Entries"), optionsWidget );
  mGridLayout->addMultiCellWidget( mOnlyReplace, 3, 3, 0, 1 );

  // SynML Version
  label = new QLabel( i18n("SyncML Version:"), optionsWidget );
  mGridLayout->addWidget( label, 4, 0 );

  mSyncmlVersion = new QComboBox( optionsWidget );
  mGridLayout->addWidget( mSyncmlVersion, 4, 1 );

  mSyncmlVersions.append( SyncmlVersion( 0,  i18n( "1.0" ) ) );
  mSyncmlVersions.append( SyncmlVersion( 1,  i18n( "1.1" ) ) );
  mSyncmlVersions.append( SyncmlVersion( 2,  i18n( "1.2" ) ) );

  SyncmlVersionList::ConstIterator itVersion;
  for ( itVersion = mSyncmlVersions.begin(); itVersion != mSyncmlVersions.end(); itVersion++ )
    mSyncmlVersion->insertItem( (*itVersion).second );

  // WBXML
  mWbxml = new QCheckBox( i18n("WAP Binary XML"), optionsWidget );
  mGridLayout->addMultiCellWidget( mWbxml, 12, 12, 0, 1 );

  // Identifier
  label = new QLabel( i18n("Software Identifier:"), optionsWidget );
  mGridLayout->addWidget( label, 13, 0 );

  mIdentifier = new KComboBox( true, optionsWidget );
  mGridLayout->addWidget( mIdentifier, 13, 1 );

  mIdentifier->insertItem( "" );
  mIdentifier->insertItem( "PC Suite" );

  // recvLimit
  label = new QLabel( i18n("Receive Limit:"), optionsWidget );
  mGridLayout->addWidget( label, 14, 0 );

  mRecvLimit = new QSpinBox( optionsWidget );
  mRecvLimit->setMinValue( 1 );
  mRecvLimit->setMaxValue( 65536 );
  mGridLayout->addWidget( mRecvLimit, 14, 1 );

  // maxObjSize
  label = new QLabel( i18n("Maximum Object Size"), optionsWidget );
  mGridLayout->addWidget( label, 15, 0 );

  mMaxObjSize = new QSpinBox( optionsWidget );
  mMaxObjSize->setMinValue( 1 );
  mMaxObjSize->setMaxValue( 65536 );
  mGridLayout->addWidget( mMaxObjSize, 15, 1 );

  topLayout()->addStretch( 1 );
}

void ConfigGuiSyncmlObex::slotConnectionChanged( int pos )
{
  mUsb->hide();
  mBluetooth->hide();

  if ( pos == 0 )
    mBluetooth->show();
  else if ( pos == 1 )
    mUsb->show();
}

void ConfigGuiSyncmlObex::load( const QString &xml )
{
  QDomDocument document;
  document.setContent( xml );

  QDomElement docElement = document.documentElement();

  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "username" ) {
      mUsername->setText( element.text() );
    } else if ( element.tagName() == "password" ) {
      mPassword->setText( element.text() );
    } else if ( element.tagName() == "type" ) {
      for ( uint i = 0; i < mConnectionTypes.count(); i++ ) {
        if ( mConnectionTypes[i].first == element.text().toInt() ) {
          mConnection->setCurrentItem( i );
          slotConnectionChanged( i );
          break;
        }
      }
    } else if ( element.tagName() == "version" ) {
      for ( uint i = 0; i < mSyncmlVersions.count(); i++ ) {
        if ( mSyncmlVersions[i].first == element.text().toInt() ) {
          mSyncmlVersion->setCurrentItem( i );
          break;
        }
      }
    } else if ( element.tagName() == "bluetooth_address" ) {
      if ( mBluetooth ) mBluetooth->setAddress( element.text() );
    } else if ( element.tagName() == "bluetooth_channel" ) {
      if ( mBluetooth ) mBluetooth->setChannel( element.text() );
    } else if ( element.tagName() == "identifier" ) {
      if ( mIdentifier ) mIdentifier->setCurrentText( element.text() );
    } else if ( element.tagName() == "interface" ) {
      if ( mUsb ) mUsb->setInterface( element.text().toInt() );
    } else if ( element.tagName() == "wbxml" ) {
      if ( mWbxml) mWbxml->setChecked( element.text() == "1" );
    } else if ( element.tagName() == "recvLimit" ) {
      if ( mRecvLimit ) mRecvLimit->setValue( element.text().toInt() );
    } else if ( element.tagName() == "maxObjSize" ) {
      if ( mMaxObjSize ) mMaxObjSize->setValue( element.text().toInt() );
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

QString ConfigGuiSyncmlObex::save() const
{
  QString xml;
  xml = "<config>\n";
  xml += "<username>" + mUsername->text() + "</username>\n";
  xml += "<password>" + mPassword->text() + "</password>\n";
  ConnectionTypeList::ConstIterator it;
  for ( it = mConnectionTypes.begin(); it != mConnectionTypes.end(); it++ ) {
    if ( mConnection->currentText() == (*it).second ) {
      xml += "<type>" + QString("%1").arg((*it).first) + "</type>\n";
      break;
    }
  }

  // Bluetooth Address
  xml += "<bluetooth_address>" + mBluetooth->address() + "</bluetooth_address>\n";

  // Bluetooth Channel
  xml += "<bluetooth_channel>" + mBluetooth->channel() + "</bluetooth_channel>\n";

  // USB Interface
  xml += "<interface>" + QString::number( mUsb->interface() ) +"</interface>\n";

  // SyncML Version
  SyncmlVersionList::ConstIterator itVersion;
  for ( itVersion = mSyncmlVersions.begin(); itVersion != mSyncmlVersions.end(); itVersion++ ) {
    if ( mSyncmlVersion->currentText() == (*itVersion).second ) {
      xml += "<version>" + QString("%1").arg((*itVersion).first) + "</version>\n";
      break;
    }
  }

  // (Software) Identifier
  xml += "<identifier>" + mIdentifier->currentText() + "</identifier>\n";

  // WBXML
  xml += "<wbxml>";
  if ( mWbxml->isChecked() )
    xml += "1";
  else
    xml += "0";
  xml += "</wbxml>\n";

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

void ConfigGuiSyncmlObex::addLineEdit( QWidget *parent, const QString &text, KComboBox **edit, int row )
{
  QLabel *label = new QLabel( text, parent );
  mGridLayout->addWidget( label, row, 0 );

  *edit = new KComboBox( true, parent );
  mGridLayout->addWidget( *edit, row, 1 );
}

#include "configguisyncmlobex.moc"
