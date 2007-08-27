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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

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
  KABC::AddressBook::ConstIterator abIt( mAddressBook->begin() );
  const KABC::AddressBook::ConstIterator abEndIt( mAddressBook->end() );
  for ( ; abIt != abEndIt; ++abIt )
    allContacts.append( *abIt );
#endif

#ifdef KDEPIM_NEW_DISTRLISTS
  // Extract distribution lists from allContacts
  mDistributionLists.clear();
  KABC::Addressee::List::Iterator rmIt( allContacts.begin() );
  const KABC::Addressee::List::Iterator rmEndIt( allContacts.end() );
  while ( rmIt != rmEndIt ) {
    if ( KPIM::DistributionList::isDistributionList( *rmIt ) ) {
      mDistributionLists.append( static_cast<KPIM::DistributionList>( *rmIt ) );
      rmIt = allContacts.remove( rmIt );
    } else
      ++rmIt;
  }

  typedef KPIM::DistributionList::Entry Entry;
  if ( !mSelectedDistributionList.isNull() ) {
    const KPIM::DistributionList dl = KPIM::DistributionList::findByName( mAddressBook, mSelectedDistributionList );
    if ( !dl.isEmpty() ) {
      allContacts.clear();
      const Entry::List entries = dl.entries( mAddressBook );
      const Entry::List::ConstIterator end = entries.end();
      for ( Entry::List::ConstIterator it = entries.begin(); it != end; ++it ) {
        allContacts.append( (*it).addressee ); 
      }
    }
  }

#endif

  if ( mPattern.isEmpty() ) { // no pattern, return all
    mContacts = allContacts;

    emit contactsUpdated();

    return;
  }

  const KABC::Field::List fieldList = !mFields.isEmpty() ? mFields : KABC::Field::allFields();

  KABC::Addressee::List::ConstIterator it( allContacts.begin() );
  const KABC::Addressee::List::ConstIterator endIt( allContacts.end() );
  for ( ; it != endIt; ++it ) {
#ifdef KDEPIM_NEW_DISTRLISTS
    if ( KPIM::DistributionList::isDistributionList( *it ) )
      continue;
#endif

    bool found = false;
    // search over all fields
    KABC::Field::List::ConstIterator fieldIt( fieldList.begin() );
    const KABC::Field::List::ConstIterator fieldEndIt( fieldList.end() );
    for ( ; fieldIt != fieldEndIt; ++fieldIt ) {

      if ( type == StartsWith && (*fieldIt)->value( *it ).startsWith( pattern, false ) ) {
        mContacts.append( *it );
        found = true;
        break;
      } else if ( type == EndsWith && (*fieldIt)->value( *it ).endsWith( pattern, false ) ) {
        mContacts.append( *it );
        found = true;
        break;
      } else if ( type == Contains && (*fieldIt)->value( *it ).find( pattern, 0, false ) != -1 ) {
        mContacts.append( *it );
        found = true;
        break;
      } else if ( type == Equals && (*fieldIt)->value( *it ).localeAwareCompare( pattern ) == 0 ) {
        mContacts.append( *it );
        found = true;
        break;
      }
    }

    if ( !found ) {
      // search over custom fields
      const QStringList customs = (*it).customs();

      QStringList::ConstIterator customIt( customs.begin() );
      const QStringList::ConstIterator customEndIt( customs.end() );
      for ( ; customIt != customEndIt; ++customIt ) {
        int pos = (*customIt).find( ':' );
        if ( pos != -1 ) {
          const QString value = (*customIt).mid( pos + 1 );
          if ( type == StartsWith && value.startsWith( pattern, false ) ) {
            mContacts.append( *it );
            break;
          } else if ( type == EndsWith && value.endsWith( pattern, false ) ) {
            mContacts.append( *it );
            break;
          } else if ( type == Contains && value.find( pattern, 0, false ) != -1 ) {
            mContacts.append( *it );
            break;
          } else if ( type == Equals && value.localeAwareCompare( pattern ) == 0 ) {
            mContacts.append( *it );
            break;
          }
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

#ifdef KDEPIM_NEW_DISTRLISTS

void KAB::SearchManager::setSelectedDistributionList( const QString &name )
{
    mSelectedDistributionList = name;
    reload();
}

KPIM::DistributionList::List KAB::SearchManager::distributionLists() const
{
  return mDistributionLists;
}

QStringList KAB::SearchManager::distributionListNames() const
{
  QStringList lst;
  KPIM::DistributionList::List::ConstIterator it( mDistributionLists.begin() );
  const KPIM::DistributionList::List::ConstIterator endIt( mDistributionLists.end() );
  for ( ; it != endIt; ++it ) {
    lst.append( (*it).formattedName() );
  }
  return lst;
}
#endif

#include "searchmanager.moc"
