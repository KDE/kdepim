/* vcal-conduit.cc                      KPilot
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

static const char *TodoConduit_id = "$Id$";

#include <options.h>
#include <unistd.h>

#include <qdatetime.h>
#include <qtimer.h>

#include <pilotUser.h>
#include <kconfig.h>

#include <calendar.h>
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
#include <pilotTodoEntry.h>

#include "todo-factory.h"
#include "todo-conduit.moc"

QDateTime readTm(const struct tm &t)
{
  QDateTime dt;
  dt.setDate(QDate(1900 + t.tm_year, t.tm_mon + 1, t.tm_mday));
  dt.setTime(QTime(t.tm_hour, t.tm_min, t.tm_sec));
  return dt;
}


struct tm writeTm(const QDateTime &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.date().year() - 1900;
  t.tm_mon = dt.date().month() - 1;
  t.tm_mday = dt.date().day();
  t.tm_hour = dt.time().hour();
  t.tm_min = dt.time().minute();
  t.tm_sec = dt.time().second();

  return t;
}


struct tm writeTm(const QDate &dt)
{
  struct tm t;

  t.tm_wday = 0; // unimplemented
  t.tm_yday = 0; // unimplemented
  t.tm_isdst = 0; // unimplemented

  t.tm_year = dt.year() - 1900;
  t.tm_mon = dt.month() - 1;
  t.tm_mday = dt.day();
  t.tm_hour = 0;
  t.tm_min = 0;
  t.tm_sec = 0;

  return t;
}


class TodoConduit::VCalPrivate
{
public:
	VCalPrivate(KCal::Calendar *buddy);

#ifdef KDE2
	QList<KCal::Todo> fAllTodos;
#else
	QPtrList<KCal::Todo> fAllTodos;
#endif

	int updateTodos();
	void removeTodo(KCal::Todo *);
	KCal::Todo *findTodo(recordid_t);
	KCal::Todo *getNextTodo();
	KCal::Todo *getNextModifiedTodo();

protected:
	bool reading;

private:
	KCal::Calendar *fCalendar;
} ;


TodoConduit::VCalPrivate::VCalPrivate(KCal::Calendar *b) :
	fCalendar(b)
{
	fAllTodos.setAutoDelete(false);
	reading=false;
}


int TodoConduit::VCalPrivate::updateTodos()
{
	fAllTodos = fCalendar->getTodoList();
	fAllTodos.setAutoDelete(false);
	return fAllTodos.count();
}


void TodoConduit::VCalPrivate::removeTodo(KCal::Todo *e)
{
	fAllTodos.remove(e);
	fCalendar->deleteTodo(e);
}


KCal::Todo *TodoConduit::VCalPrivate::findTodo(recordid_t id)
{
	KCal::Todo *todo = fAllTodos.first();
	while(todo)
	{
		if (todo->pilotId() == id) return todo;
		todo = fAllTodos.next();
	}

	return 0L;
}


KCal::Todo *TodoConduit::VCalPrivate::getNextTodo()
{
	if (reading) return fAllTodos.next();
	reading=true;
	return fAllTodos.first();
}


KCal::Todo *TodoConduit::VCalPrivate::getNextModifiedTodo()
{
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
	while (e && e->syncStatus()==KCal::Incidence::SYNCNONE)
	{
		e=fAllTodos.next();
	}
	return e;
}

/****************************************************************************
 *                          TodoConduit class                               *
 ****************************************************************************/

TodoConduit::TodoConduit(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) :
	ConduitAction(d,n,a),
	fCalendar(0L),
	fCurrentDatabase(0L),
	fBackupDatabase(0L),
	fP(0L)
{
	FUNCTIONSETUP;
	(void) TodoConduit_id;
}


TodoConduit::~TodoConduit()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP);
	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);
	KPILOT_DELETE(fCalendar);
}


