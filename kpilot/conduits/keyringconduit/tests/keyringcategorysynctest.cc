/* keyringcategorysynctest.cc			KPilot
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

#include <QtTest>
#include <QFile>

#include "qtest_kde.h"

#include "options.h"
#include "idmapping.h"
#include "keyringhhrecord.h"
#include "testkeyringconduit.h"
#include "testkeyringproxy.h"

class KeyringCategorySyncTest : public QObject
{
	Q_OBJECT

public:
	KeyringCategorySyncTest() : fConduit( 0L ) {};

private: // members
	TestKeyringConduit *fConduit;	
	
	// key: [testname], value: [record id]
	QMap<QString, QString> fTestRecordIds;

private: // methods
	// General setup. WARNING: Make sure you call this before every test. Trying
	// to sync twice without reloading is not appreciated by the proxies due to
	// the nature of their behavior.
	void init();
	
	// Sets up the preconditions for the tests
	void initTest1();
	void initTest2();
	void initTest3();
	
private slots:
	void initTestCase();
	
	/**
	 * Preconditions:
	 * - A category which is not already on the HHDataProxy is added.
	 *
	 * Postconditions:
	 * - After a HHDataProxy::commit() the category should be in the database file.
	 */
	void test0();
	
	/**
	 * Preconditions:
	 * - A new record is added on the pc side.
	 * - There is no mapping yet for the record.
	 * - It has a category
	 * - That category is available on the hh side.
	 *
	 * Postconditions:
	 * - There is a mapping for the record.
	 * - There is a record added to the hh.
	 * - It has the same category as on the pc.
	 */
	void test1();
	
	/**
	 * Preconditions:
	 * - A new record is added on the pc side.
	 * - There is no mapping yet for the record.
	 * - It has a category that is not yet available on the handheld
	 * - The handheld has room for a new category.
	 *
	 * Postconditions:
	 * - There is a mapping for the record.
	 * - The category of the pc record is added tot the handheld
	 * - There is a record added to the hh.
	 * - It has the same category as on the pc.
	 */
	void test2();
	
	/**
	 * Preconditions:
	 * - A new record is added on the pc side.
	 * - There is no mapping yet for the record.
	 * - It has a category that is not yet available on the handheld
	 * - The handheld has no room for a new category.
	 *
	 * Postconditions:
	 * - There is a mapping for the record.
	 * - There is a record added to the hh.
	 * - It has it's category set to unfiled.
	 */
	void test3();
	
	
	void cleanupTestCase();
};

/**
 * In a newly created keyring data proxy the following categories are available
 *
 * 0: Unfiled
 * 1: Banking
 * 2: Computer
 * 3: Phone
 */

void KeyringCategorySyncTest::test0()
{
	// Let's make sure that the file does not already exists.
	QFile f( "test.pdb" );
	if( f.exists() )
	{
		f.remove();
	}

	// Let's create a keyring proxy for a local file.
	TestKeyringProxy* tmp = new TestKeyringProxy( "test.pdb" );
	tmp->openDatabase( "test" );
	tmp->createDataStore();
	
	// We want of course check HHDataProxy so change the pointer type to be sure
	// that the HHDataProxy methods, as far as implemented are called.
	HHDataProxy* proxy = tmp;
	
	// It should not have the new category.
	QVERIFY( !proxy->containsCategory( "NewCategory" ) );
	
	// Let's add the new category now.
	proxy->addGlobalCategory( "NewCategory" );
	
	// It now should have the new category.
	QVERIFY( proxy->containsCategory( "NewCategory" ) );
	
	// Let's commit the changes and check if it's still there.
	proxy->commit();
	QVERIFY( proxy->containsCategory( "NewCategory" ) );
	
	// Now we reload the proxy to see if the changes are actually saved to the
	// file.
	delete proxy;
	
	// The keyring data proxy needs a pass to open correctly.
	tmp = new TestKeyringProxy( "test.pdb" );
	tmp->openDatabase( "test" );
	
	proxy = tmp;
	
	QVERIFY( proxy->containsCategory( "NewCategory" ) );
	
	// And clean up.
	delete proxy;
}

void KeyringCategorySyncTest::test1()
{
	// Set up the conduit and the dataproxies.
	initTest1();
	
	// Preconditions:
	QString pcId = fTestRecordIds.value( "test1" );
	KeyringHHRecord* pcRec = fConduit->pcProxy()->record( pcId );
	
	QString hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( hhId.isEmpty() );
	
	QVERIFY( pcRec );
	QCOMPARE( pcRec->categories().size(), 1 );
	QCOMPARE( pcRec->category(), CSL1( "Banking" ) );
	
	// Sync
	fConduit->hotSync();
	
	// Postconditions
	hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( !hhId.isEmpty() );
	
	KeyringHHRecord *hhRec = fConduit->hhProxy()->record( hhId );
	QVERIFY( hhRec );
	QCOMPARE( hhRec->categories().size(), 1 );
	QCOMPARE( hhRec->category(), CSL1( "Banking" ) );
}

