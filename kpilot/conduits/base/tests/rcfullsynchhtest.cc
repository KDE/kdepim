/* rcfullsynchhtest.cc			KPilot
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

#include "qtest_kde.h"

#include "options.h"
#include "dataproxy.h"
#include "hhdataproxy.h"
#include "idmapping.h"

#include "testrecordconduit.h"
#include "testrecord.h"
#include "testhhrecord.h"

/**
 * This class tests all cases that occur when doing an fullsync. The 
 * conflict resolution used for all testcases is: eHHOverides.
 * 
 * The following table gives the status of the TestRecordConduit before sync.
 *
 * N = new, - = Unchanged, X = no record, D = deleted, M = content modified,
 * A = Archived, ~ = Different from backup but no flags set.
 *
 * NOTE: Records marked as N give true on modified().
 *
 * CASE  |     PC   |M|HH & BACKUP|
 *       |  Id |STAT| |BU|HH| Id  | Resolution for fullsync and hhOverides
 * 6.5.1 |pc-1 |  - |Y| -| -|hh-1 | Do nothing
 * 6.5.2 |     |  X |N| X| N|hh-2 | add to pc
 * 6.5.3 |pc-2 |  - |Y| -| M|hh-3 | sync hh to pc
 * 6.5.4 |pc-3 |  - |Y| -| D|hh-4 | delete from pc
 * 6.5.5 |pc-4 |  N |N| X| X|     | add to hh
 * 6.5.6 |pc-5 |  M |Y| -| -|hh-5 | sync pc to hh
 * 6.5.7 |pc-6 |  D |Y| -| -|hh-6 | delete from hh
 * 6.5.8 |pc-7 |  N |N| X| N|hh-7 | Same data, create mapping
 * 6.5.9 |pc-8 |  M |Y| -| M|hh-8 | Sync hh to pc
 * 6.5.10|pc-9 |  M |Y| -| D|hh-9 | delete from pc
 * 6.5.11|pc-10|  D |Y| -| M|hh-10| Sync hh to pc
 * 6.5.12|pc-11|  D |Y| -| D|hh-11| Delete mapping
 * 6.5.13|     |  X |N| X| -|hh-12| Add to PC, create mapping <-- ONLY FULLSYNC
 * 6.5.14|pc-12|  - |N| X| X|     | Add to HH, create mapping <-- ONLY FULLSYNC
 * 6.5.15|pc-13|  - |Y| -| A|hh-13| Delete record from hh, mark mapping as archived
 * 6.5.16|pc-14|  M |Y| -| A|hh-14| Undo modifications, delete record from hh.
 * 6.5.17|pc-15|  D |A| X| X|hh-15| Delete record from pc and delete mapping
 * 6.5.18|pc-16|  ~ |Y| -| -|hh-16| Copy changes to hh record. <-- ONLY FULLSYNC
 * 6.5.19|pc-17|  - |Y| -| ~|hh-17| Copy changes to pc record. <-- ONLY FULLSYNC
 * 6.5.20|pc-18|  ~ |Y| -| ~|hh-18| Copy changes to pc record. <-- ONLY FULLSYNC
 */
class RCFullSyncHHTest : public QObject
{
	Q_OBJECT

public:
	RCFullSyncHHTest();

private slots:
	void testCUD();
	
	/** FullSync and HH overides tests (See table below) **/
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
	void testCase_6_5_15();
	void testCase_6_5_16();
	void testCase_6_5_17();
	void testCase_6_5_18();
	void testCase_6_5_19();
	void testCase_6_5_20();

private:
	QStringList fFields;
	TestRecordConduit *fConduit;
	
	void initTestCase_1();
	
	TestHHRecord* duplicateHHRecord( const QString &id );
	TestHHRecord* duplicateBackupRecord( const QString &id );
	TestRecord* duplicatePCRecord( const QString &id );
};

RCFullSyncHHTest::RCFullSyncHHTest()
{
	fFields << CSL1( "f1" ) << CSL1( "f2" );
}

void RCFullSyncHHTest::initTestCase_1()
{
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QVariantList args = QVariantList() << CSL1( "--fullsync" )
		<< CSL1( "--conflictResolution 2" );
	
	fConduit = new TestRecordConduit( args, true );
	fConduit->initDataProxies();
	
	// Prints out all records from all datastores..
	//fConduit->test();
}

