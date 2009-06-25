/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kglobal.h>
#include <klocale.h>

#include "knapplication.h"
#include "resource.h"
#include "knode.h"
#include "aboutdata.h"
#include "knode_options.h"
#include "utils/startup.h"

using KNode::AboutData;

int main(int argc, char* argv[])
{
  AboutData aboutData;

  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( knode_options() );
  KUniqueApplication::addCmdLineOptions();

  if (!KNApplication::start())
    return 0;

  KNApplication app;

  KNode::Utilities::Startup::loadLibrariesIconsAndTranslations();

  return app.exec();
}
