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

#include "kabcore.h"

#include "kaddressbookmain.h"

KAddressBookMain::KAddressBookMain()
  : DCOPObject( "KAddressBookIface" ), KMainWindow( 0 ) 
{
  setCaption( i18n( "Address Book Browser" ) );

  mCore = new KABCore( this, true, this );
  mCore->restoreSettings();

  initActions();

  setCentralWidget( mCore );

  statusBar()->show();

  setStandardToolBarMenuEnabled(true);
  
  createGUI( "kaddressbookui.rc", false );

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

void KAddressBookMain::saveProperties( KConfig* )
{
}

void KAddressBookMain::readProperties( KConfig* )
{
}

void KAddressBookMain::initActions()
{
  KStdAction::quit( this, SLOT( close() ), actionCollection() );
  KStdAction::configureToolbars( this, SLOT( configureToolbars() ), actionCollection() );
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
  applyMainWindowSettings( KGlobal::config(), "MainWindow" );
}

void KAddressBookMain::configureKeys()
{
  KKeyDialog::configureKeys( actionCollection(), xmlFile(), true, this );
}

bool KAddressBookMain::queryClose()
{
  if ( mCore->modified() ) {
    QString text = i18n( "The address book was modified. Do you want to save your changes?" );
    int ret = KMessageBox::warningYesNoCancel( this, text, "",
                                              KStdGuiItem::yes(),
                                              KStdGuiItem::no(), "AskForSave" );
    switch ( ret ) {
      case KMessageBox::Yes:
        mCore->save();
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

#include "kaddressbookmain.moc"
