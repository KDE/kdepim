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

#include <kabc/address.h>
#include <kdebug.h>

using namespace Kolab;


KABC::Address* Contact::xmlToAddress( const QString& xml )
{
  Contact contact;
  contact.load( xml );
  KABC::Address* address = new KABC::Address();
  contact.saveTo( address );
  return address;
}

QString Contact::addressToXML( KABC::Address* address )
{
  Contact contact( address );
  return contact.saveXML();
}

Contact::Contact( KABC::Address* addr )
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

QString Contact::freebusyUrl() const
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
  kdError() << "NYI: " << k_funcinfo << endl;
  return mPreferredAddress;
}

bool Contact::loadNameAttribute( QDomElement& element )
{
  kdError() << "NYI: " << k_funcinfo << endl;
  return false;
}

bool Contact::loadPhoneAttribute( QDomElement& element )
{
  kdError() << "NYI: " << k_funcinfo << endl;
  return false;
}

bool Contact::loadEmailAttribute( QDomElement& element )
{
  kdError() << "NYI: " << k_funcinfo << endl;
  return false;
}

bool Contact::loadAddressAttribute( QDomElement& element )
{
  kdError() << "NYI: " << k_funcinfo << endl;
  return false;
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

bool Contact::saveAttributes( QDomElement& ) const
{
  kdError() << "NYI: " << k_funcinfo << endl;
  return false;
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
        qDebug( "Warning: Unhandled tag %s", e.tagName().ascii() );
    } else
      qDebug( "Node is not a comment or an element???" );
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

void Contact::setFields( KABC::Address* address )
{
  kdError() << "NYI: " << k_funcinfo << endl;
}

void Contact::saveTo( KABC::Address* address )
{
  kdError() << "NYI: " << k_funcinfo << endl;
#if 0
  KolabBase::saveTo( journal );

  // TODO: background and foreground
  journal->setSummary( summary() );
#endif
}
