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

// define conduit versions, one for the version when categories were synced for the first time, and the current version number
#define CONDUIT_VERSION_CATEGORYSYNC 10
#define CONDUIT_VERSION 10



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
#if KDE_VERSION >= 305
	fAllTodos = fCalendar->todos();
#else
	fAllTodos = fCalendar->getTodoList();
#endif
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
	FUNCTIONSETUP;
	VCalConduitBase::readConfig();
	// determine if the categories have ever been synce. Needed to prevent loosing the categories on the desktop.
	// also use a full sync for the first time to make sure the palm categories are really transferred to the desktop
	categoriesSynced = fConfig->readNumEntry("ConduitVersion", 0)>=CONDUIT_VERSION_CATEGORYSYNC;
	if (!categoriesSynced) fFullSync=true;
#ifdef DEBUG
	DEBUGCONDUIT<<"categoriesSynced="<<categoriesSynced<<endl;
#endif
}



void TodoConduit::postSync()
{
	FUNCTIONSETUP;
	VCalConduitBase::postSync();
	fConfig->setGroup(configGroup());
	// after this successful sync the categories have been synced for sure
	fConfig->writeEntry("ConduitVersion", CONDUIT_VERSION);
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

	setCategory(de, todo);

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



void TodoConduit::preRecord(PilotRecord*r) 
{
	FUNCTIONSETUP;
	if (!categoriesSynced && r) 
	{
		const PilotAppCategory*de=newPilotEntry(r);
		KCal::Incidence *e = fP->findIncidence(r->getID());
		setCategory(dynamic_cast<KCal::Todo*>(e), dynamic_cast<const PilotTodoEntry*>(de));
	}
}
 
 

void TodoConduit::setCategory(PilotTodoEntry*de, const KCal::Todo*todo)
{
	if (!de || !todo) return;
	de->setCat(_getCat(de->getCat(), todo->categories()));
#ifdef DEBUG
	DEBUGCONDUIT<<"old Category="<<de->getCat()<<", new cat will be "<<_getCat(de->getCat(), todo->categories())<<endl;
	DEBUGCONDUIT<<"Available Categories: "<<todo->categories().join(" - ")<<endl;
#endif
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
	setCategory(e, de);
	
	// PRIORITY //
	e->setPriority(de->getPriority());

	// COMPLETED? //
	e->setCompleted(de->getComplete());

	e->setSummary(de->getDescription());
	e->setDescription(de->getNote());

	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return e;
}



void TodoConduit::setCategory(KCal::Todo *e, const PilotTodoEntry *de)
{
	if (!e || !de) return;
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
}


// $Log$
// Revision 1.21  2002/08/20 20:49:11  adridg
// Make sure the HEAD code compiles under KDE 3.0.x too, wrt. libkcal changes
//
// Revision 1.20  2002/08/15 10:47:56  kainhofe
// Finished categories syncing for the todo conduit
//
// Revision 1.19  2002/07/28 17:27:54  cschumac
// Move file loading/saving code from CalendarLocal to own class.
//
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

