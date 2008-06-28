/* testrecordconduit.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
** Copyright (C) 2007 by Jason "vanRijn" Kasper <vr@movingparts.net>
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

#include "idmapping.h"
#include "record.h"

#include "testrecordconduit.h"
#include "testdataproxy.h"
#include "testhhdataproxy.h"
#include "testrecord.h"
#include "testhhrecord.h"

#include <QtCore/QDebug>

TestRecordConduit::TestRecordConduit( const QVariantList &args, bool createRecs )
	: RecordConduit( 0L, args, CSL1( "test-db" ), CSL1( "test-conduit" ) )
		,fCreateRecords( createRecs )
{
	
	// Create a mapping, we don't test the exec() function.
	fMapping = new IDMapping( CSL1( "test-user" ), fConduitName );
}

void TestRecordConduit::loadSettings()
{
}
	
bool TestRecordConduit::initDataProxies()
{
	if( fCreateRecords )
	{
		// Create all three proxies and mappings. The backup proxy only contains
		// unmodified records:
		/*
		 * The following table gives the status of the TestRecordConduit before sync.
		 *
		 * CASE  |      PC   |M| HH & BACKUP    | Notes
		 *       |   Id |STAT| | BU | HH | Id   |
		 * 6.5.1 | pc-1 |  - |Y|  - |  - | hh-1 |
		 * 6.5.2 |      |  X |N|  X |  N | hh-2 | HHRecord has category
		 * 6.5.3 | pc-2 |  - |Y|  - |  M | hh-3 |
		 * 6.5.4 | pc-3 |  - |Y|  - |  D | hh-4 |
		 * 6.5.5 | pc-4 |  N |N|  X |  X |      |
		 * 6.5.6 | pc-5 |  M |Y|  - |  - | hh-5 |
		 * 6.5.7 | pc-6 |  D |Y|  - |  - | hh-6 |
		 * 6.5.8 | pc-7 |  N |N|  X |  N | hh-7 |
		 * 6.5.9 | pc-8 |  M |Y|  - |  M | hh-8 |
		 * 6.5.10| pc-9 |  M |Y|  - |  D | hh-9 |
		 * 6.5.11| pc-10|  D |Y|  - |  M | hh-10|
		 * 6.5.12| pc-11|  D |Y|  - |  D | hh-11|
		 * 6.5.13|      |  X |N|  X |  - | hh-12|
		 * 6.5.14| pc-12|  - |N|  X |  X |      |
		 * 6.5.15| pc-13|  - |Y|  - |  A | hh-13|
		 * 6.5.16| pc-14|  M |Y|  - |  A | hh-14|
		 * 6.5.17| pc-15|  D |A|  X |  X | hh-15|
		 * 6.5.18| pc-16|  ~ |Y|  - |  - | hh-16|
		 * 6.5.19| pc-17|  - |Y|  - |  ~ | hh-17|
		 * 6.5.20| pc-18|  ~ |Y|  - |  ~ | hh-18|
		 */
		TestHHDataProxy* fHHDataProxy = new TestHHDataProxy();
		TestHHDataProxy* fBackupDataProxy = new TestHHDataProxy();
		TestDataProxy* fPCDataProxy = new TestDataProxy();
		
		QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
		QString value1;
		QString value2;
		TestHHRecord *hhRec;
		TestHHRecord *backupRec;
		TestRecord *pcRec;
		
		// 6.5.1 | pc-1 |  - |Y|  - |  - | hh-1 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-1" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-1" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		pcRec = new TestRecord( fields, CSL1( "pc-1" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		fMapping->map( CSL1( "hh-1" ), CSL1( "pc-1" ) );
		 
		// 6.5.2 |      |  X |N|  X |  N | hh-2 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-2" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setModified();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		// 6.5.3 | pc-2 |  - |Y|  - |  M | hh-3 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-3" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 + CSL1( "-modified" ) );
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-3" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		pcRec = new TestRecord( fields, CSL1( "pc-2" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		fMapping->map( CSL1( "hh-3" ), CSL1( "pc-2" ) );
		
		// 6.5.4 | pc-3 |  - |Y|  - |  D | hh-4 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-3" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-4" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-4" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setDeleted();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-4" ), CSL1( "pc-3" ) );
		
		// 6.5.5 | pc-4 |  N |N|  X |  X |      |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-4" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		// 6.5.6 | pc-5 |  M |Y|  - |  - | hh-5 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-5" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-5" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		pcRec = new TestRecord( fields, CSL1( "pc-5" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 + CSL1( "-modified" ) );
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		fMapping->map( CSL1( "hh-5" ), CSL1( "pc-5" ) );
		
		// 6.5.7 | pc-6 |  D |Y|  - |  - | hh-6 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-6" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-6" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		pcRec = new TestRecord( fields, CSL1( "pc-6" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->setDeleted();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		fMapping->map( CSL1( "hh-6" ), CSL1( "pc-6" ) );
		
		// 6.5.8 | pc-7 |  N |N|  X |  N | hh-7 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-7" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		pcRec = new TestRecord( fields, CSL1( "pc-7" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		// 6.5.9 | pc-8 |  M |Y|  - |  M | hh-8 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-8" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 + CSL1( "-modified" ) );
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-8" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-8" ) );
		hhRec->setValue( CSL1( "f1" ), value1 + CSL1( "-modified" ) );
		hhRec->setValue( CSL1( "f2" ), value2 );
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-8" ), CSL1( "pc-8" ) );
		
		// 6.5.10| pc-9 |  M |Y|  - |  D | hh-9 |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-9" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 + CSL1( "-modified" ) );
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-9" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-9" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setDeleted();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-9" ), CSL1( "pc-9" ) );
		
		// 6.5.11| pc-10|  D |Y|  - |  M | hh-10|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-10" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->setDeleted();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-10" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-10" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setModified();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-10" ), CSL1( "pc-10" ) );
		
		// 6.5.12| pc-11|  D |Y|  - |  D | hh-11|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-11" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->setDeleted();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-11" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-11" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setDeleted();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-11" ), CSL1( "pc-11" ) );
		
		// 6.5.13|      |  X |N|  X |  - | hh-12|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-12" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		// 6.5.14| pc-12|  - |N|  X |  X |      |
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-12" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		// 6.5.15| pc-13|  - |Y|  - |  A | hh-13|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-13" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-13" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-13" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->setArchived();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-13" ), CSL1( "pc-13" ) );
		
		// 6.5.16| pc-14|  M |Y|  - |  A | hh-14|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-14" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->setModified();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-14" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-14" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		hhRec->setArchived();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-14" ), CSL1( "pc-14" ) );
		
		// 6.5.17| pc-15|  D |A|  X |  X | hh-15|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-15" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->setDeleted();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		fMapping->map( CSL1( "hh-15" ), CSL1( "pc-15" ) );
		fMapping->archiveRecord( CSL1( "hh-15" ) );
		
		// 6.5.18| pc-16|  ~ |Y|  - |  - | hh-16|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-16" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 + CSL1( "-different-but-synced" ) );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-16" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-16" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-16" ), CSL1( "pc-16" ) );
		
		// 6.5.19| pc-17|  - |Y|  - |  ~ | hh-17|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-17" ) );
		pcRec->setValue( CSL1( "f1" ), value1 );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-17" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-17" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 + CSL1( "-different-but-synced" ) );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-17" ), CSL1( "pc-17" ) );
		
		// 6.5.20| pc-18|  ~ |Y|  - |  ~ | hh-18|
		value1 = CSL1( "A value - " ) + QString::number( qrand() );
		value2 = CSL1( "Another value - " ) + QString::number( qrand() );
		
		pcRec = new TestRecord( fields, CSL1( "pc-18" ) );
		pcRec->setValue( CSL1( "f1" ), value1 + CSL1( "-different-but-synced" ) );
		pcRec->setValue( CSL1( "f2" ), value2 );
		pcRec->synced();
		fPCDataProxy->records()->insert( pcRec->id(), pcRec );
		
		backupRec = new TestHHRecord( fields, CSL1( "hh-18" ) );
		backupRec->setValue( CSL1( "f1" ), value1 );
		backupRec->setValue( CSL1( "f2" ), value2 );
		backupRec->synced();
		fBackupDataProxy->records()->insert( backupRec->id(), backupRec );
		
		hhRec = new TestHHRecord( fields, CSL1( "hh-18" ) );
		hhRec->setValue( CSL1( "f1" ), value1 );
		hhRec->setValue( CSL1( "f2" ), value2 + CSL1( "-different-but-synced" ) );
		hhRec->synced();
		fHHDataProxy->records()->insert( hhRec->id(), hhRec );
		
		fMapping->map( CSL1( "hh-18" ), CSL1( "pc-18" ) );
		
		this->fHHDataProxy = fHHDataProxy;
		this->fBackupDataProxy = fBackupDataProxy;
		this->fPCDataProxy = fPCDataProxy;
	}
	else
	{
		fHHDataProxy = new TestHHDataProxy();
		fBackupDataProxy = new TestHHDataProxy();
		fPCDataProxy = new TestDataProxy();
	}
	
	return true;
}

