/* dataproxytest.cc			KPilot
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

#include <options.h>
#include <idmapping.h>
#include <cudcounter.h>

#include "qtest_kde.h"

#include "testdataproxy.h"
#include "testrecord.h"

class DataProxyTest : public QObject
{
	Q_OBJECT
	
private slots:
	void testCreate();
	void testFind();
	void testUpdate();
	void testRemove();
	void testRecordCount();
	void testIds();
	void testSyncFinished();
	void testIteration();
};

void DataProxyTest::testCreate()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord();
	Record *rec2 = new TestRecord();
	
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
	
	Record *rec1 = new TestRecord();
	
	QString id1 = fProxy.create( rec1 );
	
	QVERIFY( fProxy.find( id1 ) == rec1 );
	QVERIFY( fProxy.find( CSL1( "ID" ) ) == 0L );
}

void DataProxyTest::testUpdate()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord();
	rec1->setValue( CSL1( "f1" ), CSL1( "A TEST VALUE" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "ANOTHER TEST VALUE" ) );
	
	// Because the setValue() is a testimplementation, we want to make sure it
	// works.
	QVERIFY( rec1->value( CSL1( "f1" ) ) == CSL1( "A TEST VALUE" ) );
	
	Record *rec2 = new TestRecord();
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
	
	Record *rec1 = new TestRecord();
	rec1->setValue( CSL1( "f1" ), CSL1( "A TEST VALUE" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "ANOTHER TEST VALUE" ) );
	
	QString id1 = fProxy.create( rec1 );
	
	QVERIFY( fProxy.find( id1 ) == rec1 );
	
	fProxy.remove( id1 );
	
	QVERIFY( fProxy.find( id1 ) == 0L );
	QVERIFY( fProxy.counter()->countDeleted() == 1 );
}

void DataProxyTest::testRecordCount()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord();
	Record *rec2 = new TestRecord();
	
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
	
	Record *rec1 = new TestRecord();
	Record *rec2 = new TestRecord();
	
	QString id1 = fProxy.create( rec1 );
	QString id2 = fProxy.create( rec2 );
	
	QVERIFY( fProxy.ids().size() == 2 );
	QVERIFY( fProxy.ids().contains( id1 ) );
	QVERIFY( fProxy.ids().contains( id2 ) );
}

void DataProxyTest::testSyncFinished()
{
	TestDataProxy fProxy;
	
	Record *rec1 = new TestRecord();
	Record *rec2 = new TestRecord();
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	
	QVERIFY( fProxy.counter()->countEnd() == 0 );
	fProxy.syncFinished();
	QVERIFY( fProxy.counter()->countEnd() == 2 );
}

void DataProxyTest::testIteration()
{
	// First test the mode: All
	TestDataProxy fProxy;
	fProxy.setIterateMode( DataProxy::All );
	
	TestRecord *rec1 = new TestRecord();
	rec1->setValue( CSL1( "f1" ), CSL1( "A test value" ) );
	rec1->setValue( CSL1( "f2" ), CSL1( "Another test value" ) );
	
	TestRecord *rec2 = new TestRecord();
	rec2->setValue( CSL1( "f1" ), CSL1( "And more test value" ) );
	rec2->setValue( CSL1( "f2" ), CSL1( "Yet another one" ) );
	
	TestRecord *rec3 = new TestRecord();
	rec3->setValue( CSL1( "f1" ), CSL1( "One for the third" ) );
	rec3->setValue( CSL1( "f2" ), CSL1( "Two for the third" ) );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	fProxy.create( rec3 );
	
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec1 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec2 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec3 );
	QVERIFY( !fProxy.hasNext() );
	
	// Test iteration over modified records.
	fProxy = TestDataProxy();
	
	rec2->setUnmodified();
	
	QVERIFY( rec1->isModified() );
	QVERIFY( !rec2->isModified() );
	QVERIFY( rec3->isModified() );
	
	fProxy.create( rec1 );
	fProxy.create( rec2 );
	fProxy.create( rec3 );
	
	fProxy.setIterateMode( DataProxy::Modified );
	
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec1 );
	QVERIFY( fProxy.hasNext() );
	QVERIFY( fProxy.next() == rec3 );
	QVERIFY( !fProxy.hasNext() );
}

QTEST_KDEMAIN(DataProxyTest, NoGUI)

#include "dataproxytest.moc"
