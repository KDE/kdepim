/* pilotLocalDatabase.cc        PilotDaemon
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This defines an interface to Pilot databases on the local disk.
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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/


static const char *pilotlocaldatabase_id =
	"$Id$";

#include <config.h>
#include "../lib/debug.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream.h>

#include <qstring.h>
#include <qfile.h>


#include "pilotLocalDatabase.h"

PilotLocalDatabase::PilotLocalDatabase(const QString & path,
	const QString & dbName) : PilotDatabase(checkDBName(dbName)),
	fPathName(path),
	fAppInfo(0L), fAppLen(0), 
	fPendingRec(-1)
{
	FUNCTIONSETUP;

	openDatabase();

	fRecordList.setAutoDelete(true);

	/* NOTREACHED */
	(void) pilotlocaldatabase_id;
}

#if 0
PilotLocalDatabase::PilotLocalDatabse(const QString &path,
	PilotSerialDatabase *db) : 
	PilotDatabase(checkDBName(db->name())),
	fPathName(path),
	fAppInfo(0L),fAppLen(0),
	fPendingRec(-1)
{
	FUNCTIONSETUP;

	// Must copy DBInfo from serial db and also
	// get AppInfo.

	fRecordList.setAutoDelete(true);
}
#endif

PilotLocalDatabase::~PilotLocalDatabase()
{
	FUNCTIONSETUP;

	closeDatabase();
	delete[]fAppInfo;
}

// Changes any forward slashes to underscores
QString PilotLocalDatabase::checkDBName(const QString &s)
{
	FUNCTIONSETUP;

	QString t = s;
	t=t.replace(QRegExp("/"), "_");

	return t;
}

// Reads the application block info
int PilotLocalDatabase::readAppBlock(unsigned char *buffer, int)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return -1;
	}
	memcpy((void *) buffer, fAppInfo, fAppLen);
	return fAppLen;
}

// Writes the application block info.
int PilotLocalDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return -1;
	}
	delete[]fAppInfo;
	fAppLen = len;
	fAppInfo = new char[fAppLen];

	memcpy(fAppInfo, (void *) buffer, fAppLen);
	return 0;
}

// Reads a record from database by id, returns record length
PilotRecord *PilotLocalDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUP;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdDebug() << fname << ": DB not open!" << endl;
		return 0L;
	}

	for (PilotRecord *r=fRecordList.first(); r; r=fRecordList.next() )
	{
		if (r->getID() == id)
		{
			PilotRecord *newRecord = new PilotRecord(r);
			return newRecord;
		}
	}

	return 0L;
}

PilotRecord *PilotLocalDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return 0L;
	}

	PilotRecord *r = fRecordList.at(index);

	if (r)
	{
		return new PilotRecord(r);
	}
	else
	{
		return 0L;
	}
}

// Reads the next record from database in category 'category'
PilotRecord *PilotLocalDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return 0L;
	}

	for (PilotRecord *r = fRecordList.current(); r; r=fRecordList.next() )
	{
		if (r->getCat() == category) 
		{
			// Skip over the record just found, for next time.
			(void) fRecordList.next();
			return new PilotRecord(r);
		}
	}

	return 0L;
}

// Reads the next record from database that has the dirty flag set.
PilotRecord *PilotLocalDatabase::readNextModifiedRec()
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return 0L;
	}

	for (PilotRecord *r = fRecordList.current(); r; r=fRecordList.next() )
	{
		if (r->isDirty()) 
		{
			fPendingRec = fRecordList.at();
			// Skip over the record just found, for next time.
			(void) fRecordList.next();
			return new PilotRecord(r);
		}
	}

	return 0L;
}

// Writes a new ID to the record specified the index.  Not supported on Serial connections
recordid_t PilotLocalDatabase::writeID(PilotRecord * rec)
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return 0;
	}
	if (fPendingRec == -1)
	{
		kdError() << __FUNCTION__ <<
			": Last call was _NOT_ readNextModifiedRec()" << endl;
		return 0;
	}

	fRecordList.at(fPendingRec)->setID(rec->getID());
	fPendingRec = -1;
	return rec->getID();
}

