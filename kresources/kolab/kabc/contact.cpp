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

#include <kabc/addressee.h>
#include <kdebug.h>

using namespace Kolab;


KABC::Addressee* Contact::xmlToAddressee( const QString& xml )
{
  Contact contact;
  contact.load( xml );
  KABC::Addressee* addressee = new KABC::Addressee();
  contact.saveTo( addressee );
  return addressee;
}

QString Contact::addresseeToXML( KABC::Addressee* addressee )
{
  Contact contact( addressee );
  return contact.saveXML();
}

Contact::Contact( KABC::Addressee* addr )
{
  setFields( addr );
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

void Contact::setJobTitle( const QString& title )
{
  mJobTitle = title;
}

QString Contact::jobTitle() const
{
  return mJobTitle;
}

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

#if 0
// TODO. Probably a QPixmap
void Contact::setPicture( const QMap& picture )
{
  mPicture = picture;
}

QPixmap Contact::picture() const
{
  return mPicture;
}
#endif

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
    QDomElement e = element.ownerDocument().createElement( "phone-number" );
    const PhoneNumber& p = *it;
    writeString( e, "type", p.type );
    writeString( e, "number", p.number );
  }
}

bool Contact::loadEmailAttribute( QDomElement& element )
{
  Email email;

  for ( QDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() ) {
    if ( n.isComment() )
      continue;
    if ( n.isElement() ) {
      QDomElement e = n.toElement();
      QString tagName = e.tagName();

      if ( tagName == "display-name" )
        email.displayName = e.text();
      else if ( tagName == "smtp-address" )
        email.smtpAddress = e.text();
      else
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
    } else
      kdDebug() << "Node is not a comment or an element???" << endl;
  }

  addEmail( email );
  return true;
}

