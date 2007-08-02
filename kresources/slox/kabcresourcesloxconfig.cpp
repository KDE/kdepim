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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kabcresourcesloxconfig.h"

#include "kabcresourceslox.h"
#include "kabcsloxprefs.h"
#include "sloxbase.h"
#include "sloxfolder.h"
#include "sloxfolderdialog.h"
#include "sloxfoldermanager.h"

#include <kdebug.h>
#include <kdialog.h>
#include <klocale.h>
#include <klineedit.h>
#include <kurlrequester.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QGridLayout>

using namespace KABC;

ResourceSloxConfig::ResourceSloxConfig( QWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent ), mRes( 0 )
{
  setObjectName(name);
  QGridLayout *mainLayout = new QGridLayout( this );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( 0 );

  QLabel *label = new QLabel( i18n( "URL:" ), this );
  mURL = new KUrlRequester( this );

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

  mLastSyncCheck = new QCheckBox( i18n("Only load data since last sync"),
                                  this );
  mainLayout->addWidget( mLastSyncCheck, 3, 0, 1, 2);

  mFolderButton = new KPushButton( i18n("Select Folder..."), this );
  mainLayout->addWidget( mFolderButton, 4, 0, 1, 2 );
  connect( mFolderButton, SIGNAL( clicked() ), SLOT( selectAddressFolder() ) );

}

void ResourceSloxConfig::loadSettings( KRES::Resource *res )
{
  ResourceSlox *resource = dynamic_cast<ResourceSlox*>( res );
  mRes = resource;

  if ( !resource ) {
    kDebug(5700) <<"ResourceSloxConfig::loadSettings(): cast failed";
    return;
  }

  if ( mRes->resType() == "slox" )
    mFolderButton->setEnabled( false ); // TODO folder selection for SLOX

  mURL->setUrl( resource->prefs()->url() );
  mUser->setText( resource->prefs()->user() );
  mPassword->setText( resource->prefs()->password() );
  mLastSyncCheck->setChecked( resource->prefs()->useLastSync() );
  mFolderId = resource->prefs()->folderId();
}

void ResourceSloxConfig::saveSettings( KRES::Resource *res )
{
  ResourceSlox *resource = dynamic_cast<ResourceSlox*>( res );

  if ( !resource ) {
    kDebug(5700) <<"ResourceSloxConfig::saveSettings(): cast failed";
    return;
  }

  resource->prefs()->setUrl( mURL->url().url() );
  resource->prefs()->setUser( mUser->text() );
  resource->prefs()->setPassword( mPassword->text() );
  resource->prefs()->setUseLastSync( mLastSyncCheck->isChecked() );
  resource->prefs()->setFolderId( mFolderId );
}

void KABC::ResourceSloxConfig::selectAddressFolder( )
{
  SloxFolderManager *manager = new SloxFolderManager( mRes, mURL->url() );
  SloxFolderDialog *dialog = new SloxFolderDialog( manager, Contacts, this );
  dialog->setSelectedFolder( mFolderId );
  if ( dialog->exec() == QDialog::Accepted )
    mFolderId = dialog->selectedFolder();
}

#include "kabcresourcesloxconfig.moc"
