/* RecordConduit.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
** Copyright (C) 2007 by Jason "vanRijn" Kasper
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

#include "recordconduit.h"
#include "idmapping.h"
#include "cudcounter.h"
#include "dataproxy.h"
#include "hhdataproxy.h"
#include "record.h"
#include "hhrecord.h"
#include "recordconduitSettings.h"

#include "options.h"
#include "kpilotSettings.h"

#include <KMessageBox>

RecordConduit::RecordConduit( KPilotLink *o, const QStringList &a
	, const QString &databaseName, const QString &conduitName ) :
	ConduitAction(o, a),
	fHHDataProxy(0L),
	fBackupDataProxy(0L),
	fPCDataProxy(0L)
{
	fDatabaseName = databaseName;
	fConduitName = conduitName;
}

RecordConduit::~RecordConduit()
{
	delete fHHDataProxy;
	delete fBackupDataProxy;
	delete fPCDataProxy;
}

/* virtual */ bool RecordConduit::exec()
{
	FUNCTIONSETUP;
	
	loadSettings();
	
	// Try to open the handheld database and the local backup database.
	bool retrieved = false;
	
	bool hhDatabaseOpen = openDatabases( fDatabaseName, &retrieved );
	// If retrieved is true there is no local backup database so a first sync 
		// should be done then (see 6.3.2).
	bool backupDatabaseOpen = !retrieved;
	setFirstSync( retrieved );
	
	// NOTE: Do not forget that the HHData proxy and the backup proxy must use
	// the opened databases, maybe we should pass them for clearity to this method.
	initDataProxies();
	
	// Make sure that the implementing class did initialize all proxies.
	if( !fPCDataProxy || !fHHDataProxy || !fBackupDataProxy )
	{
		DEBUGKPILOT << fname << ": One of the dataproxies was not initialized" << endl;
		return false;
	}
	
	// For the pc data proxy we can only after initilisation know if it could be
	// opened.
	bool pcDatabaseOpen = fPCDataProxy->isOpen();
	
	// See 6.2
	fMapping = new IDMapping( KPilotSettings::userName(), fConduitName );
	if( !fMapping->isValid( fBackupDataProxy->ids() ) )
	{
		setFirstSync( true );
	}
	
	// Determine syncmode, see 6.3
	// FIXME: Is this the most efficient way, or is it even possible what i do 
	//        here?
	if( hhDatabaseOpen && pcDatabaseOpen && backupDatabaseOpen )
	{
		changeSync( SyncMode::eHotSync ); // 6.3.1
		fHHDataProxy->setIterateMode( DataProxy::Modified );
		fPCDataProxy->setIterateMode( DataProxy::Modified );
		hotSync();
	}
	else if( hhDatabaseOpen && pcDatabaseOpen && !backupDatabaseOpen )
	{
		setFirstSync( true ); // 6.3.2
		fHHDataProxy->setIterateMode( DataProxy::All );
		fPCDataProxy->setIterateMode( DataProxy::All );
		firstSync();
	}
	else if( hhDatabaseOpen && !pcDatabaseOpen )
	{
		changeSync( SyncMode::eCopyHHToPC ); // 6.3.3 and 6.3.4
		fHHDataProxy->setIterateMode( DataProxy::All );
		fPCDataProxy->setIterateMode( DataProxy::All );
		//copyHHToPC();
	}
	else if( !hhDatabaseOpen && pcDatabaseOpen )
	{
		changeSync( SyncMode::eCopyPCToHH ); // 6.3.5 and 6.3.6
		fHHDataProxy->setIterateMode( DataProxy::All );
		fPCDataProxy->setIterateMode( DataProxy::All );
		//copyPCToHH();
	}
	else
	{
		emit logError( i18n( "Failed to open the pc database and the handheld "
			"datbase, no data to sync." ) );
		return false; // 6.3.7 and 6.3.8
	}
	
	// Sync finished, clean up things.
	fHHDataProxy->syncFinished();
	fPCDataProxy->syncFinished();
	
	fMapping->setLastSyncedDate( QDateTime::currentDateTime() );
	if( !fMapping->isValid( fHHDataProxy->ids() ) )
	{
		// TODO: Warn the user.
		return false;
	}
	
	if( !checkVolatility() )
	{
		// volatility bounds are exceeded or the user did not want to proceed.
		return false;
	}
	
	/*
	 * If from this point something goes wrong (which shouldn't because we did our
	 * very best to deliver sane data) some of the data (mapping, hh database or
	 * pc database) will be corrupted.
	 */
	if( !fHHDataProxy->commit() )
	{
		fHHDataProxy->rollback();
		// TODO: notify user.
		return false;
	}
	
	if( !fPCDataProxy->commit() )
	{
		fPCDataProxy->rollback();
		fHHDataProxy->rollback();
		// TODO: notify user.
		return false;
	}
	
	// Fix the ids.
	QMapIterator<QString,QString> it( fHHDataProxy->changedIds() );
	while( it.hasNext() )
	{
		it.next();
		fMapping->changeHHId( it.key(), it.value() );
	}
	
	it = QMapIterator<QString,QString>( fPCDataProxy->changedIds() );
	while( it.hasNext() )
	{
		it.next();
		fMapping->changePCId( it.key(), it.value() );
	}
	
	// Now we can commit the mapping.
	if( !fMapping->commit() )
	{
		fMapping->rollback();
		fPCDataProxy->rollback();
		fHHDataProxy->rollback();
		return false;
	}
	
	// If commit fails every commit should be undone and the user should be 
	// notified about the failure.
	if( !createBackupDatabase() )
	{
		fMapping->rollback();
		fPCDataProxy->rollback();
		fHHDataProxy->rollback();
		// TODO: notify user.
		return false;
	}
	
	return true;
}

