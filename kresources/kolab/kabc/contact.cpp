/*
    This file is part of libkabc and/or kaddressbook.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB
        <info@klaralvdalens-datakonsult.se>

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

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include "contact.h"
#include "resourcekolab.h"

#include <kabc/addressee.h>
#include <kio/netaccess.h>
#include <kdebug.h>
#include <qfile.h>
#include <float.h>

using namespace Kolab;

static const char* s_pictureAttachmentName = "kolab-picture.png";
static const char* s_logoAttachmentName = "kolab-logo.png";
static const char* s_soundAttachmentName = "sound";
static const char* s_unhandledTagAppName = "KOLABUNHANDLED"; // no hyphens in appnames!

// saving (addressee->xml)
Contact::Contact( const KABC::Addressee* addr )
  : mHasGeo( false )
{
  setFields( addr );
}

// loading (xml->addressee)
Contact::Contact( const QString& xml, KABC::ResourceKolab* resource, const QString& subResource, Q_UINT32 sernum )
  : mHasGeo( false )
{
  load( xml );
  if ( !mPictureAttachmentName.isEmpty() )
    mPicture = loadPictureFromKMail( mPictureAttachmentName, resource, subResource, sernum );
  if ( !mLogoAttachmentName.isEmpty() )
    mLogo = loadPictureFromKMail( mLogoAttachmentName, resource, subResource, sernum );
  if ( !mSoundAttachmentName.isEmpty() )
    mSound = loadDataFromKMail( mSound, resource, subResource, sernum );
}

Contact::~Contact()
{
}

void Contact::setGivenName( const QString& name )
{
  mGivenName = name;
}

QString Contact::givenName() const
{
  return mGivenName;
}

void Contact::setMiddleNames( const QString& names )
{
  mMiddleNames = names;
}

QString Contact::middleNames() const
{
  return mMiddleNames;
}

void Contact::setLastName( const QString& name )
{
  mLastName = name;
}

QString Contact::lastName() const
{
  return mLastName;
}

void Contact::setFullName( const QString& name )
{
  mFullName = name;
}

QString Contact::fullName() const
{
  return mFullName;
}

void Contact::setInitials( const QString& initials )
{
  mInitials = initials;
}

QString Contact::initials() const
{
  return mInitials;
}

void Contact::setPrefix( const QString& prefix )
{
  mPrefix = prefix;
}

QString Contact::prefix() const
{
  return mPrefix;
}

void Contact::setSuffix( const QString& suffix )
{
  mSuffix = suffix;
}

QString Contact::suffix() const
{
  return mSuffix;
}

void Contact::setRole( const QString& role )
{
  mRole = role;
}

QString Contact::role() const
{
  return mRole;
}

void Contact::setFreeBusyUrl( const QString& fbUrl )
{
  mFreeBusyUrl = fbUrl;
}

QString Contact::freeBusyUrl() const
{
  return mFreeBusyUrl;
}

void Contact::setOrganization( const QString& organization )
{
  mOrganization = organization;
}

QString Contact::organization() const
{
  return mOrganization;
}

void Contact::setWebPage( const QString& url )
{
  mWebPage = url;
}

QString Contact::webPage() const
{
  return mWebPage;
}

void Contact::setIMAddress( const QString& imAddress )
{
  mIMAddress = imAddress;
}

QString Contact::imAddress() const
{
  return mIMAddress;
}

void Contact::setDepartment( const QString& department )
{
  mDepartment = department;
}

QString Contact::department() const
{
  return mDepartment;
}

void Contact::setOfficeLocation( const QString& location )
{
  mOfficeLocation = location;
}

QString Contact::officeLocation() const
{
  return mOfficeLocation;
}

void Contact::setProfession( const QString& profession )
{
  mProfession = profession;
}

QString Contact::profession() const
{
  return mProfession;
}

// void Contact::setJobTitle( const QString& title )
// {
//   mJobTitle = title;
// }

// QString Contact::jobTitle() const
// {
//   return mJobTitle;
// }

void Contact::setManagerName( const QString& name )
{
  mManagerName = name;
}

QString Contact::managerName() const
{
  return mManagerName;
}

void Contact::setAssistant( const QString& name )
{
  mAssistant = name;
}

QString Contact::assistant() const
{
  return mAssistant;
}

void Contact::setNickName( const QString& name )
{
  mNickName = name;
}

QString Contact::nickName() const
{
  return mNickName;
}

void Contact::setSpouseName( const QString& name )
{
  mSpouseName = name;
}

QString Contact::spouseName() const
{
  return mSpouseName;
}

void Contact::setBirthday( const QDate& date )
{
  mBirthday = date;
}

QDate Contact::birthday() const
{
  return mBirthday;
}

void Contact::setAnniversary( const QDate& date )
{
  mAnniversary = date;
}

QDate Contact::anniversary() const
{
  return mAnniversary;
}

void Contact::setChildren( const QString& children )
{
  mChildren = children;
}

QString Contact::children() const
{
  return mChildren;
}

void Contact::setGender( const QString& gender )
{
  mGender = gender;
}

QString Contact::gender() const
{
  return mGender;
}

void Contact::setLanguage( const QString& language )
{
  mLanguage = language;
}

QString Contact::language() const
{
  return mLanguage;
}

void Contact::addPhoneNumber( const PhoneNumber& number )
{
  mPhoneNumbers.append( number );
}

QValueList<Contact::PhoneNumber>& Contact::phoneNumbers()
{
  return mPhoneNumbers;
}

const QValueList<Contact::PhoneNumber>& Contact::phoneNumbers() const
{
  return mPhoneNumbers;
}

void Contact::addEmail( const Email& email )
{
  mEmails.append( email );
}

QValueList<Contact::Email>& Contact::emails()
{
  return mEmails;
}

const QValueList<Contact::Email>& Contact::emails() const
{
  return mEmails;
}

void Contact::addAddress( const Contact::Address& address )
{
  mAddresses.append( address );
}

QValueList<Contact::Address>& Contact::addresses()
{
  return mAddresses;
}

const QValueList<Contact::Address>& Contact::addresses() const
{
  return mAddresses;
}

void Contact::setPreferredAddress( const QString& address )
{
  mPreferredAddress = address;
}

QString Contact::preferredAddress() const
{
  return mPreferredAddress;
}

bool Contact::loadNameAttribute( QDomElement& element )
{
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "given-name" )
        setGivenName( e.text() );
      else if ( tagName == "middle-names" )
        setMiddleNames( e.text() );
      else if ( tagName == "last-name" )
        setLastName( e.text() );
      else if ( tagName == "full-name" )
        setFullName( e.text() );
      else if ( tagName == "initials" )
        setInitials( e.text() );
      else if ( tagName == "prefix" )
        setPrefix( e.text() );
      else if ( tagName == "suffix" )
        setSuffix( e.text() );
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

void Contact::saveNameAttribute( QDomElement& element ) const
{
  QDomElement e = element.ownerDocument().createElement( "name" );
  element.appendChild( e );

  writeString( e, "given-name", givenName() );
  writeString( e, "middle-names", middleNames() );
  writeString( e, "last-name", lastName() );
  writeString( e, "full-name", fullName() );
  writeString( e, "initials", initials() );
  writeString( e, "prefix", prefix() );
  writeString( e, "suffix", suffix() );
}

bool Contact::loadPhoneAttribute( QDomElement& element )
{
  PhoneNumber number;
  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "type" )
        number.type = e.text();
      else if ( tagName == "number" )
        number.number = e.text();
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  addPhoneNumber( number );
  return true;
}

void Contact::savePhoneAttributes( QDomElement& element ) const
{
  QValueList<PhoneNumber>::ConstIterator it = mPhoneNumbers.begin();
  for ( ; it != mPhoneNumbers.end(); ++it ) {
    QDomElement e = element.ownerDocument().createElement( "phone" );
    element.appendChild( e );
    const PhoneNumber& p = *it;
    writeString( e, "type", p.type );
    writeString( e, "number", p.number );
  }
}

void Contact::saveEmailAttributes( QDomElement& element ) const
{
  QValueList<Email>::ConstIterator it = mEmails.begin();
  for ( ; it != mEmails.end(); ++it )
    saveEmailAttribute( element, *it );
}

void Contact::loadCustomAttributes( QDomElement& element )
{
  Custom custom;
  custom.app = element.attribute( "app" );
  custom.name = element.attribute( "name" );
  custom.value = element.attribute( "value" );
  mCustomList.append( custom );
}

void Contact::saveCustomAttributes( QDomElement& element ) const
{
  QValueList<Custom>::ConstIterator it = mCustomList.begin();
  for ( ; it != mCustomList.end(); ++it ) {
    Q_ASSERT( !(*it).name.isEmpty() );
    if ( (*it).app == s_unhandledTagAppName ) {
      writeString( element, (*it).name, (*it).value );
    } else {
      // Let's use attributes so that other tag-preserving-code doesn't need sub-elements
      QDomElement e = element.ownerDocument().createElement( "x-custom" );
      element.appendChild( e );
      e.setAttribute( "app", (*it).app );
      e.setAttribute( "name", (*it).name );
      e.setAttribute( "value", (*it).value );
    }
  }
}

bool Contact::loadAddressAttribute( QDomElement& element )
{
  Address address;

  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "type" )
        address.type = e.text();
      else if ( tagName == "street" )
        address.street = e.text();
      else if ( tagName == "locality" )
        address.locality = e.text();
      else if ( tagName == "region" )
        address.region = e.text();
      else if ( tagName == "postal-code" )
        address.postalCode = e.text();
      else if ( tagName == "country" )
        address.country = e.text();
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  addAddress( address );
  return true;
}

void Contact::saveAddressAttributes( QDomElement& element ) const
{
  QValueList<Address>::ConstIterator it = mAddresses.begin();
  for ( ; it != mAddresses.end(); ++it ) {
    QDomElement e = element.ownerDocument().createElement( "address" );
    element.appendChild( e );
    const Address& a = *it;
    writeString( e, "type", a.type );
    writeString( e, "street", a.street );
    writeString( e, "locality", a.locality );
    writeString( e, "region", a.region );
    writeString( e, "postal-code", a.postalCode );
    writeString( e, "country", a.country );
  }
}

bool Contact::loadAttribute( QDomElement& element )
{
  QString tagName = element.tagName();

  if ( tagName == "name" )
    return loadNameAttribute( element );
  else if ( tagName == "role" )
    setRole( element.text() );
  else if ( tagName == "free-busy-url" )
    setFreeBusyUrl( element.text() );
  else if ( tagName == "organization" )
    setOrganization( element.text() );
  else if ( tagName == "web-page" )
    setWebPage( element.text() );
  else if ( tagName == "im-address" )
    setIMAddress( element.text() );
  else if ( tagName == "department" )
    setDepartment( element.text() );
  else if ( tagName == "office-location" )
    setOfficeLocation( element.text() );
  else if ( tagName == "profession" )
    setProfession( element.text() );
  else if ( tagName == "job-title" )
    // see saveAttributes: <job-title> is mapped to the Role field
    setRole( element.text() );
  else if ( tagName == "manager-name" )
    setManagerName( element.text() );
  else if ( tagName == "assistant" )
    setAssistant( element.text() );
  else if ( tagName == "nick-name" )
    setNickName( element.text() );
  else if ( tagName == "spouse-name" )
    setSpouseName( element.text() );
  else if ( tagName == "birthday" ) {
    if ( !element.text().isEmpty() )
      setBirthday( stringToDate( element.text() ) );
  }
  else if ( tagName == "anniversary" ) {
    if ( !element.text().isEmpty() )
      setAnniversary( stringToDate( element.text() ) );
  }
  else if ( tagName == "picture" ) {
    mPictureAttachmentName = element.text();
  }
  else if ( tagName == "x-logo" ) {
    mLogoAttachmentName = element.text();
  }
  else if ( tagName == "x-sound" ) {
    mSoundAttachmentName = element.text();
  }
  else if ( tagName == "children" )
    setChildren( element.text() );
  else if ( tagName == "gender" )
    setGender( element.text() );
  else if ( tagName == "language" )
    setLanguage( element.text() );
  else if ( tagName == "phone" )
    return loadPhoneAttribute( element );
  else if ( tagName == "email" ) {
    Email email;
    if ( loadEmailAttribute( element, email ) ) {
      addEmail( email );
      return true;
    } else
      return false;
  } else if ( tagName == "address" )
    return loadAddressAttribute( element );
  else if ( tagName == "preferred-address" )
    setPreferredAddress( element.text() );
  else if ( tagName == "latitude" ) {
    setLatitude( element.text().toFloat() );
    mHasGeo = true;
  }
  else if ( tagName == "longitude" ) {
    setLongitude( element.text().toFloat() );
    mHasGeo = true;
  }
  else if ( tagName == "x-custom" )
    loadCustomAttributes( element );
  else
    return KolabBase::loadAttribute( element );

  // We handled this
  return true;
}

bool Contact::saveAttributes( QDomElement& element ) const
{
  // Save the base class elements
  KolabBase::saveAttributes( element );

  saveNameAttribute( element );
  writeString( element, "free-busy-url", freeBusyUrl() );
  writeString( element, "organization", organization() );
  writeString( element, "web-page", webPage() );
  writeString( element, "im-address", imAddress() );
  writeString( element, "department", department() );
  writeString( element, "office-location", officeLocation() );
  writeString( element, "profession", profession() );
  // <role> is gone; jobTitle() is not shown in the addresseeeditor,
  // so let's bind <job-title> to role()
  //writeString( element, "role", role() );
  //writeString( element, "job-title", jobTitle() );
  writeString( element, "job-title", role() );
  writeString( element, "manager-name", managerName() );
  writeString( element, "assistant", assistant() );
  writeString( element, "nick-name", nickName() );
  writeString( element, "spouse-name", spouseName() );
  writeString( element, "birthday", dateToString( birthday() ) );
  writeString( element, "anniversary", dateToString( anniversary() ) );
  if ( !picture().isNull() )
    writeString( element, "picture", mPictureAttachmentName );
  if ( !logo().isNull() )
    writeString( element, "x-logo", mLogoAttachmentName );
  if ( !sound().isNull() )
    writeString( element, "x-sound", mSoundAttachmentName );
  writeString( element, "children", children() );
  writeString( element, "gender", gender() );
  writeString( element, "language", language() );
  savePhoneAttributes( element );
  saveEmailAttributes( element );
  saveAddressAttributes( element );
  writeString( element, "preferred-address", preferredAddress() );
  if ( mHasGeo ) {
    writeString( element, "latitude", QString::number( latitude(), 'g', DBL_DIG ) );
    writeString( element, "longitude", QString::number( longitude(), 'g', DBL_DIG ) );
  }
  saveCustomAttributes( element );

  return true;
}

bool Contact::loadXML( const QDomDocument& document )
{
  QDomElement top = document.documentElement();

  if ( top.tagName() != "contact" ) {
    qWarning( "XML error: Top tag was %s instead of the expected contact",
              top.tagName().ascii() );
    return false;
  }

  for ( QDomNode n = top.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      if ( !loadAttribute( e ) ) {
        // Unhandled tag - save for later storage
        kdDebug() << "Saving unhandled tag " << e.tagName() << endl;
        Custom c;
        c.app = s_unhandledTagAppName;
        c.name = e.tagName();
        c.value = e.text();
        mCustomList.append( c );
      }
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  return true;
}

QString Contact::saveXML() const
{
  QDomDocument document = domTree();
  QDomElement element = document.createElement( "contact" );
  element.setAttribute( "version", "1.0" );
  saveAttributes( element );
  document.appendChild( element );
  return document.toString();
}

static QString addressTypeToString( int /*KABC::Address::Type*/ type )
{
  if ( type & KABC::Address::Home )
    return "home";
  if ( type & KABC::Address::Work )
    return "business";
  return "other";
}

