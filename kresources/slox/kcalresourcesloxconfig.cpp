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

#include <typeinfo>

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kpushbutton.h>

#include <libkcal/resourcecachedconfig.h>

#include "kcalresourceslox.h"
#include "kcalsloxprefs.h"
#include "sloxfolder.h"
#include "sloxfolderdialog.h"
#include "sloxfoldermanager.h"

#include "kcalresourcesloxconfig.h"

KCalResourceSloxConfig::KCalResourceSloxConfig( QWidget* parent,  const char* name ) :
  KRES::ConfigWidget( parent, name ), mRes( 0 )
{
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this, 6, 2, KDialog::spacingHint(), KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "Download from:" ), this );

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

  mCalButton = new KPushButton( i18n("Calendar Folder..."), this );
  mainLayout->addWidget( mCalButton, 5, 0 );
  connect( mCalButton, SIGNAL( clicked() ), SLOT( selectCalendarFolder() ) );

  mTaskButton = new KPushButton( i18n("Task Folder..."), this );
  mainLayout->addWidget( mTaskButton, 5, 1 );
  connect( mTaskButton, SIGNAL( clicked() ), SLOT( selectTaskFolder() ) );

  mReloadConfig = new KCal::ResourceCachedReloadConfig( this );
  mainLayout->addMultiCellWidget( mReloadConfig, 6, 6, 0, 1 );

  mSaveConfig = new KCal::ResourceCachedSaveConfig( this );
  mainLayout->addMultiCellWidget( mSaveConfig, 7, 7, 0, 1 );
}

void KCalResourceSloxConfig::loadSettings( KRES::Resource *resource )
{
  KCalResourceSlox *res = static_cast<KCalResourceSlox *>( resource );
  mRes = res;
  if ( mRes->resType() == "slox" ) { // we don't have folder selection for SLOX
    mCalButton->setEnabled( false );
    mTaskButton->setEnabled( false );
  }
  if ( res ) {
    mDownloadUrl->setURL( res->prefs()->url() );
    mLastSyncCheck->setChecked( res->prefs()->useLastSync() );
    mUserEdit->setText( res->prefs()->user() );
    mPasswordEdit->setText( res->prefs()->password() );
    mCalendarFolderId = res->prefs()->calendarFolderId();
    mTaskFolderId = res->prefs()->taskFolderId();
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
    res->prefs()->setCalendarFolderId( mCalendarFolderId );
    res->prefs()->setTaskFolderId( mTaskFolderId );
    mReloadConfig->saveSettings( res );
    mSaveConfig->saveSettings( res );
  } else {
    kdError(5700) << "KCalResourceSloxConfig::saveSettings(): no KCalResourceSlox, cast failed" << endl;
  }
}

void KCalResourceSloxConfig::selectCalendarFolder()
{
  SloxFolderManager *manager = new SloxFolderManager( mRes, mDownloadUrl->url() );
  SloxFolderDialog *dialog = new SloxFolderDialog( manager, ::Calendar, this );
  dialog->setSelectedFolder( mCalendarFolderId );
  if ( dialog->exec() == QDialog::Accepted )
    mCalendarFolderId = dialog->selectedFolder();
}

void KCalResourceSloxConfig::selectTaskFolder( )
{
  SloxFolderManager *manager = new SloxFolderManager( mRes, mDownloadUrl->url() );
  SloxFolderDialog *dialog = new SloxFolderDialog( manager, Tasks, this );
  dialog->setSelectedFolder( mTaskFolderId );
  if ( dialog->exec() == QDialog::Accepted )
    mTaskFolderId = dialog->selectedFolder();
}

#include "kcalresourcesloxconfig.moc"
