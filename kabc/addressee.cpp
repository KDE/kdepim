/*
    This file is part of libkabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
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

bool Addressee::isEmpty()
{
  if ( mData->name.isEmpty() && mData->formattedName.isEmpty() &&
       mData->emails.isEmpty() ) return true;
       
  return false;
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



QString Addressee::realName() const
{
  if ( !formattedName().isEmpty() ) return formattedName();

  QString n = prefix() + " " + givenName() + " " + additionalName() + " " +
              familyName() + " " + suffix();
  n = n.simplifyWhiteSpace();             

  if ( n.isEmpty() ) n = name();
  
  return n;
}

void Addressee::insertEmail( const QString &email, bool preferred )
{
  detach();

  QStringList::Iterator it = mData->emails.find( email );

  if ( it != mData->emails.end() ) {
    if ( !preferred || it == mData->emails.begin() ) return;
    mData->emails.remove( it );
    mData->emails.prepend( email );
  } else {
    if ( preferred ) {
      mData->emails.prepend( email );
    } else {
      mData->emails.append( email );
    }
  }
}

void Addressee::removeEmail( const QString &email )
{
  detach();

  QStringList::Iterator it = mData->emails.find( email );
  if ( it == mData->emails.end() ) return;

  mData->emails.remove( it );
}

QString Addressee::preferredEmail() const
{
  if ( mData->emails.count() == 0 ) return QString::null;
  else return mData->emails.first();
}

QStringList Addressee::emails() const
{
  return mData->emails;
}

void Addressee::insertPhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      *it = phoneNumber;
      return;
    }
  }
  mData->phoneNumbers.append( phoneNumber );
}

void Addressee::removePhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      mData->phoneNumbers.remove( it );
      return;
    }
  }
}

PhoneNumber Addressee::phoneNumber( int type ) const
{
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).type() & type ) {
      return *it;
    }
  }
  return PhoneNumber();
}

PhoneNumber::List Addressee::phoneNumbers() const
{
  return mData->phoneNumbers;
}

PhoneNumber Addressee::findPhoneNumber( const QString &id ) const
{
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return PhoneNumber();
}


void Addressee::dump() const
{
  kdDebug() << "Addressee {" << endl;

  kdDebug() << "  Uid: '" << uid() << "'" << endl;
  kdDebug() << "  Name: '" << name() << "'" << endl;
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
    if ( (*it).type() & type ) {
      return *it;
    }
  }
  return Address();
}

Address::List Addressee::addresses() const
{
  return mData->addresses;
}

Address Addressee::findAddress( const QString &id ) const
{
  Address::List::ConstIterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Address();
}

void Addressee::insertCategory( const QString &c )
{
  detach();

  if ( mData->categories.contains( c ) ) return;

  mData->categories.append( c );
}

void Addressee::removeCategory( const QString &c )
{
  detach();

  QStringList::Iterator it = mData->categories.find( c );
  if ( it == mData->categories.end() ) return;

  mData->categories.remove( it );
}

bool Addressee::hasCategory( const QString &c ) const
{
  return ( mData->categories.find( c ) != mData->categories.end() );
}

void Addressee::setCategories( const QStringList &c )
{
  detach();

  mData->categories = c;
}

QStringList Addressee::categories() const
{
  return mData->categories;
}

void Addressee::insertCustom( const QString &app, const QString &name,
                              const QString &value )
{
  detach();
  
  QString qualifiedName = app + "-" + name + ":";
  
  QStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      (*it) = qualifiedName + value;
      return;
    }
  }
  
  mData->custom.append( qualifiedName + value );
}

void Addressee::removeCustom( const QString &app, const QString &name)
{
  detach();
  
  QString qualifiedName = app + "-" + name + ":";
  
  QStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      mData->custom.remove( it );
      return;
    }
  }
}

QString Addressee::custom( const QString &app, const QString &name ) const
{
  QString qualifiedName = app + "-" + name + ":";
  QString value;
  
  QStringList::ConstIterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      value = (*it).mid( (*it).find( ":" ) + 1 );
      break;
    }
  }
  
  return value;
}

void Addressee::setCustoms( const QStringList &l )
{
  detach();
  
  mData->custom = l;
}

QStringList Addressee::customs() const
{
  return mData->custom;
}
