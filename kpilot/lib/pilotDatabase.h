#ifndef _KPILOT_PILOTDATABASE_H
#define _KPILOT_PILOTDATABASE_H
/* pilotDatabase.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qstring.h>
#include <qvaluelist.h>

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

class KDE_EXPORT PilotDatabase
{
public:
	PilotDatabase(const QString &name = QString::null);
	virtual ~PilotDatabase();

	static const int MAX_APPINFO_SIZE=8192;


	QString name() const { return fName; } ;

	/**
	* Debugging information: tally how many databases are created
	* or destroyed. Returns the count of currently existing databases.
	*/
	static int count();

	/* -------------------- Abstract interface for subclasses ----------------- */

	/**
	* Creates the database with the given creator, type and flags
	* on the given card (default is RAM). If the database already
	* exists, this function does nothing.
	*/
	virtual bool createDatabase(long creator=0, long type=0,
		int cardno=0, int flags=0, int version=0) = 0;

	/**
	* Deletes the database (by name, as given in the constructor,
	* the database name is stored depending on the implementation
	* of PilotLocalDatabase and PilotSerialDatabas)
	*/
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

	/**
	* Reads the next record from database that has the dirty flag set.
	* If @p ind is non-NULL, *ind is set to the index of the current
	* record (i.e. before the record pointer moves to the next
	* modified record).
	*/
	virtual PilotRecord* readNextModifiedRec(int *ind=NULL) = 0;

	/**
	* Writes a new record to database (if 'id' == 0, one will be
	* assigned to newRecord)
	*/
	virtual recordid_t writeRecord(PilotRecord* newRecord) = 0;

	/**
	* Deletes a record with the given recordid_t from the database,
	* or all records, if @p all is set to true. The recordid_t will
	* be ignored in this case.
	*
	* Return value is negative on error, 0 otherwise.
	*/
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
	* Use this instead of RTTI to determine the type of a
	* PilotDatabase, for those cases where it's important.
	*/
	typedef enum { eNone=0,
		eLocalDB=1,
		eSerialDB=2 } DBType;
	virtual DBType dbType() const = 0;

	/**
	* Here are some static utility functions. listAppInfo() is primarily
	* meant for debugging, and it dumps an appinfo block to stdout.
	*/
	static void listAppInfo(const struct CategoryAppInfo *);

	static inline bool isResource(struct DBInfo *info)
	{
		return (info->flags & dlpDBFlagResource);
	}

protected:
	virtual void openDatabase() = 0;
	virtual void closeDatabase() = 0;

	void setDBOpen(bool yesno) { fDBOpen = yesno; }

private:
	bool fDBOpen;
	QString fName;
};

#endif
