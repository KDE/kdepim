/* vcal-conduitbase.cc                      KPilot
**
** Copyright (C) 2002 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
** 
** Contributions:
**    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
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

static const char *vcalconduitbase_id = "$Id$";

#include <options.h>
#include <unistd.h>

#include <qdatetime.h>
#include <qtimer.h>

#include <pilotUser.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include <calendar.h>
#include <calendarlocal.h>
#include <incidence.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>


/*
** KDE 2.2 uses class KORecurrence in a different header file.
*/
#ifdef KDE2
#include <korecurrence.h>
#define Recurrence_t KCal::KORecurrence
#define DateList_t QDateList
#define DateListIterator_t QDateListIterator
#else
#include <recurrence.h>
#define Recurrence_t KCal::Recurrence
#define DateList_t KCal::DateList
#define DateListIterator_t KCal::DateList::ConstIterator
#endif

#include <pilotSerialDatabase.h>
#include <pilotLocalDatabase.h>
#include <pilotDateEntry.h>

#include "vcal-factorybase.h"
#include "vcal-conduitbase.moc"

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



/****************************************************************************
 *                          VCalConduitBase class                               *
 ****************************************************************************/

VCalConduitBase::VCalConduitBase(KPilotDeviceLink *d,
	const char *n,
	const QStringList &a) :
	ConduitAction(d,n,a),
	fCalendar(0L),
	fP(0L)
{
	FUNCTIONSETUP;
	(void) vcalconduitbase_id;
}


VCalConduitBase::~VCalConduitBase()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP);
	KPILOT_DELETE(fCalendar);
}


/* There are several different scenarios for a record on the Palm and its PC counterpart
  N means a new record, M flags a modified record, D a deleted and - an unmodified record
  first is the Palm record, second the corresponding PC record
  (-,-)...unchanged, just sync if first time or full sync
  (N,-)...no rec matching the Palm ID in the backupDB/calendar yet => add KCal::Event
  (M,-)...record is in backupDB, unchanged in calendar => modify in calendar and in backupDB
  (D,-)...deleted on Palm, exists in backupDB and calendar => just delete from calendar and backupDB
  (-,N)...no or invalid pilotID set for the KCal::Event => just add to palm and backupDB
  (-,M)...valid PilotID set => just modify on Palm
  (-,D)...Record in backupDB, but not in calendar => delete from Palm and backupDB
  (N,N)...Can't find out (the two records are not correlated in any way, they just have the same data!!
  (M,M),(M,L),(L,M)...(Record exists on Palm and the Event has the ID) CONFLICT, ask the user what to do
                      or use a config setting
  (L,L)...already deleted on both, no need to do anything.


   The sync process is as follows (for a fast sync):
	1) syncPalmRecToPC goes through all records on Palm (just the modified one are necessary), find it
	   in the backupDB. The following handles ([NMD],*)
	   a) if it doesn't exist and was not deleted, add it to the calendar and the backupDB
	   b) if it exists and was not deleted,
			A) if it is unchanged in the calendar, just modify in the calendar
		c) if it exists and was deleted, delete it from the calendar if necessary
	2) syncEvent goes through all KCale::Events in the calendar (just modified, this is the modification
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
-) a first sync completely ignores the pilotID setting of the calendar events. All records are added,
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

/* virtual */ void VCalConduitBase::exec()
{
	FUNCTIONSETUP;

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
	
	readConfig();

	// don't do a first sync by default in any case, only when explicitely requested, or the backup
	// database or the alendar are empty.
	fFirstTime = (syncAction==SYNC_FIRST /*|| nextSyncAction!=0*/);
	usr=fHandle->getPilotUser();
	// changing the PC or using a different Palm Desktop app causes a full sync
	// Use gethostid for this, since JPilot uses 1+(2000000000.0*random()/(RAND_MAX+1.0))
	// as PC_ID, so using JPilot and KPilot is the same as using two differenc PCs
	fFullSync = (syncAction==SYNC_FULL) ||
		((usr->getLastSyncPC()!=(unsigned long) gethostid()) && fConfig->readBoolEntry(VCalConduitFactoryBase::fullSyncOnPCChange, true));

	if (!openDatabases(dbname(), &fFullSync) ) goto error;
	if (!openCalendar() ) goto error;
	
	preSync();


#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": fullsync="<<fFullSync<<", firstSync="<<fFirstTime<<endl;
	DEBUGCONDUIT<<fname<<": syncAction="<<syncAction<<", nextSyncAction = "<<nextSyncAction
		<<", conflictResolution = "<<conflictResolution<<", archive = "<<archive<<endl;
#endif

	addSyncLogEntry(i18n("Syncing with file \"%s\"").arg(fCalendarFile));
	pilotindex=0;
	switch (nextSyncAction)
	{
	case 2:
		// TODO: Clear the palm and backup database??? Or just add the new items ignore
		// the Palm->PC side and leave the existing items on the palm?
		QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
		break;
	case 1:
		// TODO: Clear the backup database and the calendar, update fP
		//       or just add the palm items and leave the PC ones there????
	default:
		QTimer::singleShot(0, this, SLOT(syncPalmRecToPC()));
	}
	return;

error:
			cout<<"Error opening the dbs"<<endl;

	emit logError(i18n("Couldn't open the calendar databases."));

	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fP);
	emit syncDone(this);
}