void KeyringCategorySyncTest::test2()
{
	// Set up the conduit and the dataproxies.
	initTest2();
	
	// Preconditions:
	QVERIFY( fConduit->pcProxy()->categories().contains( "NewCategory1" ) );
	QVERIFY( (uint) fConduit->hhProxy()->categories().size() < Pilot::CATEGORY_COUNT  );
	
	QString pcId = fTestRecordIds.value( "test2" );
	QString hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( hhId.isEmpty() );
	
	KeyringHHRecord* pcRec = fConduit->pcProxy()->record( pcId );
	QVERIFY( pcRec );
	QCOMPARE( pcRec->categories().size(), 1 );
	QCOMPARE( pcRec->category(), CSL1( "NewCategory1" ) );
	
	// Sync
	fConduit->hotSync();
	
	// Postconditions
	QVERIFY( fConduit->hhProxy()->categories().contains( "NewCategory1" ) );
	
	hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( !hhId.isEmpty() );
	
	KeyringHHRecord *hhRec = fConduit->hhProxy()->record( hhId );
	QVERIFY( hhRec );
	QCOMPARE( hhRec->categories().size(), 1 );
	QCOMPARE( hhRec->category(), CSL1( "NewCategory1" ) );
}

void KeyringCategorySyncTest::test3()
{
	// Set up the conduit and the dataproxies.
	initTest3();
	
	// Preconditions:
	QVERIFY( fConduit->pcProxy()->categories().contains( "NewCategory2" ) );
	QCOMPARE( (uint) fConduit->hhProxy()->categories().size(), Pilot::CATEGORY_COUNT );
	
	QString pcId = fTestRecordIds.value( "test3" );
	QString hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( hhId.isEmpty() );
	
	KeyringHHRecord* pcRec = fConduit->pcProxy()->record( pcId );
	QVERIFY( pcRec );
	QCOMPARE( pcRec->categories().size(), 1 );
	QCOMPARE( pcRec->category(), CSL1( "NewCategory2" ) );
	
	// Sync
	fConduit->hotSync();
	
	// Postconditions
	QCOMPARE( (uint) fConduit->hhProxy()->categories().size(), Pilot::CATEGORY_COUNT );
	// The category should not get added in favor of one of the other categories.
	QVERIFY( !fConduit->hhProxy()->categories().contains( "NewCategory2" ) );
	
	hhId = fConduit->mapping()->hhRecordId( pcId );
	QVERIFY( !hhId.isEmpty() );
	
	KeyringHHRecord *hhRec = fConduit->hhProxy()->record( hhId );
	QVERIFY( hhRec );
	QCOMPARE( hhRec->categories().size(), 1 );
	QCOMPARE( hhRec->category(), CSL1( "Unfiled" ) );
}

/* ************************** INIT METHODS ********************************** */

void KeyringCategorySyncTest::initTestCase()
{
	Pilot::setupPilotCodec( CSL1( "ISO8859-15" ) );
}

void KeyringCategorySyncTest::init()
{
	delete fConduit;

	// Remove all old files before beginning.
	QFile f( "hhproxy.pdb" );
	f.remove();
	
	f.setFileName( "hhproxy.pdb~" );
	f.remove();
	
	f.setFileName( "backupproxy.pdb" );
	f.remove();
	
	f.setFileName( "backupproxy.pdb~" );
	f.remove();
	
	f.setFileName( "pcproxy.pdb" );
	f.remove();
	
	f.setFileName( "pcproxy.pdb~" );
	f.remove();

	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QVariantList args;
	args << CSL1( "--hotsync" ) << CSL1( "--conflictResolution 2" );
	
	fConduit = new TestKeyringConduit( args );
	fConduit->initDataProxies();
}

void KeyringCategorySyncTest::initTest1()
{
	init();

	KeyringHHRecord* rec = fConduit->pcProxy()->createRecord();
	fConduit->pcProxy()->setCategory( rec, CSL1( "Banking" ) );
	fConduit->pcProxy()->addRecord( rec );
	
	fTestRecordIds.insert( "test1", rec->id() );
}

void KeyringCategorySyncTest::initTest2()
{
	init();
	
	KeyringHHRecord* rec = fConduit->pcProxy()->createRecord();

	fConduit->pcProxy()->addGlobalCategory( CSL1( "NewCategory1" ) );
	fConduit->pcProxy()->setCategory( rec, CSL1( "NewCategory1" ) );
	fConduit->pcProxy()->addRecord( rec );
	
	fTestRecordIds.insert( "test2", rec->id() );
}

void KeyringCategorySyncTest::initTest3()
{
	init();

	int i = 1;
	QString category( "OtherCat" );

	while( (uint) fConduit->hhProxy()->categories().size() < Pilot::CATEGORY_COUNT )
	{
		QString c( category );
		c.append( QString::number( i ) );
		
		fConduit->hhProxy()->addGlobalCategory( c );
		i++;
	}

	KeyringHHRecord* rec = fConduit->pcProxy()->createRecord();
	fConduit->pcProxy()->addGlobalCategory( CSL1( "NewCategory2" ) );
	fConduit->pcProxy()->setCategory( rec, CSL1( "NewCategory2" ) );
	fConduit->pcProxy()->addRecord( rec );
	
	fTestRecordIds.insert( "test3", rec->id() );
}

void KeyringCategorySyncTest::cleanupTestCase()
{
	delete fConduit;
}

QTEST_KDEMAIN( KeyringCategorySyncTest, NoGUI )

#include "keyringcategorysynctest.moc"