bool RecordConduit::checkVolatility()
{
	FUNCTIONSETUP;
	
	const CUDCounter *fCtrHH = fHHDataProxy->counter();
	const CUDCounter *fCtrPC = fPCDataProxy->counter();

	addSyncLogEntry(fCtrHH->moo() +'\n',false);
	DEBUGKPILOT << fname << ": " << fCtrHH->moo() << endl;
	addSyncLogEntry(fCtrPC->moo() +'\n',false);
	DEBUGKPILOT << fname << ": " << fCtrPC->moo() << endl;

	// STEP2 of making sure we don't delete our little user's
	// precious data...
	// sanity checks for handheld...
	int hhVolatility = fCtrHH->percentDeleted() +
		fCtrHH->percentUpdated() + fCtrHH->percentCreated();
	
	int pcVolatility = fCtrPC->percentDeleted() +
		fCtrPC->percentUpdated() + fCtrPC->percentCreated();

	// TODO: allow user to configure this...
	// this is a percentage...
	int allowedVolatility = 70;
		
	QString caption = i18n( "Large Changes Detected" );
	// args are already i18n'd
	KLocalizedString template_query = ki18n( "The %1 conduit has made a "
		"large number of changes to your %2.  Do you want "
		"to allow this change?\nDetails:\n\t%3");

	// Default to allow changes
	int rc = KMessageBox::Yes;

	if (hhVolatility > allowedVolatility)
	{
		QString query = template_query
			.subs( fConduitName )
			.subs( i18n("handheld") )
			.subs( fCtrHH->moo() )
			.toString();
		
		DEBUGKPILOT << fname << ": high volatility."
			<< " Check with user: [" << query << "]." << endl;
		
		rc = questionYesNo( query, caption, QString(), 0 );
	}
	
	if (pcVolatility > allowedVolatility)
	{
		QString query = template_query
			.subs( fConduitName )
			.subs( i18n( "pc" ) )
			.subs( fCtrPC->moo() )
			.toString();
		
		DEBUGKPILOT << fname << ": high volatility."
			<< "  Check with user: [" << query << "]." << endl;
		
		rc = questionYesNo( query, caption, QString(), 0 );
	}
	
	// Allow the changes
	if( rc == KMessageBox::Yes )
	{
		return true;
	}

	// Disallow the changes
	return false;
}

