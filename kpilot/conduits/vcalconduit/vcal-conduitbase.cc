/* KPilot
**
** Copyright (C) 2002-3 by Reinhold Kainhofer
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

static const char *vcalconduitbase_id = "$Id$";

#include <options.h>

#include <qtimer.h>
#include <qfile.h>

#include <kmessagebox.h>
#include <kio/netaccess.h>

#include "libkcal/calendar.h"
#include "libkcal/calendarlocal.h"
#include "libkcal/calendarresources.h"
#include <kstandarddirs.h>

#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "pilotDateEntry.h"

#include "vcal-conduitbase.moc"
#include "vcalconduitSettings.h"


#ifndef LIBKCAL_IS_VERSION
#warning "Using an old version of libkcal with timezone bug."
#define LIBKCAL_IS_VERSION(a,b,c) (0)
#endif



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
#ifdef DEBUG
	DEBUGCONDUIT<<vcalconduitbase_id<<endl;
#endif
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
	1) slotPalmRecToPC goes through all records on Palm (just the modified one are necessary), find it
		in the backupDB. The following handles ([NMD],*)
		a) if it doesn't exist and was not deleted, add it to the calendar and the backupDB
		b) if it exists and was not deleted,
			A) if it is unchanged in the calendar, just modify in the calendar
		c) if it exists and was deleted, delete it from the calendar if necessary
	2) slotEvent goes through all KCale::Events in the calendar (just modified, this is the modification
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
	b) it was explicitly requested by pressing the full sync button in KPilot
	c) the setting "always full sync" was selected in the configuration dlg
-) a first sync is done if
	a) either the calendar or the backup DB does not exist.
	b) the calendar and the backup DB exists, but the sync is done for a different User name
	c) it was explicitly requested in KPilot

*/

/* virtual */ bool VCalConduitBase::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT<<vcalconduitbase_id<<endl;

	readConfig();

	// don't do a first sync by default in any case, only when explicitly
	// requested, or the backup database or the alendar are empty.
	setFirstSync( false );

	// TODO: Check Full sync and First sync
	bool retrieved = false;
	if (!openCalendar() ) goto error;
	if (!openDatabases(dbname(), &retrieved) ) goto error;
	setFirstSync( retrieved );
	preSync();


#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": fullsync="<<isFullSync()<<", firstSync="<<isFirstSync()<<endl;
	DEBUGCONDUIT<<fname<<": syncAction="<<getSyncDirection()<<
		", conflictResolution = "<<getConflictResolution()<<", archive = "<<config()->syncArchived()<<endl;
#endif

	pilotindex=0;
	switch (getSyncDirection())
	{
	case SyncAction::eCopyPCToHH:
		// TODO: Clear the palm and backup database??? Or just add the new items ignore
		// the Palm->PC side and leave the existing items on the palm?
		emit logMessage(i18n("Copying records to Pilot ..."));
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		break;
	case SyncAction::eCopyHHToPC:
		// TODO: Clear the backup database and the calendar, update fP
		//       or just add the palm items and leave the PC ones there????
	default:
		emit logMessage(i18n("Copying records to PC ..."));
		QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
	}
	return true;

error:

	emit logError(i18n("Could not open the calendar databases."));

	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fP);
	return false;
}



/* virtual */ void VCalConduitBase::readConfig()
{
	config()->readConfig();
	SyncAction::ConflictResolution res=(SyncAction::ConflictResolution)(config()->conflictResolution());
	setConflictResolution(res);
}

#ifdef DEBUG
static void listResources(KCal::CalendarResources *p)
{
	FUNCTIONSETUP;
	KCal::CalendarResourceManager *manager = p->resourceManager();

	DEBUGCONDUIT << fname << ": Resources in calendar:" << endl;
	KCal::CalendarResourceManager::Iterator it;
	for( it = manager->begin(); it != manager->end(); ++it )
	{
		DEBUGCONDUIT << fname << ": " << (*it)->resourceName() << endl;
	}
}
#endif

