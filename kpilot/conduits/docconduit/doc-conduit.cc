// doc-conduit.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
//
// The doc conduit synchronizes text files on the PC with DOC databases on the Palm
//


// naming of the bookmark file:
// PDB->TXT:	convert bookmarks to a .bm file
// TXT->PDB:	If a .bmk file exists, use it, otherwise use the .bm file (from the PDB->TXT conversion)
//			This way, the bookmark file is not overwritten, a manual bookmark file overrides, but the bookmarks from the handheld are still available


#include "options.h"
#include "doc-conduit.moc"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
#include <qtimer.h>
#include <qdir.h>

#include <kconfig.h>
#include <kmdcodec.h> 

#include <pilotLocalDatabase.h>
#include <pilotSerialDatabase.h>

#include "doc-factory.h"
#include "doc-conflictdialog.h"
#include "DOC-converter.h"
#include "pilotDOCHead.h"


// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *doc_conduit_id = "$Id$";

QString dirToString(eSyncDirectionEnum dir) {
	switch(dir) {
//		case eSyncAll: return "eSyncAll";
		case eSyncPDAToPC: return CSL1("eSyncPDAToPC");
		case eSyncPCToPDA: return CSL1("eSyncPCToPDA");
		case eSyncNone: return CSL1("eSyncNone");
		case eSyncConflict: return CSL1("eSyncConflict");
		case eSyncDelete: return CSL1("eSyncDelete");
		default: return CSL1("ERROR");
	}
}


/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/


DOCConduit::DOCConduit(KPilotDeviceLink * o,
	const char *n, const QStringList & a):ConduitAction(o, n, a)
{
	FUNCTIONSETUP;
	(void) doc_conduit_id;
}



DOCConduit::~DOCConduit()
{
	FUNCTIONSETUP;
}


bool DOCConduit::isCorrectDBTypeCreator(DBInfo dbinfo) {
	return dbinfo.type == dbtype() && dbinfo.creator == dbcreator();
};
const unsigned long DOCConduit::dbtype() {
	return get_long(DOCConduitFactory::dbDOCtype);
};
const unsigned long DOCConduit::dbcreator() {
	return get_long(DOCConduitFactory::dbDOCcreator);
};



/*********************************************************************
                L O A D I N G   T H E   D A T A
 *********************************************************************/



void DOCConduit::readConfig()
{
	FUNCTIONSETUP;

	KConfigGroupSaver g(fConfig, DOCConduitFactory::fGroup);

	fDOCDir = fConfig->readEntry(DOCConduitFactory::fDOCDir, QString::null);
	fPDBDir = fConfig->readEntry(DOCConduitFactory::fPDBDir, QString::null);
	fKeepPDBLocally =
		fConfig->readBoolEntry(DOCConduitFactory::fKeepPDBLocally, true);
	eConflictResolution =
		(enum eSyncDirectionEnum) (fConfig->
		readNumEntry(DOCConduitFactory::fConflictResolution, 0));
	fBookmarks = DOCConverter::eBmkNone;
	if (fConfig->readBoolEntry(DOCConduitFactory::fConvertBookmarks, true))
	{
		if (fConfig->readBoolEntry(DOCConduitFactory::fBookmarksBmk, true))
			fBookmarks |= DOCConverter::eBmkFile;
		if (fConfig->readBoolEntry(DOCConduitFactory::fBookmarksInline, true))
			fBookmarks |= DOCConverter::eBmkInline;
		if (fConfig->readBoolEntry(DOCConduitFactory::fBookmarksEndtags, true))
			fBookmarks |= DOCConverter::eBmkEndtags;
	}
	fCompress = fConfig->readBoolEntry(DOCConduitFactory::fCompress, true);
	eSyncDirection =
		(enum eSyncDirectionEnum) (fConfig->
		readNumEntry(DOCConduitFactory::fSyncDirection, 1));
		
	fIgnoreBmkChangesOnly = fConfig->readBoolEntry(DOCConduitFactory::fIgnoreBmkChanges, false);
	fLocalSync = fConfig->readBoolEntry(DOCConduitFactory::fLocalSync, false);
	fAlwaysUseResolution = fConfig->readBoolEntry(DOCConduitFactory::fAlwaysUseResolution, false);
	
	fDBListSynced=fConfig->readListEntry(DOCConduitFactory::fDOCList);

#ifdef DEBUG
	DEBUGCONDUIT << fname
		<< ": Settings "
		<< " fdocdir=" << fDOCDir
		<< " fPDBDir=" << fPDBDir
		<< " fkeepPDBLocally=" << fKeepPDBLocally
		<< " eConflictResolution=" << eConflictResolution
		<< " fBookmarks=" << fBookmarks
		<< " fCompress=" << fCompress
		<< " eSyncDirection=" << eSyncDirection << endl;
#endif
}

