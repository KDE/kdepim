#ifndef _doc_CONDUIT_H
#define _doc_CONDUIT_H
// doc-conduit.cc
//
// Copyright (C) 2002 by Reinhold Kainhofer
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 
//
// $Revision$
//

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/



#include <plugin.h>

#include <kapplication.h>


class PilotLocalDatabase;
class PilotSerialDatabase;
class PilotDatabase;
class docDBInfo;
#include <qdir.h> 

#include "doc-factory.h"

class docSyncInfo;
typedef QValueList<docSyncInfo> syncInfoList;

typedef enum eSyncDirectionEnum {
		eSyncNone,
//		eSyncAll,
		eSyncPDAToPC,
		eSyncPCToPDA,
		eSyncDelete,
		eSyncConflict
	};
typedef enum eTextStatus {
	eStatNone=0,
	eStatNew=1,
	eStatChanged=2,
	eStatBookmarksChanged=4,
	eStatDeleted=8,
	eStatDoesntExist=16
	};


QString dirToString(eSyncDirectionEnum dir);

class DOCConduit:public ConduitAction {
Q_OBJECT
public:
	eSyncDirectionEnum eSyncDirection;

public:
	DOCConduit(KPilotDeviceLink * o,
		const char *n = 0L, const QStringList & a = QStringList());
	 virtual ~ DOCConduit();

	bool encode(QStringList fileName, PilotDatabase * db);
	bool decode(PilotDatabase * db, QString fileName);
	virtual bool exec();
protected:
	virtual bool isCorrectDBTypeCreator(DBInfo dbinfo) {
		return dbinfo.type == dbtype() && dbinfo.creator == dbcreator();
	};
	virtual const unsigned long dbtype() {
		return get_long(DOCConduitFactory::dbDOCtype);
	}
	virtual const unsigned long dbcreator() {
		return get_long(DOCConduitFactory::dbDOCcreator);
	}

public slots:
/** syncNextDB walks through all PalmDoc databases on the handheld and decides if they are supposed to be synced to the PC. 
 * syncNextDB and syncNextDOC fist build the list of all PalmDoc texts, and then the method syncDatabases does the actual sync. */
	void syncNextDB();
	void syncNextDOC();
	void checkPDBFiles();
	void resolve();
	void syncDatabases();
	void cleanup();

 private:
	 /**
    * Read the global KPilot config file for settings
    * particular to the docConduit conduit.
    */
	void readConfig();

	/** 
	* Check if the database needs to be synced at all.
	*/
	bool needsSync(docSyncInfo &sinfo);
	 /**
    * If necessary, copy the database from the palm to a local dir. 
    * Also initialize the docDBInfo that will be passed to the docconverter
    */
	PilotDatabase *preSyncAction(docSyncInfo &sinfo) const;

	bool doSync(docSyncInfo &sinfo);
	 /**
    * Clean up after the sync. The bool parameter res tells 
    * the function if the  conversion was successful or not
    */
	bool postSyncAction(PilotDatabase * dbinfo, docSyncInfo &sinfo, bool res = true);

	bool textChanged(QString docfn);
	/** Opens the databse with name dbname. For a local sync, this will be a 
	 *  PilotLocalDatabase, otherwise it will be a database on the serial device 
	 *  (i.e. an object of class PilotSerialDatabase) */
	PilotDatabase*openDOCDatabase(QString dbname);

	QString constructPDBFileName(QString name);
	QString constructDOCFileName(QString name);




	QString fDOCDir, fPDBDir;
	bool fKeepPDBLocally;
	eSyncDirectionEnum  eConflictResolution;
	int fBookmarks;
	bool fCompress, fIgnoreBmkChangesOnly, fLocalSync, fAlwaysUseResolution;
	QStringList fDBListSynced;
	QStringList fDBNames;
	syncInfoList fSyncInfoList;
	syncInfoList::Iterator fSyncInfoListIterator;
//	QString docfilename, pdbfilename;
	long int dbnr;

	QStringList docnames;
	QStringList::Iterator dociterator;
};

class docSyncInfo 
{
public:
	docSyncInfo(QString hhDB=QString(), QString docfn=QString(), QString pdbfn=QString(), eSyncDirectionEnum dir=eSyncNone)
		{handheldDB=hhDB; docfilename=docfn; pdbfilename=pdbfn; direction=dir; fPCStatus=eStatNone; fPalmStatus=eStatNone;};
	~docSyncInfo(){};
	QString handheldDB, docfilename, pdbfilename;
	DBInfo dbinfo;
	eSyncDirectionEnum direction;
	eTextStatus fPCStatus, fPalmStatus;
};



// $Log$
// Revision 1.2  2002/12/31 00:22:10  kainhofe
// Currently restructuring everything. Not yet finished.
//
// Revision 1.1  2002/12/13 16:29:53  kainhofe
// New PalmDOC conduit to syncronize text files with doc databases (AportisDoc, TealReader, etc) on the handheld
//
//
#endif
