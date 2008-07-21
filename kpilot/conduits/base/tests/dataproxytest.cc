/* dataproxytest.cc			KPilot
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

#include <QtTest>
#include <QtCore>

#include <options.h>
#include <idmapping.h>
#include <cudcounter.h>

#include "qtest_kde.h"

#include "testdataproxy.h"
#include "record.h"
#include "testrecord.h"

class DataProxyTest : public QObject
{
	Q_OBJECT

public:
	DataProxyTest();

private:
	QStringList fFields;
	
private slots:
	void testCreate();
	void testFind();
	void testUpdate();
	void testRemove();
	void testRecordCount();
	void testIds();
	void testSetEndcount();
	void testIterationModeAll();
	void testIterationModeModified();
	void testCommitCreated();
	void testCommitUpdated();
	void testCommitDeleted();
};

DataProxyTest::DataProxyTest()
{
	fFields << CSL1( "f1" ) << CSL1( "f2" );
}

void DataProxyTest::testCreate()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord( fFields );
	Record *rec2 = new TestRecord( fFields );
	
	QString id1 = fProxy.create( rec1 );
	QString id2 = fProxy.create( rec2 );
	
	QVERIFY( id1 != id2 );
	QVERIFY( rec1->id() == id1 );
	QVERIFY( rec2->id() == id2 );	
	QVERIFY( fProxy.counter()->countCreated() == 2 );
}

void DataProxyTest::testFind()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord( fFields );
	
	QString id1 = fProxy.create( rec1 );
	
	QVERIFY( fProxy.find( id1 ) == rec1 );
	QVERIFY( fProxy.find( CSL1( "ID" ) ) == 0L );
}

void DataProxyTest::testUpdate()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A TEST VALUE" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "ANOTHER TEST VALUE" ) );
	
	// Because the setValue() is a testimplementation, we want to make sure it
	// works.
	QVERIFY( rec1->value( CSL1( "f1" ) ) == CSL1( "A TEST VALUE" ) );
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And Yet another test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "And the last one" ) );
	
	QString id1 = fProxy.create( rec1 );
	fProxy.update( id1, rec2 );
	
	QVERIFY( rec2->value( CSL1( "f1" ) ) == CSL1( "And Yet another test value" ) );
	QVERIFY( rec2->value( CSL1( "f2" ) ) == CSL1( "And the last one" ) );
	QVERIFY( fProxy.counter()->countUpdated() == 1 );
}

void DataProxyTest::testRemove()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A TEST VALUE" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "ANOTHER TEST VALUE" ) );
	
	QString id1 = fProxy.create( rec1 );
	
	QVERIFY( rec1 == fProxy.find( id1 ) );
	
	fProxy.remove( id1 );
	
	QVERIFY( fProxy.find( id1 ) == 0L );
	QVERIFY( fProxy.counter()->countDeleted() == 1 );
}

void DataProxyTest::testRecordCount()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord( fFields );
	Record *rec2 = new TestRecord( fFields );
	
	QVERIFY( fProxy.recordCount() == 0 );
	
	QString id1 = fProxy.create( rec1 );
	QVERIFY( fProxy.recordCount() == 1 );
	
	QString id2 = fProxy.create( rec2 );
	QVERIFY( fProxy.recordCount() == 2 );
	
	fProxy.remove( id1 );
	QVERIFY( fProxy.recordCount() == 1 );
}

void DataProxyTest::testIds()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord( fFields );
	Record *rec2 = new TestRecord( fFields );
	
	QString id1 = fProxy.create( rec1 );
	QString id2 = fProxy.create( rec2 );
	
	QVERIFY( fProxy.ids().size() == 2 );
	QVERIFY( fProxy.ids().contains( id1 ) );
	QVERIFY( fProxy.ids().contains( id2 ) );
}

void DataProxyTest::testSetEndcount()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord( fFields );
	Record *rec2 = new TestRecord( fFields );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	
	QVERIFY( fProxy.counter()->countEnd() == 0 );
	fProxy.setEndcount();
	QVERIFY( fProxy.counter()->countEnd() == 2 );
}

void DataProxyTest::testIterationModeAll()
{
	// First test the mode: All
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	TestRecord *rec3 = new TestRecord( fFields );
	rec3->setValue( CSL1( "f1" ), CSL1( "One for the third" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Two for the third" ) );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	fProxy.create( rec3 );
	
	fProxy.setIterateMode( DataProxy::All );
	fProxy.resetIterator();
	
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec1 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec2 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec3 );
	QVERIFY( !fProxy.hasNext() );
}

void DataProxyTest::testIterationModeModified()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	rec2->synced();
	
	TestRecord *rec3 = new TestRecord( fFields );
	rec3->setValue( CSL1( "f1" ), CSL1( "One for the third" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Two for the third" ) );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	fProxy.create( rec3 );
	
	QVERIFY( rec1->isModified() );
	QVERIFY( !rec2->isModified() );
	QVERIFY( rec3->isModified() );
	
	fProxy.setIterateMode( DataProxy::Modified );
	fProxy.resetIterator();
	
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec1 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec3 );
	QVERIFY( !fProxy.hasNext() );
}

void DataProxyTest::testCommitCreated()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	rec2->synced();
	
	TestRecord *rec3 = new TestRecord( fFields );
	rec3->setValue( CSL1( "f1" ), CSL1( "One for the third" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Two for the third" ) );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	fProxy.create( rec3 );
	
	// Make sure that we know what ids the records have.
	rec1->setId( CSL1( "1" ) );
	rec2->setId( CSL1( "2" ) );
	rec3->setId( CSL1( "3" ) );
	
	fProxy.commit();
	
	QVERIFY( fProxy.createCount() == 3 );
	QVERIFY( fProxy.created().value( CSL1( "1" ) ) );
	QVERIFY( fProxy.created().value( CSL1( "2" ) ) );
	QVERIFY( fProxy.created().value( CSL1( "3" ) ) );
	
	fProxy.rollback();
	
	QVERIFY( fProxy.deleteCount() == 3 );
	QVERIFY( !fProxy.created().value( CSL1( "1" ) ) );
	QVERIFY( !fProxy.created().value( CSL1( "2" ) ) );
	QVERIFY( !fProxy.created().value( CSL1( "3" ) ) );
}

void DataProxyTest::testCommitUpdated()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setId( CSL1( "1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	rec1->synced();
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setId( CSL1( "2" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	rec2->synced();
	
	// Add records to proxy as if they where there already.
	fProxy.records()->insert( rec1->id(), rec1 );
	fProxy.records()->insert( rec2->id(), rec2 );
	
	// New  values for the existing records.
	TestRecord *rec3 = new TestRecord( fFields );
	rec3->setId( CSL1( "1" ) );
	rec3->setValue( CSL1( "f1" ), CSL1( "An updated test value" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Another updated test value" ) );
	
	TestRecord *rec4 = new TestRecord( fFields );
	rec4->setId( CSL1( "2" ) );
	rec4->setValue( CSL1( "f1" ), CSL1( "And more updated test value" ) );
	rec4->setValue( CSL1( "f2" ), CSL1( "Yet another updated one" ) );
	
	// Now update the records.
	fProxy.update( rec3->id(), rec3 );
	fProxy.update( rec4->id(), rec4 );
	fProxy.commit();
	
	QVERIFY( fProxy.updateCount() == 2 );
	QVERIFY( fProxy.updatedRecords()->value( rec3->id() ) != rec1 );
	QVERIFY( fProxy.updatedRecords()->value( rec4->id() ) != rec2 );
	QVERIFY( fProxy.updatedRecords()->value( rec3->id() ) == rec3 );
	QVERIFY( fProxy.updatedRecords()->value( rec4->id() ) == rec4 );
	
	fProxy.rollback();
	
	QVERIFY( fProxy.updateCount() == 4 );
	QVERIFY( fProxy.updatedRecords()->value( rec3->id() ) == rec1 );
	QVERIFY( fProxy.updatedRecords()->value( rec4->id() ) == rec2 );
	QVERIFY( fProxy.updatedRecords()->value( rec3->id() ) != rec3 );
	QVERIFY( fProxy.updatedRecords()->value( rec4->id() ) != rec4 );
}

void DataProxyTest::testCommitDeleted()
{
	TestDataProxy fProxy;
	
	TestRecord *rec1 = new TestRecord( fFields );
	rec1->setId( CSL1( "1" ) );
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	rec1->synced();
	
	TestRecord *rec2 = new TestRecord( fFields );
	rec2->setId( CSL1( "2" ) );
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	rec2->synced();
	
	// Add records to proxy as if they where there already.
	fProxy.records()->insert( rec1->id(), rec1 );
	fProxy.records()->insert( rec2->id(), rec2 );
	
	// Now update the records.
	fProxy.remove( rec1->id() );
	fProxy.remove( rec2->id() );
	fProxy.commit();
	
	QVERIFY( fProxy.deleteCount() == 2 );
	QVERIFY( fProxy.deletedRecords()->value( rec1->id() ) == rec1 );
	QVERIFY( fProxy.deletedRecords()->value( rec2->id() ) == rec2 );
	
	fProxy.rollback();
	
	QVERIFY( fProxy.createCount() == 2 );
	QVERIFY( fProxy.deletedRecords()->size() == 0 );
}
QTEST_KDEMAIN(DataProxyTest, NoGUI)

#include "dataproxytest.moc"
