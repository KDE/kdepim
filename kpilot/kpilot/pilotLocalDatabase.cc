// pilotLocalDatabase.cc
//
// Copyright (C) 1998,1999 Dan Pilone
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING.
//
//
static const char *id="$Id$";

#include "options.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream.h>
#include <string.h>
#include "pilotLocalDatabase.h"

PilotLocalDatabase::PilotLocalDatabase(const char* path, const char* dbName)
  : PilotDatabase(), fPathName(0L), fDBName(0L), fAppInfo(0L), fAppLen(0), 
    fNumRecords(0), fCurrentRecord(0), fPendingRec(-1)
    {
    fPathName = new char[strlen(path) + 1];
    strcpy(fPathName, path);
    fDBName = new char[strlen(dbName) + 1];
    strcpy(fDBName, dbName);
    checkDBName();
    openDatabase();
    }

PilotLocalDatabase::~PilotLocalDatabase()
    {
    int i;

    closeDatabase();
    delete [] fPathName;
    delete [] fDBName;
    delete [] fAppInfo;
    for(i = 0; i < fNumRecords; i++)
	delete fRecords[i];
    }

// Changes any forward slashes to underscores
void PilotLocalDatabase::checkDBName()
    {
    int i = -1;
    while(fDBName[++i])
	if(fDBName[i] == '/')
	    fDBName[i] = '_';
    }

// Reads the application block info
int PilotLocalDatabase::readAppBlock(unsigned char* buffer, int )
    {
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readAppBlock() - Error: DB not open!" << endl;
	return -1;
	}
    memcpy((void*)buffer, fAppInfo, fAppLen);
    return fAppLen;
#ifdef DEBUG
	/* NOTREACHED */
	(void) id;
#endif
    }

// Writes the application block info.
int PilotLocalDatabase::writeAppBlock(unsigned char* buffer, int len)
{
  if(isDBOpen() == false)
    {
      cerr << "PilotLocalDatabase::writeAppBlock() - Error: DB not open!" << endl;
      return -1;
    }
  delete [] fAppInfo;
  fAppLen = len;
  fAppInfo = new char[fAppLen];
  memcpy(fAppInfo, (void*)buffer, fAppLen);
  return 0;
}

// Reads a record from database by id, returns record length
PilotRecord* PilotLocalDatabase::readRecordById(recordid_t id)
    {
    int i;

    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readRecordById() - Error: DB not open!" << endl;
	return 0L;
	}
    for(i = 0; i < fNumRecords; i++)
	if(fRecords[i]->getID() == id)
	    {
	    PilotRecord* newRecord = new PilotRecord(fRecords[i]);
	    return newRecord;
	    }
    return 0L;
    }

// Reads a record from database, returns the record length
PilotRecord* PilotLocalDatabase::readRecordByIndex(int index)
    {
    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readRecordByIndex() - Error: DB not open!" << endl;
	return 0L;
	}
    if(index >= fNumRecords)
	return 0L;
    PilotRecord* newRecord = new PilotRecord(fRecords[index]);
    return newRecord;
    }

// Reads the next record from database in category 'category'
PilotRecord* PilotLocalDatabase::readNextRecInCategory(int category)
    {
    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readNextRecInCategory() - Error: DB not open!" << endl;
	return 0L;
	}
    while((fCurrentRecord < fNumRecords) && (fRecords[fCurrentRecord]->getCat() != category))
	fCurrentRecord++;
    if(fCurrentRecord == fNumRecords)
	return 0L;
    PilotRecord* newRecord = new PilotRecord(fRecords[fCurrentRecord]);
    fCurrentRecord++; // so we skip it next time
    return newRecord;
    }

// Reads the next record from database that has the dirty flag set.
PilotRecord* PilotLocalDatabase::readNextModifiedRec()
    {
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readNextModifiedRec() - Error: DB not open!" << endl;
	return 0L;
	}
    // Should this also check for deleted?
    while((fCurrentRecord < fNumRecords) && !(fRecords[fCurrentRecord]->getAttrib() & dlpRecAttrDirty))
	fCurrentRecord++;
    if(fCurrentRecord == fNumRecords)
	return 0L;
    PilotRecord* newRecord = new PilotRecord(fRecords[fCurrentRecord]);
    fPendingRec = fCurrentRecord; // Record which one needs the new id
    fCurrentRecord++; // so we skip it next time
    return newRecord;
    }

// Writes a new ID to the record specified the index.  Not supported on Serial connections
int PilotLocalDatabase::writeID(PilotRecord* rec)
{
  if(isDBOpen() == false)
    {
      cerr << "PilotLocalDatabase::writeID() - Error: DB not open!" << endl;
      return -1;
    }
  if(fPendingRec == -1)
    {
      cerr << "PilotLocalDatabase::writeID() - Error: Last call was _NOT_ readNextModifiedRec()" << endl;
      return -1;
    }
  fRecords[fPendingRec]->setID(rec->getID());
  fPendingRec = -1;
  return 0;
}

