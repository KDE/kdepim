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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "contactconverter.h"

#include <kdebug.h>
#include <klocale.h>
#include <kdeversion.h>

ContactConverter::ContactConverter( struct soap* soap )
  : GWConverter( soap )
{
}

ngwt__Contact* ContactConverter::convertToContact( const KABC::Addressee &addr )
{
  if ( addr.isEmpty() )
    return 0;

  ngwt__Contact* contact = soap_new_ngwt__Contact( soap(), -1 );

  // ngwt__Contact
  contact->fullName = 0;
  contact->emailList = 0;
  contact->imList = 0;
  contact->addressList = 0;
  contact->officeInfo = 0;
  contact->personalInfo = 0;
  contact->referenceInfo = 0;
  // ngwt__AddressBookItem
  contact->uuid = 0;
  contact->comment = 0;
  contact->sync = 0;
  contact->domain = 0;
  contact->postOffice = 0;
  contact->distinguishedName = 0;
  contact->userid = 0;
  // ngwt__ContainerItem
  contact->categories = 0;
  contact->created = 0;
  contact->customs = 0;
  // ngwt__contact
  contact->id = 0;
  contact->name = 0;
  contact->version = 0;
  contact->modified = 0;
  contact->changes = 0;

  // Uid
  contact->id = qStringToString( addr.custom( "GWRESOURCE", "UID" ) );

  // Container
  if ( !addr.custom( "GWRESOURCE", "CONTAINER" ).isEmpty() ) {
    std::vector<ngwt__ContainerRef*>* container = soap_new_std__vectorTemplateOfPointerTongwt__ContainerRef( soap(), -1 );
    ngwt__ContainerRef* containerRef = soap_new_ngwt__ContainerRef( soap(), -1 );
    containerRef->deleted = 0;
    containerRef->__item = addr.custom( "GWRESOURCE", "CONTAINER" ).toLatin1().data();
    container->push_back( containerRef );

    contact->container = *container;
  }

  // Name parts
  ngwt__FullName* fullName = soap_new_ngwt__FullName( soap(), -1 );
  fullName->displayName = 0;
  fullName->namePrefix = 0;
  fullName->firstName = 0;
  fullName->middleName = 0;
  fullName->lastName = 0;
  fullName->nameSuffix = 0;

  if ( !addr.formattedName().isEmpty() )
    fullName->displayName = qStringToString( addr.formattedName() );

  if ( !addr.prefix().isEmpty() )
    fullName->namePrefix = qStringToString( addr.prefix() );

  if ( !addr.givenName().isEmpty() )
    fullName->firstName = qStringToString( addr.givenName() );

  if ( !addr.additionalName().isEmpty() )
    fullName->middleName = qStringToString( addr.additionalName() );

  if ( !addr.familyName().isEmpty() )
    fullName->lastName = qStringToString( addr.familyName() );

  if ( !addr.suffix().isEmpty() )
    fullName->nameSuffix = qStringToString( addr.suffix() );

  contact->fullName = fullName;

  // Emails
  if ( !addr.emails().isEmpty() ) {
    ngwt__EmailAddressList* emailList = soap_new_ngwt__EmailAddressList( soap(), -1 );
    std::vector<std::string>* list = soap_new_std__vectorTemplateOfstd__string( soap(), -1 );

    QStringList emails = addr.emails();
    emailList->primary = qStringToString( emails.first() );

    QStringList::Iterator it;
    for ( it = emails.begin(); it != emails.end(); ++it )
      list->push_back( std::string( (*it).toUtf8().data() ) );

    emailList->email = *list;
    contact->emailList = emailList;
  } else
    contact->emailList = 0;

  // Phone numbers
  if ( !addr.phoneNumbers().isEmpty() ) {
    ngwt__PhoneList* phoneList = soap_new_ngwt__PhoneList( soap(), -1 );
    phoneList->default_ = 0;
    std::vector<class ngwt__PhoneNumber*> *list = soap_new_std__vectorTemplateOfPointerTongwt__PhoneNumber( soap(), -1 );

    KABC::PhoneNumber::List phones = addr.phoneNumbers();
    KABC::PhoneNumber::List::Iterator it;
    for ( it = phones.begin(); it != phones.end(); ++it ) {
      ngwt__PhoneNumber* number = convertPhoneNumber( *it );
      if ( number ) {
        list->push_back( number );

        // if preferred number assign it
        if ( (*it).type() & KABC::PhoneNumber::Pref )
          phoneList->default_ = qStringToString( (*it).number() );
      }
    }

    phoneList->phone = *list;
    contact->phoneList = phoneList;
  } else
    contact->phoneList = 0;

  // Addresses
  if ( !addr.addresses().isEmpty() ) {
    ngwt__PostalAddressList* addressList = soap_new_ngwt__PostalAddressList( soap(), -1 );
    std::vector<class ngwt__PostalAddress*> *list = soap_new_std__vectorTemplateOfPointerTongwt__PostalAddress( soap(), -1 );

    KABC::Address::List addresses = addr.addresses();
    KABC::Address::List::Iterator it;
    for ( it = addresses.begin(); it != addresses.end(); ++it ) {
      ngwt__PostalAddress* address = convertPostalAddress( *it );
      if ( address )
        list->push_back( address );
    }

    addressList->address = *list;
    contact->addressList = addressList;
  } else
    contact->addressList = 0;

  // IM addresses
  {
    contact->imList = convertImAddresses( addr );
  }

  // Office information
  {
    ngwt__OfficeInfo* info = soap_new_ngwt__OfficeInfo( soap(), -1 );

    // TODO: write back organization?

#if KDE_IS_VERSION(3,5,8)
    if ( addr.department().isEmpty() )
      info->department = qStringToString(addr.department());
#else
    if ( !addr.custom( "KADDRESSBOOK", "X-Department" ).isEmpty() )
      info->department = qStringToString( addr.custom( "KADDRESSBOOK", "X-Department" ) );
#endif
    else
      info->department = 0;

    if ( !addr.title().isEmpty() )
      info->title = qStringToString( addr.title() );
    else
      info->title = 0;

    if ( !addr.url().isEmpty() )
      info->website = qStringToString( addr.url().url() );
    else
      info->website = 0;

    info->organization = 0;

    contact->officeInfo = info;
  }

  // Personal information
  {
    ngwt__PersonalInfo* info = soap_new_ngwt__PersonalInfo( soap(), -1 );

    if ( addr.birthday().isValid() )
      info->birthday = qDateToString( addr.birthday().date() );
    else
      info->birthday = 0;

    // don't write back the web site here, otherwise we'll have it twice
    info->website = 0;

    contact->personalInfo = info;
  }

  return contact;
}

