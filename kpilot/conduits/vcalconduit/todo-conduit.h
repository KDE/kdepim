#ifndef _KPILOT_TODO_CONDUIT_H
#define _KPILOT_TODO_CONDUIT_H
/* todo-conduit.h                       KPilot
**
** Copyright (C) 2002 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
**
** This file is part of the todo conduit, a conduit for KPilot that
** synchronises the Pilot's todo application with the outside world,
** which currently means KOrganizer.
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

#include <todo.h>
#include <calendar.h>
#include <pilotTodoEntry.h>
//#include <pilotRecord.h>
#include "todo-factory.h"
#include "vcal-conduitbase.h"

class PilotRecord;
class PilotSerialDatabase;
class PilotLocalDatabase;
//class PilotAppCategory;
//class PilotDateEntry: public PilotAppCategory;
//class VCalConduitFactory;

//class VCalConduitBase;

class TodoConduitPrivate : public VCalConduitPrivateBase
{
public:
	TodoConduitPrivate(KCal::Calendar *buddy);
	virtual ~TodoConduitPrivate() {};

#ifdef KDE2
	QList<KCal::Todo> fAllTodos;
#else
	QPtrList<KCal::Todo> fAllTodos;
#endif

	virtual int updateIncidences();
	virtual void addIncidence(KCal::Incidence*);
	virtual void removeIncidence(KCal::Incidence *);
	virtual KCal::Incidence *findIncidence(recordid_t);
	virtual KCal::Incidence *getNextIncidence();
	virtual KCal::Incidence *getNextModifiedIncidence();
	virtual int count() {return fAllTodos.count();};
} ;



class TodoConduit : public VCalConduitBase
{
Q_OBJECT
//protected:
//	class VCalPrivate;
public:
	TodoConduit(KPilotDeviceLink *,
		const char *name=0L,
		const QStringList &args = QStringList());
	virtual ~TodoConduit();
   
protected:
	virtual const QString getTitle(PilotAppCategory*de);
	
	virtual const QString configGroup() { return ToDoConduitFactory::group; };
	virtual const QString dbname() { return "ToDoDB"; };

   virtual VCalConduitPrivateBase* newVCalPrivate(KCal::Calendar *fCalendar) { return new TodoConduitPrivate(fCalendar);};


	virtual PilotAppCategory*newPilotEntry(PilotRecord*r) {FUNCTIONSETUP; if (r) return new PilotTodoEntry(r); else return new PilotTodoEntry;};
	virtual KCal::Incidence*newIncidence() { return new KCal::Todo; };

protected:

	PilotRecord *recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e);
	PilotRecord *recordFromIncidence(PilotTodoEntry*de, const KCal::Todo*e);
	KCal::Incidence *incidenceFromRecord(KCal::Incidence *, const PilotAppCategory *);
	KCal::Todo *incidenceFromRecord(KCal::Todo *, const PilotTodoEntry *);

} ;

// $Log$
// Revision 1.7  2002/05/01 21:18:23  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.4.2.2  2002/05/01 21:11:49  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.4.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.5  2002/04/22 22:51:51  kainhofe
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