void Contact::saveEmailAttributes( QDomElement& element ) const
{
  QValueList<Email>::ConstIterator it = mEmails.begin();
  for ( ; it != mEmails.end(); ++it ) {
    QDomElement e = element.ownerDocument().createElement( "email" );
    const Email& email = *it;
    writeString( e, "display-name", email.displayName );
    writeString( e, "smtp-address", email.smtpAddress );
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
      else if ( tagName == "city" )
        address.city = e.text();
      else if ( tagName == "state" )
        address.state = e.text();
      else if ( tagName == "zip" )
        address.zip = e.text();
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
    const Address& a = *it;
    writeString( e, "type", a.type );
    writeString( e, "street", a.street );
    writeString( e, "city", a.city );
    writeString( e, "state", a.state );
    writeString( e, "zip", a.zip );
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
    setJobTitle( element.text() );
  else if ( tagName == "manager-name" )
    setManagerName( element.text() );
  else if ( tagName == "assistant" )
    setAssistant( element.text() );
  else if ( tagName == "nick-name" )
    setNickName( element.text() );
  else if ( tagName == "spouse-name" )
    setSpouseName( element.text() );
  else if ( tagName == "birthday" )
    setBirthday( stringToDate( element.text() ) );
  else if ( tagName == "anniversary" )
    setAnniversary( stringToDate( element.text() ) );
  else if ( tagName == "picture" )
    // TODO
    ; //setPicture( element.text() );
  else if ( tagName == "children" )
    setChildren( element.text() );
  else if ( tagName == "gender" )
    setGender( element.text() );
  else if ( tagName == "language" )
    setLanguage( element.text() );
  else if ( tagName == "phone" )
    return loadPhoneAttribute( element );
  else if ( tagName == "email" )
    return loadEmailAttribute( element );
  else if ( tagName == "address" )
    return loadAddressAttribute( element );
  else if ( tagName == "preferred-address" )
    setPreferredAddress( element.text() );
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
  writeString( element, "role", role() );
  writeString( element, "free-busy-url", freeBusyUrl() );
  writeString( element, "organization", organization() );
  writeString( element, "web-page", webPage() );
  writeString( element, "im-address", imAddress() );
  writeString( element, "department", department() );
  writeString( element, "office-location", officeLocation() );
  writeString( element, "profession", profession() );
  writeString( element, "job-title", jobTitle() );
  writeString( element, "manager-name", managerName() );
  writeString( element, "assistant", assistant() );
  writeString( element, "nick-name", nickName() );
  writeString( element, "spouse-name", spouseName() );
  writeString( element, "birthday", dateToString( birthday() ) );
  writeString( element, "anniversary", dateToString( anniversary() ) );
#if 0
  writeString( element, "picture", picture() );
#endif
  writeString( element, "children", children() );
  writeString( element, "gender", gender() );
  writeString( element, "language", language() );
  savePhoneAttributes( element );
  saveEmailAttributes( element );
  saveAddressAttributes( element );
  writeString( element, "preferred-address", preferredAddress() );

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
      if ( !loadAttribute( e ) )
        // TODO: Unhandled tag - save for later storage
        kdDebug() << "Warning: Unhandled tag " << e.tagName() << endl;
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

void Contact::setFields( KABC::Addressee* addressee )
{
  KolabBase::setFields( addressee );

  setGivenName( addressee->givenName() );
  setMiddleNames( addressee->additionalName() );
  setLastName( addressee->familyName() );
  setFullName( addressee->formattedName() );
  setPrefix( addressee->prefix() );
  setSuffix( addressee->suffix() );
  setRole( addressee->role() );
  setOrganization( addressee->organization() );
  setWebPage( addressee->url().url() );
  setIMAddress( addressee->custom( "KADDRESSBOOK", "X-IMAddress" ) );
  setDepartment( addressee->custom( "KADDRESSBOOK", "X-Department" ) );
  setOfficeLocation( addressee->custom( "KADDRESSBOOK", "X-Office" ) );
  setProfession( addressee->custom( "KADDRESSBOOK", "X-Profession" ) );
  setJobTitle( addressee->title() );
  setManagerName( addressee->custom( "KADDRESSBOOK", "X-ManagersName" ) );
  setAssistant( addressee->custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  setNickName( addressee->nickName() );
  setSpouseName( addressee->custom( "KADDRESSBOOK", "X-SpousesName" ) );
  setBirthday( addressee->birthday().date() );
  setAnniversary( stringToDate( addressee->custom( "KADDRESSBOOK", "X-Anniversary" ) ) );
#if 0
  setPicture( addressee->photo() );
#endif

  // TODO: Unhandled Addressee fields:
  // mailer, timezone, geo, productId, sortString, logo, sound
  // agent, preferred email, emails, phoneNumbers, addresses,
  // extra custom fields, name(), preferred address, preferred phone number,
  // extra im addresses, crypto settings

  // TODO: Things KAddressBook can't handle:
  // initials, children, gender, language

  // TODO: Free/Busy URL. This is done rather awkward in KAddressBook -
  // it stores it in a local file through a korganizer file :-(
}

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
  addressee->setRole( role() );
  addressee->setOrganization( organization() );
  addressee->setUrl( webPage() );
  addressee->insertCustom( "KADDRESSBOOK", "X-IMAddress", imAddress() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Department", department() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Office", officeLocation() );
  addressee->insertCustom( "KADDRESSBOOK", "X-Profession", profession() );
  addressee->setTitle( jobTitle() );
  addressee->insertCustom( "KADDRESSBOOK", "X-ManagersName", managerName() );
  addressee->insertCustom( "KADDRESSBOOK", "X-AssistantsName", assistant() );
  addressee->setNickName( nickName() );
  addressee->insertCustom( "KADDRESSBOOK", "X-SpousesName", spouseName() );
  addressee->setBirthday( QDateTime( birthday() ) );

  if ( anniversary().isValid() )
    addressee->insertCustom( "KADDRESSBOOK", "X-Anniversary",
                             dateToString( anniversary() ) );
  else
    addressee->removeCustom( "KADDRESSBOOK", "X-Anniversary" );

#if 0
  setPicture( addressee->photo() );
#endif
}
