/* memofile-conduit.cc			KPilot
**
** Copyright (C) 2004-2004 by Jason 'vanRijn' Kasper
**
** This file does the actual conduit work.
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

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *memofile_conduit_id=
    "$Id$";

// Only include what we really need:
// First UNIX system stuff, then std C++,
// then Qt, then KDE, then local includes.
//
//

#include <time.h>  // required by pilot-link includes

#include <pi-memo.h>

#include "pilotMemo.h"

#include <qfile.h>
#include <qdir.h>
#include <qtextcodec.h>

#include <kconfig.h>
#include <kdebug.h>

#include "pilotAppCategory.h"
#include "pilotSerialDatabase.h"
#include "memofile-factory.h"
#include "memofile-conduit.h"
#include "memofileSettings.h"


/**
 * Our workhorse.  This is the main driver for the conduit.
 */
MemofileConduit::MemofileConduit(KPilotDeviceLink *d,
                                 const char *n,
                                 const QStringList &l) :
		ConduitAction(d,n,l),
		_DEFAULT_MEMODIR(QDir::homePath() + CSL1("/MyMemos")),
		fMemoAppInfo(0L),
		_memofiles(0L)
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT<<memofile_conduit_id<<endl;
#endif
	fConduitName=i18n("Memofile");
	fMemoList.setAutoDelete(true);
}

MemofileConduit::~MemofileConduit()
{
	FUNCTIONSETUP;
	// we're not guaranteed to have this
// 	if (_memofiles) delete _memofiles;
}

/* virtual */ bool MemofileConduit::exec()
{
	FUNCTIONSETUP;

	setFirstSync( false );
	if(!openDatabases(CSL1("MemoDB"))) {
		emit logError(i18n("Unable to open the memo databases on the handheld."));
		return false;
	}

	readConfig();

	if (! initializeFromPilot()) {
		emit logError(i18n("Cannot initialize from pilot."));
		return false;
	}

	_memofiles = new Memofiles(fCategories, *fMemoAppInfo, _memo_directory);
	if (! _memofiles || ! _memofiles->isReady()) {
		emit logError(TODO_I18N("Cannot initialize from filesystem."));
		return false;
	}

	setFirstSync( _memofiles->isFirstSync() );
	addSyncLogEntry(i18n(" Syncing with %1.", _memo_directory));

	if ( (syncMode() == SyncAction::SyncMode::eCopyHHToPC) || _memofiles->isFirstSync() ) {
		addSyncLogEntry(i18n(" Copying Pilot to PC..."));
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": copying Pilot to PC." << endl;
#endif
		copyHHToPC();
	} else if ( syncMode() == SyncAction::SyncMode::eCopyPCToHH ) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": copying PC to Pilot." << endl;
#endif
		addSyncLogEntry(i18n(" Copying PC to Pilot..."));
		copyPCToHH();
	} else {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": doing regular sync." << endl;
#endif
		addSyncLogEntry(i18n(" Doing regular sync..."));
		sync();
	}

	cleanup();

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": stats: " << getResults() << endl;
#endif
	addSyncLogEntry(getResults());

	return delayDone();
}

bool MemofileConduit::readConfig()
{
	FUNCTIONSETUP;

	QString dir(MemofileConduitSettings::directory());
	if (dir.isEmpty()) {
		dir = _DEFAULT_MEMODIR;

#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": no directory given to us.  defaulting to: ["
		<< _DEFAULT_MEMODIR
		<< "]" << endl;
#endif

	}

	_memo_directory = dir;
	_sync_private = MemofileConduitSettings::syncPrivate();


#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": Settings... "
	<< "  directory: [" << _memo_directory
	<< "], first sync: [" << isFirstSync()
	<< "], sync private: [" << _sync_private
	<< "]" << endl;
#endif

	return true;

}

bool MemofileConduit::setAppInfo()
{
	FUNCTIONSETUP;

	// reset our category mapping from the filesystem
	MemoCategoryMap map = _memofiles->readCategoryMetadata();

	if (map.count() <=0) {
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": category metadata map is empty, nothing to do." << endl;
#endif
		return true;
	}

	fCategories = map;

	for (int i = 0; i < PILOT_CATEGORY_MAX; i++)
	{
		if (fCategories.contains(i)) {
			QString name = fCategories[i].left(16);
			fMemoAppInfo->setCategoryName(i,name);

#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": setting category: [" << i
			<< "] to name: ["
			<< name << "]" << endl;
#endif
		}
	}

	int appLen = 0;
	unsigned char *buffer = doPackAppInfo( &appLen );
	if ( buffer )
	{	if (fDatabase)
		fDatabase->writeAppBlock( buffer, appLen );
		if (fLocalDatabase)
			fLocalDatabase->writeAppBlock( buffer, appLen );
		delete[] buffer;
	}

	return true;
}