/* There are several different scenarios for a record on the Palm and its PC counterpart
  N means a new record, M flags a modified record, D a deleted and - an unmodified record
  first is the Palm record, second the corresponding PC record
  (-,-)...unchanged, just sync if first time or full sync
  (N,-)...no rec matching the Palm ID in the backupDB/calendar yet => add KCal::Todo
  (M,-)...record is in backupDB, unchanged in calendar => modify in calendar and in backupDB
  (D,-)...deleted on Palm, exists in backupDB and calendar => just delete from calendar and backupDB
  (-,N)...no or invalid pilotID set for the KCal::Todo => just add to palm and backupDB
  (-,M)...valid PilotID set => just modify on Palm
  (-,D)...Record in backupDB, but not in calendar => delete from Palm and backupDB
  (N,N)...Can't find out (the two records are not correlated in any way, they just have the same data!!
  (M,M),(M,L),(L,M)...(Record exists on Palm and the Todo has the ID) CONFLICT, ask the user what to do
                      or use a config setting
  (L,L)...already deleted on both, no need to do anything.


   The sync process is as follows (for a fast sync):
	1) syncRecord goes through all records on Palm (just the modified one are necessary), find it
	   in the backupDB. The following handles ([NMD],*)
	   a) if it doesn't exist and was not deleted, add it to the calendar and the backupDB
	   b) if it exists and was not deleted,
			A) if it is unchanged in the calendar, just modify in the calendar
		c) if it exists and was deleted, delete it from the calendar if necessary
	2) syncTodo goes through all KCale::Todos in the calendar (just modified, this is the modification
	   time is later than the last sync time). This handles (-,N),(-,M)
		a) if it does not have a pilotID, add it to the palm and backupDB, store the PalmID
		b) if it has a valid pilotID, update the Palm record and the backup
	3) finally, deleteRecord goes through all records (which don't have the deleted flag) of the backup db
	   and if one does not exist in the Calendar, it was deleted there, so delete it from the Palm, too.
		This handles the last remaining case of (-,D)


In addition to the fast sync, where the last sync was done with this very PC and calendar file,
there are two special cases: a full and a first sync.
-) a full sync goes through all records, not just the modified ones. The pilotID setting of the calendar
   records is used to determine if the record already exists. if yes, the record is just modified
-) a first sync completely ignores the pilotID setting of the calendar todos. All records are added,
	so there might be duplicates. The add function for the calendar should check if a similar record already
	exists, but this is not done yet.


-) a full sync is done if
	a) there is a backupdb and a calendar, but the PC id number changed
	b) it was explicitely requested by pressing the full sync button in KPilot
	c) the setting "always full sync" was selected in the configuration dlg
-) a first sync is done if
	a) either the calendar or the backup DB does not exist.
	b) the calendar and the backup DB exists, but the sync is done for a different User name
	c) it was explicitely requested in KPilot

*/

/* virtual */ void TodoConduit::exec()
{
//	debug_level=0;
	FUNCTIONSETUP;

	bool loadSuccesful = true;
	KPilotUser*usr;

	if (!fConfig)
	{
		kdWarning() << k_funcinfo
			<< ": No configuration set for vcal-conduit"
			<< endl;
		goto error;
	}

	if (PluginUtility::isRunning("korganizer") ||
		PluginUtility::isRunning("alarmd"))
	{
		addSyncLogEntry(i18n("KOrganizer is running, can't update datebook."));
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": KOrganizer is running, can't update datebook."<<endl;
#endif
		emit syncDone(this);
		return;
	}

	fConfig->setGroup(ToDoConduitFactory::group);

	fCalendarFile = fConfig->readEntry(ToDoConduitFactory::calendarFile);
	fDeleteOnPilot = fConfig->readBoolEntry(ToDoConduitFactory::deleteOnPilot, false);
	// don't do a fist sync by default, only when explicitely requested, or the backup
	// database or the calendar are empty.
	fFirstTime = fConfig->readBoolEntry(ToDoConduitFactory::firstTime, false);
	usr=fHandle->getPilotUser();
	// changing the PC or using a different Palm Desktop app causes a full sync
	// User gethostid for this, since JPilot uses 1+(2000000000.0*random()/(RAND_MAX+1.0))
	// as PC_ID, so using JPilot and KPilot is the same as using two differenc PCs
	fFullSync = (fConfig->readBoolEntry(ToDoConduitFactory::alwaysFullSync, false) ||
		((usr->getLastSyncPC()!=(unsigned long) gethostid()) && fConfig->readBoolEntry(ToDoConduitFactory::fullSyncOnPCChange, true)) );

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Using calendar file "
		<< fCalendarFile
		<< endl;
#endif

	fCurrentDatabase = new PilotSerialDatabase(pilotSocket(),
		"ToDoDB",
		this,
		"ToDoDB");
	fBackupDatabase = new PilotLocalDatabase("ToDoDB");
	fCalendar = new KCal::CalendarLocal();


	// Handle lots of error cases.
	//
	fFullSync=(fBackupDatabase==0);
	if (!fCurrentDatabase || !fBackupDatabase || !fCalendar) goto error;
	if (!fCurrentDatabase->isDBOpen() ||
		!fBackupDatabase->isDBOpen()) goto error;
	loadSuccesful = fCalendar->load(fCalendarFile);
	fFullSync=!loadSuccesful;
// TODO: remove this:
fFullSync=true;
fFirstTime=false;
	if (!loadSuccesful) goto error;

	fP = new VCalPrivate(fCalendar);
	fP->updateTodos();

#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": fullsync="<<fFullSync<<", firstSync="<<fFirstTime<<endl;
#endif

	pilotindex=0;
	QTimer::singleShot(0,this,SLOT(syncRecord()));
	return;

error:
	if (!fCurrentDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open database on Pilot"
			<< endl;
	}
	if (!fBackupDatabase)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't open local copy"
			<< endl;
	}
	if (!loadSuccesful)
	{
		kdWarning() << k_funcinfo
			<< ": Couldn't load <"
			<< fCalendarFile
			<< ">"
			<< endl;
	}

	emit logError(i18n("Couldn't open the calendar databases."));

	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);
	KPILOT_DELETE(fCalendar);
	emit syncDone(this);
}


