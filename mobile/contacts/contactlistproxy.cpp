/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#include <akonadi/entitytreemodel.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <QIcon>
#include <QPixmap>
#include <KIconLoader>

ContactImageProvider::ContactImageProvider()
  : QDeclarativeImageProvider( QDeclarativeImageProvider::Pixmap ), mModel( 0 )
{
}

QPixmap ContactImageProvider::requestPixmap( const QString &id, QSize *size, const QSize &requestedSize )
{
  int width = 50;
  int height = 50;
  if ( requestedSize.isValid() ) {
    width = requestedSize.width();
    height = requestedSize.height();
  }

  if ( size )
    *size = QSize( width, height );

  const Akonadi::Item item( id.toLongLong() );
  const QModelIndexList indexes = Akonadi::EntityTreeModel::modelIndexesForItem( mModel, item );
  if ( indexes.isEmpty() || !indexes.first().isValid() )
    return QPixmap();

  const QModelIndex index = indexes.first();

  const Akonadi::Item contactItem = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( !contactItem.isValid() )
    return QPixmap();

  if ( contactItem.hasPayload<KABC::Addressee>() ) {
    const KABC::Addressee addressee = contactItem.payload<KABC::Addressee>();
    if ( addressee.photo().isEmpty() ) {
      const QIcon icon = KIconLoader::global()->loadIcon( QLatin1String("user-identity"), KIconLoader::Dialog, KIconLoader::SizeHuge );
      return icon.pixmap( width, height );
    }

    return QPixmap::fromImage( addressee.photo().data().scaled( width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
  } else if ( contactItem.hasPayload<KABC::ContactGroup>() ) {
    const QIcon icon = KIconLoader::global()->loadIcon( QLatin1String("x-mail-distribution-list"), KIconLoader::Dialog, KIconLoader::SizeHuge );
    return icon.pixmap( width, height );
  }

  return QPixmap();
}

void ContactImageProvider::setModel( QAbstractItemModel *model )
{
  mModel = model;
}

ContactListProxy::ContactListProxy(QObject* parent) : ListProxy( parent )
{
  setDynamicSortFilter( true );
  sort( 0, Qt::AscendingOrder );
}

QVariant ContactListProxy::data(const QModelIndex& index, int role) const
{
  const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
    const KABC::Addressee addressee = item.payload<KABC::Addressee>();
    switch ( role ) {
      case NameRole:
        return addressee.realName();
      case PictureRole:
        return QString::fromLatin1( "image://contact_images/%1" ).arg( item.id() );
      case TypeRole:
        return QLatin1String( "contact" );
    }
  } else if ( item.isValid() && item.hasPayload<KABC::ContactGroup>() ) {
    const KABC::ContactGroup group = item.payload<KABC::ContactGroup>();
    switch( role ) {
      case NameRole:
        return group.name();
      case PictureRole:
        return QString::fromLatin1( "image://contact_images/%1" ).arg( item.id() );
      case TypeRole:
        return QLatin1String( "group" );
    }
  } else {
    if ( role == TypeRole )
      return QString();
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

static QString nameForItem( const Akonadi::Item &item )
{
  if ( item.hasPayload<KABC::Addressee>() )
    return item.payload<KABC::Addressee>().realName();

  if ( item.hasPayload<KABC::ContactGroup>() )
    return item.payload<KABC::ContactGroup>().name();

  return QString();
}

bool ContactListProxy::lessThan( const QModelIndex& left, const QModelIndex& right ) const
{
  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  const QString leftName = nameForItem( leftItem );
  const QString rightName = nameForItem( rightItem );

  return (QString::localeAwareCompare( leftName, rightName ) < 0);
}

QString ContactListProxy::typeForIndex(int row) const
{
  return index( row, 0 ).data( TypeRole ).toString();
}

#include "contactlistproxy.moc"