/* virtual */ void VCalConduitBase::readConfig()
{
	fConfig->setGroup(configGroup());

	fCalendarFile = fConfig->readEntry(VCalConduitFactoryBase::calendarFile);
	syncAction = fConfig->readNumEntry(VCalConduitFactoryBase::syncAction);
	nextSyncAction = fConfig->readNumEntry(VCalConduitFactoryBase::nextSyncAction);
	fConfig->writeEntry(VCalConduitFactoryBase::nextSyncAction, 0);
	conflictResolution = fConfig->readNumEntry(VCalConduitFactoryBase::conflictResolution);
	archive = fConfig->readBoolEntry(VCalConduitFactoryBase::archive);
}
	

/* virtual */ bool VCalConduitBase::openCalendar()
{
	FUNCTIONSETUP;
	
	KConfig korgcfg( locate( "config", "korganizerrc" ) );
	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	QString tz(korgcfg.readEntry( "TimeZoneId" ) );
#ifdef DEBUG
	DEBUGCONDUIT << fname<<": KOrganizer's time zone = "<<tz<<endl;
#endif


#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Using calendar file "
		<< fCalendarFile
		<< endl;
#endif

	fCalendar = new KCal::CalendarLocal(tz);
	if ( !fCalendar) 
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ":Cannot initialize calendar object"<<endl;
#endif
		return false;
	}

	// if there is no calendar yet, use a first sync..
	// the calendar is initialized, so nothing more to do...
	if (!fCalendar->load(fCalendarFile) )
	{
#ifdef DEBUG
		DEBUGCONDUIT << "calendar file "<<fCalendarFile<<" could not be opened. Will create a new one"<<endl;
#endif
		fFirstTime=true;
	}

	fP = newVCalPrivate(fCalendar);
	if (!fP) return false;
	fP->updateIncidences();
	if (fP->count()<1) fFullSync=true;
	
	return (fCalendar && fP);
}