TestHHRecord* RCFullSyncHHTest::duplicateHHRecord( const QString &id )
{
	Record *rec = fConduit->hhDataProxy()->find( id );
	
	if( rec )
	{
		return static_cast<TestHHRecord*>( rec )->duplicate();
	}
	
	return 0L;
}
TestHHRecord* RCFullSyncHHTest::duplicateBackupRecord( const QString &id )
{
	Record *rec = fConduit->backupDataProxy()->find( id );
	
	if( rec )
	{
		return static_cast<TestHHRecord*>( rec );
	}
	
	return 0L;
}

TestRecord* RCFullSyncHHTest::duplicatePCRecord( const QString &id )
{
	Record *rec = fConduit->pcDataProxy()->find( id );
	
	if( rec )
	{
		return static_cast<TestRecord*>( rec );
	}
	
	return 0L;
}

void RCFullSyncHHTest::testCUD()
{
	// This tests if the CUD counters gets updated correctly.
	initTestCase_1();
	
	const CUDCounter *cudHH = fConduit->hhDataProxy()->counter();
	const CUDCounter *cudPC = fConduit->pcDataProxy()->counter();
	
	int C = cudHH->countCreated();
	int U = cudHH->countUpdated();
	int D = cudHH->countDeleted();
	
	QCOMPARE( C, 0 );
	QCOMPARE( U, 0 );
	QCOMPARE( D, 0 );
	
	fConduit->hotSyncTest();
	
	C = cudHH->countCreated();
	U = cudHH->countUpdated();
	D = cudHH->countDeleted();
	
	QCOMPARE( C, 3 ); // 6.5.{5, 8, 14}
	QCOMPARE( U, 2 ); // 6.5.{6, 18}
	QCOMPARE( D, 6 ); // 6.5.{4, 7, 10, 12, 15, 17}
	
	C = cudPC->countCreated();
	U = cudPC->countUpdated();
	D = cudPC->countDeleted();
	
	QCOMPARE( C, 4 ); // 6.5.{2, 8, 11, 13}
	QCOMPARE( U, 5 ); // 6.5.{3, 9, 16, 19, 20}
	QCOMPARE( D, 6 ); // 6.5.{4, 10, 11, 12, 15, 16}

}

