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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qclipboard.h>

#include <kaccel.h>
#include <kaction.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <kstdaction.h>

#include "actionmanager.h"
#include "kaddressbook.h"
#include "kaddressbookmain.h"
#include "kaddressbookmain.moc"

KAddressBookMain::KAddressBookMain()
  : KMainWindow( 0 ), DCOPObject( "KAddressBookIface" )
{
  setCaption( i18n( "Address Book Browser" ) );

  mWidget = new KAddressBook( this, "KAddressBook" );

  mActionManager = new ActionManager( this, mWidget, true, this );

  initActions();

  setCentralWidget( mWidget );

  statusBar()->show();

  mWidget->readConfig();

  createGUI( "kaddressbookui.rc", false );

  mActionManager->initActionViewList();

  setAutoSaveSettings();
}

KAddressBookMain::~KAddressBookMain()
{
  mWidget->writeConfig();
}

void KAddressBookMain::addEmail( QString addr )
{
  mWidget->addEmail( addr );
}

ASYNC KAddressBookMain::showContactEditor( QString uid )
{
  mWidget->showContactEditor( uid );
}

void KAddressBookMain::newContact()
{
  mWidget->newContact();
}

QString KAddressBookMain::getNameByPhone( QString phone )
{
  return mWidget->getNameByPhone( phone );
}

void KAddressBookMain::save()
{
  mWidget->save();
}

void KAddressBookMain::exit()
{
  close();
}

void KAddressBookMain::saveProperties( KConfig* )
{
  // the 'config' object points to the session managed
  // config file.  anything you write here will be available
  // later when this app is restored

  //what I want to save
  //windowsize
  //background image/underlining color/alternating color1,2
  //chosen fields
  //chosen fieldsWidths

  // e.g., config->writeEntry("key", var);
}

void KAddressBookMain::readProperties( KConfig* )
{
  // the 'config' object points to the session managed
  // config file.  this function is automatically called whenever
  // the app is being restored.  read in here whatever you wrote
  // in 'saveProperties'

  // e.g., var = config->readEntry("key");
}

void KAddressBookMain::initActions()
{
  KStdAction::quit( this, SLOT( close() ), actionCollection() );

  KStdAction::preferences( mWidget, SLOT( configure() ), actionCollection() );
  KStdAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
  KStdAction::keyBindings( this, SLOT( configureKeys() ), actionCollection() );
}

void KAddressBookMain::configureToolbars()
{
  saveMainWindowSettings( KGlobal::config(), "MainWindow" );

  KEditToolbar dlg( factory() );
  connect( &dlg, SIGNAL( newToolbarConfig() ), SLOT( slotNewToolbarConfig() ) );

  dlg.exec();
}

void KAddressBookMain::slotNewToolbarConfig()
{
  mActionManager->initActionViewList();

  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KAddressBookMain::configureKeys()
{
  KKeyDialog::configureKeys( actionCollection(), xmlFile(), true, this );
}

bool KAddressBookMain::queryClose()
{
  if ( mActionManager->isModified() ) {
    QString text = i18n( "The address book was modified. Do you want to save your changes?" );
    int ret = KMessageBox::warningYesNoCancel( this, text, "",
                                              KStdGuiItem::yes(),
                                              KStdGuiItem::no(), "AskForSave" );
    switch ( ret ) {
      case KMessageBox::Yes:
        mWidget->save();
        break;
      case KMessageBox::No:
        return true;
        break;
      default: //cancel
        return false;
        break;
    }
  }

  return true;
}
