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

public:
	RecordConduitTest();

private slots:
	void testSyncfFields();
	
	/** HotSync and HH overides tests (See table below) **/
	void testCase_6_5_1();
	void testCase_6_5_2();
	void testCase_6_5_3();
	void testCase_6_5_4();
	void testCase_6_5_5();
	void testCase_6_5_6();
	void testCase_6_5_7();
	void testCase_6_5_9();
	void testCase_6_5_10();
	void testCase_6_5_11();
	void testCase_6_5_12();
	void testCase_6_5_13();
	void testCase_6_5_14();

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
 * NOTE: Records marked as N give true on modified().
 *
 * CASE  |      PC   || HH & BACKUP    |
 *       |   Id |STAT|| BU | HH | Id   | Resolution for hotSync and hhOverides
 * 6.5.1 | pc-1 |  - ||  - |  - | hh-1 | Do nothing
 * 6.5.2 |      |  X ||  X |  N | hh-2 | add to pc
 * 6.5.3 | pc-2 |  - ||  - |  M | hh-3 | sync hh to pc
 * 6.5.4 | pc-3 |  - ||  - |  D | hh-4 | delete from pc
 * 6.5.5 | pc-4 |  N ||  X |  X |      | add to hh
 * 6.5.6 | pc-5 |  M ||  - |  - | hh-5 | sync pc to hh
 * 6.5.7 | pc-6 |  D ||  - |  - | hh-6 | delete from hh
 * 6.5.8 | pc-7 |  N ||  X |  N | hh-7 | Same data, create mapping
 * 6.5.9 | pc-8 |  M ||  - |  M | hh-8 | Sync hh to pc
 * 6.5.10| pc-9 |  M ||  - |  D | hh-9 | delete from pc
 * 6.5.11| pc-10|  D ||  - |  M | hh-10| Sync hh to pc
 * 6.5.12| pc-11|  D ||  - |  D | hh-11| Delete mapping
 * 6.5.13|      |  X ||  X |  - | hh-12| Do nothing
 * 6.5.14| pc-12|  - ||  X |  X |      | Do nothing
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

