/*
    This file is part of libkcal.

    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
    Based on the remote resource:
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
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kcal_resourcebloggingconfig.h"

#include "kcal_resourceblogging.h"
#include "resourcebloggingsettings.h"


#include <libkcal/resourcecachedconfig.h>

#include <kdialog.h>
#include <kurl.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kcombobox.h>

#include <qlayout.h>


using namespace KCal;
using namespace KBlog;

ResourceBloggingConfig::ResourceBloggingConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );
  mainLayout->setSpacing( KDialog::spacingHint() );

  mPage = new ResourceBloggingSettings( this );
  mainLayout->addMultiCellWidget( mPage, 1, 1, 0, 1 );

  mReloadConfig = new ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 2, 2, 0, 1 );

  mSaveConfig = new ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 3, 3, 0, 1 );
}

void ResourceBloggingConfig::loadSettings( KRES::Resource *resource )
{
  ResourceBlogging *res = static_cast<ResourceBlogging *>( resource );
  if ( res && mPage ) {
    KURL url( res->url() );
    
    mPage->mUser->setText( url.user() );
    mPage->mPassword->setText( url.pass() );
    
    url.setUser( QString::null ) ;
    url.setPass( QString::null );
    mPage->mURL->setURL( url.url() );
    
    mPage->mServerAPI->setCurrentItem( res->serverAPI() );
    
    const BlogTemplate templ( res->getTemplate() );
    mPage->mOpenTitle->setText( templ.titleTagOpen() );
    mPage->mCloseTitle->setText( templ.titleTagClose() );
    mPage->mOpenCategory->setText( templ.categoryTagOpen() );
    mPage->mCloseCategory->setText( templ.categoryTagClose() );
    
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kdError(5700) << "ResourceBloggingConfig::loadSettings(): no ResourceBlogging, cast failed" << endl;
  }
}

void ResourceBloggingConfig::saveSettings( KRES::Resource *resource )
{
  ResourceBlogging* res = static_cast<ResourceBlogging*>( resource );
  if ( res && mPage ) {
    KURL url( mPage->mURL->url() );
    QString user( mPage->mUser->text() );
    if ( !user.isEmpty() ) url.setUser( user );
    QString pw( mPage->mPassword->text() );
    if ( !pw.isEmpty() ) url.setPass( pw );

    res->setURL( url );
    
    res->setServerAPI( mPage->mServerAPI->currentItem() );
    
    BlogTemplate templ;
    templ.setTitleTagOpen( mPage->mOpenTitle->text() );
    templ.setTitleTagClose( mPage->mCloseTitle->text() );
    templ.setCategoryTagOpen( mPage->mOpenCategory->text() );
    templ.setCategoryTagClose( mPage->mCloseCategory->text() );
    res->setTemplate( templ );

    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );

  } else {
    kdError(5700) << "ResourceBloggingConfig::saveSettings(): no ResourceBlogging, cast failed" << endl;
  }
}

#include "kcal_resourcebloggingconfig.moc"
