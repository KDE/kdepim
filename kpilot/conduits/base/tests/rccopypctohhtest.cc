/* rccopypctohhtest.cc			KPilot
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

// Qt includes
#include <QtTest>
#include <QtCore>

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
 * See 5.4 in the use case document for a better explanation of what should
 * happen.
 *
 * There are a couple of things to test currently.
 * 1. Test what happens when there is a mapping for a HH record.
 * 2. Test what happens when there is not a mapping for a HH record.
 * 3. Test what happens when there is a mapping for a PC record but HH record
 *    does not exist.
 * 4. Test what happens when there is not a mapping for a PC record.
 */

class RCCopyPCToHHTest : public QObject
{
	Q_OBJECT

private slots:
	void testMappingForPCRecord();
	void testNoMappingForPCRecord();
	void testMappingForHHRecord();
	void testNoMappingForHHRecord();
	void testDeletedRecordOnPc();
};

void RCCopyPCToHHTest::testMappingForPCRecord()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" )
		<< CSL1( "--conflictResolution 2" );
	
	// Create conduit
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	conduit.pcDataProxy()->setIterateMode( DataProxy::All );
	conduit.hhDataProxy()->setIterateMode( DataProxy::All );
	
	// Create records
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *pcRec = new TestRecord( fields, CSL1( "pc-1" ) );
	pcRec->setValue( CSL1( "f1" ), CSL1( "A changed test value" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestHHRecord *hhRec = new TestHHRecord( fields, CSL1( "hh-1" ) );
	hhRec->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	conduit.pcDataProxy()->records()->insert( pcRec->id(), pcRec );
	conduit.hhDataProxy()->records()->insert( hhRec->id(), hhRec );
	conduit.mapping().map( hhRec->id(), pcRec->id() );
	
	QVERIFY( !conduit.equal( pcRec, hhRec ) );
	
	conduit.copyPCToHHTest();
	
	// Both records should be equal now.
	QVERIFY( conduit.equal( pcRec, hhRec ) );
	
	// And HH overides
	QVERIFY( pcRec->value( CSL1( "f1" ) ) == CSL1( "A test value" ) );
}

void RCCopyPCToHHTest::testNoMappingForPCRecord()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" )
		<< CSL1( "--conflictResolution 2" );
	
	// Create conduit
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	conduit.pcDataProxy()->setIterateMode( DataProxy::All );
	conduit.hhDataProxy()->setIterateMode( DataProxy::All );
	
	// Create record
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *pcRec = new TestRecord( fields, CSL1( "pc-1" ) );
	pcRec->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	conduit.pcDataProxy()->records()->insert( pcRec->id(), pcRec );
	
	conduit.copyPCToHHTest();
	
	// There should be a mapping right now.
	QVERIFY( conduit.mapping().containsPCId( pcRec->id() ) );
	
	QString hhId = conduit.mapping().hhRecordId( pcRec->id() );
	HHRecord *hhRec = static_cast<HHRecord*>( conduit.hhDataProxy()->find( hhId ) );
	
	QVERIFY( hhRec );
	QVERIFY( conduit.equal( pcRec, hhRec ) );
}

void RCCopyPCToHHTest::testMappingForHHRecord()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" )
		<< CSL1( "--conflictResolution 2" );
	
	// Create conduit
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	conduit.pcDataProxy()->setIterateMode( DataProxy::All );
	conduit.hhDataProxy()->setIterateMode( DataProxy::All );
	
	// Create record
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestHHRecord *hhRec = new TestHHRecord( fields, CSL1( "hh-1" ) );
	hhRec->setValue( CSL1( "f1" ), CSL1( "A changed test value" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	conduit.hhDataProxy()->records()->insert( hhRec->id(), hhRec );
	conduit.mapping().map( hhRec->id(), CSL1( "removed-id" ) );
	
	conduit.copyPCToHHTest();
	
	// Mapping should be removed
	QVERIFY( !conduit.mapping().containsPCId( CSL1( "removed-id" ) ) );
	QVERIFY( !conduit.mapping().containsHHId( hhRec->id() ) );
	
	// PC record should be removed from database.
	QVERIFY( !conduit.hhDataProxy()->find( hhRec->id() ) );
}

void RCCopyPCToHHTest::testNoMappingForHHRecord()
{
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" )
		<< CSL1( "--conflictResolution 2" );
	
	// Create conduit
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	conduit.pcDataProxy()->setIterateMode( DataProxy::All );
	conduit.hhDataProxy()->setIterateMode( DataProxy::All );
	
	// Create record
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestHHRecord *hhRec = new TestHHRecord( fields, CSL1( "hh-1" ) );
	hhRec->setValue( CSL1( "f1" ), CSL1( "A changed test value" ) );
	hhRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	conduit.hhDataProxy()->records()->insert( hhRec->id(), hhRec );
	
	// There shouldn't be a mapping.
	QVERIFY( !conduit.mapping().containsHHId( hhRec->id() ) );
	// Record should be there.
	QVERIFY( conduit.hhDataProxy()->find( hhRec->id() ) );
	
	conduit.copyPCToHHTest();
	
	// The still shouldn't be a mapping
	QVERIFY( !conduit.mapping().containsHHId( hhRec->id() ) );
	
	// PC record should be removed from database.
	QVERIFY( !conduit.hhDataProxy()->find( hhRec->id() ) );
}

void RCCopyPCToHHTest::testDeletedRecordOnPc()
{
	/*
	 * If a record on the handheld is marked as deleted, it should be removed from
	 * handheld and no mapping should be created for it.
	 */
	QVariantList args = QVariantList() << CSL1( "--copyHHToPC" )
		<< CSL1( "--conflictResolution 2" );
	
	// Create conduit
	TestRecordConduit conduit( args );
	conduit.initDataProxies();
	conduit.pcDataProxy()->setIterateMode( DataProxy::All );
	conduit.hhDataProxy()->setIterateMode( DataProxy::All );
	
	// Create record
	QStringList fields = QStringList() << CSL1( "f1" ) << CSL1( "f2" );
	
	TestRecord *pcRec = new TestRecord( fields, CSL1( "pc-1" ) );
	pcRec->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	pcRec->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	pcRec->setDeleted();
	
	conduit.hhDataProxy()->records()->insert( pcRec->id(), pcRec );
	
	// Verify starting values.
	QVERIFY( !conduit.mapping().containsHHId( pcRec->id() ) );
	QVERIFY( conduit.hhDataProxy()->find( pcRec->id() ) );
	QVERIFY( conduit.hhDataProxy()->find( pcRec->id() )->isDeleted() );
	
	// Eexecute the conduit.
	conduit.copyPCToHHTest();
	
	// Verify ending values.
	QVERIFY( !conduit.mapping().containsPCId( pcRec->id() ) );
	QVERIFY( !conduit.pcDataProxy()->find( pcRec->id() ) );
}

QTEST_KDEMAIN(RCCopyPCToHHTest, NoGUI)

#include "rccopypctohhtest.moc"
