/* pilotSerialDatabase.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** See the .cc file for an explanation of what this file is for.
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
#ifndef __PILOT_SERIAL_DATABASE_H
#define __PILOT_SERIAL_DATABASE_H

// Database class for a database on the pilot connected
// via the serial port (ie: hot-sync cradle)

#include <pi-macros.h> /* for recordid_t */
#include <pilotDatabase.h>
#include <pilotRecord.h>

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


// $Log$
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
