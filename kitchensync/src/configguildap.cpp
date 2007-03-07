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

#include "configguildap.h"

#include <qcheckbox.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

ConfigGuiLdap::ConfigGuiLdap( const QSync::Member &member, QWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();

  bindModeChanged( false );

  mSearchScope->addItem( i18n( "Base" ) );
  mSearchScope->addItem( i18n( "One" ) );
  mSearchScope->addItem( i18n( "Sub" ) );

  mAuthMech->addItem( i18n( "Simple" ) );
}

void ConfigGuiLdap::load( const QString &xml )
{
  QDomDocument doc;
  doc.setContent( xml );
  QDomElement docElement = doc.documentElement();
  QDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    QDomElement element = node.toElement();
    if ( element.tagName() == "servername" ) {
      mServerName->setText( element.text() );
    } else if ( element.tagName() == "serverport" ) {
      mPort->setValue( element.text().toInt() );
    } else if ( element.tagName() == "binddn" ) {
      mBindDn->setText( element.text() );
    } else if ( element.tagName() == "password" ) {
      mPassword->setText( element.text() );
    } else if ( element.tagName() == "anonymous" ) {
      mAnonymousBind->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "searchbase" ) {
      mSearchBase->setText( element.text() );
    } else if ( element.tagName() == "searchfilter" ) {
      mSearchFilter->setText( element.text() );
    } else if ( element.tagName() == "storebase" ) {
      mStoreBase->setText( element.text() );
    } else if ( element.tagName() == "keyattr" ) {
      mKeyAttribute->setText( element.text() );
    } else if ( element.tagName() == "scope" ) {
      QStringList list;
      list << "base" << "one" << "sub";

      for ( int i = 0; i < list.count(); ++i )
        if ( list[ i ] == element.text() )
          mSearchScope->setCurrentIndex( i );

    } else if ( element.tagName() == "authmech" ) {
      QStringList list;
      list << "SIMPLE";

      for ( int i = 0; i < list.count(); ++i )
        if ( list[ i ] == element.text() )
          mAuthMech->setCurrentIndex( i );

    } else if ( element.tagName() == "encryption" ) {
      mEncryption->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "encryption" ) {
      mEncryption->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "ldap_read" ) {
      mReadLdap->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "ldap_write" ) {
      mWriteLdap->setChecked( element.text().toInt() == 1 );
    }
  }
}

QString ConfigGuiLdap::save()
{
  QString config = "<config>";

  config += QString( "<servername>%1</servername>" ).arg( mServerName->text() );
  config += QString( "<serverport>%1</serverport>" ).arg( mPort->value() );
  config += QString( "<binddn>%1</binddn>" ).arg( mBindDn->text() );
  config += QString( "<password>%1</password>" ).arg( mPassword->text() );
  config += QString( "<anonymous>%1</anonymous>" ).arg( mAnonymousBind->isChecked() ? "1" : "0" );
  config += QString( "<searchbase>%1</searchbase>" ).arg( mSearchBase->text() );
  config += QString( "<searchfilter>%1</searchfilter>" ).arg( mSearchFilter->text() );
  config += QString( "<storebase>%1</storebase>" ).arg( mStoreBase->text() );
  config += QString( "<keyattr>%1</keyattr>" ).arg( mKeyAttribute->text() );

  QStringList scopes;
  scopes << "base" << "one" << "sub";

  config += QString( "<scope>%1</scope>" ).arg( scopes[ mSearchScope->currentIndex() ] );

  QStringList authMechs;
  authMechs << "SIMPLE";

  config += QString( "<authmech>%1</authmech>" ).arg( authMechs[ mAuthMech->currentIndex() ] );
  config += QString( "<encryption>%1</encryption>" ).arg( mEncryption->isChecked() ? "1" : "0" );

  config += QString( "<ldap_read>%1</ldap_read>" ).arg( mReadLdap->isChecked() ? "1" : "0" );
  config += QString( "<ldap_write>%1</ldap_write>" ).arg( mWriteLdap->isChecked() ? "1" : "0" );

  config += "</config>";

  return config;
}

void ConfigGuiLdap::bindModeChanged( bool checked )
{
  mBindLabel->setEnabled( !checked );
  mBindDn->setEnabled( !checked );

  mPasswordLabel->setEnabled( !checked );
  mPassword->setEnabled( !checked );
}

void ConfigGuiLdap::initGUI()
{
  QGridLayout *layout = new QGridLayout();
  topLayout()->addLayout( layout );
  layout->setMargin( KDialog::marginHint() );
  layout->setSpacing( KDialog::spacingHint() );

  layout->addWidget( new QLabel( i18n( "Server:" ), this ), 0, 0 );
  mServerName = new KLineEdit( this );
  layout->addWidget( mServerName, 0, 1 );

  layout->addWidget( new QLabel( i18n( "Port:" ), this ), 0, 2, Qt::AlignRight );
  mPort = new QSpinBox( this );
  mPort->setRange( 1, 65536 );
  layout->addWidget( mPort, 0, 3 );

  mAnonymousBind = new QCheckBox( i18n( "Use anonymous bind" ), this );
  layout->addWidget( mAnonymousBind, 1, 0, 1, 2 );

  connect( mAnonymousBind, SIGNAL( toggled( bool ) ),
           this, SLOT( bindModeChanged( bool ) ) );

  mBindLabel = new QLabel( i18n( "Bind Dn:" ), this );
  layout->addWidget( mBindLabel, 2, 0 );
  mBindDn = new KLineEdit( this );
  layout->addWidget( mBindDn, 2, 1, 1, 2 );

  mPasswordLabel = new QLabel( i18n( "Password:" ), this );
  layout->addWidget( mPasswordLabel, 3, 0 );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );
  layout->addWidget( mPassword, 3, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Search Base:" ), this ), 4, 0 );
  mSearchBase = new KLineEdit( this );
  layout->addWidget( mSearchBase, 4, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Search Filter:" ), this ), 5, 0 );
  mSearchFilter = new KLineEdit( this );
  layout->addWidget( mSearchFilter, 5, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Storage Base:" ), this ), 6, 0 );
  mStoreBase = new KLineEdit( this );
  layout->addWidget( mStoreBase, 6, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Key Attribute:" ), this ), 7, 0 );
  mKeyAttribute = new KLineEdit( this );
  layout->addWidget( mKeyAttribute, 7, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Search Scope:" ), this ), 8, 0 );
  mSearchScope = new KComboBox( this );
  layout->addWidget( mSearchScope, 8, 1, 1, 2 );

  layout->addWidget( new QLabel( i18n( "Authentication Mechanism:" ), this ), 9, 0 );
  mAuthMech = new KComboBox( this );
  layout->addWidget( mAuthMech, 9, 1, 1, 2 );

  mEncryption = new QCheckBox( i18n( "Use encryption" ), this );
  layout->addWidget( mEncryption, 10, 1, 1, 2 );

  mReadLdap = new QCheckBox( i18n( "Load data from LDAP" ), this );
  layout->addWidget( mReadLdap, 11, 1, 1, 2 );

  mWriteLdap = new QCheckBox( i18n( "Save data to LDAP" ), this );
  layout->addWidget( mWriteLdap, 11, 2, 1, 2 );
}

#include "configguildap.moc"
