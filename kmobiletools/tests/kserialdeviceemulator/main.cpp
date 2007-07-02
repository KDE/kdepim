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


#include "kserialdeviceemulator.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("A KDE KPart Application");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
    KAboutData about("kserialdeviceemulator", 0, ki18n("KSerialDeviceEmulator"), version, ki18n(description),
                     KAboutData::License_GPL, ki18n("(C) 2006 Marco Gulino"), KLocalizedString(), 0, "marco@kmobiletools.org");
    about.addAuthor( ki18n("Marco Gulino"), KLocalizedString(), "marco@kmobiletools.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    KSerialDeviceEmulator *mainWin = 0;

    if (app.isSessionRestored())
    {
        RESTORE(KSerialDeviceEmulator);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        /// @todo do something with the command line args here

        mainWin = new KSerialDeviceEmulator();
        app.setMainWidget( mainWin );
        mainWin->show();

        args->clear();
    }

    // mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
}

