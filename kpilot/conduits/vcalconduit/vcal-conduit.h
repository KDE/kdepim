#ifndef _KPILOT_VCAL_CONDUIT_H
#define _KPILOT_VCAL_CONDUIT_H
/* vcal-conduit.h                       KPilot
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

#include <libkcal/event.h>

#include "pilotDateEntry.h"

#include "vcal-conduitbase.h"

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;

class VCalConduitPrivate : public VCalConduitPrivateBase
{
public:
	VCalConduitPrivate(KCal::Calendar *buddy);
	virtual ~VCalConduitPrivate() {};

	KCal::Event::List fAllEvents;
	KCal::Event::List::ConstIterator fAllEventsIterator;

	virtual int updateIncidences();
	virtual void addIncidence(KCal::Incidence*);
	virtual void removeIncidence(KCal::Incidence *);
	virtual KCal::Incidence *findIncidence(recordid_t);
	/**
	 * Find the incidence based on tosearch's description and date information.
	 * Returns 0L if no incidence could be found.
	 */
	virtual KCal::Incidence *findIncidence(PilotRecordBase *tosearch);
	virtual KCal::Incidence *getNextIncidence();
	virtual KCal::Incidence *getNextModifiedIncidence();
	virtual int count() {return fAllEvents.count();};
} ;



class VCalConduit : public VCalConduitBase
{
Q_OBJECT
public:
	VCalConduit(KPilotLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~VCalConduit();

protected:
	virtual const QString dbname() { return CSL1("DatebookDB"); };

	virtual void preSync() {VCalConduitBase::preSync(); _getAppInfo(); };
	virtual VCalConduitPrivateBase *createPrivateCalendarData(KCal::Calendar *fCalendar);

	void _getAppInfo();
	void _setAppInfo();

	virtual PilotRecordBase *newPilotEntry(PilotRecord*r);
	virtual KCal::Incidence*newIncidence();
	virtual const QString getTitle(PilotRecordBase *de);
	virtual VCalConduitSettings *config();
public:
	static VCalConduitSettings *theConfig();

protected:
	virtual PilotRecord *recordFromIncidence(PilotRecordBase *de,
		const KCal::Incidence *e);
	virtual KCal::Incidence *incidenceFromRecord(KCal::Incidence *e,
		const PilotRecordBase *de);

	PilotDateInfo *fAppointmentAppInfo;
};

#endif
