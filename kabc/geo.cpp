#include "geo.h"

using namespace KABC;

Geo::Geo() :
  mLatitude( 0 ), mLongitude( 0 ), mValidLat( false ), mValidLong( false )
{
}

Geo::Geo( float latitude, float longitude ) :
  mLatitude( latitude ), mLongitude( longitude ),
  mValidLat( true ), mValidLong( true )
{
}

void Geo::setLatitude( float latitude )
{
  mLatitude = latitude;
  mValidLat = true;
}

float Geo::latitude() const
{
  return mLatitude;
}

void Geo::setLongitude( float longitude)
{
  mLongitude = longitude;
  mValidLong = true;
}

float Geo::longitude() const
{
  return mLongitude;
}

bool Geo::isValid() const
{
  return mValidLat && mValidLong;
}

bool Geo::operator==( const Geo &g ) const
{
  if ( !g.isValid() || !isValid() ) return false;
  if ( g.mLatitude == mLatitude && g.mLongitude == mLongitude ) return true;
  return false;
}

QString Geo::asString() const
{
  return "(" + QString::number(mLatitude) + "," + QString::number(mLongitude) + ")";
}
