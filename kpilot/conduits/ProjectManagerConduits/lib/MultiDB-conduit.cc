/* MultiDB-conduit.cc  MultiDB-Conduit for syncing KPilot and KOrganizer
**
** Copyright (C) 1998-2001 Dan Pilone
** Copyright (C) 1998-2000 Preston Brown
** Copyright (C) 1998 Herwin-Jan Steehouwer
** Copyright (C) 2001 Cornelius Schumacher
** Copyright (C) 2002 by Reinhold Kainhofer
**
** This file is part of the MultiDB conduit, a conduit for KPilot that
** synchronises the Pilot's MultiDB application with the outside world,
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

#if QT_VERSION < 300
#include <qmsgbox.h>
#else
#include <qmessagebox.h>
#endif

#include <unistd.h>
#include <qtimer.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include <pilotUser.h>
#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
#include "MultiDB-conduit.h"
#include "DatabaseAction.h"

using namespace KCal;

static const char *MultiDB_conduit_id = "$Id$";



MultiDBConduit::MultiDBConduit(KPilotDeviceLink *d, 	const char *n, 	const QStringList &l, SyncTypeList_t *tps) : ConduitAction(d,n,l), dbases(), fCurrentDatabase(), fBackupDatabase() {
	FUNCTIONSETUP;
	(void)MultiDB_conduit_id;
	dbnr=0;
	synctypes=tps;
}

void MultiDBConduit::cleanup() {
	FUNCTIONSETUP;
	
	if (!fConfig) return;
	
	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
	fConfig->writeEntry(conduitSettingsGroup(), dbases);
	cleanupDB();
}

void MultiDBConduit::cleanupDB() {
	FUNCTIONSETUP;
	KPILOT_DELETE(fCurrentDatabase);
	KPILOT_DELETE(fBackupDatabase);
}

// if false is returned, the database will be skipped, so use the return value!!!
bool MultiDBConduit::preSyncAction(DBSyncInfo*dbinfo) {
	FUNCTIONSETUP;
	switch (dbinfo->syncaction) {
		case st_ask:
		case st_ignore:
		case st_backup:
			return false;
			break;
	}
	return true;
}

void MultiDBConduit::finishedDB() {
	QTimer::singleShot(0, this, SLOT(syncNextDB()));
};
void MultiDBConduit::syncNextRecord() {
	QTimer::singleShot(0, this, SLOT(finishedDB()));
};


void MultiDBConduit::syncNextDB() {
	FUNCTIONSETUP;
	DBInfo dbinfo;
	KPilotUser*usr;
	// just to make sure
	cleanupDB();
	if (fHandle->getNextDatabase(++dbnr, &dbinfo/*, cardno*/)<=0) {
		// no more databases available, so cleanup
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
	
	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
	// db could be opened, so sync, sync sync...
	if (!fConfig) {
		kdWarning() << k_funcinfo <<": No configuration set for conduit. Skipping database "<<dbinfo.name<<"..."<<endl;
		goto skip;
	}
		
	// if creator and/or type don't match, go to next db
	if (!isCorrectDBTypeCreator(dbinfo)) goto skip;
	// read config entry for the db, will ask if needed. if user cancels, just skip this database...
	if (!GetSyncType(dbinfo, &syncinfo)) goto skip;
	
	dbases.append(dbinfo.name);
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Using file " << syncinfo.filename << " to sync the palm database "<< dbinfo.name << endl;
#endif
		
	// preSyncAction should initialize the custom databases/files for the specific action chosen for this db
	if (!preSyncAction(&syncinfo)) {
#ifdef DEBUG
		DEBUGCONDUIT << "Error initializing the databases for "<<dbinfo.name<<endl;
#endif
		emit logError(i18n("Unable to initialize the database: ")+dbinfo.name);
		goto error;
	}

	usr=fHandle->getPilotUser();
	fFullSync |= (syncinfo.synctype==SYNC_FULL) || ( (usr->getLastSyncPC()!=(unsigned long)gethostid()) && fConfig->readBoolEntry(MultiDBConduitFactory::fullSyncOnPCChange, true));
	

	// open the db on palm, backup db on harddisk and the calendar file
	fBackupDatabase = new PilotLocalDatabase(dbinfo.name);
	if (!fBackupDatabase) goto error;
	
	if (!fBackupDatabase->isDBOpen()) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Backup databse "<<fBackupDatabase->dbPathName()<<" could not be opened. Will fetch a copy from the palm and do a full sync"<<endl;
#endif
		struct DBInfo dbinfo;
		char nm[50];
		strncpy(&nm[0], &dbinfo.name[0], sizeof(nm));
		if (fHandle->findDatabase(&nm[0], &dbinfo)<0 ){
#ifdef DEBUG
			DEBUGCONDUIT<<fname<<": Could not get DBInfo for "<<nm<<"! Skipping databse"<<endl;
#endif
			goto error;
		}
		dbinfo.flags &= ~dlpDBFlagOpen;
		if (!fHandle->retrieveDatabase(fBackupDatabase->dbPathName(), &dbinfo) ) goto error;
		KPILOT_DELETE(fBackupDatabase);
		fBackupDatabase=new PilotLocalDatabase(dbinfo.name);
		if (!fBackupDatabase || ! fBackupDatabase->isDBOpen() ) goto error;
		fFullSync=true;
	}

	fCurrentDatabase = new PilotSerialDatabase(pilotSocket(), dbinfo.name, this, dbinfo.name );
	
	// error messages if any of them could not be loaded
	if (!fCurrentDatabase || !fBackupDatabase || !fCurrentDatabase->isDBOpen() || !fBackupDatabase->isDBOpen()) {
		emit logError(i18n("Couldn't open the calendar databases."));
		goto skip;
	}


	// Start the sync by emitting syncRecord()
	QTimer::singleShot(0,this,SLOT(syncNextRecord()));
	return;
	
