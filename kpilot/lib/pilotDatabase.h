#ifndef _KPILOT_PILOTDATABASE_H
#define _KPILOT_PILOTDATABASE_H
/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2005-2006 Adriaan de Groot <groot@kde.org>
**
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


#include "pilot.h"


/** @file
* This is the abstract base class for databases, which is used both
* by local databases and by the serial databases held in the Pilot.
*/


/**
 * Methods to access a database on the pilot.
 *
 * NOTE:  It is the users responsibility
 * to delete PilotRecords returned by
 * PilotDatabase methods when finished with them!
 */

class KDE_EXPORT PilotDatabase
{
public:
	PilotDatabase(const QString &name = QString::null);
	virtual ~PilotDatabase();


	QString name() const { return fName; } ;

	/**
	* Debugging information: tally how many databases are created
	* or destroyed. Returns the count of currently existing databases.
	*/
	static int instanceCount();

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

	/** Returns the number of records in the database.
	 *  If the database is not open, return -1.
	 */
	virtual unsigned int recordCount() const=0;

	/** Returns a QValueList of all record ids in the database.
	    This implementation is really bad. */
	virtual Pilot::RecordIDList idList();

	/** Returns a list of all record ids that have been modified in the
	    database. This implementation is really bad. */
	virtual Pilot::RecordIDList modifiedIDList();


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

	bool isOpen() const { return fDBOpen; }

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

/** A template class for reading and interpreting a database. This removes
* the need for a lot of boilerplate code that does the conversions.
* Parameters are two interpretation classes: one for the KDE side of
* things (e.g. Event) and one that interprets the Pilot's records into
* a more sensible structure (e.g. PilotDatebookEntry). The mapping from
* the KDE type to the Pilot type and vice-versa is done by the mapper
* class's convert() functions.
*
* To interpret a database as pilot-link interpretations (e.g. as
* PilotDatebookEntry records, not as Events) use the NullMapper class
* below in combination with a template instantiation with kdetype==pilottype.
*
* The database interpreter intentionally has an interface similar to
* that of a PilotDatabase, but it isn't one.
*/
template <class kdetype, class pilottype, class mapper>
class DatabaseInterpreter
{
private:
	/** Interpret a PilotRecord as an object of type kdetype. */
	kdetype *interpret(PilotRecord *r)
	{
		// NULL records return NULL kde objects.
		if (!r) return 0;
		// Interpret the binary blob as a pilot-link object.
		pilottype *a = new pilottype(r);
		// The record is now obsolete.
		delete r;
		// Interpretation failed.
		if (!a) { return 0; }
		// Now convert to KDE type.
		kdetype *t = mapper::convert(a);
		// The NULL mapper just returns the pointer a, so we
		// need to check if anything has changed before deleting.
		if ( (void *)t != (void *)a )
		{
			delete a;
		}
		return t;
	}
public:
	/** Constructor. Interpret the database @p d. */
	DatabaseInterpreter(PilotDatabase *d) : fDB(d) { } ;

	/** Reads a record from database by @p id */
	kdetype *readRecordById(recordid_t id)
	{
		return interpret(fDB->readRecordById(id));
	}

	/** Reads a record from database with index @p index */
	kdetype *readRecordByIndex(int index)
	{
		return interpret(fDB->readRecordByIndex(index));
	}

	/** Reads the next record from database in category @p category */
	kdetype *readNextRecInCategory(int category)
	{
		return interpret(fDB->readNextRecInCategory(category));
	}

	/**
	* Reads the next record from database that has the dirty flag set.
	* If @p ind is non-NULL, *ind is set to the index of the current
	* record (i.e. before the record pointer moves to the next
	* modified record).
	*/
	kdetype *readNextModifiedRec(int *ind=NULL)
	{
		return interpret(fDB->readNextModifiedRec(ind));
	}


	/** Retrieve the database pointer; this is useful to just pass
	* around DatabaseInterpreter objects as if they are databases,
	* and then perform DB operations on the database it wraps.
	*/
	PilotDatabase *db() const { return fDB; }

protected:
	PilotDatabase *fDB;
} ;

/** NULL mapper class; the conversions here don't @em do anything,
* so you can use this when you only need 1 conversion step (from
* PilotRecord to PilotDatebookEntry, for instance) instead of 2.
*/
template <class T>
class NullMapper
{
public:
	/** NULL Conversion function. */
	static T *convert(T *t) { return t; }
} ;

#endif
