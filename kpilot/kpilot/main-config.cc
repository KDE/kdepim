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
** Bug reports and questions can be sent to kde-pim@kde.org
*/


static const char *config_id =
	"$Id$";


#include "options.h"

#include <stdlib.h>

#include <qcombobox.h>
#include <qvbox.h>


#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kpilotConfigDialog.h"
#include "conduitConfigDialog.h"
#include "kpilotConfig.h"

static KCmdLineOptions kpilotoptions[] = {
	{ "c",0,0 },
	{ "conduits", I18N_NOOP("Configure conduits instead."), 0},
	{ 0,0,0 }
} ;

int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KAboutData about("kpilotConfig", I18N_NOOP("KPilot Configurator"),
		KPILOT_VERSION,
		"KPilot Configurator",
		KAboutData::License_GPL, "(c) 2001, Dan Pilone");
	about.addAuthor("Dan Pilone",
		I18N_NOOP("Project Leader"),
		"pilone@slac.com", "http://www.slac.com/pilone/kpilot_home/");
	about.addAuthor("Adriaan de Groot",
		I18N_NOOP("Maintainer"),
		"groot@kde.org", "http://www.cs.kun.nl/~adridg/kpilot/");

	KCmdLineArgs::init(argc, argv, &about);
#ifdef DEBUG
	KCmdLineArgs::addCmdLineOptions(debug_options, "debug", "debug");
#endif
	KCmdLineArgs::addCmdLineOptions(kpilotoptions,"kpilotconfig",0L,"debug");
	KApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	KApplication a;

	KPilotConfig::getDebugLevel(true);

	int r = 0;

	if (p->isSet("conduits"))
	{
		ConduitConfigDialog *d = new ConduitConfigDialog(0L,
			"conduitConfig",true);
		r = d->exec();
	}
	else
	{
		KDialogBase *d = new KPilotConfigDialog(0L, "configDialog", true);
		r = d->exec();
	}

	if (r)
	{
#ifdef DEBUG
		DEBUGKPILOT << fname << ": Configuration was okayed." << endl;
#endif
	}
	else
	{
#ifdef DEBUG
		DEBUGKPILOT << fname
			<< ": Configuration was cancelled." << endl;
#endif
	}

	return r;

	/* NOTREACHED */
	(void) config_id;
}


// $Log$
// Revision 1.8  2001/12/29 15:40:47  adridg
// Sanity checking and simplification
//
// Revision 1.7  2001/10/17 08:46:08  adridg
// Minor cleanups
//
// Revision 1.6  2001/10/02 17:49:03  adridg
// Use new-style conduit config
//
// Revision 1.5  2001/10/01 19:53:15  adridg
// Added conduit config
//
// Revision 1.4  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.3  2001/09/23 21:42:35  adridg
// Factored out debugging options
//
// Revision 1.2  2001/09/23 18:28:52  adridg
// Adjusted tests to new .ui and config
//
// Revision 1.1  2001/09/05 21:53:51  adridg
// Major cleanup and architectural changes. New applications kpilotTest
// and kpilotConfig are not installed by default but can be used to test
// the codebase. Note that nothing else will actually compile right now.
//
