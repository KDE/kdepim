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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
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
#ifdef DEBUG
	{"debug <level>", I18N_NOOP("Set debugging level"), "0"},
#endif
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
	KCmdLineArgs::addCmdLineOptions(kpilotoptions,"kpilotconfig");
	KApplication::addCmdLineOptions();
	KCmdLineArgs *p = KCmdLineArgs::parsedArgs();

	KApplication a;

#ifdef DEBUG
	debug_level = p->getOption("debug").toInt();
#endif
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
