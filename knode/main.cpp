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


void convert(const char*);

static const KCmdLineOptions options[] =
{
	{ "convert <version>", I18N_NOOP("convert the database from <version>"), 0 },
	{ "c", 0, 0},
	{ 0, 0, 0}
};

int main(int argc, char* argv[])
{

	KAboutData aboutData("knode",
                        I18N_NOOP("KNode"),
                        KNODE_VERSION,
                        I18N_NOOP("A newsreader for KDE"),
                        KAboutData::License_GPL,
                        "Copyright (C) 1999, 2000, Christian Thurner",
                        0,
                        "http://knode.sourceforge.net/",
                        "cthurner@freepage.de");
    										
  aboutData.addAuthor("Christian Thurner",I18N_NOOP("Maintainer"),"cthurner@freepage.de");
  aboutData.addAuthor("Christian Gebauer",0,"gebauer@bigfoot.com");
  aboutData.addAuthor("Dirk Mueller",0,"mueller@kde.org");
  aboutData.addAuthor("Matthias Kalle Dalheimer",0,"kalle@kde.org");
  KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	QCString val = args->getOption("convert");
	if (val.length()) {
	 	cout << I18N_NOOP("Converting ...\n");
  	convert(val);
	}
	
	args->clear();
	
	KApplication app;
	KNodeApp* knode = 0;

	if (app.isRestored())
		RESTORE(KNodeApp)
  else {
	 	knode = new KNodeApp;
	 	QObject::connect(&app, SIGNAL(saveYourself()), knode, SLOT(slotSaveYourself()));
   	knode->show();
  }

  int ret=app.exec();

  return ret;
}
