/*
    This file is part of libkpimexchange.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>

#include "exchangeaccount.h"
#include "resourceexchangeconfig.h"
#include "resourceexchange.h"

using namespace KCal;

ResourceExchangeConfig::ResourceExchangeConfig( QWidget* parent,  const char* name )
    : KRES::ResourceConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  QLabel *label = new QLabel( i18n( "Host:" ), this );
  mHostEdit = new KLineEdit( this );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mHostEdit, 1, 1 );

  label = new QLabel( i18n( "Account:" ), this );
  mAccountEdit = new KLineEdit( this );
  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mAccountEdit, 2, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPasswordEdit = new KLineEdit( this );
  mPasswordEdit->setEchoMode( QLineEdit::Password );
  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( mPasswordEdit, 3, 1 );

  label = new QLabel( i18n( "Cache timeout:" ), this );
  mCacheEdit = new KIntNumInput( this );
  mCacheEdit->setMinValue( 0 );
  mCacheEdit->setSuffix( i18n(" seconds") );
  mainLayout->addWidget( label, 4, 0 );
  mainLayout->addWidget( mCacheEdit, 4, 1 );
}

void ResourceExchangeConfig::loadSettings( KRES::Resource *resource )
{
  ResourceExchange* res = dynamic_cast<ResourceExchange*>( resource );
  if (res) {
    mHostEdit->setText( res->mAccount->host() );
    mAccountEdit->setText( res->mAccount->account() );
    mPasswordEdit->setText( res->mAccount->password() );
    mCacheEdit->setValue( res->mCachedSeconds );
  } else
    kdDebug(5700) << "ERROR: ResourceExchangeConfig::loadSettings(): no ResourceExchange, cast failed" << endl;
}

void ResourceExchangeConfig::saveSettings( KRES::Resource *resource )
{
  ResourceExchange* res = dynamic_cast<ResourceExchange*>( resource );
  if (res) {
    res->mAccount->setHost(mHostEdit->text());
    res->mAccount->setAccount(mAccountEdit->text());
    res->mAccount->setPassword(mPasswordEdit->text());
    res->mCachedSeconds = mCacheEdit->value();
  } else
    kdDebug(5700) << "ERROR: ResourceExchangeConfig::saveSettings(): no ResourceExchange, cast failed" << endl;
}

#include "resourceexchangeconfig.moc"
