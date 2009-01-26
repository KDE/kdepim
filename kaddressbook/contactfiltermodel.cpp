/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "contactfiltermodel.h"

#include <akonadi/itemmodel.h>
#include <kabc/addressee.h>

static bool contactMatchesFilter( const KABC::Addressee &contact, const QString &filterString );

ContactFilterModel::ContactFilterModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}

void ContactFilterModel::setFilterString( const QString &filter )
{
  mFilter = filter;
  invalidateFilter();
}

bool ContactFilterModel::filterAcceptsRow( int row, const QModelIndex &parent ) const
{
  if ( mFilter.isEmpty() )
    return true;

  const QAbstractItemModel *model = sourceModel();
  const Akonadi::ItemModel *itemModel = static_cast<const Akonadi::ItemModel*>( model );

  const QModelIndex index = itemModel->index( row, 0, parent );
  const Akonadi::Item item = itemModel->itemForIndex( index );

  if ( item.mimeType() == QLatin1String( "text/directory" ) ) {
    const KABC::Addressee contact = item.payload<KABC::Addressee>();
    return contactMatchesFilter( contact, mFilter );
  }

  return true;
}

static bool addressMatchesFilter( const KABC::Address &address, const QString &filterString )
{
  const QString lowerFilterString( filterString.toLower() );

  if ( address.street().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.locality().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.region().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.postalCode().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.country().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.label().toLower().contains( lowerFilterString ) )
    return true;

  if ( address.postOfficeBox().toLower().contains( lowerFilterString ) )
    return true;

  return false;
}

static bool contactMatchesFilter( const KABC::Addressee &contact, const QString &filterString )
{
  const QString lowerFilterString( filterString.toLower() );

  if ( contact.assembledName().toLower().contains( lowerFilterString ) )
    return true;

  if ( contact.formattedName().contains( lowerFilterString ) )
    return true;

  if ( contact.nickName().contains( lowerFilterString ) )
    return true;

  if ( contact.birthday().toString().toLower().contains( lowerFilterString ) )
    return true;

  const KABC::Address::List addresses = contact.addresses();
  for ( int i = 0; i < addresses.count(); ++i ) {
    if ( addressMatchesFilter( addresses.at( i ), filterString ) )
      return true;
  }

  const KABC::PhoneNumber::List phoneNumbers = contact.phoneNumbers();
  for ( int i = 0; i < phoneNumbers.count(); ++i ) {
    if ( phoneNumbers.at( i ).number().toLower().contains( lowerFilterString ) )
      return true;
  }

  const QStringList emails = contact.emails();
  for ( int i = 0; i < emails.count(); ++i ) {
    if ( emails.at( i ).toLower().contains( lowerFilterString ) )
      return true;
  }

  if ( contact.mailer().contains( lowerFilterString ) )
    return true;

  if ( contact.title().contains( lowerFilterString ) )
    return true;

  if ( contact.role().contains( lowerFilterString ) )
    return true;

  if ( contact.organization().contains( lowerFilterString ) )
    return true;

  if ( contact.department().contains( lowerFilterString ) )
    return true;

  if ( contact.note().contains( lowerFilterString ) )
    return true;

  if ( contact.url().url().contains( lowerFilterString ) )
    return true;

  const QStringList customs = contact.customs();
  for ( int i = 0; i < customs.count(); ++i ) {
    if ( customs.at( i ).toLower().contains( lowerFilterString ) )
      return true;
  }

  return false;
}

#include "contactfiltermodel.moc"
