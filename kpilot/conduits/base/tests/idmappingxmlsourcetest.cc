/* idmappingxmlsourcetests.cc			KPilot
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

#include <idmappingxmlsource.h>
#include <options.h>

#include <KGlobal>
#include <KStandardDirs>

#include "qtest_kde.h"

class TestIDMappingXmlSource: public QObject
{
	Q_OBJECT
	
public:
	TestIDMappingXmlSource();
	
private slots:
	void testConstructor();
	void testLastSyncedDate();
	void testLastSyncedPC();
	void testMappings();
	void testSaveLoad();
	void testHHCategory();
	void cleanupTestCase();

private:
	void cleanDir();
	
	QString fUser;
	QString fConduit;
};

TestIDMappingXmlSource::TestIDMappingXmlSource() : fUser( "test-user" )
	, fConduit("test-conduit")
{
}

void TestIDMappingXmlSource::testConstructor()
{
	QString pathName = KGlobal::dirs()->saveLocation( "data",
		CSL1("kpilot/conduits/"));

	IDMappingXmlSource source( fUser, fConduit );
	
	QDir dir( pathName );
	QVERIFY( dir.exists( fUser ) );
	
	dir.cd( fUser );
	QVERIFY( dir.exists( CSL1( "mapping" ) ) );
}

void TestIDMappingXmlSource::testLastSyncedDate()
{
	QDateTime dt = QDateTime::currentDateTime();
	
	IDMappingXmlSource source( fUser, fConduit );
	source.setLastSyncedDate( dt );
	
	QDateTime dtSource = source.lastSyncedDate();
	QVERIFY( dt == dtSource );
}

void TestIDMappingXmlSource::testLastSyncedPC()
{
	QString pc( CSL1( "test-pc" ) );
	
	IDMappingXmlSource source( fUser, fConduit );
	source.setLastSyncedPC( pc );
	
	QString pcSource = source.lastSyncedPC();
	QVERIFY( pc == pcSource );
}

void TestIDMappingXmlSource::testMappings()
{
	IDMappingXmlSource source( fUser, fConduit );
	
	QMap<QString, QString> *mappings = source.mappings();
	mappings->insert( CSL1( "test-pc" ) , CSL1( "test-hh" ) );
	
	QMap<QString, QString> *mappings2 = source.mappings();
	
	QVERIFY( mappings2->size() == mappings->size() );
	
	QVERIFY( mappings2->value( CSL1( "test-pc" ) ) == CSL1( "test-hh" ) );
}

void TestIDMappingXmlSource::testSaveLoad()
{
	QDateTime dt = QDateTime::currentDateTime();
	// Correct msecs, these wont be saved and the test 
	// dt == source2.lastSyncedDate() will fail.
	dt = dt.addMSecs( -dt.time().msec() );
	QString pc( CSL1( "test-pc" ) );

	IDMappingXmlSource source( fUser, fConduit );
	source.setLastSyncedDate( dt );
	source.setLastSyncedPC( pc );
	source.mappings()->insert( CSL1( "test-hh" ), CSL1( "test-pc" ) );
	source.mappings()->insert( CSL1( "test-hh2" ), CSL1( "test-pc2" ) );
	source.archivedRecords()->append( "test-pc2" );
	
	source.saveMapping();
	
	QString path = KGlobal::dirs()->saveLocation( "data", 
		CSL1("kpilot/conduits/") );

	QDir dir( path );
	dir.cd( fUser + CSL1( "/mapping" ) );
	
	QVERIFY( dir.exists( fConduit + CSL1( "-mapping.xml" ) ) );
	
	IDMappingXmlSource source2( fUser, fConduit );
	source2.loadMapping();
	
	// There should be a backup now.
	QVERIFY( dir.exists( fConduit + CSL1( "-mapping.xml-backup" ) ) );
	
	QVERIFY( pc == source2.lastSyncedPC() );
	QVERIFY( dt == source2.lastSyncedDate() );
	QVERIFY( source2.mappings()->size() == 2 );
	QVERIFY( source2.mappings()->value( CSL1( "test-hh" ) ) 
		== CSL1( "test-pc" ) );
	QVERIFY( source2.mappings()->value( CSL1( "test-hh2" ) ) 
		== CSL1( "test-pc2" ) );
		
	QVERIFY( source2.archivedRecords()->size() == 1 );
	QVERIFY( source2.archivedRecords()->contains( CSL1( "test-pc2" ) ) );
}

void TestIDMappingXmlSource::testHHCategory()
{
	
	QDateTime dt = QDateTime::currentDateTime();
	// Correct msecs, these wont be saved and the test 
	// dt == source2.lastSyncedDate() will fail.
	dt = dt.addMSecs( -dt.time().msec() );
	QString pc( CSL1( "test-pc" ) );

	IDMappingXmlSource source( fUser, fConduit );
	source.setLastSyncedDate( dt );
	source.setLastSyncedPC( pc );
	source.mappings()->insert( CSL1( "test-hh" ), CSL1( "test-pc" ) );
	source.setHHCategory( CSL1( "test-hh" ), CSL1( "TestCategory" ) );
	source.saveMapping();
	
	IDMappingXmlSource source2( fUser, fConduit );
	source2.loadMapping();
	
	QVERIFY( source2.hhCategory( "test-hh" ) == "TestCategory" );
}

void TestIDMappingXmlSource::cleanDir()
{
	QString pathName = KGlobal::dirs()->saveLocation( "data",
		CSL1("kpilot/conduits/"));

	QDir dir( pathName );
	dir.cd( fUser );
	dir.cd( CSL1( "mapping" ) );
	dir.remove( fConduit + CSL1( "-mapping.xml" ) );
	dir.remove(  fConduit + CSL1( "-mapping.xml-backup" ) );
	dir.remove(  fConduit + CSL1( "-mapping.xml.fail" ) );
	dir.cd( CSL1( ".." ) );
	dir.rmdir( CSL1( "mapping" ) );
	dir.cd( CSL1( ".." ) );
	dir.rmdir( fUser );
}

void TestIDMappingXmlSource::cleanupTestCase()
{
	cleanDir();
}

 
QTEST_KDEMAIN(TestIDMappingXmlSource, NoGUI)

#include "idmappinggxmlsourcetest.moc"
