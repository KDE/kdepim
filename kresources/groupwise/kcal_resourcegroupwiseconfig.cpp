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
  mainLayout->addMultiCellWidget( mReloadConfig, 5, 5, 0, 1 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 6, 6, 0, 1 );
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
  
    mUrl->setText( res->prefs()->url() );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kdError(5700) << "KCalResourceGroupwiseConfig::loadSettings(): no KCalResourceGroupwise, cast failed" << endl;
  }
}

void ResourceGroupwiseConfig::saveSettings( KRES::Resource *resource )
{
  ResourceGroupwise *res = static_cast<ResourceGroupwise*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mUrl->text() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kdError(5700) << "KCalResourceGroupwiseConfig::saveSettings(): no KCalResourceGroupwise, cast failed" << endl;
  }
}

#include "kcal_resourcegroupwiseconfig.moc"
