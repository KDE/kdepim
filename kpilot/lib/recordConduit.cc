/* recordConduit.cc                           KPilot
**
** Copyright (C) 2004 by Reinhold Kainhofer
** Based on the addressbook conduit:
** Copyright (C) 2000,2001 by Dan Pilone
** Copyright (C) 2000 Gregory Stern
** Copyright (C) 2002-2003 by Reinhold Kainhofer
**
** This conduit is the base class for all record-based conduits.
** all the sync logic is included in this class, and all child classes
** just have to implement some specific copying and conflict resolution
** methods.
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org.
*/



#include "options.h"

#include <qtimer.h>
#include <qtextcodec.h>
#include <qfile.h>

#include "pilotAppCategory.h"
#include "pilotSerialDatabase.h"
#include "pilotLocalDatabase.h"
#include "recordConduit.h"

#include "recordConduit.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
extern "C"
{
long version_record_conduit = KPILOT_PLUGIN_API;
const char *id_record_conduit="$Id$";
}



/** make that entry on the pc archived (i.e. deleted on the handheld, 
 *  while present on the pc, but not synced to the handheld */
bool RecordConduit::PCData::makeArchived( RecordConduit::PCEntry *pcEntry ) 
{
	if ( pcEntry ) {
		pcEntry->makeArchived();
		setChanged( true );
		return true;
	} else return false;
}


/* Builds the map which links record ids to uid's of PCEntry. This is the slow implementation,
 * that should always work. subclasses should reimplement it to speed things up.
*/
bool RecordConduit::PCData::mapContactsToPilot( QMap<recordid_t,QString> &idContactMap )
{
	FUNCTIONSETUP;

	idContactMap.clear();

	Iterator it = begin();
	PCEntry *ent;
	while ( !atEnd( it ) ) {
		ent = *it;
		recordid_t id( ent->recid() );
		if ( id != 0 ) {
			idContactMap.insert( id, ent->uid() );
		}
		++it;
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Loaded " << idContactMap.size() <<
	    " Entries on the pc and mapped them to records on the handheld. " << endl;
#endif
	return true;
}



/*********************************************************************
                        C O N S T R U C T O R
 *********************************************************************/



bool RecordConduit::mArchiveDeleted = false;

RecordConduit::RecordConduit(QString name, KPilotDeviceLink * o, const char *n, const QStringList & a):
		ConduitAction(o, n, a),
		mPCData(0), mPalmIndex(0),
		mEntryMap(), mSyncedIds(), mAllIds()
{
	FUNCTIONSETUP;
#ifdef DEBUG
	DEBUGCONDUIT << id_record_conduit << endl;
#endif
	fConduitName = name;
}



RecordConduit::~RecordConduit()
{
	if ( mPCData ) KPILOT_DELETE(mPCData);
}






/*********************************************************************
                S Y N C   S T R U C T U R E
 *********************************************************************/



/* virtual */ bool RecordConduit::exec()
{
	FUNCTIONSETUP;
	DEBUGCONDUIT << id_record_conduit << endl;

	if ( !_prepare() ) return false;

	fFirstSync = false;
	// Database names probably in latin1.
	if( !openDatabases( dbName(), &fFirstSync ) )
	{
		emit logError(i18n("Unable to open the %1 database on the handheld.").arg( dbName() ) );
		return false;
	}
	_getAppInfo();
	if( !mPCData->loadData() )
	{
		emit logError( i18n("Unable to open %1.").arg( mPCData->description() ) );
		return false;
	}
	// get the addresseMap which maps Pilot unique record(address) id's to
	// a Abbrowser Addressee; allows for easy lookup and comparisons
	if ( mPCData->isEmpty() )
		fFirstSync = true;
	else
		mPCData->mapContactsToPilot( mEntryMap );
	fFirstSync = fFirstSync || ( mPCData->isEmpty() );

	// perform syncing from palm to abbrowser
	// iterate through all records in palm pilot
	mPalmIndex = 0;

#ifdef DEBUG
	DEBUGCONDUIT << fname << ": fullsync=" << isFullSync() << ", firstSync=" << isFirstSync() << endl;
	DEBUGCONDUIT << fname << ": "
		<< "syncDirection=" << getSyncDirection() << ", "
//		<< "archive = " << AbbrowserSettings::archiveDeleted() 
		<< endl;
	DEBUGCONDUIT << fname << ": conflictRes="<< getConflictResolution() << endl;
//	DEBUGCONDUIT << fname << ": PilotStreetHome=" << AbbrowserSettings::pilotStreet() << ", PilotFaxHOme" << AbbrowserSettings::pilotFax() << endl;
#endif

	if ( !isFirstSync() )
		mAllIds=fDatabase->idList();

	/* Note:
	   if eCopyPCToHH or eCopyHHToPC, first sync everything, then lookup
	   those entries on the receiving side that are not yet syncced and delete
	   them. Use slotDeleteUnsyncedPCRecords and slotDeleteUnsyncedHHRecords
	   for this, and no longer purge the whole addressbook before the sync to
	   prevent data loss in case of connection loss. */

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));

	return true;
}



