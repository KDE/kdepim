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

#include "kmailcvt.h"

int main(int argc, char *argv[])
{
  KLocale::setMainCatalog("kmailcvt");

  KAboutData aboutData( "kmailcvt", 0, ki18n("KMailCVT"),
    "3", ki18n("KMail Import Filters"), KAboutData::License_GPL_V2,
    ki18n("(c) 2000-2009, The KMailCVT developers"));
  aboutData.addAuthor(ki18n("Hans Dijkema"),ki18n("Original author"), "kmailcvt@hum.org");
  aboutData.addAuthor(ki18n("Danny Kukawka"), ki18n("Maintainer & New filters"), "danny.kukawka@web.de");
  aboutData.addAuthor(ki18n("Laurence Anderson"), ki18n("New GUI & cleanups"), "l.d.anderson@warwick.ac.uk");
  aboutData.addCredit(ki18n("Daniel Molkentin"), ki18n("New GUI & cleanups"), "molkentin@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );

  KCmdLineOptions options;
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  KApplication a;
  KMailCVT *kmailcvt = new KMailCVT();
  a.setMainWidget(kmailcvt);
  kmailcvt->show();

  return a.exec();
}