void VCalConduitBase::syncPalmRecToPC()
{
	FUNCTIONSETUP;

	PilotRecord *r;
	if (fFirstTime || fFullSync)
	{
		r = fDatabase->readRecordByIndex(pilotindex++);
	}
	else
	{
		r = fDatabase->readNextModifiedRec();
	}
	PilotRecord *s = 0L;

	if (!r)
	{
		fP->updateIncidences();
		if (nextSyncAction==1)
		{
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
		else
		{
			QTimer::singleShot(0 ,this,SLOT(syncPCRecToPalm()));
			return;
		}
	}

	// first, delete the dirty flag from the palm database. Prolly should do this after the record has been synced
//	r->setAttrib(r->getAttrib() & ~dlpRecAttrDirty);
//	fDatabase->writeRecord(r);
	bool archiveRecord=(r->isArchived());//getAttrib() & dlpRecAttrArchived);

	s = fLocalDatabase->readRecordById(r->getID());
	if (!s || fFirstTime)
	{
#ifdef DEBUG
		if (r->getID()>0)
		{
			DEBUGCONDUIT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGCONDUIT<< fname<<": Could not read palm record with ID "<<r->getID()<<endl;
		}
#endif
		if (!r->isDeleted() || (archive && archiveRecord))
		{
			KCal::Incidence*e=addRecord(r);
			if (archive && archiveRecord)  {
				e->setSyncStatus(KCal::Incidence::SYNCDEL);
			}
		}
	}
	else
	{
		if (r->isDeleted())
		{
			if (archive && archiveRecord) 
			{
				changeRecord(r,s);
			}
			else
			{
				deleteRecord(r,s);
			}
		}
		else
		{
			changeRecord(r,s);
		}
	}

	KPILOT_DELETE(r);
	KPILOT_DELETE(s);

	QTimer::singleShot(0,this,SLOT(syncPalmRecToPC()));
}


void VCalConduitBase::syncPCRecToPalm()
{
	FUNCTIONSETUP;
	KCal::Incidence*e=0L;
	if (fFirstTime || fFullSync) e=fP->getNextIncidence();
	else e=fP->getNextModifiedIncidence();

	if (!e)
	{
		pilotindex=0;
		if (nextSyncAction!=0) 
		{
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
		QTimer::singleShot(0,this,SLOT(syncDeletedIncidence()));
		return;
	}
	// find the corresponding index on the palm and sync. If there is none, create it.
	int ix=e->pilotId();
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": found PC entry with pilotID "<<ix<<endl;
		DEBUGCONDUIT<<fname<<": Description: "<<e->summary()<<endl;
#endif
	PilotRecord *s=0L;
	if (ix>0 && (s=fDatabase->readRecordById(ix)))
	{
		if (e->syncStatus()==KCal::Incidence::SYNCDEL)
		{
			deletePalmRecord(e, s);
		}
		else
		{
			changePalmRecord(e, s);
		}
		KPILOT_DELETE(s);
	} else {
#ifdef DEBUG
		if (ix>0)
		{
			DEBUGCONDUIT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGCONDUIT<< fname<<": Could not read palm record with ID "<<ix<<endl;
		}
#endif
		addPalmRecord(e);
	}
	QTimer::singleShot(0, this, SLOT(syncPCRecToPalm()));
}


void VCalConduitBase::syncDeletedIncidence()
{
	FUNCTIONSETUP;

	PilotRecord *r = fLocalDatabase->readRecordByIndex(pilotindex++);
	if (!r || fFullSync || fFirstTime)
	{
		QTimer::singleShot(0 ,this,SLOT(cleanup()));
		return;
	}

	KCal::Incidence *e = fP->findIncidence(r->getID());
	if (!e)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"didn't find incidence with id="<<r->getID()<<", deleting it"<<endl;
#endif
		// entry was deleted from Calendar, so delete it from the palm
//		PilotRecord*s=fLocalDatabase->readRecordById(r->getID());
//		if (s)
//		{
//			// delete the record from the palm
//			s->makeDeleted();
////			s->setAttrib(s->getAttrib() & ~dlpRecAttrDeleted & ~dlpRecAttrDirty);
//			fDatabase->writeRecord(s);
//			KPILOT_DELETE(s);
//		}
		deletePalmRecord(NULL, r);
//		r->makeDeleted();
////		r->setAttrib(r->getAttrib() & ~dlpRecAttrDeleted & ~dlpRecAttrDirty);
//		fLocalDatabase->writeRecord(r);
//		fDatabase->writeRecord(r);
	}

	KPILOT_DELETE(r);
	QTimer::singleShot(0,this,SLOT(syncDeletedIncidence()));
}


void VCalConduitBase::cleanup()
{
	FUNCTIONSETUP;
	postSync();

	if (fDatabase) 
	{
		fDatabase->resetSyncFlags();
		fDatabase->cleanup();
	}
	if (fLocalDatabase) 
	{
		fLocalDatabase->resetSyncFlags();
		fLocalDatabase->cleanup();
	}
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
	if (fCalendar) fCalendar->save(fCalendarFile);
	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fP);

	emit syncDone(this);
}