void RecordConduit::slotPalmRecToPC()
{
	FUNCTIONSETUP;
	PilotRecord *palmRec = 0L, *backupRec = 0L;

	if ( getSyncDirection() == SyncAction::eCopyPCToHH )
	{
		mPCIter = mPCData->begin();
		QTimer::singleShot(0, this, SLOT(slotPCRecToPalm()));
		return;
	}

	if ( isFullSync() )
		palmRec = fDatabase->readRecordByIndex( mPalmIndex++ );
	else
		palmRec = dynamic_cast <PilotSerialDatabase * >(fDatabase)->readNextModifiedRec();

	if ( !palmRec )
	{
		mPCIter = mPCData->begin();
		QTimer::singleShot( 0, this, SLOT( slotPCRecToPalm() ) );
		return;
	}

	// already synced, so skip:
	if ( mSyncedIds.contains( palmRec->id() ) )
	{
		KPILOT_DELETE( palmRec );
		QTimer::singleShot( 0, this, SLOT( slotPalmRecToPC() ) );
		return;
	}

	backupRec = fLocalDatabase->readRecordById( palmRec->id() );
	PilotRecord *compareRec = backupRec ? backupRec : palmRec;
	PilotAppCategory *compareEntry = createPalmEntry( compareRec );
	PCEntry *pcEntry = findMatch( compareEntry );
	KPILOT_DELETE( compareEntry );

	PilotAppCategory *backupEntry=0L;
	if ( backupRec ) 
		backupEntry = createPalmEntry( backupRec );
	PilotAppCategory *palmEntry=0L;
	if ( palmRec ) 
		palmEntry = createPalmEntry( palmRec );

	syncEntry( pcEntry, backupEntry, palmEntry );

	mSyncedIds.append( palmRec->id() );

	KPILOT_DELETE( pcEntry );
	KPILOT_DELETE( palmEntry );
	KPILOT_DELETE( backupEntry );
	KPILOT_DELETE( palmRec );
	KPILOT_DELETE( backupRec );

	QTimer::singleShot(0, this, SLOT(slotPalmRecToPC()));
}



void RecordConduit::slotPCRecToPalm()
{
	FUNCTIONSETUP;

	if ( ( getSyncDirection()==SyncAction::eCopyHHToPC ) ||
		mPCData->atEnd( mPCIter ) )
	{
		mPalmIndex = 0;
		QTimer::singleShot( 0, this, SLOT( slotDeletedRecord() ) );
		return;
	}

	PilotRecord *backupRec=0L;
	PCEntry *pcEntry = *mPCIter;
	++mPCIter;

	// If marked as archived, don't sync!
	if ( isArchived( pcEntry ) )
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": address with id " << pcEntry->uid() <<
			" marked archived, so don't sync." << endl;
#endif
		KPILOT_DELETE( pcEntry );
		QTimer::singleShot( 0, this, SLOT( slotPCRecToPalm() ) );
		return;
	}

	recordid_t recID( pcEntry->recid() );
	if ( recID == 0 )
	{
		// it's a new item(no record ID and not inserted by the Palm -> PC sync), so add it
		syncEntry( pcEntry, 0L, 0L );
		KPILOT_DELETE( pcEntry );
		QTimer::singleShot( 0, this, SLOT( slotPCRecToPalm() ) );
		return;
	}

	// look into the list of already synced record ids to see if the PCEntry hasn't already been synced
	if ( mSyncedIds.contains( recID ) )
	{
#ifdef DEBUG
		DEBUGCONDUIT << ": address with id " << recID << " already synced." << endl;
#endif
		KPILOT_DELETE( pcEntry );
		QTimer::singleShot( 0, this, SLOT( slotPCRecToPalm() ) );
		return;
	}


	backupRec = fLocalDatabase->readRecordById( recID );
	// only update if no backup record or the backup record is not equal to the PCEntry

	PilotAppCategory*backupEntry=0L;
	if ( backupRec ) 
		backupEntry = createPalmEntry( backupRec );
	if( !backupRec || isFirstSync() || !_equal( backupEntry, pcEntry ) )
	{
		PilotRecord *palmRec = fDatabase->readRecordById( recID );
		PilotAppCategory *palmEntry=0L;
		if (palmRec) 
			palmEntry = createPalmEntry( palmRec );
		syncEntry( pcEntry, backupEntry, palmEntry );
		// update the id just in case it changed
		if ( palmRec ) 
			recID = palmRec->id();
		KPILOT_DELETE( palmRec );
		KPILOT_DELETE( palmEntry );
	}
	
	KPILOT_DELETE( pcEntry );
	KPILOT_DELETE( backupEntry );
	KPILOT_DELETE( backupRec );
	mSyncedIds.append( recID );
	
	// done with the sync process, go on with the next one:
	QTimer::singleShot( 0, this, SLOT( slotPCRecToPalm() ) );
}



