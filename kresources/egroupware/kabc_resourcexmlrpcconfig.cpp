/*
    This file is part of libkabc.
    Copyright (c) 2002 - 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include "kabc_resourcexmlrpc.h"
#include "kabc_resourcexmlrpcconfig.h"

using namespace KABC;

ResourceXMLRPCConfig::ResourceXMLRPCConfig( QWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent, name )
{
  QGridLayout *mainLayout = new QGridLayout( this, 4, 2, 0, KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "URL:" ), this );
  mURL = new KURLRequester( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mURL, 0, 1 );

  label = new QLabel( i18n( "Domain:" ), this );
  mDomain = new KLineEdit( this );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mDomain, 1, 1 );

  label = new QLabel( i18n( "User:" ), this );
  mUser = new KLineEdit( this );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mUser, 2, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( mPassword, 3, 1 );
}

void ResourceXMLRPCConfig::loadSettings( KRES::Resource *res )
{
  ResourceXMLRPC *resource = dynamic_cast<ResourceXMLRPC*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceXMLRPCConfig::loadSettings(): cast failed" << endl;
    return;
  }

  mURL->setURL( resource->url().url() );
  mDomain->setText( resource->domain() );
  mUser->setText( resource->user() );
  mPassword->setText( resource->password() );
}

void ResourceXMLRPCConfig::saveSettings( KRES::Resource *res )
{
  ResourceXMLRPC *resource = dynamic_cast<ResourceXMLRPC*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceXMLRPCConfig::saveSettings(): cast failed" << endl;
    return;
  }

  resource->setURL( KURL( mURL->url() ) );
  resource->setDomain( mDomain->text() );
  resource->setUser( mUser->text() );
  resource->setPassword( mPassword->text() );
}

#include "kabc_resourcexmlrpcconfig.moc"
