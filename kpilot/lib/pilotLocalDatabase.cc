/* KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
** Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#include "options.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>

#include <pi-file.h>

#include <qstring.h>
#include <qfile.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qvaluevector.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <ksavefile.h>

#include "pilotRecord.h"
#include "pilotLocalDatabase.h"

typedef QValueVector<PilotRecord *> Records;

class PilotLocalDatabase::Private : public Records
{
public:
	static const int DEFAULT_SIZE = 128;
	Private(int size=DEFAULT_SIZE) : Records(size) { resetIndex(); }
	~Private() { deleteRecords(); }

	void deleteRecords()
	{
		for (unsigned int i=0; i<size(); i++)
		{
			delete at(i);
		}
		clear();
		resetIndex();
	}

	void resetIndex()
	{
		current = 0;
		pending = -1;
	}

	unsigned int current;
	int pending;
} ;

PilotLocalDatabase::PilotLocalDatabase(const QString & path,
	const QString & dbName, bool useDefaultPath) :
	PilotDatabase(dbName),
	fPathName(path),
	fDBName(dbName),
	fAppInfo(0L),
	fAppLen(0),
	d(0L)
{
	FUNCTIONSETUP;
	fixupDBName();
	openDatabase();

	if (!isOpen() && useDefaultPath)
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
		if (!isOpen())
		{
			fPathName=path;
		}
	}

}

PilotLocalDatabase::PilotLocalDatabase(const QString &dbName) :
	PilotDatabase( QString() ),
	fPathName( QString() ),
	fDBName( QString() ),
	fAppInfo(0L),
	fAppLen(0),
	d(0L)
{
	FUNCTIONSETUP;

	int p = dbName.findRev( '/' );
	if (p<0)
	{
		// No slash
		fPathName = CSL1(".");
		fDBName = dbName;
	}
	else
	{
		fPathName = dbName.left(p);
		fDBName = dbName.mid(p+1);
	}
	openDatabase();
}

PilotLocalDatabase::~PilotLocalDatabase()
{
	FUNCTIONSETUP;

	closeDatabase();
	delete[]fAppInfo;
	delete d;
}

// Changes any forward slashes to underscores
void PilotLocalDatabase::fixupDBName()
{
	FUNCTIONSETUP;
	fDBName = fDBName.replace(CSL1("/"),CSL1("_"));
}

bool PilotLocalDatabase::createDatabase(long creator, long type, int, int flags, int version)
{
	FUNCTIONSETUP;

	// if the database is already open, we cannot create it again.
	// How about completely resetting it? (i.e. deleting it and then
	// creating it again)
	if (isOpen())
	{
		DEBUGKPILOT << fname << ": Database " << fDBName
			<< " already open. Cannot recreate it." << endl;
		return true;
	}

	DEBUGKPILOT << fname << ": Creating database " << fDBName << endl;

	// Database names seem to be latin1.
	Pilot::toPilot(fDBName, fDBInfo.name, sizeof(fDBInfo.name));
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

	d = new Private;

	// TODO: Do I have to open it explicitly???
	setDBOpen(true);
	return true;
}

int PilotLocalDatabase::deleteDatabase()
{
	FUNCTIONSETUP;
	if (isOpen())
	{
		closeDatabase();
	}

	QString dbpath=dbPathName();
	QFile fl(dbpath);
	if (QFile::remove(dbPathName()))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}



// Reads the application block info
int PilotLocalDatabase::readAppBlock(unsigned char *buffer, int size)
{
	FUNCTIONSETUP;

	size_t m = kMin((size_t)size,(size_t)fAppLen);

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		memset(buffer,0,m);
		return -1;
	}

	memcpy((void *) buffer, fAppInfo, m);
	return fAppLen;
}

int PilotLocalDatabase::writeAppBlock(unsigned char *buffer, int len)
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return -1;
	}
	delete[]fAppInfo;
	fAppLen = len;
	fAppInfo = new char[fAppLen];

	memcpy(fAppInfo, (void *) buffer, fAppLen);
	return 0;
}


// returns the number of records in the database
unsigned int PilotLocalDatabase::recordCount() const
{
	if (d && isOpen())
	{
		return d->size();
	}
	else
	{
		return 0;
	}
}


// Returns a QValueList of all record ids in the database.
QValueList<recordid_t> PilotLocalDatabase::idList()
{
	int idlen=recordCount();
	QValueList<recordid_t> idlist;
	if (idlen<=0)
	{
		return idlist;
	}

	// now create the QValue list from the idarr:
	for (int i=0; i<idlen; i++)
	{
		idlist.append((*d)[i]->id());
	}

	return idlist;
}

// Reads a record from database by id, returns record length
PilotRecord *PilotLocalDatabase::readRecordById(recordid_t id)
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "Database '" << fDBName << " not open!" << endl;
		return 0L;
	}

	d->pending = -1;

	for (unsigned int i = 0; i < d->size(); i++)
	{
		if ((*d)[i]->id() == id)
		{
			PilotRecord *newRecord = new PilotRecord((*d)[i]);
			d->current = i;
			return newRecord;
		}
	}
	return 0L;
}

// Reads a record from database, returns the record
PilotRecord *PilotLocalDatabase::readRecordByIndex(int index)
{
	FUNCTIONSETUP;

	if (index < 0)
	{
		DEBUGKPILOT << fname << ": Index " << index << " is bogus." << endl;
		return 0L;
	}

	d->pending = -1;
	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0L;
	}

	DEBUGKPILOT << fname << ": Index=" << index << " Count=" << recordCount() << endl;

	if ( (unsigned int)index >= recordCount() )
	{
		return 0L;
	}
	PilotRecord *newRecord = new PilotRecord((*d)[index]);
	d->current = index;

	return newRecord;
}

// Reads the next record from database in category 'category'
PilotRecord *PilotLocalDatabase::readNextRecInCategory(int category)
{
	FUNCTIONSETUP;
	d->pending  = -1;
	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0L;
	}

	while ((d->current < d->size())
		&& ((*d)[d->current]->category() != category))
	{
		d->current++;
	}

	if (d->current >= d->size())
		return 0L;
	PilotRecord *newRecord = new PilotRecord((*d)[d->current]);

	d->current++;	// so we skip it next time
	return newRecord;
}

const PilotRecord *PilotLocalDatabase::findNextNewRecord()
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0L;
	}
	DEBUGKPILOT << fname << ": looking for new record from " << d->current << endl;
	// Should this also check for deleted?
	while ((d->current < d->size())
		&& ((*d)[d->current]->id() != 0 ))
	{
		d->current++;
	}

	if (d->current >= d->size())
		return 0L;

	d->pending = d->current;	// Record which one needs the new id
	d->current++;	// so we skip it next time
	return (*d)[d->pending];
}

PilotRecord *PilotLocalDatabase::readNextModifiedRec(int *ind)
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0L;
	}

	d->pending = -1;
	// Should this also check for deleted?
	while ((d->current < d->size())
		&& !((*d)[d->current]->isModified())  && ((*d)[d->current]->id()>0 ))
	{
		d->current++;
	}

	if (d->current >= d->size())
	{
		return 0L;
	}
	PilotRecord *newRecord = new PilotRecord((*d)[d->current]);
	if (ind)
	{
		*ind=d->current;
	}

	d->pending = d->current;	// Record which one needs the new id
	d->current++;	// so we skip it next time
	return newRecord;
}

// Writes a new ID to the record specified the index.  Not supported on Serial connections
recordid_t PilotLocalDatabase::updateID(recordid_t id)
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0;
	}
	if (d->pending  < 0)
	{
		WARNINGKPILOT << "Last call was NOT readNextModifiedRec()" << endl;
		return 0;
	}
	(*d)[d->pending]->setID(id);
	d->pending = -1;
	return id;
}

// Writes a new record to database (if 'id' == 0, it is assumed that this is a new record to be installed on pilot)
recordid_t PilotLocalDatabase::writeRecord(PilotRecord * newRecord)
{
	FUNCTIONSETUP;

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return 0;
	}

	d->pending = -1;
	if (!newRecord)
	{
		WARNINGKPILOT << "Record to be written is invalid!" << endl;
		return 0;
	}

	// Instead of making the app do it, assume that whenever a record is
	// written to the database it is dirty.  (You can clean up the database with
	// resetSyncFlags().)  This will make things get copied twice during a hot-sync
	// but shouldn't cause any other major headaches.
	newRecord->setModified( true );

	// First check to see if we have this record:
  	if (newRecord->id() != 0)
  	{
 		for (unsigned int i = 0; i < d->size(); i++)
 			if ((*d)[i]->id() == newRecord->id())
  			{
 				delete (*d)[i];

 				(*d)[i] = new PilotRecord(newRecord);
  				return 0;
  			}
  	}
  	// Ok, we don't have it, so just tack it on.
 	d->append( new PilotRecord(newRecord) );
  	return newRecord->id();
}

// Deletes a record with the given recordid_t from the database, or all records, if all is set to true. The recordid_t will be ignored in this case
int PilotLocalDatabase::deleteRecord(recordid_t id, bool all)
{
	FUNCTIONSETUP;
	if (!isOpen())
	{
		WARNINGKPILOT <<"DB not open"<<endl;
		return -1;
	}
	d->resetIndex();
	if (all)
	{
		d->deleteRecords();
		d->clear();
		return 0;
	}
	else
	{
		Private::Iterator i;
		for ( i=d->begin() ; i!=d->end(); ++i)
		{
			if ((*i) && (*i)->id() == id) break;
		}
		if ( (i!=d->end()) && (*i) && (*i)->id() == id)
		{
			d->erase(i);
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

	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return -1;
	}
	d->pending = -1;
	for (unsigned int i = 0; i < d->size(); i++)
	{
		(*d)[i]->setModified( false );
	}
	return 0;
}

// Resets next record index to beginning
int PilotLocalDatabase::resetDBIndex()
{
	FUNCTIONSETUP;
	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return -1;
	}
	d->resetIndex();
	return 0;
}

// Purges all Archived/Deleted records from Palm Pilot database
int PilotLocalDatabase::cleanup()
{
	FUNCTIONSETUP;
	if (!isOpen())
	{
		WARNINGKPILOT << "DB not open!" << endl;
		return -1;
	}
	d->resetIndex();

	/* Not the for loop one might expect since when we erase()
	* a record the iterator changes too.
	*/
	Private::Iterator i = d->begin();
	while ( i!=d->end() )
	{
		if ( (*i)->isDeleted() || (*i)->isArchived() )
		{
			delete (*i);
			i = d->erase(i);
		}
		else
		{
			++i;
		}
	}

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

	pi_file *dbFile;

	setDBOpen(false);

	dbFile = pi_file_open( QFile::encodeName(dbPathName()) );
	if (dbFile == 0L)
	{
		QString path = dbPathName();
		DEBUGKPILOT << fname << ": Failed to open " << path << endl;
		return;
	}


	PI_SIZE_T size = 0;
	void *tmpBuffer;
	pi_file_get_info(dbFile, &fDBInfo);
	pi_file_get_app_info(dbFile, &tmpBuffer, &size);
	fAppLen = size;
	fAppInfo = new char[fAppLen];
	memcpy(fAppInfo, tmpBuffer, fAppLen);

	int count;
	pi_file_get_entries(dbFile, &count);
	if (count >= 0)
	{
		KPILOT_DELETE(d);
		d = new Private(count);
	}

	int attr, cat;
	recordid_t id;
	unsigned int i = 0;
	while (pi_file_read_record(dbFile, i,
			&tmpBuffer, &size, &attr, &cat, &id) == 0)
	{
		pi_buffer_t *b = pi_buffer_new(size);
		memcpy(b->data,tmpBuffer,size);
		b->used = size;
		(*d)[i] = new PilotRecord(b, attr, cat, id);
		i++;
	}
	pi_file_close(dbFile);	// We done with it once we've read it in.

	KSaveFile::backupFile( dbPathName() );

	setDBOpen(true);
}

