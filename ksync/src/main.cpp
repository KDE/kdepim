/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>

#include "ksync.h"


static const char description[] =
	I18N_NOOP("KSync");

static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("file to open"), 0 },
  KCmdLineLastOption
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "ksync", I18N_NOOP("KSync"),
                          "0.1", description, KAboutData::License_GPL,
                          "(c) 2001, Cornelius Schumacher", 0, 0, "schumacher@kde.org");
	aboutData.addAuthor("Cornelius Schumacher",0, "schumacher@kde.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

    KApplication app;
    KGlobal::locale()->insertCatalogue("libksync");

    if (app.isRestored())
    {
        RESTORE(KSync);
    }
    else
    {
        KSync *ksync = new KSync();
        ksync->show();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (args->count())
        {
            ksync->openDocumentFile(args->url(0));
        }
        else
        {
            ksync->openDocumentFile();
        }
        args->clear();
    }

    return app.exec();
}
