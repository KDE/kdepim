/* pilotDatabase.h			KPilot
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
#ifndef __PILOT_DATABASE_H
#define __PILOT_DATABASE_H

// Abstract base class for PalmPilot database access.

#include <pi-macros.h> /* for recordid_t */
#include "pilotRecord.h"

class KPilotLink;

/**
 * Methods to access a database on the pilot.  NOTE:  It is the users responsibility
 * to delete the PilotRecord return when finished with it!!
 */

class PilotDatabase
    {
    public:
    PilotDatabase() : fDBOpen(false)  { }
    virtual ~PilotDatabase() { }

    /** Reads the application block info, returns size. */
    virtual int readAppBlock(unsigned char* buffer, int maxLen) = 0;
    /** Writes the application block info. */
    virtual int writeAppBlock(unsigned char* buffer, int len) = 0;  
    /** Reads a record from database by id, returns record length */
    virtual PilotRecord* readRecordById(recordid_t id) = 0;
    /** Reads a record from database, returns the record length */
    virtual PilotRecord* readRecordByIndex(int index) = 0;
    /** Reads the next record from database in category 'category' */
    virtual PilotRecord* readNextRecInCategory(int category) = 0;
    /** Reads the next record from database that has the dirty flag set. */
    virtual PilotRecord* readNextModifiedRec() = 0;
    /** Writes a new ID to the record specified the index.  Not supported on Serial connections */
    virtual int writeID(PilotRecord* rec) = 0;
    /** Writes a new record to database (if 'id' == 0, one will be assigned to newRecord) */
    virtual int writeRecord(PilotRecord* newRecord) = 0;
    /** Resets all records in the database to not dirty. */
    virtual int resetSyncFlags() = 0;
    /** Resets next record index to beginning */
    virtual int resetDBIndex() = 0;
    /** Purges all Archived/Deleted records from Palm Pilot database */
    virtual int cleanUpDatabase() = 0;

    bool isDBOpen() { return fDBOpen; }

    protected:
    virtual void openDatabase() = 0;
    virtual void closeDatabase() = 0;

    void setDBOpen(bool yesno) { fDBOpen = yesno; }

    private:
    bool fDBOpen;
    };

#endif


// $Log$
// Revision 1.4  2001/02/06 08:05:20  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