void PilotLocalDatabase::closeDatabase()
{
	FUNCTIONSETUP;
	pi_file *dbFile;

	if (!isOpen())
	{
		DEBUGKPILOT << fname << ": Database " << fDBName
			<< " is not open. Cannot close and write it"
			<< endl;
		return;
	}

	QString newName = dbPathName() + CSL1(".new");
	QString path = dbPathName();
	DEBUGKPILOT << fname
		<< ": Creating temp file " << newName
		<< " for the database file " << path << endl;

	dbFile = pi_file_create(QFile::encodeName(newName),&fDBInfo);
	pi_file_set_app_info(dbFile, fAppInfo, fAppLen);

	for (unsigned int i = 0; i < d->size(); i++)
	{
		// How did a NULL pointer sneak in here?
		if (!(*d)[i])
		{
			continue;
		}

		if (((*d)[i]->id() == 0) && ((*d)[i]->isDeleted()))
		{
			// Just ignore it
		}
		else
		{
			pi_file_append_record(dbFile,
				(*d)[i]->data(),
				(*d)[i]->size(),
				(*d)[i]->attributes(), (*d)[i]->category(),
				(*d)[i]->id());
		}
	}

	pi_file_close(dbFile);
	QFile::remove(dbPathName());
	rename((const char *) QFile::encodeName(newName),
		(const char *) QFile::encodeName(dbPathName()));
	setDBOpen(false);
}


QString *PilotLocalDatabase::fPathBase = 0L;

void PilotLocalDatabase::setDBPath(const QString &s)
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname
		<< ": Setting default DB path to "
		<< s
		<< endl;

	if (!fPathBase)
	{
		fPathBase = new QString(s);
	}
	else
	{
		*fPathBase = s;
	}
}

/* virtual */ PilotDatabase::DBType PilotLocalDatabase::dbType() const
{
	return eLocalDB;
}


/* static */ bool PilotLocalDatabase::infoFromFile( const QString &path, DBInfo *d )
{
	FUNCTIONSETUP;

	pi_file *f = 0L;

	if (!d)
	{
		return false;
	}
	if (!QFile::exists(path))
	{
		return false;
	}

	QCString fileName = QFile::encodeName( path );
	f = pi_file_open( fileName );
	if (!f)
	{
		WARNINGKPILOT << "Can't open " << path << endl;
		return false;
	}

	pi_file_get_info(f,d);
	pi_file_close(f);

	return true;
}

