#ifndef _KPILOT_PILOTDATABASE_H
#define _KPILOT_PILOTDATABASE_H
/* pilotDatabase.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This is the abstract base class for databases, which is used both
** by local databases and by the serial databases held in the Pilot.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qobject.h>

// Handle all time.h variations properly.
// Required because pi-macros.h sometimes forgets it.
//
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <pi-macros.h>

#include "pilotRecord.h"



/**
 * Methods to access a database on the pilot.  
 *
 * NOTE:  It is the users responsibility
 * to delete PilotRecords returned by 
 * PilotDatabase methods when finished with them!!
 */

class PilotDatabase : public QObject
{
Q_OBJECT
public:
	PilotDatabase(QObject *,const char *);
	virtual ~PilotDatabase();

	enum { MAX_APPINFO_SIZE=8192 
		} Constants;

	/** Creates the database with the given creator, type and flags on the given card (default is RAM). If the database already exists, this function does nothing. */
	virtual bool createDatabase(long creator=0, long type=0, int cardno=0, int flags=0, int version=0) = 0;
	
	/** Deletes the database (by name, as given in the constructor, the database name is stored depending on the implementation of PilotLocalDatabase and PilotSerialDatabas) */
	virtual int deleteDatabase()=0;
	
	/** Reads the application block info, returns size. */
	virtual int readAppBlock(unsigned char* buffer, int maxLen) = 0;

	/** Writes the application block info. */
	virtual int writeAppBlock(unsigned char* buffer, int len) = 0;

	/** returns the number of records in the database */
	virtual int recordCount()=0;
	
	/** Returns a QValueList of all record ids in the database.  */
	 virtual QValueList<recordid_t> idList()=0;

	/** Reads a record from database by id, returns record length */
	virtual PilotRecord* readRecordById(recordid_t id) = 0;

	/** Reads a record from database, returns the record length */
	virtual PilotRecord* readRecordByIndex(int index) = 0;

	/** Reads the next record from database in category 'category' */
	virtual PilotRecord* readNextRecInCategory(int category) = 0;

	/** Reads the next record from database that has the dirty flag set. */
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL) = 0;

	/** Writes a new record to database (if 'id' == 0, one will be assigned to newRecord) */
	virtual recordid_t writeRecord(PilotRecord* newRecord) = 0;

	/** Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case */
	virtual int deleteRecord(recordid_t id, bool all=false) = 0;
	
	/** Resets all records in the database to not dirty. */
	virtual int resetSyncFlags() = 0;

	/** Resets next record index to beginning */
	virtual int resetDBIndex() = 0;

	/** Purges all Archived/Deleted records from Palm Pilot database */
	virtual int cleanup() = 0;

	bool isDBOpen() const { return fDBOpen; }

	/** Returns some sensible human-readable identifier for
	*   the database. Serial databases get Pilot:, local
	*   databases return the full path.
	*/
	virtual QString dbPathName() const = 0;

	/**
	* Here are some static utility functions. listAppInfo() is primarily
	* meant for debugging, and it dumps an appinfo block to stdout.
	*/
	static void listAppInfo(const struct CategoryAppInfo *);
	
protected:
	virtual void openDatabase() = 0;
	virtual void closeDatabase() = 0;

	void setDBOpen(bool yesno) { fDBOpen = yesno; }

private:
	bool fDBOpen;
};



// $Log$
// Revision 1.10  2002/12/13 16:26:09  kainhofe
// Added default args to readNextModifiedRec, and findDatabase, new functions: deleteRecord and createDatabase
//
// Revision 1.9  2002/11/27 21:29:07  adridg
// See larger ChangeLog entry
//
// Revision 1.8  2002/08/20 21:18:31  adridg
// License change in lib/ to allow plugins -- which use the interfaces and
// definitions in lib/ -- to use non-GPL'ed libraries, in particular to
// allow the use of libmal which is MPL.
//
// Revision 1.7  2002/06/30 14:49:53  kainhofe
// added a function idList, some minor bug fixes
//
// Revision 1.6  2002/06/07 07:13:25  adridg
// Make VCal conduit use base-class fDatabase and fLocalDatabase (hack).
// Extend *Database classes with dbPathName() for consistency.
//
// Revision 1.5  2002/05/22 20:40:13  adridg
// Renaming for sensibility
//
// Revision 1.4  2002/01/17 16:24:10  adridg
// Compile fixes on Solaris
//
// Revision 1.3  2002/01/08 01:25:48  cschumac
// Compile fixes.
//
// Revision 1.2  2001/10/17 08:46:08  adridg
// Minor cleanups
//
// Revision 1.1  2001/10/10 21:47:14  adridg
// Shared files moved from ../kpilot/ and polished
//
// Revision 1.10  2001/09/29 16:26:18  adridg
// The big layout change
//
// Revision 1.9  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.8  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.7  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.6  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.5  2001/02/07 14:21:49  brianj
// Changed all include definitions for libpisock headers
// to use include path, which is defined in Makefile.
//
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
#endif
