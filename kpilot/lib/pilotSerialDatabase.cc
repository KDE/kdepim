/* pilotSerialDatabase.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Databases approached through DLP / Pilot-link look different,
** so this file defines an API for them.
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
#include "options.h"

#include <time.h>
#include <iostream.h>

#include <pi-dlp.h>

#include <klocale.h>
#include <kdebug.h>


#include "pilotSerialDatabase.h"

static const char *pilotSerialDatabase_id =
	"$Id$";

PilotSerialDatabase::PilotSerialDatabase(int linksocket,
	const char *dbName,
	QObject *p,const char *n) :
	PilotDatabase(p,n),
	fDBName(0L), 
	fDBSocket(linksocket), 
	fDBHandle(-1)
{
	FUNCTIONSETUP;
	fDBName = new char[strlen(dbName) + 1];

	strcpy(fDBName, dbName);
	openDatabase();

	/* NOTREACHED */
	(void) pilotSerialDatabase_id;
}

PilotSerialDatabase::~PilotSerialDatabase()
{
	FUNCTIONSETUP;
	closeDatabase();
	delete[]fDBName;
}

// Reads the application block info
int PilotSerialDatabase::readAppBlock(unsigned char *buffer, int maxLen)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_ReadAppBlock(fDBSocket, getDBHandle(), 0, (void *) buffer,
		maxLen);
}

// Writes the application block info.
int PilotSerialDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_WriteAppBlock(fDBSocket, getDBHandle(), buffer, len);
}


// Reads a record from database by id, returns record length
PilotRecord *PilotSerialDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUP;
	char *buffer[0xffff];
	int index, size, attr, category;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadRecordById(fDBSocket, getDBHandle(), id, buffer, &index,
			&size, &attr, &category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads a record from database, returns the record length
PilotRecord *PilotSerialDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUP;
	char *buffer[0xffff];
	int size, attr, category;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadRecordByIndex(fDBSocket, getDBHandle(), index,
			(void *) buffer, &id, &size, &attr, &category) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads the next record from database in category 'category'
PilotRecord *PilotSerialDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	char *buffer[0xffff];
	int index, size, attr;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadNextRecInCategory(fDBSocket, getDBHandle(),
			category, buffer, &id, &index, &size, &attr) >= 0)
		return new PilotRecord(buffer, size, attr, category, id);
	return 0L;
}

// Reads the next record from database that has the dirty flag set.
PilotRecord *PilotSerialDatabase::readNextModifiedRec()
{
	FUNCTIONSETUP;
	char *buffer[0xffff];
	int index, size, attr, category;
	recordid_t id;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0L;
	}
	if (dlp_ReadNextModifiedRec(fDBSocket, getDBHandle(), (void *) buffer,
			&id, &index, &size, &attr, &category) >= 0)
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
		kdError() << k_funcinfo << ": DB not open" << endl;
		return 0;
	}
	success =
		dlp_WriteRecord(fDBSocket, getDBHandle(),
		newRecord->getAttrib(), newRecord->getID(),
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
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetSyncFlags(fDBSocket, getDBHandle());
}

// Resets next record index to beginning
int PilotSerialDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_ResetDBIndex(fDBSocket, getDBHandle());
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotSerialDatabase::cleanUpDatabase()
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open" << endl;
		return -1;
	}
	return dlp_CleanUpDatabase(fDBSocket, getDBHandle());
}

void PilotSerialDatabase::openDatabase()
{
	FUNCTIONSETUP;
	int db;

	if (dlp_OpenDB(fDBSocket, 0, dlpOpenReadWrite, 
		const_cast<char *>(getDBName()), &db) < 0)
	{
		kdError() << k_funcinfo
			<< i18n("Cannot open database")
			<< i18n("Pilot database error") << endl;
		return;
	}
	setDBHandle(db);
	setDBOpen(true);
}

void PilotSerialDatabase::closeDatabase()
{
	FUNCTIONSETUP;
	if (!isDBOpen() ) return;

	dlp_CloseDB(fDBSocket, getDBHandle());
	setDBOpen(false);
}


// $Log$
// Revision 1.13  2001/09/16 13:37:48  adridg
// Large-scale restructuring
//
// Revision 1.12  2001/04/16 13:54:17  adridg
// --enable-final file inclusion fixups
//
// Revision 1.11  2001/03/27 23:54:43  stern
// Broke baseConduit functionality out into PilotConduitDatabase and added support for local mode in BaseConduit
//
// Revision 1.10  2001/03/27 11:10:39  leitner
// ported to Tru64 unix: changed all stream.h to iostream.h, needed some
// #ifdef DEBUG because qstringExpand etc. were not defined.
//
// Revision 1.9  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.8  2001/02/05 20:58:48  adridg
// Fixed copyright headers for source releases. No code changed
//
