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


void Addressee::setUid( const QString &uid )
{
  if ( uid == mData->uid ) return;
  detach();
  mData->uid = uid;
}

QString Addressee::uid() const
{
  return mData->uid;
}

void Addressee::setName( const QString &name )
{
  if ( name == mData->name ) return;
  detach();
  mData->name = name;  
}

QString Addressee::name() const
{
  return mData->name;
}


void Addressee::setFormattedName( const QString formattedName )
{
  if ( formattedName == mData->formattedName ) return;
  detach();
  mData->formattedName = formattedName;
}

QString Addressee::formattedName() const
{
  return mData->formattedName;
}


void Addressee::setEmail( const QString &email )
{
  if ( email == mData->email ) return;
  detach();
  mData->email = email;
}

QString Addressee::email() const
{
  return mData->email;
}


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

void Addressee::setUrl( const KURL &url )
{
  detach();
  mData->url = url;
}

KURL Addressee::url() const
{
  return mData->url;
}

void Addressee::dump() const
{
  kdDebug() << "Addressee {" << endl;
  kdDebug() << "  Uid: '" << uid() << "'" << endl;
  kdDebug() << "  Name: '" << name() << "'" << endl;
  kdDebug() << "  FormattedName: '" << formattedName() << "'" << endl;
  kdDebug() << "  Email: '" << email() << "'" << endl;
  kdDebug() << "  URL: '" << url().url() << "'" << endl;
  
  kdDebug() << "  PhoneNumbers {" << endl;
  PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it;
  for( it = p.begin(); it != p.end(); ++it ) {
    kdDebug() << "    Type: " << int((*it).type()) << " Number: " << (*it).number() << endl;
  }
  kdDebug() << "  }" << endl;

  kdDebug() << "}" << endl;
}
