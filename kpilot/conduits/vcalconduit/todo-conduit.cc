/* todo-conduit.cc  Todo-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 2002 Reinhold Kainhofer
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
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

static const char *TodoConduit_id = "$Id$";

#include <options.h>
#include <unistd.h>

#include <qdatetime.h>
#include <qtimer.h>

#include <pilotUser.h>
#include <kconfig.h>

#include <calendarlocal.h>
#include <todo.h>


/*
** KDE 2.2 uses class KORecurrence in a different header file.
*/
#ifdef KDE2
#define DateList_t QDateList
#define DateListIterator_t QDateListIterator
#else
#define DateList_t KCal::DateList
#define DateListIterator_t KCal::DateList::ConstIterator
#endif

#include <pilotSerialDatabase.h>
#include <pilotLocalDatabase.h>
//#include <pilotTodoEntry.h>

//#include "vcal-conduitbase.h"
//#include "todo-factory.h"
#include "todo-conduit.moc"




TodoConduitPrivate::TodoConduitPrivate(KCal::CalendarLocal *b) :
	VCalConduitPrivateBase(b)
{
	fAllTodos.setAutoDelete(false);
}

void TodoConduitPrivate::addIncidence(KCal::Incidence*e)
{
	fAllTodos.append(static_cast<KCal::Todo*>(e));
	fCalendar->addTodo(static_cast<KCal::Todo*>(e));
}

int TodoConduitPrivate::updateIncidences()
{
	fAllTodos = fCalendar->todos();
	fAllTodos.setAutoDelete(false);
	return fAllTodos.count();
}


void TodoConduitPrivate::removeIncidence(KCal::Incidence *e)
{
	fAllTodos.remove(static_cast<KCal::Todo*>(e));
	fCalendar->deleteTodo(static_cast<KCal::Todo*>(e));
}


KCal::Incidence *TodoConduitPrivate::findIncidence(recordid_t id)
{
	KCal::Todo *todo = fAllTodos.first();
	while(todo)
	{
		if ((recordid_t)(todo->pilotId()) == id) return todo;
		todo = fAllTodos.next();
	}

	return 0L;
}


KCal::Incidence *TodoConduitPrivate::getNextIncidence()
{
	if (reading) return fAllTodos.next();
	reading=true;
	return fAllTodos.first();
}


KCal::Incidence *TodoConduitPrivate::getNextModifiedIncidence()
{
FUNCTIONSETUP;
	KCal::Todo*e=0L;
	if (!reading)
	{
		reading=true;
		e=fAllTodos.first();
	}
	else
	{
		e=fAllTodos.next();
	}
	while (e && e->syncStatus()!=KCal::Incidence::SYNCMOD)
	{
		e=fAllTodos.next();
#ifdef DEBUG
if (e)
DEBUGCONDUIT<< e->summary()<<" had SyncStatus="<<e->syncStatus()<<endl;
#endif
	}
	return e;
}



/****************************************************************************
 *                          TodoConduit class                               *
 ****************************************************************************/

TodoConduit::TodoConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) : VCalConduitBase(d,n,a)
{
	FUNCTIONSETUP;
	(void) TodoConduit_id;
}


TodoConduit::~TodoConduit()
{
//	FUNCTIONSETUP;
}

void TodoConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer =
		new unsigned char[PilotTodoEntry::APP_BUFFER_SIZE];
	int appLen = fDatabase->readAppBlock(buffer,PilotTodoEntry::APP_BUFFER_SIZE);

	unpack_ToDoAppInfo(&fTodoAppInfo, buffer, appLen);
	delete[]buffer;
	buffer = NULL;

#ifdef DEBUG
	DEBUGCONDUIT << fname << " lastUniqueId"
		<< fTodoAppInfo.category.lastUniqueID << endl;
#endif
	for (int i = 0; i < 16; i++)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " cat " << i << " =" <<
			fTodoAppInfo.category.name[i] << endl;
#endif
	}

}



const QString TodoConduit::getTitle(PilotAppCategory*de)
{
	PilotTodoEntry*d=dynamic_cast<PilotTodoEntry*>(de);
	if (d) return QString(d->getDescription());
	return "";
}

