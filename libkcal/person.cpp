/*
    This file is part of libkcal.
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

#include <kdebug.h>
#include <klocale.h>

#include "person.h"

using namespace KCal;

Person::Person( const QString &fullName )
{
  // @TODO: This is completely messed. A space at the end messes up everyting! Fix this properly!
  int emailPos = fullName.find( '<' );
  if ( emailPos < 0 ) {
    setEmail(fullName);
  } else {
    setEmail(fullName.mid( emailPos + 1, fullName.length() - 1 ));
    setName(fullName.left( emailPos - 2 ));
  }
}

Person::Person( const QString &name, const QString &email )
{
  setName(name);
  setEmail(email);
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
    else
      return mName + " <" + mEmail + ">";
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
  if ( email.startsWith( "mailto:", false ) ) {
    mEmail = email.mid(7);
  } else {
    mEmail = email;
  }
}
