/* rcfirstsynctest.cc			KPilot
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

// KDE includes
#include <qtest_kde.h>

// KPilot includes
#include "options.h"
#include "idmapping.h"
#include "dataproxy.h"
#include "hhdataproxy.h"
#include "testrecordconduit.h"
#include "testrecord.h"
#include "testhhrecord.h"


/**
 * There are a couple of things to test currently.
 * 1. Test if the implemented equal method works as expected.
 * 2. Test if the implemented newPCRecord and new HHRecord work as expected.
 * 3. Test what happens if two records match.
 * 4. Test what happens if there is no match for a record.
 */

class RCFirstSyncTest : public QObject
{
	Q_OBJECT

private slots:
	void testEqual();
	void testNewRecords();
	void testMatch();
	void testNoMatch();
};

void RCFirstSyncTest::testEqual()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields, CSL1( "1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *rec2 = new TestHHRecord( fields, CSL1( "2" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *rec3 = new TestHHRecord( fields, CSL1( "3" ) );
	rec3->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	QVERIFY( conduit.equal( rec1, rec2 ) );
	QVERIFY( !conduit.equal( rec1, rec3 ) );
}

void RCFirstSyncTest::testNewRecords()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields, CSL1( "1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *rec2 = new TestHHRecord( fields, CSL1( "2" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	HHRecord *rec3 = conduit.newHHRecord( rec1 );
	Record *rec4 = conduit.newPCRecord( rec2 );
	
	// Created records should not be 0L
	QVERIFY( rec3 );
	QVERIFY( rec4 );
	
	// Created records should be equal to the one from which they are created.
	QVERIFY( conduit.equal( rec1, rec3 ) );
	QVERIFY( conduit.equal( rec4, rec2 ) );
}

void RCFirstSyncTest::testMatch()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// Add some records to the data proxies.
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields, CSL1( "pc-1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *rec2 = new TestHHRecord( fields, CSL1( "hh-1" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	conduit.pcDataProxy()->records()->insert( rec1->id(), rec1 );
	conduit.pcDataProxy()->recordsByDescription()->insert( rec1->description(), rec1 );
	conduit.hhDataProxy()->records()->insert( rec2->id(), rec2 );
	conduit.hhDataProxy()->recordsByDescription()->insert( rec2->description(), rec2 );
	
	// There should be a valid mapping
	QVERIFY( conduit.mapping().hhRecordId( CSL1( "pc-1" ) ).isEmpty() );
	QVERIFY( conduit.mapping().pcRecordId( CSL1( "hh-1" ) ).isEmpty() );
	QVERIFY( conduit.equal( rec1, rec2 ) );
	
	// Everything is ok, do a hotsync now.
	conduit.firstSyncTest();
	
	// There should be a valid mapping
	QVERIFY( conduit.mapping().hhRecordId( CSL1( "pc-1" ) ) == CSL1( "hh-1" ) );
	QVERIFY( conduit.mapping().pcRecordId( CSL1( "hh-1" ) ) == CSL1( "pc-1" ) );
}

void RCFirstSyncTest::testNoMatch()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" );
	
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	
	// Add some records to the data proxies.
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *rec1 = new TestRecord( fields, CSL1( "pc-1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *rec2 = new TestHHRecord( fields, CSL1( "hh-1" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "A handheld test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Another handheld test value" ) );
	
	conduit.pcDataProxy()->records()->insert( rec1->id(), rec1 );
	conduit.hhDataProxy()->records()->insert( rec2->id(), rec2 );
	
	// There should be a valid mapping
	QVERIFY( conduit.mapping().hhRecordId( CSL1( "pc-1" ) ).isEmpty() );
	QVERIFY( conduit.mapping().pcRecordId( CSL1( "hh-1" ) ).isEmpty() );
	QVERIFY( !conduit.equal( rec1, rec2 ) );
	
	// Everything is ok, do a hotsync now.
	conduit.firstSyncTest();
	
	// There should be a valid mapping
	QVERIFY( !conduit.mapping().hhRecordId( CSL1( "pc-1" ) ).isEmpty() );
	QVERIFY( conduit.mapping().hhRecordId( CSL1( "pc-1" ) ) != CSL1( "hh-1" ) );
	QVERIFY( !conduit.mapping().pcRecordId( CSL1( "hh-1" ) ).isEmpty() );
	QVERIFY( conduit.mapping().pcRecordId( CSL1( "hh-1" ) ) != CSL1( "pc-1" ) );
	
	QString hhId = conduit.mapping().hhRecordId( rec1->id() );
	HHRecord *hhRec = static_cast<HHRecord*>( conduit.hhDataProxy()->find( hhId ) );
	
	QVERIFY( conduit.equal( rec1, hhRec ) );
	
	QString pcId = conduit.mapping().pcRecordId( rec2->id() );
	Record *pcRec = conduit.pcDataProxy()->find( pcId );
	
	QVERIFY( conduit.equal( pcRec, rec2 ) );
}

QTEST_KDEMAIN(RCFirstSyncTest, NoGUI)

#include "rcfirstsynctest.moc"
