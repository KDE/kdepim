/*
    This file is part of KitchenSync.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <stdlib.h>

#include "mainwidget.h"
#include "mainwindow.h"

static KCmdLineOptions options[] =
{
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData *about = MainWidget::aboutData();

  KCmdLineArgs::init( argc, argv, about );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ) {
    kdDebug() << "kitchensync already runs." << endl;
    exit( 0 );
  };

  KUniqueApplication app;
  
  KGlobal::locale()->insertCatalogue( "libkcal" );

  MainWindow *mainWindow = new MainWindow;
  mainWindow->show();

  app.exec();
}
