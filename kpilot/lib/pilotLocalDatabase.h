#ifndef _KPILOT_PILOTLOCALDATABASE_H
#define _KPILOT_PILOTLOCALDATABASE_H
/* pilotLocalDatabase.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"    /* for PI_SIZE_T */

#include "pilotDatabase.h"

class PilotLocalDatabase : public PilotDatabase
{
public:
	/**
	* Opens the local database. If the database cannot be found at the
	* given position, a default path is used
	*     ($KDEHOME/share/apps/kpilot/DBBackup)
	* and if the file is found there, it is opened. In some cases this should
	* not be done, so the parameter useDefaultPath controls this behavior.
	* If it is set to true, the default path is used if the file cannot be
	* found in the explicitly given location. If it is set to false and
	* the database cannot be found, no database is opened. It can then be
	* created explicitly at the specified location.
	*/
	PilotLocalDatabase( const QString& path,
		const QString& name, bool useDefaultPath=true);
	/**
	* Opens the local database. A default path is used
	* ($KDEHOME/share/apps/kpilot/DBBackup)
	* and if the file is found there, it is opened.
	* Since a backup messes up the state of the conduits (i.e.
	* changes on the handheld might no longer be detected after
	* a backup run, since the conduit assumes the database to have
	* the state of the previous conduit run,  useConduitDBs=true
	* opens the database in $KDEHOME/share/apps/kpilot/conduits
	*/
	PilotLocalDatabase(const QString &name, bool useConduitDBs=false);


	virtual ~PilotLocalDatabase();

	/** Creates the database with the given creator, type and flags on
	* the given card (default is RAM). If the database already exists,
	* this function does nothing.
	*/
	virtual bool createDatabase(long creator=0,
		long type=0, int cardno=0, int flags=0, int version=0);



	/** Deletes the database (by name, as given in the constructor
	* and stored in the fDBName field. )
	*/
	virtual int deleteDatabase();

	// Reads the application block info
	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	// Writes the application block info.
	virtual int writeAppBlock(unsigned char* buffer, int len);
	// returns the number of records in the database
	virtual int recordCount();
	// Returns a QValueList of all record ids in the database.
	virtual QValueList<recordid_t> idList();
	// Reads a record from database by id, returns record length
	virtual PilotRecord* readRecordById(recordid_t id);
	// Reads a record from database, returns the record length
	virtual PilotRecord* readRecordByIndex(int index);
	// Reads the next record from database in category 'category'
	virtual PilotRecord* readNextRecInCategory(int category);
	/**
	* Reads the next record from database that has the dirty flag set.
	* ind (if a valid pointer is given) will receive the index of the
	* returned record.
	*/
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL);
	// Writes a new record to database (if 'id' == 0, one will be
	// assigned to newRecord)
	virtual recordid_t writeRecord(PilotRecord* newRecord);
	/**
	* Deletes a record with the given recordid_t from the database,
	* or all records, if all is set to true. The recordid_t will be
	* ignored in this case. Return value is negative on error, 0 otherwise.
	*/
	virtual int deleteRecord(recordid_t id, bool all=false);
	// Resets all records in the database to not dirty.
	virtual int resetSyncFlags();
	// Resets next record index to beginning
	virtual int resetDBIndex();
	// Purges all Archived/Deleted records from Palm Pilot database
	virtual int cleanup();


	// Writes a new ID to the record specified.
	virtual recordid_t writeID(PilotRecord* rec);
	QString getDBName() const { return fDBName; }

	/**
	* Returns the full path of the current database, based on
	* the path and dbname passed to the constructor, and including
	* the .pdb extension.
	*/
	virtual QString dbPathName() const;

	/**
	* Accessor functions for the application info block.
	*/
	int appInfoSize() const
		{ if (isDBOpen()) return fAppLen; else return -1; } ;
	char *appInfo() { return fAppInfo; } ;

	struct DBInfo getDBInfo() const { return fDBInfo; }
	void setDBInfo(struct DBInfo dbi) {fDBInfo=dbi; }

	virtual DBType dbType() const;


protected:
	// Changes any forward slashes to underscores
	void fixupDBName();
	virtual void openDatabase();
	virtual void closeDatabase();

private:
	struct DBInfo fDBInfo;
	QString fPathName,fDBName;
	char*       fAppInfo;
	PI_SIZE_T   fAppLen;
	int         fNumRecords;
	int         fCurrentRecord;
	PilotRecord* fRecords[10000]; // Current max records in DB.. hope it's enough
	int         fPendingRec; // Temp index for the record about to get an ID.

#ifdef SHADOW_LOCAL_DB
	QValueList<PilotRecord *> fRecordList;
	QValueList<PilotRecord *>::Iterator fRecordIndex;
#endif

	/**
	* For databases opened by name only (constructor 2 -- which is the
	* preferred one, too) try this path first before the default path.
	* Set statically so it's shared for all local databases.
	*/
public:
	static void setDBPath(const QString &);
	static const QString *getDBPath() { return fPathBase; } ;
private:
	static QString *fPathBase;
};

#endif
