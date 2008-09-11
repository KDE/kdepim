/* categoryhotsynctest.cc			KPilot
**
** Copyright (C) 2007 by Bertjan Broeksema <b.broeksema@kdemail.net>
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

#include "record.h"
#include "hhrecord.h"
#include "idmapping.h"

#include "testrecordconduit.h"
#include "testhhrecord.h"
#include "testrecord.h"

/**
 * Testcases:
 *
 * CASE  |     PC   |M|HH & BACKUP|
 *       |  Id |STAT| |BU|HH| Id  | Resolution for hotSync and hhOverides
 * ******|*****|****|*|**|**|*****|*********************************************
 * 6.5.2 |     |  X |N| X| N|hh-2 | add to pc
 * 6.5.3 |pc-2 |  - |Y| -| M|hh-3 | sync hh to pc
 *
 ********* Needs attention, for now just a copy from rchotsynctest ************
 *
 * 6.5.4 |pc-3 |  - |Y| -| D|hh-4 | delete from pc
 * 6.5.5 |pc-4 |  N |N| X| X|     | add to hh
 * 6.5.6 |pc-5 |  M |Y| -| -|hh-5 | sync pc to hh
 * 6.5.7 |pc-6 |  D |Y| -| -|hh-6 | delete from hh
 * 6.5.8 |pc-7 |  N |N| X| N|hh-7 | Same data, do not find match, add both records.
 * 6.5.9 |pc-8 |  M |Y| -| M|hh-8 | Sync hh to pc
 * 6.5.10|pc-9 |  M |Y| -| D|hh-9 | delete from pc
 * 6.5.11|pc-10|  D |Y| -| M|hh-10| Sync hh to pc
 * 6.5.12|pc-11|  D |Y| -| D|hh-11| Delete mapping
 * 6.5.13|     |  X |N| X| -|hh-12| Do nothing
 * 6.5.14|pc-12|  - |N| X| X|     | Do nothing
 * 6.5.15|pc-13|  - |Y| -| A|hh-13| Delete record from hh, mark mapping as archived
 * 6.5.16|pc-14|  M |Y| -| A|hh-14| Undo modifications, delete record from hh.
 * 6.5.17|pc-15|  D |A| X| X|hh-15| Delete record from pc and delete mapping
 * 6.5.18|pc-16|  ~ |Y| -| -|hh-16| Do nothing <-- ONLY FULLSYNC
 * 6.5.19|pc-17|  - |Y| -| ~|hh-17| Do nothing <-- ONLY FULLSYNC
 * 6.5.20|pc-18|  ~ |Y| -| ~|hh-18| Do nothing <-- ONLY FULLSYNC
 */
class CategoryHotSyncTest : public QObject
{
	Q_OBJECT

private slots:
	void testCase_6_5_2();
	void testCase_6_5_3a();
	//void testCase_6_5_3b();
	
	void cleanupTestCase();

private:
	TestRecordConduit *fConduit;
	
	void initTestCase();
	
	/** These methods set up the preconditions for the tests. */
	void initTestCase_652();
	void initTestCase_653a();
	//void initTestCase_653b();
};

/**
 * Because of the fact that a record on the handheld only supports one category,
 * we only have to test this one case. Whether a record on the pc supports one or
 * more categories does not matter here.
 */
void CategoryHotSyncTest::testCase_6_5_2()
{
	// Set up the conduit and the dataproxies.
	initTestCase_652();
	
	QString hhId( "hh-2" );
	
	// Preconditions:
	// - The Handheld record should have a category.
	// - The Handheld record should (always) have exactly one category.
	Record* hhRec = fConduit->hhDataProxy()->records()->value( hhId );
	
	QVERIFY( !hhRec->categories().isEmpty() );
	QCOMPARE( hhRec->categories().size(), 1 );
	
	QString hhCat = hhRec->categories().first();
	// Sync
	fConduit->hotSyncTest();
	
	// Postconditions relevant for category syncing.
	// - The record in the pc datastore that maps to the hh record should have the
	//   right Category.
	QString pcId = fConduit->mapping().pcRecordId( hhId );
	
	Record *pcRec = fConduit->pcDataProxy()->records()->value( pcId );
	
	
	// Warning I'm not sure if test will pass because hhCat is a pointer. The
	// category class contains a ==( const Category *other) but i'm not sure if
	// that's enough to make the test working as done below.
	QVERIFY( pcRec->categories().contains( hhCat ) );
}

