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

  mSearchScope->insertItem( i18n( "Base" ) );
  mSearchScope->insertItem( i18n( "One" ) );
  mSearchScope->insertItem( i18n( "Sub" ) );
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
      mLdapWidget->setHost( element.text() );
    } else if ( element.tagName() == "serverport" ) {
      mLdapWidget->setPort( element.text().toInt() );
    } else if ( element.tagName() == "binddn" ) {
      mLdapWidget->setBindDN( element.text() );
    } else if ( element.tagName() == "password" ) {
      mLdapWidget->setPassword( element.text() );
    } else if ( element.tagName() == "anonymous" ) { 
        mLdapWidget->setAuthAnon( element.text().toInt() == 1 );
    } else if ( element.tagName() == "searchbase" ) {
      mLdapWidget->setDn( element.text() );
    } else if ( element.tagName() == "searchfilter" ) {
      mLdapWidget->setFilter( element.text() );
    } else if ( element.tagName() == "storebase" ) {
      mLdapWidget->setDn( element.text() );
    } else if ( element.tagName() == "keyattr" ) {
      mKeyAttribute->setText( element.text() );
    } else if ( element.tagName() == "scope" ) {
      QStringList list;
      list << "base" << "one" << "sub";
      for ( uint i = 0; i < list.count(); ++i )
        if ( list[ i ] == element.text() )
          mSearchScope->setCurrentItem( i );

    } else if ( element.tagName() == "authmech" ) {
      if ( element.text() == "SIMPLE" ) {
        mLdapWidget->setAuthSimple( true );
      }
    } else if ( element.tagName() == "encryption" ) {
        mEncryption->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "ldap_read" ) {
      mReadLdap->setChecked( element.text().toInt() == 1 );
    } else if ( element.tagName() == "ldap_write" ) {
      mWriteLdap->setChecked( element.text().toInt() == 1 );
    }
  }
}

QString ConfigGuiLdap::save() const
{
  QString config = "<config>\n";

  config += QString( "<servername>%1</servername>\n" ).arg( mLdapWidget->host() );
  config += QString( "<serverport>%1</serverport>\n" ).arg( mLdapWidget->port() );
  config += QString( "<binddn>%1</binddn>\n" ).arg( mLdapWidget->bindDN() );
  config += QString( "<password>%1</password>\n" ).arg( mLdapWidget->password() );
  config += QString( "<anonymous>%1</anonymous>\n" ).arg( mLdapWidget->isAuthAnon() ? "1" : "0" );
  config += QString( "<searchbase>%1</searchbase>\n" ).arg( mLdapWidget->dn() );
  config += QString( "<searchfilter>%1</searchfilter>\n" ).arg( mLdapWidget->filter() );
  config += QString( "<storebase>%1</storebase>\n" ).arg( mLdapWidget->dn() );
  config += QString( "<keyattr>%1</keyattr>\n" ).arg( mKeyAttribute->text() );

  QStringList scopes;
  scopes << "base" << "one" << "sub";

  config += QString( "<scope>%1</scope>\n" ).arg( scopes[ mSearchScope->currentItem() ] );

  config += QString( "<authmech>SIMPLE</authmech>\n" );
  config += QString( "<encryption>%1</encryption>\n" ).arg( mEncryption->isChecked() ? "1" : "0" );

  config += QString( "<ldap_read>%1</ldap_read>\n" ).arg( mReadLdap->isChecked() ? "1" : "0" );
  config += QString( "<ldap_write>%1</ldap_write>\n" ).arg( mWriteLdap->isChecked() ? "1" : "0" );

  config += "</config>";

  return config;
}

void ConfigGuiLdap::initGUI()
{
  QGridLayout *layout = new QGridLayout( topLayout(), 12, 4, KDialog::spacingHint() );
  layout->setMargin( KDialog::marginHint() );

  mLdapWidget = new KABC::LdapConfigWidget( KABC::LdapConfigWidget::W_HOST |
                                            KABC::LdapConfigWidget::W_PORT |
                                            KABC::LdapConfigWidget::W_USER |
                                            KABC::LdapConfigWidget::W_PASS |
                                            KABC::LdapConfigWidget::W_BINDDN |
                                            KABC::LdapConfigWidget::W_DN |
                                            KABC::LdapConfigWidget::W_FILTER |
                                            KABC::LdapConfigWidget::W_AUTHBOX, this );

  mKeyAttribute = new KLineEdit( this );
  mSearchScope = new KComboBox( this );
  mEncryption = new QCheckBox( i18n( "Use encryption" ), this );
  mReadLdap = new QCheckBox( i18n( "Load data from LDAP" ), this );
  mWriteLdap = new QCheckBox( i18n( "Save data to LDAP" ), this );

  layout->addMultiCellWidget( mLdapWidget, 0, 9, 0, 3 );
  layout->addWidget( new QLabel( i18n( "Key Attribute:" ), this ), 10, 0 );
  layout->addMultiCellWidget( mKeyAttribute, 10, 10, 1, 2 );
  layout->addWidget( new QLabel( i18n( "Search Scope:" ), this ), 11, 0 );
  layout->addMultiCellWidget( mSearchScope, 11, 11, 1, 2 );
  layout->addWidget( mEncryption, 12, 0 );
  layout->addWidget( mReadLdap, 13, 0 );
  layout->addWidget( mWriteLdap, 13, 3 );

}

#include "configguildap.moc"
