#include <kapplication.h>

#include "phonenumber.h"

using namespace KABC;

PhoneNumber::PhoneNumber() :
  mType( Home )
{
  init();
}

PhoneNumber::PhoneNumber( const QString &number, int type ) :
  mType( type ), mNumber( number )
{
  init();
}

PhoneNumber::~PhoneNumber()
{
}

void PhoneNumber::init()
{
  mId = KApplication::randomString(8);
}


void PhoneNumber::setId( const QString &id )
{
  mId = id;
}

QString PhoneNumber::id() const
{
  return mId;
}

void PhoneNumber::setNumber( const QString &number )
{
  mNumber = number;
}

QString PhoneNumber::number() const
{
  return mNumber;
}

void PhoneNumber::setType( int type )
{
  mType = type;
}

int PhoneNumber::type() const
{
  return mType;
}