void RecordConduit::slotDeletedRecord()
{
	FUNCTIONSETUP;

	PilotRecord *backupRec = fLocalDatabase->readRecordByIndex( mPalmIndex++ );
	if( !backupRec || isFirstSync() )
	{
		KPILOT_DELETE(backupRec);
		QTimer::singleShot( 0, this, SLOT( slotDeleteUnsyncedPCRecords() ) );
		return;
	}

	// already synced, so skip this record:
	if ( mSyncedIds.contains( backupRec->id() ) )
	{
		KPILOT_DELETE( backupRec );
		QTimer::singleShot( 0, this, SLOT( slotDeletedRecord() ) );
		return;
	}

	QString uid = mEntryMap[ backupRec->id() ];
	PCEntry *pcEntry = mPCData->findByUid( uid );
	PilotRecord *palmRec = fDatabase->readRecordById( backupRec->id() );
	PilotAppCategory *backupEntry = 0L;
	if (backupRec) 
		backupEntry = createPalmEntry( backupRec );
	PilotAppCategory*palmEntry=0L;
	if (palmRec) 
		palmEntry = createPalmEntry( palmRec );

	mSyncedIds.append( backupRec->id() );
	syncEntry( pcEntry, backupEntry, palmEntry );

	KPILOT_DELETE( pcEntry );
	KPILOT_DELETE( palmEntry );
	KPILOT_DELETE( backupEntry );
	KPILOT_DELETE( palmRec );
	KPILOT_DELETE( backupRec );
	QTimer::singleShot( 0, this, SLOT( slotDeletedRecord() ) );
}



void RecordConduit::slotDeleteUnsyncedPCRecords()
{
	FUNCTIONSETUP;
	if ( getSyncDirection() == SyncAction::eCopyHHToPC )
	{
		QStringList uids;
		RecordIDList::iterator it;
		QString uid;
		for ( it = mSyncedIds.begin(); it != mSyncedIds.end(); ++it)
		{
			uid = mEntryMap[ *it ];
			if ( !uid.isEmpty() ) uids.append( uid );
		}
		// TODO: Does this speed up anything?
		// qHeapSort( uids );
		const QStringList alluids( mPCData->uids() );
		QStringList::ConstIterator uidit;
		for ( uidit = alluids.constBegin(); uidit != alluids.constEnd(); ++uidit )
		{
			if ( !uids.contains( *uidit ) )
			{
#ifdef DEBUG
				DEBUGCONDUIT << "Deleting PCEntry with uid " << (*uidit) << " from PC (is not on HH, and syncing with HH->PC direction)" << endl;
#endif
				mPCData->removeEntry( *uidit );
			}
		}
	}
	QTimer::singleShot(0, this, SLOT(slotDeleteUnsyncedHHRecords()));
}



