/* main-config.cc                  KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is a non-installed application that exercises the
** configuration dialog and config code for KPilot.
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


static const char *config_id="$Id$";


#include "options.h"

#include <stdlib.h>

#include <qcombobox.h>
#include <qvbox.h>


#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kpilotConfigDialog.h"
#include "kpilotConfig.h"

int main(int argc, char **argv)
{
	FUNCTIONSETUP;

        KAboutData about("kpilotConfig", I18N_NOOP("KPilot Configurator"),
                         KPILOT_VERSION,
                         "KPilot Configurator",
                         KAboutData::License_GPL,
                         "(c) 2001, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com",
		"http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"adridg@cs.kun.nl",
		"http://www.cs.kun.nl/~adridg/kpilot/");

        KCmdLineArgs::init(argc, argv, &about);
#ifdef DEBUG
	KCmdLineArgs::addCmdLineOptions(debug_options,"debug");
#endif
	KApplication::addCmdLineOptions();
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

	KApplication a;

	KPilotConfig::getDebugLevel(false);

	int r=0;

	KDialogBase *d = new KPilotConfigDialog(0L,"configDialog",true);
	r = d->exec();

	if (r)
	{
		DEBUGKPILOT << fname
			<< ": Configuration was okayed."
			<< endl;
	}
	else
	{
		DEBUGKPILOT << fname
			<< ": Configuration was cancelled."
			<< endl;
	}

	return r;

	/* NOTREACHED */
	(void) config_id;
}


// $Log$
// Revision 1.2  2001/09/23 18:28:52  adridg
// Adjusted tests to new .ui and config
//
// Revision 1.1  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
