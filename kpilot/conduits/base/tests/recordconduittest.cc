/* recordconduittest.cc			KPilot
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

#include <QtTest>
#include <QtCore>

#include "qtest_kde.h"

#include "options.h"
#include "dataproxy.h"
#include "idmapping.h"

#include "testrecordconduit.h"
#include "record.h"


class RecordConduitTest : public QObject
{
	Q_OBJECT

	/* possibilities (O = deleted, M = modified, - = unchanged, Does not exist):
	 *
	 * The sync algorithm should be tested for all of the following cases:
	 *
	 * CASE | PC | BU | HH | Conflict
	 *   1  |  - |  - |  - | N
	 *   2  |  - |  - |  M | N
	 *   3  |  - |  - |  0 | N
	 *   4  |  M |  - |  - | N
	 *   5  |  M |  - |  M | Y
	 *   6  |  M |  - |  0 | Y
	 *   7  |  0 |  - |  - | N   The resolution for all this cases is depending on
	 *   8  |  0 |  - |  M | Y   the way of syncing, and the conflict resolution
	 *   9  |  0 |  - |  0 | N   that is used.
	 *  10  |  X |  X |  - | N
	 *  11  |  X |  X |  M | N
	 *  12  |  - |  X |  X | N
	 *  13  |  M |  X |  X | N
	 *
	 */

public:
	RecordConduitTest();

private slots:
	void testSyncfFields();
	
	/** HotSync and HH overides tests (See table below) **/
	void testCase_1_1();
	void testCase_1_2();
	void testCase_1_3();
	void testCase_1_4();
	void testCase_1_5();
	void testCase_1_6();
	void testCase_1_7();
	void testCase_1_8();
	void testCase_1_9();
	void testCase_1_10();
	void testCase_1_11();
	void testCase_1_12();
	void testCase_1_13();

private:
	QStringList fFields;
	TestRecordConduit *fConduit;
	
	void initTestCase_1();
};

RecordConduitTest::RecordConduitTest()
{
	fFields << CSL1( "f1" ) << CSL1( "f2" );
}