void RecordConduit::slotDeleteUnsyncedHHRecords()
{
	FUNCTIONSETUP;
	if ( getSyncDirection() == SyncAction::eCopyPCToHH )
	{
		RecordIDList ids = fDatabase->idList();
		RecordIDList::iterator it;
		for ( it = ids.begin(); it != ids.end(); ++it )
		{
			if ( !mSyncedIds.contains(*it) )
			{
#ifdef DEBUG
				DEBUGCONDUIT << "Deleting record with ID " << *it << " from handheld (is not on PC, and syncing with PC->HH direction)" << endl;
#endif
				fDatabase->deleteRecord(*it);
				fLocalDatabase->deleteRecord(*it);
			}
		}
	}
	QTimer::singleShot( 0, this, SLOT( slotCleanup() ) );
}


void RecordConduit::slotCleanup()
{
	FUNCTIONSETUP;

	// Set the appInfoBlock, just in case the category labels changed
	_setAppInfo();
	doPostSync();
	if(fDatabase)
	{
		fDatabase->resetSyncFlags();
		fDatabase->cleanup();
	}
	if(fLocalDatabase)
	{
		fLocalDatabase->resetSyncFlags();
		fLocalDatabase->cleanup();
	}
	KPILOT_DELETE( fDatabase );
	KPILOT_DELETE( fLocalDatabase );
	// TODO: do something if saving fails!
	mPCData->saveData();
	mPCData->cleanup();
	emit syncDone(this);
}


/** Return the list of category names on the handheld
 */
const QStringList RecordConduit::categories() const
{
	QStringList cats;
	for ( int j = 0; j <=  15; j++ ) {
		QString catName( category( j ) );
		if ( !catName.isEmpty() ) cats << catName;
	}
	return cats;
}
int RecordConduit::findFlags() const
{
	return eqFlagsAlmostAll;
}


bool RecordConduit::isDeleted( const PilotAppCategory *palmEntry )
{
	if ( !palmEntry ) 
		return true;
	if ( palmEntry->isDeleted() && !palmEntry->isArchived() ) 
		return true;
	if ( palmEntry->isArchived() ) 
		return !archiveDeleted();
	return false;
}
bool RecordConduit::isArchived( const PilotAppCategory *palmEntry )
{
	if ( palmEntry && palmEntry->isArchived() )
		return archiveDeleted();
	else 
		return false;
}




/*********************************************************************
                L O A D I N G   T H E   D A T A
 *********************************************************************/



bool RecordConduit::_prepare()
{
	FUNCTIONSETUP;

	readConfig();
	mSyncedIds.clear();
	mPCData = initializePCData();

	return mPCData && doPrepare();
}


void RecordConduit::_getAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	unsigned char *buffer = new unsigned char[PilotRecord::APP_BUFFER_SIZE];
	int appLen=fDatabase->readAppBlock(buffer, PilotRecord::APP_BUFFER_SIZE);

	doUnpackAppInfo( buffer, appLen );
	delete[] buffer;
	buffer = 0;
}

void RecordConduit::_setAppInfo()
{
	FUNCTIONSETUP;
	// get the address application header information
	int appLen = 0;
	unsigned char *buffer = doPackAppInfo( &appLen );
	if ( buffer ) 
	{	if (fDatabase) 
			fDatabase->writeAppBlock( buffer, appLen );
		if (fLocalDatabase) 
			fLocalDatabase->writeAppBlock( buffer, appLen );
		delete[] buffer;
	}
}


int RecordConduit::compareStr( const QString & str1, const QString & str2 )
{
//	FUNCTIONSETUP;
	if ( str1.isEmpty() && str2.isEmpty() ) 
		return 0;
	else 
		return str1.compare( str2 );
}


/**
 * _getCat returns the id of the category from the given categories list.
 * If the address has no categories on the PC, QString::null is returned.
 * If the current category exists in the list of cats, it is returned
 * Otherwise the first cat in the list that exists on the HH is returned
 * If none of the categories exists on the palm, QString::null is returned
 */
QString RecordConduit::getCatForHH( const QStringList cats, const QString curr ) const
{
	FUNCTIONSETUP;
	int j;
	if ( cats.size() < 1 ) 
		return QString::null;
	if ( cats.contains( curr ) )
		return curr;
	for ( QStringList::ConstIterator it = cats.begin(); it != cats.end(); ++it)
	{
		for ( j = 0; j <= 15; j++ )
		{
			QString catnm( category( j ) );
			if ( !(*it).isEmpty() && ( (*it)==catnm ) )
			{
				return catnm;
			}
		}
	}
	// If we have a free label, return the first possible cat
	QString lastCat( category( 15 ) );
	return ( lastCat.isEmpty() ) ? ( cats.first() ) : ( QString::null );
}

