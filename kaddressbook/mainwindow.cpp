/*
  This file is part of KAddressBook.

  Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"
#include "mainwidget.h"
#include "xxportmanager.h"

#include <KGlobal>
#include <KConfigGroup>

#include <QAction>
#include <KActionCollection>
#include <KEditToolBar>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KLocalizedString>
#include <KToolBar>

MainWindow::MainWindow()
  : KXmlGuiWindow( 0 )
{
  mMainWidget = new MainWidget( this, this );

  setCentralWidget( mMainWidget );

  initActions();

  setStandardToolBarMenuEnabled( true );

  toolBar()->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  setupGUI( Save | Create, QLatin1String("kaddressbookui.rc") );

  setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
}

MainWidget *MainWindow::mainWidget() const
{
    return mMainWidget;
}

void MainWindow::initActions()
{
  KStandardAction::quit( this, SLOT(close()), actionCollection() );

  QAction *action =
    KStandardAction::keyBindings( this, SLOT(configureKeyBindings()), actionCollection() );
  action->setWhatsThis(
    i18nc( "@info:whatsthis",
           "You will be presented with a dialog where you can configure "
           "the application-wide shortcuts." ) );
  KStandardAction::configureToolbars( this, SLOT(configureToolbars()), actionCollection() );
  KStandardAction::preferences( this, SLOT(configure()), actionCollection() );
}

void MainWindow::configure()
{
  mMainWidget->configure();
}

void MainWindow::configureKeyBindings()
{
  if (KShortcutsDialog::configure( actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this )) {
      mMainWidget->updateQuickSearchText();
  }
}

void MainWindow::configureToolbars()
{
  KConfigGroup grp = KGlobal::config()->group( "MainWindow");
  saveMainWindowSettings( grp );

  KEditToolBar dlg( factory() );
  connect( &dlg, SIGNAL(newToolBarConfig()), this, SLOT(newToolbarConfig()) );
  dlg.exec();
}

void MainWindow::newToolbarConfig()
{
  createGUI( QLatin1String("kaddressbookui.rc") );

  applyMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );
}