static int addressTypeFromString( const QString& type )
{
  if ( type == "home" )
    return KABC::Address::Home;
  if ( type == "business" )
    return KABC::Address::Work;
  // well, this shows "other" in the editor, which is what we want...
  return KABC::Address::Dom | KABC::Address::Intl | KABC::Address::Postal | KABC::Address::Parcel;
}

static QStringList phoneTypeToString( int /*KABC::PhoneNumber::Types*/ type )
{
  // KABC has a bitfield, i.e. the same phone number can be used for work and home
  // and fax and cellphone etc. etc.
  // So when saving we need to create as many tags as bits that were set.
  QStringList types;
  if ( type & KABC::PhoneNumber::Fax ) {
    if ( type & KABC::PhoneNumber::Home )
      types << "homefax";
    else // assume work -- if ( type & KABC::PhoneNumber::Work )
      types << "businessfax";
    type = type & ~KABC::PhoneNumber::Home;
    type = type & ~KABC::PhoneNumber::Work;
  }

  if ( type & KABC::PhoneNumber::Home )
    types << "home1";
  if ( type & KABC::PhoneNumber::Msg ) // messaging
    types << "home2"; // #
  if ( type & KABC::PhoneNumber::Work )
    types << "business1";
  if ( type & KABC::PhoneNumber::Pref )
    types << "primary";
  if ( type & KABC::PhoneNumber::Voice )
    types << "callback"; // ##
  if ( type & KABC::PhoneNumber::Cell )
    types << "mobile";
  if ( type & KABC::PhoneNumber::Video )
    types << "radio"; // ##
  if ( type & KABC::PhoneNumber::Bbs )
    types << "ttytdd";
  if ( type & KABC::PhoneNumber::Modem )
    types << "telex"; // #
  if ( type & KABC::PhoneNumber::Car )
    types << "car";
  if ( type & KABC::PhoneNumber::Isdn )
    types << "isdn";
  if ( type & KABC::PhoneNumber::Pcs )
    types << "assistant"; // ## Assistant is e.g. secretary
  if ( type & KABC::PhoneNumber::Pager )
    types << "pager";
  // "company" and "business2" are not generated...
  return types;
}