void RCFullSyncHHTest::testCase_6_5_1()
{
	// 6.5.1 | pc-1 |  - ||  - |  - | hh-1 | Check if pc and hh records have changed
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-1" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-1" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-1" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !pcRec->isDeleted() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !pcRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	QVERIFY( fConduit->equal( pcRec, hhRec ) );
	QVERIFY( fConduit->equal( pcRec, backupRec ) );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-1" ) ) == CSL1( "hh-1" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-1" ) ) == CSL1( "pc-1" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-1" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-1" ) ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( syncedPCRec == pcRec );
	QVERIFY( syncedHHRec->equal( hhRec ) );
	
	// There should be a valid mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-1" ) ) == CSL1( "hh-1" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-1" ) ) == CSL1( "pc-1" ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_2()
{
	// 6.5.2 |      |  X ||  X |  N | hh-2 | add to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-2" ) );
	
	// Verify the startsituation
	QVERIFY( hhRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "hh-2" ) ).isEmpty() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-2" ) ) );
	
	QString pcRecId = fConduit->mapping().pcRecordId( CSL1( "hh-2" ) );
	
	QVERIFY( !pcRecId.isEmpty() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecId );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( syncedHHRec->equal( hhRec ) );
	QVERIFY( fConduit->equal( syncedPCRec, hhRec ) );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_3()
{
	//  6.5.3 | pc-2 |  - ||  - |  M | hh-3 | sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-2" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-3" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-3" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( !fConduit->equal( pcRec, hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-3" ) ) == CSL1( "pc-2" ) );
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-2" ) ) == CSL1( "hh-3" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// So... did hotsync do what whe expected it to do?
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-2" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-3" ) ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( fConduit->equal( syncedPCRec, hhRec ) );
	QVERIFY( syncedHHRec->equal( hhRec ) );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( syncedPCRec->id() == pcRec->id() );
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There still should be a correct mapping.
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-2" ) ) == CSL1( "hh-3" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-3" ) ) == CSL1( "pc-2" ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_4()
{
	// 6.5.4 | pc-3 |  - ||  - |  D | hh-4 | delete from pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-3") );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-4" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-4" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-3" ) ) == CSL1( "hh-4" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-4" ) ) == CSL1( "pc-3" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping().pcRecordId( CSL1( "hh-4" ) );
	QString hhRecordId = fConduit->mapping().hhRecordId( CSL1( "pc-3" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-3" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-4" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_5()
{
	// 6.5.5 | pc-4 |  N ||  X |  X |      | add to hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-4" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-4" ) ).isEmpty() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-4" ) );
	
	QString hhRecId = fConduit->mapping().hhRecordId( CSL1( "pc-4" ) );
	
	QVERIFY( !hhRecId.isEmpty() );
	
	HHRecord *syncedHHRec = 
		static_cast<HHRecord*>( fConduit->hhDataProxy()->find( hhRecId ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( fConduit->equal( pcRec, syncedHHRec ) );
	QVERIFY( syncedPCRec == pcRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_6()
{
	// 6.5.6 | pc-5 |  M ||  - |  - | hh-5 | sync pc to hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-5" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-5" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-5" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( !fConduit->equal( pcRec, hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-5" ) ) == CSL1( "hh-5" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-5" ) ) == CSL1( "pc-5" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-5" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-5" ) ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the pc record before sync.
	QVERIFY( fConduit->equal( pcRec, syncedHHRec ) );
	QVERIFY( syncedPCRec == pcRec );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-5" ) ) == CSL1( "hh-5" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-5" ) ) == CSL1( "pc-5" ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_7()
{
	// 6.5.7 | pc-6 |  D ||  - |  - | hh-6 | delete from hh
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-6" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-6" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-6" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-6" ) ) == CSL1( "hh-6" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-6" ) ) == CSL1( "pc-6" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping().pcRecordId( CSL1( "pc-6" ) );
	QString hhRecordId = fConduit->mapping().hhRecordId( CSL1( "hh-6" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// After the sync the records should be really gone from the proxies.
	QVERIFY( !fConduit->pcDataProxy()->find( CSL1( "pc-6" ) ) );
	QVERIFY( !fConduit->hhDataProxy()->find( CSL1( "hh-6" ) ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_9()
{
	// 6.5.9 | pc-8 |  M ||  - |  M | hh-8 | Sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-8" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-8" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-8" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( !fConduit->equal( pcRec, hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-8" ) ) == CSL1( "hh-8" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-8" ) ) == CSL1( "pc-8" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-8" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-8" ) ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Pc and hh record should have the values from the handheld record before 
	// sync.
	QVERIFY( syncedHHRec->equal( hhRec ) );
	QVERIFY( fConduit->equal( syncedPCRec, hhRec ) );
	
	// Records are in sync so shouldn't be modified anymore after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	QVERIFY( !syncedHHRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-8" ) ) == CSL1( "hh-8" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-8" ) ) == CSL1( "pc-8" ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_10()
{
	// 6.5.10| pc-9 |  M ||  - |  D | hh-9 | delete from pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-9" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-9" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-9" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-9" ) ) == CSL1( "hh-9" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-9" ) ) == CSL1( "pc-9" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping().pcRecordId( CSL1( "hh-9" ) );
	QString hhRecordId = fConduit->mapping().hhRecordId( CSL1( "pc-9" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-9" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-9" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_11()
{
	// 6.5.11| pc-10|  D ||  - |  M | hh-10| Sync hh to pc
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-10" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-10" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-10" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-10" ) ) == CSL1( "hh-10" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-10" ) ) == CSL1( "pc-10" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// First determine the new id for the pc record.
	QString pcRecordId = fConduit->mapping().pcRecordId( CSL1( "hh-10" ) );
	
	QVERIFY( !pcRecordId.isNull() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecordId );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-10" ) ) );
	
	QVERIFY( syncedHHRec );
	QVERIFY( syncedPCRec );
	QVERIFY( hhRec->equal( syncedHHRec ) );
	QVERIFY( fConduit->equal( syncedPCRec, hhRec ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_12()
{
	// 6.5.12| pc-11|  D ||  - |  D | hh-11| Delete mapping
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-11" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-11" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-11" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( !backupRec->isModified() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-11" ) ) == CSL1( "hh-11" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-11" ) ) == CSL1( "pc-11" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// Mapping should be gone.
	QString pcRecordId = fConduit->mapping().pcRecordId( CSL1( "hh-11" ) );
	QString hhRecordId = fConduit->mapping().hhRecordId( CSL1( "pc-11" ) );
	
	QVERIFY( pcRecordId.isNull() );
	QVERIFY( hhRecordId.isNull() );
	
	// Records should be gone.
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-11" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-11" ) );
	
	QVERIFY( !syncedHHRec );
	QVERIFY( !syncedPCRec );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_13()
{
	// 6.5.13|      |  X ||  X |  - | hh-12| Add to PC, create mapping <-- ONLY FULLSYNC
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-12" ) );
	
	// Verify the startsituation
	QVERIFY( !hhRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-12" ) ).isEmpty() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	TestHHRecord *syncedHHRec = static_cast<TestHHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-12" ) ) );
	
	QString pcRecId = fConduit->mapping().pcRecordId( CSL1( "hh-12" ) );
	Record *syncedPCRec = fConduit->pcDataProxy()->find( pcRecId );
	
	// Mapping should exist.
	QVERIFY( !pcRecId.isEmpty() );
	// PC record should exist.
	QVERIFY( syncedPCRec );
	
	QVERIFY( fConduit->equal( syncedPCRec, syncedHHRec ) );
	
	// Record should be there.
	QVERIFY( syncedHHRec );
	
	// Pc shouldn't have changed
	QVERIFY( syncedHHRec->equal( hhRec ) );
	
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedHHRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_14()
{
	// 6.5.14| pc-12|  - ||  X |  X |   | Add to HH, create mapping <-- ONLY FULLSYNC
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-12" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	
	// There shouldn't be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-12" ) ).isEmpty() );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-12" ) );
	
	QString hhRecId = fConduit->mapping().hhRecordId( CSL1( "pc-12" ) );
	
	QVERIFY( !hhRecId.isEmpty() );
	TestHHRecord *syncedHHRec = static_cast<TestHHRecord*>(
		fConduit->hhDataProxy()->find( hhRecId ) );
	
	// Records should be there.
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// HH record should be equal to pc record.
	QVERIFY( fConduit->equal( syncedPCRec, syncedHHRec ) );
	
	// Pc shouldn't have changed
	QVERIFY( syncedPCRec == pcRec );
	
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_15()
{
	// 6.5.15|pc-13|  - |Y| -| A|hh-13| Delete record from hh, mark mapping as archived
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-13" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-13" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-13" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( hhRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( hhRec->isArchived() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-13" ) ) == CSL1( "hh-13" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-13" ) ) == CSL1( "pc-13" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-13" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-13" ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-13" ) ) == CSL1( "hh-13" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-13" ) ) == CSL1( "pc-13" ) );
	
	// The pc record should be marked as archived in the mapping
	QVERIFY( fConduit->mapping().isArchivedRecord( CSL1( "pc-13" ) ) );
	
	// Record should be there.
	QVERIFY( syncedPCRec );
	// HH record should be gone
	QVERIFY( !syncedHHRec );
	
	// Pc shouldn't have changed
	QVERIFY( syncedPCRec == pcRec );
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_16()
{
	// 6.5.16|pc-14|  M |Y| -| A|hh-14| Undo modifications, delete record from hh.
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-14" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-14" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-14" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( backupRec->equal( hhRec ) );
	QVERIFY( hhRec->isModified() );
	QVERIFY( hhRec->isDeleted() );
	QVERIFY( hhRec->isArchived() );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-14" ) ) == CSL1( "hh-14" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-14" ) ) == CSL1( "pc-14" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-14" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-14" ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-14" ) ) == CSL1( "hh-14" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-14" ) ) == CSL1( "pc-14" ) );
	
	// The pc record should be marked as archived in the mapping
	QVERIFY( fConduit->mapping().isArchivedRecord( CSL1( "pc-14" ) ) );
	
	// Record should be there.
	QVERIFY( syncedPCRec );
	// HH record should be gone
	QVERIFY( !syncedHHRec );
	
	// Changes to pc record should have been undone.
	QVERIFY( fConduit->equal( syncedPCRec, hhRec ) );
	// Records still shouldn't be modified after a hotsync.
	QVERIFY( !syncedPCRec->isModified() );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_17()
{
	// 6.5.17|pc-15|  D |A| X| X|hh-15| Delete record from pc and delete mapping
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-15" ) );
	
	// Verify the startsituation
	QVERIFY( pcRec->isDeleted() );
	QVERIFY( !fConduit->hhDataProxy()->find( CSL1( "hh-15" ) ) );
	QVERIFY( !fConduit->backupDataProxy()->find( CSL1( "hh-15" ) ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-15" ) ) == CSL1( "hh-15" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-15" ) ) == CSL1( "pc-15" ) );
	// And it should be flagged as archived.
	QVERIFY( fConduit->mapping().isArchivedRecord( CSL1( "pc-15" ) ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// There shouldn't be a mapping anymore
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-15" ) ).isEmpty() );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-15" ) ).isEmpty() );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-15" ) );
	Record *syncedHHRec = fConduit->hhDataProxy()->find( CSL1( "hh-15" ) );
	
	// PC Record should be gone
	QVERIFY( !syncedPCRec );
	// HH record should be gone
	QVERIFY( !syncedHHRec );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_18()
{
	// 6.5.18|pc-16|  ~ |Y| -| -|hh-16| Copy changes to hh record. <-- ONLY FULLSYNC
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-16" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-16" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-16" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !fConduit->equal( pcRec, backupRec ) );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( backupRec->equal( hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-16" ) ) == CSL1( "hh-16" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-16" ) ) == CSL1( "pc-16" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// There should still be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-16" ) ) == CSL1( "hh-16" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-16" ) ) == CSL1( "pc-16" ) );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-16" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-16" ) ) );
	
	// Records should be there
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Changes in pc rec should be on handheld now.
	QVERIFY( pcRec == syncedPCRec );
	QVERIFY( fConduit->equal( syncedPCRec, syncedHHRec ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_19()
{
	// 6.5.19|pc-17|  - |Y| -| ~|hh-17| Copy changes to pc record. <-- ONLY FULLSYNC
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-17" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-17" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-17" ) );
	
	// Verify the startsituation
	QVERIFY( fConduit->equal( pcRec, backupRec ) );
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( !backupRec->equal( hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-17" ) ) == CSL1( "hh-17" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-17" ) ) == CSL1( "pc-17" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// There should still be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-17" ) ) == CSL1( "hh-17" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-17" ) ) == CSL1( "pc-17" ) );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-17" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-17" ) ) );
	
	// Records should be there
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Changes in hh rec should be on pc now.
	QVERIFY( fConduit->equal( syncedPCRec, syncedHHRec ) );
	// This should of course be changes that where made to the hhRec.
	QVERIFY( syncedHHRec->equal( hhRec ) );
	
	delete fConduit;
}

void RCFullSyncHHTest::testCase_6_5_20()
{
	// 6.5.20|pc-18|  ~ |Y| -| ~|hh-18| Copy changes to pc record. <-- ONLY FULLSYNC
	initTestCase_1();
	
	// Duplicate the records before the sync.
	TestRecord *pcRec = duplicatePCRecord( CSL1( "pc-18" ) );
	TestHHRecord *hhRec = duplicateHHRecord( CSL1( "hh-18" ) );
	TestHHRecord *backupRec = duplicateBackupRecord( CSL1( "hh-18" ) );
	
	// Verify the startsituation
	QVERIFY( !pcRec->isModified() );
	QVERIFY( !fConduit->equal( pcRec, backupRec ) );
	QVERIFY( !hhRec->isModified() );
	QVERIFY( !backupRec->isModified() );
	QVERIFY( !backupRec->equal( hhRec ) );
	
	// There should be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-18" ) ) == CSL1( "hh-18" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-18" ) ) == CSL1( "pc-18" ) );
	
	// Everything is ok, do a hotsync now.
	fConduit->hotSyncTest();
	
	// There should still be a mapping
	QVERIFY( fConduit->mapping().hhRecordId( CSL1( "pc-18" ) ) == CSL1( "hh-18" ) );
	QVERIFY( fConduit->mapping().pcRecordId( CSL1( "hh-18" ) ) == CSL1( "pc-18" ) );
	
	Record *syncedPCRec = fConduit->pcDataProxy()->find( CSL1( "pc-18" ) );
	HHRecord *syncedHHRec = static_cast<HHRecord*>(
		fConduit->hhDataProxy()->find( CSL1( "hh-18" ) ) );
	
	// Records should be there
	QVERIFY( syncedPCRec );
	QVERIFY( syncedHHRec );
	
	// Changes in hh rec should be on pc now.
	QVERIFY( fConduit->equal( syncedPCRec, syncedHHRec ) );
	// This should be changes that where made to the hhRec because the conflict
	// resolution.
	QVERIFY( syncedHHRec->equal( hhRec ) );
	
	delete fConduit;
}

QTEST_KDEMAIN(RCFullSyncHHTest, NoGUI)

#include "rcfullsynchhtest.moc"