Record* TestRecordConduit::createPCRecord( const HHRecord* record )
{
	const TestHHRecord *hhRec = static_cast<const TestHHRecord*>( record );
	return new TestRecord( hhRec );
}

HHRecord* TestRecordConduit::createHHRecord( const Record* record )
{
	const TestRecord *pcRec = static_cast<const TestRecord*>( record );
	return new TestHHRecord( pcRec );
}

void TestRecordConduit::copy( const Record *from, HHRecord *to )
{
	RecordConduit::copy( from, to );
}

void TestRecordConduit::copy( const HHRecord *from, Record *to  )
{
	RecordConduit::copy( from, to );
}

void TestRecordConduit::_copy( const HHRecord *from, Record *to  )
{
	const TestHHRecord *hhRec = static_cast<const TestHHRecord*>( from );
	TestRecord *pcRec = static_cast<TestRecord*>( to );

	QStringList fields = hhRec->fields();
	QStringListIterator it( fields );
	
	while( it.hasNext() )
	{
		QString field = it.next();
		pcRec->setValue( field, hhRec->value( field ) );
	}
}

void TestRecordConduit::_copy( const Record *from, HHRecord *to )
{
	const TestRecord *pcRec = static_cast<const TestRecord*>( from );
	TestHHRecord *hhRec = static_cast<TestHHRecord*>( to );

	QStringList fields = pcRec->fields();
	QStringListIterator it( fields );
	
	while( it.hasNext() )
	{
		QString field = it.next();
		hhRec->setValue( field, pcRec->value( field ) );
	}
}

