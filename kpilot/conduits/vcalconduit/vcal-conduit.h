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

// $Log$
// Revision 1.28  2002/08/24 18:06:51  kainhofe
// First sync no longer generates duplicates, addIncidence checks if a similar entry already exists
//
// Revision 1.27  2002/07/28 17:27:54  cschumac
// Move file loading/saving code from CalendarLocal to own class.
//
// Revision 1.26  2002/05/14 23:07:49  kainhofe
// Added the conflict resolution code. the Palm and PC precedence is currently swapped, and will be improved in the next few days, anyway...
//
// Revision 1.25  2002/05/01 21:18:23  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.20.2.2  2002/05/01 21:11:49  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.20.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.23  2002/04/22 22:51:51  kainhofe
// Added the first version of the todo conduit, fixed a check for a null pointer in the datebook conduit
//
// Revision 1.22  2002/04/19 19:10:29  kainhofe
// added some comments describin the sync logic, deactivated the sync again (forgot it when I commited last time)
//
// Revision 1.21  2002/04/14 22:18:16  kainhofe
// Implemented the second part of the sync (PC=>Palm), but disabled it, because it corrupts the Palm datebook
//
// Revision 1.20  2002/01/26 15:01:02  adridg
// Compile fixes and more
//
// Revision 1.19  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//

#endif
