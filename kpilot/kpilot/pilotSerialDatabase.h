/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __PILOT_SERIAL_DATABASE_H
#define __PILOT_SERIAL_DATABASE_H

// Database class for a database on the pilot connected
// via the serial port (ie: hot-sync cradle)

#include "pi-macros.h" /* for recordid_t */
#include "pilotDatabase.h"
#include "pilotRecord.h"

class KPilotLink;

class PilotSerialDatabase : public PilotDatabase
    {
    public:
    PilotSerialDatabase(KPilotLink* pilotLink, const char* dbName);
    virtual ~PilotSerialDatabase();

    /** Reads the application block info, returns size */
    virtual int readAppBlock(unsigned char* buffer, int maxLen);
    /** Writes the application block info. */
    virtual int writeAppBlock(unsigned char* buffer, int len);  
    /** Reads a record from database by id, returns record length */
    virtual PilotRecord* readRecordById(recordid_t id);
    /** Reads a record from database, returns the record length */
    virtual PilotRecord* readRecordByIndex(int index);
    /** Reads the next record from database in category 'category' */
    virtual PilotRecord* readNextRecInCategory(int category);
    /** Reads the next record from database that has the dirty flag set. */
    virtual PilotRecord* readNextModifiedRec();
    /** Writes a new ID to the record specified the index.  Not supported on Serial connections */
    virtual int writeID(PilotRecord* ) { return 0; }
    /** Writes a new record to database (if 'id' == 0, one will be assigned to newRecord) */
    virtual int writeRecord(PilotRecord* newRecord);
    /** Resets all records in the database to not dirty. */
    virtual int resetSyncFlags();
    /** Resets next record index to beginning */
    virtual int resetDBIndex();
    /** Purges all Archived/Deleted records from Palm Pilot database */
    virtual int cleanUpDatabase();

    char* getDBName() { return fDBName; }

    protected:
    void openDatabase();
    void closeDatabase();
    int getDBHandle() { return fDBHandle; }

    KPilotLink* getPilotLink() { return fPilotLink; }

    private:
    void setDBHandle(int handle) { fDBHandle = handle; }
    char*       fDBName;
    int         fDBHandle;
    KPilotLink* fPilotLink;
    };

#endif
