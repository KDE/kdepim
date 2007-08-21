/* memofile-conduit.cc			KPilot
**
** Copyright (C) 2004-2007 by Jason 'vanRijn' Kasper
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

#include "pilotRecord.h"
#include "pilotSerialDatabase.h"
#include "memofile-factory.h"
#include "memofile-conduit.h"
#include "memofileSettings.h"


/**
 * Our workhorse.  This is the main driver for the conduit.
 */
MemofileConduit::MemofileConduit(KPilotLink *d,
                                 const char *n,
                                 const QStringList &l) :
		ConduitAction(d,n,l),
		_DEFAULT_MEMODIR(QDir::homeDirPath() + CSL1("/MyMemos")),
		fMemoAppInfo(0L),
		_memofiles(0L)
{
	FUNCTIONSETUP;
	fConduitName=i18n("Memofile");
	fMemoList.setAutoDelete(true);
}

MemofileConduit::~MemofileConduit()
{
	FUNCTIONSETUP;
	KPILOT_DELETE(_memofiles);
}

/* virtual */ bool MemofileConduit::exec()
{
	FUNCTIONSETUP;

	setFirstSync( false );
	// try new format first...
	// DEBUGKPILOT << fname << ": trying new format database first." << endl;
	bool _open = false;
	/*
	_open = openDatabases(CSL1("MemosDB-PMem"));
	if(!_open) {
		DEBUGKPILOT << fname << ": unable to open new format database. trying old one." << endl;
	*/
		_open = openDatabases(CSL1("MemoDB"));
	/*
	} else {
		DEBUGKPILOT << fname << ": able to open new format database." << endl;
	}
	*/

	if(!_open) {
		emit logError(i18n("Unable to open the memo databases on the handheld."));
		DEBUGKPILOT << fname << ": unable to open new or old format database." << endl;
		return false;
	}

	readConfig();

	if (! initializeFromPilot()) {
		emit logError(i18n("Cannot initialize from pilot."));
		return false;
	}

	_memofiles = new Memofiles(fCategories, *fMemoAppInfo,
		_memo_directory, *fCtrHH);
	if (! _memofiles || ! _memofiles->isReady()) {
		emit logError(i18n("Cannot initialize the memo files from disk."));
		return false;
	}

	fCtrPC->setStartCount(_memofiles->count());

	setFirstSync( _memofiles->isFirstSync() );
	addSyncLogEntry(i18n(" Syncing with %1.").arg(_memo_directory));

	if ( (syncMode() == SyncAction::SyncMode::eCopyHHToPC) || _memofiles->isFirstSync() ) {
		addSyncLogEntry(i18n(" Copying Pilot to PC..."));
		DEBUGKPILOT << fname << ": copying Pilot to PC." << endl;
		copyHHToPC();
	} else if ( syncMode() == SyncAction::SyncMode::eCopyPCToHH ) {
		DEBUGKPILOT << fname << ": copying PC to Pilot." << endl;
		addSyncLogEntry(i18n(" Copying PC to Pilot..."));
		copyPCToHH();
	} else {
		DEBUGKPILOT << fname << ": doing regular sync." << endl;
		addSyncLogEntry(i18n(" Doing regular sync..."));
		sync();
	}

	cleanup();

	return delayDone();
}

bool MemofileConduit::readConfig()
{
	FUNCTIONSETUP;

	QString dir(MemofileConduitSettings::directory());
	if (dir.isEmpty()) {
		dir = _DEFAULT_MEMODIR;

		DEBUGKPILOT << fname
			<< ": no directory given to us.  defaulting to: ["
			<< _DEFAULT_MEMODIR
			<< "]" << endl;
	}

	_memo_directory = dir;
	_sync_private = MemofileConduitSettings::syncPrivate();


	DEBUGKPILOT << fname
		<< ": Settings... "
		<< "  directory: [" << _memo_directory
		<< "], first sync: [" << isFirstSync()
		<< "], sync private: [" << _sync_private
		<< "]" << endl;

	return true;

}

