#include <ksharedptr.h>
#include <kdebug.h>
#include <kapplication.h>

#include "phonenumber.h"

#include "addressee.h"

using namespace KABC;


Addressee::Addressee()
{
  mData = new AddresseeData;
  mData->uid = KApplication::randomString( 10 );
}

Addressee::~Addressee()
{
}

Addressee::Addressee( const Addressee &a )
{
  mData = a.mData;
}

Addressee &Addressee::operator=( const Addressee &a )
{
  mData = a.mData;
  return (*this);
}

Addressee Addressee::copy()
{
  Addressee a;
  a.mData = new AddresseeData;
  *(a.mData) = *mData;
  return a;
}

void Addressee::detach()
{
  *this = copy();
}

--DEFINITIONS--

void Addressee::insertPhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).type() == phoneNumber.type() ) {
      *it = phoneNumber;
      return;
    }
  }
  mData->phoneNumbers.append( phoneNumber );
}

PhoneNumber Addressee::phoneNumber( PhoneNumber::Type type ) const
{
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).type() == type ) {
      return *it;
    }
  }
  return PhoneNumber();
}

PhoneNumber::List Addressee::phoneNumbers() const
{
  return mData->phoneNumbers;
}

void Addressee::dump() const
{
  kdDebug() << "Addressee {" << endl;

  --DEBUG--
  
  kdDebug() << "  PhoneNumbers {" << endl;
  PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it;
  for( it = p.begin(); it != p.end(); ++it ) {
    kdDebug() << "    Type: " << int((*it).type()) << " Number: " << (*it).number() << endl;
  }
  kdDebug() << "  }" << endl;

  kdDebug() << "}" << endl;
}


void Addressee::insertAddress( const Address &address )
{
  detach();

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).type() == address.type() ) {
      *it = address;
      return;
    }
  }
  mData->addresses.append( address );  
}

Address Addressee::address( int type ) const
{
  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).type() == type ) {
      return *it;
    }
  }
  return Address();
}

Address::List Addressee::addresses() const
{
  return mData->addresses;
}
