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

#include <kaction.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <libkdepim/progressdialog.h>
#include <libkdepim/statusbarprogresswidget.h>

#include "engine.h"
#include "konnectorpairview.h"
#include "konnectorpairmanager.h"
#include "logdialog.h"
#include "paireditordialog.h"

#include "mainwindow.h"

MainWindow::MainWindow( QWidget *widget, const char *name )
  : KMainWindow( widget, name )
{
  mManager = new KonnectorPairManager( this );
  mManager->load();

  mEngine = new KSync::Engine();

  mLogDialog = new LogDialog( this );
  mLogDialog->hide();

  initGUI();

  mView->refresh();
}

MainWindow::~MainWindow()
{
  mManager->save();

  delete mManager;
  mManager = 0;

  delete mEngine;
  mEngine = 0;
}

void MainWindow::addPair()
{
  PairEditorDialog dlg( this );

  KonnectorPair *pair = new KonnectorPair;
  dlg.setPair( pair );

  if ( dlg.exec() )
    mManager->add( dlg.pair() );
  else
    delete pair;
}

void MainWindow::editPair()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );
  if ( !pair ) {
    qDebug( "oops we lost a pair" );
    return;
  }

  PairEditorDialog dlg( this );
  dlg.setPair( pair );

  if ( dlg.exec() )
    mManager->change( dlg.pair() );
}

void MainWindow::deletePair()
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

void MainWindow::showLog()
{
  mLogDialog->show();
  mLogDialog->raise();
}

void MainWindow::startSync()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );

  connect( pair->manager(), SIGNAL( synceesRead( Konnector* ) ),
           mEngine, SLOT( slotSynceesRead( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeReadError( Konnector* ) ),
           mEngine, SLOT( slotSynceeReadError( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceesWritten( Konnector* ) ),
           mEngine, SLOT( slotSynceesWritten( Konnector* ) ) );
  connect( pair->manager(), SIGNAL( synceeWriteError( Konnector* ) ),
           mEngine, SLOT( slotSynceeWriteError( Konnector* ) ) );
  connect( mEngine, SIGNAL( doneSync() ),
           this, SLOT( syncDone() ) );

  mEngine->go( pair );
}

void MainWindow::syncDone()
{
  QString uid = mView->selectedPair();

  if ( uid.isEmpty() )
    return;

  KonnectorPair *pair = mManager->pair( uid );
  if ( pair == 0 )
    qDebug( "No pair available" );

  disconnect( pair->manager(), SIGNAL( synceesRead( Konnector* ) ),
              mEngine, SLOT( slotSynceesRead( Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceeReadError( Konnector* ) ),
              mEngine, SLOT( slotSynceeReadError( Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceesWritten( Konnector* ) ),
              mEngine, SLOT( slotSynceesWritten( Konnector* ) ) );
  disconnect( pair->manager(), SIGNAL( synceeWriteError( Konnector* ) ),
              mEngine, SLOT( slotSynceeWriteError( Konnector* ) ) );
  disconnect( mEngine, SIGNAL( doneSync() ),
              this, SLOT( syncDone() ) );
}

void MainWindow::initGUI()
{
  mView = new KonnectorPairView( mManager, this );

  setCentralWidget( mView );

  KStdAction::quit( this, SLOT( close() ), actionCollection() );

  new KAction( i18n( "New..." ), "filenew", 0, this, SLOT( addPair() ),
               actionCollection(), "new" );
  new KAction( i18n( "Edit..." ), "edit", 0, this, SLOT( editPair() ),
               actionCollection(), "edit" );
  new KAction( i18n( "Delete..." ), "editdelete", 0, this, SLOT( deletePair() ),
               actionCollection(), "delete" );

  new KAction( i18n( "Log..." ), "filefind", 0, this, SLOT( showLog() ),
               actionCollection(), "log" );

  new KAction( i18n( "Sync..." ), "hotsync", 0, this, SLOT( startSync() ),
               actionCollection(), "sync" );

  setXMLFile( "multisynkui.rc" );
  createGUI( 0 );

  setAutoSaveSettings();
}

#include "mainwindow.moc"
