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
#ifndef _PILOTSERIALDATABASE_H
#define _PILOTSERIALDATABASE_H

// Database class for a database on the pilot connected
// via the serial port (ie: hot-sync cradle)

#include "pilot-link/include/pi-macros.h"

#include "pilotDatabase.h"
#include "pilotRecord.h"

class PilotDaemon;

class PilotSerialDatabase : public PilotDatabase
{
friend class PilotDaemon;

protected:
	PilotSerialDatabase(int sd,const char* dbName);
	virtual ~PilotSerialDatabase();

public:
	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	virtual int writeAppBlock(unsigned char* buffer, int len);  
	virtual PilotRecord* readRecordById(recordid_t id);
	virtual PilotRecord* readRecordByIndex(int index);
	virtual PilotRecord* readNextRecInCategory(int category);
	virtual PilotRecord* readNextModifiedRec();
	virtual recordid_t writeID(PilotRecord* ) { return 0; }
	virtual recordid_t writeRecord(PilotRecord* newRecordb);
	virtual int resetSyncFlags();
	virtual int resetDBIndex();
	virtual int cleanUpDatabase();

	virtual int recordCount() const;

	const char *getDBName() const { return fDBName; }

protected:
	void openDatabase();
	void closeDatabase();

	int getDBHandle() const { return fDBHandle; }
	int getSocket() const { return fSocket; } 

private:
	void setDBHandle(int handle) { fDBHandle = handle; }

	int         fSocket;
	char        *fDBName;
	int         fDBHandle;
};

#endif


// $Log$
// Revision 1.1.1.1  2001/06/21 19:50:05  adridg
// KitchenSync is the next-gen KDE-PIM Handheld Device Synchronization
// Framework, which aims to integrate all the Handheld sync tools in 
// KDE, such as KPilot and Kandy. (This is the *real* import).
//
