/*
    This file is part of KAddressBook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <kabc/addresseelist.h>
#include <kdeversion.h>

#include "searchmanager.h"

using namespace KAB;

SearchManager::SearchManager( KABC::AddressBook *ab,
                              QObject *parent, const char *name )
  : QObject( parent, name ), mAddressBook( ab )
{
}

void SearchManager::search( const QString &pattern, const KABC::Field::List &fields, Type type )
{
  mPattern = pattern;
  mFields = fields;
  mType = type;

  KABC::Addressee::List allContacts;
  mContacts.clear();

#if KDE_VERSION >= 319
  KABC::AddresseeList list( mAddressBook->allAddressees() );
  if ( !fields.isEmpty() )
    list.sortByField( fields.first() );

  allContacts = list;
#else
  KABC::AddressBook::ConstIterator abIt;
  for ( abIt = mAddressBook->begin(); abIt != mAddressBook->end(); ++abIt )
    allContacts.append( *abIt );
#endif

  if ( mPattern.isEmpty() ) { // no pattern, return all
    mContacts = allContacts;

    emit contactsUpdated();

    return;
  }

  if ( !mFields.isEmpty() ) {
    KABC::Field::List::ConstIterator fieldIt;
    KABC::Addressee::List::ConstIterator it;
    for ( it = allContacts.begin(); it != allContacts.end(); ++it ) {
      for ( fieldIt = mFields.begin(); fieldIt != mFields.end(); ++fieldIt ) {
        if ( type == StartsWith && (*fieldIt)->value( *it ).startsWith( pattern, false ) ) {
          mContacts.append( *it );
          break;
        } else if ( type == EndsWith && (*fieldIt)->value( *it ).endsWith( pattern, false ) ) {
          mContacts.append( *it );
          break;
        } else if ( type == Contains && (*fieldIt)->value( *it ).find( pattern, 0, false ) != -1 ) {
          mContacts.append( *it );
          break;
        } else if ( type == Equals && (*fieldIt)->value( *it ).localeAwareCompare( pattern ) == 0 ) {
          mContacts.append( *it );
          break;
        }
      }
    }
  } else {
    KABC::Addressee::List::ConstIterator it;
    for ( it = allContacts.begin(); it != allContacts.end(); ++it ) {
      KABC::Field::List fieldList = KABC::Field::allFields();
      KABC::Field::List::ConstIterator fieldIt;
      for ( fieldIt = fieldList.begin(); fieldIt != fieldList.end(); ++fieldIt ) {
        if ( type == StartsWith && (*fieldIt)->value( *it ).startsWith( pattern, false ) ) {
          mContacts.append( *it );
          break;
        } else if ( type == EndsWith && (*fieldIt)->value( *it ).endsWith( pattern, false ) ) {
          mContacts.append( *it );
          break;
        } else if ( type == Contains && (*fieldIt)->value( *it ).find( pattern, 0, false ) != -1 ) {
          mContacts.append( *it );
          break;
        } else if ( type == Equals && (*fieldIt)->value( *it ).localeAwareCompare( pattern ) == 0 ) {
          mContacts.append( *it );
          break;
        }
      }
    }
  }

  emit contactsUpdated();
}

KABC::Addressee::List SearchManager::contacts() const
{
  return mContacts;
}

void SearchManager::reload()
{
  search( mPattern, mFields, mType );
}

#include "searchmanager.moc"
