/*
    main.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
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

#include "knapplication.h"
#include "resource.h"
#include "knode.h"
#include "aboutdata.h"
using KNode::AboutData;

static KCmdLineOptions knoptions[] =
{
  { "+[url]", I18N_NOOP("A 'news://server/group' URL."), 0 },
  KCmdLineLastOption
};


int main(int argc, char* argv[])
{
  AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( knoptions );
  KUniqueApplication::addCmdLineOptions();

  if (!KNApplication::start())
    return 0;

  KNApplication app;
  KGlobal::locale()->insertCatalogue("libkdenetwork");
  return app.exec();
}

