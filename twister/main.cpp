/*
    Twister - PIM app for KDE
    
    Copyright 2000
        Rik Hemsley <rik@kde.org>
    
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// KDE includes
#include <kuniqueapp.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kcmdlineargs.h>

// Local includes
#include "Twister.h"
#include "TwisterUI.h"

static const char* description=I18N_NOOP("Twister, the KDE PIM client");
static const char* VERSION="0.0.1";

    int
main(int argc, char ** argv)
{
    KAboutData aboutData(
        "twister",
        I18N_NOOP("Twister"),
        VERSION,
        description,
        KAboutData::License_GPL,
        "(c) 2000, The KDE PIM Team",
        0,
        "http://without.netpedia.net",
        "kde-pim@kde.org"
    );

    aboutData.addAuthor(
        "Rik Hemsley",
        I18N_NOOP("Design and coding"),
        "rik@kde.org",
        "http://without.netpedia.net"
    );
 
    KCmdLineArgs::init(argc, argv, &aboutData);
  
#ifdef DOSOMEDOSOMEDOSOME

    if (!KUniqueApplication::start())
        exit(1);
    
    KUniqueApplication app;

#else

    KApplication app;

#endif
    
    // Create the kernel.
    Twister::start();
    
    // Create the user interface.
    TwisterUI::instance();

    // Initialise the kernel.
    twister->init();
    
    // Enter the event loop.
    int retval = app.exec();
    
    // Clean up.
    twister->shutdown();

    return retval;
}

// vim:ts=4:sw=4:tw=78
