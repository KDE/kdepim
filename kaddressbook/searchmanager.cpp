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

SearchManager *SearchManager::mSelf = 0;

SearchManager *SearchManager::self()
{
  if ( !mSelf )
    mSelf = new SearchManager( KABC::StdAddressBook::self(), 0, "SearchManager" );

  return mSelf;
}

SearchManager::SearchManager( KABC::AddressBook *ab,
                              QObject *parent, const char *name )
  : QObject( parent, name ),
    mAddressBook( ab ), mLastField( 0 ), mLastType( Contains ), 
    mLastSingleSearch( true )
{
}

void SearchManager::search( const QString &pattern, KABC::Field *field, Type type )
{
  mLastPattern = pattern;
  mLastField = field;
  mLastType = type;
  mLastSingleSearch = true;

  mContacts.clear();
  doSearch( mLastPattern, mLastField, mLastType );
  emit contactsUpdated();
}

void SearchManager::searchList( const QStringList &patterns, KABC::Field *field, Type type )
{
  mLastPatterns = patterns;
  mLastField = field;
  mLastType = type;
  mLastSingleSearch = false;

  mContacts.clear();
  QStringList::ConstIterator it;
  for ( it = patterns.begin(); it != patterns.end(); ++it )
    doSearch( *it, mLastField, mLastType );
  emit contactsUpdated();
}

void SearchManager::doSearch( const QString &pattern, KABC::Field *field, Type type )
{
  if ( pattern.isEmpty() ) {
    mContacts = mAddressBook->allAddressees();
    if ( mContacts.count() > 100 ) { // show only 100 contacts
      KABC::Addressee::List::Iterator it = mContacts.at( 100 );
      while ( it != mContacts.end() )
        it = mContacts.remove( it );
    }

    return;
  }

#if KDE_VERSION >= 319
  KABC::AddresseeList list( mAddressBook->allAddressees() );
  if ( field ) {
    list.sortByField( field );
    KABC::AddresseeList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
      if ( type == StartsWith && field->value( *it ).startsWith( pattern, false ) )
        mContacts.append( *it );
      else if ( type == EndsWith && field->value( *it ).endsWith( pattern, false ) )
        mContacts.append( *it );
      else if ( type == Contains && field->value( *it ).find( pattern, 0, false ) != -1 )
        mContacts.append( *it );
      else if ( type == Equals && field->value( *it ).localeAwareCompare( pattern ) == 0 )
        mContacts.append( *it );
    }
  } else {
    KABC::AddresseeList::Iterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
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
#else
  KABC::AddressBook::Iterator it;
  for ( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    if ( field ) {
      if ( type == StartsWith && field->value( *it ).startsWith( pattern, false ) )
        mContacts.append( *it );
      else if ( type == EndsWith && field->value( *it ).endsWith( pattern, false ) )
        mContacts.append( *it );
      else if ( type == Contains && field->value( *it ).find( pattern, 0, false ) != -1 )
        mContacts.append( *it );
      else if ( type == Equals && field->value( *it ).localeAwareCompare( pattern ) == 0 )
        mContacts.append( *it );
    } else {
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
#endif
}

KABC::Addressee::List SearchManager::contacts() const
{
  return mContacts;
}

void SearchManager::reload()
{
  if ( mLastSingleSearch )
    search( mLastPattern, mLastField, mLastType );
  else
    searchList( mLastPatterns, mLastField, mLastType );
}

#include "searchmanager.moc"
