#ifndef _KPILOT_PILOTLOCALDATABASE_H
#define _KPILOT_PILOTLOCALDATABASE_H
/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
** Copyright (C) 2006 Adriaan de Groot <groot@kde.org>
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

#include "pilotDatabase.h"

/** @file
* Defines the PilotLocalDatabase class, for databases stored
* on disk (as opposed to in a handheld).
*/

/**
* PilotLocalDatabase represents databases in the same binary format
* as on the handheld but which are stored on local disk.
*/
class KDE_EXPORT PilotLocalDatabase : public PilotDatabase
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
	* Opens the local database. This is primarily for testing
	* purposes; only tries the given path.
	*/
	PilotLocalDatabase(const QString &name);

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
	// returns the number of records in the database, 0 if not open
	virtual unsigned int recordCount() const;
	// Returns a QValueList of all record ids in the database.
	virtual QValueList<recordid_t> idList();
	// Reads a record from database by id, returns record
	virtual PilotRecord* readRecordById(recordid_t id);
	// Reads a record from database, returns the record
	virtual PilotRecord* readRecordByIndex(int index);
	// Reads the next record from database in category 'category'
	virtual PilotRecord* readNextRecInCategory(int category);
	/**
	* Returns the next "new" record, ie. the next record
	* that has not been synced yet. These records all have ID=0, so are
	* not easy to find with the other methods. The record is the one
	* contained in the database, not a copy like the read*() functions
	* give you -- so be careful with it. Don't delete it, in any case.
	* Casting it to non-const and marking it deleted is OK, though,
	* which is mostly its intended use.
	*/
	const PilotRecord *findNextNewRecord();

	/**
	* Reads the next record from database that has the dirty flag set.
	* ind (if a valid pointer is given) will receive the index of the
	* returned record.
	*/
	virtual PilotRecord* readNextModifiedRec(int *ind=0L);
	// Writes a new record to database (if 'id' == 0, none is assigned, either)
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


	/** Update the ID of the current record in the database with
	* the specified @param id . This is allowed only after
	* reading or writing a modified or new record.
	*/
	virtual recordid_t  updateID(recordid_t id);


	/** Return the name of the database (as it would be on the handheld). */
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
		{ if (isOpen()) return fAppLen; else return -1; } ;
	char *appInfo() { return fAppInfo; } ;

	const struct DBInfo &getDBInfo() const { return fDBInfo; }
	void setDBInfo(const struct DBInfo &dbi) {fDBInfo=dbi; }

	virtual DBType dbType() const;

	/** Reads local file @p path and fills in the DBInfo
	*   structure @p d with the DBInfo from the file.
	*
	*   @return @c false if d is NULL
	*   @return @c false if the file @p path does not exist
	*   @return @c true if reading the DBInfo succeeds
	*
	*   @note Relatively expensive operation, since the pilot-link
	*         library doesn't provide a cheap way of getting this
	*         information.
	*/
	static bool infoFromFile( const QString &path, DBInfo *d );

protected:
	// Changes any forward slashes to underscores
	void fixupDBName();
	virtual void openDatabase();
	virtual void closeDatabase();

private:
	struct DBInfo fDBInfo;
	QString fPathName,fDBName;
	char*       fAppInfo;
	size_t      fAppLen;

	class Private;
	Private *d;

public:
	/**
	* For databases opened by name only (constructor 2 -- which is the
	* preferred one, too) try this path first before the default path.
	* Set statically so it's shared for all local databases.
	*/
	static void setDBPath(const QString &);
	/**
	* Accessor for the extra search path.
	*/
	static const QString &getDBPath() { return *fPathBase; } ;
private:
	static QString *fPathBase;
};

#endif
