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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#include "options.h"

#include <time.h>
#include <stream.h>
#include <pi-dlp.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>

#include "pilotSerialDatabase.h"
#include "kpilotlink.h"

PilotSerialDatabase::PilotSerialDatabase(KPilotLink* pilotLink, const char* dbName)
  : PilotDatabase(), fDBName(0L), fDBHandle(-1), fPilotLink(pilotLink)
    {
    fDBName = new char[strlen(dbName) + 1];
    strcpy(fDBName, dbName);
    openDatabase();
    }

PilotSerialDatabase::~PilotSerialDatabase()
    {
    closeDatabase();
    delete [] fDBName;
    }

// Reads the application block info
int PilotSerialDatabase::readAppBlock(unsigned char* buffer, int maxLen)
    {
	FUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return -1;
	}
    return dlp_ReadAppBlock(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), 0, (void *)buffer, maxLen);
    }

// Writes the application block info.
int PilotSerialDatabase::writeAppBlock(unsigned char* buffer, int len)
{
	FUNCTIONSETUP;
  if(isDBOpen() == false)
    {
      kdError() << __FUNCTION__ << ": DB not open" << endl;
      return -1;
    }
  return dlp_WriteAppBlock(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), buffer, len);
}


// Reads a record from database by id, returns record length
PilotRecord* PilotSerialDatabase::readRecordById(recordid_t id)
    {
	FUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr, category;

    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadRecordById(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), id, buffer, &index,
			      &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Reads a record from database, returns the record length
PilotRecord* PilotSerialDatabase::readRecordByIndex(int index)
    {
	FUNCTIONSETUP;
    char* buffer[0xffff];
    int size, attr, category;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadRecordByIndex(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), index, 
				 (void*)buffer, &id, &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;	
    }

// Reads the next record from database in category 'category'
PilotRecord* PilotSerialDatabase::readNextRecInCategory(int category)
    {
	FUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadNextRecInCategory (getPilotLink()->getCurrentPilotSocket(), getDBHandle(), 
				      category, buffer, &id, &index, &size, &attr) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Reads the next record from database that has the dirty flag set.
PilotRecord* PilotSerialDatabase::readNextModifiedRec()
    {
	FUNCTIONSETUP;
    char* buffer[0xffff];
    int index, size, attr, category;
    recordid_t id;

    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return 0L;
	}
    if(dlp_ReadNextModifiedRec(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), (void*)buffer, 
				   &id, &index, &size, &attr, &category) >= 0)
	return new PilotRecord(buffer, size, attr, category, id);
    return 0L;
    }

// Writes a new record to database (if 'id' == 0, one will be assigned and returned in 'newid')
int PilotSerialDatabase::writeRecord(PilotRecord* newRecord)
    {
	FUNCTIONSETUP;
    recordid_t newid;
    int success;

    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return -1;
	}
    success = dlp_WriteRecord(getPilotLink()->getCurrentPilotSocket(), getDBHandle(), newRecord->getAttrib(), 
			      newRecord->getID(), newRecord->getCat(), newRecord->getData(),
			      newRecord->getLen(), &newid);
    if(newRecord->getID() == 0)
	newRecord->setID(newid);
    return success;
    }

// Resets all records in the database to not dirty.
int PilotSerialDatabase::resetSyncFlags()
    {
	FUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return -1;
	}
    return dlp_ResetSyncFlags(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

// Resets next record index to beginning
int PilotSerialDatabase::resetDBIndex()
    {
	FUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return -1;
	}
    return dlp_ResetDBIndex(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

// Purges all Archived/Deleted records from Palm Pilot database
int PilotSerialDatabase::cleanUpDatabase()
    {
	FUNCTIONSETUP;
    if(isDBOpen() == false)
	{
	kdError() << __FUNCTION__ << ": DB not open" << endl;
	return -1;
	}
    return dlp_CleanUpDatabase(getPilotLink()->getCurrentPilotSocket(), getDBHandle()); 
    }

void PilotSerialDatabase::openDatabase()
    {
    int db;

    if(dlp_OpenDB(getPilotLink()->getCurrentPilotSocket(), 0, dlpOpenReadWrite, getDBName(), &db) < 0)
	{
	KMessageBox::error(getPilotLink()->getOwningWidget(),
		i18n("Cannot open database"),
		i18n("Pilot database error"));
	return;
	}
    setDBHandle(db);
    setDBOpen(true);
    }

void PilotSerialDatabase::closeDatabase()
    {
    if(isDBOpen() == false)
	return;
    dlp_CloseDB(getPilotLink()->getCurrentPilotSocket(), getDBHandle());
    }


// $Log:$