void RecordConduitTest::testCase_6_5_1()
{
	// 6.5.1 | pc-1 |  - ||  - |  - | hh-1 | Do nothing
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-1" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-1" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-1" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-1" ) ) == CSL1( "hh-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-1" ) ) == CSL1( "pc-1" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-1" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-1" ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( *syncedPCRec == *pcRec );
	QVERIFY( *syncedHHRec == *hhRec );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-1" ) ) == CSL1( "hh-1" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-1" ) ) == CSL1( "pc-1" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_2()
{
	// 6.5.2 |      |  X ||  X |  N | hh-2 | add to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-2" )) ) );
	
	// Verify the startsituation
	QVERIFY( hhRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "hh-2" ) ) == QString() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-2" ) );
	
	QString pcRecId = fConduit->mapping()->pcRecordId( CSL1( "hh-2" ) );
	
	QVERIFY( pcRecId != QString() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecId );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( *syncedHHRec == *hhRec );
	QVERIFY( *syncedPCRec == *hhRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_3()
{
	//  6.5.3 | pc-2 |  - ||  - |  M | hh-3 | sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-2" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-3" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-3" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-3" ) ) == CSL1( "pc-2" ) );
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-2" ) ) == CSL1( "hh-3" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// So... did hotsync do what whe expected it to do?
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-2" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-3" ) );
	
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
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-2" ) ) == CSL1( "hh-3" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-3" ) ) == CSL1( "pc-2" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_4()
{
	// 6.5.4 | pc-3 |  - ||  - |  D | hh-4 | delete from pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-3" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-4" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-4" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-3" ) ) == CSL1( "hh-4" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-4" ) ) == CSL1( "pc-3" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "hh-4" ) );
	QString hhRecordId = fConduit->mapping()->hhRecordId( CSL1( "pc-3" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-3" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-4" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_5()
{
	// 6.5.5 | pc-4 |  N ||  X |  X |      | add to hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-4" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-4" ) ) == QString() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-4" ) );
	
	QString hhRecId = fConduit->mapping()->hhRecordId( CSL1( "pc-4" ) );
	
	QVERIFY( hhRecId != QString() );
	
	Record *syncedHHRec = fConduit->hhDataProxy()->find( hhRecId );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( *syncedHHRec == *pcRec );
	QVERIFY( *syncedPCRec == *pcRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_6()
{
	// 6.5.6 | pc-5 |  M ||  - |  - | hh-5 | sync pc to hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-5" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-5" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-5" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-5" ) ) == CSL1( "hh-5" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-5" ) ) == CSL1( "pc-5" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-5" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-5" ) );
	
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
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-5" ) ) == CSL1( "hh-5" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-5" ) ) == CSL1( "pc-5" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_7()
{
	// 6.5.7 | pc-6 |  D ||  - |  - | hh-6 | delete from hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-6" ) ) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-6" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-6" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-6" ) ) == CSL1( "hh-6" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-6" ) ) == CSL1( "pc-6" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "pc-6" ) );
	QString hhRecordId = fConduit->mapping()->hhRecordId( CSL1( "hh-6" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// After the sync the records should be realy gone from the proxies.
	QVERIFY( !fConduit->pcDataProxy()->find( CSL1( "pc-6" ) ) );
	QVERIFY( !fConduit->hhDataProxy()->find( CSL1( "hh-6" ) ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_9()
{
	// 6.5.9 | pc-8 |  M ||  - |  M | hh-8 | Sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-8" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-8" )) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-8" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( *pcRec != *hhRec );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-8" ) ) == CSL1( "hh-8" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-8" ) ) == CSL1( "pc-8" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-8" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-8" ) );
	
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
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-8" ) ) == CSL1( "hh-8" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-8" ) ) == CSL1( "pc-8" ) );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_10()
{
	// 6.5.10| pc-9 |  M ||  - |  D | hh-9 | delete from pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-9" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-9" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-9" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-9" ) ) == CSL1( "hh-9" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-9" ) ) == CSL1( "pc-9" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "hh-9" ) );
	QString hhRecordId = fConduit->mapping()->hhRecordId( CSL1( "pc-9" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-9" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-9" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_11()
{
	// 6.5.11| pc-10|  D ||  - |  M | hh-10| Sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-10" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-10" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-10" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-10" ) ) == CSL1( "hh-10" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-10" ) ) == CSL1( "pc-10" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "hh-10" ) );
	
	QVERIFY( !pcRecordId.isNull() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecordId );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-10" ) );
	
	QVERIFY( syncedHHRec );
	QVERIFY( syncedPCRec );
	QVERIFY( *hhRec == *syncedHHRec );
	QVERIFY( *hhRec == *syncedPCRec );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_12()
{
	// 6.5.12| pc-11|  D ||  - |  D | hh-11| Delete mapping
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-11" )) ) );
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-11" ) ) ) );
	Record *backupRec = new Record( 
		*(fConduit->backupDataProxy()->find( CSL1( "hh-11" )) ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-11" ) ) == CSL1( "hh-11" ) );
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-11" ) ) == CSL1( "pc-11" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping()->pcRecordId( CSL1( "hh-11" ) );
	QString hhRecordId = fConduit->mapping()->hhRecordId( CSL1( "pc-11" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-11" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-11" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_13()
{
	// 6.5.13|      |  X ||  X |  - | hh-12| Do nothing
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *hhRec = new Record( *(fConduit->hhDataProxy()->find( CSL1( "hh-12" )) ) );
	
	// Verify the startsituation
	QVERIFY( !hhRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping()->pcRecordId( CSL1( "hh-12" ) ) == QString() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-12" ) );
	
	QString pcRecId = fConduit->mapping()->pcRecordId( CSL1( "hh-12" ) );
	
	QVERIFY( pcRecId == QString() );
	
	// Record should be there.
	QVERIFY( syncedHHRec );
	
	// Pc shouldn't have changed
	QVERIFY( *syncedHHRec == *hhRec );
	
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RecordConduitTest::testCase_6_5_14()
{
	// 6.5.14| pc-12|  - ||  X |  X |   | Do nothing
	initTestCase_1();
	
	// Duplicate the records before the sync.
	Record *pcRec = new Record( *(fConduit->pcDataProxy()->find( CSL1( "pc-12" )) ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping()->hhRecordId( CSL1( "pc-12" ) ) == QString() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-12" ) );
	
	QString hhRecId = fConduit->mapping()->hhRecordId( CSL1( "pc-12" ) );
	
	QVERIFY( hhRecId == QString() );
	
	// Record should be there.
	QVERIFY( syncedPCRec );
	
	// Pc shouldn't have changed
	QVERIFY( *syncedPCRec == *pcRec );
	
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	
	delete fConduit;
}

QTEST_KDEMAIN(RecordConduitTest, NoGUI)

#include "recordconduittest.moc"
