/* pilotSerialDatabase.cc       PilotDaemon
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2001 by Adriaan de Groot
**
** This program is part of KitchenSync, the KDE handheld-device
** synchronisation framework.
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

static const char *pilotSerialDatabase_id = "$Id$";


#include <config.h>
#include "../lib/debug.h"

#include "pilot-link/include/pi-dlp.h"

#include <kmessagebox.h>
#include <klocale.h>


#include "pilotSerialDatabase.h"


PilotSerialDatabase::PilotSerialDatabase(int sd,const char *dbName) :
	PilotDatabase(QString(dbName)), 
	fSocket(sd),
	fDBName(0L), 
	fDBHandle(-1),
	fDBInfo(0L)
{
	fDBName = new char[strlen(dbName) + 1];

	strcpy(fDBName, dbName);
	openDatabase();
	/* NOTREACHED */
	(void) pilotSerialDatabase_id;
}

PilotSerialDatabase::~PilotSerialDatabase()
{
	closeDatabase();
	delete[]fDBName;
}

// Reads the application block info
int PilotSerialDatabase::readAppBlock(unsigned char *buffer, int maxLen)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return -1;
	}
	return dlp_ReadAppBlock(fSocket,
		getDBHandle(), 0, (void *) buffer, maxLen);
}

// Writes the application block info.
int PilotSerialDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return -1;
	}
	return dlp_WriteAppBlock(fSocket,
		getDBHandle(), buffer, len);
}


// Reads a record from database by id, returns record length
PilotRecord *PilotSerialDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUP;
	char buffer[DLP_BUF_SIZE];
	int index, size, attr, category;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadRecordById(fSocket,
			getDBHandle(), id, buffer, &index, &size, &attr,
			&category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads a record from database, returns the record length
PilotRecord *PilotSerialDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUP;
	char buffer[DLP_BUF_SIZE];
	int size, attr, category;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadRecordByIndex(fSocket,
			getDBHandle(), index, (void *) buffer, &id, &size,
			&attr, &category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads the next record from database in category 'category'
PilotRecord *PilotSerialDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	char buffer[DLP_BUF_SIZE];
	int index, size, attr;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadNextRecInCategory(fSocket,
			getDBHandle(), category, buffer, &id, &index, &size,
			&attr) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads the next record from database that has the dirty flag set.
PilotRecord *PilotSerialDatabase::readNextModifiedRec()
{
	FUNCTIONSETUP;
	char buffer[DLP_BUF_SIZE];
	int index, size, attr, category;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadNextModifiedRec(fSocket,
			getDBHandle(), (void *) buffer, &id, &index, &size,
			&attr, &category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Writes a new record to database (if 'id' == 0, one will be assigned and returned in 'newid')
recordid_t PilotSerialDatabase::writeRecord(PilotRecord * newRecord)
{
	FUNCTIONSETUP;
	recordid_t newid;
	int success;

	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return 0;
	}
	success =
		dlp_WriteRecord(fSocket,
		getDBHandle(), newRecord->getAttrib(), newRecord->getID(),
		newRecord->getCat(), newRecord->getData(),
		newRecord->getLen(), &newid);
	if (newRecord->getID() == 0)
		newRecord->setID(newid);
	return newid;
}

// Resets all records in the database to not dirty.
int PilotSerialDatabase::resetSyncFlags()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetSyncFlags(fSocket,
		getDBHandle());
}

// Resets next record index to beginning
int PilotSerialDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetDBIndex(fSocket,
		getDBHandle());
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotSerialDatabase::cleanUpDatabase()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << __FUNCTION__ << ": DB not open" << endl;
		return -1;
	}
	return dlp_CleanUpDatabase(fSocket,
		getDBHandle());
}

void PilotSerialDatabase::openDatabase()
{
	FUNCTIONSETUP;

	int db;

	if (dlp_OpenDB(fSocket, 0, dlpOpenReadWrite, 
		const_cast<char *>(getDBName()), &db) < 0)
	{
		return;
	}
	setDBHandle(db);
	setDBOpen(true);
}

void PilotSerialDatabase::closeDatabase()
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
		return;
	dlp_CloseDB(fSocket, getDBHandle());
	setDBOpen(false);
}

int PilotSerialDatabase::recordCount() const
{
	FUNCTIONSETUP;

	int records=0;

	if (dlp_ReadOpenDBInfo(fSocket,getDBHandle(),&records)<0)
	{
		kdWarning() << "Can't get number of records." << endl;
		records=-1;
	}

	return records;
}


// $Log$
// Revision 1.1.1.1  2001/06/21 19:50:08  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
