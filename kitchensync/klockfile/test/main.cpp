/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mit Nov 28 19:38:52 CET 2001
    copyright            : (C) 2001 by Holger "zecke" Freyther
    email                : freyther@kde.org
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
#include <kapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <qstring.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include "ksharedtest.h"

static const char *description =
	I18N_NOOP("Testapp");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


static KCmdLineOptions options[] =
{
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};
int main(int argc, char *argv[])
{

  KAboutData aboutData( "dcopclient", I18N_NOOP("Testapp"),
    "0.01", description, KAboutData::License_GPL,
    "(c) 2001, Holger  Freyther", 0, 0, "freyther@kde.org");
  aboutData.addAuthor("Holger  Freyther",0, "freyther@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
  KApplication a;
  Form1 *form = new Form1;
  a.setMainWidget(form );
  form->show();

  return a.exec();
}
