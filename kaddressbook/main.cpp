/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

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

#include <stdlib.h>
#include <unistd.h>

#include <qstring.h>

#include <kabc/stdaddressbook.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstartupinfo.h>
#include <kuniqueapplication.h>
#include <kwin.h>

#include "kaddressbookmain.h"
#include "kaddressbook_options.h"
#include "kabcore.h"

class KAddressBookApp : public KUniqueApplication {
  public:
    KAddressBookApp() : mMainWin( 0 ), mDefaultIsOpen( false ) {}
    ~KAddressBookApp() {}

    int newInstance();

  private:
    KAddressBookMain *mMainWin;
    bool mDefaultIsOpen;
};

int KAddressBookApp::newInstance()
{
  if ( isRestored() ) {
    // There can only be one main window
    if ( KMainWindow::canBeRestored( 1 ) ) {
      mMainWin = new KAddressBookMain;
      setMainWidget( mMainWin );
      mMainWin->show();
      mMainWin->restore( 1 );
    }
  } else {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->isSet( "editor-only" ) ) {
        if ( !mMainWin ) {
          mMainWin = new KAddressBookMain;
          setMainWidget( mMainWin );
          mMainWin->hide();
        }
        // otherwise, leave the window like it is (hidden or shown)
        KStartupInfo::appStarted();
    } else {
      QString file;
      if ( args->isSet( "document" ) ) {
         file = args->getOption( "document" );
      }
      if ( !( file.isEmpty() && mDefaultIsOpen ) ) {
        if ( !mMainWin ) {
          mMainWin = new KAddressBookMain( file );
          setMainWidget( mMainWin );
          mMainWin->show();
        } else {
          KAddressBookMain *m = new KAddressBookMain( file );
          m->show();
        }
        if ( file.isEmpty() ) mDefaultIsOpen = true;
      }
    }

    mMainWin->handleCommandLine();
  }

  // Handle startup notification and window activation
  // We do it ourselves instead of calling KUniqueApplication::newInstance
  // to avoid the show() call there.
#if defined Q_WS_X11 && ! defined K_WS_QTONLY
  KStartupInfo::setNewStartupId( mMainWin, kapp->startupId() );
#endif

  return 0;
}

int main( int argc, char *argv[] )
{
  KLocale::setMainCatalogue( "kaddressbook" );

  KCmdLineArgs::init( argc, argv, KABCore::createAboutData() );
  KCmdLineArgs::addCmdLineOptions( kaddressbook_options );
  KUniqueApplication::addCmdLineOptions();

  if ( !KAddressBookApp::start() )
    return 0;

  KAddressBookApp app;
  KGlobal::locale()->insertCatalogue( "libkdepim" );

  bool ret = app.exec();
  while (KMainWindow::memberList->first())
      delete KMainWindow::memberList->first();
  return ret;
}
