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

static const char *description =
	I18N_NOOP("A little tool to convert mail boxes and address books to KMail format");
	
	
static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{
  KLocale::setMainCatalogue("kmailcvt");

  KAboutData aboutData( "kmailcvt2", I18N_NOOP("Kmailcvt2"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2000, Hans Dijkema");
  aboutData.addAuthor("Hans Dijkema",0, "kmailcvt@hum.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
  Kmailcvt2 *kmailcvt2 = new Kmailcvt2();
  a.setMainWidget(kmailcvt2);
  kmailcvt2->show();  

  DCOPClient *client=a.dcopClient();
  if (!client->attach()) {
    exit(1);
  }

  return a.exec();
}
