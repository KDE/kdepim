/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "kmobiletools.h"
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
//#include <dcopclient.h>
#include <libkmobiletools/crashhandler.h>
#include <libkmobiletools/aboutdata.h>

//Added by qt3to4:
#include <Q3CString>

// static const char description[] =
//     I18N_NOOP("A KDE KPart Application");

// static const char version[] = "0.5.0";

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP( "Document to open" ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    AboutData *aboutdata=new AboutData();
    KCmdLineArgs::init(argc, argv, aboutdata );
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(kmobiletools);
    }
    else
    {
#if 0 // FIXME port to D-Bus
        Q3CString dcopclient=app.dcopClient()->registerAs("kmobiletools",false) ;
        kDebug() << "DCOPClient registration: " << dcopclient << endl;
        if(dcopclient!="kmobiletools")
        {
            kDebug() << "Previous instance detected, exiting\n";
            kDebug() << app.dcopClient()->send( "kmobiletools", "kmobiletools", "show()", QString() ) << endl;
            return 0;
        }
#endif
        // no session.. just start up normally
    kDebug() << "Starting '" << aboutdata->programName() << "` version '" << aboutdata->version() << "`" << endl;
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
#ifdef USE_CRASHHANDLER
        KCrash::setCrashHandler( KMobileTools::Crash::crashHandler );
#endif

        if ( args->count() == 0 )
        {
        kmobiletools *widget = new kmobiletools;
        widget->show();
        }
        else
        {
            int i = 0;
            for (; i < args->count(); i++ )
            {
                kmobiletools *widget = new kmobiletools;
                widget->show();
//                 widget->load( args->url( i ) );
            }
        }
        args->clear();
    }

    return app.exec();
}
