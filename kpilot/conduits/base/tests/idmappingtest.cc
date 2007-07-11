#include <QtTest>
#include <QtCore>

#include <options.h>
#include <idmapping.h>

#include <KGlobal>
#include <KStandardDirs>

#include "qtest_kde.h"

class IDMappingTest : public QObject
{
	Q_OBJECT
	
public:
	IDMappingTest();
	
private slots:
	void testMap();
	void testRemove();
	void testIsValid();
	void testRecordId();
	void testCommitRollBack();
	void cleanupTestCase();
	

private:
	void cleanDir();
	
	QString fUser;
	QString fConduit;
};

IDMappingTest::IDMappingTest() : fUser( "test-user" )
	, fConduit("test-conduit")
{
}

void IDMappingTest::testMap()
{
	IDMapping mapping( fUser, fConduit );
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	
	QVERIFY( mapping.contains( CSL1( "hh-1" ) ) );
	QVERIFY( mapping.contains( CSL1( "pc-100" ) ) );
}

void IDMappingTest::testRemove()
{
	IDMapping mapping( fUser, fConduit );
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	mapping.remove( CSL1( "hh-1" ) );
	
	QVERIFY( !mapping.contains( CSL1( "hh-1" ) ) );
	QVERIFY( !mapping.contains( CSL1( "pc-100" ) ) );
	
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	mapping.remove( CSL1( "pc-100" ) );
	
	QVERIFY( !mapping.contains( CSL1( "hh-1" ) ) );
	QVERIFY( !mapping.contains( CSL1( "pc-100" ) ) );
}

void IDMappingTest::testIsValid()
{
	IDMapping mapping( fUser, fConduit );
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	mapping.map( CSL1( "hh-2" ), CSL1( "pc-40" ) );
	mapping.map( CSL1( "hh-3" ), CSL1( "pc-46" ) );
	
	QStringList hhIds;
	hhIds << CSL1( "hh-1" ) << CSL1( "hh-2" ) << CSL1( "hh-3" );
	
	QStringList pcIds;
	pcIds << CSL1( "pc-100" ) << CSL1( "pc-40" ) << CSL1( "pc-46" );
	
	/*
	 * This is a bit tricky. There are three id's in the list and there are three
	 * mappings. Also for each id in this list exists a mapping. BUT the ids of 
	 * hh and are mixed here and have a mapping all by coincidence, however this
	 * does not garantee that the mapping is valid. If for example "hh-1" maps to
	 * "pc-78" the mapping is invalid because it should map to "pc-100"
	 */
	QStringList trickyIds;
	trickyIds << CSL1( "hh-1" ) << CSL1( "pc-40" ) << CSL1( "pc-46" );
	
	QStringList randomIds;
	randomIds << CSL1( "random-2" ) << CSL1( "random-14" );
	randomIds	<< CSL1( "random-109" );
	
	QVERIFY( mapping.isValid( hhIds ) );
	QVERIFY( mapping.isValid( pcIds ) );
	QVERIFY( !mapping.isValid( trickyIds ) );
	QVERIFY( !mapping.isValid( randomIds ) );
}

void IDMappingTest::testRecordId()
{
	IDMapping mapping( fUser, fConduit );
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	mapping.map( CSL1( "hh-2" ), CSL1( "pc-40" ) );
	
	QVERIFY( mapping.recordId( CSL1( "hh-1" ) ) == CSL1( "pc-100" ) );
	QVERIFY( mapping.recordId( CSL1( "pc-100" ) ) == CSL1( "hh-1" ) );
	QVERIFY( mapping.recordId( CSL1( "hh-2" ) ) == CSL1( "pc-40" ) );
	QVERIFY( mapping.recordId( CSL1( "pc-40" ) ) == CSL1( "hh-2" ) );
}

void IDMappingTest::testCommitRollBack()
{
	// Clean up things to ensure this test won't be influenced by earlier data
	// which might be there.
	cleanDir();
	
	IDMapping mapping( fUser, fConduit );
	mapping.map( CSL1( "hh-1" ), CSL1( "pc-100" ) );
	mapping.map( CSL1( "hh-2" ), CSL1( "pc-40" ) );
	mapping.commit();
	
	// Should contain 2 mappings
	IDMapping mapping2( fUser, fConduit );
	
	QStringList hhIds, hhIds2;
	hhIds << CSL1( "hh-1" ) << CSL1( "hh-2" );
	hhIds2 << hhIds << CSL1( "hh-3" );
	
	QVERIFY( mapping.isValid( hhIds ) );
	
	mapping2.map( CSL1( "hh-3" ), CSL1( "pc-23" ) );
	mapping2.commit();
	
	QVERIFY( mapping2.isValid( hhIds2 ) );
	
	mapping2.rollback();
	
	QVERIFY( mapping2.isValid( hhIds ) );
	
	IDMapping mapping3( fUser, fConduit );
	
	QVERIFY( mapping3.isValid( hhIds ) );
}

void IDMappingTest::cleanDir()
{
	QString pathName = KGlobal::dirs()->saveLocation( "data",
		CSL1("kpilot/conduits/"));
		
	QDir dir( pathName );
	dir.cd( fUser );
	dir.cd( CSL1( "mapping" ) );
	dir.remove( fConduit + CSL1( "-mapping.xml" ) );
	dir.remove(  fConduit + CSL1( "-mapping.xml~" ) );
	dir.remove(  fConduit + CSL1( "-mapping.xml.fail" ) );
	dir.cd( CSL1( ".." ) );
	dir.rmdir( CSL1( "mapping" ) );
	dir.cd( CSL1( ".." ) );
	dir.rmdir( fUser );
}

void IDMappingTest::cleanupTestCase()
{
	//cleanDir();
}

 
QTEST_KDEMAIN(IDMappingTest, NoGUI)

#include "idmappingtest.moc"
