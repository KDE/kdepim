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

#include "kcalresourceslox.h"
#include "kcalsloxprefs.h"

#include "kcalresourcesloxconfig.h"

KCalResourceSloxConfig::KCalResourceSloxConfig( QWidget* parent,  const char* name )
    : KRES::ConfigWidget( parent, name )
{
  resize( 245, 115 ); 
  QGridLayout *mainLayout = new QGridLayout( this, 2, 2 );

  // FIXME: Post 3.2: i18n("Download from:") ( bug 67330 )
  QLabel *label = new QLabel( i18n( "Download URL:" ), this );

  mDownloadUrl = new KURLRequester( this );
  mDownloadUrl->setMode( KFile::File );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mDownloadUrl, 1, 1 );

  label = new QLabel( i18n("User:"), this );
  mainLayout->addWidget( label, 2, 0 );
  
  mUserEdit = new KLineEdit( this );
  mainLayout->addWidget( mUserEdit, 2, 1 );
  
  label = new QLabel( i18n("Password:"), this );
  mainLayout->addWidget( label, 3, 0 );
  
  mPasswordEdit = new KLineEdit( this );
  mainLayout->addWidget( mPasswordEdit, 3, 1 );
  mPasswordEdit->setEchoMode( KLineEdit::Password );

  mLastSyncCheck = new QCheckBox( i18n("Only load data since last sync"),
                                  this );
  mainLayout->addMultiCellWidget( mLastSyncCheck, 4, 4, 0, 1 );

  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 5, 5, 0, 1 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 6, 6, 0, 1 );
}

void KCalResourceSloxConfig::loadSettings( KRES::Resource *resource )
{
  KCalResourceSlox *res = static_cast<KCalResourceSlox *>( resource );
  if ( res ) {
    mDownloadUrl->setURL( res->prefs()->url() );
    mLastSyncCheck->setChecked( res->prefs()->useLastSync() );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mReloadConfig->loadSettings( res );
    mSaveConfig->loadSettings( res );
  } else {
    kdError(5700) << "KCalResourceSloxConfig::loadSettings(): no KCalResourceSlox, cast failed" << endl;
  }
}

void KCalResourceSloxConfig::saveSettings( KRES::Resource *resource )
{
  KCalResourceSlox *res = static_cast<KCalResourceSlox*>( resource );
  if ( res ) {
    res->prefs()->setUrl( mDownloadUrl->url() );
    res->prefs()->setUseLastSync( mLastSyncCheck->isChecked() );
    res->prefs()->setUser( mUserEdit->text() );
    res->prefs()->setPassword( mPasswordEdit->text() );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kdError(5700) << "KCalResourceSloxConfig::saveSettings(): no KCalResourceSlox, cast failed" << endl;
  }
}

#include "kcalresourcesloxconfig.moc"
