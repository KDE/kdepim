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
  : QObject( parent, name ),
    mAddressBook( ab ), mLastField( 0 ), mLastType( Contains ),
    mJumpButtonField( 0 )
{
  mJumpButtonPatterns.append( "" );

  reconfigure();
}

void SearchManager::search( const QString &pattern, KABC::Field *field, Type type )
{
  mLastPattern = pattern;
  mLastField = field;
  mLastType = type;

  KABC::Addressee::List allContacts;
  mContacts.clear();
  mDistributionLists.clear();

#if KDE_VERSION >= 319
  KABC::AddresseeList list( mAddressBook->allAddressees() );
  if ( field )
    list.sortByField( field );

  allContacts = list;
#else
  KABC::AddressBook::Iterator abIt;
  for ( abIt = mAddressBook->begin(); abIt != mAddressBook->end(); ++abIt )
    allContacts.append( *abIt );
#endif

  sortOutDistributionLists( allContacts, mDistributionLists );

  QStringList::ConstIterator it;
  for ( it = mJumpButtonPatterns.begin(); it != mJumpButtonPatterns.end(); ++it )
    doSearch( *it, mJumpButtonField, StartsWith, allContacts );

  allContacts = mContacts;
  mContacts.clear();

  doSearch( mLastPattern, mLastField, mLastType, allContacts );

  // Remove distr. lists from the search results
  KPIM::DistributionList::List dummy;
  sortOutDistributionLists( mContacts, dummy );

  emit contactsUpdated();
}

void SearchManager::setJumpButtonFilter( const QStringList &patterns, KABC::Field *field )
{
  mJumpButtonPatterns = patterns;
  mJumpButtonField = field;

  search( mLastPattern, mLastField, mLastType );
}

void SearchManager::reconfigure()
{
  KConfig config( "kabcrc", false, false );
  config.setGroup( "General" );

  mLimitContactDisplay = config.readBoolEntry( "LimitContactDisplay", true );

  reload();
}

void SearchManager::doSearch( const QString &pattern, KABC::Field *field, Type type,
                              const KABC::Addressee::List &list )
{
  if ( pattern.isEmpty() ) {
    mContacts = list;
// Don't delete the contacts. There are addressbooks with more than 100 entres
// and there doesn't seem to be a way to get the deleted contacts back with
// another search
#if 0
    if ( mLimitContactDisplay && mContacts.count() > 100 ) { // show only 100 contacts
      KABC::Addressee::List::Iterator it = mContacts.at( 100 );
      while ( it != mContacts.end() )
        it = mContacts.remove( it );
    }
#endif

    return;
  }

  if ( field ) {
    KABC::Addressee::List::ConstIterator it;
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
    KABC::Addressee::List::ConstIterator it;
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
}

KABC::Addressee::List SearchManager::contacts() const
{
  return mContacts;
}

void SearchManager::reload()
{
  search( mLastPattern, mLastField, mLastType );
}

void SearchManager::sortOutDistributionLists( KABC::Addressee::List& list,
                                              KPIM::DistributionList::List& distrlists )
{
  KABC::Addressee::List::Iterator it = list.begin();
  while ( it != list.end() ) {
    //kdDebug() << (*it).formattedName() << "   distrlist=" << KPIM::DistributionList::isDistributionList( *it ) << endl;
    if ( KPIM::DistributionList::isDistributionList( *it ) ) {
      distrlists.append( static_cast<KPIM::DistributionList>( *it ) );
      it = list.remove( it );
    } else
      ++it;
  }
}


KPIM::DistributionList::List KAB::SearchManager::distributionLists() const
{
  return mDistributionLists;
}

QStringList KAB::SearchManager::distributionListNames() const
{
  QStringList lst;
  KPIM::DistributionList::List::ConstIterator it;
  for ( it = mDistributionLists.begin(); it != mDistributionLists.end(); ++it ) {
    lst.append( (*it).formattedName() );
  }
  return lst;
}

#include "searchmanager.moc"
