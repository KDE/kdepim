/*
    main.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2000 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include <kapp.h>
#include "knapplication.h"
#include "resource.h"
#include "knode.h"


static KCmdLineOptions knoptions[] =
{
  { "+[url]", I18N_NOOP("A 'news://server/group' URL."), 0 },
  { 0, 0, 0 }
};


int main(int argc, char* argv[])
{
  KAboutData aboutData("knode",
                        I18N_NOOP("KNode"),
                        KNODE_VERSION,
                        I18N_NOOP("A newsreader for KDE"),
                        KAboutData::License_GPL,
                        "Copyright (c) 1999-2000 the KNode authors",
                        0,
                        "http://knode.sourceforge.net/");
                        
  aboutData.addAuthor("Christian Thurner",I18N_NOOP("Maintainer"),"cthurner@freepage.de");
  aboutData.addAuthor("Christian Gebauer",0,"gebauer@bigfoot.com");
  aboutData.addAuthor("Dirk Mueller",0,"mueller@kde.org");
  aboutData.addAuthor("Mathias Waack",0,"mathias@atoll-net.de");
  aboutData.addCredit("Stephan Johach",0,"lucardus@onlinehome.de");
  aboutData.addCredit("Matthias Kalle Dalheimer",0,"kalle@kde.org");

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( knoptions );
  KUniqueApplication::addCmdLineOptions();

  if (!KNApplication::start())
    return 0;

  KNApplication app;
  return app.exec();
}