void TodoConduit::readConfig()
{
	VCalConduitBase::readConfig();
	// determine if the categories have ever been synce. Needed to prevent loosing the categories on the desktop.
	// also use a full sync for the first time to make sure the palm categories are really transferred to the desktop
	categoriesSynced = fConfig->readBoolEntry("Categories already synced");
	if (!categoriesSynced) fFullSync=true;
}

void TodoConduit::postSync()
{
	VCalConduitBase::postSync();
	fConfig->setGroup(configGroup());
	// after this successful sync the categories have been synced for sure
	fConfig->writeEntry("Categories already synced", true);
}

PilotRecord*TodoConduit::recordFromIncidence(PilotAppCategory*de, const KCal::Incidence*e)
{
	// don't need to check for null pointers here, the recordFromIncidence(PTE*, KCal::Todo*) will do that.
	return recordFromIncidence(dynamic_cast<PilotTodoEntry*>(de), dynamic_cast<const KCal::Todo*>(e));
}

PilotRecord*TodoConduit::recordFromIncidence(PilotTodoEntry*de, const KCal::Todo*todo)
{
	FUNCTIONSETUP;
	if (!de || !todo) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL todo given... Skipping it"<<endl;
#endif
		return NULL;
	}

	// set secrecy, start/end times, alarms, recurrence, exceptions, summary and description:
	if (todo->secrecy()!=KCal::Todo::SecrecyPublic) de->makeSecret();

	// update it from the iCalendar Todo.

	if (todo->hasDueDate()) {
		struct tm t = writeTm(todo->dtDue());
		de->setDueDate(t);
		de->setIndefinite(0);
	} else {
		de->setIndefinite(1);
	}
	
	// TODO: take recurrence (code in VCAlConduit) from ActionNames
	// TODO: take categories from the pilot
	de->setCat(_getCat(de->getCat(), todo->categories()));
#ifdef DEBUG
	DEBUGCONDUIT<<"old Category="<<de->getCat()<<", new cat will be "<<_getCat(de->getCat(), todo->categories())<<endl;
	DEBUGCONDUIT<<"Available Categories: "<<todo->categories().join(" - ")<<endl;
#endif
	
	// TODO: sync the alarm from ActionNames. Need to extend PilotTodoEntry
	de->setPriority(todo->priority());

	de->setComplete(todo->isCompleted());

	// what we call summary pilot calls description.
	de->setDescription(todo->summary());

	// what we call description pilot puts as a separate note
	de->setNote(todo->description());

#ifdef DEBUG
DEBUGCONDUIT<<"-------- "<<todo->summary()<<endl;
#endif
	return de->pack();
}


/** 
 * _getCat returns the id of the category from the given categories list. If none of the categories exist 
 * on the palm, the "Nicht abgelegt" (don't know the english name) is used.
 */
int TodoConduit::_getCat(int cat, const QStringList cats) const
{
	FUNCTIONSETUP;
	int j;
	if (cats.contains(fTodoAppInfo.category.name[cat])) 
		return cat;
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it ) {
		for (j=1; j<=15; j++) 
		{
			if (!(*it).isEmpty() && ! (*it).compare( fTodoAppInfo.category.name[j] ) ) 
			{
				return j;
			}
		}
	}
	return 0;
}

KCal::Incidence *TodoConduit::incidenceFromRecord(KCal::Incidence *e, const PilotAppCategory *de)
{
	return dynamic_cast<KCal::Incidence*>(incidenceFromRecord(dynamic_cast<KCal::Todo*>(e), dynamic_cast<const PilotTodoEntry*>(de)));
}


KCal::Todo *TodoConduit::incidenceFromRecord(KCal::Todo *e, const PilotTodoEntry *de)
{
	FUNCTIONSETUP;

	KCal::Todo*vtodo=e;
	if (!vtodo)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": null todo entry given. skipping..."<<endl;
#endif
		return NULL;
	}

	e->setOrganizer(fCalendar->getEmail());
	e->setPilotId(de->getID());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de->isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic);

	// we don't want to modify the vobject with pilot info, because it has
	// already been  modified on the desktop.  The VObject's modified state
	// overrides the PilotRec's modified state.
	// TODO: Also include this in the vcal conduit!!!
