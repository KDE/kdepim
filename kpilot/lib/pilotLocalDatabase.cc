/* pilotLocalDatabase.cc			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
**
** This defines an interface to Pilot databases on the local disk.
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


static const char *pilotlocaldatabase_id =
	"$Id$";

#include "options.h"

#include <stdio.h>
#include <unistd.h>

#include <iostream>

#include <qstring.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdatetime.h> 

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>


#include "pilotLocalDatabase.h"

PilotLocalDatabase::PilotLocalDatabase(const QString & path,
	const QString & dbName, bool useDefaultPath,
	QObject *p, const char *n) :
	PilotDatabase(p,n),
	fPathName(path),
	fDBName(dbName),
	fAppInfo(0L), 
	fAppLen(0), 
	fNumRecords(0), 
	fCurrentRecord(0), 
	fPendingRec(-1)
{
	FUNCTIONSETUP;
	fixupDBName();
	openDatabase();

	if (!isDBOpen() && useDefaultPath)
	{
		if (fPathBase && !fPathBase->isEmpty())
		{
			fPathName = *fPathBase;
		}
		else
		{
			fPathName = KGlobal::dirs()->saveLocation("data",
				CSL1("kpilot/DBBackup/"));
		}
		fixupDBName();
		openDatabase();
		if (!isDBOpen())
			fPathName=path;
	}

	/* NOTREACHED */
	(void) pilotlocaldatabase_id;
}

PilotLocalDatabase::PilotLocalDatabase(const QString & dbName,
	QObject *p, const char *n) :
	PilotDatabase(p,n),
	fPathName(QString::null),
	fDBName(dbName),
	fAppInfo(0L), 
	fAppLen(0), 
	fNumRecords(0), 
	fCurrentRecord(0), 
	fPendingRec(-1)
{
	FUNCTIONSETUP;

	if (fPathBase && !fPathBase->isEmpty())
	{
		fPathName = *fPathBase;
	}
	else
	{
		fPathName = KGlobal::dirs()->saveLocation("data",
			CSL1("kpilot/DBBackup/"));
	}

	fixupDBName();
	openDatabase();
}


PilotLocalDatabase::~PilotLocalDatabase()
{
	FUNCTIONSETUP;
	int i;

	closeDatabase();
	delete[]fAppInfo;
	for (i = 0; i < fNumRecords; i++)
	{
		delete fRecords[i];
	}
}

// Changes any forward slashes to underscores
void PilotLocalDatabase::fixupDBName()
{
	FUNCTIONSETUP;
#if QT_VERSION < 0x30100
	fDBName = fDBName.replace(QRegExp(CSL1("/")),CSL1("_"));
#else
	// Actually, I don't know if this char-replace
	// is more efficient than the QString one.
	fDBName = fDBName.replace('/', CSL1("_"));
#endif
}

bool PilotLocalDatabase::createDatabase(long creator, long type, int, int flags, int version) 
{
	FUNCTIONSETUP;
	
	// if the database is already open, we cannot create it again. How about completely resetting it? (i.e. deleting it and the createing it again)
	if (isDBOpen()) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Database "<<fDBName<<" already open. Cannot recreate it."<<endl;
#endif
		return true;
	}

#ifdef DEBUG
	DEBUGCONDUIT<<"Creating database "<<fDBName<<endl;
#endif
	
	memcpy(&fDBInfo.name[0], fDBName, 34*sizeof(char));
	fDBInfo.creator=creator;
	fDBInfo.type=type;
	fDBInfo.more=0;
	fDBInfo.flags=flags;
	fDBInfo.miscFlags=0;
	fDBInfo.version=version;
	fDBInfo.modnum=0;
	fDBInfo.index=0;
	fDBInfo.createDate=(QDateTime::currentDateTime()).toTime_t();
	fDBInfo.modifyDate=(QDateTime::currentDateTime()).toTime_t();
	fDBInfo.backupDate=(QDateTime::currentDateTime()).toTime_t();
	
	delete[] fAppInfo;
	fAppInfo=0L;
	fAppLen=0;
	
	for (int i=0; i<fNumRecords; i++) {
		KPILOT_DELETE(fRecords[i]);
		fRecords[i]=NULL;
	}
	fNumRecords=0;
	fCurrentRecord=0;
	fPendingRec=0;
	
	// TODO: Do I have to open it explicitely???
	setDBOpen(true);
	return true;
}

int PilotLocalDatabase::deleteDatabase()
{
	FUNCTIONSETUP;
	if (isDBOpen()) closeDatabase();
	
	QString dbpath=dbPathName();
	QFile fl(dbpath);
	if (QFile::remove(dbPathName())) 
		return 0; 
	else 
		return -1;
}



// Reads the application block info
int PilotLocalDatabase::readAppBlock(unsigned char *buffer, int)
{
	FUNCTIONSETUP;

	if (!isDBOpen())
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return -1;
	}

	memcpy((void *) buffer, fAppInfo, fAppLen);
	return fAppLen;
}

int PilotLocalDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return -1;
	}
	delete[]fAppInfo;
	fAppLen = len;
	fAppInfo = new char[fAppLen];

	memcpy(fAppInfo, (void *) buffer, fAppLen);
	return 0;
}


	// returns the number of records in the database 
int PilotLocalDatabase::recordCount()
{
	return fNumRecords;
}


// Returns a QValueList of all record ids in the database. 
QValueList<recordid_t> PilotLocalDatabase::idList()
{
	int idlen=recordCount();
	QValueList<recordid_t> idlist;
	if (idlen<=0) return idlist;
	
	// now create the QValue list from the idarr:
	for (int id=0; id<idlen; id++) 
	{
		idlist.append(fRecords[id]->getID());
	}
	return idlist;
}

// Reads a record from database by id, returns record length
PilotRecord *PilotLocalDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUP;

	int i;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		DEBUGKPILOT << fDBName << ": DB not open!" << endl;
		return 0L;
	}
	for (i = 0; i < fNumRecords; i++)
	{
		if (fRecords[i]->getID() == id)
		{
			PilotRecord *newRecord = new PilotRecord(fRecords[i]);

			return newRecord;
		}
	}
	return 0L;
}

// Reads a record from database, returns the record length
PilotRecord *PilotLocalDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUP;
	fPendingRec = (-1);
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return 0L;
	}
	if (index >= fNumRecords)
		return 0L;
	PilotRecord *newRecord = new PilotRecord(fRecords[index]);

	return newRecord;
}

// Reads the next record from database in category 'category'
PilotRecord *PilotLocalDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return 0L;
	}
	while ((fCurrentRecord < fNumRecords)
		&& (fRecords[fCurrentRecord]->getCat() != category))
	{
		fCurrentRecord++;
	}
	if (fCurrentRecord == fNumRecords)
		return 0L;
	PilotRecord *newRecord = new PilotRecord(fRecords[fCurrentRecord]);

	fCurrentRecord++;	// so we skip it next time
	return newRecord;
}

// Reads the next record from database that has the dirty flag set.
PilotRecord *PilotLocalDatabase::readNextModifiedRec(int *ind)
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return 0L;
	}
	// Should this also check for deleted?
	while ((fCurrentRecord < fNumRecords)
		&& !(fRecords[fCurrentRecord]->getAttrib() & dlpRecAttrDirty))
	{
		fCurrentRecord++;
	}
	if (fCurrentRecord == fNumRecords)
		return 0L;
	PilotRecord *newRecord = new PilotRecord(fRecords[fCurrentRecord]);
	if (ind) *ind=fCurrentRecord;

	fPendingRec = fCurrentRecord;	// Record which one needs the new id
	fCurrentRecord++;	// so we skip it next time
	return newRecord;
}

// Writes a new ID to the record specified the index.  Not supported on Serial connections
recordid_t PilotLocalDatabase::writeID(PilotRecord * rec)
{
	FUNCTIONSETUP;

	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return 0;
	}
	if (fPendingRec == -1)
	{
		kdError() << k_funcinfo <<
			": Last call was _NOT_ readNextModifiedRec()" << endl;
		return 0;
	}
	fRecords[fPendingRec]->setID(rec->getID());
	fPendingRec = -1;
	return rec->getID();
}

// Writes a new record to database (if 'id' == 0, it is assumed that this is a new record to be installed on pilot)
recordid_t PilotLocalDatabase::writeRecord(PilotRecord * newRecord)
{
	FUNCTIONSETUP;
	int i;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return 0;
	}
	// We can't do this since it's possible the local apps need to rewrite a record
	// that also exists on the pilot, ie: it would already have a uid but incorrectly
	// get marked as clean.  So it's up to the app to mark them clean/dirty as appropriate.
//     if(id != 0)
//      {
//      // This must have come from the pilot, so turn off the modified flags
//      flags = flags & ~dlpRecAttrDirty;
//      }
//     else
//      flags = flags | dlpRecAttrDirty;

	// Instead of making the app do it, assume that whenever a record is
	// written to the database it is dirty.  (You can clean up the database with 
	// resetSyncFlags().)  This will make things get copied twice during a hot-sync
	// but shouldn't cause any other major headaches.
	newRecord->setAttrib(newRecord->getAttrib() | dlpRecAttrDirty);

	// First check to see if we have this record:
	if (newRecord->getID() != 0)
	{
		for (i = 0; i < fNumRecords; i++)
			if (fRecords[i]->getID() == newRecord->getID())
			{
				delete fRecords[i];

				fRecords[i] = new PilotRecord(newRecord);
				return 0;
			}
	}
	// Ok, we don't have it, so just tack it on.
	fRecords[fNumRecords++] = new PilotRecord(newRecord);
	return newRecord->getID();
}

// Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case
int PilotLocalDatabase::deleteRecord(recordid_t id, bool all) 
{
	FUNCTIONSETUP;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo <<": DB not open"<<endl;
		return -1;
	}
	
	if (all) 
	{
		for (int i=0; i<fNumRecords; i++) 
		{
			delete fRecords[i];
			fRecords[i]=0L;
		}
		fNumRecords=0;
		fCurrentRecord=0;
		fPendingRec=0;
		return 0;
	}
	else
	{
		int i=0;
		while ( (i<fNumRecords) && (fRecords[i]->getID()!=id) ) 
			i++;
		if (fRecords[i]->getID() == id)
		{
			delete fRecords[i];
			for (int j=i+1; j<fNumRecords; j++)
			{
				fRecords[j-1]=fRecords[j];
			}
			fNumRecords--;
		}
		else
		{
			// Record with this id does not exist!
			return -1;
		}
	}
	return 0;
}


// Resets all records in the database to not dirty.
int PilotLocalDatabase::resetSyncFlags()
{
	FUNCTIONSETUP;

	int i;

	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return -1;
	}
	for (i = 0; i < fNumRecords; i++)
		fRecords[i]->setAttrib(fRecords[i]->
			getAttrib() & ~dlpRecAttrDirty);
	return 0;
}

// Resets next record index to beginning
int PilotLocalDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return -1;
	}
	fCurrentRecord = 0;
	return 0;
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotLocalDatabase::cleanup()
{
	FUNCTIONSETUP;
	fPendingRec = -1;
	if (isDBOpen() == false)
	{
		kdError() << k_funcinfo << ": DB not open!" << endl;
		return -1;
	}
	int i, j;

	for (i = 0; (i < fNumRecords) && (fRecords[i]);)
		if (fRecords[i]->getAttrib() & dlpRecAttrDeleted)
		{
			delete fRecords[i];

			if ((i + 1) < fNumRecords)
				for (j = i + 1; j < fNumRecords; j++)
					fRecords[j - 1] = fRecords[j];
			else
				fRecords[i] = 0L;
			fNumRecords--;
		}
		else
			i++;

	// Don't have to do anything.  Will be taken care of by closeDatabase()...
	// Changed!
	return 0;
}

QString PilotLocalDatabase::dbPathName() const
{
	FUNCTIONSETUP;
	QString tempName(fPathName);
	QString slash = CSL1("/");
	
	if (!tempName.endsWith(slash)) tempName += slash;
	tempName += getDBName();
	tempName += CSL1(".pdb");
	return tempName;
}

void PilotLocalDatabase::openDatabase()
{
	FUNCTIONSETUP;

	void *tmpBuffer;
	pi_file *dbFile;
	int size, attr, cat;
	pi_uid_t id;

	QString tempName = dbPathName();
	QCString fileName = QFile::encodeName(tempName);
	dbFile = pi_file_open(const_cast < char *>((const char *) fileName));

	if (dbFile == 0L)
	{
		kdError() << k_funcinfo
			<< ": Failed to open " << tempName << endl;
		return;
	}
	pi_file_get_info(dbFile, &fDBInfo);
	pi_file_get_app_info(dbFile, &tmpBuffer, &fAppLen);
	fAppInfo = new char[fAppLen];

	memcpy(fAppInfo, tmpBuffer, fAppLen);
	while (pi_file_read_record(dbFile, fCurrentRecord,
			&tmpBuffer, &size, &attr, &cat, &id) == 0)
	{
		fRecords[fCurrentRecord] =
			new PilotRecord(tmpBuffer, size, attr, cat, id);
		fCurrentRecord++;
	}
	pi_file_close(dbFile);	// We done with it once we've read it in.
	fNumRecords = fCurrentRecord;
	fCurrentRecord = 0;
	setDBOpen(true);
}

void PilotLocalDatabase::closeDatabase()
{
	FUNCTIONSETUP;
	pi_file *dbFile;
	int i;

	if (isDBOpen() == false) {
#ifdef DEBUG
		DEBUGCONDUIT<<"Database "<<fDBName<<" is not open. Cannot close and write it"<<endl;
#endif
		return;
	}

	QString tempName_ = dbPathName();
	QString newName_ = tempName_ + CSL1(".bak");
	QCString tempName = QFile::encodeName(tempName_);
	QCString newName = QFile::encodeName(newName_);

	dbFile = pi_file_create(const_cast < char *>((const char *)newName),
		&fDBInfo);
#ifdef DEBUG
	DEBUGCONDUIT<<"Created temp file "<<newName<<" for the database file "<<dbPathName()<<endl;
#endif

	pi_file_set_app_info(dbFile, fAppInfo, fAppLen);
	for (i = 0; i < fNumRecords; i++)
	{
		pi_file_append_record(dbFile,
			fRecords[i]->getData(),
			fRecords[i]->getLen(),
			fRecords[i]->getAttrib(), fRecords[i]->getCat(),
			fRecords[i]->getID());
	}

	pi_file_close(dbFile);
	unlink((const char *) QFile::encodeName(tempName));
	rename((const char *) QFile::encodeName(newName),
		(const char *) QFile::encodeName(tempName));
	setDBOpen(false);
}


QString *PilotLocalDatabase::fPathBase = 0L;

void PilotLocalDatabase::setDBPath(const QString &s)
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGDAEMON << fname
		<< ": Setting default DB path to "
		<< s
		<< endl;
#endif

	if (!fPathBase)
	{
		fPathBase = new QString(s);
	}
	else
	{
		*fPathBase = s;
	}
}
