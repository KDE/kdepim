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

#include <tqcheckbox.h>
#include <tqdom.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqspinbox.h>

#include <kcombobox.h>
#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

ConfigGuiLdap::ConfigGuiLdap( const QSync::Member &member, TQWidget *parent )
  : ConfigGui( member, parent )
{
  initGUI();

  mSearchScope->insertItem( i18n( "Base" ) );
  mSearchScope->insertItem( i18n( "One" ) );
  mSearchScope->insertItem( i18n( "Sub" ) );
}

void ConfigGuiLdap::load( const TQString &xml )
{
  TQDomDocument doc;
  doc.setContent( xml );
  TQDomElement docElement = doc.documentElement();
  TQDomNode node;
  for( node = docElement.firstChild(); !node.isNull(); node = node.nextSibling() ) {
    TQDomElement element = node.toElement();
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
      TQStringList list;
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

TQString ConfigGuiLdap::save() const
{
  TQString config = "<config>\n";

  config += TQString( "<servername>%1</servername>\n" ).arg( mLdapWidget->host() );
  config += TQString( "<serverport>%1</serverport>\n" ).arg( mLdapWidget->port() );
  config += TQString( "<binddn>%1</binddn>\n" ).arg( mLdapWidget->bindDN() );
  config += TQString( "<password>%1</password>\n" ).arg( mLdapWidget->password() );
  config += TQString( "<anonymous>%1</anonymous>\n" ).arg( mLdapWidget->isAuthAnon() ? "1" : "0" );
  config += TQString( "<searchbase>%1</searchbase>\n" ).arg( mLdapWidget->dn() );
  config += TQString( "<searchfilter>%1</searchfilter>\n" ).arg( mLdapWidget->filter() );
  config += TQString( "<storebase>%1</storebase>\n" ).arg( mLdapWidget->dn() );
  config += TQString( "<keyattr>%1</keyattr>\n" ).arg( mKeyAttribute->text() );

  TQStringList scopes;
  scopes << "base" << "one" << "sub";

  config += TQString( "<scope>%1</scope>\n" ).arg( scopes[ mSearchScope->currentItem() ] );

  config += TQString( "<authmech>SIMPLE</authmech>\n" );
  config += TQString( "<encryption>%1</encryption>\n" ).arg( mEncryption->isChecked() ? "1" : "0" );

  config += TQString( "<ldap_read>%1</ldap_read>\n" ).arg( mReadLdap->isChecked() ? "1" : "0" );
  config += TQString( "<ldap_write>%1</ldap_write>\n" ).arg( mWriteLdap->isChecked() ? "1" : "0" );

  config += "</config>";

  return config;
}

void ConfigGuiLdap::initGUI()
{
  TQGridLayout *layout = new TQGridLayout( topLayout(), 12, 4, KDialog::spacingHint() );
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
  mEncryption = new TQCheckBox( i18n( "Use encryption" ), this );
  mReadLdap = new TQCheckBox( i18n( "Load data from LDAP" ), this );
  mWriteLdap = new TQCheckBox( i18n( "Save data to LDAP" ), this );

  layout->addMultiCellWidget( mLdapWidget, 0, 9, 0, 3 );
  layout->addWidget( new TQLabel( i18n( "Key Attribute:" ), this ), 10, 0 );
  layout->addMultiCellWidget( mKeyAttribute, 10, 10, 1, 2 );
  layout->addWidget( new TQLabel( i18n( "Search Scope:" ), this ), 11, 0 );
  layout->addMultiCellWidget( mSearchScope, 11, 11, 1, 2 );
  layout->addWidget( mEncryption, 12, 0 );
  layout->addWidget( mReadLdap, 13, 0 );
  layout->addWidget( mWriteLdap, 13, 3 );

}

#include "configguildap.moc"
