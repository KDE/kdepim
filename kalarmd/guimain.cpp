/*
    This file is part of the KDE alarm daemon GUI.
    Copyright (c) 1997-1999 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$
//
// KOrganizer/KAlarm alarm daemon main program
//

#include <stdlib.h>
#include <kdebug.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "guiapp.h"

static const char* kalarmdGuiVersion = "0.9";
static const KCmdLineOptions options[] =
{
   {0L,0L,0L}
};

int main(int argc, char **argv)
{
  KAboutData aboutData("kalarmdgui", I18N_NOOP("Alarm Daemon GUI"),
      kalarmdGuiVersion, I18N_NOOP("KOrganizer/KAlarm Alarm Daemon GUI"), KAboutData::License_GPL,
      "(c) 1997-1999 Preston Brown\n(c) 2000-2001 Cornelius Schumacher\n(c) 2001 David Jarvie", 0L,
      "http://pim.kde.org");
  aboutData.addAuthor("Cornelius Schumacher",I18N_NOOP("Maintainer"),
                      "schumacher@kde.org");
  aboutData.addAuthor("David Jarvie",I18N_NOOP("KAlarm Author"),
                      "software@astrojar.org.uk");
  aboutData.addAuthor("Preston Brown",I18N_NOOP("Original Author"),
                      "pbrown@kde.org");

  KCmdLineArgs::init(argc,argv,&aboutData);
  KCmdLineArgs::addCmdLineOptions(options);
  KUniqueApplication::addCmdLineOptions();

  if (!AlarmGuiApp::start())
    exit(0);

  AlarmGuiApp app;

  return app.exec();
}
