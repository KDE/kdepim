/* pilotConduitDatabase.h			KPilot
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
** Bug reports and questions can be sent to adridg@cs.kun.nl or
*  gstern@slac.com
*/
#ifndef _KPILOT_PILOTCONDUITDATABASE_H
#define _KPILOT_PILOTCONDUITDATABASE_H

#ifndef _KPILOT_PILOTDATABASE_H
#include "pilotDatabase.h"
#endif

class KSocket;
/**
 * Extract the baseConduit socket code out and into this class
 *
 */
class PilotConduitDatabase : public PilotDatabase
    {
    public :
      PilotConduitDatabase();
      ~PilotConduitDatabase();

      /**
	* Adds a message to the sync log by talking to
	* the daemon; the message s should not contain
	* any `weird' characters, no \r or \n, and must be
	* null-terminated. In addition, s must be no longer
	* than 30 characters.
	* Returns 1 on success, 0 on failure.
	*/
	bool addSyncLogMessage(const char *s);
      
      /** Reads the application block info, returns size. */
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
      /** Writes a new ID to the record specified the index.
       * Not supported on Serial connections */
      virtual recordid_t writeID(PilotRecord* rec);
      /** Writes a new record to database
       *  (if 'id' == 0, one will be assigned to newRecord)
       * @return the new record id
       */
      virtual recordid_t writeRecord(PilotRecord* newRecord);

      // Resets all records in the database to not dirty.
      virtual int resetSyncFlags() { return 1; }
      // Resets next record index to beginning
      virtual int resetDBIndex() { return 1; }
      // Purges all Archived/Deleted records from Palm Pilot database
      virtual int cleanUpDatabase() { return 1; }

    protected :
      virtual void openDatabase();
      virtual void closeDatabase();
    private :
      PilotRecord* _getRecord();
      void _writeRecord(PilotRecord* rec);

      KSocket *fDaemonSocket;
    };

#endif
