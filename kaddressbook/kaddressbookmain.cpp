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

#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ksettings/dialog.h>
#include <ksettings/dispatcher.h>
#include <kstatusbar.h>

#include "kabcore.h"

#include "kaddressbookmain.h"

KAddressBookMain::KAddressBookMain()
  : DCOPObject( "KAddressBookIface" ), KMainWindow( 0 ), mConfigDialog( 0 )
{
  setCaption( i18n( "Address Book Browser" ) );

  mCore = new KABCore( this, true, this );
  mCore->restoreSettings();

  initActions();

  setCentralWidget( mCore->widget() );

  statusBar()->show();
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

  KAction *action;
  action = KStdAction::preferences( this, SLOT( configure() ), actionCollection() );
  action->setWhatsThis( i18n( "You will be presented with a dialog, that offers you all possibilities to configure KAddressBook." ) );
  action = KStdAction::keyBindings( this, SLOT( configureKeyBindings() ), actionCollection() );
  action->setWhatsThis( i18n( "You will be presented with a dialog, where you can configure the application wide shortcuts." ) );
}

void KAddressBookMain::configureKeyBindings()
{
  KKeyDialog::configure( actionCollection(), this );
}

void KAddressBookMain::configure()
{
  // Save the current config so we do not loose anything if the user accepts
  mCore->saveSettings();

  if ( !mConfigDialog ) {
    mConfigDialog = new KSettings::Dialog( this );

    KSettings::Dispatcher::self()->registerInstance( KGlobal::instance(), mCore,
                                                     SLOT( configurationChanged() ) );
  }

  mConfigDialog->show();
}


#include "kaddressbookmain.moc"
