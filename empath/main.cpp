/*
	Empath - Mailer for KDE
	
	Copyright (C) 1998 Rik Hemsley rik@kde.org
	
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
#include <kconfig.h>
#include <kglobal.h>

// Local includes
#include "lib/Empath.h"
#include "ui/EmpathUI.h"

	int
EmpathMain(int argc, char * argv[])
{
	fprintf(stderr, "===================== Empath: Creating KApplication\n");

	// Create a KApplication.
	KApplication * app = new KApplication(argc, argv, "empath");
	CHECK_PTR(app);
	
	// Don't do dollar expansion by default.
	KGlobal::config()->setDollarExpansion(false);
	
	fprintf(stderr, "===================== Empath: Creating kernel\n");
	
	// Create the kernel.
	Empath * e = new Empath;
	
	fprintf(stderr, "===================== Empath: Creating user interface\n");
	
	// Create the user interface.
	new EmpathUI;

	// Attempt to get the UI up and running quickly.
	app->processEvents();

	fprintf(stderr, "===================== Empath: Initialising kernel\n");

	// Initialise the kernel.
	e->init();

	fprintf(stderr, "===================== Empath: Entering event loop\n");
	// Enter the event loop.
	return app->exec();
}

	int
main(int argc, char * argv[])
{
	fprintf(stderr, "======================================================\n");
	fprintf(stderr, "==================== Empath Startup ==================\n");
	fprintf(stderr, "======================================================\n");
	
	// Don't run if we're being run as root.
	if (getuid() == 0 || geteuid() == 0) {
		fprintf(stderr, "DO NOT RUN GUI APPS AS ROOT !\n");
		return 1;
	}	

	int prev_umask = umask(077);
	
	EmpathMain(argc, argv);
	
	umask(prev_umask);
	
	fprintf(stderr, "======================================================\n");
	fprintf(stderr, "=================== Empath Shutdown ==================\n");
	fprintf(stderr, "======================================================\n");
}

