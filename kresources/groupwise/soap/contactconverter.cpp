/*
    This file is part of kdepim.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "contactconverter.h"

#include <kdebug.h>

ContactConverter::ContactConverter( struct soap* soap )
  : GWConverter( soap )
{
}

ns1__Contact* ContactConverter::convertToContact( const KABC::Addressee &addr )
{
  if ( addr.isEmpty() )
    return 0;

  ns1__Contact* contact = soap_new_ns1__Contact( soap(), -1 );

  // null pointer initialization
  contact->comment = 0;
  contact->categories = 0;
  contact->created = 0;
  contact->customs = 0;
  contact->modified = 0;
  contact->changes = 0;
  contact->type = 0;

  // Uid
  contact->id = addr.custom( "GWRESOURCE", "UID" ).utf8();

  // Container
  if ( !addr.custom( "GWRESOURCE", "CONTAINER" ).isEmpty() ) {
    std::vector<ns1__ContainerRef*>* container = soap_new_std__vectorTemplateOfPointerTons1__ContainerRef( soap(), -1 );
    ns1__ContainerRef* containerRef = soap_new_ns1__ContainerRef( soap(), -1 );
    containerRef->__item = addr.custom( "GWRESOURCE", "CONTAINER" ).utf8();
    container->push_back( containerRef );

    contact->container = container;
  } else
    contact->container = 0;

  // Name parts
  ns1__FullName* fullName = soap_new_ns1__FullName( soap(), -1 );

  if ( !addr.formattedName().isEmpty() )
    fullName->displayName = addr.formattedName().utf8();

  if ( !addr.prefix().isEmpty() )
    fullName->namePrefix = qStringToString( addr.prefix() );
  else
    fullName->namePrefix = 0;

  if ( !addr.givenName().isEmpty() )
    fullName->firstName = qStringToString( addr.givenName() );
  else
    fullName->firstName = 0;

  if ( !addr.additionalName().isEmpty() )
    fullName->middleName = qStringToString( addr.additionalName() );
  else
    fullName->middleName = 0;

  if ( !addr.familyName().isEmpty() )
    fullName->lastName = qStringToString( addr.familyName() );
  else
    fullName->lastName = 0;

  if ( !addr.suffix().isEmpty() )
    fullName->nameSuffix = qStringToString( addr.suffix() );
  else
    fullName->nameSuffix = 0;

  contact->fullName = fullName;

  // Emails
  if ( !addr.emails().isEmpty() ) {
    ns1__EmailAddressList* emailList = soap_new_ns1__EmailAddressList( soap(), -1 );
    std::vector<std::string>* list = soap_new_std__vectorTemplateOfstd__string( soap(), -1 );

    QStringList emails = addr.emails();
    emailList->primary = emails.first().utf8();

    QStringList::Iterator it;
    for ( it = emails.begin(); it != emails.end(); ++it )
      list->push_back( std::string( (*it).utf8() ) );

    emailList->email = list;
    contact->emailList = emailList;
  } else
    contact->emailList = 0;

  // Phone numbers
  if ( !addr.phoneNumbers().isEmpty() ) {
    ns1__PhoneList* phoneList = soap_new_ns1__PhoneList( soap(), -1 );
    std::vector<class ns1__PhoneNumber*> *list = soap_new_std__vectorTemplateOfPointerTons1__PhoneNumber( soap(), -1 );

    KABC::PhoneNumber::List phones = addr.phoneNumbers();
    KABC::PhoneNumber::List::Iterator it;
    for ( it = phones.begin(); it != phones.end(); ++it ) {
      ns1__PhoneNumber* number = convertPhoneNumber( *it );
      if ( number ) {
        list->push_back( number );

        // if preferred number assign it
        if ( (*it).type() & KABC::PhoneNumber::Pref )
          phoneList->default_ = (*it).number().utf8();
      }
    }

    phoneList->phone = list;
    contact->phoneList = phoneList;
  } else
    contact->phoneList = 0;

  // Addresses
  if ( !addr.addresses().isEmpty() ) {
    ns1__PostalAddressList* addressList = soap_new_ns1__PostalAddressList( soap(), -1 );
    std::vector<class ns1__PostalAddress*> *list = soap_new_std__vectorTemplateOfPointerTons1__PostalAddress( soap(), -1 );

    KABC::Address::List addresses = addr.addresses();
    KABC::Address::List::Iterator it;
    for ( it = addresses.begin(); it != addresses.end(); ++it ) {
      ns1__PostalAddress* address = convertPostalAddress( *it );
      if ( address )
        list->push_back( address );
    }

    addressList->address = list;
    contact->addressList = addressList;
  } else
    contact->addressList = 0;

  // IM addresses
  contact->imList = 0;

  // Office information
  {
    ns1__OfficeInfo* info = soap_new_ns1__OfficeInfo( soap(), -1 );

    // TODO: write back organization?

    if ( !addr.custom( "KADDRESSBOOK", "X-Department" ).isEmpty() )
      info->department = qStringToString( addr.custom( "KADDRESSBOOK", "X-Department" ) );
    else
      info->department = 0;

    if ( !addr.title().isEmpty() )
      info->title = qStringToString( addr.title() );
    else
      info->title = 0;

    if ( !addr.url().isEmpty() )
      info->website = qStringToChar( addr.url().url() );
    else
      info->website = 0;

    info->organization = 0;

    contact->officeInfo = info;
  }

  // Personal information
  {
    ns1__PersonalInfo* info = soap_new_ns1__PersonalInfo( soap(), -1 );

    if ( addr.birthday().isValid() )
      info->birthday = qDateToChar( addr.birthday().date() );
    else
      info->birthday = 0;

    // don't write back the web site here, otherwise we'll have it twice
    info->website = 0;

    contact->personalInfo = info;
  }

  return contact;
}

KABC::Addressee ContactConverter::convertFromContact( ns1__Contact* contact )
{
  kdDebug() << "ContactConverter::convertFromContact()" << endl;

  KABC::Addressee addr;

  if ( !contact )
    return addr;

  addr.insertCustom( "GWRESOURCE", "UID", stringToQString( contact->id ) );

  // Name parts
  ns1__FullName* fullName = contact->fullName;

  if ( !fullName->displayName.empty() )
    addr.setFormattedName( stringToQString( fullName->displayName ) );

  if ( fullName->namePrefix )
    addr.setPrefix( stringToQString( fullName->namePrefix ) );

  if ( fullName->firstName )
    addr.setGivenName( stringToQString( fullName->firstName ) );

  if ( fullName->middleName )
    addr.setAdditionalName( stringToQString( fullName->middleName ) );

  if ( fullName->lastName )
    addr.setFamilyName( stringToQString( fullName->lastName ) );

  if ( fullName->nameSuffix )
    addr.setSuffix( stringToQString( fullName->nameSuffix ) );

  // Emails
  if ( contact->emailList && contact->emailList->email ) {
    QStringList emails;

    if ( !contact->emailList->primary.empty() )
      emails.append( stringToQString( contact->emailList->primary ) );

    std::vector<std::string> *list = contact->emailList->email;
    std::vector<std::string>::const_iterator it;
    for ( it = list->begin(); it != list->end(); ++it ) {
      if ( emails.find( stringToQString( *it ) ) == emails.end() )
        emails.append( stringToQString( *it ) );
    }

    addr.setEmails( emails );
  }

  // Phone numbers
  if ( contact->phoneList && contact->phoneList->phone ) {
    QString defaultNumber = stringToQString( contact->phoneList->default_ );

    std::vector<class ns1__PhoneNumber*> *list = contact->phoneList->phone;
    std::vector<class ns1__PhoneNumber*>::const_iterator it;
    for ( it = list->begin(); it != list->end(); ++it ) {
      KABC::PhoneNumber phone = convertPhoneNumber( *it );
      if ( !phone.number().isEmpty() ) {
        if ( phone.number() == defaultNumber )
          phone.setType( phone.type() | KABC::PhoneNumber::Pref );
        addr.insertPhoneNumber( phone );
      }
    }
  }

  // Addresses
  if ( contact->addressList && contact->addressList->address ) {
    std::vector<class ns1__PostalAddress*> *list = contact->addressList->address;
    std::vector<class ns1__PostalAddress*>::const_iterator it;
    for ( it = list->begin(); it != list->end(); ++it ) {
      KABC::Address address = convertPostalAddress( *it );
      if ( !address.isEmpty() )
        addr.insertAddress( address );
    }
  }

  // Office information
  if ( contact->officeInfo ) {
    ns1__OfficeInfo* info = contact->officeInfo;

    if ( info->organization )
      addr.setOrganization( stringToQString( info->organization->__item ) );

    if ( info->department )
      addr.insertCustom( "KADDRESSBOOK", "X-Department", stringToQString( info->department ) );

    if ( info->title )
      addr.setTitle( stringToQString( info->title ) );

    if ( info->website )
      addr.setUrl( KURL( stringToQString( info->website ) ) );
  }

  // Personal information
  if ( contact->personalInfo ) {
    ns1__PersonalInfo* info = contact->personalInfo;

    if ( info->birthday ) {
      QDate date = charToQDate( info->birthday );
      if ( date.isValid() )
        addr.setBirthday( date );
    }

    if ( info->website ) // we might overwrite the office info website here... :(
      addr.setUrl( KURL( stringToQString( info->website ) ) );
  }

  return addr;
}

KABC::PhoneNumber ContactConverter::convertPhoneNumber( ns1__PhoneNumber *phone ) const
{
  KABC::PhoneNumber phoneNumber;

  if ( !phone )
    return phoneNumber;

  phoneNumber.setNumber( stringToQString( phone->__item ) );
  if ( phone->type == Fax ) {
    phoneNumber.setType( KABC::PhoneNumber::Fax );
  } else if ( phone->type == Home ) {
    phoneNumber.setType( KABC::PhoneNumber::Home );
  } else if ( phone->type == Mobile ) {
    phoneNumber.setType( KABC::PhoneNumber::Cell );
  } else if ( phone->type == Office ) {
    phoneNumber.setType( KABC::PhoneNumber::Work );
  } else if ( phone->type == Pager ) {
    phoneNumber.setType( KABC::PhoneNumber::Pager );
  } else {
    // should never been reached, phone numbers have always a type set...
  }

  return phoneNumber;
}

ns1__PhoneNumber* ContactConverter::convertPhoneNumber( const KABC::PhoneNumber &number ) const
{
  if ( number.number().isEmpty() )
    return 0;

  ns1__PhoneNumber* phoneNumber = soap_new_ns1__PhoneNumber( soap(), -1 );
  phoneNumber->__item = number.number().utf8();

  if ( number.type() & KABC::PhoneNumber::Fax ) {
    phoneNumber->type = Fax;
  } else if ( number.type() == KABC::PhoneNumber::Home ) {
    phoneNumber->type = Home;
  } else if ( number.type() & KABC::PhoneNumber::Cell ) {
    phoneNumber->type = Mobile;
  } else if ( number.type() == KABC::PhoneNumber::Work ) {
    phoneNumber->type = Office;
  } else if ( number.type() & KABC::PhoneNumber::Pager ) {
    phoneNumber->type = Pager;
  } else {
    // TODO: cache unsupported types
  }

  return phoneNumber;
}

KABC::Address ContactConverter::convertPostalAddress( ns1__PostalAddress *addr ) const
{
  KABC::Address address;

  if ( !addr )
    return address;

  if ( addr->streetAddress )
    address.setStreet( stringToQString( addr->streetAddress ) );

  if ( addr->location )
    address.setExtended( stringToQString( addr->location ) );

  if ( addr->city ) // isn't city the same like location?
    address.setLocality( stringToQString( addr->city ) );

  if ( addr->state )
    address.setRegion( stringToQString( addr->state ) );

  if ( addr->postalCode )
    address.setPostalCode( stringToQString( addr->postalCode ) );

  if ( addr->country )
    address.setCountry( stringToQString( addr->country ) );

  if ( addr->type == Home_ ) {
    address.setType( KABC::Address::Home );
  } else if ( addr->type == Office_ ) {
    address.setType( KABC::Address::Work );
  } else {
    // should never been reached, addresses have always a type set...
  }

  return address;
}

ns1__PostalAddress* ContactConverter::convertPostalAddress( const KABC::Address &address )
{
  if ( address.isEmpty() )
    return 0;

  ns1__PostalAddress* postalAddress = soap_new_ns1__PostalAddress( soap(), -1 );

  if ( !address.street().isEmpty() )
    postalAddress->streetAddress = qStringToString( address.street() );
  else
    postalAddress->streetAddress = 0;

  if ( !address.extended().isEmpty() )
    postalAddress->location = qStringToString( address.extended() );
  else
    postalAddress->location = 0;

  if ( !address.locality().isEmpty() )
    postalAddress->city = qStringToString( address.locality() );
  else
    postalAddress->city = 0;

  if ( !address.region().isEmpty() )
    postalAddress->state = qStringToString( address.region() );
  else
    postalAddress->state = 0;

  if ( !address.postalCode().isEmpty() )
    postalAddress->postalCode = qStringToString( address.postalCode() );
  else
    postalAddress->postalCode = 0;

  if ( !address.country().isEmpty() )
    postalAddress->country = qStringToString( address.country() );
  else
    postalAddress->country = 0;

  if ( address.type() & KABC::Address::Home ) {
    postalAddress->type = Home_;
  } else if ( address.type() & KABC::Address::Work ) {
    postalAddress->type = Office_;
  } else {
    // TODO: cache unsupported types
  }

  return postalAddress;
}
