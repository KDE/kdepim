#ifndef _KPILOT_VCAL_CONDUIT_H
#define _KPILOT_VCAL_CONDUIT_H
/* vcal-conduit.h                       KPilot
**
** Copyright (C) 2002 Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include <plugin.h>

#include <libkcal/event.h>
#include <libkcal/calendarlocal.h>
#include "vcal-factory.h"
#include "vcal-conduitbase.h"
#include <pilotDateEntry.h>
//#include <pilotRecord.h>

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
//class PilotAppCategory;
//class PilotDateEntry: public PilotAppCategory;
//class VCalConduitFactory;

//class VCalConduitPrivateBase;

class VCalConduitPrivate : public VCalConduitPrivateBase
{
public:
	VCalConduitPrivate(KCal::CalendarLocal *buddy);
	virtual ~VCalConduitPrivate() {};

#ifdef KDE2
	QList<KCal::Event> fAllEvents;
#else
	QPtrList<KCal::Event> fAllEvents;
#endif

	virtual int updateIncidences();
	virtual void addIncidence(KCal::Incidence*);
	virtual void removeIncidence(KCal::Incidence *);
	virtual KCal::Incidence *findIncidence(recordid_t);
	/** Find the incidence based on tosearch's description and date information. Returns 0L if no incidence could be found.
	 */
	virtual KCal::Incidence *findIncidence(PilotAppCategory*tosearch);
	virtual KCal::Incidence *getNextIncidence();
	virtual KCal::Incidence *getNextModifiedIncidence();
	virtual int count() {return fAllEvents.count();};
} ;



class VCalConduit : public VCalConduitBase
{
Q_OBJECT
//private:
//	class VCalPrivate;
public:
	VCalConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~VCalConduit();

protected:
	virtual const QString configGroup() { return VCalConduitFactory::group; };
	virtual const QString dbname() { return "DatebookDB"; };

	virtual VCalConduitPrivateBase*newVCalPrivate(KCal::CalendarLocal *fCalendar);


	virtual PilotAppCategory*newPilotEntry(PilotRecord*r) { if (r) return new PilotDateEntry(r);  else return new PilotDateEntry();};
	virtual KCal::Incidence*newIncidence() { return new KCal::Event; };
	virtual const QString getTitle(PilotAppCategory*de);

protected:
	virtual PilotRecord *recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e);
	virtual PilotRecord *recordFromIncidence(PilotDateEntry*de, const KCal::Event*e);
	virtual KCal::Incidence *incidenceFromRecord(KCal::Incidence *, const PilotAppCategory *);
	virtual KCal::Event *incidenceFromRecord(KCal::Event *, const PilotDateEntry *);


	void setStartEndTimes(KCal::Event *,const PilotDateEntry *);
	void setAlarms(KCal::Event *,const PilotDateEntry *);
	void setRecurrence(KCal::Event *,const PilotDateEntry *);
	void setExceptions(KCal::Event *,const PilotDateEntry *);

	void setStartEndTimes(PilotDateEntry *, const KCal::Event * );
	void setAlarms(PilotDateEntry *, const KCal::Event * );
	void setRecurrence(PilotDateEntry *, const KCal::Event * );
	void setExceptions(PilotDateEntry *, const KCal::Event * );

} ;


#endif