bool DOCConduit::textChanged(QString docfn)
{
	KConfigGroupSaver g(fConfig, DOCConduitFactory::fGroup);
	
	// How do I find out if a text file has changed shince we last synced it??
	// Use KMD5 for now. If I realize it is too slow, then I have to go back to comparing modification times
	// if there is no config setting yet, assume the file has been changed. the md5 sum will be written to the config file after the sync.
	QString oldDigest=fConfig->readEntry(docfn);
	if (oldDigest.length()<=0) 
	{
		return true;
	}
#ifdef DEBUG
	DEBUGCONDUIT<<"Old digest is "<<oldDigest<<endl;
#endif
	
	KMD5 docmd5;
	QFile docfile(docfn);
	if (docfile.open(IO_ReadOnly)){
		docmd5.update(docfile);
		QString thisDigest(docmd5.hexDigest() /* .data() */);
#ifdef DEBUG
		DEBUGCONDUIT<<"New digest is "<<thisDigest<<endl;
#endif
		return (thisDigest.length()<=0) || (thisDigest!=oldDigest);
	} else {
		// File does not exist. This should actually never happen. Anyways, just return true to indicate it has changed. 
		// doSync should detect this and delete the doc from the handheld.
		return true;
	}
	return false;
}




/*********************************************************************
 *     Helper functions
 ********************************************************************/

QString DOCConduit::constructPDBFileName(QString name) {
	FUNCTIONSETUP;
	QString fn;
	QDir dr(fPDBDir);
	QFileInfo pth(dr, name);
	if (!name.isEmpty()) fn=pth.absFilePath()+CSL1(".pdb");
	return fn;
}
QString DOCConduit::constructDOCFileName(QString name) {
	FUNCTIONSETUP;
	QString fn;
	QDir dr(fDOCDir);
	QFileInfo pth(dr, name);
	if (!name.isEmpty()) fn=pth.absFilePath()+CSL1(".txt");
	return fn;
}





/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/





/* virtual */ bool DOCConduit::exec()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<"Conduit version: "<<doc_conduit_id<<endl;
#endif

	if (!fConfig) {
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		return false;
	}
	readConfig();
	dbnr=0;
	
	emit logMessage(i18n("Searching for texts and databases to syncronize"));

	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	return true;
}



