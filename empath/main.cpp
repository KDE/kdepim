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
#include <unistd.h>

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
	if (getuid() == 0 || geteuid() == 0) {
		exit(1);
	}

	KApplication app(argc, argv, "empath");
	
	KGlobal::config()->setDollarExpansion(false);
	
	Empath		e;
	EmpathUI	ui;

	// Attempt to get the UI up and running quickly.
	app.processEvents();

	e.init();

	return app.exec();
}

main(int argc, char * argv[])
{
	return EmpathMain(argc, argv);
}
