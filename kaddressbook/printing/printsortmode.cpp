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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/field.h>

#include "printsortmode.h"

#if KDE_IS_VERSION(3,3,91)

PrintSortMode::PrintSortMode( KABC::Field *field, bool ascending )
  : mSortField( field ), mAscending( ascending )
{
  const KABC::Field::List fields = KABC::Field::allFields();
  KABC::Field::List::ConstIterator it;
  for ( it = fields.begin(); it != fields.end(); ++it ) {
    if ( (*it)->label() == KABC::Addressee::givenNameLabel() )
      mGivenNameField = *it;
    else if ( (*it)->label() == KABC::Addressee::familyNameLabel() )
      mFamilyNameField = *it;
    else if ( (*it)->label() == KABC::Addressee::formattedNameLabel() )
      mFormattedNameField = *it;
  }
}

bool PrintSortMode::lesser( const KABC::Addressee &first,
                            const KABC::Addressee &second ) const
{
  if ( !mSortField )
    return false;

  int result = QString::localeAwareCompare( mSortField->value( first ),
                                            mSortField->value( second ) );
  if ( result == 0 ) {
    int givenNameResult = QString::localeAwareCompare( mGivenNameField->value( first ),
                                                       mGivenNameField->value( second ) );
    if ( givenNameResult == 0 ) {
      int familyNameResult = QString::localeAwareCompare( mFamilyNameField->value( first ),
                                                          mFamilyNameField->value( second ) );
      if ( familyNameResult == 0 ) {
        result = QString::localeAwareCompare( mFormattedNameField->value( first ),
                                              mFormattedNameField->value( second ) );
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

#endif