bool MemofileConduit::setAppInfo()
{
	FUNCTIONSETUP;

	// reset our category mapping from the filesystem
	MemoCategoryMap map = _memofiles->readCategoryMetadata();

	if (map.count() <=0) {
		DEBUGKPILOT << fname
			<< ": category metadata map is empty, nothing to do." << endl;
		return true;
	}

	fCategories = map;

	for (unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++)
	{
		if (fCategories.contains(i)) {
			fMemoAppInfo->setCategoryName(i,fCategories[i]);
		}
	}

	if (fDatabase)
	{
		fMemoAppInfo->writeTo(fDatabase);
	}
	if (fLocalDatabase)
	{
		fMemoAppInfo->writeTo(fLocalDatabase);
	}

	return true;
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

	for (unsigned int i = 0; i < Pilot::CATEGORY_COUNT; i++)
	{
		_category_name = fMemoAppInfo->categoryName(i);
		if (!_category_name.isEmpty())
		{
			_category_name = Memofiles::sanitizeName( _category_name );
			_category_id   = fMemoAppInfo->categoryInfo()->ID[i];
			_category_num  = i;
			fCategories[_category_num] = _category_name;

			DEBUGKPILOT << fname
				<< ": Category #"
				<< _category_num
				<< " has ID "
				<< _category_id
				<< " and name "
				<<_category_name << endl;
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

	DEBUGKPILOT << fname
		<< ": Database has " << fDatabase->recordCount()
		<< " records." << endl;

	fMemoList.clear();

	int currentRecord = 0;
	PilotRecord *pilotRec;
	PilotMemo *memo = 0;

	while ((pilotRec = fDatabase->readRecordByIndex(currentRecord)) != NULL) {
		if ((!pilotRec->isSecret()) || _sync_private) {
			memo = new PilotMemo(pilotRec);
			fMemoList.append(memo);

			DEBUGKPILOT << fname
				<< ": Added memo: ["
				<< currentRecord
				<< "], id: ["
				<< memo->id()
				<< "], category: ["
				<< fCategories[memo->category()]
				<< "], title: ["
				<< memo->getTitle()
				<< "]" << endl;
		} else {
			DEBUGKPILOT << fname
				<< ": Skipped secret record: ["
				<< currentRecord
				<< "], title: ["
				<< memo->getTitle()
				<< "]" << endl;
		}

		KPILOT_DELETE(pilotRec);

		currentRecord++;
	}

	DEBUGKPILOT << fname
		<< ": read: [" << fMemoList.count()
		<< "] records from palm." << endl;
}

/**
 *  Read all modified memos in from Pilot.
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

			DEBUGKPILOT << fname
				<< ": modified memo id: ["
				<< memo->id()
				<< "], title: ["
				<< memo->getTitle()
				<< "]" << endl;
		} else {
			DEBUGKPILOT << fname
				<< ": skipped secret modified record id: ["
				<< memo->id()
				<< "], title: ["
				<< memo->getTitle()
				<< "]" << endl;
		}

		KPILOT_DELETE(pilotRec);

		currentRecord++;
	}

	DEBUGKPILOT << fname
		<< ": read: [" << fMemoList.count()
		<< "] modified records from palm." << endl;
}


/* slot */ void MemofileConduit::process()
{
	FUNCTIONSETUP;

	DEBUGKPILOT << fname << ": Now in state " << fActionStatus << endl;
}


void MemofileConduit::listPilotMemos()
{
	FUNCTIONSETUP;

	PilotMemo *memo;
	for ( memo = fMemoList.first(); memo; memo = fMemoList.next() ) {
		QString _category_name = fCategories[memo->category()];

		DEBUGKPILOT << fConduitName
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
	KPILOT_DELETE(_memofiles);
	_memofiles = new Memofiles(fCategories, *fMemoAppInfo,
		_memo_directory, *fCtrHH);

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
		Pilot::RecordIDList ids=fDatabase->idList();
		Pilot::RecordIDList::iterator it;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if (!_memofiles->find(*it))
			{
				DEBUGKPILOT << fname
					<< "Deleting record with ID "<< *it <<" from handheld "
					<< "(is not on PC, and syncing with PC->HH direction)"
					<< endl;
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
		DEBUGKPILOT << fname
			<< ": ERROR: [" << memofile->toString()
			<< "] could not be written to the pilot."
			<< endl;
		return -1;
	}

	int newid = fDatabase->writeRecord(r);
	fLocalDatabase->writeRecord(r);

	KPILOT_DELETE(r);

	memofile->setID(newid);

	QString status;
	if (oldid <=0) {
		fCtrHH->created();
		status = "new to pilot";
	} else {
		fCtrHH->updated();
		status = "updated";
	}

	DEBUGKPILOT << fname
		<< ": memofile: [" << memofile->toString()
		<< "] written to the pilot, [" << status << "]."
		<< endl;

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
	KPILOT_DELETE(r);

	fCtrHH->deleted();

	DEBUGKPILOT << fname
		<< ": memo: [" << memo->getTitle()
		<< "] deleted from the pilot."
		<< endl;
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

void MemofileConduit::cleanup()
{
	FUNCTIONSETUP;

	fDatabase->resetSyncFlags();
	fDatabase->cleanup();
	fLocalDatabase->resetSyncFlags();
	fLocalDatabase->cleanup();

	fCtrPC->setEndCount(_memofiles->count());
}


#include "memofile-conduit.moc"

