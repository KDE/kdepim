/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qlayout.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstdaction.h>
#include <kxmlguiclient.h>

#include "engine.h"
#include "konnectorpairview.h"
#include "konnectorpairmanager.h"
#include "logdialog.h"
#include "paireditordialog.h"

#include "mainwidget.h"

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *widget, const char *name )
  : QWidget( widget, name ), mGUIClient( guiClient )
{
  mManager = new KonnectorPairManager( this );
  mManager->load();

  mEngine = new KSync::Engine();

  mLogDialog = new LogDialog( this );
  mLogDialog->hide();

  initGUI();

  connect( mView, SIGNAL( konnectorPairSelected( bool ) ),
           this, SLOT( konnectorPairSelected( bool ) ) );

  mView->refresh();
}

MainWidget::~MainWidget()
{
  mManager->save();

  delete mManager;
  mManager = 0;

  delete mEngine;
  mEngine = 0;
}

KActionCollection *MainWidget::actionCollection() const
{
  return mGUIClient->actionCollection();
}

KAboutData *MainWidget::aboutData()
{
  KAboutData *about = new KAboutData( "multisynk", I18N_NOOP( "MultiSynK" ),
                                      "0.1", I18N_NOOP( "The KDE Syncing Application" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 2004, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );

  return about;
}

void MainWidget::addPair()
{
  PairEditorDialog dlg( this );

  KonnectorPair *pair = new KonnectorPair;
  dlg.setPair( pair );

  if ( dlg.exec() )
    mManager->add( dlg.pair() );
  else
    delete pair;
}

void MainWidget::editPair()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );
  if ( !pair )
    return;

  PairEditorDialog dlg( this );
  dlg.setPair( pair );

  if ( dlg.exec() )
    mManager->change( dlg.pair() );
}

void MainWidget::deletePair()
{
  QString uid = mView->selectedPair();

  if ( !uid.isEmpty() ) {
    KonnectorPair *pair = mManager->pair( uid );
    int result = KMessageBox::questionYesNo( this, i18n( "Do you really want to delete '%1'?" ).arg( pair->name() ),
                                             i18n( "Delete Synchronization Pair" ) );
    if ( result == KMessageBox::Yes )
      mManager->remove( uid );
  }
}

void MainWidget::showLog()
{
  mLogDialog->show();
  mLogDialog->raise();
}

void MainWidget::startSync()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );

  connect( pair->manager(), SIGNAL( synceesRead( KSync::Konnector* ) ),
           mEngine, SLOT( slotSynceesRead( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeReadError( KSync::Konnector* ) ),
           mEngine, SLOT( slotSynceeReadError( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceesWritten( KSync::Konnector* ) ),
           mEngine, SLOT( slotSynceesWritten( KSync::Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeWriteError( KSync::Konnector* ) ),
           mEngine, SLOT( slotSynceeWriteError( KSync::Konnector* ) ) );
  connect( mEngine, SIGNAL( doneSync() ),
           this, SLOT( syncDone() ) );

  mEngine->go( pair );
}

void MainWidget::syncDone()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );

  disconnect( pair->manager(), SIGNAL( synceesRead( KSync::Konnector* ) ),
              mEngine, SLOT( slotSynceesRead( KSync::Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceeReadError( KSync::Konnector* ) ),
              mEngine, SLOT( slotSynceeReadError( KSync::Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceesWritten( KSync::Konnector* ) ),
              mEngine, SLOT( slotSynceesWritten( KSync::Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceeWriteError( KSync::Konnector* ) ),
              mEngine, SLOT( slotSynceeWriteError( KSync::Konnector* ) ) );
  disconnect( mEngine, SIGNAL( doneSync() ),
              this, SLOT( syncDone() ) );
}

void MainWidget::konnectorPairSelected( bool state )
{
  mEditAction->setEnabled( state );
  mDeleteAction->setEnabled( state );
  mSyncAction->setEnabled( state );
}

void MainWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  mView = new KonnectorPairView( mManager, this );
  layout->addWidget( mView );

  new KAction( i18n( "New..." ), "filenew", 0, this, SLOT( addPair() ),
               actionCollection(), "new" );
  mEditAction = new KAction( i18n( "Edit..." ), "edit", 0, this,
                             SLOT( editPair() ), actionCollection(), "edit" );
  mEditAction->setEnabled( false );

  mDeleteAction = new KAction( i18n( "Delete..." ), "editdelete", 0, this,
                               SLOT( deletePair() ), actionCollection(), "delete" );
  mDeleteAction->setEnabled( false );

  new KAction( i18n( "Log..." ), "filefind", 0, this, SLOT( showLog() ),
               actionCollection(), "log" );

  mSyncAction = new KAction( i18n( "Sync..." ), "hotsync", 0, this,
                             SLOT( startSync() ), actionCollection(), "sync" );
  mSyncAction->setEnabled( false );
}

#include "mainwidget.moc"