error:
	emit logError(i18n("Sync failed for database: ")+dbinfo.name);
skip:
	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	return;
}


bool MultiDBConduit::GetSyncType(DBInfo dbinfo, DBSyncInfo*syncinfo) {
	fConfig->setGroup(dbinfo.name);
	syncinfo->set(dbinfo.name,
		fConfig->readNumEntry(getSyncTypeEntry(), st_ask),
		fConfig->readEntry(getSyncFileEntry()));
	if (syncinfo->syncaction!=st_ask) return true;
	
	while (true) { // loop until the user either selects a valid sync method or skips the db
		// if sync type is set to st_ask, display the settings dialog...
		DBSettings*actiondlg=new DBSettings(0, "dbsettings", syncinfo, synctypes, false, false); // grayed out items
		int dlgres=actiondlg->exec();
		// we can dispose of the dialog now (we could also keep the dialog, just in case we have to
		// display it again, but then I have no idea when to delete it...
		delete actiondlg;
		if (dlgres==QDialog::Accepted ) {
			// write out the new settings to the config file and return true to proceed with syncing
			fConfig->writeEntry(getSyncTypeEntry(), syncinfo->syncaction);
			fConfig->writeEntry(getSyncFileEntry(), syncinfo->filename);
			if (syncinfo->syncaction!=st_ask) return true;
		}
		// the dialog was cancelled, so display a message box...
		if (KMessageBox::warningContinueCancel (0, i18n("You did not select a "
				"sync method for the database. If you continue, the database will be "
				"skipped during this sync. If you cancel, you will be returned to the "
				"previous dialog."),
				i18n("&Continue"), KStdGuiItem::cont(), "noEmptySyncSettingWarning") ) {
			return false;
		}
	}
	return false;
}



/* virtual */ bool MultiDBConduit::exec() {
	FUNCTIONSETUP;
	DEBUGCONDUIT<<MultiDB_conduit_id<<endl;

	if (!fConfig) {
		kdWarning() << k_funcinfo <<": No configuration set for conduit. SKIPPING..."<<endl;
		return false;
	}

	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
	
	dbases=fConfig->readListEntry(settingsFileList());
	conflictResolution = fConfig->readNumEntry(MultiDBConduitFactory::conflictResolution);
	archive = fConfig->readBoolEntry(MultiDBConduitFactory::archive);
	
		#ifdef DEBUG
		DEBUGCONDUIT<<k_funcinfo<<": starting sync with syncNextDB "<<endl;
		#endif
	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	
	// OLD CODE: BAD RESPONSE TIME...
/*	// Go through all cards on the palm
	int cardno=-1;
	DBInfo dbinfo;
//	CardInfo cardinfo;
//	while (dlp_ReadStorageInfo(pilotSocket(), ++cardno, &cardinfo)>0) {
		// go through databases
		int dbnr=-1;
		while (fHandle->getNextDatabase(++dbnr, &dbinfo)>0) {
			if (!isCorrectDBTypeCreator(dbinfo)) continue; // if creator and/or type don't match, go to next db
			// read config entry for the db, will also ask if needed
			// if the user cancels, just skip this database...
			if (!GetSyncType(dbinfo, &syncinfo)) continue;
			strl << dbinfo.name;
			fDatabase = new PilotSerialDatabase(pilotSocket(), dbinfo.name, this, dbinfo.name );
			if (!fDatabase || !fDatabase->isDBOpen()) {
				kdWarning() << k_funcinfo << ": Couldn't open database "<< dbinfo.name <<"."<< endl;
				KPILOT_DELETE(fDatabase);
//				emit syncDone(this);
				continue;
			}

			if (isTest()) {
				// Nothing: the MultiDB conduit doesn't have a test mode.
			} else if (isBackup()) {
				doBackup();
				fDatabase->resetSyncFlags();
			} else {
				doSync();
				fDatabase->resetSyncFlags();
			}

			KPILOT_DELETE(fDatabase);
		} // end walk through dbs
//	} // end walp through cards
	// cardno and dbnr will contain the nr of cards and the nr of dbs on the last card
	fConfig->writeEntry(settingsFileList(), strl);
	emit syncDone(this);*/
	return true;
}

