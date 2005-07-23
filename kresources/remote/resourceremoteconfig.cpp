/*
    This file is part of libkcal.

    Copyright (c) 2003,2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kdialog.h>

#include <libkcal/resourcecachedconfig.h>

#include "resourceremote.h"

#include "resourceremoteconfig.h"

using namespace KCal;

ResourceRemoteConfig::ResourceRemoteConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );
  mainLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Download from:" ), this );

  mDownloadUrl = new KURLRequester( this );
  mDownloadUrl->setMode( KFile::File );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mDownloadUrl, 1, 1 );

  label = new QLabel( i18n( "Upload to:" ), this );
  mUploadUrl = new KURLRequester( this );
  mUploadUrl->setMode( KFile::File );
  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mUploadUrl, 2, 1 );

  mReloadConfig = new ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 3, 3, 0, 1 );

  mSaveConfig = new ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 4, 4, 0, 1 );
}

void ResourceRemoteConfig::loadSettings( KRES::Resource *resource )
{
  ResourceRemote *res = static_cast<ResourceRemote *>( resource );
  if ( res ) {
    mDownloadUrl->setURL( res->downloadUrl().url() );
    mUploadUrl->setURL( res->uploadUrl().url() );
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kdError(5700) << "ResourceRemoteConfig::loadSettings(): no ResourceRemote, cast failed" << endl;
  }
}

void ResourceRemoteConfig::saveSettings( KRES::Resource *resource )
{
  ResourceRemote* res = static_cast<ResourceRemote*>( resource );
  if ( res ) {
    res->setDownloadUrl( KURL( mDownloadUrl->url() ) );
    res->setUploadUrl( KURL( mUploadUrl->url() ) );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );


    if ( mUploadUrl->url().isEmpty() && !resource->readOnly() ) {
      KMessageBox::information( this, i18n( "You have specified no upload URL, "
			    "the calendar will be read-only." ), "RemoteResourseNoUploadURL" );
      resource->setReadOnly( true );
    }
  } else {
    kdError(5700) << "ResourceRemoteConfig::saveSettings(): no ResourceRemote, cast failed" << endl;
  }
}

#include "resourceremoteconfig.moc"
