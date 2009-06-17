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

static QStringList phoneNumberHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Home );
}

static QStringList phoneNumberWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Work );
}

static QStringList phoneNumberCarValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Car );
}

static QStringList phoneNumberIsdnValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Isdn );
}

static QStringList phoneNumberCellValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Cell );
}

static QStringList phoneNumberPagerValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Pager );
}

static QStringList faxNumberHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax );
}

static QStringList faxNumberWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return phoneNumberValues( addressee, KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax );
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

static QStringList addressStreetHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressStreetValues( addressee, KABC::Address::Home );
}

static QStringList addressStreetWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressStreetValues( addressee, KABC::Address::Work );
}

static QStringList addressLocalityHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressLocalityValues( addressee, KABC::Address::Home );
}

static QStringList addressLocalityWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressLocalityValues( addressee, KABC::Address::Work );
}

static QStringList addressRegionHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressRegionValues( addressee, KABC::Address::Home );
}

static QStringList addressRegionWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressRegionValues( addressee, KABC::Address::Work );
}

static QStringList addressCountryHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressCountryValues( addressee, KABC::Address::Home );
}

static QStringList addressCountryWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressCountryValues( addressee, KABC::Address::Work );
}

static QStringList addressPostalCodeHomeValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressPostalCodeValues( addressee, KABC::Address::Home );
}

static QStringList addressPostalCodeWorkValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressPostalCodeValues( addressee, KABC::Address::Work );
}

static QStringList emailValues( KABC::Field&, const KABC::Addressee &addressee )
{
  return addressee.emails();
}

static QStringList singleFieldValues( KABC::Field& field, const KABC::Addressee &addressee )
{
  return QStringList() << field.value( addressee );
}

using namespace KAB;

SearchManager::SearchManager( KABC::AddressBook *ab,
                              QObject *parent, const char *name )
  : QObject( parent ), mAddressBook( ab )
{
  setObjectName( name );

  mValueListGetters[ KABC::Addressee::homePhoneLabel() ] = phoneNumberHomeValues;
  mValueListGetters[ KABC::Addressee::businessPhoneLabel() ] = phoneNumberWorkValues;
  mValueListGetters[ KABC::Addressee::carPhoneLabel() ] = phoneNumberCarValues;
  mValueListGetters[ KABC::Addressee::mobilePhoneLabel() ] = phoneNumberCellValues;
  mValueListGetters[ KABC::Addressee::isdnLabel() ] = phoneNumberIsdnValues;
  mValueListGetters[ KABC::Addressee::pagerLabel() ] = phoneNumberPagerValues;
  mValueListGetters[ KABC::Addressee::homeFaxLabel() ] = faxNumberHomeValues;
  mValueListGetters[ KABC::Addressee::businessFaxLabel() ] = faxNumberWorkValues;

  mValueListGetters[ KABC::Addressee::homeAddressStreetLabel() ] = addressStreetHomeValues;
  mValueListGetters[ KABC::Addressee::businessAddressStreetLabel() ] = addressStreetWorkValues;
  mValueListGetters[ KABC::Addressee::homeAddressLocalityLabel() ] = addressLocalityHomeValues;
  mValueListGetters[ KABC::Addressee::businessAddressLocalityLabel() ] = addressLocalityWorkValues;
  mValueListGetters[ KABC::Addressee::homeAddressRegionLabel() ] = addressRegionHomeValues;
  mValueListGetters[ KABC::Addressee::businessAddressRegionLabel() ] = addressRegionWorkValues;
  mValueListGetters[ KABC::Addressee::homeAddressCountryLabel() ] = addressCountryHomeValues;
  mValueListGetters[ KABC::Addressee::businessAddressCountryLabel() ] = addressCountryWorkValues;
  mValueListGetters[ KABC::Addressee::homeAddressPostalCodeLabel() ] = addressPostalCodeHomeValues;
  mValueListGetters[ KABC::Addressee::businessAddressPostalCodeLabel() ] = addressPostalCodeWorkValues;

  mValueListGetters[ KABC::Addressee::emailLabel() ] = emailValues;
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

  QList<valueListGetter> fieldGetters;
  KABC::Field::List::ConstIterator fieldIt( fieldList.constBegin() );
  const KABC::Field::List::ConstIterator fieldEndIt( fieldList.constEnd() );
  for ( ; fieldIt != fieldEndIt; ++fieldIt ) {
    const QString label = (*fieldIt)->label();
    ValueListGetters::const_iterator getterIt = mValueListGetters.constFind( label );
    if ( getterIt != mValueListGetters.constEnd() ) {
      fieldGetters << getterIt.value();
    } else {
      fieldGetters << singleFieldValues;
    }
  }

  KABC::Addressee::List::ConstIterator it( allContacts.constBegin() );
  const KABC::Addressee::List::ConstIterator endIt( allContacts.constEnd() );
  for ( ; it != endIt; ++it ) {
    bool found = false;
    // search over all fields
    fieldIt = fieldList.constBegin();
    QList<valueListGetter>::ConstIterator getterIt( fieldGetters.constBegin() );
    const QList<valueListGetter>::ConstIterator getterEndIt( fieldGetters.constEnd() );
    for ( ; getterIt != getterEndIt; ++getterIt, ++fieldIt ) {
      const QStringList values = (*(*getterIt))( *(*fieldIt), *it );
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