bool DOCConduit::doSync(docSyncInfo &sinfo) {
	bool res=false;
	
	if (sinfo.direction==eSyncDelete) {
		if (!sinfo.docfilename.isEmpty()) {
			if (!QFile::remove(sinfo.docfilename)) {
				kdWarning()<<i18n("Unable to delete the text file \"%1\" on the PC").arg(sinfo.docfilename)<<endl;
			}
			QString bmkfilename = sinfo.docfilename;
			if (bmkfilename.endsWith(CSL1(".txt"))){
				bmkfilename.remove(bmkfilename.length()-4, 4);
			}
			bmkfilename+=CSL1(PDBBMK_SUFFIX);
			if (!QFile::remove(bmkfilename)) {
#ifdef DEBUG
				DEBUGCONDUIT<<"Could not remove bookmarks file "<<bmkfilename<<" for database "<<sinfo.handheldDB<<endl;
#endif
			}
		}
		if (!sinfo.pdbfilename.isEmpty() && fKeepPDBLocally) {
			PilotLocalDatabase*database=new PilotLocalDatabase(fPDBDir, 
				QString::fromLatin1(sinfo.dbinfo.name), false);
			if (database) {
				if ( database->deleteDatabase() !=0 ) {
					kdWarning()<<i18n("Unable to delete database \"%1\" on the PC")
						.arg(QString::fromLatin1(sinfo.dbinfo.name))<<endl;
				}
				KPILOT_DELETE(database);
			}
		}
		if (!fLocalSync) {
			PilotDatabase *database=new PilotSerialDatabase(pilotSocket(), 
				QString::fromLatin1(sinfo.dbinfo.name));
			if ( database->deleteDatabase() !=0 ) {
				kdWarning()<<i18n("Unable to delete database \"%1\" from the handheld")
					.arg(QString::fromLatin1(sinfo.dbinfo.name))<<endl;
			}
			KPILOT_DELETE(database);
		}
		return true;
	}
	// preSyncAction should initialize the custom databases/files for the
	// specific action chosen for this db and return a pointer to a docDBInfo
	// instance which points either to a local database or a database on the handheld.
	PilotDatabase *database = preSyncAction(sinfo);

	if (database && ( !database->isDBOpen() ) ) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Database "<<sinfo.dbinfo.name<<" does not yet exist. Creating it:"<<endl;
#endif
		if (!database->createDatabase(dbcreator(), dbtype()) ) {
#ifdef DEBUG
			DEBUGCONDUIT<<"Failed"<<endl;
#endif
		}
	}

	if (database && database->isDBOpen()) {
		DOCConverter docconverter;
		connect(&docconverter, SIGNAL(logError(const QString &)), SIGNAL(logError(const QString &)));
		connect(&docconverter, SIGNAL(logMessage(const QString &)), SIGNAL(logMessage(const QString &)));

		docconverter.setDOCpath(fDOCDir, sinfo.docfilename);
		docconverter.setPDB(database);
		docconverter.setBookmarkTypes(fBookmarks);
		docconverter.setCompress(fCompress);

		switch (sinfo.direction) {
			case eSyncPDAToPC:
				res = docconverter.convertPDBtoDOC();
				break;
			case eSyncPCToPDA:
				res = docconverter.convertDOCtoPDB();
				break;
			default:
				break;
		}
		
		// Now calculate the md5 checksum of the PC text and write it to the config file
		{
			KConfigGroupSaver g(fConfig, DOCConduitFactory::fGroup);
			KMD5 docmd5;
			QFile docfile(docconverter.docFilename());
			if (docfile.open(IO_ReadOnly)) {
				docmd5.update(docfile);
				QString thisDigest(docmd5.hexDigest() /* .data() */);
				fConfig->writeEntry(docconverter.docFilename(), thisDigest);
				fConfig->sync();
#ifdef DEBUG
				DEBUGCONDUIT<<"MD5 Checksum of the text "<<sinfo.docfilename<<" is "<<thisDigest<<endl;
#endif
			} else {
#ifdef DEBUG
				DEBUGCONDUIT<<"couldn't open file "<<docconverter.docFilename()<<" for reading!!!"<<endl;
#endif
			}
		}
		
		if (!postSyncAction(database, sinfo, res)) 
			emit logError(i18n("Unable to install the locally created PalmDOC %1 to the handheld.")
				.arg(QString::fromLatin1(sinfo.dbinfo.name)));
		if (!res)
			emit logError(i18n("Conversion of PalmDOC \"%1\" failed.")
				.arg(QString::fromLatin1(sinfo.dbinfo.name)));
//		disconnect(&docconverter, SIGNAL(logError(const QString &)), SIGNAL(logError(const QString &)));
//		disconnect(&docconverter, SIGNAL(logMessage(const QString &)), SIGNAL(logMessage(const QString &)));
//		KPILOT_DELETE(database);
	}
	else
	{
		emit logError(i18n("Unable to open or create the database %1")
			.arg(QString::fromLatin1(sinfo.dbinfo.name)));
	}
	return res;
}


/** syncNextDB walks through all PalmDoc databases on the handheld and decides if they are supposed to be synced to the PC. 
 * syncNextDB and syncNextDOC fist build the list of all PalmDoc texts, and then the method syncDatabases does the actual sync. */
