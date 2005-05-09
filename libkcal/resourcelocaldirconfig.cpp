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

#include "resourcelocaldir.h"

#include "resourcelocaldirconfig.h"

using namespace KCal;

ResourceLocalDirConfig::ResourceLocalDirConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Location:" ), this );
  mURL = new KURLRequester( this );
  mURL->setMode( KFile::Directory | KFile::LocalOnly );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mURL, 1, 1 );
}

void ResourceLocalDirConfig::loadSettings( KRES::Resource *resource )
{
  ResourceLocalDir* res = static_cast<ResourceLocalDir*>( resource );
  if ( res ) {
    mURL->setURL( res->mURL.prettyURL() );
  } else
    kdDebug(5700) << "ERROR: ResourceLocalDirConfig::loadSettings(): no ResourceLocalDir, cast failed" << endl;
}

void ResourceLocalDirConfig::saveSettings( KRES::Resource *resource )
{
  ResourceLocalDir* res = static_cast<ResourceLocalDir*>( resource );
  if (res) {
    res->mURL = mURL->url();
  } else
    kdDebug(5700) << "ERROR: ResourceLocalDirConfig::saveSettings(): no ResourceLocalDir, cast failed" << endl;
}

#include "resourcelocaldirconfig.moc"
