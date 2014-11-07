/*
    Copyright (c) 2010 Tobias Koenig <tokoe@kde.org>

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

#include "locationmodel.h"

LocationModel::LocationModel( QObject *parent )
  : QAbstractTableModel( parent )
{
}

LocationModel::~LocationModel()
{
}

void LocationModel::setLocations( const KContacts::Address::List &locations )
{
  mLocations = locations;
  reset();
}

KContacts::Address::List LocationModel::locations() const
{
  return mLocations;
}

int LocationModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mLocations.count();
}

int LocationModel::columnCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return 8;
}

QVariant LocationModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mLocations.count() || index.column() >= 8 )
    return QVariant();

  const KContacts::Address address = mLocations.at( index.row() );
  if ( role == Qt::DisplayRole ) {
    switch ( index.column() ) {
      case 0:
        return address.typeLabel();
        break;
      case 1:
        return address.street();
        break;
      case 2:
        return address.postOfficeBox();
        break;
      case 3:
        return address.locality();
        break;
      case 4:
        return address.region();
        break;
      case 5:
        return address.postalCode();
        break;
      case 6:
        return address.country();
        break;
      case 7:
        return address.label();
        break;
    }
  }
  if ( role == Qt::EditRole ) {
    switch ( index.column() ) {
      case 0:
        return QVariant::fromValue( (int)address.type() );
        break;
      case 1:
        return address.street();
        break;
      case 2:
        return address.postOfficeBox();
        break;
      case 3:
        return address.locality();
        break;
      case 4:
        return address.region();
        break;
      case 5:
        return address.postalCode();
        break;
      case 6:
        return address.country();
        break;
      case 7:
        return address.label();
        break;
    }
  }

  return QVariant();
}

bool LocationModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.row() >= mLocations.count() || index.column() >= 8 )
    return false;

  KContacts::Address &address = mLocations[ index.row() ];
  if ( role == Qt::EditRole ) {
    switch ( index.column() ) {
      case 0:
        address.setType( KContacts::Address::Type( value.toInt() ) );
        emit dataChanged( index, index );
        return true;
        break;
      case 1:
        address.setStreet( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 2:
        address.setPostOfficeBox( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 3:
        address.setLocality( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 4:
        address.setRegion( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 5:
        address.setPostalCode( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 6:
        address.setCountry( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
      case 7:
        address.setLabel( value.toString() );
        emit dataChanged( index, index );
        return true;
        break;
    }
  }

  return false;
}

Qt::ItemFlags LocationModel::flags( const QModelIndex &index ) const
{
  return (QAbstractTableModel::flags( index ) | Qt::ItemIsEditable);
}

bool LocationModel::insertRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row > mLocations.count() || parent.isValid() )
    return false;

  beginInsertRows( parent, row, row + count - 1 );

  for ( int pos = row; pos < row + count; ++pos )
    mLocations.insert( row, KContacts::Address() );

  endInsertRows();

  return true;
}

bool LocationModel::removeRows( int row, int count, const QModelIndex &parent )
{
  if ( row < 0 || row >= mLocations.count() || parent.isValid() )
    return false;

  beginRemoveRows( parent, row, row + count - 1 );

  for ( int pos = row; pos < row + count; ++pos )
    mLocations.removeAt( row );

  endRemoveRows();

  return true;
}
