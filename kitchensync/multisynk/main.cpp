/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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
#include <stdlib.h>

#include "mainwindow.h"

static KCmdLineOptions options[] =
{
  KCmdLineLastOption
};

int main( int argc, char **argv )
{
  KAboutData about( "multisynk", I18N_NOOP( "MultiSynK" ),
                    "0.1", I18N_NOOP( "The KDE Syncing Application" ),
                    KAboutData::License_GPL_V2,
                    I18N_NOOP( "(c) 2004, The KDE PIM Team" ) );
  about.addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );

  KCmdLineArgs::init( argc, argv, &about );
  KCmdLineArgs::addCmdLineOptions( options );
  KUniqueApplication::addCmdLineOptions();

  KUniqueApplication::addCmdLineOptions();

  if( !KUniqueApplication::start() ) {
    kdDebug() << "multisynk already runs." << endl;
    exit( 0 );
  };

  KUniqueApplication app;

  MainWindow *mainWindow = new MainWindow;
  mainWindow->show();

  app.exec();
}
