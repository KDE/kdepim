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
#include "kabcore.h"

extern "C" {

void crashHandler( int )
{
  KABC::StdAddressBook::handleCrash();
  ::exit( 0 );
}

}

class KAddressBookApp : public KUniqueApplication {
  public:
    KAddressBookApp() : mMainWin( 0 ) {}
    ~KAddressBookApp() {}

    int newInstance();

  private:
    KAddressBookMain *mMainWin;
};

int KAddressBookApp::newInstance()
{
  if ( isRestored() ) {
    // There can only be one main window
    if ( KMainWindow::canBeRestored( 1 ) ) {
      mMainWin = new KAddressBookMain;
      mMainWin->show();
      mMainWin->restore( 1 );
    }
  } else {
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QCString addrStr = args->getOption( "addr" );
    QCString uidStr = args->getOption( "uid" );
    QString addr;
    QString uid;
    if ( !addrStr.isEmpty() )
      addr = QString::fromLocal8Bit( addrStr );
    if ( !uidStr.isEmpty() )
      uid = QString::fromLocal8Bit( uidStr );


    if ( args->isSet( "editor-only" ) ) {
      if ( !mMainWin )
        mMainWin = new KAddressBookMain;
        KStartupInfo::appStarted();
        mMainWin->hide();
    } else {
      if ( mMainWin ) {
        mMainWin->show();
        KWin::setActiveWindow( mMainWin->winId() );
      } else {
        mMainWin = new KAddressBookMain;
        mMainWin->show();
      }
    }
    // Can not see why anyone would pass both a uid and an email address, so I'll leave it that two contact editors will show if they do
    if ( !addr.isEmpty() )
      mMainWin->addEmail( addr );

    if ( !uid.isEmpty() )
      mMainWin->showContactEditor( uid );
    if ( args->isSet( "new-contact" ) ) {
      mMainWin->newContact();
    }
  }

  KCrash::setEmergencySaveFunction( crashHandler );

  return 0;
}

// the dummy argument is required, because KMail apparently sends an empty
// argument.
static KCmdLineOptions kmoptions[] =
{
  { "a", 0 , 0 },
  { "addr <email>", I18N_NOOP( "Shows contact editor with given email address" ), 0 },
  { "uid <uid>", I18N_NOOP( "Shows contact editor with given uid" ), 0 },
  { "editor-only", I18N_NOOP( "Launches in editor only mode" ), 0 },
  { "new-contact", I18N_NOOP( "Launches editor for the new contact" ), 0 },
  { "+[argument]", I18N_NOOP( "dummy argument" ), 0},
  KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
  KLocale::setMainCatalogue( "kaddressbook" );

  KCmdLineArgs::init( argc, argv, KABCore::createAboutData() );
  KCmdLineArgs::addCmdLineOptions( kmoptions );
  KUniqueApplication::addCmdLineOptions();

  if ( !KAddressBookApp::start() )
    exit( 0 );

  KAddressBookApp app;
  KGlobal::locale()->insertCatalogue( "libkdepim" );

  bool ret = app.exec();
  while (KMainWindow::memberList->first())
      delete KMainWindow::memberList->first();
  return ret;
}
