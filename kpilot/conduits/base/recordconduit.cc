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
#include "options.h"

#include "recordconduit.h"
#include "idmapping.h"
#include "pcdataproxy.h"
#include "hhdataproxy.h"
#include "record.h"

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
	// FIXME: It was this: CSL1( "fDatabaseName" .... I don't how to handle
	// QString properly at this point.
	bool hhDatabaseOpen = openDatabases( fDatabaseName, &retrieved );
	// If retrieved is true there is no local backup database so a first sync 
		// should be done then (see 6.3.2).
	bool backupDatabaseOpen = !retrieved;
	setFirstSync( retrieved );
	
	initDataProxies();
	
	// For the pc data proxy we can only after initilisation know if it could be
	// opened.
	bool pcDatabaseOpen = fPCDataProxy->isOpen();
	
	// See 6.2
	fMapping = new IDMapping( fConduitName );
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
		//firstSync();
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
	
	return true;
}

void RecordConduit::hotSync()
{
	FUNCTIONSETUP;
	// Walk through all modified hand held records. The proxy is responsible for
	// serving the right records.
	while( fHHDataProxy->hasNext() )
	{
		Record *hhRecord = fHHDataProxy->next();
		Record *backupRecord = fBackupDataProxy->readRecordById( hhRecord->id() );
		Record *pcRecord = 0L;
		
		QVariant pcRecordId = fMapping->recordId( hhRecord->id() );
		if( pcRecordId.isValid() ) {
			// There is a mapping.
			pcRecord = fPCDataProxy->readRecordById( pcRecordId );
		}
		
		syncRecords( hhRecord, backupRecord, pcRecord );
	}
	
	// Walk through all modified pc records. The proxy is responsible for
	// serving the right records.
	while( fPCDataProxy->hasNext() )
	{
		Record *pcRecord = fPCDataProxy->next();
		Record *backupRecord = 0L;
		Record *hhRecord = 0L;
		
		QVariant hhRecordId = fMapping->recordId( pcRecord->id() );
		if( hhRecordId.isValid() ) {
			// There is a mapping.
			backupRecord = fBackupDataProxy->readRecordById( pcRecord->id() );
			hhRecord = fHHDataProxy->readRecordById( pcRecord->id() );
		}
		
		syncRecords( hhRecord, backupRecord, pcRecord );
	}
}

void RecordConduit::syncRecords( Record *pcRecord, Record *backupRecord,
			Record *hhRecord )
{
	FUNCTIONSETUP;
	#warning implemented, but needs work
	
	// Two records for which we seem to have a mapping.
	if( hhRecord && pcRecord )
	{
		// Cases: 6.5.1 (fullSync), 6.5.3
		if( hhRecord->isModified() || isFullSync() )
		{
			// Case: 6.5.9
			if( pcRecord->isModified() )
			{
				solveConflict( hhRecord, pcRecord );
			}
			else
			{
				//            from      to
				syncFields( hhRecord, pcRecord );
			}
		}
		// Case: 6.5.6
		else if( pcRecord->isModified() )
		{
			//            from      to
			syncFields( pcRecord, hhRecord );
		}
		/*
		else
		{
			// Case: 6.5.1 (hotSync)
		}
		*/
	}
	
	if( hhRecord )
	{
		if( backupRecord )
		{
			// Case: 6.5.11
			if( hhRecord->isModified() )
			{
				// Pc record deleted, hh record modified.
				solveConflict( 0L, hhRecord );
			}
			// Case: 6.5.7
			else
			{
				fHHDataProxy->remove( hhRecord->id() );
				fMapping->remove( hhRecord->id() );
			}
		}
		// Case: 6.5.2 (and 6.5.8 if the conduit iterates over the HH data proxy
		// first )
		else
		{
			// Warning id is a temporary id. Only after commit we know what id is
			// assigned to the record. So on commit the proxy should get the mapping
			// so that it can change the mapping.
			QVariant id = fPCDataProxy->create( hhRecord );
			fMapping->map( hhRecord->id(), id );
		}
	}
	
	if( pcRecord )
	{
		if( backupRecord )
		{
			// Case: 6.5.10
			if( pcRecord->isModified() )
			{
				solveConflict( pcRecord, 0L );
			}
			// Case: 6.5.4
			else
			{
				fPCDataProxy->remove( pcRecord->id() );
				fMapping->remove( pcRecord->id() );
			}
		}
		// Case: 6.5.5 (and 6.5.8 if the conduit iterates over the PC data proxy 
		// first )
		else
		{
			QVariant id = fHHDataProxy->create( pcRecord );
			fMapping->map( id, pcRecord->id() );
		}
	}
	
	// For completeness: Case 6.5.12
	/*
	if( backupRecord )
	{
		return true;
	}
	
	return false;
	*/
}

void RecordConduit::syncFields( Record *to, Record *from )
{
	FUNCTIONSETUP;
	Q_UNUSED(to);
	Q_UNUSED(from);
	#warning Not implemented!
}

void RecordConduit::solveConflict( Record *pcRecord, Record *hhRecord )
{
	FUNCTIONSETUP;
	Q_UNUSED(pcRecord);
	Q_UNUSED(hhRecord);
	#warning Not implemented!
}

bool RecordConduit::askConfirmation(const QString & volatilityMessage)
{
	FUNCTIONSETUP;
	#warning Not implemented!
	Q_UNUSED(volatilityMessage);
	return false;
}

void RecordConduit::createBackupDatabase() 
{
	FUNCTIONSETUP;
	#warning Not implemented!
}

