#ifndef _KPILOT_PILOTSERIALDATABASE_H
#define _KPILOT_PILOTSERIALDATABASE_H
/* pilotSerialDatabase.h			KPilot
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

// Database class for a database on the pilot connected
// via the serial port (ie: hot-sync cradle)

#include "pilotDatabase.h"
#include "pilotRecord.h"


class PilotSerialDatabase : public PilotDatabase
{
public:
	PilotSerialDatabase(int linksocket, const QString &dbName,
		QObject * = 0L, const char * = 0L);
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
	/** Reads the next record from database that has the dirty flag set. */
	virtual PilotRecord* readNextModifiedRec();
	/** Writes a new record to database (if 'id' == 0, one will be assigned to newRecord) */
	virtual recordid_t writeRecord(PilotRecord* newRecordb);
	/** Resets all records in the database to not dirty. */
	virtual int resetSyncFlags();
	/** Resets next record index to beginning */
	virtual int resetDBIndex();
	/** Purges all Archived/Deleted records from Palm Pilot database */
	virtual int cleanup();

	virtual QString dbPathName() const;

	const QString getDBName() { return fDBName; }

protected:
	virtual void openDatabase();
	virtual void closeDatabase();
	int getDBHandle() { return fDBHandle; }


private:
	void setDBHandle(int handle) { fDBHandle = handle; }

	QString     fDBName;
	int         fDBHandle;
	int         fDBSocket;
};

#endif
