/* KPilot
**
** Copyright (C) 2002-3 by Reinhold Kainhofer
** Copyright (C) 2001 by Dan Pilone
**
** Contributions:
**    Copyright (c) 2001 David Jarvie <software@astrojar.org.uk>
**    Copyright (C) 2006 by Bertjan Broeksema <b.broeksema@gmail.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

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

#include "conduitstate.h"
#include "initstate.h"


/****************************************************************************
 *                          VCalConduitBase class                           *
 ****************************************************************************/

VCalConduitBase::VCalConduitBase(KPilotLink *d,
	const char *n,
	const QStringList &a) :
	ConduitAction(d,n,a),
	fCalendar(0L),
	fP(0L)
{
	FUNCTIONSETUP;

	fState = new InitState();
}

VCalConduitBase::~VCalConduitBase()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fP);
	KPILOT_DELETE(fState);
	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fDatabase);
	KPILOT_DELETE(fLocalDatabase);
}


/*
	There are several different scenarios for a record on the Palm and its PC
	counterpart. N means a new record, M flags a modified record, D a deleted
	and - an unmodified record. First is the Palm record, second the
	corresponding PC record:
	(-,-) unchanged, just sync if first time or full sync
	(N,-) no rec matching the Palm ID in the backupDB/calendar yet => add
	      KCal::Event
	(M,-) record is in backupDB, unchanged in calendar => modify in calendar and
	      in backupDB
	(D,-) deleted on Palm, exists in backupDB and calendar => just delete from
	      calendar and backupDB
	(-,N) no or invalid pilotID set for the KCal::Event => just add to palm and
	      backupDB
	(-,M) valid PilotID set => just modify on Palm
	(-,D) Record in backupDB, but not in calendar => delete from Palm and
	      backupDB
	(N,N) Can't find out (the two records are not correlated in any way, they
	      just have the same data!!
	(M,M),(M,L),(L,M) (Record exists on Palm and the Event has the ID) CONFLICT,
	      ask the user what to do or use a config setting
	(L,L) already deleted on both, no need to do anything.

	The sync process is as follows (for a fast sync):
	1) HHToPCState goes through all records on Palm (just the modified one
	   are necessary), find it in the backupDB. The following handles ([NMD],*)
		a) if it doesn't exist and was not deleted, add it to the calendar and
		   the backupDB
		b) if it exists, is unchanged in the calendar and was not deleted,
		   just modify in the calendar
		c) if it exists and was deleted, delete it from the calendar if
		   necessary
	2) PCToHHState goes through all KCale::Events in the calendar (just
	   modified, this is the modification time is later than the last sync time
		). This handles (-,N),(-,M)
		a) if it does not have a pilotID, add it to the palm and backupDB,
		   store the PalmID
		b) if it has a valid pilotID, update the Palm record and the backup
	3) DeletedUnsyncedHHState goes through all palm records (which don't
	   have the deleted flag) of the palm db and if one does not exist in the
	   Calendar, it was deleted there, so delete it from the Palm and backup,
	   too. This handles the case of (-,D)
	4) DeletedUnsyncedPCState goes through all KCal::Events in the calendar and
	   looks for a corresponding event in the palm database. If it does not
	   exist, that means that it was deleted on the palm, so we need to also
	   delete it from the local calendar.  This handles the case of (D,-).

	In addition to the fast sync, where the last sync was done with this very
	PC and calendar file, there are two special cases: a full and a first sync.
	-) a full sync goes through all records, not just the modified ones. The
	   pilotID setting of the calendar records is used to determine if the
	   record already exists. if yes, the record is just modified.
	-) a first sync completely ignores the pilotID setting of the calendar
	   events. All records are added, so there might be duplicates. The add
	   function for the calendar should check if a similar record already
	   exists, but this is not done yet.

	-) a full sync is done if
	   a) there is a backupdb and a calendar, but the PC id number changed
	   b) it was explicitly requested by pressing the full sync button in KPilot
	   c) the setting "always full sync" was selected in the configuration dlg
	-) a first sync is done if
	   a) either the calendar or the backup DB does not exist.
	   b) the calendar and the backup DB exists, but the sync is done for a
	      different User name
	   c) it was explicitly requested in KPilot
*/

