/* testaddresses			KPilot
**
** Copyright (C) 2007 by Jason 'vanRijn' Kasper <vR@movingparts.net)
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
#include <kconfigskeleton.h>

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
	FUNCTIONSETUP;

	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("exportdatebook","Emport Date Book","0.1");
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
		WARNINGKPILOT << "! Must provide a vcal-file to write to." << endl;
	}
	if (datadir.isEmpty() || vcalfile.isEmpty())
	{
		return 1;
	}

	/*
	KConfig korgcfg( locate( "config", CSL1("korganizerrc") ) );

	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	QString tz(korgcfg.readEntry( "TimeZoneId" ) );

	DEBUGKPILOT << fname << ": KOrganizer's time zone = " << tz << endl;

	KCal::CalendarLocal *calendar = new KCal::CalendarLocal( tz );
	*/
	KCal::CalendarLocal *calendar = new KCal::CalendarLocal( QString() );

	if (!calendar)
	{
		WARNINGKPILOT << "! Can't create calendar object." << endl;
		return 1;
	}

	Pilot::setupPilotCodec( CSL1("Latin1") );

	PilotLocalDatabase db( datadir, "DatebookDB" );

	PilotDateInfo *fAppointmentAppInfo = new PilotDateInfo( &db );

	int currentRecord = 0;
	PilotRecord *pilotRec = 0;
	PilotDateEntry *d = 0;

	while ((pilotRec = db.readRecordByIndex(currentRecord++)) != NULL)
	{
		d = new PilotDateEntry(pilotRec);

		KCal::Event*event = new KCal::Event;

		KCalSync::setEvent(event, d,*fAppointmentAppInfo->categoryInfo());

		event->setSyncStatus( KCal::Incidence::SYNCNONE );

		calendar->addEvent(event);

	}

	calendar->save(vcalfile);

	return 0;
}

