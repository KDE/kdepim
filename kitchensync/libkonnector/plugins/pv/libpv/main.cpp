/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Oct 02 2002
    copyright            : (C) 2002 by Maurus Erni
    email                : erni@pocketviewer.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>

#include <iostream>

#include "pvdaemon.h"

#include <kapplication.h>

int main( int argc, char **argv )
{
  KAboutData aboutData( "pvDaemon", I18N_NOOP("pvDaemon"),
      I18N_NOOP( "0.1" ), I18N_NOOP( "pvDaemon" ), KAboutData::License_GPL,
      I18N_NOOP("(c) 2002 Maurus Erni") );

  aboutData.addAuthor("Maurus Erni", I18N_NOOP("Developer"), "erni@pocketviewer.de");

  KCmdLineArgs::init( argc, argv, &aboutData );

  // Check if unique application is already running...
  if ( !KUniqueApplication::start() )
  {
    cerr << "pvDaemon is already running, exiting..." << endl;
    return 1;
  }
  KUniqueApplication app(false, false);

  CasioPV::pvDaemon* pvd = new CasioPV::pvDaemon();

  return app.exec();
}