void TodoConduit::syncRecord()
{
	FUNCTIONSETUP;

	PilotRecord *r;
	if (fFirstTime || fFullSync)
	{
		r = fCurrentDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r = fCurrentDatabase->readNextModifiedRec();
	}
	PilotRecord *s = 0L;

	if (!r)
	{
		fP->updateTodos();
		QTimer::singleShot(0 ,this,SLOT(syncTodo()));
		return;
	}

	s = fBackupDatabase->readRecordById(r->getID());
	if (!s || (fFirstTime && !r->isDeleted()) )
	{
#ifdef DEBUG
		if (r->getID()>0)
		{
			DEBUGCONDUIT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGCONDUIT<< fname<<": Could not read palm record with ID "<<r->getID()<<endl;
		}
#endif
		addRecord(r);
	}
	else
	{
		if (r->isDeleted())
		{
			deleteRecord(r,s);
		}
		else
		{
			changeRecord(r,s);
		}
	}

	KPILOT_DELETE(r);
	KPILOT_DELETE(s);

	QTimer::singleShot(0,this,SLOT(syncRecord()));
}


void TodoConduit::syncTodo()
{
// TODO: skip PC => Palm sync for now because the date is off by an hour (dst). Also, duplicates appear on the PC
//QTimer::singleShot(0,this,SLOT(deleteRecord()));
//return;


	FUNCTIONSETUP;
	KCal::Todo*e=0L;
	if (fFirstTime || fFullSync) e=fP->getNextTodo();
	else e=fP->getNextModifiedTodo();

	if (!e)
	{
		pilotindex=0;
		debug_level=1;
		QTimer::singleShot(0,this,SLOT(deleteRecord()));
		return;
	}
	// find the corresponding index on the palm and sync. If there is none, create it.
	int ix=e->pilotId();
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": found PC entry with pilotID "<<ix<<endl;
#endif
	PilotRecord *s=0L;
	PilotTodoEntry*de;
	if (ix>0 && (s=fCurrentDatabase->readRecordById(ix)))
	{
		if (e->syncStatus()==KCal::Incidence::SYNCDEL)
		{
			deletePalmRecord(e, s);
		}
		else
		{
			de=new PilotTodoEntry(s);
			updateTodoOnPalm(e, de);
			delete de;
		}
	} else {
#ifdef DEBUG
		if (ix>0)
		{
			DEBUGCONDUIT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGCONDUIT<< fname<<": Could not read palm record with ID "<<ix<<endl;
		}
#endif
		de=new PilotTodoEntry();
		updateTodoOnPalm(e, de);
		delete de;
	}
	KPILOT_DELETE(s);
	QTimer::singleShot(0, this, SLOT(syncTodo()));
}


void TodoConduit::deleteRecord()
{
// TODO: This does not work currently yet (the Todos with the PilotID are not yet found?!?!?!) Also happens with the Palm->PC sync.
	QTimer::singleShot(0, this, SLOT(cleanup()));

	FUNCTIONSETUP;

	PilotRecord *r = fBackupDatabase->readRecordByIndex(pilotindex++);
	if (!r)
	{
		QTimer::singleShot(0 ,this,SLOT(cleanup()));
		return;
	}

	KCal::Todo *e = fP->findTodo(r->getID());
	if (!e)
	{
		// entry was deleted from Calendar, so delete it from the palm
		PilotRecord*s=fBackupDatabase->readRecordById(r->getID());
		if (s)
		{
			// delete the record from the palm
			// TODO: 1) Really delete the record, or just flag it deleted???
			//       2) Should use a method of PilotSerialDatabase for the deleting
//			dlp_DeleteRecord(fHandle->pilotSocket(), int dbhandle, int all, recordid_t recID);
			s->setAttrib(~dlpRecAttrDeleted);
			fCurrentDatabase->writeRecord(s);
			KPILOT_DELETE(s);
		}
		r->setAttrib(~dlpRecAttrDeleted);
		fBackupDatabase->writeRecord(r);
	}

			KPILOT_DELETE(r);
	QTimer::singleShot(0,this,SLOT(deleteRecord()));
}


void TodoConduit::cleanup()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);

	fCalendar->save(fCalendarFile);
	KPILOT_DELETE(fCalendar);

	emit syncDone(this);
}


