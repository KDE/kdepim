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

QString Person::fullName() const
{
  if( mName.isEmpty() ) {
    if( mEmail.isEmpty() )
      return i18n( "Unknown" );
    else
      return mEmail;
  } else {
    if( mEmail.isEmpty() )
      return mName;
    else
      return mName + " <" + mEmail + ">";
  }
}

void Person::setName(const QString &name)
{
  mName = name;
}

void Person::setEmail(const QString &email)
{
  if (email.left(7).lower() == "mailto:") {
    mEmail = email.mid(7);
  } else {
    mEmail = email;
  }
}