/* virtual */ bool VCalConduitBase::exec()
{
	FUNCTIONSETUP;

	readConfig();

	// don't do a first sync by default in any case, only when explicitly
	// requested, or the backup database or the alendar are empty.
	setFirstSync( false );

	// TODO: Check Full sync and First sync
	bool retrieved = false;
	if ( !openDatabases( dbname(), &retrieved ) ) goto error;
	setFirstSync( retrieved );

	// If we are in testmode we don't need the local calendar. Else a
	// calendar *must* be opened, we want to sync something don't we?
	if (!syncMode().isTest() && !openCalendar() ) goto error;

	// Start processing the sync
	QTimer::singleShot(0, this, SLOT(slotProcess()));
	return true;

error:
	emit logError( i18n( "Could not open the calendar databases." ) );

	KPILOT_DELETE(fCalendar);
	KPILOT_DELETE(fP);
	KPILOT_DELETE(fState);
	return false;
}

void VCalConduitBase::slotProcess() {
	FUNCTIONSETUP;

	// start the current state if necessary
	if( fState && !fState->started() ) {
		fState->startSync( this );
	}

	// Process next record if applicable
	if( hasNextRecord )
	{
		fState->handleRecord( this );
		QTimer::singleShot( 0, this, SLOT( slotProcess() ) );
	}
	// Else finish the current state if there is one
	else if( fState )
	{
		fState->finishSync( this );
		QTimer::singleShot( 0, this, SLOT( slotProcess() ) );
	}
	// No state so sync is finished
	else
	{
		DEBUGKPILOT << fname << ": Sync finished." << endl;
		delayDone();
	}
}

/* virtual */ void VCalConduitBase::readConfig()
{
	config()->readConfig();
	SyncAction::ConflictResolution res = (SyncAction::ConflictResolution)
		(config()->conflictResolution());
	setConflictResolution( res );
}

static void listResources( KCal::CalendarResources *p )
{
	FUNCTIONSETUP;
	KCal::CalendarResourceManager *manager = p->resourceManager();

	DEBUGKPILOT << fname << ": Resources in calendar:" << endl;
	KCal::CalendarResourceManager::Iterator it;
	for( it = manager->begin(); it != manager->end(); ++it )
	{
		DEBUGKPILOT << fname << ": " << (*it)->resourceName() << endl;
	}
}

/* virtual */ bool VCalConduitBase::openCalendar()
{
	FUNCTIONSETUP;

	KConfig korgcfg( locate( "config", CSL1("korganizerrc") ) );

	// this part taken from adcalendarbase.cpp:
	korgcfg.setGroup( "Time & Date" );
	QString tz(korgcfg.readEntry( "TimeZoneId" ) );

	DEBUGKPILOT << fname << ": KOrganizer's time zone = " << tz << endl;

	// Need a subclass ptr. for the ResourceCalendar methods
	KCal::CalendarResources *rescal = 0L;

	DEBUGKPILOT << fname << ": Got calendar type " << config()->calendarType()
		<< endl;

	switch(config()->calendarType())
	{
		case VCalConduitSettings::eCalendarLocal:
		{
			DEBUGKPILOT << fname << "Using CalendarLocal, file = "
				<< config()->calendarFile() << endl;

			if ( config()->calendarFile().isEmpty() )
			{
				DEBUGKPILOT << fname << "Empty calendar file name." << endl;

				emit logError( i18n( "You selected to sync with an iCalendar"
						" file, but did not give a filename. Please select a"
						" valid file name in the conduit's configuration"
						" dialog" ) );
				return false;
			}

			fCalendar = new KCal::CalendarLocal( tz );
			if ( !fCalendar )
			{
				WARNINGKPILOT
					<< "Cannot initialize calendar object for file "
					<< config()->calendarFile() << endl;
				return false;
			}

			DEBUGKPILOT << fname << "Calendar's timezone: "
				<< fCalendar->timeZoneId() << endl;
			DEBUGKPILOT << fname << "Calendar is local time: "
				<< fCalendar->isLocalTime() << endl;

			emit logMessage( fCalendar->isLocalTime() ?
				i18n( "Using local time zone: %1" ).arg( tz ) :
				i18n( "Using non-local time zone: %1" ).arg( tz ) );

			KURL kurl( config()->calendarFile() );
			if( !KIO::NetAccess::download( config()->calendarFile(),
				fCalendarFile, 0L ) && !kurl.isLocalFile() )
			{
				emit logError(i18n( "You chose to sync with the file \"%1\", which "
					"cannot be opened. Please make sure to supply a "
					"valid file name in the conduit's configuration dialog. "
					"Aborting the conduit." ).arg( config()->calendarFile() ) );
				KIO::NetAccess::removeTempFile( fCalendarFile );
				return false;
			}

			// if there is no calendar yet, use a first sync..
			// the calendar is initialized, so nothing more to do...
			if (!dynamic_cast<KCal::CalendarLocal*>(fCalendar)->load(fCalendarFile) )
			{
				DEBUGKPILOT << fname << "Calendar file " << fCalendarFile
					<< " could not be opened. Will create a new one" << endl;

				// Try to create empty file. if it fails,
				// no valid file name was given.
				QFile fl(fCalendarFile);
				if (!fl.open(IO_WriteOnly | IO_Append))
				{
					DEBUGKPILOT << fname << "Invalid calendar file name "
						<< fCalendarFile << endl;

					emit logError( i18n( "You chose to sync with the file \"%1\", which "
						"cannot be opened or created. Please make sure to supply a "
						"valid file name in the conduit's configuration dialog. "
						"Aborting the conduit." ).arg( config()->calendarFile() ) );
					return false;
				}
				fl.close();
				setFirstSync( true );
			}
			addSyncLogEntry( i18n( "Syncing with file \"%1\"" )
				.arg( config()->calendarFile() ) );
			break;
		}

		case VCalConduitSettings::eCalendarResource:
			DEBUGKPILOT << "Using CalendarResource!" << endl;

			rescal = new KCal::CalendarResources( tz );
			listResources(rescal);
			fCalendar = rescal;
			if ( !fCalendar)
			{
				WARNINGKPILOT << "Cannot initialize calendar " <<
					"object for ResourceCalendar" << endl;
				return false;
			}

#if LIBKCAL_IS_VERSION(1,1,0)
			rescal->readConfig();
			rescal->load();
#else
#warning "Timezone bug is present."
#endif
			addSyncLogEntry( i18n( "Syncing with standard calendar resource." ) );
			emit logMessage( fCalendar->isLocalTime() ?
				i18n( "Using local time zone: %1" ).arg( tz ) :
				i18n( "Using non-local time zone: %1" ).arg( tz ) );
			break;
		default:
			break;
	}

	if ( !fCalendar )
	{
		WARNINGKPILOT << "Unable to initialize calendar object."
			<< " Please check the conduit's setup." << endl;
		emit logError( i18n( "Unable to initialize the calendar object. Please"
			" check the conduit's setup") );
		return false;
	}
	fP = createPrivateCalendarData( fCalendar );
	if ( !fP )
	{
		return false;
	}
	int rc = fP->updateIncidences();
	DEBUGKPILOT << fname << ": return from updateIncidences: [" << rc
		<< "]" << endl;

	if ( fP->count() < 1 )
	{
		setFirstSync( true );
	}

	return true;
}