void RecordConduit::hotSync()
{
	FUNCTIONSETUP;
	// Walk through all modified hand held records. The proxy is responsible for
	// serving the right records.
	
	DEBUGKPILOT << fname << ": Walking over modified hh records." << endl;
	
	fHHDataProxy->resetIterator();
	while( fHHDataProxy->hasNext() )
	{
		HHRecord *hhRecord = static_cast<HHRecord*>( fHHDataProxy->next() );
		HHRecord *backupRecord = static_cast<HHRecord*>( 
			fBackupDataProxy->find( hhRecord->id() ) );
		Record *pcRecord = 0L;
		
		QString pcRecordId = fMapping->pcRecordId( hhRecord->id() );
		if( !pcRecordId.isNull() ) {
			// There is a mapping.
			pcRecord = fPCDataProxy->find( pcRecordId );
		}
		
		syncRecords( pcRecord, backupRecord, hhRecord );
	}
	
	// Walk through all modified pc records. The proxy is responsible for
	// serving the right records.
	
	DEBUGKPILOT << fname << ": Walking over modified pc records." << endl;
	
	fPCDataProxy->resetIterator();
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		HHRecord *backupRecord = 0L;
		HHRecord *hhRecord = 0L;
		
		QString hhRecordId = fMapping->hhRecordId( pcRecord->id() );
		
		if( !hhRecordId.isNull() ) {
			// There is a mapping.
			backupRecord = static_cast<HHRecord*>( fBackupDataProxy->find( hhRecordId ) );
			hhRecord = static_cast<HHRecord*>( fHHDataProxy->find( hhRecordId ) );
		}
		
		syncRecords( pcRecord, backupRecord, hhRecord );
	}
}

void RecordConduit::firstSync()
{
	FUNCTIONSETUP;
	
	DEBUGKPILOT << fname << ": Walking over all hh records." << endl;
	
	fHHDataProxy->resetIterator();
	while( fHHDataProxy->hasNext() )
	{
		HHRecord *hhRecord = static_cast<HHRecord*>( fHHDataProxy->next() );
		Record *pcRecord = findMatch( hhRecord );
		
		if( pcRecord )
		{
			// TODO: Make this configurable or something, maybe pcRecord should
			// overide or user should be asked which record should be used. In this
			// case it might even be usefull to let the user select per field which
			// record should be used. For now the handheld record overides.
			
			// Overide pcRecord values with hhRecord values.
			syncFields( pcRecord, hhRecord );
			
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
	
	fPCDataProxy->resetIterator();
	
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		
		if( !fMapping->containsPCId( pcRecord->id() ) )
		{
			HHRecord *hhRecord = newHHRecord( pcRecord );
			fHHDataProxy->create( hhRecord );
			
			fMapping->map( hhRecord->id(), pcRecord->id() );
		}
	}
}

Record* RecordConduit::findMatch( HHRecord *hhRec )
{
	fPCDataProxy->resetIterator();
	
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRec = fPCDataProxy->next();
		
		if( equal( pcRec, hhRec ) )
		{
			return pcRec;
		}
	}
	
	return 0L;
}

