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
	KAboutData aboutData("importdatebook","Import Date Book","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	//  KApplication app( false, false );
	KApplication app;

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

#ifdef DEBUG
	debug_level= (args->isSet("verbose")) ? 4 : 0;
#endif
	QString datadir = args->getOption("data-dir");
	QString vcalfile = args->getOption("vcal-file");

	if (datadir.isEmpty())
	{
		kdWarning() << "! Must provide a data-directory." << endl;
	}
	if (vcalfile.isEmpty())
	{
		kdWarning() << "! Must provide a vcal-file to read." << endl;
	}
	if (datadir.isEmpty() || vcalfile.isEmpty())
	{
		return 1;
	}

	KCal::CalendarLocal *calendar = new KCal::CalendarLocal( QString() );
	if (!calendar->load( vcalfile ))
	{
		return 1;
	}

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "DatebookDB" );
	db.createDatabase( 0xdead, 0xbeef );
	PilotDateInfo appInfo;
	appInfo.writeTo(&db);

	KCal::Event::List events = calendar->rawEvents();

	for (KCal::Event::List::ConstIterator i = events.begin();
		i != events.end(); ++i)
	{
		PilotDateEntry d(*appInfo.info());
		memset(&d,0,sizeof(d));
		if (VCalRecord::setDateEntry(&d,*i))
		{
			PilotRecord *r = d.pack();
			if (r)
			{
				db.writeRecord(r);
				delete r;
			}
		}
	}

	return 0;
}