void DOCConduit::syncNextDB() {
	FUNCTIONSETUP;
	DBInfo dbinfo;

	if (eSyncDirection==eSyncPCToPDA || fHandle->findDatabase(NULL, &dbinfo, dbnr, dbtype(), dbcreator() /*, cardno */ ) < 0)
	{
		// no more databases available, so check for PC->Palm sync
		QTimer::singleShot(0, this, SLOT(syncNextDOC()));
		return;
	}
	dbnr=dbinfo.index+1;
#ifdef DEBUG
	DEBUGCONDUIT<<"Next Palm database to sync: "<<dbinfo.name<<", Index="<<dbinfo.index<<endl;
#endif

	// if creator and/or type don't match, go to next db
	if (!isCorrectDBTypeCreator(dbinfo) || 
		fDBNames.contains(QString::fromLatin1(dbinfo.name)))
	{
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}

	QString docfilename=constructDOCFileName(QString::fromLatin1(dbinfo.name));
	QString pdbfilename=constructPDBFileName(QString::fromLatin1(dbinfo.name));

	docSyncInfo syncInfo(QString::fromLatin1(dbinfo.name), 
		docfilename, pdbfilename, eSyncNone);
	syncInfo.dbinfo=dbinfo;
	needsSync(syncInfo);
	fSyncInfoList.append(syncInfo);
	fDBNames.append(QString::fromLatin1(dbinfo.name));
	
	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	return;
}



void DOCConduit::syncNextDOC()
{
	FUNCTIONSETUP;
	
	if (eSyncDirection==eSyncPDAToPC  )
	{
		// We don't sync from PC to PDB, so start the conflict resolution and then the actual sync process
		docnames.clear();
		QTimer::singleShot(0, this, SLOT(checkPDBFiles()));
		return;
	}
	
	// if docnames isn't initialized, get a list of all *.txt files in fDOCDir
	if (docnames.isEmpty()/* || dociterator==docnames.end() */) {
		docnames=QDir(fDOCDir, CSL1("*.txt")).entryList() ;
		dociterator=docnames.begin();
	}
	if (dociterator==docnames.end()) {
		// no more databases available, so start the conflict resolution and then the actual sync proces
		docnames.clear();
		QTimer::singleShot(0, this, SLOT(checkPDBFiles()));
		return;
	}

	QString fn=(*dociterator);

	QDir dr(fDOCDir);
	QFileInfo fl(dr, fn );
	QString docfilename=fl.absFilePath();
	QString pdbfilename;
	dociterator++;
	
	DBInfo dbinfo;
	// Include all "extensions" except the last. This allows full stops inside the database name (e.g. abbreviations)
	// first fill everything with 0, so we won't have a buffer overflow.
	memset(&dbinfo.name[0], 0, 33);
	strncpy(&dbinfo.name[0], fl.baseName(TRUE).latin1(), 30);

	bool alreadySynced=fDBNames.contains(fl.baseName(TRUE));
	if (!alreadySynced) {
		docSyncInfo syncInfo(QString::fromLatin1(dbinfo.name), 
			docfilename, pdbfilename, eSyncNone);
		syncInfo.dbinfo=dbinfo;
		needsSync(syncInfo);
		fSyncInfoList.append(syncInfo);
		fDBNames.append(QString::fromLatin1(dbinfo.name));
	} else {
#ifdef DEBUG
		DEBUGCONDUIT<<docfilename<<" has already been synced, skipping it."<<endl;
#endif
	}
	
	QTimer::singleShot(0, this, SLOT(syncNextDOC()));
	return;
}



/** This slot will only be used if fKeepPDBLocally to check if new doc databases have been copied to the pdb directory.
 *  If so, install it to the handheld and sync it to the PC */
