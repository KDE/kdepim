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

#include "aboutdata.h"
#include "mainwindow.h"

#include <akonadi/control.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kuniqueapplication.h>

int main( int argc, char **argv )
{
  AboutData about;

  KCmdLineArgs::init( argc, argv, &about );

  KCmdLineOptions options;
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  if ( !KUniqueApplication::start() )
    exit( 0 );

  KUniqueApplication app;
  KGlobal::locale()->insertCatalog( "libkdepim" );
  KGlobal::locale()->insertCatalog( "kabc" );
  KGlobal::locale()->insertCatalog( "libakonadi" );
  KGlobal::locale()->insertCatalog( "kabcakonadi" );

  MainWindow *window = new MainWindow;
  window->show();

  if ( !Akonadi::Control::start( window ) ) {
    //TODO: add message box after string freeze
    kWarning() << "Unable to start Akonadi server, exit application";
    return 1;
  }

  return app.exec();
}