KCal::Incidence* VCalConduitBase::addRecord(PilotRecord *r)
{
	FUNCTIONSETUP;

	recordid_t id=fLocalDatabase->writeRecord(r);
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": Pilot Record ID="<<r->getID()<<", backup ID="<<id<<endl;
#endif

	PilotAppCategory *de=newPilotEntry(r);
	KCal::Incidence*e = newIncidence();
	if (e && de)
	{
		incidenceFromRecord(e,de);
		// TODO: find out if there is already an entry with this data...

		fP->addIncidence(e);
	}
	KPILOT_DELETE(de);
	return e;
}

// return how to resolve conflicts. for now PalmOverrides=0=false, PCOverrides=1=true, Ask=2-> ask the user using a messagebox
int VCalConduitBase::resolveConflict(KCal::Incidence*e, PilotAppCategory*de) {
	if (conflictResolution==RES_ASK)
	{
		return KMessageBox::warningYesNo(NULL, 
			i18n("The following item was modified both on the Pilot and on your PC:\nPC entry:\n\t")+e->summary()+i18n("\nPilot entry:\n\t")+getTitle(de)+
				i18n("\n\nShould the Pilot entry overwrite the PC entry? If you select \"No\", the PC entry will overwrite the Pilot entry."),
			i18n("Conflicting Entries")
		)==KMessageBox::No;
	}
	return conflictResolution;
}

KCal::Incidence*VCalConduitBase::changeRecord(PilotRecord *r,PilotRecord *)
{
	FUNCTIONSETUP;

	PilotAppCategory*de=newPilotEntry(r);
	KCal::Incidence *e = fP->findIncidence(r->getID());

	if (e && de)
	{
		// TODO: check for conflict, and if there is one, ask for resolution
		if ( (e->syncStatus()!=KCal::Incidence::SYNCNONE) && (r->getAttrib() &dlpRecAttrDirty) )
		{
			// TODO: I have not yet found a way to complete ignore an item
			if (resolveConflict(e, de))
			{
				// PC record takes precedence:
				KPILOT_DELETE(de);
				return e;
			}
		}
		// no conflict or conflict resolution says, Palm overwrites, so do it:
		incidenceFromRecord(e,de);
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		fLocalDatabase->writeRecord(r);
	}
	else
	{
		kdWarning() << k_funcinfo << ": While changing record -- not found in iCalendar" << endl;
		addRecord(r);
	}
	KPILOT_DELETE(de);
	return e;
}


KCal::Incidence*VCalConduitBase::deleteRecord(PilotRecord *r, PilotRecord *)
{
	FUNCTIONSETUP;

	KCal::Incidence *e = fP->findIncidence(r->getID());
	if (e)
	{
		// RemoveEvent also takes it out of the calendar.
		fP->removeIncidence(e);
	}
	fLocalDatabase->writeRecord(r);
	return NULL;
}


void VCalConduitBase::addPalmRecord(KCal::Incidence*e)
{
	FUNCTIONSETUP;

	PilotAppCategory*de=newPilotEntry(NULL);
	updateIncidenceOnPalm(e, de);
	KPILOT_DELETE(de);
}


void VCalConduitBase::changePalmRecord(KCal::Incidence*e, PilotRecord*s)
{
	PilotAppCategory*de=newPilotEntry(s);
	updateIncidenceOnPalm(e, de);
	KPILOT_DELETE(de);
}


