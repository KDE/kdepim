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
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL);
	/** Writes a new record to database (if 'id' == 0, one will be assigned to newRecord) */
	virtual recordid_t writeRecord(PilotRecord* newRecordb);
	/** Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case */
	virtual int deleteRecord(recordid_t id, bool all=false);
	/** Resets all records in the database to not dirty. */
	virtual int resetSyncFlags();
	/** Resets next record index to beginning */
	virtual int resetDBIndex();
	/** Purges all Archived/Deleted records from Palm Pilot database */
	virtual int cleanup();

	virtual QString dbPathName() const;

	/** Creates the database with the given creator, type and flags on the given card (default is RAM). If the database already exists, this function does nothing. */
	virtual bool createDatabase(long creator=0, long type=0, int cardno=0, int flags=0, int version=0);
	QString getDBName() { return fDBName; }

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



// $Log$
// Revision 1.6  2002/12/08 14:09:24  waba
// Some cleanup
//
// Revision 1.5  2002/08/20 21:18:31  adridg
// License change in lib/ to allow plugins -- which use the interfaces and
// definitions in lib/ -- to use non-GPL'ed libraries, in particular to
// allow the use of libmal which is MPL.
//
// Revision 1.4  2002/06/30 14:49:53  kainhofe
// added a function idList, some minor bug fixes
//
// Revision 1.3  2002/06/07 07:13:25  adridg
// Make VCal conduit use base-class fDatabase and fLocalDatabase (hack).
// Extend *Database classes with dbPathName() for consistency.
//
// Revision 1.2  2002/05/22 20:40:13  adridg
// Renaming for sensibility
//
// Revision 1.1  2001/10/10 22:01:24  adridg
// Moved from ../kpilot/, shared files
//
// Revision 1.9  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.8  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.7  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.6  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.5  2001/02/07 14:21:59  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
