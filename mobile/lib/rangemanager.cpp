
#include "rangemanager_p.h"

#include <QtCore/QStringList>

RangeManager::RangeManager()
{
}

RangeManager::~RangeManager()
{
}

int RangeManager::count() const
{
  return mRangeSizes.count();
}

void RangeManager::clear()
{
  mRangeSizes.clear();
}

void RangeManager::insertRange( int range, int size )
{
  mRangeSizes.insert( range, size );
}

void RangeManager::removeRange( int range )
{
  mRangeSizes.remove( range );
}

void RangeManager::increaseRange( int range, int elements )
{
  mRangeSizes[ range ] += elements;
}

void RangeManager::decreaseRange( int range, int elements )
{
  mRangeSizes[ range ] -= elements;
}

int RangeManager::rangeStart( int range ) const
{
  int start = 0;

  for ( int i = 0; i < range; ++i )
    start += mRangeSizes.at( i );

  return start;
}

int RangeManager::rangeSize( int range ) const
{
  return mRangeSizes.at( range );
}

int RangeManager::rangeForPosition( int position ) const
{
  int start = 0;
  for ( int range = 0; range < mRangeSizes.size(); ++range ) {
    start += mRangeSizes.at( range );
    if ( start > position )
      return range;
  }

  return -1;
}

void RangeManager::dump() const
{
  QStringList output;
  int counter = 0;
  for ( int range = 0; range < mRangeSizes.size(); ++range ) {
    QStringList foo;
    for ( int i = 0; i < mRangeSizes.at( range ); ++i ) {
      foo += QString::number( counter );
      counter++;
    }
    output += foo.join(QLatin1String(" "));
  }

  qDebug("[%s]", qPrintable( output.join(QLatin1String(" | ")) ) );
}
