#include <QtTest>
#include <QtCore>

#include "idmappingxmlsource.h"

class TestIDMappingXmlSource: public QObject
{
	Q_OBJECT
private slots:
	void testConstructor();
	void testMonth();
};
 
void TestIDMappingXmlSource::testConstructor()
{
	IDMappingXmlSource source( "testUser", "testConduit" );
	
	QDate date( 1967, 3, 11 );
	QVERIFY( date.isValid() );
}
 
void TestIDMappingXmlSource::testMonth()
{
	// 11 March 1967
	QDate date;
	date.setYMD( 1967, 3, 11 );
	QCOMPARE( date.month(), 3 );
	QCOMPARE( QDate::longMonthName( date.month() ),
		QString("March") );
}
 
QTEST_MAIN(TestIDMappingXmlSource)

#include "idmappubgxmlsourcetest.moc"
