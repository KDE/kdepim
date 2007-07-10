#include <QtTest>
#include <QtCore>
 
class TestIDMappingXmlSource: public QObject
{
	Q_OBJECT
private slots:
	void testValidity();
	void testMonth();
};
 
void TestIDMappingXmlSource::testValidity()
{
	// 11 March 1967
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

#include "tutorial1.moc"
