/*
    main.cpp

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>

#include "ktnefmain.h"

static const char description[] =
	I18N_NOOP("Viewer for mail attachments using TNEF format");


static KCmdLineOptions options[] =
{
  { "+[file]", I18N_NOOP("An optional argument 'file'"), 0 },
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

  KAboutData aboutData( "ktnef", I18N_NOOP("KTnef"),
    "1.0", description, KAboutData::License_GPL,
    "(c) 2000, Michael Goffioul");
  aboutData.addAuthor("Michael Goffioul",0, "kdeprint@swing.be");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication::addCmdLineOptions();

  KApplication a;
  KTNEFMain *tnef = new KTNEFMain();
  a.setMainWidget(tnef);
  tnef->show();

  KCmdLineArgs	*args = KCmdLineArgs::parsedArgs();
  if (args->count() > 0)
    tnef->loadFile(args->arg(0));

  return a.exec();
}
