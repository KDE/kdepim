/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Wed Aug  2 11:23:04 CEST 2000
    copyright            : (C) 2000 by Hans Dijkema
    email                : kmailcvt@hum.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <dcopclient.h>
#include <kapplication.h>

#include "kmailcvt.h"

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
  KLocale::setMainCatalogue("kmailcvt");

  KAboutData aboutData( "kmailcvt", I18N_NOOP("KMailCVT"),
    KMAILCVT_VERSION, KMAILCVT, KAboutData::License_GPL_V2,
    "(c) 2000-3, The KMailCVT developers");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
  KMailCVT *kmailcvt = new KMailCVT();
  a.setMainWidget(kmailcvt);
  kmailcvt->show();  

  DCOPClient *client=a.dcopClient();
  if (!client->attach()) {
    exit(1);
  }

  return a.exec();
}
