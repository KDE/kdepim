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

#include <qtimer.h>

#include <kconfig.h>
#include <kmessagebox.h>

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
void MultiDBConduit::syncNextRecod() {
};


void MultiDBConduit::syncNextDB() {
	FUNCTIONSETUP;
	DBInfo dbinfo;
	// just to make sure
	cleanupDB();
	if (fHandle->getNextDatabase(++dbnr, &dbinfo/*, cardno*/)<=0) {
		// no more databases available, so cleanup
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
	
	// db could be opened, so sync, sync sync...
	if (!fConfig) {
		kdWarning() << k_funcinfo <<": No configuration set for conduit. Skipping database "<<dbinfo.name<<"..."<<endl;
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}
	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
		
	if (!isCorrectDBTypeCreator(dbinfo)) {  // if creator and/or type don't match, go to next db
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}
	// read config entry for the db, will also ask if needed
	// if the user cancels, just skip this database...
	if (!GetSyncType(dbinfo, &syncinfo))  {
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}
	dbases.append(dbinfo.name);
	#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Using file " << syncinfo.filename << " to sync the palm database "<< dbinfo.name << endl;
	#endif
		
	// preSyncAction should initialize the custom databases/files for the specific action chosen for this db
	if (!preSyncAction(&syncinfo)) {
		#ifdef DEBUG
		DEBUGCONDUIT << "Error initializing the databases for "<<dbinfo.name<<endl;
		#endif
		// TODO: write out a real KPilot error message to be included in the log
		kdWarning() << k_funcinfo << ": Couldn't initialize the database "<<dbinfo.name<<endl;
		emit logError(i18n("Couldn't initialize the database ")+dbinfo.name);
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}


	// open the db on palm, backup db on harddisk and the calendar file
	fCurrentDatabase = new PilotSerialDatabase(pilotSocket(), dbinfo.name, this, dbinfo.name );
	fBackupDatabase = new PilotLocalDatabase(dbinfo.name);

	// error messages if any of them could not be loaded
	if (!fCurrentDatabase)
		kdWarning() << k_funcinfo << ": Couldn't open database "<<dbinfo.name<<" on Pilot" << endl;
	if (!fBackupDatabase)
		kdWarning() << k_funcinfo << ": Couldn't open local copy for "<<dbinfo.name << endl;

	if (!fCurrentDatabase || !fBackupDatabase || !fCurrentDatabase->isDBOpen() || !fBackupDatabase->isDBOpen()) {
		emit logError(i18n("Couldn't open the calendar databases."));
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}


	// Start the sync by emitting syncRecord()
	QTimer::singleShot(0,this,SLOT(syncNextRecord()));
	// TODO: maybe we should also set a timer at, say 5 Minutes so that the conduit won't wait forever...
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
		if (KMessageBox::warningContinueCancel (0, i18n("You did not select any "
				"sync method for the database. If you continue, the database will be "
				"skipped during this sync, if you cancel, you will get back to the "
				"previous dialog."),
				i18n("&Continue"), KStdGuiItem::cont(), "noEmptySyncSettingWarning") ) {
			return false;
		}
	}
	return false;
}



/* virtual */ void MultiDBConduit::exec() {
	FUNCTIONSETUP;

	if (!fConfig) {
		kdWarning() << k_funcinfo <<": No configuration set for conduit. SKIPPING..."<<endl;
		return;
	}

	KConfigGroupSaver cfgs(fConfig, conduitSettingsGroup());
	
	dbases=fConfig->readListEntry(settingsFileList());
	
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
	return;
}

// $Log$
// Revision 1.1  2002/04/07 12:09:42  kainhofe
// Initial checkin of the conduit. The gui works mostly, but syncing crashes KPilot...
//
// Revision 1.2  2002/04/07 11:56:19  reinhold
// Last version before moving to KDE CVS
//
// Revision 1.1  2002/04/07 01:03:52  reinhold
// the list of possible actions is now created dynamically
//
// Revision 1.9  2002/04/05 21:17:00  reinhold
// *** empty log message ***
//
// Revision 1.8  2002/03/28 13:47:53  reinhold
// Added the list of synctypes, aboutbox is now directly passed on to the setup dlg (instead of being a static var)
//
// Revision 1.6  2002/03/23 21:46:42  reinhold
// config  dlg works, but the last changes crash the plugin itself
//
// Revision 1.5  2002/03/23 18:21:14  reinhold
// Cleaned up the structure. Works with QTimer instead of loops.
//
// Revision 1.4  2002/03/15 20:43:17  reinhold
// Fixed the crash on loading (member function not defined)...
//
// Revision 1.3  2002/03/10 23:58:32  reinhold
// Made the conduit compile...
//
// Revision 1.2  2002/03/10 16:06:43  reinhold
// Cleaned up the class hierarchy, implemented some more features (should be quite finished now...)
//
// Revision 1.1.1.1  2002/03/09 15:38:45  reinhold
// Initial checin of the  project manager / List manager conduit.
//
//
