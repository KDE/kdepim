#include "timezone.h"

using namespace KABC;

TimeZone::TimeZone() :
  mOffset( 0 ), mValid( false )
{
}

TimeZone::TimeZone( int offset ) :
  mOffset( offset ), mValid( true )
{
}

void TimeZone::setOffset( int offset )
{
  mOffset = offset;
  mValid = true;
}

int TimeZone::offset() const
{
  return mOffset;
}

bool TimeZone::isValid() const
{
  return mValid;
}

bool TimeZone::operator==( const TimeZone &t ) const
{
  if ( !t.isValid() || !isValid() ) return false;
  if ( t.mOffset == mOffset ) return true;
  return false;
}

QString TimeZone::asString() const
{
  return QString::number( mOffset );
}