KABC::Addressee ContactConverter::convertFromAddressBookItem( ngwt__AddressBookItem * addrBkItem )
{
  KABC::Addressee addr;
  if ( !addrBkItem )
  {
    kDebug() <<"Null AddressBookItem, bailing out!";
    return addr;
  }
  // gwt:Item
  addr.insertCustom( "GWRESOURCE", "UID", stringToQString( addrBkItem->id ) );
  addr.setFormattedName( stringToQString( addrBkItem->name ) );
  // gwt::AddressBookItem
  addr.insertCustom( "GWRESOURCE", "UUID", stringToQString( addrBkItem->uuid ) );
  addr.setNote( stringToQString( addrBkItem->comment ) );

  return addr;
}

KABC::Addressee ContactConverter::convertFromResource( ngwt__Resource* resource )
{
  KABC::Addressee addr = convertFromAddressBookItem( resource );
  if ( !resource )
  {
    kDebug() <<"Null Resource, bailing out!";
    return addr;
  }
  if ( resource->phone )
    addr.insertPhoneNumber( KABC::PhoneNumber( stringToQString( resource->phone ), KABC::PhoneNumber::Work ) );
  if ( resource->email )
    addr.insertEmail( stringToQString( resource->email ), true );
  if ( resource->owner )
    addr.insertCustom( "KADDRESSBOOK", "X-ManagersName", stringToQString( resource->owner->__item ) );

  addr.insertCategory( i18n( "Resource" ) );
  return addr;
}

KABC::Addressee ContactConverter::convertFromGroup( ngwt__Group* group)
{
  KABC::Addressee addr = convertFromAddressBookItem( group );
  if ( !group )
  {
    kDebug() <<"Null Group, bailing out!";
    return addr;
  }
  addr.insertCategory( i18n( "Group" ) );
  return addr;
}