// Writes a new record to database (if 'id' == 0, it is assumed that this is a new record to be installed on pilot)
int PilotLocalDatabase::writeRecord(PilotRecord* newRecord)
    {
    int i;

    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::writeRecord() - Error: DB not open!" << endl;
	return -1;
	}
    // We can't do this since it's possible the local apps need to rewrite a record
    // that also exists on the pilot, ie: it would already have a uid but incorrectly
    // get marked as clean.  So it's up to the app to mark them clean/dirty as appropriate.
//     if(id != 0)
// 	{
// 	// This must have come from the pilot, so turn off the modified flags
// 	flags = flags & ~dlpRecAttrDirty;
// 	}
//     else
// 	flags = flags | dlpRecAttrDirty;

    // Instead of making the app do it, assume that whenever a record is
    // written to the database it is dirty.  (You can clean up the database with 
    // resetSyncFlags().)  This will make things get copied twice during a hot-sync
    // but shouldn't cause any other major headaches.
    newRecord->setAttrib(newRecord->getAttrib() | dlpRecAttrDirty);

    // First check to see if we have this record:
    if(newRecord->getID() != 0)
      {
	for(i = 0; i < fNumRecords; i++)
	  if(fRecords[i]->getID() == newRecord->getID())
	    {
	      delete fRecords[i];
	      fRecords[i] = new PilotRecord(newRecord);
	      return 0;
	    }
      }
    // Ok, we don't have it, so just tack it on.
    fRecords[fNumRecords++] = new PilotRecord(newRecord);
    return 0;
    }

// Resets all records in the database to not dirty.
int PilotLocalDatabase::resetSyncFlags()
    {
    int i;

    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::readRecordByIndex() - Error.DB not open!" << endl;
	return -1;
	}
    for(i = 0; i < fNumRecords; i++)
	fRecords[i]->setAttrib(fRecords[i]->getAttrib() & ~dlpRecAttrDirty);
    return 0;
    }

// Resets next record index to beginning
int PilotLocalDatabase::resetDBIndex()
    {
    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::resetDBIndex() - Error: DB not open!" << endl;
	return -1;
	}
    fCurrentRecord = 0;
    return 0;
    }

// Purges all Archived/Deleted records from Palm Pilot database
int PilotLocalDatabase::cleanUpDatabase()
    {
    fPendingRec = -1;
    if(isDBOpen() == false)
	{
	cerr << "PilotLocalDatabase::cleanUpDatabase() - Error: DB not open!" << endl;
	return -1;
	}
    int i,j;
    for(i = 0; (i < fNumRecords) && (fRecords[i]);)
      if(fRecords[i]->getAttrib() & dlpRecAttrDeleted)
	{
	  delete fRecords[i];
	  if((i + 1) < fNumRecords)
	    for(j = i + 1; j < fNumRecords; j++)
	      fRecords[j-1] = fRecords[j];
	  else
	    fRecords[i] = 0L;
	  fNumRecords--;
	}
      else i++;
	
    // Don't have to do anything.  Will be taken care of by closeDatabase()...
    // Changed!
    return 0;
    }

void PilotLocalDatabase::openDatabase()
    {
    void* tmpBuffer;
    pi_file* dbFile;
    char tempName[256];
    int size, attr, cat;
    pi_uid_t id;
    
    strcpy(tempName, fPathName);
    strcat(tempName, "/");
    strcat(tempName, getDBName());
    strcat(tempName, ".pdb");
    dbFile = pi_file_open(tempName);
    if(dbFile == 0L)
	{
	cerr << "Failed to open " << tempName << endl;
	return;
	}
    pi_file_get_info(dbFile, &fDBInfo);
    pi_file_get_app_info(dbFile, &tmpBuffer, &fAppLen);
    fAppInfo = new char[fAppLen];
    memcpy(fAppInfo, tmpBuffer, fAppLen);
    while(pi_file_read_record(dbFile, fCurrentRecord, 
			      &tmpBuffer, &size, &attr, &cat, &id) == 0)
	{
	fRecords[fCurrentRecord] = new PilotRecord(tmpBuffer, size, attr, cat, id);
	fCurrentRecord++;
	}
    pi_file_close(dbFile); // We done with it once we've read it in.
    fNumRecords = fCurrentRecord;
    fCurrentRecord = 0;
    setDBOpen(true);
    }

void PilotLocalDatabase::closeDatabase()
    {
    pi_file* dbFile;
    char tempName[256];
    char newName[256];
    int i;

    if(isDBOpen() == false)
	return;
    strcpy(tempName, fPathName);
    strcat(tempName, "/");
    strcat(tempName, getDBName());
    strcat(tempName, ".pdb");
    strcpy(newName, tempName);
    strcat(newName, ".bak");
    dbFile = pi_file_create(newName, &fDBInfo);
    pi_file_set_app_info(dbFile, fAppInfo, fAppLen);
    for(i = 0; i < fNumRecords; i++)
	{
	  //	if(!(fRecords[i]->getAttrib() & dlpRecAttrDeleted))
	    pi_file_append_record(dbFile, fRecords[i]->getData(), fRecords[i]->getLen(),
				  fRecords[i]->getAttrib(), fRecords[i]->getCat(),
				  fRecords[i]->getID());
	}
    pi_file_close(dbFile);
    unlink(tempName);
    rename(newName, tempName);
    }
