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
class Todo;
} ;

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
class PilotTodoEntry;

class TodoConduit : public ConduitAction
{
Q_OBJECT
public:
	TodoConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~TodoConduit();

	virtual void exec();

protected slots:
	/**
	* This function is called to sync modified records from the Pilot to KOrganizer.
	*/
	void syncRecord();
	/**
	* This function goes the other way around: KOrganizer -> Pilot.
	*/
	void syncTodo();
	void cleanup();
	void deleteRecord();

protected:
	void addRecord(PilotRecord *);
	void deleteRecord(PilotRecord *,PilotRecord *);
	void changeRecord(PilotRecord *,PilotRecord *);

	void deletePalmRecord(KCal::Todo*e, PilotRecord*s);
	void updateTodoOnPalm(KCal::Todo*e, PilotTodoEntry*de);
	//void addPalmRecord(KCal::Todo *e);

	KCal::Todo *todoFromRecord(KCal::Todo *, const PilotTodoEntry &);
	PilotRecord *entryFromTodo(PilotTodoEntry*de, const KCal::Todo*e);


	KCal::Todo *findTodo(recordid_t);

//	virtual const QString configGroup() const { return TodoConduitFactory::group;};
//	virtual const QString dbname() const { return "DatebookDB";};

protected:
	KCal::Calendar *fCalendar;
	PilotSerialDatabase *fCurrentDatabase;
	PilotLocalDatabase *fBackupDatabase;

	QString fCalendarFile;
	bool fFirstTime,fDeleteOnPilot, fFullSync;
	int pilotindex;

private:
	class VCalPrivate;
	VCalPrivate *fP;
} ;

// $Log$
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
