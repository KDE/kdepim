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

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kpilotConfig.h"
#include "conduitSetup.h"
#include "kpilotOptions.h"

KCmdLineOptions kpilotoptions[] =
{
	{ "c",0,0 },
	{ "config-conduits", I18N_NOOP("Configure KPilot conduits."),0 },
	{ "debug <level>", I18N_NOOP("Set debug level."), "0" },
	{ 0,0,0 }
} ;

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
	KCmdLineArgs::addCmdLineOptions(kpilotoptions);
	KApplication::addCmdLineOptions();
	KCmdLineArgs *p=KCmdLineArgs::parsedArgs();

	KPilotConfig::getDebugLevel(p);

	KApplication a;

	if (p->isSet("config-conduits"))
	{
		CConduitSetup *cs = new CConduitSetup(0L);
		int r = cs->exec();
		if (!r)
		{
			DEBUGKPILOT << fname
				<< ": Conduit config was cancelled."
				<< endl;
			return 1;	// Dialog cancelled
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": Conduit config was okayed."
				<< endl;
			return 0;
		}
	}
	else
	{
		KPilotOptions* options = new KPilotOptions(0L);
		int r = options->exec();

		if (!r)
		{
			DEBUGKPILOT << fname
				<< ": Configuration was cancelled."
				<< endl;
			return 1;
		}
		else
		{
			DEBUGKPILOT << fname
				<< ": Configuration was okayed."
				<< endl;
			return 0;
		}
	}

	/* NOTREACHED */
	(void) config_id;
}


// $Log$
