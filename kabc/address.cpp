#include "address.h"

using namespace KABC;

Address::Address() :
  mType( 0 )
{
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