static int /*KABC::PhoneNumber::Types*/ phoneTypeFromString( const QString& type )
{
  if ( type == "homefax" )
    return KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax;
  if ( type == "businessfax" )
    return KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax;

  if ( type == "home1" )
    return KABC::PhoneNumber::Home;
  if ( type == "home2" )
    return KABC::PhoneNumber::Msg;
  if ( type == "business1" )
    return KABC::PhoneNumber::Work;
  if ( type == "primary" )
    return KABC::PhoneNumber::Pref;
  if ( type == "callback" )
    return KABC::PhoneNumber::Voice;
  if ( type == "mobile" )
    return KABC::PhoneNumber::Cell;
  if ( type == "radio" )
    return KABC::PhoneNumber::Video;
  if ( type == "ttytdd" )
    return KABC::PhoneNumber::Bbs;
  if ( type == "telex" )
    return KABC::PhoneNumber::Modem;
  if ( type == "car" )
    return KABC::PhoneNumber::Car;
  if ( type == "isdn" )
    return KABC::PhoneNumber::Isdn;
  if ( type == "assistant" )
    return KABC::PhoneNumber::Pcs;
  if ( type == "pager" )
    return KABC::PhoneNumber::Pager;
  if ( type == "company" )
    return KABC::PhoneNumber::Work; // # duplicated
  if ( type == "business2" )
    return KABC::PhoneNumber::Work; // # duplicated
  return KABC::PhoneNumber::Home; // whatever
}

