/*
    This file is part of libkcal.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "person.h"

#include <kdebug.h>
#include <klocale.h>

#include <emailfunctions/email.h>

#include <QRegExp>

using namespace KCal;

Person::Person( const QString &fullName )
{
  QString name, email;
  EmailAddressTools::extractEmailAddressAndName( fullName, email, name );
  setName( name );
  setEmail( email );
}

Person::Person( const QString &name, const QString &email )
{
  setName( name );
  setEmail( email );
}


bool KCal::operator==( const Person& p1, const Person& p2 )
{
    return ( p1.name() == p2.name() &&
             p1.email() == p2.email() );
}


QString Person::fullName() const
{
  if( mName.isEmpty() ) {
    return mEmail;
  } else {
    if( mEmail.isEmpty() )
      return mName;
    else {
      // Taken from KABC::Addressee::fullEmail
      QString name = mName;
      QRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
      bool weNeedToQuote = name.indexOf( needQuotes ) != -1;
      if ( weNeedToQuote ) {
          if ( name[0] != '"' )
              name.prepend( '"' );
          if ( name[ name.length()-1 ] != '"' )
              name.append( '"' );
      }
      return name + " <" + mEmail + '>';
    }
  }
}

bool Person::isEmpty() const
{
  return mEmail.isEmpty() && mName.isEmpty();
}

void Person::setName(const QString &name)
{
  mName = name;
}

void Person::setEmail(const QString &email)
{
  if ( email.startsWith( "mailto:", Qt::CaseInsensitive ) ) {
    mEmail = email.mid(7);
  } else {
    mEmail = email;
  }
}
