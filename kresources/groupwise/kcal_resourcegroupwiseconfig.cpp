/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klineedit.h>

#include <libkcal/resourcecachedconfig.h>

#include "kcal_resourcegroupwise.h"
#include "kcal_groupwiseprefs.h"

#include "kcal_resourcegroupwiseconfig.h"

using namespace KCal;

ResourceGroupwiseConfig::ResourceGroupwiseConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n("Server:"), this );
  mainLayout->addWidget( label, 1, 0 );
  mHost = new KLineEdit( this );
  mainLayout->addWidget( mHost, 1, 1 );
  
  label = new QLabel( i18n("Port:"), this );
  mainLayout->addWidget( label, 2, 0 );
  mPort = new KLineEdit( this );
  mainLayout->addWidget( mPort, 2, 1 );
  
  label = new QLabel( i18n("User:"), this );
  mainLayout->addWidget( label, 3, 0 );
  mUserEdit = new KLineEdit( this );
  mainLayout->addWidget( mUserEdit, 3, 1 );
  
  label = new QLabel( i18n("Password:"), this );
  mainLayout->addWidget( label, 4, 0 );
  mPasswordEdit = new KLineEdit( this );
  mainLayout->addWidget( mPasswordEdit, 4, 1 );
  mPasswordEdit->setEchoMode( KLineEdit::Password );

  mSSL = new QCheckBox( i18n("&Encrypt communication with server"), this );
  mainLayout->addWidget( mSSL, 5, 0 );

#if 0
  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 5, 5, 0, 1 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 6, 6, 0, 1 );
#endif
}

void ResourceGroupwiseConfig::loadSettings( KRES::Resource *resource )
{
  kdDebug() << "KCal::ResourceGroupwiseConfig::loadSettings()" << endl;

  ResourceGroupwise *res = static_cast<ResourceGroupwise *>( resource );
  if ( res ) {
    if ( !res->prefs() ) {
      kdError() << "No PREF" << endl;
      return;
    }
  
    mHost->setText( res->prefs()->host() );
    mPort->setText( QString::number(res->prefs()->port()) );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mSSL->setChecked( res->prefs()->useHttps() );
#if 0
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
#endif
  } else {
    kdError(5700) << "KCalResourceGroupwiseConfig::loadSettings(): no KCalResourceGroupwise, cast failed" << endl;
  }
}

void ResourceGroupwiseConfig::saveSettings( KRES::Resource *resource )
{
  ResourceGroupwise *res = static_cast<ResourceGroupwise*>( resource );
  if ( res ) {
    res->prefs()->setHost( mHost->text() );
    res->prefs()->setPort( mPort->text().toUInt() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    res->prefs()->setUseHttps( mSSL->isChecked() );
#if 0
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
#endif
  } else {
    kdError(5700) << "KCalResourceGroupwiseConfig::saveSettings(): no KCalResourceGroupwise, cast failed" << endl;
  }
}

#include "kcal_resourcegroupwiseconfig.moc"