KCal::Incidence* VCalConduitBase::addRecord( PilotRecord *r )
{
	FUNCTIONSETUP;

	recordid_t id = fLocalDatabase->writeRecord( r );
	DEBUGKPILOT<<fname<<": Pilot Record ID = " << r->id() << ", backup ID = "
		<< id << endl;

	PilotRecordBase *de = newPilotEntry( r );
	KCal::Incidence*e = 0L;

	if ( de )
	{
		e = fP->findIncidence( r->id() );
		if ( !e )
		{
			// no corresponding entry found, so create, copy and insert it.
			e = newIncidence();
			incidenceFromRecord( e, de );
			fP->addIncidence( e );
			fCtrPC->created();
		}
		else
		{
			// similar entry found, so just copy, no need to insert again
			incidenceFromRecord( e, de );
			fCtrPC->updated();
		}
	}
	KPILOT_DELETE( de );
	return e;
}

int VCalConduitBase::resolveConflict( KCal::Incidence *e, PilotRecordBase *de ) {
	if ( getConflictResolution() == SyncAction::eAskUser )
	{
		// TODO: This is messed up!!!
		QString query = i18n( "The following item was modified "
			"both on the Handheld and on your PC:\nPC entry:\n\t" );
		query += e->summary();
		query += i18n( "\nHandheld entry:\n\t" );
		query += getTitle( de );
		query += i18n( "\n\nWhich entry do you want to keep? It will "
			"overwrite the other entry." );

		return KMessageBox::No == questionYesNo(
			query,
			i18n( "Conflicting Entries" ),
			QString::null,
			0 /* Never timeout */,
			i18n( "Handheld" ), i18n( "PC" ));
	}
	return getConflictResolution();
}

KCal::Incidence*VCalConduitBase::changeRecord(PilotRecord *r,PilotRecord *)
{
	FUNCTIONSETUP;

	PilotRecordBase *de = newPilotEntry( r );
	KCal::Incidence *e = fP->findIncidence( r->id() );

	DEBUGKPILOT << fname << ": Pilot Record ID: [" << r->id() << "]" << endl;

	if ( e && de )
	{
		// TODO: check for conflict, and if there is one, ask for resolution
		if ( ( e->syncStatus() != KCal::Incidence::SYNCNONE )
			&& r->isModified() )
		{
			// TODO: I have not yet found a way to complete ignore an item
			if (resolveConflict( e, de ) )
			{
				// PC record takes precedence:
				KPILOT_DELETE( de );
				return e;
			}
		}
		// no conflict or conflict resolution says, Palm overwrites, so do it:
		incidenceFromRecord( e, de );

		// NOTE: This MUST be done last, since every other set* call
		// calls updated(), which will trigger an
		// setSyncStatus(SYNCMOD)!!!
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		fLocalDatabase->writeRecord( r );
	}
	else
	{
		WARNINGKPILOT
			<< "While changing record -- not found in iCalendar" << endl;
		addRecord( r );
	}

	KPILOT_DELETE( de );
	return e;
}


