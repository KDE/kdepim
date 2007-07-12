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
#include "testrecord.h"


class RecordConduitTest : public QObject
{
	Q_OBJECT

	/* possibilities (O = deleted, M = modified, - = unchanged):
	 *
	 * Tests the solving algorithm for conflicting records when HH Overides:
	 *
	 * CASE | PC | HH | Resolotion
	 *  1-1 |  - |  - | Not a conflict
	 *  1-2 |  - |  M | Sync hh fields to pc record
	 *  1-3 |  - |  0 | Delete pc record from pc datastore
	 *  1-4 |  M |  - | Sync hh fields to pc record
	 *  1-5 |  M |  M | Sync hh fields to pc record
	 *  1-6 |  M |  0 | Delete pc record from pc datastore
	 *  1-7 |  0 |  - | Add duplicate of hh rec to pc datastore.
	 *  1-8 |  0 |  M | Add duplicate of hh rec to pc datastore.
	 *  1-9 |  0 |  0 | Not a conflict
	 *
	 */

private slots:
	void testSyncFields();
	void testCase_1_2();
	void testCase_1_3();
	void testCase_1_4();
	void testCase_1_5();
	void testCase_1_6();
	void testCase_1_7();
	void testCase_1_8();
};

void RecordConduitTest::testSyncFields()
{
	QStringList args = QStringList() << CSL1( "--hotsync" );
		//<< CSL1( "--conflictResolution 1" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord( fields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	//                             ( from, to )
	QVERIFY( conduit.syncFieldsTest( rec1, rec2 ) );
	QVERIFY( rec2->value( CSL1( "f1" ) ) == CSL1( "A test value" ) );
	QVERIFY( rec2->value( CSL1( "f2" ) ) == CSL1( "Another test value" ) );
	
	// Make a record with other fields
	QStringList fields2 = QStringList() << CSL1( "afield" ) << CSL1( "f2" );
	
	TestRecord *rec3 = new TestRecord( fields2 );
	rec3->setValue( CSL1( "afield" ), CSL1( "Test 3-1" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Test 3-2" ) );
	
	// Then try to sync, because not all fields are the same the to record should
	// remain unmodified.
	//                             ( from, to )
	QVERIFY( !conduit.syncFieldsTest( rec1, rec3 ) );
	QVERIFY( rec3->value( CSL1( "afield" ) ) == CSL1( "Test 3-1" ) );
	QVERIFY( rec3->value( CSL1( "f2" ) ) == CSL1( "Test 3-2" ) );
}

void RecordConduitTest::testCase_1_2()
{
	// CASE | PC | HH | Resolotion
	//  1-2 |  - |  M | Sync hh fields to pc record
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// An unmodified pc rec
	TestRecord *pcRec = new TestRecord();
	pcRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "B" ) );
	pcRec->setUnmodified();
	
	QString pcId = conduit.pcDataProxy()->create( pcRec );
	
	// A modified hh rec
	TestRecord *hhRec = new TestRecord();
	hhRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "D" ) );
	
	QString hhId = conduit.hhDataProxy()->create( hhRec );
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( hhId, pcId );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( pcRec->value( CSL1( "f1" ) ) == CSL1( "A" ) );
	QVERIFY( pcRec->value( CSL1( "f2" ) ) == CSL1( "D" ) );
	QVERIFY( conduit.mapping()->contains( hhId ) );
	QVERIFY( conduit.mapping()->contains( pcId ) );
}

void RecordConduitTest::testCase_1_3()
{
	// CASE | PC | HH | Resolotion
	//  1-3 |  - |  0 | Delete pc record from pc datastore
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// An unmodified pc rec
	TestRecord *pcRec = new TestRecord();
	pcRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "B" ) );
	pcRec->setUnmodified();
	
	QString pcId = conduit.pcDataProxy()->create( pcRec );
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( CSL1( "hh-1" ), pcId );
	
	// A deleted hh rec
	TestRecord *hhRec = 0L;
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	conduit.solveConflictTest( pcRec, hhRec );
	
	QVERIFY( conduit.pcDataProxy()->find( pcRec->id() ) == 0L );
	QVERIFY( !conduit.mapping()->contains( pcRec->id() ) );
}