static const char* s_knownCustomFields[] = {
  "X-IMAddress",
  "X-Department",
  "X-Office",
  "X-Profession",
  "X-ManagersName",
  "X-AssistantsName",
  "X-SpousesName",
  "X-Anniversary",
  0
};

// The saving is addressee -> Contact -> xml, this is the first part
void Contact::setFields( const KABC::Addressee* addressee )
{
  KolabBase::setFields( addressee );

  setGivenName( addressee->givenName() );
  setMiddleNames( addressee->additionalName() );
  setLastName( addressee->familyName() );
  setFullName( addressee->formattedName() );
  setPrefix( addressee->prefix() );
  setSuffix( addressee->suffix() );
  setOrganization( addressee->organization() );
  setWebPage( addressee->url().url() );
  setIMAddress( addressee->custom( "KADDRESSBOOK", "X-IMAddress" ) );
  setDepartment( addressee->custom( "KADDRESSBOOK", "X-Department" ) );
  setOfficeLocation( addressee->custom( "KADDRESSBOOK", "X-Office" ) );
  setProfession( addressee->custom( "KADDRESSBOOK", "X-Profession" ) );
  setRole( addressee->role() );
  //setJobTitle( addressee->title() );
  setManagerName( addressee->custom( "KADDRESSBOOK", "X-ManagersName" ) );
  setAssistant( addressee->custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  setNickName( addressee->nickName() );
  setSpouseName( addressee->custom( "KADDRESSBOOK", "X-SpousesName" ) );
  setBirthday( addressee->birthday().date() );
  setAnniversary( stringToDate( addressee->custom( "KADDRESSBOOK", "X-Anniversary" ) ) );

  const QStringList emails = addressee->emails();
  // Conversion problem here:
  // KABC::Addressee has only one full name and N addresses, but the XML format
  // has N times (fullname+address). So we just copy the fullname over and ignore it on loading.
  for ( QStringList::ConstIterator it = emails.begin(); it != emails.end(); ++it ) {
    Email email;
    email.displayName = fullName();
    email.smtpAddress = *it;
    addEmail( email );
  }

  // Now the real-world addresses
  QString preferredAddress = "home";
  const KABC::Address::List addresses = addressee->addresses();
  for ( KABC::Address::List::ConstIterator it = addresses.begin() ; it != addresses.end(); ++it ) {
    Address address;
    address.type = addressTypeToString( (*it).type() );
    address.street = (*it).street();
    address.locality = (*it).locality();
    address.region = (*it).region();
    address.postalCode = (*it).postalCode();
    address.country = (*it).country();
    // ## TODO not in the XML format: post-office-box and extended address info.
    // ## KDE-specific tags? Or hiding those fields? Or adding a warning?
    addAddress( address );
    if ( (*it).type() & KABC::Address::Pref ) {
      preferredAddress = address.type; // home, business or other
    }
  }
  setPreferredAddress( preferredAddress );

  const KABC::PhoneNumber::List phones = addressee->phoneNumbers();
  for ( KABC::PhoneNumber::List::ConstIterator it = phones.begin(); it != phones.end(); ++it ) {
    // Create a tag per phone type set in the bitfield
    QStringList types = phoneTypeToString( (*it).type() );
    for( QStringList::Iterator typit = types.begin(); typit != types.end(); ++typit ) {
      PhoneNumber phoneNumber;
      phoneNumber.type = *typit;
      phoneNumber.number = (*it).number();
      addPhoneNumber( phoneNumber );
    }
  }

  setPicture( loadPictureFromAddressee( addressee->photo() ) );
  mPictureAttachmentName = addressee->custom( "KOLAB", "PictureAttachmentName" );
  if ( mPictureAttachmentName.isEmpty() )
    mPictureAttachmentName = s_pictureAttachmentName;

  setLogo( loadPictureFromAddressee( addressee->logo() ) );
  mLogoAttachmentName = addressee->custom( "KOLAB", "LogoAttachmentName" );
  if ( mLogoAttachmentName.isEmpty() )
    mLogoAttachmentName = s_logoAttachmentName;

  setSound( loadSoundFromAddressee( addressee->sound() ) );
  mSoundAttachmentName = addressee->custom( "KOLAB", "SoundAttachmentName" );
  if ( mSoundAttachmentName.isEmpty() )
    mSoundAttachmentName = s_soundAttachmentName;

  if ( addressee->geo().isValid() ) {
    setLatitude( addressee->geo().latitude() );
    setLongitude( addressee->geo().longitude() );
    mHasGeo = true;
  }

  // Other KADDRESSBOOK custom fields than those already handled
  //    (includes e.g. crypto settings, and extra im addresses)
  QStringList knownCustoms;
  for ( const char** p = s_knownCustomFields; *p; ++p )
    knownCustoms << QString::fromLatin1( *p );
  QStringList customs = addressee->customs();
  for( QStringList::Iterator it = customs.begin(); it != customs.end(); ++it ) {
    // KABC::Addressee doesn't offer a real way to iterate over customs, other than splitting strings ourselves
    // The format is "app-name:value".
    int pos = (*it).find( '-' );
    if ( pos == -1 ) continue;
    QString app = (*it).left( pos );
    if ( app == "KOLAB" ) continue;
    QString name = (*it).mid( pos + 1 );
    pos = name.find( ':' );
    if ( pos == -1 ) continue;
    QString value = name.mid( pos + 1 );
    name = name.left( pos );
    if ( !knownCustoms.contains( name ) ) {
      //kdDebug() << k_funcinfo << "app=" << app << " name=" << name << " value=" << value << endl;
      Custom c;
      if ( app != "KADDRESSBOOK" ) // that's the default
        c.app = app;
      c.name = name;
      c.value = value;
      mCustomList.append( c );
    }
  }

  // Those fields, although defined in Addressee, are not used in KDE
  // (e.g. not visible in kaddressbook/addresseeeditorwidget.cpp)
  // So it doesn't matter much if we don't have them in the XML.
  // mailer, timezone, productId, sortString, agent, rfc2426 name()

  // TODO: Things KAddressBook can't handle:
  // initials, children, gender, language
  // Well, so that's part of the "unhandled" tag thing.

  // TODO: Free/Busy URL. This is done rather awkward in KAddressBook -
  // it stores it in a local file through a korganizer file :-(
}

// The loading is: xml -> Contact -> addressee, this is the second part
void Contact::saveTo( KABC::Addressee* addressee )
{
  // TODO: This needs the same set of TODOs as the setFields method
  KolabBase::saveTo( addressee );

  addressee->setGivenName( givenName() );
  addressee->setAdditionalName( middleNames() );
  addressee->setFamilyName( lastName() );
  addressee->setFormattedName( fullName() );
  addressee->setPrefix( prefix() );
  addressee->setSuffix( suffix() );
  addressee->setOrganization( organization() );
  addressee->setUrl( webPage() );
  addressee->insertCustom( "KADDRESSBOOK", "X-IMAddress", imAddress() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Department", department() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Office", officeLocation() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Profession", profession() );
  addressee->setRole( role() );
  //addressee->setTitle( jobTitle() );
  addressee->insertCustom( "KADDRESSBOOK", "X-ManagersName", managerName() );
  addressee->insertCustom( "KADDRESSBOOK", "X-AssistantsName", assistant() );
  addressee->setNickName( nickName() );
  addressee->insertCustom( "KADDRESSBOOK", "X-SpousesName", spouseName() );
  if ( birthday().isValid() )
    addressee->setBirthday( QDateTime( birthday() ) );

  if ( anniversary().isValid() )
    addressee->insertCustom( "KADDRESSBOOK", "X-Anniversary",
                             dateToString( anniversary() ) );
  else
    addressee->removeCustom( "KADDRESSBOOK", "X-Anniversary" );

  // We need to store both the original attachment name and the picture data into the addressee.
  // This is important, otherwise we would save the image under another attachment name w/o deleting the original one!
  if ( !mPicture.isNull() )
    addressee->setPhoto( KABC::Picture( mPicture ) );
  // Note that we must save the filename in all cases, so that removing the picture
  // actually deletes the attachment.
  addressee->insertCustom( "KOLAB", "PictureAttachmentName", mPictureAttachmentName );
  if ( !mLogo.isNull() )
    addressee->setLogo( KABC::Picture( mLogo ) );
  addressee->insertCustom( "KOLAB", "LogoAttachmentName", mLogoAttachmentName );
  if ( !mSound.isNull() )
    addressee->setSound( KABC::Sound( mSound ) );
  addressee->insertCustom( "KOLAB", "SoundAttachmentName", mSoundAttachmentName );

  if ( mHasGeo )
    addressee->setGeo( KABC::Geo( mLatitude, mLongitude ) );

  QStringList emailAddresses;
  for ( QValueList<Email>::ConstIterator it = mEmails.begin(); it != mEmails.end(); ++it ) {
    // we can't do anything with (*it).displayName
    emailAddresses.append( (*it).smtpAddress );
  }
  addressee->setEmails( emailAddresses );

  for ( QValueList<Address>::ConstIterator it = mAddresses.begin(); it != mAddresses.end(); ++it ) {
    KABC::Address address;
    int type = addressTypeFromString( (*it).type );
    if ( (*it).type == mPreferredAddress )
      type |= KABC::Address::Pref;
    address.setType( type );
    address.setStreet( (*it).street );
    address.setLocality( (*it).locality );
    address.setRegion( (*it).region );
    address.setPostalCode( (*it).postalCode );
    address.setCountry( (*it).country );
    addressee->insertAddress( address );
  }

  for ( QValueList<PhoneNumber>::ConstIterator it = mPhoneNumbers.begin(); it != mPhoneNumbers.end(); ++it ) {
    KABC::PhoneNumber number;
    number.setType( phoneTypeFromString( (*it).type ) );
    number.setNumber( (*it).number );
    addressee->insertPhoneNumber( number );
  }

  for( QValueList<Custom>::ConstIterator it = mCustomList.begin(); it != mCustomList.end(); ++it ) {
    QString app = (*it).app.isEmpty() ? QString::fromLatin1( "KADDRESSBOOK" ) : (*it).app;
    addressee->insertCustom( app, (*it).name, (*it).value );
  }
  //kdDebug(5006) << addressee->customs() << endl;
}

QImage Contact::loadPictureFromKMail( const QString& attachmentName, KABC::ResourceKolab* resource, const QString& subResource, Q_UINT32 sernum )
{
  QImage img;
  KURL url;
  if ( resource->kmailGetAttachment( url, subResource, sernum, attachmentName ) && !url.isEmpty() ) {
    const QString path = url.path();
    img.load( path );
    QFile::remove(path);
  }
  return img;
}

QImage Contact::loadPictureFromAddressee( const KABC::Picture& picture )
{
  QImage img;
  if ( !picture.isIntern() && !picture.url().isEmpty() ) {
    QString tmpFile;
    if ( KIO::NetAccess::download( picture.url(), tmpFile, 0 /*no widget known*/ ) ) {
      img.load( tmpFile );
      KIO::NetAccess::removeTempFile( tmpFile );
    }
  } else
    img = picture.data();
  return img;
}

QByteArray Kolab::Contact::loadDataFromKMail( const QString& attachmentName, KABC::ResourceKolab* resource, const QString& subResource, Q_UINT32 sernum )
{
  QByteArray data;
  KURL url;
  if ( resource->kmailGetAttachment( url, subResource, sernum, attachmentName ) && !url.isEmpty() ) {
    QFile f( url.path() );
    if ( f.open( IO_ReadOnly ) ) {
      data = f.readAll();
      f.close();
    }
    f.remove();
  }
  return data;
}

QByteArray Kolab::Contact::loadSoundFromAddressee( const KABC::Sound& sound )
{
  QByteArray data;
  if ( !sound.isIntern() && !sound.url().isEmpty() ) {
    QString tmpFile;
    if ( KIO::NetAccess::download( sound.url(), tmpFile, 0 /*no widget known*/ ) ) {
      QFile f( tmpFile );
      if ( f.open( IO_ReadOnly ) ) {
        data = f.readAll();
        f.close();
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    }
  } else
    data = sound.data();
  return data;
}

QString Kolab::Contact::productID() const
{
  // TODO: When KAB has the version number in a header file, don't hardcode (Bo)
  // Or we could use Addressee::productID? (David)
  return "KAddressBook 3.3, Kolab resource";
}
