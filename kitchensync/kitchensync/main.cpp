/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
    Copyright (c) 2002 Maximilian Reiﬂ <harlekin@handhelds.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kuniqueapplication.h>
#include <klocale.h>
#include <stdlib.h>

#include <mainwindow.h>

#include "splash.h"
#include "aboutdata.h"

static KCmdLineOptions options[] =
{
  KCmdLineLastOption
};

int main( int argc, char *argv[] )
{
  KSync::AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ) {
    kdDebug(5210) << "KitchenSync already runs." << endl;
    exit( 0 );
  };

  KUniqueApplication a;
  
  /*
   * Install translation
   */
  KGlobal::locale()->insertCatalogue( "libkcal" );
  KGlobal::locale()->insertCatalogue( "libkitchensync" );

  
  // time for a Widget
  KSync::Splash *splash = new KSync::Splash;
  KSync::MainWindow *mainwindow = new KSync::MainWindow;
  delete splash;
  mainwindow->show();
  kdDebug(5210) << "exec now " << endl;
  a.exec();
}