void RecordConduitTest::testCase_1_4()
{
	// CASE | PC | HH | Resolotion
	//  1-4 |  M |  - | Sync hh fields to pc record
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// A modified pc rec
	TestRecord *pcRec = new TestRecord();
	pcRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "B" ) );
	
	QString pcId = conduit.pcDataProxy()->create( pcRec );
	
	// An unmodified hh rec
	TestRecord *hhRec = new TestRecord();
	hhRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "D" ) );
	hhRec->setUnmodified();
	
	QString hhId = conduit.hhDataProxy()->create( hhRec );
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( hhId, pcId );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( pcRec->value( CSL1( "f1" ) ) == CSL1( "A" ) );
	QVERIFY( pcRec->value( CSL1( "f2" ) ) == CSL1( "D" ) );
	QVERIFY( conduit.mapping()->contains( hhId ) );
	QVERIFY( conduit.mapping()->contains( pcId ) );
}

void RecordConduitTest::testCase_1_5()
{
	// CASE | PC | HH | Resolotion
	//  1-5 |  M |  M | Sync hh fields to pc record
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// A modified pc rec
	TestRecord *pcRec = new TestRecord();
	pcRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "B" ) );
	
	QString pcId = conduit.pcDataProxy()->create( pcRec );
	
	// A modified hh rec
	TestRecord *hhRec = new TestRecord();
	hhRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "D" ) );
	
	QString hhId = conduit.hhDataProxy()->create( hhRec );
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( hhId, pcId );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( pcRec->value( CSL1( "f1" ) ) == CSL1( "A" ) );
	QVERIFY( pcRec->value( CSL1( "f2" ) ) == CSL1( "D" ) );
	QVERIFY( conduit.mapping()->contains( hhId ) );
	QVERIFY( conduit.mapping()->contains( pcId ) );
}

void RecordConduitTest::testCase_1_6()
{
	// CASE | PC | HH | Resolotion
	//  1-6 |  M |  0 | Delete pc record from pc datastore
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// A modified pc rec
	TestRecord *pcRec = new TestRecord();
	pcRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "B" ) );
	
	QString pcId = conduit.pcDataProxy()->create( pcRec );
	
	// A deleted hh rec
	TestRecord *hhRec = 0L;
	
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( CSL1( "hh-1" ), pcId );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( conduit.pcDataProxy()->find( pcRec->id() ) == 0L );
	QVERIFY( !conduit.mapping()->contains( pcRec->id() ) );
}

void RecordConduitTest::testCase_1_7()
{
	// CASE | PC | HH | Resolotion
	//  1-7 |  0 |  - | Add duplicate of hh rec to pc datastore.
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// A deleted pc rec
	TestRecord *pcRec = 0L;
	
	// An unmodified hh rec
	TestRecord *hhRec = new TestRecord();
	hhRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "D" ) );
	hhRec->setUnmodified();
	
	QString hhId = conduit.pcDataProxy()->create( hhRec );
	
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( hhId, CSL1( "pc-unknown" ) );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	// NOTE: This test assumes that the duplicate method of Record does work!
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( conduit.mapping()->contains( hhId ) );
	
	Record *rec = conduit.pcDataProxy()->find( conduit.mapping()->recordId( hhId ) );
		
	QVERIFY( rec );
	QVERIFY( rec->value( CSL1( "f1" ) ) == CSL1( "A" ) );
	QVERIFY( rec->value( CSL1( "f2" ) ) == CSL1( "D" ) );
}

void RecordConduitTest::testCase_1_8()
{
	// CASE | PC | HH | Resolotion
	//  1-8 |  0 |  M | Add duplicate of hh rec to pc datastore.
	
	// NOTE: 2 == eHHOverrides, this is important for the solveConflict() method
	QStringList args = QStringList() << CSL1( "--hotsync" )
		<< CSL1( "--conflictResolution 2" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// A deleted pc rec
	TestRecord *pcRec = 0L;
	
	// A modified hh rec
	TestRecord *hhRec = new TestRecord();
	hhRec->setValue( CSL1( "f1" ), CSL1( "A" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "D" ) );
	
	QString hhId = conduit.pcDataProxy()->create( hhRec );
	
	// There must be a mapping! Conflicts only occure for known records.
	conduit.mapping()->map( hhId, CSL1( "pc-unknown" ) );
	
	/** END OF PREPARATIONS **/
	
	// NOTE: we do not care about the CUDCounter, that is tested in DataProxyTest.
	// NOTE: This test assumes that the duplicate method of Record does work!
	conduit.solveConflictTest( pcRec, hhRec );
	QVERIFY( conduit.mapping()->contains( hhId ) );
	
	Record *rec = conduit.pcDataProxy()->find( conduit.mapping()->recordId( hhId ) );
		
	QVERIFY( rec );
	QVERIFY( rec->value( CSL1( "f1" ) ) == CSL1( "A" ) );
	QVERIFY( rec->value( CSL1( "f2" ) ) == CSL1( "D" ) );
}
QTEST_KDEMAIN(RecordConduitTest, NoGUI)

#include "recordconduittest.moc"
