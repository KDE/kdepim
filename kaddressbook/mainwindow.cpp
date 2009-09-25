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

#include <kaction.h>
#include <kactioncollection.h>
#include <kcmultidialog.h>
#include <kedittoolbar.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <ktoolbar.h>

#include "mainwidget.h"

MainWindow::MainWindow()
  : KXmlGuiWindow( 0 )
{
  mMainWidget = new MainWidget( this, this );

  setCentralWidget( mMainWidget );

  initActions();

  setStandardToolBarMenuEnabled( true );

  setupGUI( Keys | Save | Create, "kaddressbookui.rc" );

  toolBar()->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );

  setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initActions()
{
  KStandardAction::quit( this, SLOT( close() ), actionCollection() );

  KAction *action = KStandardAction::keyBindings( this, SLOT( configureKeyBindings() ), actionCollection() );
  action->setWhatsThis( i18n( "You will be presented with a dialog, where you can configure the application wide shortcuts." ) );

  KStandardAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
  KStandardAction::preferences( this, SLOT( configure() ), actionCollection() );
}

void MainWindow::configure()
{
  KCMultiDialog dlg( this );
  dlg.addModule( "akonadicontact_actions.desktop" );
  dlg.addModule( "kcmldap.desktop" );

  dlg.exec();
}

void MainWindow::configureKeyBindings()
{
  KShortcutsDialog::configure( actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this );
}

void MainWindow::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );

  KEditToolBar dlg( factory() );
  connect( &dlg, SIGNAL( newToolbarConfig() ), this, SLOT( newToolbarConfig() ) );
  dlg.exec();
}

void MainWindow::newToolbarConfig()
{
  createGUI( "kaddressbookui.rc" );

  applyMainWindowSettings( KGlobal::config()->group( "MainWindow" ) );
}

#include "mainwindow.moc"
