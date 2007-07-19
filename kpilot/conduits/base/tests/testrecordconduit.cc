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
#include "testhhdataproxy.h"
#include "idmapping.h"
#include "record.h"
#include "testrecord.h"
#include "testhhrecord.h"

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
		 * CASE  |      PC   |M| HH & BACKUP    |
		 *       |   Id |STAT| | BU | HH | Id   |
		 * 5.5.1 | pc-1 |  - |Y|  - |  - | hh-1 |
		 * 5.5.2 |      |  X |N|  X |  N | hh-2 |
		 * 5.5.3 | pc-2 |  - |Y|  - |  M | hh-3 |
		 * 5.5.4 | pc-3 |  - |Y|  - |  D | hh-4 |
		 * 5.5.5 | pc-4 |  N |N|  X |  X |      |
		 * 5.5.6 | pc-5 |  M |Y|  - |  - | hh-5 |
		 * 5.5.7 | pc-6 |  D |Y|  - |  - | hh-6 |
		 * 5.5.8 | pc-7 |  N |N|  X |  N | hh-7 |
		 * 5.5.9 | pc-8 |  M |Y|  - |  M | hh-8 |
		 * 5.5.10| pc-9 |  M |Y|  - |  D | hh-9 |
		 * 5.5.11| pc-10|  D |Y|  - |  M | hh-10|
		 * 5.5.12| pc-11|  D |Y|  - |  D | hh-11|
		 * 5.5.13|      |  X |N|  X |  - | hh-12|
		 * 5.5.14| pc-12|  - |N|  X |  X |      |
		 * 6.5.15| pc-13|  - |Y|  - |  A | hh-13|
		 * 6.5.16| pc-14|  M |Y|  - |  A | hh-14|
		 * 6.5.17| pc-15|  D |A|  X |  X | hh-15|
		 */
		fHHDataProxy = new TestHHDataProxy( 14 );
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-2" ) ) )->setModified();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-3" ) ) )->setModified();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-4" ) ) )->setDeleted();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-8" ) ) )->setModified();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-9" ) ) )->setDeleted();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-10" ) ) )->setModified();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-11" ) ) )->setDeleted();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-13" ) ) )->setArchived();
		static_cast<TestHHRecord*>( fHHDataProxy->find( CSL1( "hh-14" ) ) )->setArchived();
		
		fBackupDataProxy = new TestHHDataProxy( 14 );
		fBackupDataProxy->remove( CSL1( "hh-2" ) );
		fBackupDataProxy->remove( CSL1( "hh-7" ) );
		fBackupDataProxy->remove( CSL1( "hh-12" ) );
		
		fPCDataProxy = new TestDataProxy( 15, CSL1( "pc-" ), false );
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-4" ) ) )->setModified();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-5" ) ) )->setModified();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-6" ) ) )->setDeleted();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-8" ) ) )->setModified();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-9" ) ) )->setModified();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-10" ) ) )->setDeleted();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-11" ) ) )->setDeleted();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-14" ) ) )->setModified();
		static_cast<TestRecord*>( fPCDataProxy->find( CSL1( "pc-15" ) ) )->setDeleted();
		 
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
		fMapping->map( CSL1( "hh-13" ), CSL1( "pc-13" ) );
		fMapping->map( CSL1( "hh-14" ), CSL1( "pc-14" ) );
		fMapping->map( CSL1( "hh-15" ), CSL1( "pc-15" ) );
		
		fMapping->archiveRecord( CSL1( "hh-15" ) );
	}
	else
	{
		fHHDataProxy = new TestHHDataProxy();
		fBackupDataProxy = new TestHHDataProxy();
		fPCDataProxy = new TestDataProxy();
	}
}

Record* TestRecordConduit::createPCRecord( const HHRecord* record )
{
	return new TestRecord( record );
}

HHRecord* TestRecordConduit::createHHRecord( const Record* record )
{
	return new TestHHRecord( record );
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

void TestRecordConduit::solveConflictTest( Record *pcRecord, HHRecord *hhRecord )
{
	solveConflict( pcRecord, hhRecord );
}

void TestRecordConduit::hotSyncTest()
{
	hotSync();
}