void RecordConduit::syncRecords( Record *pcRecord, HHRecord *backupRecord,
	HHRecord *hhRecord )
{
	FUNCTIONSETUP;
	
	// Two records for which we seem to have a mapping.
	if( hhRecord && pcRecord )
	{
		if( hhRecord->isModified() || isFullSync() )
		{
			if( pcRecord->isModified() )
			{
				if( pcRecord->isDeleted() && hhRecord->isDeleted() )
				{
					// Case: 6.5.12
					DEBUGKPILOT << fname << ": Case 6.5.12" << endl;
					deleteRecords( pcRecord, hhRecord );
				}
				else
				{
					// Case: 6.5.9, 6.5.10 or 6.5.11
					DEBUGKPILOT << fname << ": Case 6.5.9, 6.5.10 or 6.5.11" << endl;
					solveConflict( pcRecord, hhRecord );
				}
			}
			else
			{
				if( hhRecord->isDeleted() )
				{
					// Case 6.5.4
					DEBUGKPILOT << fname << ": Case 6.5.4" << endl;
					deleteRecords( pcRecord, hhRecord );
				}
				else
				{
					// Case 6.5.3 or 6.5.1 (fullSync)
					DEBUGKPILOT << fname << ": Case 6.5.3 or 6.5.1 (fullSync)" << endl;

					// Keep hhRecord values.
					syncFields( pcRecord, hhRecord );
				}
			}
		}
		else if( pcRecord->isModified() )
		{
			if( pcRecord->isDeleted() )
			{
				//  Case: 6.5.7
				DEBUGKPILOT << fname << ": Case 6.5.7" << endl;
				deleteRecords( pcRecord, hhRecord );
			}
			else
			{
				// Case: 6.5.6
				DEBUGKPILOT << fname << ": Case 6.5.6" << endl;
				// Keep pc record values.
				syncFields( pcRecord, hhRecord, false );
			}
		}
		else
		{
			// Case: 6.5.1 (hotSync)
			DEBUGKPILOT << fname << ": Case 6.5.1" << endl;
		}
	}
	else if( hhRecord )
	{
		// Case: 6.5.2 or 6.5.8
		// Warning id is a temporary id. Only after commit we know what id is
		// assigned to the record. So on commit the proxy should get the mapping
		// so that it can change the mapping.
		DEBUGKPILOT << fname << ": Case 6.5.2 and 6.5.8" << endl;
		pcRecord = createPCRecord( hhRecord );
		QString id = fPCDataProxy->create( pcRecord );
		fMapping->map( hhRecord->id(), id );
		
		pcRecord->synced();
		hhRecord->synced();
	}
	else if( pcRecord )
	{
		if( fMapping->containsPCId( pcRecord->id() ) && pcRecord->isDeleted() )
		{
			DEBUGKPILOT << fname << ": Case 6.5.17" << endl;
			fMapping->removePCId( pcRecord->id() );
			fPCDataProxy->remove( pcRecord->id() );
		}
		else
		{
			// Case: 6.5.5 or 6.5.8
			DEBUGKPILOT << fname << ": Case 6.5.5 or 6.5.8" << endl;
			hhRecord = createHHRecord( pcRecord );
			QString id = fHHDataProxy->create( hhRecord );
			fMapping->map( id, pcRecord->id() );
			
			pcRecord->synced();
			hhRecord->synced();
		}
	}
	else
	{
		DEBUGKPILOT << fname << ": This should not happen." << endl;
	}
}

/*
bool RecordConduit::syncFields( Record *from, Record *to )
{
	FUNCTIONSETUP;
	
	// This shouldn't happen.
	if( !to || !from )
	{
		DEBUGKPILOT << fname << ": one of the two records is zero! Not syncing"
			<< endl;
		return false;
	}
	
	if( to->fields().size() != from->fields().size() )
	{
		DEBUGKPILOT << fname << ": fieldcount of both records differ. Not syncing"
			<< endl;
		return false;
	}
	else
	{
		QStringList fields = from->fields();
		QStringListIterator it( fields );
		while( it.hasNext() )
		{
			QString field = it.next();
			if( !to->fields().contains( field ) )
			{
				DEBUGKPILOT << fname << ": One of the fields is not present in the "
					<< "to-record. Not syncing." << endl;
				return false;
			}
		}
	}
	
	QStringList fields = from->fields();
	QStringListIterator it( fields );
	while( it.hasNext() )
	{
		QString field = it.next();
		bool result = to->setValue( field, from->value( field ) );
		if( !result )
		{
			DEBUGKPILOT << fname << ": error, field " << field << " does not exists ";
			DEBUGKPILOT	<< "or the value given is not valid!" << endl;
			return false;
		}
	}
	
	// Both records are in sync so they are no longer modified.
	from->synced();
	to->synced();
	
	return true;
}
*/

