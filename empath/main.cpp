/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
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
#include <kuniqueapp.h>

// Local includes
#include "Empath.h"
#include "EmpathUI.h"

    int
main(int argc, char ** argv)
{
    // Don't do anything if we're being run as root.
    if (getuid() == 0 || geteuid() == 0) {
        fprintf(stderr, "DO NOT RUN GUI APPS AS ROOT !\n");
        return 1;
    }    

    // Pick a sensible umask for everything Empath does.
    int prev_umask = umask(077);
    
//    if (!KUniqueApplication::start(argc, argv, "empath"))
//        exit(0);
    
//    KUniqueApplication app(argc, argv, "empath");
    KApplication app(argc, argv, "empath");
    
    // Create the kernel.
    Empath::start();
    
    // Create the user interface.
    EmpathUI * ui = new EmpathUI;

    // Attempt to get the UI up and running quickly.
    app.processEvents();

    // Initialise the kernel.
    empath->init();
    
    empathDebug("Entering event loop");
    // Enter the event loop.
    int retval = app.exec();
    
    delete ui;
    ui = 0;
    
    empath->shutdown();

    // Restore umask.
    umask(prev_umask);

    return retval;
}

// vim:ts=4:sw=4:tw=78
