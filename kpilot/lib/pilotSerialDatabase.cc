/* pilotSerialDatabase.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** Databases approached through DLP / Pilot-link look different,
** so this file defines an API for them.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
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

#include <qfile.h>

#include <pi-dlp.h>

#include <klocale.h>
#include <kdebug.h>


#include "pilotSerialDatabase.h"

static const char *pilotSerialDatabase_id =
	"$Id$";

PilotSerialDatabase::PilotSerialDatabase(int linksocket,
	const QString &dbName,
	QObject *p,const char *n) :
	PilotDatabase(p,n),
	fDBName(0L), 
	fDBHandle(-1),
	fDBSocket(linksocket)
{
	FUNCTIONSETUP;
	fDBName = dbName;

	openDatabase();

	/* NOTREACHED */
	(void) pilotSerialDatabase_id;
}

PilotSerialDatabase::~PilotSerialDatabase()
{
	FUNCTIONSETUP;
	closeDatabase();
}

QString PilotSerialDatabase::dbPathName() const
{
	QString s("Pilot:");
	s.append(fDBName);
	return s;
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

	// returns the number of records in the database 
int PilotSerialDatabase::recordCount()
{
	int idlen;
	// dlp_ReadOpenDBInfo returns the number of bytes read and sets idlen to the # of recs
	if (isDBOpen() && dlp_ReadOpenDBInfo(fDBSocket, getDBHandle(), &idlen)>0)
	{
		return idlen;
	}
	else return -1;
}


// Returns a QValueList of all record ids in the database. 
QValueList<recordid_t> PilotSerialDatabase::idList()
{
	QValueList<recordid_t> idlist;
	int idlen=recordCount();
	if (idlen<=0) return idlist;
	
	recordid_t *idarr=new recordid_t[idlen];
	int idlenread;
	dlp_ReadRecordIDList (fDBSocket, getDBHandle(), 0, 0, idlen, idarr, &idlenread); 
	
	// now create the QValue list from the idarr:
	for (idlen=0; idlen<idlenread; idlen++) 
	{
		idlist.append(idarr[idlen]);
	}
	delete[] idarr;
	return idlist;
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
PilotRecord *PilotSerialDatabase::readNextModifiedRec(int *ind)
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
	{
		if (ind) *ind=index;
		return new PilotRecord(buffer, size, attr, category, id);
	}
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

// Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case
int PilotSerialDatabase::deleteRecord(recordid_t id, bool all) 
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo <<": DB not open"<<endl;
		return -1;
	}
	return dlp_DeleteRecord(fDBSocket, getDBHandle(), all?1:0, id);
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
int PilotSerialDatabase::cleanup()
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
		QFile::encodeName(getDBName()).data(), &db) < 0)
	{
		kdError() << k_funcinfo
			<< i18n("Cannot open database")
			<< i18n("Pilot database error") << endl;
		return;
	}
	setDBHandle(db);
	setDBOpen(true);
}

bool PilotSerialDatabase::createDatabase(long creator, long type, int cardno, int flags, int version) 
{
	FUNCTIONSETUP;
	int db;
	
	// if the database is already open, we cannot create it again. How about completely resetting it? (i.e. deleting it and the createing it again)
	if (isDBOpen()) return true;
	int res=dlp_CreateDB(fDBSocket, creator, type, cardno, flags, version, getDBName(), &db);
	if (res<0) {
		kdError() <<k_funcinfo
			<< i18n("Cannot create database %1 on the handheld").arg(getDBName())<<endl;
		return false;
	}
	// TODO: Do I have to open it explicitely???
	setDBHandle(db);
	setDBOpen(true);
	return true;
}

void PilotSerialDatabase::closeDatabase()
{
	FUNCTIONSETUP;
	if (!isDBOpen() ) return;

	dlp_CloseDB(fDBSocket, getDBHandle());
	setDBOpen(false);
}


// $Log$
// Revision 1.6  2002/12/08 14:09:24  waba
// Some cleanup
//
// Revision 1.5  2002/08/20 21:18:31  adridg
// License change in lib/ to allow plugins -- which use the interfaces and
// definitions in lib/ -- to use non-GPL'ed libraries, in particular to
// allow the use of libmal which is MPL.
//
// Revision 1.4  2002/06/30 14:49:53  kainhofe
// added a function idList, some minor bug fixes
//
// Revision 1.3  2002/06/07 07:13:25  adridg
// Make VCal conduit use base-class fDatabase and fLocalDatabase (hack).
// Extend *Database classes with dbPathName() for consistency.
//
// Revision 1.2  2002/05/22 20:40:13  adridg
// Renaming for sensibility
//
// Revision 1.1  2001/10/10 22:01:24  adridg
// Moved from ../kpilot/, shared files
//
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