void RecordConduit::setCategory(PCEntry * pcEntry, QString cat)
{
	if ( !cat.isEmpty() && cat!=category( 0 ) )
		pcEntry->insertCategory(cat);
}






/*********************************************************************
              G E N E R A L   S Y N C   F U N C T I O N
         These functions modify the Handheld and the addressbook
 *********************************************************************/



bool RecordConduit::syncEntry( PCEntry *pcEntry, PilotAppCategory*backupEntry,
	PilotAppCategory*palmEntry)
{
	FUNCTIONSETUP;

	if ( getSyncDirection() == SyncAction::eCopyPCToHH )
	{
		if ( pcEntry->isEmpty() )
		{
			return pcDeleteEntry( pcEntry, backupEntry, palmEntry );
		}
		else
		{
			return pcCopyToPalm( pcEntry, backupEntry, palmEntry );
		}
	}

	if ( getSyncDirection() == SyncAction::eCopyHHToPC )
	{
		if (!palmEntry)
			return pcDeleteEntry(pcEntry, backupEntry, palmEntry);
		else
			return palmCopyToPC(pcEntry, backupEntry, palmEntry);
	}

	if ( !backupEntry || isFirstSync() )
	{
		/*
		Resolution matrix (0..does not exist, E..exists, D..deleted flag set, A..archived):
		  HH    PC  | Resolution
		  ------------------------------------------------------------
		   0     A  |  -
		   0     E  |  PC -> HH, reset ID if not set correctly
		   D     0  |  delete (error, should never occur!!!)
		   D     E  |  CR (ERROR)
		   E/A   0  |  HH -> PC
		   E/A   E/A|  merge/CR
		 */
		if  ( !palmEntry && isArchived( pcEntry ) )
		{
			return true;
		}
		else if ( !palmEntry && !pcEntry->isEmpty() )
		{
			// PC->HH
			bool res = pcCopyToPalm( pcEntry, 0L, 0L );
			return res;
		}
		else if ( !palmEntry && pcEntry->isEmpty() )
		{
			// everything's empty -> ERROR
			return false;
		}
		else if ( ( isDeleted( palmEntry ) || isArchived( palmEntry ) ) && pcEntry->isEmpty())
		{
			if ( isArchived( palmEntry ) )
				return palmCopyToPC( pcEntry, 0L, palmEntry );
			else
				// this happens if you add a record on the handheld and delete it again before you do the next sync
				return pcDeleteEntry( pcEntry, 0L, palmEntry );
		}
		else if ( ( isDeleted(palmEntry) || isArchived( palmEntry ) ) && !pcEntry->isEmpty() )
		{
			// CR (ERROR)
			return smartMergeEntry( pcEntry, 0L, palmEntry );
		}
		else if ( pcEntry->isEmpty() )
		{
			// HH->PC
			return palmCopyToPC( pcEntry, 0L, palmEntry );
		}
		else
		{
			// Conflict Resolution
			return smartMergeEntry( pcEntry, 0L, palmEntry );
		}
	} // !backupEntry
	else
	{
		/*
		Resolution matrix:
		  1) if HH.(empty| (deleted &! archived) ) -> { if (PC==B) -> delete, else -> CR }
		     if HH.archived -> {if (PC==B) -> copyToPC, else -> CR }
		     if PC.empty -> { if (HH==B) -> delete, else -> CR }
		     if PC.archived -> {if (HH==B) -> delete on HH, else CR }
		  2) if PC==HH -> { update B, update ID of PC if needed }
		  3) if PC==B -> { HH!=PC, thus HH modified, so copy HH->PC }
		     if HH==B -> { PC!=HH, thus PC modified, so copy PC->HH }
		  4) else: all three PCEntrys are different -> CR
		*/

		if ( !palmEntry || isDeleted(palmEntry) )
		{
			if ( _equal( backupEntry, pcEntry ) || pcEntry->isEmpty() )
			{
				return pcDeleteEntry( pcEntry, backupEntry, 0L );
			}
			else
			{
				return smartMergeEntry( pcEntry, backupEntry, 0L );
			}
		}
		else if ( pcEntry->isEmpty() )
		{
			if (*palmEntry == *backupEntry)
			{
				return pcDeleteEntry( pcEntry, backupEntry, palmEntry );
			}
			else
			{
				return smartMergeEntry( pcEntry, backupEntry, palmEntry );
			}
		}
		else if ( _equal( palmEntry, pcEntry ) )
		{
			// update Backup, update ID of PC if neededd
			return backupSaveEntry( palmEntry );
		}
		else if ( _equal( backupEntry, pcEntry ) )
		{
#ifdef DEBUG
			DEBUGCONDUIT << "Flags: " << palmEntry->getAttrib() << ", isDeleted=" <<
				isDeleted( palmEntry ) << ", isArchived=" << isArchived( palmEntry ) 
				<< endl;
#endif
			if ( isDeleted( palmEntry ) )
			{
				return pcDeleteEntry( pcEntry, backupEntry, palmEntry );
			}
			else
			{
				return palmCopyToPC( pcEntry, backupEntry, palmEntry );
			}
		}
		else if ( *palmEntry == *backupEntry )
		{
			return pcCopyToPalm( pcEntry, backupEntry, palmEntry );
		}
		else
		{
			// CR, since all are different
			return smartMergeEntry( pcEntry, backupEntry, palmEntry );
		}
	} // backupEntry
	return false;
}

