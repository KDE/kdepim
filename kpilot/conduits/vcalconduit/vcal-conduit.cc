/* KPilot
**
** Copyright (C) 2002-2003 Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** This file defines the vcal-conduit plugin.
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <libkcal/calendar.h>
#include <libkcal/calendarlocal.h>
#include <libkcal/recurrence.h>
#include <libkcal/vcalformat.h>

#include "pilotDateEntry.h"
#include "pilotDatabase.h"

#include "vcal-conduit.moc"
#include "vcalconduitSettings.h"

#include "kcalRecord.h"
#include "vcalRecord.h"


extern "C"
{

unsigned long version_conduit_vcal = Pilot::PLUGIN_API;

}




VCalConduitPrivate::VCalConduitPrivate(KCal::Calendar *b) :
	VCalConduitPrivateBase(b)
{
	fAllEvents.setAutoDelete(false);
}

void VCalConduitPrivate::addIncidence(KCal::Incidence*e)
{
	fAllEvents.append(dynamic_cast<KCal::Event*>(e));
	fCalendar->addEvent(dynamic_cast<KCal::Event*>(e));
}

int VCalConduitPrivate::updateIncidences()
{
	FUNCTIONSETUP;
	if (!fCalendar) return 0;
	fAllEvents = fCalendar->events();
	fAllEvents.setAutoDelete(false);
	return fAllEvents.count();
}


void VCalConduitPrivate::removeIncidence(KCal::Incidence *e)
{
	// use dynamic_cast which returns a null pointer if the class does not match...
	fAllEvents.remove(dynamic_cast<KCal::Event*>(e));
	if (!fCalendar) return;
	fCalendar->deleteEvent(dynamic_cast<KCal::Event*>(e));
	// now just in case we're in the middle of reading through our list
	// and we delete something, set reading to false so we start at the
	// top again next time and don't have problems with our iterator
	reading = false;
}


KCal::Incidence *VCalConduitPrivate::findIncidence(recordid_t id)
{
	KCal::Event::List::ConstIterator it;
	for( it = fAllEvents.begin(); it != fAllEvents.end(); ++it ) {
		KCal::Event *event = *it;
		if ((recordid_t)event->pilotId() == id) return event;
	}
	return 0L;
}

KCal::Incidence *VCalConduitPrivate::findIncidence(PilotRecordBase *tosearch)
{
	PilotDateEntry*entry=dynamic_cast<PilotDateEntry*>(tosearch);
	if (!entry) return 0L;

	QString title=entry->getDescription();
	QDateTime dt=readTm( entry->getEventStart() );

	KCal::Event::List::ConstIterator it;
	for( it = fAllEvents.begin(); it != fAllEvents.end(); ++it ) {
		KCal::Event *event = *it;
		if ( (event->dtStart() == dt) && (event->summary() == title) ) return event;
	}
	return 0L;
}



KCal::Incidence *VCalConduitPrivate::getNextIncidence()
{
	FUNCTIONSETUP;

	if (reading) {
		++fAllEventsIterator;
	} else {
		reading=true;
		fAllEventsIterator = fAllEvents.begin();
	}
	// At end of list, or empty list.
	return (fAllEventsIterator == fAllEvents.end()) ? 0L : *fAllEventsIterator;
}

/** Find the next incidence in the list which ddoes not have the SYNCNONE flag set. The
 *  current position is always stored in the iteratoor fAllEventsIterator, so we can just
 *  start from there. Only if reading==false, we haven't yet started goind through the
 *  incidents, so start at fAllEvents.begin() in that case */
KCal::Incidence *VCalConduitPrivate::getNextModifiedIncidence()
{
	FUNCTIONSETUP;
	KCal::Event*e=0L;
	if (!reading)
	{
		// Start from the top
		reading=true;
		fAllEventsIterator = fAllEvents.begin();
	}
	else
	{
		// Move on from current position
		++fAllEventsIterator;
	}

	// Fetch (new) current if possible.
	if ( fAllEventsIterator != fAllEvents.end() ) e = *fAllEventsIterator;
	// Then walk the list until we find an unsynced entry
	while ( fAllEventsIterator != fAllEvents.end() &&
		e && e->syncStatus()!=KCal::Incidence::SYNCMOD && e->pilotId() > 0)
	{
		e = (++fAllEventsIterator != fAllEvents.end()) ? *fAllEventsIterator : 0L;
	}
	return (fAllEventsIterator == fAllEvents.end()) ? 0L : 	*fAllEventsIterator;
}



