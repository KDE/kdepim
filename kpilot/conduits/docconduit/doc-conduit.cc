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




#include "options.h"

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//
#include <unistd.h>
#include <iostream.h>
//
#include <qtimer.h>
#include <qdir.h>
#include <qfile.h>
//
//#include <kglobal.h>
//#include <kstddirs.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kmdcodec.h> 

#include <pi-macros.h>

#include <pilotLocalDatabase.h>
#include <pilotSerialDatabase.h>

#include "doc-factory.h"

#include "doc-conduit.moc"
#include "doc-setup.h"
#include"DOC-converter.h"
#include "pilotDOCHead.h"
#include "pilotDOCBookmark.h"
#include "pilotDOCEntry.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
const char *doc_conduit_id = "$Id: $";


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
		(enum eConflictResolutionEnum) (fConfig->
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
	//2 Possibilities:
	// -) compare the modification time of the text file
	// -) Use a md5 checksum on the contents of the text
	
//	QFileInfo docinfo(docfn);
//	QDateTime oldMTime=fConfig->readDateEntry(docfn);
//	QDateTime thisMTime=docinfo.modificationTime();
//	return (!oldMTime.isValid()) || (!thisMTime.isValid()) || (oldMTime!=thisMTime);

	// Use KMD5 for now. If I realize it is too slow, then I have to go back to comparing modification times
	// if there is no config setting yet, assume the file has been changed. the md5 sum will be written to the config file after the sync.
	QString oldDigest=fConfig->readEntry(docfn);
	if (oldDigest.length()<=0) 
	{
#ifdef DEBUG
		DEBUGCONDUIT<<"Could not read the old digest value for the file "<<docfn<<endl;
#endif
		return true;
	}
#ifdef DEBUG
	DEBUGCONDUIT<<"Old digest is "<<oldDigest<<endl;
#endif
	
	KMD5 docmd5;
	QFile docfile(docfn);
	if (docfile.open(IO_ReadOnly)){
		docmd5.update(docfile);
		QString thisDigest(docmd5.hexDigest().data());
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
                S Y N C   S T R U C T U R E
 *********************************************************************/





/* virtual */ bool DOCConduit::exec()
{
	FUNCTIONSETUP;

//  KPilotUser*usr;
#ifdef DEBUG
	DEBUGCONDUIT<<"Conduit version: "<<doc_conduit_id<<endl;
#endif

	if (!fConfig)
	{
		kdWarning() << k_funcinfo << ": No config file was set!" << endl;
		return false;
	}
	readConfig();
	dbnr=0;

	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	return true;
}


bool DOCConduit::doSync(DBInfo&dbinfo, eSyncDirectionEnum dir) {
	bool res=false;
	// preSyncAction should initialize the custom databases/files for the
	// specific action chosen for this db and return a pointer to a docDBInfo
	// instance which points either to a local database or a database on the handheld.
	PilotDatabase *database = preSyncAction(dbinfo, dir);

#ifdef DEBUG
	DEBUGCONDUIT<<"Pre-sync done"<<endl;
#endif
	if (database && ( !database->isDBOpen() ) ) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Database "<<dbinfo.name<<" does not yet exist. Trying to creat it:"<<endl;
#endif
		bool createSuccess=database->createDatabase(dbcreator(), dbtype());
#ifdef DEBUG
		if (createSuccess) DEBUGCONDUIT<<"Successfull"<<endl;
		else DEBUGCONDUIT<<"Failed"<<endl;
#endif
	}

	if (database && database->isDBOpen()) {
		DOCConverter docconverter;
		connect(&docconverter, SIGNAL(logError(const QString &)), SIGNAL(logError(const QString &)));
		connect(&docconverter, SIGNAL(logMessage(const QString &)), SIGNAL(logMessage(const QString &)));

		docconverter.setDOCpath(fDOCDir, docfilename);
		docconverter.setPDB(database);
		docconverter.setBookmarkTypes(fBookmarks);
		docconverter.setCompress(fCompress);

		switch (dir)
		{
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
#ifdef DEBUG
			DEBUGCONDUIT<<endl<<endl<<endl<<"Calculating MD5 checksum of file "<<docconverter.docFilename()<<" ("<<docfilename<<")"<<endl;
#endif
			QFile docfile(docconverter.docFilename());
			if (docfile.open(IO_ReadOnly)) {
				docmd5.update(docfile);
				QString thisDigest(docmd5.hexDigest().data());
				fConfig->writeEntry(docconverter.docFilename(), thisDigest);
				fConfig->sync();
#ifdef DEBUG
				DEBUGCONDUIT<<"MD5 Checksum of the text is "<<thisDigest<<endl;
#endif
			} else {
#ifdef DEBUG
				DEBUGCONDUIT<<"couldn't open file "<<docconverter.docFilename()<<" for reading!!!"<<endl;
#endif
			}
			
		}
		
		if (!postSyncAction(database, dir, res)) 
			emit logError(i18n("Unable to install the locally created PalmDOC %1 to the handheld.").arg(dbinfo.name));
		if (!res)
			emit logError(i18n("Conversion of PalmDOC \"%1\" failed.").arg(dbinfo.name));
//		disconnect(&docconverter, SIGNAL(logError(const QString &)), SIGNAL(logError(const QString &)));
//		disconnect(&docconverter, SIGNAL(logMessage(const QString &)), SIGNAL(logMessage(const QString &)));
//		KPILOT_DELETE(database);
	}
	else
	{
		emit logError(i18n("Unable to open or create the database %1").arg(dbinfo.name));
	}
	return res;
}

void DOCConduit::syncNextDB() {
	FUNCTIONSETUP;
	DBInfo dbinfo;

	if (eSyncDirection==eSyncPCToPDA || eSyncDirection==eSyncNone || fHandle->findDatabase(NULL, &dbinfo, dbnr, dbtype(), dbcreator() /*, cardno */ ) < 0)
	{
		// no more databases available, so check for PC->Palm sync
//		dbnr=0;
#ifdef DEBUG
	DEBUGCONDUIT<<"No databases to sync, so sync DOC files"<<endl;
#endif
		
		QTimer::singleShot(0, this, SLOT(syncNextDOC()));
		return;
	}
	dbnr=dbinfo.index+1;
#ifdef DEBUG
	DEBUGCONDUIT<<"Next Palm database to sync: "<<dbinfo.name<<", Index="<<dbinfo.index<<endl;
#endif

	// if creator and/or type don't match, go to next db
	if (!isCorrectDBTypeCreator(dbinfo) || dbnames.contains(dbinfo.name))
	{
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}

#ifdef DEBUG
	DEBUGCONDUIT<<"Vor Test Check"<<endl;

	PilotSerialDatabase*testdb=new PilotSerialDatabase(pilotSocket(), dbinfo.name);
	PilotRecord*rec=testdb->readRecordByIndex(0);
	DEBUGCONDUIT<<"DBInfo: name="<<dbinfo.name<<", creator: "<<dbinfo.creator<<endl;
	DEBUGCONDUIT<<"Database="<<dbinfo.name<<", numRecords="<<testdb->recordCount()<<", path="<<testdb->dbPathName()<<endl;
	KPILOT_DELETE(rec);
	KPILOT_DELETE(testdb);
	DEBUGCONDUIT<<"Nach type Check"<<endl;
#endif

	dbnames.append(dbinfo.name);
	{  // keep db a local variable for just these few code lines
		QDir dr(fDOCDir);
		QFileInfo pth(dr, dbinfo.name);
		if (!QString(dbinfo.name).isEmpty())
			docfilename = pth.absFilePath()+".txt";
	}
	{
		QDir dr(fPDBDir);
		QFileInfo pth(dr, dbinfo.name);
		if (!QString(dbinfo.name).isEmpty())
			pdbfilename = pth.absFilePath()+".pdb";
	}


	eSyncDirectionEnum dir = eSyncNone;

	if (!needsSync(dbinfo, dir))
	{
		QTimer::singleShot(0, this, SLOT(syncNextDB()));
		return;
	}

	if (!doSync(dbinfo, dir)) {
		// The sync could not be done, so inform the user (should maybe done inside doSync anyway, so this is empty for now)
	}

	QTimer::singleShot(0, this, SLOT(syncNextDB()));
	return;
}



void DOCConduit::syncNextDOC()
{
	FUNCTIONSETUP;
	
	if (eSyncDirection==eSyncPDAToPC || eSyncDirection==eSyncNone  )
	{
		// no more databases available, so cleanup
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}
	
	// if docnames isn't initialized, get a list of all *.txt files in fDOCDir
	if (docnames.isEmpty()/* || dociterator==docnames.end() */) {
#ifdef DEBUG
		DEBUGCONDUIT<<"docnames is empty, so get a list of files in this dir and set the iterator to the beginning"<<endl;
#endif
		docnames=QDir(fDOCDir, "*.txt").entryList() ;
		// TODO: Maybe implement some error handling?
		dociterator=docnames.begin();
	}
	if (dociterator==docnames.end()) {
		// no more databases available, so cleanup
		QTimer::singleShot(0, this, SLOT(cleanup()));
		return;
	}

	QString fn=(*dociterator);
	QDir dr(fDOCDir);
	QFileInfo fl(dr, fn );
	docfilename=fl.absFilePath();
	dociterator++;
	
	DBInfo dbinfo;
	// Include all "extensions" except the last. This allows full stops inside the database name (e.g. abbreviations)
	strncpy(&dbinfo.name[0], fl.baseName(TRUE), 34);

#ifdef DEBUG
	DEBUGCONDUIT<<docfilename<<": textChanged="<<textChanged(docfilename)<<endl;
//		QTimer::singleShot(0, this, SLOT(syncNextDOC()));
//		return;
#endif
	if (dbnames.contains(dbinfo.name) || !textChanged(docfilename))
	{
		// Already checked, no need to sync.
#ifdef DEBUG
		DEBUGCONDUIT<<docfilename<<" has already been synced, skipping it."<<endl;
#endif
		QTimer::singleShot(0, this, SLOT(syncNextDOC()));
		return;
	}

	eSyncDirectionEnum dir = eSyncPCToPDA;
	if (!doSync(dbinfo, dir)) {
		// The sync could not be done, so inform the user (should maybe done inside doSync anyway, so this is empty for now)
#ifdef DEBUG
		DEBUGCONDUIT<<"There was some error syncing "<<docfilename<<endl;
#endif
	}

	QTimer::singleShot(0, this, SLOT(syncNextDOC()));
	return;
}



bool DOCConduit::needsSync(DBInfo dbinfo, eSyncDirectionEnum & dir)
{
	FUNCTIONSETUP;
	dir = eSyncNone;

	// If the text does not exist on disk -> sync PDA to PC
	if (!QFile::exists(docfilename))
	{
		dir=eSyncPDAToPC;
		return true;
	}

	PilotSerialDatabase*docdb=new PilotSerialDatabase(pilotSocket(), dbinfo.name);

	if (!docdb || !docdb->isDBOpen())
	{
		emit logError(i18n("Could not open the PalmDOC database \"%1\" on the handheld.").	arg(dbinfo.name));
		dir = eSyncPCToPDA;
		KPILOT_DELETE(docdb);
		return true;
	}
#ifdef DEBUG
	DEBUGCONDUIT<<"Opened "<<dbinfo.name<<"..."<<endl;
#endif

	bool docChanged = false, bmksChanged = false, pcChanged = false;
	PilotRecord *firstRec = docdb->readRecordByIndex(0);
	PilotDOCHead docHeader(firstRec);
	KPILOT_DELETE(firstRec);

	int storyRecs = docHeader.numRecords;

	// TODO: How do I determine the index of the next modified record????
	int modRecInd=0;
	PilotRecord*modRec=docdb->readNextModifiedRec(&modRecInd);
#ifdef DEBUG
	DEBUGCONDUIT<<"Index of first changed records: "<<modRecInd<<endl;
#endif
	KPILOT_DELETE(modRec);
	// if the header record was changed, find out which is the first changed real document record:
	if (modRecInd==1) {
		modRec=docdb->readNextModifiedRec(&modRecInd);
#ifdef DEBUG
		DEBUGCONDUIT<<"Reread Index of first changed records: "<<modRecInd<<endl;
#endif
		KPILOT_DELETE(modRec);
	}
	KPILOT_DELETE(docdb);

	if (modRecInd > 0) {
		if (modRecInd <= storyRecs) docChanged = true;
		bmksChanged = true;
	}

	pcChanged = textChanged(docfilename);

	if (pcChanged)
	{
		if (bmksChanged)
		{
			switch (eConflictResolution)
			{
				case ePDAOverride:
					dir = eSyncPDAToPC;
					return true;
					break;
				case ePCOverride:
					dir = eSyncPCToPDA;
					return true;
					break;
				case ePCOverrideOnBookmarkChange:
					if (!docChanged)
						dir = eSyncPCToPDA;
					else
						dir = eSyncPDAToPC;
					return true;
					break;
				case eResNone:
					return false;
					break;
				case eResAsk:
				default:
					// TODO: Ask for resolution.
					break;
			}
		}
		else
		{
			dir = eSyncPCToPDA;
			return true;
		}
	}
	else
	{
		// TODO: Use some better setting to prevent syncing if only bookmarks have changed
		if ((bmksChanged && eConflictResolution==ePCOverrideOnBookmarkChange) || docChanged)
		{
			dir = eSyncPDAToPC;
			return true;
		}
	}
	return false;
}



PilotDatabase *DOCConduit::preSyncAction(DBInfo &dbinfo, eSyncDirectionEnum direction) const
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT<<"direction="<<direction<<endl;
#endif
	{
		// make sure the dir for the local texts really exists!
		QDir dir(fDOCDir);
		if (!dir.exists())
		{
			dir.mkdir(dir.absPath());
		}
	}

	switch (direction)
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
		DEBUGCONDUIT<<"Fill fetch database "<<dbinfo.name<<" to the directory "<<dir.absPath()<<endl;
#endif
				dbinfo.flags &= ~dlpDBFlagOpen;

				if (!fHandle->retrieveDatabase(pdbfilename, &dbinfo) )
				{
					kdWarning(0)<<"Unable to retrieve database "<<dbinfo.name<<" from the handheld into "<<pdbfilename<<"."<<endl;
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
			// TODO: Do I need to make sure a valid database already exists there????
			break;
		default:
			break;
	}
	if (fKeepPDBLocally)
	{
		return new PilotLocalDatabase(fPDBDir, dbinfo.name);
	}
	else
	{
		return new PilotSerialDatabase(pilotSocket(), dbinfo.name);
	}
}


// res gives us information whether the sync worked and the db might need to be
// transferred to the handheld or not (and we just need to clean up the mess)
bool DOCConduit::postSyncAction(PilotDatabase * database, eSyncDirectionEnum direction, bool res)
{
	FUNCTIONSETUP;
	bool rs = true;

	switch (direction)
	{
		case eSyncPDAToPC:
			// No need to do anything.
			break;
		case eSyncPCToPDA:
			if (fKeepPDBLocally && res)
			{
				// Copy the database to the palm
				PilotLocalDatabase*localdb=dynamic_cast<PilotLocalDatabase*>(database);
				if (localdb)
				{
				// TODO: re-uncomment this once the pdb files are correctly created...
//					if (!fHandle->installFiles(localdb->dbPathName() )) rs = false;
				}
			}
			// TODO: Do I need to make sure a valid database already exists there???? Or do these problems only occur with local databases
		default:
			break;
	}

	KPILOT_DELETE(database);
	return rs;
}



void DOCConduit::cleanup()
{
	FUNCTIONSETUP;
	emit syncDone(this);
}





// $Log$