void DOCConduit::checkPDBFiles() {
	FUNCTIONSETUP;
	
	if (fLocalSync || !fKeepPDBLocally || eSyncDirection==eSyncPCToPDA )
	{
		// no more databases available, so check for PC->Palm sync
		QTimer::singleShot(0, this, SLOT(resolve()));
		return;
	}
	
	// Walk through all files in the pdb directory and check if it has already been synced.
	// if docnames isn't initialized, get a list of all *.pdb files in fPDBDir
	if (docnames.isEmpty()/* || dociterator==docnames.end() */) {
		docnames=QDir(fPDBDir, CSL1("*.pdb")).entryList() ;
		dociterator=docnames.begin();
	}
	if (dociterator==docnames.end()) {
		// no more databases available, so start the conflict resolution and then the actual sync proces
		docnames.clear();
		QTimer::singleShot(0, this, SLOT(resolve()));
		return;
	}

	QString fn=(*dociterator);

	QDir dr(fPDBDir);
	QFileInfo fl(dr, fn );
	QString pdbfilename=fl.absFilePath();
	dociterator++;
	
	//  Get the doc title and check if it has already been synced (in the synced docs list of in fDBNames to be synced)
	// If the doc title doesn't appear in either list, install it to the Handheld, and add it to the list of dbs to be synced.
	QString dbname=fl.baseName(TRUE).left(30);
	if (!fDBNames.contains(dbname) && !fDBListSynced.contains(dbname)) {
		if (fHandle->installFiles(pdbfilename )) {
			DBInfo dbinfo;
			// Include all "extensions" except the last. This allows full stops inside the database name (e.g. abbreviations)
			// first fill everything with 0, so we won't have a buffer overflow.
			memset(&dbinfo.name[0], 0, 33);
			strncpy(&dbinfo.name[0], dbname.latin1(), 30);

			docSyncInfo syncInfo(dbname, constructDOCFileName(dbname), pdbfilename, eSyncNone);
			syncInfo.dbinfo=dbinfo;
			needsSync(syncInfo);
			fSyncInfoList.append(syncInfo);
			fDBNames.append(dbname);
		} else {
#ifdef DEBUG
			DEBUGCONDUIT<<"Could not install database "<<dbname<<" ("<<pdbfilename<<") to the handheld"<<endl;
#endif
		}
	}
	
	QTimer::singleShot(0, this, SLOT(checkPDBFiles()));
}



void DOCConduit::resolve() {
	FUNCTIONSETUP;
	
	for (fSyncInfoListIterator=fSyncInfoList.begin(); fSyncInfoListIterator!=fSyncInfoList.end(); fSyncInfoListIterator++) {
		// Walk through each database and apply the conflictResolution option. 
		// the remaining conflicts will be resolved in the resolution dialog
		if ((*fSyncInfoListIterator).direction==eSyncConflict){
#ifdef DEBUG
			DEBUGCONDUIT<<"We have a conflict for "<<(*fSyncInfoListIterator).handheldDB<<", default="<<eConflictResolution<<endl;
#endif
			switch (eConflictResolution)
			{
				case eSyncPDAToPC:
#ifdef DEBUG
					DEBUGCONDUIT<<"PDA overrides for database "<<(*fSyncInfoListIterator).handheldDB<<endl;
#endif
					(*fSyncInfoListIterator).direction = eSyncPDAToPC;
					break;
				case eSyncPCToPDA:
#ifdef DEBUG
					DEBUGCONDUIT<<"PC overrides for database "<<(*fSyncInfoListIterator).handheldDB<<endl;
#endif
					(*fSyncInfoListIterator).direction = eSyncPCToPDA;
					break;
				case eSyncNone:
#ifdef DEBUG
					DEBUGCONDUIT<<"No sync for database "<<(*fSyncInfoListIterator).handheldDB<<endl;
#endif
					(*fSyncInfoListIterator).direction = eSyncNone;
					break;
				case eSyncDelete:
				case eSyncConflict:
				default:
#ifdef DEBUG
					DEBUGCONDUIT<<"Conflict remains due to default resolution setting for database "<<(*fSyncInfoListIterator).handheldDB<<endl;
#endif
					break;
			}
		}
	}
	
	// Show the conflict resolution dialog and ask for the action for each database
	ResolutionDialog*dlg=new ResolutionDialog( 0,  i18n("Conflict Resolution"), &fSyncInfoList , fHandle);
	bool show=fAlwaysUseResolution || (dlg && dlg->hasConflicts);
	if (show) {
		if (!dlg || !dlg->exec() ) {
			KPILOT_DELETE(dlg)
			emit logMessage(i18n("Sync aborted by user."));
			QTimer::singleShot(0, this, SLOT(cleanup()));
			return;
		}
	}
	KPILOT_DELETE(dlg)
	

	// fDBNames will be filled with the names of the databases that are actually synced (not deleted), so I can write the list to the config file
	fDBNames.clear();
	fSyncInfoListIterator=fSyncInfoList.begin();
	QTimer::singleShot(0,this, SLOT(syncDatabases()));
	return;
}



void DOCConduit::syncDatabases() {
	FUNCTIONSETUP;
	if (fSyncInfoListIterator==fSyncInfoList.end()) {
		// We're done, so clean up
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
	
	docSyncInfo sinfo=(*fSyncInfoListIterator);
	fSyncInfoListIterator++;
	
	switch (sinfo.direction) {
		case eSyncConflict:
#ifdef DEBUG
			DEBUGCONDUIT<<"Entry "<<sinfo.handheldDB<<"( docfilename: "<<sinfo.docfilename<<
				", pdbfilename: "<<sinfo.pdbfilename<<") had sync direction eSyncConflict!!!"<<endl;
#endif
			break;
		case eSyncDelete:
		case eSyncPDAToPC:
		case eSyncPCToPDA:
			emit logMessage(i18n("Synchronizing text \"%1\"").arg(sinfo.handheldDB));
			if (!doSync(sinfo)) {
				// The sync could not be done, so inform the user (the error message should probably issued inside doSync)
#ifdef DEBUG
				DEBUGCONDUIT<<"There was some error syncing the text \""<<sinfo.handheldDB<<"\" with the file "<<sinfo.docfilename<<endl;
#endif
			}
			break;
		case eSyncNone:
//		case eSyncAll:
			break;
	}
	if (sinfo.direction != eSyncDelete) fDBNames.append(sinfo.handheldDB);
	
	QTimer::singleShot(0,this, SLOT(syncDatabases()));
	return;
}


PilotDatabase*DOCConduit::openDOCDatabase(QString dbname) {
	if (fLocalSync) return new PilotLocalDatabase(fPDBDir, dbname, false);
	else return new PilotSerialDatabase(pilotSocket(), dbname);
}


bool DOCConduit::needsSync(docSyncInfo &sinfo)
{
	FUNCTIONSETUP;
	sinfo.direction = eSyncNone;
	
	PilotDatabase*docdb=openDOCDatabase(QString::fromLatin1(sinfo.dbinfo.name));
	if (!fDBListSynced.contains(sinfo.handheldDB)) {
		// the database wasn't included on last sync, so it has to be new.
#ifdef DEBUG
		DEBUGCONDUIT<<"Database "<<sinfo.dbinfo.name<<" wasn't included in the previous sync!"<<endl;
#endif

		if (QFile::exists(sinfo.docfilename)) sinfo.fPCStatus=eStatNew;
		else sinfo.fPCStatus=eStatDoesntExist;
		if (docdb && docdb->isDBOpen()) sinfo.fPalmStatus=eStatNew;
		else sinfo.fPalmStatus=eStatDoesntExist;
		KPILOT_DELETE(docdb);
		
		if (sinfo.fPCStatus==eStatNew && sinfo.fPalmStatus==eStatNew) {
			sinfo.direction=eSyncConflict;
			return true;
		};
		if (sinfo.fPCStatus==eStatNew) {
			sinfo.direction=eSyncPCToPDA;
			return true;
		}
		if (sinfo.fPalmStatus==eStatNew) {
			sinfo.direction=eSyncPCToPDA;
			return true;
		}
		return true;
	}
	
	// Text was included in the last sync, so if one side doesn't exist, it was deleted and needs to be deleted from the other side, too
	if (!QFile::exists(sinfo.docfilename)) sinfo.fPCStatus=eStatDeleted;
	else if(textChanged(sinfo.docfilename)) {
		sinfo.fPCStatus=eStatChanged;
#ifdef DEBUG
		DEBUGCONDUIT<<"PC side has changed!"<<endl;
#endif
		// TODO: Check for changed bookmarks on the PC side
	} else {
#ifdef DEBUG
		DEBUGCONDUIT<<"PC side has NOT changed!"<<endl;
#endif
	}
	if (!docdb || !docdb->isDBOpen()) sinfo.fPalmStatus=eStatDeleted;
	else {
		PilotRecord *firstRec = docdb->readRecordByIndex(0);
		PilotDOCHead docHeader(firstRec);
		KPILOT_DELETE(firstRec);

		int storyRecs = docHeader.numRecords;

		// determine the index of the next modified record (does it lie beyond the actual text records?)
		int modRecInd=-1;
		PilotRecord*modRec=docdb->readNextModifiedRec(&modRecInd);
#ifdef DEBUG
		DEBUGCONDUIT<<"Index of first changed record: "<<modRecInd<<endl;
#endif
		
		KPILOT_DELETE(modRec);
		// if the header record was changed, find out which is the first changed real document record:
		if (modRecInd==0) {
			modRec=docdb->readNextModifiedRec(&modRecInd);
#ifdef DEBUG
			DEBUGCONDUIT<<"Reread Index of first changed records: "<<modRecInd<<endl;
#endif
			KPILOT_DELETE(modRec);
		}
	
		// The record index starts with 0, so only a negative number means no modified record was found
		if (modRecInd >= 0) {
//			sinfo.fPalmStatus=eStatBookmarksChanged;
#ifdef DEBUG
			DEBUGCONDUIT<<"Handheld side has changed!"<<endl;
#endif
			if ((!fIgnoreBmkChangesOnly) || (modRecInd <= storyRecs)) 
				sinfo.fPalmStatus=eStatChanged;
#ifdef DEBUG
			DEBUGCONDUIT<<"PalmStatus="<<sinfo.fPalmStatus<<", condition="<<((!fIgnoreBmkChangesOnly) || (modRecInd <= storyRecs))<<endl;
#endif
		} else {
#ifdef DEBUG
			DEBUGCONDUIT<<"Handheld side has NOT changed!"<<endl;
#endif
		}
	}
	KPILOT_DELETE(docdb);

	if (sinfo.fPCStatus == eStatNone && sinfo.fPalmStatus==eStatNone) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Nothing has changed, not need for a sync."<<endl;
#endif
		return false;
	}
	// if either is deleted, and the other is not changed, delete
	if ( ((sinfo.fPCStatus == eStatDeleted) && (sinfo.fPalmStatus!=eStatChanged)) ||
	     ((sinfo.fPalmStatus == eStatDeleted) && (sinfo.fPCStatus!=eStatChanged)) ) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Database was deleted on one side and not changed on the other -> Delete it."<<endl;
#endif
		sinfo.direction=eSyncDelete;
		return true;
	}
	
	// eStatDeleted (and both not changed) have already been treated, for all 
	// other values in combination with eStatNone, just copy the texts.
	if (sinfo.fPCStatus==eStatNone) {
#ifdef DEBUG
		DEBUGCONDUIT<<"PC side has changed!"<<endl;
#endif
		sinfo.direction=eSyncPDAToPC;
		return true;
	}

	if (sinfo.fPalmStatus==eStatNone) {
		sinfo.direction=eSyncPCToPDA;
		return true;
	}
	
	// All other cases (deleted,changed), (changed, deleted), (changed,changed) create a conflict:
	sinfo.direction=eSyncConflict;
	return true;
}



PilotDatabase *DOCConduit::preSyncAction(docSyncInfo &sinfo) const
{
	FUNCTIONSETUP;

	{
		// make sure the dir for the local texts really exists!
		QDir dir(fDOCDir);
		if (!dir.exists())
		{
			dir.mkdir(dir.absPath());
		}
	}

	DBInfo dbinfo=sinfo.dbinfo;
	switch (sinfo.direction)
	{
		case eSyncPDAToPC:
			if (fKeepPDBLocally)
			{
				// make sure the dir for the local db really exists!
				QDir dir(fPDBDir);

				if (!dir.exists())
				{
					dir.mkdir(dir.absPath());
				}
#ifdef DEBUG
				DEBUGCONDUIT<<"Need to fetch database "<<dbinfo.name<<" to the directory "<<dir.absPath()<<endl;
#endif
				dbinfo.flags &= ~dlpDBFlagOpen;

				if (!fHandle->retrieveDatabase(sinfo.pdbfilename, &dbinfo) )
				{
					kdWarning(0)<<"Unable to retrieve database "<<dbinfo.name<<" from the handheld into "<<sinfo.pdbfilename<<"."<<endl;
					return 0L;
				}
			}
			break;
		case eSyncPCToPDA:
			if (fKeepPDBLocally)
			{
				// make sure the dir for the local db really exists!
				QDir dir(fPDBDir);
				if (!dir.exists())
				{
					dir.mkdir(dir.absPath());
				}
			}
			break;
		default:
			break;
	}
	if (fKeepPDBLocally)
	{
		return new PilotLocalDatabase(fPDBDir, 
			QString::fromLatin1(dbinfo.name), false);
	}
	else
	{
		return new PilotSerialDatabase(pilotSocket(), 
			QString::fromLatin1(dbinfo.name));
	}
}


// res gives us information whether the sync worked and the db might need to be
// transferred to the handheld or not (and we just need to clean up the mess)
bool DOCConduit::postSyncAction(PilotDatabase * database, docSyncInfo &sinfo, bool res)
{
	FUNCTIONSETUP;
	bool rs = true;

	switch (sinfo.direction)
	{
		case eSyncPDAToPC:
			// also reset the sync flags on the handheld
#ifdef DEBUG
				DEBUGCONDUIT<<"Resetting sync flags for database "<<sinfo.dbinfo.name<<endl;
#endif
			if (fKeepPDBLocally && !fLocalSync) {
				PilotSerialDatabase*db=new PilotSerialDatabase(pilotSocket(), QString::fromLatin1(sinfo.dbinfo.name));
#ifdef DEBUG
				DEBUGCONDUIT<<"Middle 1 Resetting sync flags for database "<<sinfo.dbinfo.name<<endl;
#endif
				if (db) {
					db->resetSyncFlags();
					KPILOT_DELETE(db);
				}
#ifdef DEBUG
				DEBUGCONDUIT<<"Middle2 Resetting sync flags for database "<<sinfo.dbinfo.name<<endl;
#endif
			}
#ifdef DEBUG
				DEBUGCONDUIT<<"End Resetting sync flags for database "<<sinfo.dbinfo.name<<endl;
#endif
			break;
		case eSyncPCToPDA:
			if (fKeepPDBLocally && !fLocalSync && res)
			{
				// Copy the database to the palm
				PilotLocalDatabase*localdb=dynamic_cast<PilotLocalDatabase*>(database);
				if (localdb)
				{
#ifdef DEBUG
					DEBUGCONDUIT<<"Installing file "<<localdb->dbPathName()<<" ("<<sinfo.handheldDB<<") to the handheld"<<endl;
#endif
					QString dbpathname=localdb->dbPathName();
					KPILOT_DELETE(database); // deletes localdb as well, which is just casted from database!!!
/*					if (!fHandle->installFiles(dbpathname )) 
					{
						rs = false;
#ifdef DEBUG
						DEBUGCONDUIT<<"Could not install the database "<<dbpathname<<" ("<<sinfo.handheldDB<<")"<<endl;
#endif
					}*/
				}
			}
		default:
			break;
	}
	
#ifdef DEBUG
	DEBUGCONDUIT<<"Vor KPILOT_DELETE(database)"<<endl;
#endif

	KPILOT_DELETE(database);
#ifdef DEBUG
	DEBUGCONDUIT<<"End postSyncAction"<<endl;
#endif
	return rs;
}



void DOCConduit::cleanup()
{
	FUNCTIONSETUP;
	
	KConfigGroupSaver g(fConfig, DOCConduitFactory::fGroup);
	fConfig->writeEntry(DOCConduitFactory::fDOCList, fDBNames);
	fConfig->sync();

	emit syncDone(this);
}