/****************************************************************************
 *                          VCalConduit class                               *
 ****************************************************************************/

VCalConduit::VCalConduit(KPilotLink *d,
	const char *n,
	const QStringList &a) :
	VCalConduitBase(d,n,a),
	fAppointmentAppInfo( 0L )
{
	FUNCTIONSETUP;
	fConduitName=i18n("Calendar");
}


VCalConduit::~VCalConduit()
{
//	FUNCTIONSETUP;
}

VCalConduitPrivateBase *VCalConduit::createPrivateCalendarData(KCal::Calendar *fCalendar) {
	return new VCalConduitPrivate(fCalendar);
}

void VCalConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	KPILOT_DELETE(fAppointmentAppInfo);
	fAppointmentAppInfo = new PilotDateInfo( fDatabase );
}

const QString VCalConduit::getTitle(PilotRecordBase *de)
{
	PilotDateEntry*d=dynamic_cast<PilotDateEntry*>(de);
	if (d) return QString(d->getDescription());
	return QString::null;
}



PilotRecord *VCalConduit::recordFromIncidence(PilotRecordBase *de, const KCal::Incidence*e)
{
	FUNCTIONSETUP;
	if (!de || !e)
	{
		DEBUGKPILOT << fname
			<< ": got NULL entry or NULL incidence." << endl;
		return 0L;
	}

	if ( (e->recurrenceType() == KCal::Recurrence::rYearlyDay) ||
		(e->recurrenceType() ==  KCal::Recurrence::rYearlyPos) )
	{
		// Warn ahead of time
		emit logMessage(i18n("Event \"%1\" has a yearly recurrence other than by month, will change this to recurrence by month on handheld.").arg(e->summary()));
	}

	PilotDateEntry *dateEntry = dynamic_cast<PilotDateEntry*>(de);
	if (!dateEntry)
	{
		// Secretly wasn't a date entry after all
		return 0L;
	}

	const KCal::Event *event = dynamic_cast<const KCal::Event *>(e);
	if (!event)
	{
		DEBUGKPILOT << fname << ": Incidence is not an event." << endl;
		return 0L;
	}

	if (KCalSync::setDateEntry(dateEntry, event,*fAppointmentAppInfo->categoryInfo()))
	{
		return dateEntry->pack();
	}
	else
	{
		return 0L;
	}
}

KCal::Incidence *VCalConduit::incidenceFromRecord(KCal::Incidence *e, const PilotRecordBase  *de)
{
	FUNCTIONSETUP;

	if (!de || !e)
	{
		DEBUGKPILOT << fname
			<< ": Got NULL entry or NULL incidence." << endl;
		return 0L;
	}

	const PilotDateEntry *dateEntry = dynamic_cast<const PilotDateEntry *>(de);
	if (!dateEntry)
	{
		DEBUGKPILOT << fname << ": HH record not a date entry." << endl;
		return 0L;
	}

	KCal::Event *event = dynamic_cast<KCal::Event *>(e);
	if (!event)
	{
		DEBUGKPILOT << fname << ": Incidence is not an event." << endl;
		return 0L;
	}

	KCalSync::setEvent(event, dateEntry,*fAppointmentAppInfo->categoryInfo());
	return e;
}



PilotRecordBase * VCalConduit::newPilotEntry(PilotRecord*r)
{
	return new PilotDateEntry(r);
}

KCal::Incidence* VCalConduit::newIncidence()
{
  return new KCal::Event;
}

static VCalConduitSettings *config_vcal = 0L;

VCalConduitSettings *VCalConduit::theConfig()
{
	if (!config_vcal)
	{
		config_vcal = new VCalConduitSettings(CSL1("Calendar"));
	}

	return config_vcal;
}

VCalConduitSettings *VCalConduit::config() {
	return theConfig();
}



// vim: ts=4:sw=4:noexpandtab:

