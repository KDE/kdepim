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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

// Database class for a local (file based) pilot datbase.

#include <time.h>	/* for broken pilot-link libraries */

#ifndef _PILOT_MACROS_H_
#include <pi-macros.h>	/* for recordid_t */
#endif

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif

class PilotLocalDatabase : public PilotDatabase
{
public:
	/** Opens the local database */
	PilotLocalDatabase( const QString& path, const QString& name, bool useDefaultPath=true,
		QObject *p=0L,const char *n=0L);
	PilotLocalDatabase(const QString &name,
		QObject *p=0L,const char *n=0L);


	virtual ~PilotLocalDatabase();

	/** Creates the database with the given creator, type and flags on the given card (default is RAM). If the database already exists, this function does nothing. */
	virtual bool createDatabase(long creator=0, long type=0, int cardno=0, int flags=0, int version=0);
	/** Deletes the database (by name, as given in the constructor and stored in the fDBName field. ) */
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
	// Reads the next record from database that has the dirty flag set.
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL);
	// Writes a new record to database (if 'id' == 0, one will be assigned to newRecord)
	virtual recordid_t writeRecord(PilotRecord* newRecord);
	/** Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case */
	virtual int deleteRecord(recordid_t id, bool all=false);
	// Resets all records in the database to not dirty.
	virtual int resetSyncFlags();
	// Resets next record index to beginning
	virtual int resetDBIndex();
	// Purges all Archived/Deleted records from Palm Pilot database
	virtual int cleanup();


	// Writes a new ID to the record specified the index.  Not supported on Serial connections
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

protected:
	// Changes any forward slahses to underscores
	void fixupDBName();
	virtual void openDatabase();
	virtual void closeDatabase();

private:
	struct DBInfo fDBInfo;
	QString fPathName,fDBName;
	char*       fAppInfo;
	int         fAppLen;
	int         fNumRecords;
	int         fCurrentRecord;
	PilotRecord* fRecords[10000]; // Current max records in DB.. hope it's enough
	int         fPendingRec; // Temp index for the record about to get an ID.


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
