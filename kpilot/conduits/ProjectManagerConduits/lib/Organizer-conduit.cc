/* Organizer-conduit.cc  Organizer-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file is part of the Organizer conduit, a conduit for KPilot that
** synchronises the Pilot's Organizer application with the outside world,
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

#include "options.h"
#include <stdio.h>
#include <stdlib.h>

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include <qtimer.h>
#include <kconfig.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <pilotSerialDatabase.h>
#include <calendarlocal.h>
#include "Organizer-conduit.h"
#include "DatabaseAction.h"

using namespace KCal;

static const char *Organizer_conduit_id = "$Id$";


/*****************************************************************
 *          OrgConduit::VCalPrivate helper class                 *
 *****************************************************************/

class OrganizerConduit::VCalPrivate {
private:
	bool reading;
	KCal::Calendar*fCalendar;
public:
	VCalPrivate(KCal::Calendar *buddy);
	#ifdef KDE2
		QList<KCal::Todo> fAllTodos;
	#else
		QPtrList<KCal::Todo> fAllTodos;
	#endif
	int updateTodos();
	int count() {return fAllTodos.count();};
	void addTodo(KCal::Todo*);
	void removeTodo(KCal::Todo*);
	void insertTodo(KCal::Todo*);
	KCal::Todo *findTodo(recordid_t);
	KCal::Todo *getNextTodo();
	KCal::Todo *getNextModifiedTodo();
};



OrganizerConduit::VCalPrivate::VCalPrivate(KCal::Calendar *b) : fCalendar(b) {
	fAllTodos.setAutoDelete(false);
}

void OrganizerConduit::VCalPrivate::addTodo(KCal::Todo*e)
{
	fAllTodos.append(e);
	fCalendar->addTodo(e);
}

int OrganizerConduit::VCalPrivate::updateTodos() {
	fAllTodos=fCalendar->getTodoList();
	fAllTodos.setAutoDelete(false);
	return fAllTodos.count();
}

void OrganizerConduit::VCalPrivate::removeTodo(KCal::Todo*e) {
	fAllTodos.remove(e);
	fCalendar->deleteTodo(e);
}

void OrganizerConduit::VCalPrivate::insertTodo(KCal::Todo*e) {
	fAllTodos.append(e);
}

KCal::Todo*OrganizerConduit::VCalPrivate::findTodo(recordid_t id) {
	KCal::Todo *todo = fAllTodos.first();
	while (todo) {
		if ((recordid_t)todo->pilotId()==id) return todo;
		todo=fAllTodos.next();
	}
	return 0L;
}

KCal::Todo *OrganizerConduit::VCalPrivate::getNextTodo()
{
	if (reading) return fAllTodos.next();
	reading=true;
	return fAllTodos.first();
}

KCal::Todo *OrganizerConduit::VCalPrivate::getNextModifiedTodo()
{
	KCal::Todo*e=0L;
	if (!reading) {
		reading=true;
		e=fAllTodos.first();
	} else {
		e=fAllTodos.next();
	}
	while (e && e->syncStatus()==KCal::Todo::SYNCNONE) {
		e=fAllTodos.next();
	}
	return e;
}


/*****************************************************************************
 *                 OrganizerConduit main class                        *
 *****************************************************************************/
OrganizerConduit::OrganizerConduit(KPilotDeviceLink *d, const char *n, const QStringList &l, SyncTypeList_t *tps) :
		MultiDBConduit(d,n,l,tps), fCalendar(0L), fP(0L) {
	FUNCTIONSETUP;
	for (int i=0; i<7; i++) { levelparent[i]=0;}
	previd=0;
	(void)Organizer_conduit_id;
}


OrganizerConduit::~OrganizerConduit() {
	FUNCTIONSETUP;
	cleanup();
}

bool OrganizerConduit::exec() {
	FUNCTIONSETUP;
	DEBUGCONDUIT<<Organizer_conduit_id<<endl;
	KConfig korgcfg( locate( "config", "korganizerrc" ) );
	QString tz;
	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	tz = korgcfg.readEntry( "TimeZoneId" );
#ifdef DEBUG
	DEBUGCONDUIT << fname<<": KOrganizer's time zone = "<<tz<<endl;
#endif

	return MultiDBConduit::exec();
}

void OrganizerConduit::cleanup() {
	FUNCTIONSETUP;
	MultiDBConduit::cleanup();
	KPILOT_DELETE(fP);
}

void OrganizerConduit::cleanupDB() {
	FUNCTIONSETUP;
	MultiDBConduit::cleanupDB();
	KPILOT_DELETE(fCalendar);
}