//	if (e->syncStatus() != KCal::Incidence::SYNCNONE) return e;

	// otherwise, the vObject hasn't been touched.  Updated it with the
	// info from the PilotRec.
	if (de->getIndefinite()) {
		e->setHasDueDate(false);
	} else {
		e->setDtDue(readTm(de->getDueDate()));
		e->setHasDueDate(true);
	}
	
	// Categories
	// TODO: Sync categories
	// first remove all categories and then add only the appropriate one
	QStringList cats=e->categories();
	if (!categoriesSynced)
	{
		// TODO: This is not optimal because it has the consequence that only one of the
		// palm categories can be set on the desktop, all others will be deleted from the desktop entry!
		for (int j=1; j<=15; j++) 
		{
			cats.remove(fTodoAppInfo.category.name[j]);
		}
	}
	int cat=de->getCat();
	if (0<cat && cat<=15) 
	{
		cats.append( fTodoAppInfo.category.name[cat] );
	}
	e->setCategories(cats);

	// PRIORITY //
	e->setPriority(de->getPriority());

	// COMPLETED? //
	e->setCompleted(de->getComplete());

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());

	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return e;
}



// $Log$
// Revision 1.18  2002/07/23 00:45:18  kainhofe
// Fixed several bugs with recurrences.
//
// Revision 1.17  2002/07/20 17:40:34  cschumac
// Renamed Calendar::getTodoList() to Calendar::todos().
// Removed get prefix from Calendar functions returning todos.
//
// Revision 1.16  2002/07/09 22:38:04  kainhofe
// Implemented a first (not-yet-functional) version of the category sync
//
// Revision 1.15  2002/06/12 22:11:17  kainhofe
// Proper cleanup, libkcal still has some problems marking records modified on loading
//
// Revision 1.14  2002/05/14 23:07:49  kainhofe
// Added the conflict resolution code. the Palm and PC precedence is currently swapped, and will be improved in the next few days, anyway...
//
// Revision 1.13  2002/05/01 21:18:23  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.10.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
//
// Revision 1.11  2002/04/22 22:51:51  kainhofe
// Added the first version of the todo conduit, fixed a check for a null pointer in the datebook conduit
//
// Revision 1.62  2002/04/21 17:39:01  kainhofe
// recurrences without enddate work now
//
// Revision 1.61  2002/04/21 17:07:12  kainhofe
// Fixed some memory leaks, old alarms and exceptions are deleted before new are added, Alarms are now correct
//
// Revision 1.60  2002/04/20 18:05:50  kainhofe
// No duplicates any more in the calendar
//
// Revision 1.59  2002/04/20 17:38:02  kainhofe
// recurrence now correctly written to the palm, no longer crashes
//
// Revision 1.58  2002/04/20 14:21:26  kainhofe
// Alarms are now written to the palm. Some bug fixes, extensive testing. Exceptions still crash the palm ;-(((
//
// Revision 1.57  2002/04/19 19:34:11  kainhofe
// didn't compile
//
// Revision 1.56  2002/04/19 19:10:29  kainhofe
// added some comments describin the sync logic, deactivated the sync again (forgot it when I commited last time)
//
// Revision 1.55  2002/04/17 20:47:04  kainhofe
// Implemented the alarm sync
//
// Revision 1.54  2002/04/17 00:28:11  kainhofe
// Removed a few #ifdef DEBUG clauses I had inserted for debugging purposes
//
// Revision 1.53  2002/04/16 23:40:36  kainhofe
// Exceptions no longer crash the daemon, recurrences are correct now, end date is set correctly. Problems: All todos are off 1 day, lots of duplicates, exceptions are duplicate, too.
//
// Revision 1.52  2002/04/14 22:18:16  kainhofe
// Implemented the second part of the sync (PC=>Palm), but disabled it, because it corrupts the Palm datebook
//
// Revision 1.51  2002/02/23 20:57:41  adridg
// #ifdef DEBUG stuff
//
// Revision 1.50  2002/01/26 15:01:02  adridg
// Compile fixes and more
//
// Revision 1.49  2002/01/25 21:43:12  adridg
// ToolTips->WhatsThis where appropriate; vcal conduit discombobulated - it doesn't eat the .ics file anymore, but sync is limited; abstracted away more pilot-link
//

