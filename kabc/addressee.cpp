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


void Addressee::setFormattedName( const QString &formattedName )
{
  if ( formattedName == mData->formattedName ) return;
  detach();
  mData->formattedName = formattedName;
}

QString Addressee::formattedName() const
{
  return mData->formattedName;
}


void Addressee::setFamilyName( const QString &familyName )
{
  if ( familyName == mData->familyName ) return;
  detach();
  mData->familyName = familyName;
}

QString Addressee::familyName() const
{
  return mData->familyName;
}


void Addressee::setGivenName( const QString &givenName )
{
  if ( givenName == mData->givenName ) return;
  detach();
  mData->givenName = givenName;
}

QString Addressee::givenName() const
{
  return mData->givenName;
}


void Addressee::setAdditionalName( const QString &additionalName )
{
  if ( additionalName == mData->additionalName ) return;
  detach();
  mData->additionalName = additionalName;
}

QString Addressee::additionalName() const
{
  return mData->additionalName;
}


void Addressee::setPrefix( const QString &prefix )
{
  if ( prefix == mData->prefix ) return;
  detach();
  mData->prefix = prefix;
}

QString Addressee::prefix() const
{
  return mData->prefix;
}


void Addressee::setSuffix( const QString &suffix )
{
  if ( suffix == mData->suffix ) return;
  detach();
  mData->suffix = suffix;
}

QString Addressee::suffix() const
{
  return mData->suffix;
}


void Addressee::setNickName( const QString &nickName )
{
  if ( nickName == mData->nickName ) return;
  detach();
  mData->nickName = nickName;
}

QString Addressee::nickName() const
{
  return mData->nickName;
}


void Addressee::setBirthday( const QDateTime &birthday )
{
  if ( birthday == mData->birthday ) return;
  detach();
  mData->birthday = birthday;
}

QDateTime Addressee::birthday() const
{
  return mData->birthday;
}


void Addressee::setMailer( const QString &mailer )
{
  if ( mailer == mData->mailer ) return;
  detach();
  mData->mailer = mailer;
}

QString Addressee::mailer() const
{
  return mData->mailer;
}


void Addressee::setTimeZone( const TimeZone &timeZone )
{
  if ( timeZone == mData->timeZone ) return;
  detach();
  mData->timeZone = timeZone;
}

TimeZone Addressee::timeZone() const
{
  return mData->timeZone;
}


void Addressee::setGeo( const Geo &geo )
{
  if ( geo == mData->geo ) return;
  detach();
  mData->geo = geo;
}

Geo Addressee::geo() const
{
  return mData->geo;
}


void Addressee::setTitle( const QString &title )
{
  if ( title == mData->title ) return;
  detach();
  mData->title = title;
}

QString Addressee::title() const
{
  return mData->title;
}


void Addressee::setRole( const QString &role )
{
  if ( role == mData->role ) return;
  detach();
  mData->role = role;
}

QString Addressee::role() const
{
  return mData->role;
}


void Addressee::setOrganization( const QString &organization )
{
  if ( organization == mData->organization ) return;
  detach();
  mData->organization = organization;
}

QString Addressee::organization() const
{
  return mData->organization;
}


void Addressee::setCategories( const QStringList &categories )
{
  if ( categories == mData->categories ) return;
  detach();
  mData->categories = categories;
}

QStringList Addressee::categories() const
{
  return mData->categories;
}


void Addressee::setNote( const QString &note )
{
  if ( note == mData->note ) return;
  detach();
  mData->note = note;
}

QString Addressee::note() const
{
  return mData->note;
}


void Addressee::setProductId( const QString &productId )
{
  if ( productId == mData->productId ) return;
  detach();
  mData->productId = productId;
}

QString Addressee::productId() const
{
  return mData->productId;
}


void Addressee::setRevision( const QDateTime &revision )
{
  if ( revision == mData->revision ) return;
  detach();
  mData->revision = revision;
}

QDateTime Addressee::revision() const
{
  return mData->revision;
}


void Addressee::setSortString( const QString &sortString )
{
  if ( sortString == mData->sortString ) return;
  detach();
  mData->sortString = sortString;
}

QString Addressee::sortString() const
{
  return mData->sortString;
}


void Addressee::setUrl( const KURL &url )
{
  if ( url == mData->url ) return;
  detach();
  mData->url = url;
}

KURL Addressee::url() const
{
  return mData->url;
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

void Addressee::dump() const
{
  kdDebug() << "Addressee {" << endl;

  kdDebug() << "  Uid: '" << uid() << "'" << endl;
  kdDebug() << "  Name: '" << name() << "'" << endl;
  kdDebug() << "  Email: '" << email() << "'" << endl;
  kdDebug() << "  FormattedName: '" << formattedName() << "'" << endl;
  kdDebug() << "  FamilyName: '" << familyName() << "'" << endl;
  kdDebug() << "  GivenName: '" << givenName() << "'" << endl;
  kdDebug() << "  AdditionalName: '" << additionalName() << "'" << endl;
  kdDebug() << "  Prefix: '" << prefix() << "'" << endl;
  kdDebug() << "  Suffix: '" << suffix() << "'" << endl;
  kdDebug() << "  NickName: '" << nickName() << "'" << endl;
  kdDebug() << "  Birthday: '" << birthday().toString() << "'" << endl;
  kdDebug() << "  Mailer: '" << mailer() << "'" << endl;
  kdDebug() << "  TimeZone: '" << timeZone().asString() << "'" << endl;
  kdDebug() << "  Geo: '" << geo().asString() << "'" << endl;
  kdDebug() << "  Title: '" << title() << "'" << endl;
  kdDebug() << "  Role: '" << role() << "'" << endl;
  kdDebug() << "  Organization: '" << organization() << "'" << endl;
  kdDebug() << "  Categories: '" << categories().join(",") << "'" << endl;
  kdDebug() << "  Note: '" << note() << "'" << endl;
  kdDebug() << "  ProductId: '" << productId() << "'" << endl;
  kdDebug() << "  Revision: '" << revision().toString() << "'" << endl;
  kdDebug() << "  SortString: '" << sortString() << "'" << endl;
  kdDebug() << "  Url: '" << url().url() << "'" << endl;
  
  kdDebug() << "  PhoneNumbers {" << endl;
  PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it;
  for( it = p.begin(); it != p.end(); ++it ) {
    kdDebug() << "    Type: " << int((*it).type()) << " Number: " << (*it).number() << endl;
  }
  kdDebug() << "  }" << endl;

  Address::List a = addresses();
  Address::List::ConstIterator it2;
  for( it2 = a.begin(); it2 != a.end(); ++it2 ) {
    (*it2).dump();
  }

  kdDebug() << "}" << endl;
}


void Addressee::insertAddress( const Address &address )
{
  detach();

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      *it = address;
      return;
    }
  }
  mData->addresses.append( address );
}

void Addressee::removeAddress( const Address &address )
{
  detach();

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      mData->addresses.remove( it );
      return;
    }
  }
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

Address Addressee::findAddress( const Address &a ) const
{
  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == a.id() ) {
      return *it;
    }
  }
  return Address();
}
