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
	PilotLocalDatabase( const QString& path, const QString& name,
		QObject *p=0L,const char *n=0L);
	PilotLocalDatabase(const QString &name,
		QObject *p=0L,const char *n=0L);


	virtual ~PilotLocalDatabase();

	// Reads the application block info
	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	// Writes the application block info.
	virtual int writeAppBlock(unsigned char* buffer, int len);  
	// Reads a record from database by id, returns record length
	virtual PilotRecord* readRecordById(recordid_t id);
	// Reads a record from database, returns the record length
	virtual PilotRecord* readRecordByIndex(int index);
	// Reads the next record from database in category 'category'
	virtual PilotRecord* readNextRecInCategory(int category);
	// Reads the next record from database that has the dirty flag set.
	virtual PilotRecord* readNextModifiedRec();
	// Writes a new record to database (if 'id' == 0, one will be assigned to newRecord)
	virtual recordid_t writeRecord(PilotRecord* newRecord);
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
	QString dbPathName() const;

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



// $Log$
// Revision 1.3  2002/05/19 15:01:49  adridg
// Patches for the KNotes conduit
//
// Revision 1.2  2002/01/21 23:14:03  adridg
// Old code removed; extra abstractions added; utility extended
//
// Revision 1.1  2001/10/10 22:01:24  adridg
// Moved from ../kpilot/, shared files
//
// Revision 1.11  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.10  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.9  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.8  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.7  2001/02/27 15:39:21  adridg
// Added dbPathName to make .pdb name construction consistent
//
// Revision 1.6  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.5  2001/02/07 14:21:51  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
