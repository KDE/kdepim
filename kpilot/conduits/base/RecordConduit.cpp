/* RecordConduit.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema
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

#include "RecordConduit.h"
#include "idmapping.h"
#include "pcdataproxy.h"
#include "hhdataproxy.h"
//#include "DataProxy.h"
//#include "Record.h"

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
	if( !fMapping->isValid( fBackupDataProxy ) )
	{
		setFirstSync( true );
	}
	
	// Determine syncmode, see 6.3
	if( hhDatabaseOpen && pcDatabaseOpen && backupDatabaseOpen )
	{
		changeSync( SyncMode::eHotSync ); // 6.3.1
	}
	else if( hhDatabaseOpen && pcDatabaseOpen && !backupDatabaseOpen )
	{
		setFirstSync( true ); // 6.3.2
	}
	else if( hhDatabaseOpen && !pcDatabaseOpen )
	{
		changeSync( SyncMode::eCopyHHToPC ); // 6.3.3 and 6.3.4
	}
	else if( !hhDatabaseOpen && pcDatabaseOpen )
	{
		changeSync( SyncMode::eCopyPCToHH ); // 6.3.5 and 6.3.6
	}
	else
	{
		emit logError( i18n( "Failed to open the pc database and the handheld "
			"datbase, no data to sync." ) );
		return false; // 6.3.7 and 6.3.8
	}
	
	if( syncMode() == SyncMode::eHotSync )
	{
		fHHDataProxy->setIterateMode( DataProxy::MODIFIED );
		fPCDataProxy->setIterateMode( DataProxy::MODIFIED );
	}
	else
	{
		fHHDataProxy->setIterateMode( DataProxy::ALL );
		fPCDataProxy->setIterateMode( DataProxy::ALL );
	}
	
	return true;
}

bool RecordConduit::askConfirmation(const QString & volatilityMessage)
{
	Q_UNUSED(volatilityMessage);
	return false;
}

void RecordConduit::copyDatabases()
{
}

void RecordConduit::createBackupDatabase() 
{
}