bool RecordConduit::pcCopyToPalm( PCEntry *pcEntry, PilotAppCategory *backupEntry,
		PilotAppCategory*palmEntry )
{
	FUNCTIONSETUP;

	if ( pcEntry->isEmpty() ) return false;
	PilotAppCategory *hhEntry = palmEntry;
	bool hhEntryCreated = false;
	if ( !hhEntry )
	{
		hhEntry = createPalmEntry( 0 );
		hhEntryCreated=true;
	}
	_copy( hhEntry, pcEntry );
#ifdef DEBUG
	DEBUGCONDUIT << "palmEntry->id=" << hhEntry->id() << ", pcEntry.ID=" <<
		pcEntry->uid() << endl;
#endif

	if( palmSaveEntry( hhEntry, pcEntry ) )
	{
#ifdef DEBUG
		DEBUGCONDUIT << "Entry palmEntry->id=" <<
		hhEntry->id() << "saved to palm, now updating pcEntry->uid()=" << pcEntry->uid() << endl;
#endif
		pcSaveEntry( pcEntry, backupEntry, hhEntry );
	}
	if ( hhEntryCreated ) KPILOT_DELETE( hhEntry );
	return true;
}




bool RecordConduit::palmCopyToPC( PCEntry *pcEntry, PilotAppCategory *backupEntry,
		PilotAppCategory *palmEntry )
{
	FUNCTIONSETUP;
	if ( !palmEntry )
	{
		return false;
	}
	_copy( pcEntry, palmEntry );
	pcSaveEntry( pcEntry, backupEntry, palmEntry );
	backupSaveEntry( palmEntry );
	return true;
}



/*********************************************************************
                 l o w - l e v e l   f u n c t i o n s   f o r
                   adding / removing palm/pc records
 *********************************************************************/



bool RecordConduit::palmSaveEntry( PilotAppCategory *palmEntry, PCEntry *pcEntry )
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << "Saving to pilot " << palmEntry->id() << endl;
#endif

	PilotRecord *pilotRec = palmEntry->pack();
	recordid_t pilotId = fDatabase->writeRecord(pilotRec);
#ifdef DEBUG
	DEBUGCONDUIT << "PilotRec nach writeRecord (" << pilotId << 
		": ID=" << pilotRec->id() << endl;
#endif
	fLocalDatabase->writeRecord( pilotRec );
	KPILOT_DELETE( pilotRec );

	// pilotId == 0 if using local db, so don't overwrite the valid id
	if ( pilotId != 0 )
	{
		palmEntry->setID( pilotId );
		if ( !mSyncedIds.contains( pilotId ) ) 
		{
			mSyncedIds.append( pilotId );
		}
	}

	recordid_t hhId( pcEntry->recid() );
	if ( hhId != pilotId )
	{
		pcEntry->setRecid( pilotId );
		return true;
	}

	return false;
}



bool RecordConduit::backupSaveEntry( PilotAppCategory *backup )
{
	FUNCTIONSETUP;
	if ( !backup ) return false;


#ifdef DEBUG
//	showPilotAppCategory( backup );
#endif
	PilotRecord *pilotRec = backup->pack();
	fLocalDatabase->writeRecord( pilotRec );
	KPILOT_DELETE( pilotRec );
	return true;
}



