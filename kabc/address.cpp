#include <kapplication.h>
#include <kdebug.h>

#include "address.h"

using namespace KABC;

Address::Address() :
  mType( 0 )
{
  mId = KApplication::randomString( 10 );
}

void Address::setId( const QString &id )
{
  mId = id;
}

QString Address::id() const
{
  return mId;
}

void Address::setType( int type )
{
  mType = type;
}

int Address::type() const
{
  return mType;
}

void Address::setPostOfficeBox( const QString &s )
{
  mPostOfficeBox = s;
}

QString Address::postOfficeBox() const
{
  return mPostOfficeBox;
}


void Address::setExtended( const QString &s )
{
  mExtended = s;
}

QString Address::extended() const
{
  return mExtended;
}


void Address::setStreet( const QString &s )
{
  mStreet = s;
}

QString Address::street() const
{
  return mStreet;
}


void Address::setLocality( const QString &s )
{
  mLocality = s;
}

QString Address::locality() const
{
  return mLocality;
}


void Address::setRegion( const QString &s )
{
  mRegion = s;
}

QString Address::region() const
{
  return mRegion;
}


void Address::setPostalCode( const QString &s )
{
  mPostalCode = s;
}

QString Address::postalCode() const
{
  return mPostalCode;
}


void Address::setCountry( const QString &s )
{
  mCountry = s;
}

QString Address::country() const
{
  return mCountry;
}


void Address::setLabel( const QString &s )
{
  mLabel = s;
}

QString Address::label() const
{
  return mLabel;
}

void Address::dump() const
{
  kdDebug() << "  Address {" << endl;
  kdDebug() << "    Id: " << id() << endl;
  kdDebug() << "    Extended: " << extended() << endl;
  kdDebug() << "    Street: " << street() << endl;
  kdDebug() << "    Postal Code: " << postalCode() << endl;
  kdDebug() << "    Locality: " << locality() << endl;
  kdDebug() << "  }" << endl;
}