KCal::Incidence*VCalConduitBase::deleteRecord( PilotRecord *r, PilotRecord * )
{
	FUNCTIONSETUP;

	KCal::Incidence *e = fP->findIncidence(r->id());
	if (e)
	{
		// RemoveEvent also takes it out of the calendar.
		fP->removeIncidence(e);
		fCtrPC->deleted();
	}
	fLocalDatabase->writeRecord( r );
	return NULL;
}


void VCalConduitBase::addPalmRecord( KCal::Incidence *e )
{
	FUNCTIONSETUP;

	PilotRecordBase *de = newPilotEntry( 0L );
	updateIncidenceOnPalm( e, de );
	fCtrHH->created();
	KPILOT_DELETE( de );
}


void VCalConduitBase::changePalmRecord(KCal::Incidence*e, PilotRecord*s)
{
	PilotRecordBase *de = newPilotEntry( s );
	updateIncidenceOnPalm( e, de );
	fCtrHH->updated();
	KPILOT_DELETE( de );
}


void VCalConduitBase::deletePalmRecord( KCal::Incidence *e, PilotRecord *s )
{
	FUNCTIONSETUP;
	if ( s )
	{
		DEBUGKPILOT << fname << ": deleting record " << s->id() << endl;
		s->setDeleted();
		fDatabase->writeRecord( s );
		fLocalDatabase->writeRecord( s );
		fCtrHH->deleted();
	}
	else
	{
		DEBUGKPILOT << fname << ": could not find record to delete (";
		DEBUGKPILOT << e->pilotId() << ")" << endl;
	}

	Q_UNUSED(e);
}

/* I have to use a pointer to an existing PilotDateEntry so that I can handle
   new records as well (and to prevent some crashes concerning the validity
   domain of the PilotRecord*r). In syncEvent this PilotDateEntry is created. */
void VCalConduitBase::updateIncidenceOnPalm( KCal::Incidence *e,
	PilotRecordBase *de )
{
	FUNCTIONSETUP;
	if ( !de || !e ) {
		DEBUGKPILOT << fname << ": NULL event given... Skipping it" << endl;
		return;
	}

	if ( e->syncStatus() == KCal::Incidence::SYNCDEL )
	{
		DEBUGKPILOT << fname << ": don't write deleted incidence "
			<< e->summary() << " to the palm" << endl;
		return;
	}

	PilotRecord *r = recordFromIncidence( de, e );

	// TODO: Check for conflict!
	if ( r )
	{
		recordid_t id=fDatabase->writeRecord(r);
		r->setID(id);
//		r->setAttrib(r->getAttrib() & ~dlpRecAttrDeleted);
		fLocalDatabase->writeRecord( r );
//		fDatabase->writeRecord(r);
		e->setPilotId( id );

		// NOTE: This MUST be done last, since every other set* call
		// calls updated(), which will trigger an
		// setSyncStatus(SYNCMOD)!!!
		e->setSyncStatus(KCal::Incidence::SYNCNONE);
		KPILOT_DELETE( r );
	}
}

const QString VCalConduitBase::dbname()
{
    return QString::null;
}

PilotRecord *VCalConduitBase::readRecordByIndex( int index )
{
	FUNCTIONSETUP;
	return fDatabase->readRecordByIndex( index );
}

KCal::Incidence *VCalConduitBase::incidenceFromRecord( PilotRecord *r )
{
	FUNCTIONSETUP;
	PilotRecordBase *pac = newPilotEntry( r );
	KCal::Incidence *i = newIncidence();
	incidenceFromRecord( i, pac );

	KPILOT_DELETE( pac );
	return i;
}

void VCalConduitBase::setState( ConduitState *s )
{
	KPILOT_DELETE( fState );
	fState = s;
}

/* virtual */ void VCalConduitBase::postSync( )
{
	FUNCTIONSETUP;
	if (fCtrPC && fP)
		fCtrPC->setEndCount(fP->count());
}

/* virtual */ void VCalConduitBase::preSync( )
{
	FUNCTIONSETUP;
	if (fCtrPC && fP)
		fCtrPC->setStartCount(fP->count());
}
