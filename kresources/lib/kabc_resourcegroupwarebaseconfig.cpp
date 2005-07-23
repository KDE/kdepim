/*
    This file is part of kdepim.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Till Adam <adam@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "kabc_resourcegroupwarebaseconfig.h"

#include "kabc_resourcegroupwarebase.h"
#include "kresources_groupwareprefs.h"
#include "folderconfig.h"

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <qlabel.h>
#include <qlayout.h>

using namespace KABC;

ResourceGroupwareBaseConfig::ResourceGroupwareBaseConfig( QWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent, name )
{
  QGridLayout *mainLayout = new QGridLayout( this, 7, 2, 0, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "URL:" ), this );
  mURL = new KURLRequester( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mURL, 0, 1 );

  label = new QLabel( i18n( "User:" ), this );
  mUser = new KLineEdit( this );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mUser, 1, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mPassword, 2, 1 );

  QFrame *hline = new QFrame( this );
  hline->setFrameStyle( QFrame::HLine | QFrame::Sunken );

  mainLayout->addMultiCellWidget( hline, 3, 3, 0, 1 );

  mFolderConfig = new KPIM::FolderConfig( this );
  connect( mFolderConfig, SIGNAL( updateFoldersClicked() ),
    SLOT( updateFolders() ) );
  mainLayout->addMultiCellWidget( mFolderConfig, 4, 4, 0, 1 );
}

void ResourceGroupwareBaseConfig::loadSettings( KRES::Resource *res )
{
  mResource = dynamic_cast<ResourceGroupwareBase*>( res );
  
  if ( !mResource ) {
    kdDebug(5700) << "ResourceGroupwareBaseConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mURL->setURL( mResource->prefs()->url() );
  mUser->setText( mResource->prefs()->user() );
  mPassword->setText( mResource->prefs()->password() );

  mFolderConfig->setFolderLister( mResource->folderLister() );
  mFolderConfig->updateFolderList();
}

void ResourceGroupwareBaseConfig::saveSettings( KRES::Resource *res )
{
  ResourceGroupwareBase *resource = dynamic_cast<ResourceGroupwareBase*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceGroupwareBaseConfig::saveSettings(): cast failed" << endl;
    return;
  }

  resource->prefs()->setUrl( mURL->url() );
  resource->prefs()->setUser( mUser->text() );
  resource->prefs()->setPassword( mPassword->text() );  

  mFolderConfig->saveSettings();
}

void ResourceGroupwareBaseConfig::updateFolders()
{
  KURL url = mURL->url();
  url.setUser( mUser->text() );
  url.setPass( mPassword->text() );

  mFolderConfig->retrieveFolderList( url );
}

#include "kabc_resourcegroupwarebaseconfig.moc"