/**
 * 6.5.3 |pc-2 |  - |Y| -| M|hh-3 | sync hh to pc
 *
 * Case 6.5.3 can be split up in several options. If the category has changed on
 * the handheld it can be that:
 *
 * a) No category was set on the pc -> set the category to the pc record.
 * b) The pc record has one category and supports only one category -> replace
 *    current category.
 * c) The pc record has one (or more categories) and the category from the hh
 *    is not yet in it -> Add the categorie to the pc record.
 * d) The record has zero or more categories set but does not contain the
 *    categorie on the handheld and the proxy fails to set the category to the
 *    pc record -> pc record should stay unchanged and the mapping should
 *    contain the categories of both.
 * e) Category of the hh record changed to some category that is already set on
 *    the pc record -> update the mapping.
 */
void CategoryHotSyncTest::testCase_6_5_3a()
{
	// Set up the test case
	initTestCase_653a();
	
	// Preconditions:
	// - The handheld record is modified and has a category set.
	// - The backup record does have the "Unfiled" category set and only supports
	//   one category.
	// - The pc record has no category but supports more then one category.
	// - There is a mapping for the records.
	QString hhId( "hh-3" );
	QString pcId( "pc-2" );
	
	Record *hhRec = fConduit->hhDataProxy()->records()->value( hhId );
	Record *backupRec = fConduit->backupDataProxy()->records()->value( hhId );
	Record *pcRec = fConduit->pcDataProxy()->records()->value( pcId );
	
	QVERIFY( hhRec->isModified() );
	QCOMPARE( hhRec->categories().size(), 1 );
	QVERIFY( hhRec->categories().first() != QString( "Unfiled") );
	
	QVERIFY( !backupRec->isModified() );
	QCOMPARE( backupRec->categories().size(), 1 );
	QCOMPARE( backupRec->categories().first(), QString( "Unfiled") );
	
	QVERIFY( !pcRec->isModified() );
	QCOMPARE( pcRec->categories().size(), 0 );
	
	QCOMPARE( fConduit->mapping().pcRecordId( hhId ), pcId );
	
	// Sync
	fConduit->hotSyncTest();
	
	// Postconditions
	hhRec = fConduit->hhDataProxy()->records()->value( hhId );
	backupRec = fConduit->backupDataProxy()->records()->value( hhId );
	pcRec = fConduit->pcDataProxy()->records()->value( pcId );
	
	QCOMPARE( pcRec->categories().size(), 1 );
	QCOMPARE( pcRec->categories().first(), hhRec->categories().first() );
	//QCOMPARE( backupRec->categories().first(), hhRec->categories().first() );
}

