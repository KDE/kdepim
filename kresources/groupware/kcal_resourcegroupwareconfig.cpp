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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <QLabel>
#include <QLayout>
#include <QCheckBox>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klineedit.h>

#include <kcal/resourcecachedconfig.h>

#include "kcal_resourcegroupware.h"
#include "kcal_groupwareprefsbase.h"

#include "kcal_resourcegroupwareconfig.h"

using namespace KCal;

ResourceGroupwareConfig::ResourceGroupwareConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this );

  QLabel *label = new QLabel( i18n("URL:"), this );
  mainLayout->addWidget( label, 1, 0 );
  mUrl = new KLineEdit( this );
  mainLayout->addWidget( mUrl, 1, 1 );
  
  label = new QLabel( i18n("User:"), this );
  mainLayout->addWidget( label, 2, 0 );
  mUserEdit = new KLineEdit( this );
  mainLayout->addWidget( mUserEdit, 2, 1 );
  
  label = new QLabel( i18n("Password:"), this );
  mainLayout->addWidget( label, 3, 0 );
  mPasswordEdit = new KLineEdit( this );
  mainLayout->addWidget( mPasswordEdit, 3, 1 );
  mPasswordEdit->setEchoMode( KLineEdit::Password );

  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addWidget( mReloadConfig, 5, 0, 1, 2 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addWidget( mSaveConfig, 6, 0, 1, 2 );
}

void ResourceGroupwareConfig::loadSettings( KRES::Resource *resource )
{
  kDebug() <<"KCal::ResourceGroupwareConfig::loadSettings()";

  ResourceGroupware *res = static_cast<ResourceGroupware *>( resource );
  if ( res ) {
    if ( !res->prefs() ) {
      kError() <<"No PREF";
      return;
    }
  
    mUrl->setText( res->prefs()->url() );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kError(5700) <<"KCalResourceGroupwareConfig::loadSettings(): no KCalResourceGroupware, cast failed";
  }
}

void ResourceGroupwareConfig::saveSettings( KRES::Resource *resource )
{
  ResourceGroupware *res = static_cast<ResourceGroupware*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mUrl->text() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kError(5700) <<"KCalResourceGroupwareConfig::saveSettings(): no KCalResourceGroupware, cast failed";
  }
}

#include "kcal_resourcegroupwareconfig.moc"