KABC::Addressee ContactConverter::convertFromContact( ngwt__Contact* contact )
{
  KABC::Addressee addr = convertFromAddressBookItem( contact );

  if ( !contact )
  {
    kDebug() <<"Null Contact, bailing out!";
    return addr;
  }

  // Name parts
  ngwt__FullName* fullName = contact->fullName;

  if ( fullName ) {
    if ( fullName->displayName )
      addr.setFormattedName( stringToQString( fullName->displayName ) );
    else
      addr.setFormattedName( QString() );

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
  }

  // Emails
  if ( contact->emailList ) {
     QStringList emails;

     if ( contact->emailList->primary )
         emails.append( stringToQString( contact->emailList->primary ) );

     if ( !contact->emailList->email.empty() ) {
       std::vector<std::string> list = contact->emailList->email;
       std::vector<std::string>::const_iterator it;
       for ( it = list.begin(); it != list.end(); ++it ) {
         QString searchingFor = stringToQString( *it );
         if ( !emails.contains( searchingFor ) )
           emails.append( searchingFor );
       }
     }

     if ( emails.count() )
       addr.setEmails( emails );
  }

  // Phone numbers
  if ( contact->phoneList && !contact->phoneList->phone.empty() ) {
    QString defaultNumber = stringToQString( contact->phoneList->default_ );

    std::vector<class ngwt__PhoneNumber*> list = contact->phoneList->phone;
    std::vector<class ngwt__PhoneNumber*>::const_iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      KABC::PhoneNumber phone = convertPhoneNumber( *it );
      if ( !phone.number().isEmpty() ) {
        if ( phone.number() == defaultNumber )
          phone.setType( phone.type() | KABC::PhoneNumber::Pref );
        addr.insertPhoneNumber( phone );
      }
    }
  }

  // Addresses
  if ( contact->addressList && !contact->addressList->address.empty() ) {
    std::vector<class ngwt__PostalAddress*> list = contact->addressList->address;
    std::vector<class ngwt__PostalAddress*>::const_iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      KABC::Address address = convertPostalAddress( *it );
      if ( !address.isEmpty() )
        addr.insertAddress( address );
    }
  }

  // Office information
  if ( contact->officeInfo ) {
    ngwt__OfficeInfo* info = contact->officeInfo;

    if ( info->organization )
      addr.setOrganization( stringToQString( info->organization->__item ) );

    if ( info->department )
    {
#if KDE_IS_VERSION(3,5,8)
      addr.setDepartment( stringToQString( info->department ) );
#else
      addr.insertCustom( "KADDRESSBOOK", "X-Department", stringToQString( info->department ) );
#endif
    }
    if ( info->title )
      addr.setTitle( stringToQString( info->title ) );

    if ( info->website )
      addr.setUrl( KUrl( stringToQString( info->website ) ) );
  }

  // Personal information
  if ( contact->personalInfo ) {
    ngwt__PersonalInfo* info = contact->personalInfo;

    if ( info->birthday ) {
      KDateTime date = stringToKDateTime( info->birthday );
      if ( date.isValid() )
        addr.setBirthday( date.dateTime() );
    }

    if ( info->website ) // we might overwrite the office info website here... :(
      addr.setUrl( KUrl( stringToQString( info->website ) ) );
  }

  // IM addresses
  if ( contact->imList ) {
    // put all the im addresses on the same service into the same qstringlist
    QMap<QString, QStringList> addressMap;
    std::vector<ngwt__ImAddress*> list = contact->imList->im;
    std::vector<ngwt__ImAddress*>::const_iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      QStringList addressesForService = addressMap[ stringToQString( (*it)->service ) ];
      addressesForService.append( stringToQString( (*it)->address ) );
      addressMap.insert( stringToQString( (*it)->service ), addressesForService );
    }

    // then construct a custom field from each qstringlist
    QMap<QString, QStringList>::Iterator addrIt;
    for ( addrIt = addressMap.begin(); addrIt != addressMap.end(); ++addrIt )
    {
      QString protocol = addrIt.key();
      QStringList addresses = addrIt.value();
      //kDebug() <<"got IM addresses for '" << protocol <<"' :" << addresses;
      // TODO: map protocol to KDE's set of known protocol names (need to know the set of services in use elsewhere)
      if ( protocol == "nov" )
        protocol = "groupwise";
      addr.insertCustom( QString::fromLatin1("messaging/%1").arg( protocol ),
                          QString::fromLatin1( "All" ),
                          addresses.join( QString( 0xE000 ) ) );
    }
  }

  // addressbook delta sync info
  if ( contact->sync )
  {
    if ( *contact->sync == add )
      addr.insertCustom( "GWRESOURCE", "SYNC", "ADD" );
    else if ( *contact->sync == delete_ )
      addr.insertCustom( "GWRESOURCE", "SYNC", "DEL" );
    else if ( *contact->sync == update )
      addr.insertCustom( "GWRESOURCE", "SYNC", "UPD" );
  }
  //kDebug() <<"Got the following addressee:";
  //addr.dump();
  //kDebug() <<"Customs are:" << addr.customs();

  return addr;
}

