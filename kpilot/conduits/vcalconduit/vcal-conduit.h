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
	void syncRecord();
	void cleanup();

protected:
	void addRecord(PilotRecord *);
	void deleteRecord(PilotRecord *,PilotRecord *);
	void changeRecord(PilotRecord *,PilotRecord *);

	KCal::Event *eventFromRecord(const PilotDateEntry &);

	void setStartEndTimes(KCal::Event *,const PilotDateEntry &);
	void setAlarms(KCal::Event *,const PilotDateEntry &);
	void setRecurrence(KCal::Event *,const PilotDateEntry &);
	void setExceptions(KCal::Event *,const PilotDateEntry &);

	KCal::Event *findEvent(recordid_t);

protected:
	KCal::Calendar *fCalendar;
	PilotSerialDatabase *fCurrentDatabase;
	PilotLocalDatabase *fPreviousDatabase;

	QString fCalendarFile;
	bool fFirstTime,fDeleteOnPilot;
} ;

// $Log: $

#endif
