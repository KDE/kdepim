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
#include <stdio.h>

// KDE includes
#include <kapp.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kstartparams.h>

// Local includes
#include "Empath.h"
#include "EmpathDefines.h"
#include "EmpathUI.h"

int EmpathMain(int argc, char * argv[]);

    int
main(int argc, char * argv[])
{
    // Don't do anything if we're being run as root.
    if (getuid() == 0 || geteuid() == 0) {
        fprintf(stderr, "DO NOT RUN GUI APPS AS ROOT !\n");
        return 1;
    }    

    int prev_umask = umask(077);
    
    EmpathMain(argc, argv);
    
    umask(prev_umask);
}

    int
EmpathMain(int argc, char * argv[])
{
    KStartParams opts(argc, argv);

    if (opts.check("--help", "-h", true)) {
        fprintf(stderr,
            "usage: empath --help    gives you this message\n"
            "              --server  runs as CORBA server\n"
            "              --version prints version information\n");
        return 0;
    }
    
    if (opts.check("--version", "-v", true)) {
        fprintf(stderr, "empath version %s\n", EMPATH_VERSION_STRING.latin1());
        return 0;
    }
    
    // Create a KApplication.
    KApplication * app = new KApplication(argc, argv, "empath");
    CHECK_PTR(app);
    
    // Don't do dollar expansion by default.
    KGlobal::config()->setDollarExpansion(false);    

    // Create the kernel.
    Empath::start();
    
    EmpathUI * ui(0);

    if (!opts.check("--server", "-s", true)) {
    
        // Create the user interface.
        ui = new EmpathUI;
        CHECK_PTR(ui);
        
        // Attempt to get the UI up and running quickly.
        app->processEvents();
    }

    // Initialise the kernel.
    empath->init();

    // Enter the event loop.
    int retval = app->exec();
    
    delete ui;
    ui = 0;
    empath->shutdown();

    return retval;
}
// vim:ts=4:sw=4:tw=78
// vim:ts=4:sw=4:tw=78
