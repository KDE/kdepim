/* mergecalendars			KPilot
**
** Copyright (C) 2007 by Jason 'vanRijn' Kasper <vR@movingparts.net)
**
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
	{"korgfile <path>","KOrganizer master file", "."},
	{"newevents <path>","Calendar file to merge into korganizer", 0},
	KCmdLineLastOption
};



int main(int argc, char **argv)
{
	FUNCTIONSETUP;

	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("mergecalendars","Merge libkcal Calendars","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	debug_level= (args->isSet("verbose")) ? 4 : 0;

	QString korgfile = args->getOption("korgfile");
	QString newevents = args->getOption("newevents");

	if (korgfile.isEmpty())
	{
		kdError() << "! Must provide a korganizer file." << endl;
	}
	if (newevents.isEmpty())
	{
		kdError() << "! Must provide a newevents file." << endl;
	}
	if (korgfile.isEmpty() || newevents.isEmpty())
	{
		return 1;
	}

	QString korgsave = QString("%1.updated").arg(korgfile);
	QString neweventssave = QString("%1.updated").arg(newevents);

	kdDebug() << "Using korgfile: [" << korgfile 
		<< "], newevents: [" << newevents
		<< "]" << endl;
	kdDebug() << "Saving korgfile to: [" << korgsave 
		<< "], newevents: [" << neweventssave
		<< "]" << endl;

	KCal::CalendarLocal *calkorg = new KCal::CalendarLocal( QString::fromLatin1("UTC") );
	KCal::CalendarLocal *calxchg = new KCal::CalendarLocal( QString::fromLatin1("UTC") );
	if (!calkorg || !calxchg)
	{
		kdError() << "Unable to create base calendar objects." << endl;
		return 1;
	}

	if (!calkorg->load(korgfile) || !calxchg->load(newevents))
	{
		kdError() << "Unable to load calendar files." << endl;
		return 1;
	}

	int numkorgstart = calkorg->incidences().count();
	int numxchgstart = calxchg->incidences().count();

	kdDebug() << "  - Opened korganizer calendar with: [" 
		<< numkorgstart << "] incidences." << endl;
	kdDebug() << "  - Opened newevents calendar with: [" 
		<< numxchgstart << "] incidences." << endl;
	

	KCal::Event::List korgAllEvents;
	KCal::Event::List::ConstIterator korgAllEventsIterator;
	korgAllEvents = calkorg->events();
	korgAllEvents.setAutoDelete(false);

	KCal::Event::List xchgAllEvents;
	KCal::Event::List::ConstIterator xchgAllEventsIterator;
	xchgAllEvents = calxchg->events();
	xchgAllEvents.setAutoDelete(false);

	kdDebug() << "Looking for previous pilot ids for exchange events..." << endl;

	// iterate through all events and try to find a korganizer event
	// that matches up with this external event's UID
	xchgAllEventsIterator = xchgAllEvents.end();
	KCal::Event *ev = *xchgAllEventsIterator;
	while (ev)
	{
		QString uid = ev->uid();
		KCal::Event * evkorg = calkorg->event(uid);
		if (evkorg)
		{
			unsigned long pilotId = evkorg->pilotId();
			kdDebug() << "Found korg event for uid: ["
				<< uid << "], pilotId: [" 
				<< pilotId << "]" << endl;
		}
		// get the next one
		ev = * --xchgAllEventsIterator;
	}
	/*
	// use dynamic_cast which returns a null pointer if the class does not match...
	fAllEvents.remove(dynamic_cast<KCal::Event*>(e));
	if (!fCalendar) return;
	fCalendar->deleteEvent(dynamic_cast<KCal::Event*>(e));
	// now just in case we're in the middle of reading through our list
	// and we delete something, set reading to false so we start at the
	// top again next time and don't have problems with our iterator
	reading = false;
	*/

	/*
	// search for event
	KCal::Event::List::ConstIterator it;
	for( it = fAllEvents.begin(); it != fAllEvents.end(); ++it ) {
		KCal::Event *event = *it;
		if ((recordid_t)event->pilotId() == id) return event;
	}
	return 0L;
	*/



	return 0;
}