/* virtual */ bool VCalConduitBase::openCalendar()
{
	FUNCTIONSETUP;

	KConfig korgcfg( locate( "config", CSL1("korganizerrc") ) );
	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	QString tz(korgcfg.readEntry( "TimeZoneId" ) );
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": KOrganizer's time zone = " << tz << endl;
#endif

	// Need a subclass ptr. for the ResourceCalendar methods
	KCal::CalendarResources *rescal = 0L;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Got calendar type " << config()->calendarType() << endl;
#endif

	switch(config()->calendarType())
	{
		case VCalConduitSettings::eCalendarLocal:
		{
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< "Using CalendarLocal, file="
				<< config()->calendarFile() << endl;
#endif
			if (config()->calendarFile().isEmpty() )
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
					<< "Empty calendar file name."
					<< endl;
#endif
				emit logError(i18n("You selected to sync with the a iCalendar file, "
						"but did not give a filename. Please select a valid file name in "
						"the conduit's configuration dialog"));
				return false;
			}

			fCalendar = new KCal::CalendarLocal(tz);
			if ( !fCalendar)
			{
				kdWarning() << k_funcinfo
					<< "Cannot initialize calendar object for file "
					<< config()->calendarFile() << endl;
				return false;
			}
#ifdef DEBUG
			DEBUGCONDUIT << fname
				<< "Calendar's timezone: "
				<< fCalendar->timeZoneId() << endl;
			DEBUGCONDUIT << fname
				<< "Calendar is local time: "
				<< fCalendar->isLocalTime() << endl;
#endif
			emit logMessage( fCalendar->isLocalTime() ?
				i18n("Using local time zone: %1").arg(tz) :
				i18n("Using non-local time zone: %1").arg(tz) );

			KURL kurl(config()->calendarFile());
			if(!KIO::NetAccess::download(config()->calendarFile(), fCalendarFile, 0L) &&
				!kurl.isLocalFile())
			{
				emit logError(i18n("You chose to sync with the file \"%1\", which "
							"cannot be opened. Please make sure to supply a "
							"valid file name in the conduit's configuration dialog. "
							"Aborting the conduit.").arg(config()->calendarFile()));
				KIO::NetAccess::removeTempFile(fCalendarFile);
				return false;
			}

			// if there is no calendar yet, use a first sync..
			// the calendar is initialized, so nothing more to do...
			if (!dynamic_cast<KCal::CalendarLocal*>(fCalendar)->load(fCalendarFile) )
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
					<< "Calendar file "
					<< fCalendarFile
					<< " could not be opened. "
					   "Will create a new one"
					<< endl;
#endif
				// Try to create empty file. if it fails,
				// no valid file name was given.
				QFile fl(fCalendarFile);
				if (!fl.open(IO_WriteOnly | IO_Append))
				{
#ifdef DEBUG
					DEBUGCONDUIT << fname
						<< "Invalid calendar file name "
						<< fCalendarFile << endl;
#endif
					emit logError(i18n("You chose to sync with the file \"%1\", which "
							"cannot be opened or created. Please make sure to supply a "
							"valid file name in the conduit's configuration dialog. "
							"Aborting the conduit.").arg(config()->calendarFile()));
					return false;
				}
				fl.close();
				setFirstSync( true );
			}
			addSyncLogEntry(i18n("Syncing with file \"%1\"").arg(config()->calendarFile()));
			break;
		}

		case VCalConduitSettings::eCalendarResource:
#ifdef DEBUG
			DEBUGCONDUIT << "Using CalendarResource!" << endl;
#endif
			rescal = new KCal::CalendarResources( tz );
#ifdef DEBUG
			listResources(rescal);
#endif
			fCalendar = rescal;
			if ( !fCalendar)
			{
				kdWarning() << k_funcinfo << "Cannot initialize calendar "<<
					"object for ResourceCalendar"<<endl;
				return false;
			}
#if LIBKCAL_IS_VERSION(1,1,0)
			rescal->readConfig();
			rescal->load();
#else
#warning "Timezone bug is present."
#endif
			addSyncLogEntry(i18n("Syncing with standard calendar resource."));
			emit logMessage( fCalendar->isLocalTime() ?
				i18n("Using local time zone: %1").arg(tz) :
				i18n("Using non-local time zone: %1").arg(tz) );
			break;
		default:
			break;

	}

	if (!fCalendar)
	{
		kdWarning() <<k_funcinfo << "Unable to initialize calendar object. Please check the conduit's setup."<<endl;
		emit logError(i18n("Unable to initialize the calendar object. Please check the conduit's setup"));
		return false;
	}
	fP = newVCalPrivate(fCalendar);
	if (!fP)
	{
		return false;
	}
	fP->updateIncidences();
	if (fP->count()<1)
	{
		setFirstSync( true );
	}

	return true;
}



