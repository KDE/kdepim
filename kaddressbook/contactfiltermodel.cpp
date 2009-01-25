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
    return contact.assembledName().contains( mFilter );
  }

  return true;
}

#include "contactfiltermodel.moc"
