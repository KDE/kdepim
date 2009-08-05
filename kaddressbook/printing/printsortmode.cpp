/*
    This file is part of KAddressBook.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "printsortmode.h"

#include <kabc/field.h>


PrintSortMode::PrintSortMode( ContactFields::Field field, bool ascending )
  : mSortField( field ), mAscending( ascending )
{
  const ContactFields::Fields fields = ContactFields::allFields();
  ContactFields::Fields::ConstIterator it;
  for ( it = fields.begin(); it != fields.end(); ++it ) {
    if ( ContactFields::label(*it) == ContactFields::label( ContactFields::NickName) )
      mGivenNameField = *it;
    else if ( ContactFields::label(*it) == ContactFields::label(ContactFields::FamilyName) )
      mFamilyNameField = *it;
    else if ( ContactFields::label(*it) == ContactFields::label( ContactFields::FormattedName) )
      mFormattedNameField = *it;
  }
}

bool PrintSortMode::lesser( const KABC::Addressee &first,
                            const KABC::Addressee &second ) const
{
  if ( !mSortField )
    return false;

  int result = QString::localeAwareCompare( ContactFields::value( mSortField, first ),
                                            ContactFields::value( mSortField, second ) );
  if ( result == 0 ) {
    int givenNameResult = QString::localeAwareCompare( ContactFields::value( mGivenNameField, first ),
                                                       ContactFields::value( mGivenNameField, second ) );
    if ( givenNameResult == 0 ) {
      int familyNameResult = QString::localeAwareCompare( ContactFields::value( mFamilyNameField, first ),
                                                          ContactFields::value( mFamilyNameField, second ) );
      if ( familyNameResult == 0 ) {
        result = QString::localeAwareCompare( ContactFields::value( mFormattedNameField, first ),
                                              ContactFields::value( mFormattedNameField, second ) );
      } else
        result = familyNameResult;
    } else
      result = givenNameResult;
  }

  bool lesser = result < 0;

  if ( !mAscending )
    lesser = !lesser;

  return lesser;
}

