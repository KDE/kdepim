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
#include "record.h"

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
		// Create all three proxies and mappings. The backup proxy only contains
		// unmodified records:
		/*
		 * The following table gives the status of the TestRecordConduit befor sync.
		 *
		 * CASE  |      PC   || HH & BACKUP    |
		 *       |   Id |STAT|| BU | HH | Id   |
		 * 5.5.1 | pc-1 |  - ||  - |  - | hh-1 |
		 * 5.5.2 |      |  X ||  X |  N | hh-2 |
		 * 5.5.3 | pc-2 |  - ||  - |  M | hh-3 |
		 * 5.5.4 | pc-3 |  - ||  - |  D | hh-4 |
		 * 5.5.5 | pc-4 |  N ||  X |  X |      |
		 * 5.5.6 | pc-5 |  M ||  - |  - | hh-5 |
		 * 5.5.7 | pc-6 |  D ||  - |  - | hh-6 |
		 * 5.5.8 | pc-7 |  N ||  X |  N | hh-7 |
		 * 5.5.9 | pc-8 |  M ||  - |  M | hh-8 |
		 * 5.5.10| pc-9 |  M ||  - |  D | hh-9 |
		 * 5.5.11| pc-10|  D ||  - |  M | hh-10|
		 * 5.5.12| pc-11|  D ||  - |  D | hh-11|
		 * 5.5.13|      |  X ||  X |  - | hh-12|
		 * 5.5.14| pc-12|  - ||  X |  X |      |
		 */
		fHHDataProxy = new TestDataProxy( 12, CSL1( "hh-" ) );
		fHHDataProxy->find( CSL1( "hh-2" ) )->setModified();
		fHHDataProxy->find( CSL1( "hh-3" ) )->setModified();
		fHHDataProxy->find( CSL1( "hh-4" ) )->setDeleted();
		fHHDataProxy->find( CSL1( "hh-8" ) )->setModified();
		fHHDataProxy->find( CSL1( "hh-9" ) )->setDeleted();
		fHHDataProxy->find( CSL1( "hh-10" ) )->setModified();
		fHHDataProxy->find( CSL1( "hh-11" ) )->setDeleted();
		
		fBackupDataProxy = new TestDataProxy( 11, CSL1( "hh-" ) );
		fBackupDataProxy->remove( CSL1( "hh-2" ) );
		fBackupDataProxy->remove( CSL1( "hh-7" ) );
		
		fPCDataProxy = new TestDataProxy( 12, CSL1( "pc-" ) );
		fPCDataProxy->find( CSL1( "pc-4" ) )->setModified();
		fPCDataProxy->find( CSL1( "pc-5" ) )->setModified();
		fPCDataProxy->find( CSL1( "pc-6" ) )->setDeleted();
		fPCDataProxy->find( CSL1( "pc-8" ) )->setModified();
		fPCDataProxy->find( CSL1( "pc-9" ) )->setModified();
		fPCDataProxy->find( CSL1( "pc-10" ) )->setDeleted();
		fPCDataProxy->find( CSL1( "pc-11" ) )->setDeleted();
		 
		fMapping->map( CSL1( "hh-1" ), CSL1( "pc-1" ) );
		fMapping->map( CSL1( "hh-3" ), CSL1( "pc-2" ) );
		fMapping->map( CSL1( "hh-4" ), CSL1( "pc-3" ) );
		fMapping->map( CSL1( "hh-5" ), CSL1( "pc-5" ) );
		fMapping->map( CSL1( "hh-6" ), CSL1( "pc-6" ) );
		fMapping->map( CSL1( "hh-7" ), CSL1( "pc-7" ) );
		fMapping->map( CSL1( "hh-8" ), CSL1( "pc-8" ) );
		fMapping->map( CSL1( "hh-9" ), CSL1( "pc-9" ) );
		fMapping->map( CSL1( "hh-10" ), CSL1( "pc-10" ) );
		fMapping->map( CSL1( "hh-11" ), CSL1( "pc-11" ) );
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