void RecordConduitTest::testSyncfFields()
{
	QStringList args = QStringList() << CSL1( "--hotsync" );
		//<< CSL1( "--conflictResolution 1" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	Record *rec1 = new Record( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	Record *rec2 = new Record( fFields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	//                             ( from, to )
	QVERIFY( conduit.syncFieldsTest( rec1, rec2 ) );
	QVERIFY( rec2->value( CSL1( "f1" ) ) == CSL1( "A test value" ) );
	QVERIFY( rec2->value( CSL1( "f2" ) ) == CSL1( "Another test value" ) );
	
	// Make a record with other fFields
	QStringList fFields2 = QStringList() << CSL1( "afield" ) << CSL1( "f2" );
	
	Record *rec3 = new Record( fFields2 );
	rec3->setValue( CSL1( "afield" ), CSL1( "Test 3-1" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Test 3-2" ) );
	
	// Then try to sync, because not all fFields are the same the to record should
	// remain unmodified.
	//                              ( from, to )
	QVERIFY( !conduit.syncFieldsTest( rec1, rec3 ) );
	QVERIFY( rec3->value( CSL1( "afield" ) ) == CSL1( "Test 3-1" ) );
	QVERIFY( rec3->value( CSL1( "f2" ) ) == CSL1( "Test 3-2" ) );
}

/*
 * The following table gives the status of the TestRecordConduit befor sync.
 *
 *CASE|      PC   || HH & BACKUP    |
 *    |   Id |STAT|| BU | HH | Id   | Resolution for hotSync and hhOverides
 *  1 | id-2 |  - ||  - |  M | id-1 | sync hh to pc
 *  2 | id-4 |  - ||  - |  - | id-2 | Do nothing
 *  3 | id-1 |  M ||  - |  M | id-3 | sync hh to pc
 *  4 | id-3 |  M ||  - |  - | id-4 | Sync pc to hh
 *  5 | D-1  |  0 ||  - |  M | id-5 | sync hh to pc
 *  6 | D-2  |  0 ||  - |  - | id-6 | Delete hh record and mapping.
 *  7 | D-3  |  0 ||  - |  0 | D-1  | Mapping D1-D3 should be removed after sync.
 *  8 | id-6 |  - ||  - |  0 | D-2  | Delete pc record and mapping.
 *  9 | id-5 |  M ||  - |  0 | D-3  | Delete pc record and mapping.
 * 10 | id-7 |  M ||  X |  X |      | Sync pc to hh
 * 11 | id-8 |  - ||  X |  X |      | Do nothing, only full sync does this.
 * 12 |      |  X ||  X |  M | id-7 | sync hh to pc
 * 13 |      |  X ||  X |  - | id-8 | Do nothing, only full sync does this.
 */

void RecordConduitTest::initTestCase_1()
{
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	fConduit = new TestRecordConduit( args, true );
	fConduit->initDataProxies();
	
	// Set the right iteration mode for a hotsync.
	fConduit->pcDataProxy()->setIterateMode( DataProxy::Modified );
	fConduit->hhDataProxy()->setIterateMode( DataProxy::Modified );
	
	// Prints out all records from all datastores..
	//fConduit->test();
}

void RecordConduitTest::testCase_1_1()
{
	//  1 | id-2 |  - ||  - |  M | id-1 | sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "id-2" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-1" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-1" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-2" ) ) == CSL1( "id-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-1" ) ) == CSL1( "id-2" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// So... did hotsync do what whe expected it to do?
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "id-2" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "id-1" ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( *syncedPCRec == *hhRec );
	QVERIFY( *syncedHHRec == *hhRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( syncedPCRec->id() == pcRec->id() );
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There still should be a correct mapping.
	qDebug() << fConduit->mapping()->hhRecordId( CSL1( "id-2" ) );
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-2" ) ) == CSL1( "id-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-1" ) ) == CSL1( "id-2" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_1_2()
{
	// 2 | id-4 |  - ||  - |  - | id-2 | Do nothing
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "id-4" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-2" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-2" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-2" ) ) == CSL1( "id-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-1" ) ) == CSL1( "id-2" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "id-4" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "id-2" ) );
	Record *syncedBackupRec = fConduit->backupDataProxy()->find( CSL1( "id-2" ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	QVERIFY( syncedBackupRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( *syncedPCRec == *pcRec );
	QVERIFY( *syncedHHRec == *hhRec );
	QVERIFY( *syncedPCRec != *syncedHHRec );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-2" ) ) == CSL1( "id-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-1" ) ) == CSL1( "id-2" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_1_3()
{
	// 3 | id-1 |  M ||  - |  M | id-3 | sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "id-1" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-3" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-3" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-1" ) ) == CSL1( "id-3" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-3" ) ) == CSL1( "id-1" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "id-1" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "id-3" ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( *syncedHHRec == *hhRec );
	QVERIFY( *syncedPCRec == *hhRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-1" ) ) == CSL1( "id-3" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-3" ) ) == CSL1( "id-1" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_1_4()
{
	// 4 | id-3 |  M ||  - |  - | id-4 | Sync pc to hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "id-3" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-4" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-4" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-3" ) ) == CSL1( "id-4" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-4" ) ) == CSL1( "id-3" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "id-3" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "id-4" ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( *syncedHHRec == *pcRec );
	QVERIFY( *syncedPCRec == *pcRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-3" ) ) == CSL1( "id-4" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-4" ) ) == CSL1( "id-3" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_1_5()
{
	// 5 | D-1  |  0 ||  - |  M | id-5 | sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-5" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-5" )) ) );
	
	// Verify the startsituation
	QVERIFY( !fConduit->pcDataProxy()->find( CSL1( "D-1" ) ) );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "D-1" ) ) == CSL1( "id-5" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-5" ) ) == CSL1( "D-1" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "id-5" ) );
	
	QVERIFY( !pcRecordId.isNull() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecordId );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "id-5" ) );
	
	QVERIFY( syncedHHRec );
	QVERIFY( syncedPCRec );
	QVERIFY( *hhRec == *syncedHHRec );
	QVERIFY( *hhRec == *syncedPCRec );
	
	delete fConduit;
}

void RecordConduitTest::testCase_1_6()
{
	// D-2  |  0 ||  - |  - | id-6 | Delete hh record and mapping.
	/*
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "id-6" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "id-6" )) ) );
	
	// Verify the startsituation
	QVERIFY( !fConduit->pcDataProxy()->find( CSL1( "D-2" ) ) );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "D-2" ) ) == CSL1( "id-6" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-6" ) ) == CSL1( "D-2" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "id-6" ) );
	
	QVERIFY( !pcRecordId.isNull() );
	
	QVERIFY( !fConduit->pcDataProxy()->find( CSL1( "D-2" ) ) );
	QVERIFY( !fConduit->hhDataProxy()->find( CSL1( "id-6" ) ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "D-2" ) ) == QString() );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "id-6" ) ) == QString() );
	
	delete fConduit;
	*/
}

void RecordConduitTest::testCase_1_7()
{
	// 7 | D-3  |  0 ||  - |  0 | D-1  | Mapping D1-D3 should be removed after sync.
}

void RecordConduitTest::testCase_1_8()
{
	// 8 | id-6 |  - ||  - |  0 | D-2  | Delete pc record and mapping.
}

void RecordConduitTest::testCase_1_9()
{
	//9 | id-5 |  M ||  - |  0 | D-3  | Delete pc record and mapping.
}

void RecordConduitTest::testCase_1_10()
{
	// 10 | id-7 |  M ||  X |  X |      | Sync pc to hh
	/*
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "id-7" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "id-7" ) ) == QString() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "id-7" ) );
	
	QString hhRecId = fConduit->mapping()->hhRecordId( CSL1( "id-7" ) );
	
	QVERIFY( hhRecId != QString() );
	
	Record *syncedHHRec = fConduit->hhDataProxy()->find( hhRecId );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( *syncedHHRec == *pcRec );
	QVERIFY( *syncedPCRec == *pcRec );
	
	qDebug() << syncedPCRec->toString();
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
	*/
}

void RecordConduitTest::testCase_1_11()
{
	// 11 | id-8 |  - ||  X |  X |      | Do nothing, only full sync does this.
}

void RecordConduitTest::testCase_1_12()
{
	// 12 |      |  X ||  X |  M | id-7 | sync hh to pc
}

void RecordConduitTest::testCase_1_13()
{
	// 13 |      |  X ||  X |  - | id-8 | Do nothing, only full sync does this.
}


QTEST_KDEMAIN(RecordConduitTest, NoGUI)

#include "recordconduittest.moc"