void VCalConduitBase::slotPalmRecToPC()
{
	FUNCTIONSETUP;

	PilotRecord *r;
	if (isFullSync())
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
		if (getSyncDirection()==SyncAction::eCopyHHToPC)
		{
			emit logMessage(i18n("Cleaning up ..."));
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
		else
		{
			emit logMessage(i18n("Copying records to Pilot ..."));
			QTimer::singleShot(0 ,this,SLOT(slotPCRecToPalm()));
			return;
		}
	}

	// let subclasses do something with the record before we try to sync
	preRecord(r);

//	DEBUGCONDUIT<<fname<<": Event: "<<e->dtStart()<<" until "<<e->dtEnd()<<endl;
//	DEBUGCONDUIT<<fname<<": Time: "<<e->dtStart()<<" until "<<e->dtEnd()<<endl;
	bool archiveRecord=(r->isArchived());

	s = fLocalDatabase->readRecordById(r->id());
	if (!s || isFirstSync())
	{
#ifdef DEBUG
		if (r->id()>0 && !s)
		{
			DEBUGCONDUIT<<"---------------------------------------------------------------------------"<<endl;
			DEBUGCONDUIT<< fname<<": Could not read palm record with ID "<<r->id()<<endl;
		}
#endif
		if (!r->isDeleted() || (config()->syncArchived() && archiveRecord))
		{
			KCal::Incidence*e=addRecord(r);
			if (config()->syncArchived() && archiveRecord)  {
				e->setSyncStatus(KCal::Incidence::SYNCDEL);
			}
		}
	}
	else
	{
		if (r->isDeleted())
		{
			if (config()->syncArchived() && archiveRecord)
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

	QTimer::singleShot(0,this,SLOT(slotPalmRecToPC()));
}


void VCalConduitBase::slotPCRecToPalm()
{
	FUNCTIONSETUP;
	KCal::Incidence*e=0L;
	if (isFullSync()) e=fP->getNextIncidence();
	else e=fP->getNextModifiedIncidence();

	if (!e)
	{
		pilotindex=0;
		if ( (getSyncDirection()==SyncAction::eCopyHHToPC) || (getSyncDirection()==SyncAction::eCopyPCToHH) )
		{
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
		QTimer::singleShot(0,this,SLOT(slotDeletedIncidence()));
		return;
	}

	// let subclasses do something with the event
	preIncidence(e);

	// find the corresponding index on the palm and sync. If there is none, create it.
	recordid_t ix=e->pilotId();
#ifdef DEBUG
		DEBUGCONDUIT<<fname<<": found PC entry with pilotID "<<ix<<endl;
		DEBUGCONDUIT<<fname<<": Description: "<<e->summary()<<endl;
		DEBUGCONDUIT<<fname<<": Time: "<<e->dtStart().toString()<<" until "<<e->dtEnd().toString()<<endl;
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
	QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
}


void VCalConduitBase::slotDeletedIncidence()
{
	FUNCTIONSETUP;

	PilotRecord *r = fLocalDatabase->readRecordByIndex(pilotindex++);
	if (!r || isFullSync() )
	{
		QTimer::singleShot(0 ,this,SLOT(cleanup()));
		return;
	}

	KCal::Incidence *e = fP->findIncidence(r->id());
	if (!e)
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"didn't find incidence with id="<<r->id()<<", deleting it"<<endl;
#endif
		// entry was deleted from Calendar, so delete it from the palm
//		PilotRecord*s=fLocalDatabase->readRecordById(r->id());
//		if (s)
//		{
//			// delete the record from the palm
//			s->setDeleted();
////			s->setAttrib(s->getAttrib() & ~dlpRecAttrDeleted & ~dlpRecAttrDirty);
//			fDatabase->writeRecord(s);
//			KPILOT_DELETE(s);
//		}
		deletePalmRecord(NULL, r);
//		r->setDeleted();
////		r->setAttrib(r->getAttrib() & ~dlpRecAttrDeleted & ~dlpRecAttrDirty);
//		fLocalDatabase->writeRecord(r);
//		fDatabase->writeRecord(r);
	}

	KPILOT_DELETE(r);
	QTimer::singleShot(0,this,SLOT(slotDeletedIncidence()));
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
	if (fCalendar)
	{
		KURL kurl(config()->calendarFile());
		switch(config()->calendarType())
		{
			case VCalConduitSettings::eCalendarLocal:
				dynamic_cast<KCal::CalendarLocal*>(fCalendar)->save(fCalendarFile);
				if(!kurl.isLocalFile())
				{
					if(!KIO::NetAccess::upload(fCalendarFile, config()->calendarFile(), 0L)) {
						emit logError(i18n("An error occurred while uploading \"%1\". You can try to upload "
							"the temporary local file \"%2\" manually.")
							.arg(config()->calendarFile()).arg(fCalendarFile));
					}
					else {
						KIO::NetAccess::removeTempFile(fCalendarFile);
					}
					QFile backup(fCalendarFile + CSL1("~"));
					backup.remove();
				}
				break;
			case VCalConduitSettings::eCalendarResource:
				fCalendar->save();
				break;
			default:
				break;
		}
		fCalendar->close();
	}
	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fP);

	emit syncDone(this);
}



void VCalConduitBase::postSync()
{
	FUNCTIONSETUP;
}


KCal::Incidence* VCalConduitBase::addRecord(PilotRecord *r)
{
	FUNCTIONSETUP;

	recordid_t id=fLocalDatabase->writeRecord(r);
#ifdef DEBUG
	DEBUGCONDUIT<<fname<<": Pilot Record ID="<<r->id()<<", backup ID="<<id<<endl;
#else
	Q_UNUSED(id);
#endif

	PilotAppCategory *de=newPilotEntry(r);
	KCal::Incidence*e =0L;

	if (de)
	{
		e=fP->findIncidence(r->id());
		if (!e)
		{
			// no corresponding entry found, so create, copy and insert it.
			e=newIncidence();
			incidenceFromRecord(e,de);
			fP->addIncidence(e);
		}
		else
		{
			// similar entry found, so just copy, no need to insert again
			incidenceFromRecord(e,de);
		}
	}
	KPILOT_DELETE(de);
	return e;
}

// return how to resolve conflicts. for now PalmOverrides=0=false, PCOverrides=1=true, Ask=2-> ask the user using a messagebox
int VCalConduitBase::resolveConflict(KCal::Incidence*e, PilotAppCategory*de) {
	if (getConflictResolution()==SyncAction::eAskUser)
	{
		// TODO: This is messed up!!!
		QString query = i18n("The following item was modified "
			"both on the Handheld and on your PC:\nPC entry:\n\t");
		query += e->summary();
		query += i18n("\nHandheld entry:\n\t");
		query += getTitle(de);
		query += i18n("\n\nWhich entry do you want to keep? It will "
			"overwrite the other entry.");

		return KMessageBox::No == questionYesNo(
			query,
			i18n("Conflicting Entries"),
			QString::null,
			0 /* Never timeout */,
			i18n("Handheld"), i18n("PC"));
	}
	return getConflictResolution();
}

KCal::Incidence*VCalConduitBase::changeRecord(PilotRecord *r,PilotRecord *)
{
	FUNCTIONSETUP;

	PilotAppCategory*de=newPilotEntry(r);
	KCal::Incidence *e = fP->findIncidence(r->id());

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

	KCal::Incidence *e = fP->findIncidence(r->id());
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
		DEBUGCONDUIT << fname << ": deleting record " << s->id() << endl;
#endif
		s->setDeleted();
		fDatabase->writeRecord(s);
		fLocalDatabase->writeRecord(s);
	}
	else
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": could not find record to delete (" << e->pilotId() << ")" << endl;
#endif
	}

	Q_UNUSED(e);
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
		e->setPilotId(id);
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		KPILOT_DELETE(r);
	}
}

const QString VCalConduitBase::dbname()
{
    return QString::null;
}