unsigned char *MemofileConduit::doPackAppInfo( int *appLen )
{
	int appLength = pack_MemoAppInfo(fMemoAppInfo->info(), 0, 0);
	unsigned char *buffer = new unsigned char[appLength];
	pack_MemoAppInfo(fMemoAppInfo->info(), buffer, appLength);
	if ( appLen ) *appLen = appLength;
	return buffer;
}


bool MemofileConduit::getAppInfo()
{
	FUNCTIONSETUP;

	KPILOT_DELETE(fMemoAppInfo);
	fMemoAppInfo = new PilotMemoInfo(fDatabase);
	fMemoAppInfo->dump();
	return true;
}


/**
 * Methods related to getting set up from the Pilot.
 */

bool MemofileConduit::initializeFromPilot()
{

	_countDeletedToPilot = 0;
	_countModifiedToPilot = 0;
	_countNewToPilot = 0;

	if (!getAppInfo()) return false;

	if (!loadPilotCategories()) return false;

	return true;
}

bool MemofileConduit::loadPilotCategories()
{
	FUNCTIONSETUP;

	fCategories.clear();

	QString _category_name;
	int _category_id=0;
	int _category_num=0;

	for (int i = 0; i < PILOT_CATEGORY_MAX; i++)
	{
		_category_name = fMemoAppInfo->category(i);
		if (!_category_name.isEmpty())
		{
			_category_name = Memofiles::sanitizeName( _category_name );
			_category_id   = fMemoAppInfo->categoryInfo()->ID[i];
			_category_num  = i;
			fCategories[_category_num] = _category_name;
#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": Category #"
			<< _category_num
			<< " has ID "
			<< _category_id
			<< " and name "
			<<_category_name << endl;
#endif

		}
	}
	return true;
}

/**
 *  Read all memos in from Pilot.
 */
void MemofileConduit::getAllFromPilot()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": Database has " << fDatabase->recordCount()
	<< " records." << endl;
#endif

	fMemoList.clear();

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotMemo *memo = 0;

	while ((pilotRec = fDatabase->readRecordByIndex(currentRecord)) != NULL) {
		if ((!pilotRec->isSecret()) || _sync_private) {
			memo = new PilotMemo(pilotRec);
			fMemoList.append(memo);

#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": Added memo: ["
			<< currentRecord
			<< "], id: ["
			<< memo->id()
			<< "], category: ["
			<< fCategories[memo->category()]
			<< "], title: ["
			<< memo->getTitle()
			<< "]" << endl;
#endif

		} else {
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
			": Skipped secret record: ["
			<< currentRecord
			<< "], title: ["
			<< memo->getTitle()
			<< "]" << endl;

#endif

		}

		delete pilotRec;

		currentRecord++;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname <<
	": read: [" << fMemoList.count()
	<< "] records from palm." << endl;
#endif

}

/**
 *  Read all memos in from Pilot.
 */
void MemofileConduit::getModifiedFromPilot()
{
	FUNCTIONSETUP;

	fMemoList.clear();

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotMemo *memo = 0;

	while ((pilotRec = fDatabase->readNextModifiedRec()) != NULL) {
		memo = new PilotMemo(pilotRec);
		// we are syncing to both our filesystem and to the local
		// database, so take care of the local database here
		if (memo->isDeleted()) {
			fLocalDatabase->deleteRecord(memo->id());
		} else {
			fLocalDatabase->writeRecord(pilotRec);
		}

		if ((!pilotRec->isSecret()) || _sync_private) {
			fMemoList.append(memo);

#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": modified memo id: ["
			<< memo->id()
			<< "], title: ["
			<< memo->getTitle()
			<< "]" << endl;
#endif

		} else {
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
			": skipped secret modified record id: ["
			<< memo->id()
			<< "], title: ["
			<< memo->getTitle()
			<< "]" << endl;

#endif

		}

		delete pilotRec;

		currentRecord++;
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname <<
	": read: [" << fMemoList.count()
	<< "] modified records from palm." << endl;
#endif

}


/* slot */ void MemofileConduit::process()
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": Now in state " << fActionStatus << endl;
#endif
}


void MemofileConduit::listPilotMemos()
{
	FUNCTIONSETUP;

	PilotMemo *memo;
	for ( memo = fMemoList.first(); memo; memo = fMemoList.next() ) {
		QString _category_name = fCategories[memo->category()];

		DEBUGCONDUIT << fConduitName
		<< ": listing record id: [" << memo->id()
		<< "] category id: [" << memo->category()
		<< "] category name: [" << _category_name
		<< "] title: [" << memo->getTitle()
		<< "]" << endl;
	}

}