void RecordConduit::syncConflictedRecords( Record *pcRecord, HHRecord *hhRecord
	, bool pcOverides )
{
	FUNCTIONSETUP;
	
	if( pcOverides )
	{
		if( pcRecord->isDeleted() )
		{
			deleteRecords( pcRecord, hhRecord );
		}
		else
		{
			// Keep pcRecord. The hhRecord is changed so undo that changes.
			syncFields( pcRecord, hhRecord, false );
		}
	}
	else
	{
		if( hhRecord->isDeleted() )
		{
			if( pcRecord->isModified() )
			{
				DEBUGKPILOT << fname << ": Case 6.5.16" << endl;
				// Keep hhRecordValues.
				syncFields( pcRecord, hhRecord );
			}
			// else { DEBUGKPILOT << fname << ": Case 6.5.15" << endl; }
			deleteRecords( pcRecord, hhRecord );
		}
		else
		{
			// Keep hhRecord. The pcRecord is changed so undo that changes.
			syncFields( pcRecord, hhRecord );
		}
	}
}

void RecordConduit::deleteRecords( Record *pcRecord, HHRecord *hhRecord )
{
	fHHDataProxy->remove( hhRecord->id() );
	
	if( !hhRecord->isArchived() )
	{
		fPCDataProxy->remove( pcRecord->id() );
		fMapping->removePCId( pcRecord->id() );
	}
	else
	{
		fMapping->archiveRecord( hhRecord->id() );
	}
}

void RecordConduit::solveConflict( Record *pcRecord, HHRecord *hhRecord )
{
	FUNCTIONSETUP;
	
	// NOTE: One of the two records might be 0L, which means that it's deleted.
	
	DEBUGKPILOT << fname << ": solving conflict for pc: " << pcRecord->id() 
		<< " and hh: " << hhRecord->id() << endl;
	
	int res = getConflictResolution();
	if ( res == SyncAction::eAskUser )
	{
		// TODO: Make this nicer, like the abbrowser conduit had.
		QString query = i18n( "The following item was modified "
			"both on the Handheld and on your PC:\nPC entry:\n\t" );
		if( pcRecord )
		{
			query += pcRecord->toString();
		}
		else
		{
			query += i18n( "deleted" );
		}
		query += i18n( "\nHandheld entry:\n\t" );
		if( hhRecord )
		{
			query += hhRecord->toString();
		}
		else
		{
			query += i18n( "deleted" );
		}
		query += i18n( "\n\nWhich entry do you want to keep? It will "
			"overwrite the other entry." );

		if( KMessageBox::No == questionYesNo(
			query,
			i18n( "Conflicting Entries" ),
			QString(),
			0 /* Never timeout */,
			i18n( "Handheld" ), i18n( "PC" )) )
		{
			// Keep PC record
			syncConflictedRecords( pcRecord, hhRecord, true );
		}
		else
		{
			// Keep Handheld record
			syncConflictedRecords( pcRecord, hhRecord, false );
		}
	}
	else if( res == eHHOverrides )
	{
		// Keep Handheld record
		syncConflictedRecords( pcRecord, hhRecord, false );
	}
	else if( res == ePCOverrides )
	{
		// Keep PC record
		syncConflictedRecords( pcRecord, hhRecord, true );
	}
	else if( res == eDuplicate )
	{
		/*
		 * break the previous relationship and create a new one on both sides, 
		 * duplicating bothrecords
		 */
		fMapping->removePCId( pcRecord->id() );
		
		HHRecord *hhRec = createHHRecord( pcRecord );
		QString id = fHHDataProxy->create( hhRec );
		fMapping->map( id, pcRecord->id() );
		
		Record *pcRec = createPCRecord( hhRecord );
		id = fPCDataProxy->create( pcRec );
		fMapping->map( id, pcRecord->id() );
	}
	else if( res == ePreviousSyncOverrides )
	{
		// FIXME: Implement.
		
	}
	
	// else: eDoNothing
	//return true;
}
