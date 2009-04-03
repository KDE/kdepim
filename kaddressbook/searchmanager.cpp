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

#include "searchmanager.h"

#include <kabc/addresseelist.h>

static QStringList phoneNumberValues( const KABC::Addressee &addressee,
                                      KABC::PhoneNumber::Type type )
{
  QStringList result;

  const KABC::PhoneNumber::List list = addressee.phoneNumbers( type );
  foreach ( const KABC::PhoneNumber &number, list ) {
    result << number.number();
  }

  return result;
}

typedef QString (KABC::Address::*AddressQStringGetter)() const;

static QStringList addressValues( const KABC::Addressee &addressee,
                                  KABC::Address::Type type,
                                  AddressQStringGetter getter )
{
  QStringList result;

  const KABC::Address::List list = addressee.addresses( type );
  foreach ( const KABC::Address &address, list ) {
    result << (address.*getter)();
  }

  return result;
}

static QStringList addressStreetValues( const KABC::Addressee &addressee,
                                        KABC::Address::Type type )
{
  return addressValues( addressee, type, &KABC::Address::street );
}

static QStringList addressLocalityValues( const KABC::Addressee &addressee,
                                          KABC::Address::Type type )
{
  return addressValues( addressee, type, &KABC::Address::locality );
}

static QStringList addressRegionValues( const KABC::Addressee &addressee,
                                        KABC::Address::Type type )
{
  return addressValues( addressee, type, &KABC::Address::region );
}

static QStringList addressCountryValues( const KABC::Addressee &addressee,
                                         KABC::Address::Type type )
{
  return addressValues( addressee, type, &KABC::Address::country );
}

static QStringList addressPostalCodeValues( const KABC::Addressee &addressee,
                                            KABC::Address::Type type )
{
  return addressValues( addressee, type, &KABC::Address::postalCode );
}

using namespace KAB;

SearchManager::SearchManager( KABC::AddressBook *ab,
                              QObject *parent, const char *name )
  : QObject( parent ), mAddressBook( ab )
{
  setObjectName( name );
}