void OrganizerConduit::syncNextRecord() {
	PilotRecord*rec=fCurrentDatabase->readRecordByIndex(Palmix);
	KCal::Todo*todo=fP->fAllTodos.at(PCix);

	// if any of them could not be found, we are at the end, just add the rest:
	if (!rec) {
		while ( (todo=fP->fAllTodos.at(PCix++)) ) {
			insertRecordToPalm(Palmix++, todo);
		}
		QTimer::singleShot(0, this, SLOT(finishedDB()));
		return;
	}
	if (!todo) {
		while ( (rec=fCurrentDatabase->readRecordByIndex(Palmix++)) ) {
			insertRecordToPC(PCix++, rec);
		}
		QTimer::singleShot(0, this, SLOT(finishedDB()));
		return;
	}


	// Check if the two items are the same and only need to be updated but not moved:
	if ((recordid_t)rec->getID()==(recordid_t)todo->pilotId()) {
		/* There is just one case that both the current palm and pc entries are
		   in the correct order. For this the following needs to hold:
		   1) neither the palm nor the pc entry have been changed
		   2) the pc and palm entry are the same (check the palm ID)
		   3) if the entries are numbered, no entries have been inserted on the palm by the sync */
		// if any of them does not hold, update the two entries:
		if (  !(todo->syncStatus()==Incidence::SYNCNONE) ||
		      (rec->getAttrib() & dlpRecAttrDirty) ||
		      (inserted && (flags()&FLG_NUMBERED)) ) {
			updateRecords(Palmix, rec, PCix, todo);
		}
		PCix++; Palmix++;
		QTimer::singleShot(0, this, SLOT(syncNextRecord()));
		return;
	}

	// The two records don't describe the same entry so find out what to insert/move:
	// First, find the corresponding entry in the PC db:
	KCal::Todo*t=fP->findTodo(rec->getID());
	if (!t) { // there is not item on the PC, so insert
		insertRecordToPC(PCix++, rec);
		QTimer::singleShot(0, this, SLOT(syncNextRecord()));
		return;
	}

	// if there are only unsynced entries before the next synced PC entry, insert them on the palm:
	if ((recordid_t)todo->pilotId()==0)
	while ((recordid_t)todo->pilotId() ==0) {
		insertRecordToPalm(Palmix++, todo);
		if (!(todo=fP->fAllTodos.at(PCix++))) {
			QTimer::singleShot(0, this, SLOT(syncNextRecord()));
			return;
		}
	}
	// the next PC item has a SyncID, so find which has been changed and accordingly move/update:
	// if the PC entry has been removed on the palm, remove it from the PC, too
	PilotRecord*rec1=fCurrentDatabase->readRecordById(todo->pilotId());
	if (!rec1) {
		fP->removeTodo(todo);
		QTimer::singleShot(0, this, SLOT(syncNextRecord()));
		return;
	}
	if ((t->syncStatus()==Incidence::SYNCNONE) || (rec->getAttrib() & dlpRecAttrDirty)) {
		// PC entry was not changed, so use position on Palm
		movePCRecord(fP->fAllTodos.find(t), PCix);
		updateRecords(Palmix, rec, PCix, t);
		Palmix++; PCix++;
		QTimer::singleShot(0, this, SLOT(syncNextRecord()));
		return;
	}
	// Palm entry was not changed, only the PC entry, so move the Palm entry and stay at the same position
	movePalmRecord(Palmix, Palmix + (fP->fAllTodos.find(t)-Palmix));
	QTimer::singleShot(0, this, SLOT(syncNextRecord()));
	return;
}

void OrganizerConduit::updateRecords(int pid, PilotRecord*rec, int  pcid, KCal::Todo*todo) {
	// TODO: merge the two records in a sensitive way and write them out.
}

void OrganizerConduit::insertRecordToPC(int pos, PilotRecord*rec) {
	PilotOrganizerEntry*poe=createOrganizerEntry(rec);
	KCal::Todo*todo=poe->getTodo();
	fP->fAllTodos.insert(pos, todo);
	delete poe;
}
void OrganizerConduit::insertRecordToPalm(int pos, KCal::Todo*todo) {
	PilotOrganizerEntry*poe=createOrganizerEntry(todo);
	poe->setNumber(pos);
	PilotRecord*rec=poe->pack();
	// TODO!!!
//	fCurrentDatabase->insertRecord(pos, rec);
	KPILOT_DELETE(rec);
}

void OrganizerConduit::movePCRecord(int frompos, int topos) {
	// move the Record on the PC:
	if (frompos>topos) {
		// TODO
	} else {
		// TODO
	}
}
void OrganizerConduit::movePalmRecord(int frompos, int topos) {
	// move the Record on the Palm:
	if (frompos>topos) {
		// TODO
	} else {
		// TODO
	}
}


/* preSyncAction is used to initialize locally needed files and databases.
	e.g. open the local vCalendar file here and if that fails, return false.
   if false is returned, the database will be skipped, so use the return value!!! */
