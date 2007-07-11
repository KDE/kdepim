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
	source.mappings()->insert( CSL1( "test-pc" ) , CSL1( "test-hh" ) );
	
	source.saveMapping();
	
	QString path = KGlobal::dirs()->saveLocation( "data", 
		CSL1("kpilot/conduits/") );

	QDir dir( path );
	dir.cd( fUser + CSL1( "/mapping" ) );
	
	QVERIFY( dir.exists( fConduit + CSL1( "-mapping.xml" ) ) );
	
	IDMappingXmlSource source2( fUser, fConduit );
	source2.loadMapping();
	
	// There should be a backup now.
	QVERIFY( dir.exists( fConduit + CSL1( "-mapping.xml~" ) ) );
	
	QVERIFY( pc == source2.lastSyncedPC() );
	QVERIFY( dt == source2.lastSyncedDate() );
	QVERIFY( source2.mappings()->size() == 1 );
	QVERIFY( source2.mappings()->value( CSL1( "test-pc" ) ) 
		== CSL1( "test-hh" ) );
}

void TestIDMappingXmlSource::cleanDir()
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

void TestIDMappingXmlSource::cleanupTestCase()
{
	cleanDir();
}

 
QTEST_KDEMAIN(TestIDMappingXmlSource, NoGUI)

#include "idmappinggxmlsourcetest.moc"
