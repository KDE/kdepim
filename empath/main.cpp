/*
    Empath - Mailer for KDE
    
    Copyright (C) 1998, 1999 Rik Hemsley rik@kde.org
    
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

// System includes
#include <sys/stat.h>
#include <unistd.h>
#include <iostream.h>

// KDE includes
#include <kapp.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kconfig.h>
#include <kstartparams.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathUI.h"

void empathMain(int, char **);

    int
main(int argc, char ** argv)
{
    // Don't do anything if we're being run as root.
    if (getuid() == 0 || geteuid() == 0) {
        cerr << "DO NOT RUN GUI APPS AS ROOT !" << endl;
        return 1;
    }    

    int prev_umask = umask(077);
    
    KStartParams opts(argc, argv);

    if (opts.check("--help", "-h", true)) {
        cerr    << "usage: empath --help    gives you this message"     << endl
                << "              --version prints version information" << endl;
        return 0;
    }
    
    if (opts.check("--version", "-v", true)) {
        cerr << "Empath version " << EMPATH_VERSION_STRING.latin1() << endl;
        return 0;
    }
    
    empathMain(argc, argv);

    umask(prev_umask);
    
    return 0;
}

    void
empathMain(int argc, char ** argv)
{
    KApplication * app = new KApplication(argc, argv, "empath");
    
    KGlobal::dirs()->addResourceType("indices", "/share/apps/empath/indices");
    KGlobal::dirs()->addResourceType("cache", "/share/apps/empath/cache");
    
    // Don't do dollar expansion by default.
    KGlobal::config()->setDollarExpansion(false);    

    // Create the kernel.
    Empath::start();
    
    // Create the user interface.
    EmpathUI * ui = new EmpathUI;

    // Attempt to get the UI up and running quickly.
    app->processEvents();

    // Initialise the kernel.
    empath->init();
    
    cerr << endl << "Empath initialised. Entering event loop." << endl;

    // Enter the event loop.
    app->exec();
    
    cerr << endl << "Empath exited event loop. Shutting down." << endl;
    
    delete ui;
    ui = 0;
    
    empath->shutdown();
    
    cerr << endl << "Empath shutdown complete." << endl;
}

// vim:ts=4:sw=4:tw=78
