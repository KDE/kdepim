/***************************************************************************
                          main.cpp  -  description
 copyright            : (C) 1999 by Christian Thurner
 email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <iostream>

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "knode.h"
#include "resource.h"


int main(int argc, char* argv[])
{

  KAboutData aboutData("knode",
                        I18N_NOOP("KNode"),
                        KNODE_VERSION,
                        I18N_NOOP("A newsreader for KDE"),
                        KAboutData::License_GPL,
                        "Copyright (C) 1999-2000, Christian Thurner",
                        0,
                        "http://knode.sourceforge.net/");
                        
  aboutData.addAuthor("Christian Thurner",I18N_NOOP("Maintainer"),"cthurner@freepage.de");
  aboutData.addAuthor("Christian Gebauer",0,"gebauer@bigfoot.com");
  aboutData.addAuthor("Dirk Mueller",0,"mueller@kde.org");
  aboutData.addAuthor("Matthias Kalle Dalheimer",0,"kalle@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication::addCmdLineOptions();

  KApplication app;

  if (app.isRestored()) {
    int n = 1;
    while (KMainWindow::canBeRestored(n)){
      if (KMainWindow::classNameOfToplevel(n)=="KNodeApp")
        (new KNodeApp)->restore(n);
      n++;
    }
  } else {
    KNodeApp* knode = new KNodeApp;
    knode->show();
  }

  int ret=app.exec();

  return ret;
}
