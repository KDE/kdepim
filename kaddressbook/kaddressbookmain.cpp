/*
    This file is part of KAddressbook.
    Copyright (c) 1999 Don Sanders <dsanders@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kedittoolbar.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>

#include <libkdepim/statusbarprogresswidget.h>
#include <libkdepim/progressdialog.h>

#include "kabcore.h"

#include "kaddressbookmain.h"

KAddressBookMain::KAddressBookMain( const QString &file )
  : DCOPObject( "KAddressBookIface" ), KMainWindow( 0 )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  setCaption( i18n( "Address Book Browser" ) );

  mCore = new KABCore( this, true, this, file );
  mCore->restoreSettings();

  initActions();

  setCentralWidget( mCore->widget() );

  statusBar()->show();
  statusBar()->insertItem( "", 1 );

  KPIM::ProgressDialog *progressDialog = new KPIM::ProgressDialog( statusBar(),
    this );
  progressDialog->hide();

  KPIM::StatusbarProgressWidget *progressWidget;
  progressWidget = new KPIM::StatusbarProgressWidget( progressDialog,
    statusBar() );
  progressWidget->show();

  statusBar()->addWidget( progressWidget, 0, true );

  mCore->setStatusBar( statusBar() );

  setStandardToolBarMenuEnabled( true );

  createGUI( "kaddressbookui.rc", false );

  resize( 400, 300 ); // initial size
  setAutoSaveSettings();
}

KAddressBookMain::~KAddressBookMain()
{
  mCore->saveSettings();
}

void KAddressBookMain::addEmail( QString addr )
{
  mCore->addEmail( addr );
}

void KAddressBookMain::importVCard( const QString& file )
{
  mCore->importVCard( KURL( file ) );
}

ASYNC KAddressBookMain::showContactEditor( QString uid )
{
  mCore->editContact( uid );
}

void KAddressBookMain::newContact()
{
  mCore->newContact();
}

QString KAddressBookMain::getNameByPhone( QString phone )
{
  return mCore->getNameByPhone( phone );
}

void KAddressBookMain::save()
{
  mCore->save();
}

void KAddressBookMain::exit()
{
  close();
}

bool KAddressBookMain::handleCommandLine()
{
  return mCore->handleCommandLine( this );
}

void KAddressBookMain::saveProperties( KConfig* )
{
}

void KAddressBookMain::readProperties( KConfig* )
{
}

void KAddressBookMain::initActions()
{
  KStdAction::quit( this, SLOT( close() ), actionCollection() );

  KAction *action;
  action = KStdAction::keyBindings( this, SLOT( configureKeyBindings() ), actionCollection() );
  action->setWhatsThis( i18n( "You will be presented with a dialog, where you can configure the application wide shortcuts." ) );

  KStdAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
}

void KAddressBookMain::configureKeyBindings()
{
  KKeyDialog::configure( actionCollection(), this );
}

void KAddressBookMain::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );

  KEditToolbar edit( factory() );
  connect( &edit, SIGNAL( newToolbarConfig() ),
           this, SLOT( slotNewToolbarConfig() ) );

  edit.exec();
}

void KAddressBookMain::newToolbarConfig()
{
  createGUI();
  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

#include "kaddressbookmain.moc"