void TodoConduit::addRecord(PilotRecord *r)
{
	FUNCTIONSETUP;

	recordid_t id=fBackupDatabase->writeRecord(r);
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": Pilot Record ID="<<r->getID()<<", backup ID="<<id<<endl;
#endif

	PilotTodoEntry de(r);
	KCal::Todo *e = new KCal::Todo;
	todoFromRecord(e,de);
	// TODO: find out if there is already an entry with this data...

	fCalendar->addTodo(e);
}


void TodoConduit::deleteRecord(PilotRecord *r, PilotRecord *s)
{
	FUNCTIONSETUP;

	KCal::Todo *e = fP->findTodo(r->getID());
	if (e)
	{
		// RemoveTodo also takes it out of the calendar.
		fP->removeTodo(e);
	}
	fBackupDatabase->writeRecord(r);
}


void TodoConduit::changeRecord(PilotRecord *r,PilotRecord *s)
{
	FUNCTIONSETUP;

	PilotTodoEntry de(r);
	KCal::Todo *e = fP->findTodo(r->getID());
	if (e)
	{
		todoFromRecord(e,de);
		fBackupDatabase->writeRecord(r);
	}
	else
	{
		kdWarning() << k_funcinfo
			<< ": While changing record -- not found in iCalendar"
			<< endl;

		addRecord(r);
	}
}


void TodoConduit::deletePalmRecord(KCal::Todo*e, PilotRecord*s)
{
	FUNCTIONSETUP;
	if (s)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": deleting record " << s->getID() << endl;
#endif
		s->setAttrib(~dlpRecAttrDeleted);
		fCurrentDatabase->writeRecord(s);
		fBackupDatabase->writeRecord(s);
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": could not find record to delete (" << e->pilotId() << ")" << endl;
#endif
	}
}


/* I have to use a pointer to an existing PilotTodoEntry so that I can handle
   new records as well (and to prevend some crashes concerning the validity
   domain of the PilotRecord*r). In syncTodo this PilotTodoEntry is created. */
void TodoConduit::updateTodoOnPalm(KCal::Todo*e, PilotTodoEntry*de)
{
	FUNCTIONSETUP;
	if (!de) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL todo given... Skipping it"<<endl;
#endif
		return;
	}
	PilotRecord*r=entryFromTodo(de, e);

	if (r)
	{
		recordid_t id=fCurrentDatabase->writeRecord(r);
		r->setID(id);
		fBackupDatabase->writeRecord(r);
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		e->setPilotId(id);
	}
}


PilotRecord*TodoConduit::entryFromTodo(PilotTodoEntry*de, const KCal::Todo*todo)
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

	de->setPriority(todo->priority());

	de->setComplete(todo->isCompleted());

	// what we call summary pilot calls description.
	de->setDescription(todo->summary());

	// what we call description pilot puts as a separate note
	de->setNote(todo->description());

DEBUGCONDUIT<<"-------- "<<todo->summary()<<endl;
	return de->pack();
}


KCal::Todo *TodoConduit::todoFromRecord(KCal::Todo *e, const PilotTodoEntry &de)
{
	FUNCTIONSETUP;

	KCal::Todo*vtodo=e;
	if (!vtodo)
	{
		// no event was found, so we need to add one with some initial info
		// TODO: does this make sense and do I really have to add the todo, or just exit out of this function?
		vtodo = new KCal::Todo;
		fCalendar->addTodo(vtodo);
//		calendar()->addTodo(vtodo);
		vtodo->setPilotId(de.getID());
		vtodo->setSyncStatus(KCal::Incidence::SYNCNONE);
	}

	e->setOrganizer(fCalendar->getEmail());
	e->setPilotId(de.getID());
	e->setSyncStatus(KCal::Incidence::SYNCNONE);
	e->setSecrecy(de.isSecret() ? KCal::Todo::SecrecyPrivate : KCal::Todo::SecrecyPublic);

	// we don't want to modify the vobject with pilot info, because it has
	// already been  modified on the desktop.  The VObject's modified state
	// overrides the PilotRec's modified state.
	// TODO: Also include this in the vcal conduit!!!
	if (e->syncStatus() != KCal::Incidence::SYNCNONE) return e;

	// otherwise, the vObject hasn't been touched.  Updated it with the
	// info from the PilotRec.
	if (de.getIndefinite()) {
		e->setHasDueDate(false);
	} else {
		e->setDtDue(readTm(de.getDueDate()));
	}

	// PRIORITY //
	e->setPriority(de.getPriority());

	// COMPLETED? //
	e->setCompleted(de.getComplete());

	e->setSummary(de.getDescription());
	e->setDescription(de.getNote());

	e->setSyncStatus(KCal::Incidence::SYNCNONE);

	return e;
}



// $Log$
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