/*
void CategoryHotSyncTest::testCase_6_5_3b()
{
	// Set up the test case
	initTestCase_653b();
	
	// Preconditions:
	// - The handheld record is modified and has a category, and supports only one
	//   category.
	// - The backup record does have the "Unfiled" category set and only supports
	//   one category.
	// - The pc record has no category but supports only one category.
	// - There is a mapping for the records.
	QString hhId( "hh-3" );
	QString pcId( "pc-2" );
	
	Record *hhRec = fConduit->hhDataProxy()->records()->value( hhId );
	Record *backupRec = fConduit->backupDataProxy()->records()->value( hhId );
	Record *pcRec = fConduit->pcDataProxy()->records()->value( pcId );
	
	QVERIFY( hhRec->isModified() );
	QCOMPARE( hhRec->categories().size(), 1 );
	QVERIFY( hhRec->categories().first()->name() != QString( "Unfiled") );
	QVERIFY( !hhRec->supportsMultipleCategories() );
	
	QVERIFY( !backupRec->isModified() );
	QCOMPARE( backupRec->categories().size(), 1 );
	QCOMPARE( backupRec->categories().first()->name(), QString( "Unfiled") );
	QVERIFY( !backupRec->supportsMultipleCategories() );
	
	QVERIFY( !pcRec->isModified() );
	QCOMPARE( pcRec->categories().size(), 0 );
	// Supports only one
	QVERIFY( !pcRec->supportsMultipleCategories() );
	
	QCOMPARE( fConduit->mapping()->pcRecordId( hhId ), pcId );
	
	// Sync
	fConduit->hotSyncTest();
	
	// Postconditions
	hhRec = fConduit->hhDataProxy()->records()->value( hhId );
	backupRec = fConduit->backupDataProxy()->records()->value( hhId );
	pcRec = fConduit->pcDataProxy()->records()->value( pcId );
	
	QCOMPARE( pcRec->categories().size(), 1 );
	QVERIFY( pcRec->categories().first() == hhRec->categories().first() );
	QVERIFY( backupRec->categories().first() == hhRec->categories().first() );
}
*/

void CategoryHotSyncTest::cleanupTestCase()
{
	delete fConduit;
}

/** ***************************************************************************
 *                      Initialization of test cases                          *
 ******************************************************************************/
void CategoryHotSyncTest::initTestCase()
{
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QVariantList args = QVariantList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	fConduit = new TestRecordConduit( args, false );
	fConduit->initDataProxies();
}

void CategoryHotSyncTest::initTestCase_652()
{
	// Set up the conduit
	initTestCase();
	
	// Add a handheld record with a category to the hhdataproxy.
	TestHHRecord *hhRec = new TestHHRecord( QStringList(), CSL1( "hh-2" ) );
	hhRec->setCategory( 1, "TestCategory" );
	hhRec->setModified();
	fConduit->addHHRecord( hhRec );
}

void CategoryHotSyncTest::initTestCase_653a()
{
	// 6.5.3 |pc-2 |  - |Y| -| M|hh-3 | sync hh to pc
	// a) No category was set on the pc, the record supports more categories
	//
	// Solution: Category set on the Handheld must be added to the new record on
	// the pc. And must be set in the backup record.
	
	// Set up the conduit
	initTestCase();
	
	// Add a modified handheld record with a category to the hhdataproxy.
	//HHCategory *c = new HHCategory( "Second Test category", false, 2, '2' );
		
	QString hhId( "hh-3" );
	QString pcId( "pc-2" );
		
	TestHHRecord *hhRec = new TestHHRecord( QStringList(), hhId );
	hhRec->setCategory( 2, "Second test category" );
	hhRec->setModified();
	fConduit->addHHRecord( hhRec );
	
	// Add a handheld record with no category set to the backup 
	// dataproxy.
	TestHHRecord *backupRec = new TestHHRecord( QStringList(), hhId );
	fConduit->addBackupRecord( backupRec );
	
	// Add a pc record with the no category set to the pc dataproxy.
	TestRecord *pcRec = new TestRecord( QStringList(), pcId );
	fConduit->addPCRecord( pcRec );
	
	// Create a mapping
	fConduit->mapping().map( hhId, pcId );
}

/*
void CategoryHotSyncTest::initTestCase_653b()
{
	// 6.5.3 |pc-2 |  - |Y| -| M|hh-3 | sync hh to pc
	// b) No category was set on the pc, the record supports only one category
	//    (e.g. keyring conduit)
	//
	// Solution: Category set on the Handheld must be set to the new record on
	// the pc. And must be set in the backup record.
	
	// Set up the conduit
	initTestCase_653a();
	
	// Make the record in the pc datastore only support one category
	QString pcId( "pc-2" );
	TestRecord *pcRec = static_cast<TestRecord*>(
		fConduit->pcDataProxy()->records()->value( pcId ) );
	pcRec->setSupportsMultipleCategories( false );
}
*/

QTEST_KDEMAIN( CategoryHotSyncTest, NoGUI )

#include "categoryhotsynctest.moc"
