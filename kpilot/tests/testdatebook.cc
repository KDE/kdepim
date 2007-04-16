/* testaddresses			KPilot
**
** Copyright (C) 2006 by Adriaan de Groot <groot@kde.org)
**
** Test the functions related to address database handling.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>

#include "pilot.h"
#include "pilotDateEntry.h"
#include "pilotLocalDatabase.h"

static const KCmdLineOptions options[] =
{
	{"verbose", "Verbose output", 0},
	{"data-dir <path>","Set data directory", "."},
	KCmdLineLastOption
};



int main(int argc, char **argv)
{
	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("testdatebook","Test Date Book","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level= (args->isSet("verbose")) ? 4 : 0;
#endif
	QString datadir = args->getOption("data-dir");

	DEBUGKPILOT << "### testdatebook\n#\n#" << endl;

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "DatebookDB" );
	PilotDateInfo appinfo( &db );

	for (unsigned int i=0; i<db.recordCount(); ++i)
	{
		PilotRecord *r = db.readRecordByIndex( i );

		if (r)
		{
			DEBUGKPILOT << "# Record @" << (void *)r << " ID=" << r->id() << endl;
			PilotDateEntry a( r );
			DEBUGKPILOT << "# Text Representation:" << endl << a.getTextRepresentation(Qt::PlainText) << endl;
			DEBUGKPILOT << "# Category#" << a.category() << endl;
			DEBUGKPILOT << "# Category Label " << appinfo.categoryName(a.category()) << endl;
			DEBUGKPILOT << "# ID " << a.id() << endl;
			int cat = appinfo.findCategory( CSL1("Fake Cat") );
			a.setCategory( cat );
			DEBUGKPILOT << "# Category#" << a.category() << endl;
			DEBUGKPILOT << "# Category Label " << appinfo.categoryName(a.category()) << endl;
		}
	}

	return 0;
}