void VCalConduitBase::deletePalmRecord(KCal::Incidence*e, PilotRecord*s)
{
	FUNCTIONSETUP;
	if (s)
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": deleting record " << s->getID() << endl;
#endif
		s->makeDeleted();
//		s->setAttrib(s->getAttrib() & ~dlpRecAttrDeleted);
		fDatabase->writeRecord(s);
		fLocalDatabase->writeRecord(s);
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": could not find record to delete (" << e->pilotId() << ")" << endl;
#endif
	}
}


/* I have to use a pointer to an existing PilotDateEntry so that I can handle
   new records as well (and to prevent some crashes concerning the validity
   domain of the PilotRecord*r). In syncEvent this PilotDateEntry is created. */
void VCalConduitBase::updateIncidenceOnPalm(KCal::Incidence*e, PilotAppCategory*de)
{
	FUNCTIONSETUP;
	if (!de || !e ) {
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": NULL event given... Skipping it"<<endl;
#endif
		return;
	}
	if (e->syncStatus()==KCal::Incidence::SYNCDEL)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": don't write deleted incidence "<<e->summary()<<" to the palm"<<endl;
#endif
		return;
	}
	PilotRecord*r=recordFromIncidence(de, e);

	// TODO: Check for conflict!
	if (r)
	{
		recordid_t id=fDatabase->writeRecord(r);
		r->setID(id);
//		r->setAttrib(r->getAttrib() & ~dlpRecAttrDeleted);
		fLocalDatabase->writeRecord(r);
//		fDatabase->writeRecord(r);
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		e->setPilotId(id);
		KPILOT_DELETE(r);
	}
}


// $Log$
// Revision 1.14  2002/07/09 22:38:04  kainhofe
// Implemented a first (not-yet-functional) version of the category sync
//
// Revision 1.13  2002/07/05 00:00:00  kainhofe
// Add deleted record only if archived are supposed to be synced
//
// Revision 1.12  2002/06/12 22:11:17  kainhofe
// Proper cleanup, libkcal still has some problems marking records modified on loading
//
// Revision 1.11  2002/06/09 21:08:06  kainhofe
// Use the openDatabases() function and the fDatabase/fLocalDatabase instead of our own fCurrentDatabase/fBackupDatabase
//
// Revision 1.10  2002/06/07 07:13:24  adridg
// Make VCal conduit use base-class fDatabase and fLocalDatabase (hack).
// Extend *Database classes with dbPathName() for consistency.
//
// Revision 1.9  2002/06/07 06:37:15  adridg
// Be safer on cleanup to avoid crash
//
// Revision 1.8  2002/05/18 13:08:57  kainhofe
// dirty flag is now cleared, conflict resolution shows the correct item title and asks the correct question
//
// Revision 1.7  2002/05/16 13:06:48  mhunter
// Corrected typographical errors and Palm -> Pilot for consistency
//
// Revision 1.6  2002/05/15 22:57:39  kainhofe
// if the backup db does not exist, it is now correctly retrieved correctly from the palm
//
// Revision 1.5  2002/05/14 23:07:49  kainhofe
// Added the conflict resolution code. the Palm and PC precedence is currently swapped, and will be improved in the next few days, anyway...
//
// Revision 1.4  2002/05/03 19:19:57  kainhofe
// Local timezone from KOrganizer is now used for the sync
//
// Revision 1.1.2.4  2002/05/03 19:08:52  kainhofe
// Local timezone from KOrganizer is now used for the sync
//
// Revision 1.1.2.3  2002/05/01 21:11:49  kainhofe
// Reworked the settings dialog, added various different sync options
//
// Revision 1.1.2.2  2002/04/28 20:20:03  kainhofe
// calendar and backup databases are now created if they didn't exist
//
// Revision 1.1.2.1  2002/04/28 12:58:54  kainhofe
// Calendar conduit now works, no memory leaks, timezone still shifted. Todo conduit mostly works, for my large list it crashes when saving the calendar file.
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
// Exceptions no longer crash the daemon, recurrences are correct now, end date is set correctly. Problems: All events are off 1 day, lots of duplicates, exceptions are duplicate, too.
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