bool RecordConduit::pcSaveEntry( PCEntry *pcEntry, PilotAppCategory *,
	PilotAppCategory * )
{
	FUNCTIONSETUP;

#ifdef DEBUG
	DEBUGCONDUIT << "Before _savepcEntry, pcEntry->uid()=" << 
		pcEntry->uid() << endl;
#endif
	if ( pcEntry->recid() != 0 )
	{
		mEntryMap.insert( pcEntry->recid(), pcEntry->uid() );
	}

	mPCData->updateEntry( pcEntry );
	return true;
}



bool RecordConduit::pcDeleteEntry( PCEntry *pcEntry, PilotAppCategory *backupEntry,
	PilotAppCategory *palmEntry )
{
	FUNCTIONSETUP;

	if ( palmEntry )
	{
		if ( !mSyncedIds.contains( palmEntry->id() ) ) 
		{
			mSyncedIds.append(palmEntry->id());
		}
		palmEntry->makeDeleted();
		PilotRecord *pilotRec = palmEntry->pack();
		pilotRec->setDeleted();
		mPalmIndex--;
		fDatabase->writeRecord( pilotRec );
		fLocalDatabase->writeRecord( pilotRec );
		mSyncedIds.append( pilotRec->id() );
		KPILOT_DELETE( pilotRec );
	}
	else if ( backupEntry )
	{
		if ( !mSyncedIds.contains( backupEntry->id() ) )
		{
			mSyncedIds.append( backupEntry->id() );
		}
		backupEntry->makeDeleted();
		PilotRecord *pilotRec = backupEntry->pack();
		pilotRec->setDeleted();
		mPalmIndex--;
		fLocalDatabase->writeRecord( pilotRec );
		mSyncedIds.append( pilotRec->id() );
		KPILOT_DELETE( pilotRec );
	}
	if ( !pcEntry->isEmpty() )
	{
#ifdef DEBUG
		DEBUGCONDUIT << fname << " removing " << pcEntry->uid() << endl;
#endif
		mPCData->removeEntry( pcEntry );
	}
	return true;
}



/*********************************************************************
                   C O P Y   R E C O R D S
 *********************************************************************/





/*********************************************************************
 C O N F L I C T   R E S O L U T I O N   a n d   M E R G I N G
 *********************************************************************/




// TODO: right now entries are equal if both first/last name and organization are
//  equal. This rules out two entries for the same person(e.g. real home and weekend home)
//  or two persons with the same name where you don't know the organization.!!!
RecordConduit::PCEntry *RecordConduit::findMatch( PilotAppCategory *palmEntry ) const
{
	FUNCTIONSETUP;
	if ( !palmEntry ) 
		return 0;
		
	// TODO: also search with the pilotID
	// first, use the pilotID to UID map to find the appropriate record
	if( !isFirstSync() && ( palmEntry->id() > 0) )
	{
		QString id( mEntryMap[palmEntry->id()] );
#ifdef DEBUG
		DEBUGCONDUIT << fname << ": PilotRecord has id " << palmEntry->id() << ", mapped to " << id << endl;
#endif
		if( !id.isEmpty() )
		{
			PCEntry *res = mPCData->findByUid( id );
			if ( !res && !res->isEmpty() ) return res;
			KPILOT_DELETE( res );
#ifdef DEBUG
			DEBUGCONDUIT << fname << ": PilotRecord has id " << palmEntry->id() << 
				", but could not be found on the PC side" << endl;
#endif
		}
	}

	for ( PCData::Iterator iter = mPCData->begin(); !mPCData->atEnd( iter ); ++iter )
	{
		PCEntry *abEntry = *iter;
		recordid_t rid( abEntry->recid() );
		if ( rid>0 )
		{
			if ( rid == palmEntry->id() ) 
				return abEntry;// yes, we found it
			// skip this PCEntry, as it has a different corresponding address on the handheld
			//if ( mAllIds.contains( rid ) ) continue;
		}

		if ( _equal( palmEntry, abEntry, eqFlagsAlmostAll ) )
		{
			return abEntry;
		}
		KPILOT_DELETE( abEntry );
	}
#ifdef DEBUG
	DEBUGCONDUIT << fname << ": Could not find any entry matching Palm record with id " << QString::number( palmEntry->id() ) << endl;
#endif
	return 0;
}


#include "recordConduit.moc"
