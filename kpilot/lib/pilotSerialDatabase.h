#ifndef _KPILOT_PILOTSERIALDATABASE_H
#define _KPILOT_PILOTSERIALDATABASE_H
/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "pilotDatabase.h"
#include "pilotRecord.h"

/** @file
* Database class for a database on the pilot connected
* via the serial port (ie: hot-sync cradle)
*/

class KPilotDeviceLink;

/**
* PilotSerialDatabase represents databases stored on the handheld
* and accessed through the SLP / DLP protocol.
*/
class KDE_EXPORT PilotSerialDatabase : public PilotDatabase
{
friend class KPilotDeviceLink;
protected:
	PilotSerialDatabase( KPilotDeviceLink *l, const QString &dbName );
	PilotSerialDatabase( KPilotDeviceLink *l, const DBInfo *info );

public:
	virtual ~PilotSerialDatabase();

	/** Reads the application block info, returns size */
	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	/** Writes the application block info. */
	virtual int writeAppBlock(unsigned char* buffer, int len);
	/**  returns the number of records in the database, 0 if not open */
	virtual unsigned int recordCount() const;
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
	/** Returns the file handle used to communicate with the handheld.
	*   This is an internal value to be passed to DLP functions.
	*/
	int getDBHandle() const
	{
		return fDBHandle;
	}


private:
	void setDBHandle(int handle) { fDBHandle = handle; }

	QString     fDBName;
	int         fDBHandle;
	int         fDBSocket;
	// Pilot-link 0.12 allocates buffers as needed and resizes them.
	// Start with a buffer that is _probably_ big enough for most
	// PIM records, but much smaller than the 64k that we use otherwise.
	// Might want to add algorithm for trying to optimize the initial
	// allocation for a given database.
	static const int InitialBufferSize = 2048;
};

#endif
