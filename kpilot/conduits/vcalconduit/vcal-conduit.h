#ifndef _KPILOT_VCAL_CONDUIT_H
#define _KPILOT_VCAL_CONDUIT_H
/* vcal-conduit.h                       KPilot
**
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

namespace KCal
{
class Calendar;
class Event;
} ;

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotDateEntry;

class VCalConduit : public ConduitAction
{
Q_OBJECT
public:
	VCalConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~VCalConduit();

	virtual void exec();

protected slots:
	/**
	* This function is called to sync modified records from the Pilot to KOrganizer.
	*/
	void syncRecord();
	/**
	* This function goes the other way around: KOrganizer -> Pilot.
	*/
	void syncEvent();
	void cleanup();
	void deleteRecord();

protected:
	void addRecord(PilotRecord *);
	void deleteRecord(PilotRecord *,PilotRecord *);
	void changeRecord(PilotRecord *,PilotRecord *);

	void deletePalmRecord(KCal::Event*e, PilotRecord*s);
	void updateEventOnPalm(KCal::Event*e, PilotDateEntry*de);
	//void addPalmRecord(KCal::Event *e);

	KCal::Event *eventFromRecord(KCal::Event *, const PilotDateEntry &);
	PilotRecord *entryFromEvent(PilotDateEntry*de, const KCal::Event*e);

	void setStartEndTimes(KCal::Event *,const PilotDateEntry &);
	void setAlarms(KCal::Event *,const PilotDateEntry &);
	void setRecurrence(KCal::Event *,const PilotDateEntry &);
	void setExceptions(KCal::Event *,const PilotDateEntry &);

	void setStartEndTimes(PilotDateEntry *, const KCal::Event * );
	void setAlarms(PilotDateEntry *, const KCal::Event * );
	void setRecurrence(PilotDateEntry *, const KCal::Event * );
	void setExceptions(PilotDateEntry *, const KCal::Event * );

	KCal::Event *findEvent(recordid_t);

protected:
	KCal::Calendar *fCalendar;
	PilotSerialDatabase *fCurrentDatabase;
	PilotLocalDatabase *fBackupDatabase;

	QString fCalendarFile;
	bool fFirstTime,fDeleteOnPilot;
	int pilotindex;

private:
	class VCalPrivate;
	VCalPrivate *fP;
} ;

// $Log$
// Revision 1.20  2002/01/26 15:01:02  adridg
// Compile fixes and more
//
// Revision 1.19  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//

#endif
