/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>

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

#include "kcal_resourcegroupwarebaseconfig.h"

#include "kcal_resourcegroupwarebase.h"
#include "kcal_groupwareprefs.h"
#include "folderconfig.h"

#include <libkcal/resourcecachedconfig.h>

#include <klocale.h>
#include <kdebug.h>
#include <klineedit.h>
#include <kdialog.h>

#include <qlabel.h>
#include <qlayout.h>


using namespace KCal;

ResourceGroupwareBaseConfig::ResourceGroupwareBaseConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 );

  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );
  mainLayout->setSpacing( KDialog::spacingHint() );

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
  mainLayout->addMultiCellWidget( mReloadConfig, 1, 3, 2, 2 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 4, 4, 2, 2 );

  mFolderConfig = new KPIM::FolderConfig( this );
  connect( mFolderConfig, SIGNAL( updateFoldersClicked() ),
    SLOT( updateFolders() ) );
  mainLayout->addMultiCellWidget( mFolderConfig, 4, 4, 0, 1 );
}

void ResourceGroupwareBaseConfig::loadSettings( KRES::Resource *resource )
{
  kdDebug() << "KCal::ResourceGroupwareBaseConfig::loadSettings()" << endl;

  ResourceGroupwareBase *res = static_cast<ResourceGroupwareBase *>( resource );
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
    
    mFolderConfig->setFolderLister( res->folderLister() );
    mFolderConfig->updateFolderList();
  } else {
    kdError(5700) << "KCalResourceGroupwareBaseConfig::loadSettings(): no KCalOpenGroupware, cast failed" << endl;
  }
}

void ResourceGroupwareBaseConfig::saveSettings( KRES::Resource *resource )
{
  ResourceGroupwareBase *res = static_cast<ResourceGroupwareBase*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mUrl->text() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
    
    mFolderConfig->saveSettings();
  } else {
    kdError(5700) << "KCalResourceGroupwareBaseConfig::saveSettings(): no KCalOpenGroupware, cast failed" << endl;
  }
}

void ResourceGroupwareBaseConfig::updateFolders()
{
  KURL url = mUrl->text();
  url.setUser( mUserEdit->text() );
  url.setPass( mPasswordEdit->text() );

  mFolderConfig->retrieveFolderList( url );
}

#include "kcal_resourcegroupwarebaseconfig.moc"
