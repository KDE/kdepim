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

#include <kaboutdata.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kconfigskeleton.h>

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>

static const KCmdLineOptions options[] =
{
	{"korgfile <path>","KOrganizer master file", 0},
	{"newfile <path>","Calendar file to merge into korganizer", 0},
	{"verbose", "Verbose debugging", 0},
	KCmdLineLastOption
};



int main(int argc, char **argv)
{

	KApplication::disableAutoDcopRegistration();

	KAboutData aboutData("mergecalendars","Merge libkcal Calendars","0.1");
	KCmdLineArgs::init(argc,argv,&aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication app( false, false );

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	int debug_level= (args->isSet("verbose")) ? 4 : 0;

	QString korgfile = args->getOption("korgfile");
	QString newfile = args->getOption("newfile");

	if (korgfile.isEmpty())
	{
		kdError() << "! Must provide a korganizer file." << endl;
	}
	if (newfile.isEmpty())
	{
		kdError() << "! Must provide a newfile file." << endl;
	}
	if (korgfile.isEmpty() || newfile.isEmpty())
	{
		return 1;
	}

	QString korgsave = QString("%1.updated").arg(korgfile);
	QString newfilesave = QString("%1.updated").arg(newfile);

	kdDebug() << "Using korgfile: [" << korgfile 
		<< "]" << endl;
	kdDebug() << "Using newfile: [" << newfile
		<< "]" << endl;
	kdDebug() << "Will save korgfile to: [" << korgsave 
		<< "]" << endl;
	kdDebug() << "Will save newfile to: [" << newfilesave
		<< "]" << endl << endl;

	KCal::CalendarLocal *calkorg = new KCal::CalendarLocal( QString::fromLatin1("UTC") );
	KCal::CalendarLocal *calnew = new KCal::CalendarLocal( QString::fromLatin1("UTC") );
	if (!calkorg || !calnew)
	{
		kdError() << "Unable to create base calendar objects." << endl;
		return 1;
	}

	if (!calkorg->load(korgfile) || !calnew->load(newfile))
	{
		kdError() << "Unable to load calendar files." << endl;
		return 1;
	}

	int numkorgstart = calkorg->incidences().count();
	int numnewstart = calnew->incidences().count();

	kdDebug() << "  - Opened korganizer calendar with: [" 
		<< numkorgstart << "] incidences." << endl;
	kdDebug() << "  - Opened newfile calendar with: [" 
		<< numnewstart << "] incidences." << endl;
	

	KCal::Event::List korgEvents;
	KCal::Event::List::ConstIterator korgIt;
	korgEvents = calkorg->events();
	korgEvents.setAutoDelete(false);

	KCal::Event::List newEvents;
	KCal::Event::List::ConstIterator newIt;
	newEvents = calnew->events();
	newEvents.setAutoDelete(false);

	kdDebug() << "Looking for previous pilot ids for exchange events..." << endl;

	// iterate through all events and try to find a korganizer event
	// that matches up with this external event's UID
	unsigned int numkorgpilotids = 0;
	KCal::Event *ev = 0;
	for (newIt = newEvents.begin(); newIt != newEvents.end(); ++newIt ) 
	{
		ev = *newIt;
		QString uid = ev->uid();
		if (debug_level)
			kdDebug() << "  - Looking at event: [" 
			<< ev->summary() << "], uid: ["
			<< uid << "]" << endl;

		KCal::Event * evkorg = calkorg->event(uid);
		if ( evkorg && (evkorg->pilotId() > 0) )
		{
			unsigned long pilotId = evkorg->pilotId();

			if (debug_level)
				kdDebug() << "Found korg event for uid: ["
				<< uid << "], pilotId: [" 
				<< pilotId << "]" << endl;

			ev->setPilotId(pilotId);
			ev->setSyncStatus(KCal::Incidence::SYNCMOD);

			++numkorgpilotids;
		}
	}

	kdDebug() << "Matched: [" << numkorgpilotids << "] events."<< endl;

	kdDebug() << "Now searching for previous WorkXChange events in korganizer's calendar." << endl;

	// iterate through all events and try to find a korganizer event
	// that matches up with this external event's UID
	unsigned int numkorgremoved = 0;

	QString categoryToken = QString("WorkXChange");

	// careful iterating and removing...
	KCal::Event *next = 0;

	korgIt = korgEvents.begin();
	for ( ev = *korgIt; ev != 0; ev = next )
	{
		if (++korgIt == korgEvents.end()) 
		{
			next = 0;
		}
		else
		{
			next = *korgIt;
		}

		if (ev->categoriesStr().contains(categoryToken))
		{
			if (debug_level)
				kdDebug() << "  - Found matching event: [" 
				<< ev->summary() << "], uid: ["
				<< ev->uid() << "]. Removing." << endl;

			korgEvents.remove(ev);
			calkorg->deleteEvent(ev);

			++numkorgremoved;
		}
	}

	kdDebug() << "  - Found: [" << numkorgremoved
		<< "] prior: [" << categoryToken 
		<< "] category events." << endl;
	
	kdDebug() << "Merging new events into korganizer calendar..." 
		<< endl;

	for (newIt = newEvents.begin(); newIt != newEvents.end(); ++newIt ) 
	{
		ev = *newIt;
		korgEvents.append(ev);
		calkorg->addEvent(ev);
	}

	kdDebug() << "Ended up with: [" << korgEvents.count() 
		<< "] events in korganizer calendar." << endl;

	kdDebug() << "Saving updated korganizer file..." << endl;
	calkorg->save(korgsave);

	kdDebug() << "Saving updated newfile file..." << endl;
	calnew->save(newfilesave);

	return 0;
}

