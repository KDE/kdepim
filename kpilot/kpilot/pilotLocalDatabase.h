/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_LOCAL_DATABASE_H
#define __PILOT_LOCAL_DATABASE_H

// Database class for a local (file based) pilot datbase.

#include <time.h>
#include "pi-macros.h" /* for recordid_t */
#include "pi-file.h"
#include "pilotDatabase.h"
#include "pilotRecord.h"

class PilotLocalDatabase : public PilotDatabase
    {
    public:
    PilotLocalDatabase(const char* path, const char* dbName);
    virtual ~PilotLocalDatabase();

    // Changes any forward slahses to underscores
    void checkDBName();
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
    // Writes a new ID to the record specified the index.  Not supported on Serial connections
    virtual int writeID(PilotRecord* rec);
    // Writes a new record to database (if 'id' == 0, one will be assigned to newRecord)
    virtual int writeRecord(PilotRecord* newRecord);
    // Resets all records in the database to not dirty.
    virtual int resetSyncFlags();
    // Resets next record index to beginning
    virtual int resetDBIndex();
    // Purges all Archived/Deleted records from Palm Pilot database
    virtual int cleanUpDatabase();

    char* getDBName() { return fDBName; }

    protected:
    void openDatabase();
    void closeDatabase();

    private:
    struct DBInfo fDBInfo;
    char*       fPathName;
    char*       fDBName;
    char*       fAppInfo;
    int         fAppLen;
    int         fNumRecords;
    int         fCurrentRecord;
    PilotRecord* fRecords[10000]; // Current max records in DB.. hope it's enough
    int         fPendingRec; // Temp index for the record about to get an ID.
    };

#endif
