/*
    This file is part of libkcal.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"

#include "resourceremote.h"

#include "resourceremoteconfig.h"

using namespace KCal;

ResourceRemoteConfig::ResourceRemoteConfig( QWidget* parent,  const char* name )
    : KRES::ResourceConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Download URL:" ), this );

  mDownloadUrl = new KURLRequester( this );
  mDownloadUrl->setMode( KFile::Files );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mDownloadUrl, 1, 1 );

  label = new QLabel( i18n( "Upload URL:" ), this );
  mUploadUrl = new KURLRequester( this );
  mUploadUrl->setMode( KFile::Files );
  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mUploadUrl, 2, 1 );
}

void ResourceRemoteConfig::loadSettings( KRES::Resource *resource )
{
  ResourceRemote *res = static_cast<ResourceRemote *>( resource );
  if ( res ) {
    mDownloadUrl->setURL( res->downloadUrl().url() );
    mUploadUrl->setURL( res->uploadUrl().url() );
  } else {
    kdDebug(5700) << "ERROR: ResourceRemoteConfig::loadSettings(): no ResourceRemote, cast failed" << endl;
  }
}

void ResourceRemoteConfig::saveSettings( KRES::Resource *resource )
{
  ResourceRemote* res = static_cast<ResourceRemote*>( resource );
  if ( res ) {
    res->setDownloadUrl( mDownloadUrl->url() );
    res->setUploadUrl( mUploadUrl->url() );
  } else {
    kdDebug(5700) << "ERROR: ResourceRemoteConfig::saveSettings(): no ResourceRemote, cast failed" << endl;
  }
}

#include "resourceremoteconfig.moc"
