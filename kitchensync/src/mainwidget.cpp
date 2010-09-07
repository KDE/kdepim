/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2005 Cornelius Schumacher <schumacher@kde.org>

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

#include "mainwidget.h"

#include "groupconfigdialog.h"
#include "groupview.h"
#include "syncprocess.h"
#include "syncprocessmanager.h"

#include <libqopensync/result.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kdebug.h>
#include <kinputdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kxmlguiclient.h>

#include <tqlayout.h>

MainWidget::MainWidget( KXMLGUIClient *guiClient, TQWidget *widget, const char *name )
  : TQWidget( widget, name ), mGUIClient( guiClient )
{
  initGUI();
  initActions();

  mGroupView->updateView();

  connect( SyncProcessManager::self(), TQT_SIGNAL( changed() ),
           mGroupView, TQT_SLOT( updateView() ) );
  connect( SyncProcessManager::self(), TQT_SIGNAL( syncProcessChanged( SyncProcess* ) ),
           mGroupView, TQT_SLOT( updateSyncProcess( SyncProcess* ) ) );

  enableActions();
}

MainWidget::~MainWidget()
{
}

KXMLGUIClient *MainWidget::guiClient() const
{
  return mGUIClient;
}

KAboutData *MainWidget::aboutData()
{
  KAboutData *about = new KAboutData( "kitchensync", I18N_NOOP( "KitchenSync" ),
                                      "0.1", I18N_NOOP( "The KDE Syncing Application" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 2005, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  return about;
}

void MainWidget::initGUI()
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( this );

  mGroupView = new GroupView( this );
  topLayout->addWidget( mGroupView );

  connect( mGroupView, TQT_SIGNAL( addGroup() ), TQT_SLOT( addGroup() ) );
  connect( mGroupView, TQT_SIGNAL( synchronizeGroup( SyncProcess* ) ),
           TQT_SLOT( sync( SyncProcess* ) ) );
  connect( mGroupView, TQT_SIGNAL( abortSynchronizeGroup( SyncProcess* ) ),
           TQT_SLOT( abortSync( SyncProcess* ) ) );
  connect( mGroupView, TQT_SIGNAL( configureGroup( SyncProcess* ) ),
           TQT_SLOT( editGroup( SyncProcess* ) ) );
}

void MainWidget::initActions()
{
  mActionSynchronize = new KAction( i18n("Synchronize"), "hotsync", 0, this, TQT_SLOT( sync() ),
                                    mGUIClient->actionCollection(), "sync" );
  mActionAddGroup = new KAction( i18n("Add Group..."), "filenew", 0, this, TQT_SLOT( addGroup() ),
                                 mGUIClient->actionCollection(), "add_group" );
  mActionDeleteGroup = new KAction( i18n("Delete Group..."), "editdelete", 0, this, TQT_SLOT( deleteGroup() ),
                                    mGUIClient->actionCollection(), "delete_group" );
  mActionEditGroup = new KAction( i18n("Edit Group..."), "edit", 0, this, TQT_SLOT( editGroup() ),
                                  mGUIClient->actionCollection(), "edit_group" );
}

void MainWidget::enableActions()
{
  bool state = ( SyncProcessManager::self()->count() > 0 );

  mActionSynchronize->setEnabled( state );
  mActionDeleteGroup->setEnabled( state );
  mActionEditGroup->setEnabled( state );
}

void MainWidget::addGroup()
{
  bool ok;
  TQString name = KInputDialog::getText( i18n("Create Synchronization Group"),
    i18n("Name for new synchronization group."), i18n( "Default" ), &ok, this );
  if ( ok ) {
    SyncProcess *process = SyncProcessManager::self()->byGroupName( name );
    if ( process ) {
      KMessageBox::error( this, i18n( "A group with the same name exists already.\nPlease choose another name." ),
                          i18n( "Duplicated Group Name" ) );
      return;
    }

    SyncProcessManager::self()->addGroup( name );
    enableActions();

    process = SyncProcessManager::self()->byGroupName( name );
    if ( process )
      editGroup( process );
  }
}

void MainWidget::deleteGroup()
{
  SyncProcess *syncProcess = mGroupView->selectedSyncProcess();
  if ( syncProcess ) {
    int result = KMessageBox::warningContinueCancel( this,
      i18n("Delete synchronization group '%1'?").arg( syncProcess->group().name() ) );
    if ( result == KMessageBox::Continue ) {
      SyncProcessManager::self()->remove( syncProcess );
      enableActions();
    }
  }
}

void MainWidget::editGroup()
{
  editGroup( mGroupView->selectedSyncProcess() );
}

void MainWidget::editGroup( SyncProcess *syncProcess )
{
  if ( syncProcess ) {
    GroupConfigDialog dlg( this, syncProcess );
    dlg.exec();

    enableActions();
  }
}

void MainWidget::sync()
{
  sync( mGroupView->selectedSyncProcess() );
}

void MainWidget::sync( SyncProcess *syncProcess )
{
  if ( syncProcess ) {
    syncProcess->reinitEngine();
    QSync::Result result = syncProcess->engine()->synchronize();
    if ( result ) {
      qDebug( "%s", result.message().latin1() );
    } else {
      qDebug( "synchronization worked" );
    }
  }
}

void MainWidget::abortSync( SyncProcess *syncProcess )
{
  if ( syncProcess )
    syncProcess->engine()->abort();
}

#include "mainwidget.moc"