bool OrganizerConduit::preSyncAction(DBSyncInfo*dbinfo) {
	FUNCTIONSETUP;
	PCix=0;
	Palmix=0;
	inserted=false;
	switch (syncinfo.syncaction) {
		case st_vcal:
			fCalendar=new KCal::CalendarLocal(timezone);
			if (!fCalendar) return false;
			
			// if there is no calendar yet, use a first sync..
			// the calendar is initialized, so nothing more to do...
			if (!fCalendar->load(dbinfo->filename) ) {
#ifdef DEBUG
				DEBUGCONDUIT << "calendar file "<<fCalendarFile<<" could not be opened. Will create a new one"<<endl;
#endif
				fFullSync=true;
			}

			fP = new VCalPrivate(fCalendar);
			if (!fP) {
				emit logError(i18n("Could not load the calendar "+dbinfo->filename));
				kdWarning() << k_funcinfo << ": Couldn't load calendar "<<dbinfo->filename<<" from local harddrive" << endl;
				return false;
			}
			fP->updateTodos();
			if (fP->count()<1) fFullSync=true;

			return true;
			break;
		case st_pdb:
			#ifdef DEBUG
			DEBUGCONDUIT<<"  skipping database "<<dbinfo->filename<<". PDB not yet implemented." <<endl;
			#endif
			// TODO
			return false;
			break;
		default:
			return MultiDBConduit::preSyncAction(dbinfo);
			break;
	}
	return MultiDBConduit::preSyncAction(dbinfo);
}


/*
 * Given a pilot record, check to see what needs to be done to the
 * analogous vobject to bring things into sync.
 */
void OrganizerConduit::updateLocalEntry(PilotRecord *rec, bool force) {
	FUNCTIONSETUP;

	PilotOrganizerEntry*entry=createOrganizerEntry(rec);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Using rec @" << (int) rec
		<< " with ID " << ( rec ? rec->getID() : 0xffffffff)
		<< endl;
#endif

#ifdef DEBUG
	if (entry->getLevel()>hierlevel+1) {
		DEBUGCONDUIT<<fname << ": skipped one hierarchy level. Jumped from "<<hierlevel<<" to "<<entry->getLevel()<<endl;
	}
#endif
	
	// if we jumped one hierarchy level down, get the parent record. If the parent record was modified, we already have it, so we don't need to search
	// This could actually be even more optimized (most of the time we don't need the level parent, because nothing below it was changed)
	if (hierlevel<entry->getLevel() && levelparent[hierlevel]==0) {
		levelparent[hierlevel]=fP->findTodo(previd);
	}
	hierlevel=entry->getLevel();
	previd=rec->getID();
	
	// we have to search the new item only if it was changed, or the force flag is set
	if ( (rec->getAttrib() & dlpRecAttrDirty) || force ) {
		KCal::Todo *vtodo=fP->findTodo(rec->getID());
		if (!vtodo) {
			// no event was found, so we need to add one with some initial info
			vtodo = new KCal::Todo;
			fP->insertTodo(vtodo);

			vtodo->setPilotId(entry->getID());
			vtodo->setSyncStatus(Incidence::SYNCNONE);
		}
		levelparent[hierlevel]=vtodo;
		// todo has a parent element, so link it with it's parent and vice versa:
		if (hierlevel>0 && levelparent[hierlevel-1] ) {
			vtodo->setRelatedTo(levelparent[hierlevel-1]);
			if (! levelparent[hierlevel-1]->relations().containsRef(vtodo)) {
				levelparent[hierlevel-1]->addRelation(vtodo);
			}
		}

		// we don't want to modify the vobject with pilot info, because it has
		// already been  modified on the desktop.  The VObject's modified state
		// overrides the PilotRec's modified state.

		// otherwise, the vObject hasn't been touched.  Updated it with the
		// info from the PilotRec.
		
		if (entry->hasDate(DATE_CREATED)) vtodo->setCreated(entry->getDate(DATE_CREATED));

		if (entry->hasDate(DATE_STARTED)) vtodo->setDtStart(entry->getDate(DATE_STARTED));
		else vtodo->setHasStartDate(false);

		if (entry->hasDate(DATE_FINISHED)) vtodo->setCompleted(entry->getDate(DATE_FINISHED));
		else vtodo->setCompleted(false);

		if (entry->hasDate(DATE_DUE)) vtodo->setDtDue(entry->getDate(DATE_DUE));
		else vtodo->setHasDueDate(false);

		// Completed in %
		vtodo->setPercentComplete(entry->getProgress());
		vtodo->setCompleted(entry->getFlag(IS_CHECKED));
		
		// expanded does not have a counterpart: IS_EXPANDED	0x0800, IS_VISIBLE	0x0400

		// PRIORITY //
		vtodo->setPriority(entry->getPriority());

		// TODO: NUMBERING not yet implemented //
//		int nr;

		vtodo->setSummary(entry->getDescription());
		vtodo->setDescription(entry->getNote());

		// now let the child classes do it's custom initialization
		setCustomFields(vtodo, entry);
		
		vtodo->setSyncStatus(Incidence::SYNCNONE);
	}
	delete(entry);
}
