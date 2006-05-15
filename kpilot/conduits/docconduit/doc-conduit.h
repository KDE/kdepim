#ifndef _doc_CONDUIT_H
#define _doc_CONDUIT_H
/* doc-conduit.h                           KPilot
**
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
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


#include <plugin.h>

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
	virtual bool isCorrectDBTypeCreator(DBInfo dbinfo);
	virtual const unsigned long dbtype();
	virtual const unsigned long dbcreator();

public slots:
/** syncNextDB walks through all PalmDoc databases on the handheld and decides if they are supposed to be synced to the PC.
 * syncNextDB and syncNextDOC fist build the list of all PalmDoc texts, and then the method syncDatabases does the actual sync. */
	void syncNextDB();
	void syncNextTXT();
	void checkPDBFiles();
	void checkDeletedDocs();
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

	bool pcTextChanged(QString txtfn);
	bool hhTextChanged(PilotDatabase*docdb);
	
	/** Opens the database with name dbname. For a local sync, this will be a 
	 *  PilotLocalDatabase, otherwise it will be a database on the serial device 
	 *  (i.e. an object of class PilotSerialDatabase) */
	PilotDatabase*openDOCDatabase(QString dbname);

	QString constructPDBFileName(QString name);
	QString constructTXTFileName(QString name);

	eSyncDirectionEnum  eConflictResolution;
	int fTXTBookmarks, fPDBBookmarks;
	QStringList fDBListSynced;
	QStringList fDBNames;
	syncInfoList fSyncInfoList;
	syncInfoList::Iterator fSyncInfoListIterator;
	long int dbnr;

	QStringList docnames;
	QStringList::Iterator dociterator;
};

class docSyncInfo
{
public:
	docSyncInfo(QString hhDB=QString(), QString txtfn=QString(), QString pdbfn=QString(), eSyncDirectionEnum dir=eSyncNone)
	{
		handheldDB=hhDB;
		txtfilename=txtfn;
		pdbfilename=pdbfn;
		direction=dir;
		fPCStatus=eStatNone;
		fPalmStatus=eStatNone;
	};
	~docSyncInfo(){};
	QString handheldDB, txtfilename, pdbfilename;
	DBInfo dbinfo;
	eSyncDirectionEnum direction;
	eTextStatus fPCStatus, fPalmStatus;
};


#endif