bool MemofileConduit::copyHHToPC()
{
	FUNCTIONSETUP;

	getAllFromPilot();

	_memofiles->eraseLocalMemos();

	_memofiles->setPilotMemos(fMemoList);

	_memofiles->save();

	return true;

}

bool MemofileConduit::copyPCToHH()
{
	FUNCTIONSETUP;

	// set category info from the filesystem, if we can.
	// Note: This will reset both fCategories and fMemoAppInfo, so
	//       after this, we need to reinitialize our memofiles object...
	setAppInfo();

	// re-create our memofiles helper...
	delete _memofiles;
	_memofiles = new Memofiles(fCategories, *fMemoAppInfo, _memo_directory);

	_memofiles->load(true);

	QPtrList<Memofile> memofiles = _memofiles->getAll();

	Memofile * memofile;

	for ( memofile = memofiles.first(); memofile; memofile = memofiles.next() ) {
		writeToPilot(memofile);
	}

	_memofiles->save();

	// now that we've copied from the PC to our handheld, remove anything extra from the
	// handheld...
	deleteUnsyncedHHRecords();

	return true;

}

void MemofileConduit::deleteUnsyncedHHRecords()
{
	FUNCTIONSETUP;
	if ( syncMode()==SyncMode::eCopyPCToHH )
	{
		RecordIDList ids=fDatabase->idList();
		RecordIDList::iterator it;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if (!_memofiles->find(*it))
			{
#ifdef DEBUG
				DEBUGCONDUIT << fname
				<< "Deleting record with ID "<<*it <<" from handheld "
				<< "(is not on PC, and syncing with PC->HH direction)"
				<< endl;
#endif
				fDatabase->deleteRecord(*it);
				fLocalDatabase->deleteRecord(*it);
			}
		}
	}
}

int MemofileConduit::writeToPilot(Memofile * memofile)
{
	FUNCTIONSETUP;

	int oldid = memofile->id();

	PilotRecord *r = memofile->pack();

	if (!r) {
#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": ERROR: [" << memofile->toString()
	<< "] could not be written to the pilot."
	<< endl;
#endif

		return -1;
	}

	int newid = fDatabase->writeRecord(r);
	fLocalDatabase->writeRecord(r);

	delete r;

	memofile->setID(newid);

	QString status;
	if (oldid <=0) {
		_countNewToPilot++;
		status = "new to pilot";
	} else {
		_countModifiedToPilot++;
		status = "updated";
	}

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": memofile: [" << memofile->toString()
	<< "] written to the pilot, [" << status << "]."
	<< endl;
#endif

	return newid;

}

void MemofileConduit::deleteFromPilot(PilotMemo * memo)
{
	FUNCTIONSETUP;

	PilotRecord *r = memo->pack();
	if (r) {
		r->setDeleted(true);
		fDatabase->writeRecord(r);
		fLocalDatabase->writeRecord(r);
	}
	delete r;

	_countDeletedToPilot++;

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": memo: [" << memo->getTitle()
	<< "] deleted from the pilot."
	<< endl;
#endif

}

bool MemofileConduit::sync()
{
	FUNCTIONSETUP;

	_memofiles->load(false);

	getModifiedFromPilot();

	PilotMemo *memo;
	for ( memo = fMemoList.first(); memo; memo = fMemoList.next() ) {
		_memofiles->addModifiedMemo(memo);
	}

	QPtrList<Memofile> memofiles = _memofiles->getModified();

	Memofile *memofile;
	for ( memofile = memofiles.first(); memofile; memofile = memofiles.next() ) {
		if (memofile->isDeleted()) {
			deleteFromPilot(memofile);
		} else {
			writeToPilot(memofile);
		}
	}

	_memofiles->save();

	return true;
}

QString MemofileConduit::getResults()
{
	QString result;

	if (_countNewToPilot > 0)
		result += i18n("%1 new to Palm. ", _countNewToPilot);

	if (_countModifiedToPilot > 0)
		result += i18n("%1 changed to Palm. ", _countModifiedToPilot);

	if (_countDeletedToPilot > 0)
		result += i18n("%1 deleted from Palm. ", _countDeletedToPilot);

	result += _memofiles->getResults();

	if (result.length() <= 0)
		result = i18n(" no changes made.");

	return result;
}

void MemofileConduit::cleanup()
{
	FUNCTIONSETUP;

	fDatabase->resetSyncFlags();
	fDatabase->cleanup();
	fLocalDatabase->resetSyncFlags();
	fLocalDatabase->cleanup();
}


#include "memofile-conduit.moc"

