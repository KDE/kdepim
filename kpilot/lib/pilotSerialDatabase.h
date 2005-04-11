#ifndef _KPILOT_PILOTSERIALDATABASE_H
#define _KPILOT_PILOTSERIALDATABASE_H
/* pilotSerialDatabase.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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

// Database class for a database on the pilot connected
// via the serial port (ie: hot-sync cradle)

#include "pilotDatabase.h"
#include "pilotRecord.h"


class  PilotSerialDatabase : public PilotDatabase
{
public:
	PilotSerialDatabase(int linksocket, const QString &dbName);
	virtual ~PilotSerialDatabase();

	/** Reads the application block info, returns size */
	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	/** Writes the application block info. */
	virtual int writeAppBlock(unsigned char* buffer, int len);
	/**  returns the number of records in the database */
	virtual int recordCount();
	/** Returns a QValueList of all record ids in the database. */
	 virtual QValueList<recordid_t> idList();
	/** Reads a record from database by id, returns record length */
	virtual PilotRecord* readRecordById(recordid_t id);
	/** Reads a record from database, returns the record length */
	virtual PilotRecord* readRecordByIndex(int index);
	/** Reads the next record from database in category 'category' */
	virtual PilotRecord* readNextRecInCategory(int category);
	/**
	* Reads the next record from database that has the dirty flag set.
	* ind (if a valid pointer is given) will receive the index of the
	* returned record.
	*/
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL);

	/**
	* Writes a new record to database (if 'id' == 0, one will be
	* assigned to newRecord)
	*/
	virtual recordid_t writeRecord(PilotRecord* newRecordb);

	/**
	* Deletes a record with the given recordid_t from the database,
	* or all records, if all is set to true. The recordid_t will be
	* ignored in this case. Return value is negative on error, 0 otherwise.
	*/
	virtual int deleteRecord(recordid_t id, bool all=false);
	/** Resets all records in the database to not dirty. */
	virtual int resetSyncFlags();
	/** Resets next record index to beginning */
	virtual int resetDBIndex();
	/** Purges all Archived/Deleted records from Palm Pilot database */
	virtual int cleanup();

	virtual QString dbPathName() const;

	/**
	* Deletes the database (by name, as given in the constructor and
	* stored in the fDBName field).
	*/
	virtual int deleteDatabase();

	/**
	* Creates the database with the given creator, type and flags on
	* the given card (default is RAM). If the database already exists,
	* this function does nothing.
	*/
	virtual bool createDatabase(long creator=0,
		long type=0, int cardno=0, int flags=0, int version=0);
	QString getDBName() { return fDBName; }


	virtual DBType dbType() const;

protected:
	virtual void openDatabase();
	virtual void closeDatabase();
	int getDBHandle() { return fDBHandle; }


private:
	void setDBHandle(int handle) { fDBHandle = handle; }

	QString     fDBName;
	int         fDBHandle;
	int         fDBSocket;
#if PILOT_LINK_NUMBER >= PILOT_LINK_0_12_0
	// Pilot-link 0.12 allocates buffers as needed and resizes them.
	// Start with a buffer that is _probably_ big enough for most 
	// PIM records, but much smaller than the 64k that we use otherwise.
	// Might want to add algorithm for trying to optimize the initial 
	// allocation for a given database.
	static const int InitialBufferSize = 2048;
#endif
};

#endif