bool TestRecordConduit::equal( const Record *pcRecord, const HHRecord *hhRecord 
	) const
{
	// This is possible because of the implementation of equal in TestRecord.
	return pcRecord->equal( hhRecord );
}

HHRecord* TestRecordConduit::newHHRecord( Record *pcRecord )
{
	TestRecord *pcRec = static_cast<TestRecord*>( pcRecord );
	TestHHRecord *hhRec = new TestHHRecord( pcRec );
	
	return hhRec;
}

Record* TestRecordConduit::newPCRecord( HHRecord *hhRecord )
{
	TestHHRecord *hhRec = static_cast<TestHHRecord*>( hhRecord );
	TestRecord *pcRec = new TestRecord( hhRec );
	
	return pcRec;
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

void TestRecordConduit::solveConflictTest( Record *pcRecord, HHRecord *hhRecord )
{
	solveConflict( pcRecord, hhRecord );
}

void TestRecordConduit::hotSyncTest()
{
	hotOrFullSync();
}

void TestRecordConduit::firstSyncTest()
{
	firstSync();
}

void TestRecordConduit::copyHHToPCTest()
{
	copyHHToPC();
}

void TestRecordConduit::copyPCToHHTest()
{
	copyPCToHH();
}

void TestRecordConduit::addHHRecord( HHRecord *rec )
{
	static_cast<TestHHDataProxy*>( fHHDataProxy )->records()
		->insert( rec->id(), rec );
}

void TestRecordConduit::addBackupRecord( HHRecord *rec )
{
	static_cast<TestHHDataProxy*>( fBackupDataProxy )->records()
		->insert( rec->id(), rec );
}

void TestRecordConduit::addPCRecord( Record *rec )
{
	static_cast<TestDataProxy*>( fPCDataProxy )->records()
		->insert( rec->id(), rec );
}
