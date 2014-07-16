#include <QtTest/QtTest>

#include "../rangemanager_p.h"
#include "../rangemanager.cpp"

class RangeManagerTest : public QObject
{
  Q_OBJECT

  private Q_SLOTS:
    void basicTest()
    {
      RangeManager manager;
      QCOMPARE( manager.count(), 0 );

      manager.insertRange( 0, 5 );
      QCOMPARE( manager.count(), 1 );

      manager.clear();
      QCOMPARE( manager.count(), 0 );
    }

    void countTest()
    {
      RangeManager manager;
      manager.insertRange( 0, 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );

      QCOMPARE( manager.count(), 5 );
    }

    void clearTest()
    {
      RangeManager manager;

      manager.clear();
      QCOMPARE( manager.count(), 0 );

      manager.insertRange( 0, 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );
      manager.insertRange( manager.count(), 5 );

      manager.clear();
      QCOMPARE( manager.count(), 0 );
    }

    void insertRangeTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 3 );
      manager.insertRange( 2, 7 );

      QCOMPARE( manager.rangeStart( 2 ), 8 );

      manager.insertRange( 1, 5 );

      QCOMPARE( manager.rangeStart( 3 ), 13 );
    }

    void removeRangeTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 5 );
      manager.insertRange( 2, 3 );
      manager.insertRange( 3, 7 );

      QCOMPARE( manager.rangeStart( 3 ), 13 );

      manager.removeRange( 1 );
      QCOMPARE( manager.rangeStart( 2 ), 8 );
    }

    void increaseRangeTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 4 );
      manager.insertRange( 2, 3 );

      QCOMPARE( manager.rangeStart( 2 ), 9 );

      manager.increaseRange( 1, 2 );
      QCOMPARE( manager.rangeStart( 2 ), 11 );
    }

    void decreaseRangeTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 4 );
      manager.insertRange( 2, 3 );

      QCOMPARE( manager.rangeStart( 2 ), 9 );

      manager.decreaseRange( 1, 2 );
      QCOMPARE( manager.rangeStart( 2 ), 7 );
    }

    void rangeStartTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 4 );
      manager.insertRange( 2, 3 );
      manager.insertRange( 3, 8 );

      QCOMPARE( manager.rangeStart( 0 ), 0 );
      QCOMPARE( manager.rangeStart( 1 ), 5 );
      QCOMPARE( manager.rangeStart( 2 ), 9 );
      QCOMPARE( manager.rangeStart( 3 ), 12 );
    }

    void rangeSizeTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 4 );
      manager.insertRange( 2, 3 );
      manager.insertRange( 3, 8 );

      QCOMPARE( manager.rangeSize( 0 ), 5 );
      QCOMPARE( manager.rangeSize( 1 ), 4 );
      QCOMPARE( manager.rangeSize( 2 ), 3 );
      QCOMPARE( manager.rangeSize( 3 ), 8 );
    }

    void rangeForPositionTest()
    {
      RangeManager manager;

      manager.insertRange( 0, 5 );
      manager.insertRange( 1, 4 );
      manager.insertRange( 2, 3 );
      manager.insertRange( 3, 8 );

      QCOMPARE( manager.rangeForPosition( 0 ), 0 );
      QCOMPARE( manager.rangeForPosition( 2 ), 0 );
      QCOMPARE( manager.rangeForPosition( 4 ), 0 );
      QCOMPARE( manager.rangeForPosition( 5 ), 1 );
      QCOMPARE( manager.rangeForPosition( 10 ), 2 );
      QCOMPARE( manager.rangeForPosition( 19 ), 3 );
      QCOMPARE( manager.rangeForPosition( 20 ), -1 );
    }
};

QTEST_MAIN( RangeManagerTest )

#include "rangemanagertest.moc"
