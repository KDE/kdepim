/* pilotLocalDatabase.h         PilotDaemon
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
#ifndef _PILOTLOCALDATABASE_H
#define _PILOTLOCALDATABASE_H

#include <qlist.h>

#include <pilot-link/include/pi-macros.h>

#include "pilotDatabase.h"

class PilotLocalDatabase : public PilotDatabase
{
public:
	PilotLocalDatabase(const QString& path, const QString& name);
	virtual ~PilotLocalDatabase();


	virtual int readAppBlock(unsigned char* buffer, int maxLen);
	virtual int writeAppBlock(unsigned char* buffer, int len);  
	virtual PilotRecord* readRecordById(recordid_t id);
	virtual PilotRecord* readRecordByIndex(int index);
	virtual PilotRecord* readNextRecInCategory(int category);
	virtual PilotRecord* readNextModifiedRec();
	virtual recordid_t writeID(PilotRecord* rec);
	virtual recordid_t writeRecord(PilotRecord* newRecord);
	virtual int resetSyncFlags();
	virtual int resetDBIndex();
	virtual int cleanUpDatabase();

	virtual int recordCount() const { return fRecordList.count(); } ;

	/**
	* Returns the full path of the current database, based on
	* the path and dbname passed to the constructor, and including
	* the .pdb extension.
	*/
	QString dbPathName() const;

protected:
	// Changes any forward slahses to underscores
	QString checkDBName(const QString &);

	void openDatabase();
	void closeDatabase();

private:
	struct DBInfo fDBInfo;
	QString fPathName;
	char*       fAppInfo;
	int         fAppLen;
	int         fPendingRec;

	QList<PilotRecord> fRecordList;
};

#endif


// $Log$
