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

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>

#include "pilot.h"
#include "pilotDateEntry.h"
#include "pilotLocalDatabase.h"
#include "../conduits/vcalconduit/kcalRecord.cc"
#include "../conduits/vcalconduit/vcalRecord.cc"

static const KCmdLineOptions options[] =
{
	{"verbose", "Verbose output", 0},
	{"data-dir <path>","Set data directory", "."},
	{"vcal-file <path>","Set vcal file", 0},
	KCmdLineLastOption
};



int main(int argc, char **argv)
{
	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("importdatebook","Import Date Book","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	debug_level= (args->isSet("verbose")) ? 4 : 0;

	QString datadir = args->getOption("data-dir");
	QString vcalfile = args->getOption("vcal-file");

	if (datadir.isEmpty())
	{
		WARNINGKPILOT << "! Must provide a data-directory." << endl;
	}
	if (vcalfile.isEmpty())
	{
		WARNINGKPILOT << "! Must provide a vcal-file to read." << endl;
	}
	if (datadir.isEmpty() || vcalfile.isEmpty())
	{
		return 1;
	}

	DEBUGKPILOT << "Using vcal-file: [" << vcalfile 
		<< "], creating DatebookDB in: [" << datadir
		<< "]" << endl;

	KCal::CalendarLocal *calendar = new KCal::CalendarLocal( QString::fromLatin1("UTC") );
	if (!calendar || !calendar->load( vcalfile ))
	{
		return 1;
	}

	DEBUGKPILOT << "Opened calendar with: [" 
		<< calendar->incidences().count() << "] incidences." << endl;

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "DatebookDB" );
	db.createDatabase( 0xdead, 0xbeef );
	PilotDateInfo appInfo(0L);
	appInfo.resetToDefault();
	appInfo.writeTo(&db);

	KCal::Event::List events = calendar->events();

	for (KCal::Event::List::ConstIterator i = events.begin();
		i != events.end(); ++i)
	{
		PilotDateEntry * d = new PilotDateEntry();

		const KCal::Event *e = *i;
		DEBUGKPILOT << "event: [" << e->summary() << "]" << endl;

		if (KCalSync::setDateEntry(d,e,*appInfo.categoryInfo()))
		{
DEBUGKPILOT << "got here." << endl;
			PilotRecord *r = d->pack();
			if (r)
			{
				db.writeRecord(r);
				delete r;
			}
		}
	}

	return 0;
}