KABC::PhoneNumber ContactConverter::convertPhoneNumber( ngwt__PhoneNumber *phone ) const
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

ngwt__PhoneNumber* ContactConverter::convertPhoneNumber( const KABC::PhoneNumber &number ) const
{
  if ( number.number().isEmpty() )
    return 0;

  ngwt__PhoneNumber* phoneNumber = soap_new_ngwt__PhoneNumber( soap(), -1 );
  phoneNumber->__item = number.number().toLatin1().data();

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

KABC::Address ContactConverter::convertPostalAddress( ngwt__PostalAddress *addr ) const
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

ngwt__PostalAddress* ContactConverter::convertPostalAddress( const KABC::Address &address )
{
  if ( address.isEmpty() )
    return 0;

  ngwt__PostalAddress* postalAddress = soap_new_ngwt__PostalAddress( soap(), -1 );

  postalAddress->description = 0;

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

ngwt__ImAddressList* ContactConverter::convertImAddresses( const KABC::Addressee& addr )
{
  //return 0;
  /* TODO: use IM address dedicated functions in KDE 4.0.
  Change semantics so that convertToContact pulls each
  IM address out of the addressee and passes it to this
  function, which converts one at a time. */
  kDebug() ;
  ngwt__ImAddressList* imList = soap_new_ngwt__ImAddressList( soap(), -1 );
  std::vector<class ngwt__ImAddress*> *list = soap_new_std__vectorTemplateOfPointerTongwt__ImAddress( soap(), -1 );

  // for each custom
  // if it contains IM addresses
  // extract each one and add it to imList

  const QStringList customs = addr.customs();
  QStringList::ConstIterator it;
  bool isSet = false;
  for ( it = customs.begin(); it != customs.end(); ++it ) {
    QString app, name, value;
    splitField( *it, app, name, value );

    if ( app.startsWith( QString::fromLatin1( "messaging/" ) ) && name == QString::fromLatin1( "All" ) ) {
      // get the protocol for this field
      QString protocol = app.section( '/', 1, 1 );
      if ( !protocol.isEmpty() ) {
        if ( protocol == "groupwise" )
          protocol = "novell";
        QStringList addresses = value.split( QChar( 0xE000 ), QString::SkipEmptyParts );
        QStringList::iterator end = addresses.end();
        // extract each address for this protocol, and create an ngwt__ImAddress for it, and append it to list.
        for ( QStringList::ConstIterator it = addresses.begin(); it != end; ++it ) {
          ngwt__ImAddress* address = soap_new_ngwt__ImAddress( soap(), -1 );
          address->service = soap_new_std__string( soap(), -1 );
          address->address = soap_new_std__string( soap(), -1 );
          address->type = soap_new_std__string( soap(), -1 );
          address->service->append( protocol.toUtf8().data() );
          address->address->append( (*it).toUtf8().data() );
          address->type->append( "all" );
          kDebug() <<"adding: service:" << protocol <<" address:" << *it <<" type: all";
          list->push_back( address );
        }
      }
    }
  }
  imList->im = *list;
  if ( list->size() > 0 )
    return imList;
  else
  {
    delete imList;
    return 0;
  }
}

void ContactConverter::splitField( const QString &str, QString &app, QString &name, QString &value )
{
  int colon = str.indexOf( ':' );
  if ( colon != -1 ) {
    QString tmp = str.left( colon );
    value = str.mid( colon + 1 );

    int dash = tmp.indexOf( '-' );
    if ( dash != -1 ) {
      app = tmp.left( dash );
      name = tmp.mid( dash + 1 );
    }
  }
}
