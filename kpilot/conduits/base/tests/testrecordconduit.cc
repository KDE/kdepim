/* testrecordconduit.cc			KPilot
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

#include <options.h>

#include "testrecordconduit.h"
#include "testdataproxy.h"
#include "idmapping.h"

TestRecordConduit::TestRecordConduit( const QStringList &args, bool createRecs )
	: RecordConduit( 0L, args, CSL1( "test-db" ), CSL1( "test-conduit" ) )
		,fCreateRecords( createRecs )
{
	
	// Create a mapping, we don't test the exec() function.
	fMapping = new IDMapping( CSL1( "test-user" ), fConduitName );
}

TestRecordConduit::~TestRecordConduit()
{
	delete fMapping;
}

void TestRecordConduit::loadSettings()
{
}
	
void TestRecordConduit::initDataProxies()
{
	if( fCreateRecords )
	{
		// Create all three proxies. The backup proxy only contains
		// unmodified records:
		/*
		 *    Id | PC || BU | HH |
		 *  id-1 |  M ||  - | M  |
		 *  id-2 |  - ||  - | -  |
		 *  id-3 |  M ||  - | M  |
		 *  id-4 |  - ||  - | -  |
		 *  id-5 |  M ||  - | M  |
		 *  id-6 |  - ||  - | -  |
		 *  id-7 |  M ||  X | M  |
		 *  id-8 |  - ||  X | -  |
		 */
		fHHDataProxy = new TestDataProxy( 6 );
		fBackupDataProxy = new TestDataProxy( 6, false );
		fPCDataProxy = new TestDataProxy( 7 );
		 
		/*
		 * We want to map them as follows:
		 *
		 *    Id | PC || BU | HH | Id
		 *  id-2 |  - ||  - | M  | id-1 -> sync hh to pc
		 *  id-4 |  - ||  - | -  | id-2 -> Do nothing
		 *  id-1 |  M ||  - | M  | id-3 -> Conflict
		 *  id-3 |  M ||  - | -  | id-4 -> Sync pc to hh
		 */
		//                     HH    ,         PC
		fMapping->map( CSL1( "id-1" ), CSL1( "id-2" ) );
		fMapping->map( CSL1( "id-2" ), CSL1( "id-4" ) );
		fMapping->map( CSL1( "id-3" ), CSL1( "id-1" ) );
		fMapping->map( CSL1( "id-4" ), CSL1( "id-3" ) );
		
		/* We also want some conflict situations with deleted records:
		 *   Id  | PC || BU | HH | Id
		 *  D-1  |  0 ||  - |  - | id-6 -> Delete hh-record and mapping.
	   *  D-2  |  0 ||  - |  M | id-5 -> Add duplicate of hh rec to pc datastore.
	   *  D-3  |  0 ||  - |  0 | D-1  -> Not a conflict (Mapping D1-D3 should be 
	   *                                 removed after sync
	   *  id-6 |  - ||  - |  0 | D-2  -> Add duplicate of hh rec to pc datastore.
	   *  id-5 |  M ||  - |  0 | D-3  -> Add duplicate of hh rec to pc datastore.
	   */
		fMapping->map( CSL1( "id-5" ), CSL1( "D-1" ) );
		fMapping->map( CSL1( "id-6" ), CSL1( "D-2" ) );
		
		/*
		fMapping->map( CSL1( "D-1" ), CSL1( "D-3" ) );
		fMapping->map( CSL1( "D-2" ), CSL1( "id-6" ) );
		fMapping->map( CSL1( "D-3" ), CSL1( "id-5" ) );
		*/
		// For records with ids 7 and 8 we don't have to create mappings.
		
		// Now we have records and mappings for every possible situation.
	}
	else
	{
		fHHDataProxy = new TestDataProxy();
		fBackupDataProxy = new TestDataProxy();
		fPCDataProxy = new TestDataProxy();
	}
}

void TestRecordConduit::test()
{
	qDebug() << "************** HANDHELD ******************";
	((TestDataProxy*)fHHDataProxy)->printRecords();
	qDebug() << "**************  BACKUP  ******************";
	((TestDataProxy*)fBackupDataProxy)->printRecords();
	qDebug() << "**************    PC    ******************";
	((TestDataProxy*)fPCDataProxy)->printRecords();
}

bool TestRecordConduit::syncFieldsTest( Record *from, Record *to )
{
	return syncFields( from, to );
}

void TestRecordConduit::solveConflictTest( Record *pcRecord, Record *hhRecord )
{
	solveConflict( pcRecord, hhRecord );
}

void TestRecordConduit::hotSyncTest()
{
	hotSync();
}
