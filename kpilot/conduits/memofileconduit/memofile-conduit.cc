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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
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
		_DEFAULT_MEMODIR("~/MyMemos/")
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

	fFirstSync = false;
	if(!openDatabases(QString::fromLatin1("MemoDB"))) {
		emit logError(i18n("Unable to open the memo databases on the handheld."));
		return false;
	}

	readConfig();

	if (! initializeFromPilot()) {
		emit logError(i18n("Cannot initialize from pilot."));
		return false;
	}

	_memofiles = new Memofiles(fCategories, fMemoAppInfo, _memo_directory);

	fFirstSync = _memofiles->isFirstSync();

	addSyncLogEntry(" Syncing with " + _memo_directory + ".");

	if (SyncAction::eCopyHHToPC == getSyncDirection() || fFirstSync) {
		addSyncLogEntry(" Copying Pilot to PC...");
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": copying Pilot to PC." << endl;
#endif
		copyHHToPC();
	} else if (SyncAction::eCopyPCToHH == getSyncDirection()) {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": copying PC to Pilot." << endl;
#endif
		addSyncLogEntry(" Copying PC to Pilot...");
		copyPCToHH();
	} else {
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": doing regular sync." << endl;
#endif
		addSyncLogEntry(" Doing regular sync...");
		sync();
	}

	cleanup();

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": stats: " << getResults() << endl;
#endif
	addSyncLogEntry(" " + getResults());

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

void MemofileConduit::getAppInfo()
{
	FUNCTIONSETUP;


	unsigned char buffer[PilotDatabase::MAX_APPINFO_SIZE];
	int appInfoSize = fDatabase->readAppBlock(buffer,PilotDatabase::MAX_APPINFO_SIZE);

	if (appInfoSize<0) {
		fActionStatus=Error;
		return;
	}

	unpack_MemoAppInfo(&fMemoAppInfo,buffer,appInfoSize);
	PilotDatabase::listAppInfo(&fMemoAppInfo.category);

}


/**
 * Methods related to getting set up from the Pilot.
 */

bool MemofileConduit::initializeFromPilot()
{

	_countDeletedToPilot = 0;
	_countModifiedToPilot = 0;
	_countNewToPilot = 0;

	getAppInfo();

	if (!loadPilotCategories())	return false;

	return true;
}

bool MemofileConduit::loadPilotCategories()
{
	FUNCTIONSETUP;

	fCategories.clear();

	QString _category_name;
	int _category_id=0;
	int _category_num=0;

	for (int i = 0; i < 15; i++) {
		if (fMemoAppInfo.category.name[i][0]) {
			_category_name = PilotAppCategory::codec()->toUnicode(fMemoAppInfo.category.name[i]);
			_category_id   = (int)fMemoAppInfo.category.ID[i];
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
			<< memo->getID()
			<< "], category: ["
			<< fCategories[memo->getCat()]
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
		// databse, so take care of the local database here
		if (memo->isDeleted()) {
			fLocalDatabase->deleteRecord(memo->getID());
		} else {
			fLocalDatabase->writeRecord(pilotRec);
		}

		if ((!pilotRec->isSecret()) || _sync_private) {
			fMemoList.append(memo);

#ifdef DEBUG
			DEBUGCONDUIT << fname
			<< ": modified memo id: ["
			<< memo->getID()
			<< "], title: ["
			<< memo->getTitle()
			<< "]" << endl;
#endif

		} else {
#ifdef DEBUG
			DEBUGCONDUIT << fname <<
			": skipped secret modified record id: ["
			<< memo->getID()
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
		QString _category_name = fCategories[memo->getCat()];
#ifdef DEBUG
		DEBUGCONDUIT << fname
		<< ": listing record id: [" << memo->id()
#ifdef DEBUG
		<< "] category id: [" << memo->getCat()
		<< "] category name: [" << _category_name
		<< "] title: [" << memo->getTitle()
		<< "]" << endl;
#endif
	}
#endif

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

	_memofiles->load(true);

	QPtrList<Memofile> memofiles = _memofiles->getAll();

	Memofile * memofile;

	for ( memofile = memofiles.first(); memofile; memofile = memofiles.next() ) {
		writeToPilot(memofile);
	}

	_memofiles->save();

	return true;

}

int MemofileConduit::writeToPilot(Memofile * memofile)
{
	FUNCTIONSETUP;

	int oldid = memofile->id();

	PilotRecord *r = memofile->pack();

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

void MemofileConduit::deleteFromPilot(Memofile * memofile)
{
	FUNCTIONSETUP;

	fDatabase->deleteRecord(memofile->getID());
	fLocalDatabase->deleteRecord(memofile->getID());

	_countDeletedToPilot++;

#ifdef DEBUG
	DEBUGCONDUIT << fname
	<< ": memofile: [" << memofile->toString()
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
	QString result = "";

	if (_countNewToPilot > 0)
		result += QString::number(_countNewToPilot) + " new to Palm. ";

	if (_countModifiedToPilot > 0)
		result += QString::number(_countModifiedToPilot) + " changed to Palm. ";

	if (_countDeletedToPilot > 0)
		result += QString::number(_countDeletedToPilot) + " deleted from Palm. ";

	result += _memofiles->getResults();

	if (result.length() <= 0)
		result = " no changes made.";

	return result;
}

void MemofileConduit::cleanup()
{
	FUNCTIONSETUP;

	fDatabase->cleanup();
	fDatabase->resetSyncFlags();
	fLocalDatabase->cleanup();
	fLocalDatabase->resetSyncFlags();
}
