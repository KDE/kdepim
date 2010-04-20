/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "contactlistproxy.h"
#include <kabc/addressee.h>
#include <QPixmap>
#include <KIconLoader>

ContactListProxy::ContactListProxy(QObject* parent) : ListProxy( parent )
{
}

QVariant ContactListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
    const KABC::Addressee addressee = item.payload<KABC::Addressee>();
    switch ( role ) {
      case NameRole:
        return addressee.name();
      case PictureRole:
        if ( addressee.photo().isEmpty() )
          return KIconLoader::global()->loadIcon( "user-identity", KIconLoader::Dialog, KIconLoader::SizeHuge );
        return QPixmap::fromImage( addressee.photo().data() );
    }
  }

  return QSortFilterProxyModel::data( index, role );
}

void ContactListProxy::setSourceModel(QAbstractItemModel* sourceModel)
{
  ListProxy::setSourceModel(sourceModel);
  QHash<int, QByteArray> names = roleNames();
  names.insert( Akonadi::EntityTreeModel::ItemIdRole, "itemId" );
  names.insert( NameRole, "name" );
  names.insert( PictureRole, "picture" );
  setRoleNames( names );
}


#include "contactlistproxy.h"