void SearchManager::search( const QString &pattern, const KABC::Field::List &fields, Type type )
{
  mPattern = pattern;
  mFields = fields;
  mType = type;

  KABC::Addressee::List allContacts;
  mContacts.clear();

  KABC::AddresseeList list( mAddressBook->allAddressees() );
  if ( !fields.isEmpty() )
    list.sortByField( fields.first() );

  allContacts = list;

#if 0
  // Extract distribution lists from allContacts
  mDistributionLists.clear();
  KABC::Addressee::List::Iterator rmIt( allContacts.begin() );
  while ( rmIt != allContacts.constEnd() ) {
    if ( KPIM::DistributionList::isDistributionList( *rmIt ) ) {
      mDistributionLists.append( static_cast<KPIM::DistributionList>( *rmIt ) );
      rmIt = allContacts.erase( rmIt );
    } else
      ++rmIt;
  }
#endif
  typedef KABC::DistributionList::Entry Entry;
  if ( !mSelectedDistributionList.isNull() ) {
    const KABC::DistributionList *dl = mAddressBook->findDistributionListByName( mSelectedDistributionList );
    if ( dl ) {
      allContacts.clear();
      const Entry::List entries = dl->entries();
      const Entry::List::ConstIterator end = entries.constEnd();
      for ( Entry::List::ConstIterator it = entries.constBegin(); it != end; ++it ) {
        allContacts.append( (*it).addressee() );
      }
    }
  }

  if ( mPattern.isEmpty() ) { // no pattern, return all
    mContacts = allContacts;

    emit contactsUpdated();

    return;
  }

  const KABC::Field::List fieldList = !mFields.isEmpty() ? mFields : KABC::Field::allFields();

  KABC::Addressee::List::ConstIterator it( allContacts.constBegin() );
  const KABC::Addressee::List::ConstIterator endIt( allContacts.constEnd() );
  for ( ; it != endIt; ++it ) {
    bool found = false;
    // search over all fields
    KABC::Field::List::ConstIterator fieldIt( fieldList.begin() );
    const KABC::Field::List::ConstIterator fieldEndIt( fieldList.end() );
    for ( ; fieldIt != fieldEndIt; ++fieldIt ) {
      QStringList values;
      if ( (*fieldIt)->label() == KABC::Addressee::homeAddressStreetLabel() ) {
        values = addressStreetValues( *it, KABC::Address::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homeAddressLocalityLabel() ) {
        values = addressLocalityValues( *it, KABC::Address::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homeAddressRegionLabel() ) {
        values = addressRegionValues( *it, KABC::Address::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homeAddressCountryLabel() ) {
        values = addressCountryValues( *it, KABC::Address::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homeAddressPostalCodeLabel() ) {
        values = addressPostalCodeValues( *it, KABC::Address::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessAddressStreetLabel() ) {
        values = addressStreetValues( *it, KABC::Address::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessAddressLocalityLabel() ) {
        values = addressLocalityValues( *it, KABC::Address::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessAddressRegionLabel() ) {
        values = addressRegionValues( *it, KABC::Address::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessAddressCountryLabel() ) {
        values = addressCountryValues( *it, KABC::Address::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessAddressPostalCodeLabel() ) {
        values = addressPostalCodeValues( *it, KABC::Address::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homePhoneLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Home );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessPhoneLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Work );
      } else if ( (*fieldIt)->label() == KABC::Addressee::carPhoneLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Car );
      } else if ( (*fieldIt)->label() == KABC::Addressee::isdnLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Isdn );
      } else if ( (*fieldIt)->label() == KABC::Addressee::pagerLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Pager );
      } else if ( (*fieldIt)->label() == KABC::Addressee::mobilePhoneLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Cell );
      } else if ( (*fieldIt)->label() == KABC::Addressee::homeFaxLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
      } else if ( (*fieldIt)->label() == KABC::Addressee::businessFaxLabel() ) {
        values = phoneNumberValues( *it, KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax);
      } else if ( (*fieldIt)->label() == KABC::Addressee::emailLabel() ) {
        values = (*it).emails();
      } else {
        values << (*fieldIt)->value( *it );
      }
      foreach ( const QString &value, values ) {
        if ( type == StartsWith && value.startsWith( pattern, Qt::CaseInsensitive ) ) {
        mContacts.append( *it );
        found = true;
        break;
        } else if ( type == EndsWith && value.endsWith( pattern, Qt::CaseInsensitive ) ) {
        mContacts.append( *it );
        found = true;
        break;
        } else if ( type == Contains && value.contains( pattern, Qt::CaseInsensitive ) ) {
        mContacts.append( *it );
        found = true;
        break;
        } else if ( type == Equals && value.localeAwareCompare( pattern ) == 0 ) {
        mContacts.append( *it );
        found = true;
        break;
      }
    }

      if ( found ) {
        break;
      }
    }

    if ( !found ) {
      // search over custom fields
      const QStringList customs = (*it).customs();

      QStringList::ConstIterator customIt( customs.constBegin() );
      const QStringList::ConstIterator customEndIt( customs.constEnd() );
      for ( ; customIt != customEndIt; ++customIt ) {
        int pos = (*customIt).indexOf( ':' );
        if ( pos != -1 ) {
          const QString value = (*customIt).mid( pos + 1 );
          if ( type == StartsWith && value.startsWith( pattern, Qt::CaseInsensitive ) ) {
            mContacts.append( *it );
            break;
          } else if ( type == EndsWith && value.endsWith( pattern, Qt::CaseInsensitive ) ) {
            mContacts.append( *it );
            break;
          } else if ( type == Contains && value.contains( pattern, Qt::CaseInsensitive ) ) {
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

void KAB::SearchManager::setSelectedDistributionList( const QString &name )
{
  if ( mSelectedDistributionList == name )
    return;
  mSelectedDistributionList = name;
  reload();
}

QList<KABC::DistributionList*> KAB::SearchManager::distributionLists() const
{
  return mAddressBook->allDistributionLists();
}

QStringList KAB::SearchManager::distributionListNames() const
{
  return mAddressBook->allDistributionListNames();
}

#include "searchmanager.moc"
