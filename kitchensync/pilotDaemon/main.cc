/* main.cc                          PilotDaemon
**
** Copyright (C) 2001 by Adriaan de Groot
**
** This program fits into the KDE hardware device-sync architecture
** and provides Sync capabilities for Palm platform devices on a
** serial port.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

static const char *main_id = "$Id:$";

#include "config.h"
#include "lib/debug.h"

#include <qtimer.h>

#include <kaboutdata.h>
#include <kuniqueapp.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "configdialog.h"

#include "pilotDaemon.h"


int main(int argc, char **argv)
{
	KAboutData about("pilotDaemon",
		I18N_NOOP("Pilot Daemon"),
		"0.1",
		I18N_NOOP("KitchenSync Pilot Daemon"),
		KAboutData::License_GPL,
		"(C) 2001 Adriaan de Groot");
	KCmdLineArgs::init(argc,argv,&about);
	KCmdLineArgs::addCmdLineOptions(debug_options);
	KUniqueApplication::addCmdLineOptions();

	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();
	debug_level = p->isSet("debug");

	KUniqueApplication a(true,true);

	Config *c = new Config();
	PilotDaemon *d = new PilotDaemon(c);

	if (!d->connectToManager()) return 1;

	// Notify sync manager of sync types
	// 3: Open device, watch for activity
	// An device activity, start Sync
	// Close device, goto 3


	return a.exec();
}