// Writes a new record to database (if 'id' == 0, it is assumed that this is a new record to be installed on pilot)
recordid_t PilotLocalDatabase::writeRecord(PilotRecord * newRecord)
{
	FUNCTIONSETUP;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return 0;
	}

	// Instead of making the app do it, assume that whenever a record is
	// written to the database it is dirty.  (You can clean up the database with 
	// resetSyncFlags().)  This will make things get copied twice during a hot-sync
	// but shouldn't cause any other major headaches.
	newRecord->setAttrib(newRecord->getAttrib() | dlpRecAttrDirty);

	// First check to see if we have this record:
	if (newRecord->getID() != 0)
	{
		PilotRecord *r = 0L;

		for (r=fRecordList.first(); r; r=fRecordList.next())
		{
			if (r->getID() == newRecord->getID())
			{
				(*r) = (*newRecord);
				return 0;
			}
		}
	}
	// Ok, we don't have it, so just tack it on.
	fRecordList.append(new PilotRecord(newRecord));
	return newRecord->getID();
}

// Resets all records in the database to not dirty.
int PilotLocalDatabase::resetSyncFlags()
{
	FUNCTIONSETUP;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return -1;
	}

	for (PilotRecord *r = fRecordList.first(); r; r=fRecordList.next())
	{
		r->makeDirty(false);
	}

	return 0;
}

// Resets next record index to beginning
int PilotLocalDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return -1;
	}

	(void) fRecordList.first();

	return 0;
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotLocalDatabase::cleanUpDatabase()
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open!" << endl;
		return -1;
	}

	for (PilotRecord *r=fRecordList.first(); r; r=fRecordList.next())
	{
		// Removing an item from the list of records
		// automatically makes the next item in the
		// list current (see Qt 2.3 docs), and then
		// next() would skip over one record. So to handle
		// that, we explicitly look at the record after the
		// record we remove (which is current, once the
		// first record is removed).
		//
		// Of course, this process may repeat itself.
		//
		//
		while (r && r->isDeleted())
		{
			fRecordList.remove(r);
			r=fRecordList.current();
		}
	}

	return 0;
}

QString PilotLocalDatabase::dbPathName() const
{
	QString tempName(fPathName);

	tempName += "/";
	tempName += name();
	tempName += ".pdb";
	return tempName;
}

void PilotLocalDatabase::openDatabase()
{
	FUNCTIONSETUP;

	void *tmpBuffer;
	pi_file *dbFile;
	int size, attr, cat;
	pi_uid_t id;

	QString tempName = dbPathName();
	QCString fileName = QFile::encodeName(tempName);
	dbFile = pi_file_open(const_cast < char *>((const char *) fileName));

	if (dbFile == 0L)
	{
		kdError() << __FUNCTION__
			<< ": Failed to open " << tempName << endl;
		return;
	}
	pi_file_get_info(dbFile, &fDBInfo);
	pi_file_get_app_info(dbFile, &tmpBuffer, &fAppLen);
	fAppInfo = new char[fAppLen];

	memcpy(fAppInfo, tmpBuffer, fAppLen);

	int recordno=0;
	while (pi_file_read_record(dbFile, recordno,
			&tmpBuffer, &size, &attr, &cat, &id) == 0)
	{
		fRecordList.append(
			new PilotRecord(tmpBuffer, size, attr, cat, id));
		recordno++;
	}
	pi_file_close(dbFile);	// We done with it once we've read it in.
	setDBOpen(true);
}

void PilotLocalDatabase::closeDatabase()
{
	pi_file *dbFile;

	if (isDBOpen() == false)
		return;

	QString tempName_ = dbPathName();
	QString newName_ = tempName_ + ".bak";
	QCString tempName = QFile::encodeName(tempName_);
	QCString newName = QFile::encodeName(newName_);

	dbFile = pi_file_create(const_cast < char *>((const char *)newName),
		&fDBInfo);

	pi_file_set_app_info(dbFile, fAppInfo, fAppLen);

	for (PilotRecord *r=fRecordList.first(); r; r=fRecordList.next())
	{
		pi_file_append_record(dbFile,
			r->getData(),
			r->getLen(),
			r->getAttrib(), 
			r->getCat(),
			r->getID());
	}

	pi_file_close(dbFile);
	unlink((const char *) QFile::encodeName(tempName));
	rename((const char *) QFile::encodeName(newName),
		(const char *) QFile::encodeName(tempName));
	setDBOpen(false);
}


// $Log$
// Revision 1.1.1.1  2001/06/21 19:50:09  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
