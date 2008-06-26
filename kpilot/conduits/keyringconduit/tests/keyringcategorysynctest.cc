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

#include "qtest_kde.h"

#include "options.h"
#include "idmapping.h"
#include "keyringhhrecord.h"
#include "testkeyringconduit.h"
#include "testkeyringproxy.h"

class KeyringCategorySyncTest : public QObject
{
	Q_OBJECT

private: // members
	TestKeyringConduit *fConduit;	
	
	// key: [testname], value: [record id]
	QMap<QString, QString> fTestRecordIds;

private: // methods
	// Sets up the preconditions for test1.
	void initTest1();
	// Sets up the preconditions for test2.
	void initTest2();

private slots:
	void initTestCase();
	
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

void KeyringCategorySyncTest::initTestCase()
{
	Pilot::setupPilotCodec( CSL1( "ISO8859-15" ) );

	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QVariantList args;
	args << CSL1( "--hotsync" ) << CSL1( "--conflictResolution 2" );
	
	fConduit = new TestKeyringConduit( args );
	fConduit->initDataProxies();
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
}

void KeyringCategorySyncTest::initTest1()
{
	KeyringHHRecord* rec = fConduit->pcProxy()->createRecord();
	fConduit->pcProxy()->setCategory( rec, CSL1( "Banking" ) );
	fConduit->pcProxy()->addRecord( rec );
	
	fTestRecordIds.insert( "test1", rec->id() );
}

void KeyringCategorySyncTest::initTest2()
{
	KeyringHHRecord* rec = fConduit->pcProxy()->createRecord();
	fConduit->pcProxy()->setCategory( rec, CSL1( "Banking" ) );
	fConduit->pcProxy()->addRecord( rec );
	
	fTestRecordIds.insert( "test1", rec->id() );
}

void KeyringCategorySyncTest::cleanupTestCase()
{
	delete fConduit;
}

QTEST_KDEMAIN( KeyringCategorySyncTest, NoGUI )

#include "keyringcategorysynctest.moc"
