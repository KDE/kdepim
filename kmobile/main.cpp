/*  This file is part of the KDE KMobile library
    Copyright (C) 2003 Helge Deller <deller@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "kmobile.h"
#include <kuniqueapplication.h>
#include <dcopclient.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("KDE mobile devices manager");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    { "minimized", I18N_NOOP( "Minimize on startup to system tray" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kmobile", I18N_NOOP("KMobile"), version, description,
                     KAboutData::License_GPL, "(C) 2003-2005 Helge Deller", 0, 0, "deller@kde.org");
    about.addAuthor( "Helge Deller", 0, "deller@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication app;

    // register ourselves as a dcop client
    app.dcopClient()->registerAs(app.name(), false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(KMobile);
    }
    else
    {
        KMobile *widget = new KMobile;
	if (!args->isSet("minimized"))
		widget->show();
    }

    return app.exec();
}
