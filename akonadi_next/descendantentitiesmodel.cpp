/*
    Copyright (c) 2008 Stephen Kelly <steveire@gmail.com>

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

#include "descendantentitiesmodel.h"
#include "descendantentitiesmodel_p.h"

#include <QtCore/QHash>
#include <QtCore/QMimeData>

#include <KIcon>
#include <KUrl>
#include <KLocale>

#include <akonadi/attributefactory.h>
#include "collectionutils_p.h"
#include "collectionchildorderattribute.h"
#include <akonadi/entitydisplayattribute.h>
#include "entityupdateadapter.h"
#include "clientsideentitystorage.h"
// #include <akonadi/monitor.h>

#include "kdebug.h"

using namespace Akonadi;

DescendantEntitiesModel::DescendantEntitiesModel( EntityUpdateAdapter *entityUpdateAdapter,
                                  ClientSideEntityStorage *clientSideEntityStorage,
                                  QObject *parent
                                )
    : QAbstractItemModel( parent ),
    d_ptr( new DescendantEntitiesModelPrivate( this ) )
{
  Q_D( DescendantEntitiesModel );

  d->m_clientSideEntityStorage = clientSideEntityStorage;
  d->entityUpdateAdapter = entityUpdateAdapter;

  connect( d->m_clientSideEntityStorage, SIGNAL( beginInsertEntities( Collection::Id, int, int ) ),
           SLOT( rowsAboutToBeInserted( Collection::Id, int, int ) ) );
  connect( d->m_clientSideEntityStorage, SIGNAL( endInsertEntities() ),
           SLOT( rowsInserted() ) );
  connect( d->m_clientSideEntityStorage, SIGNAL( beginRemoveEntities( Collection::Id, int, int ) ),
           SLOT( rowsAboutToBeRemoved( Collection::Id, int, int ) ) );
  connect( d->m_clientSideEntityStorage, SIGNAL( endRemoveEntities() ),
           SLOT( rowsRemoved() ) );
  connect( d->m_clientSideEntityStorage, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
           SLOT( collectionChanged( const Akonadi::Collection& ) ) );
  connect( d->m_clientSideEntityStorage, SIGNAL( itemChanged( const Akonadi::Item&, const QSet< QByteArray >& ) ),
           SLOT( itemChanged( const Akonadi::Item&, const QSet< QByteArray >& ) ) );

}

DescendantEntitiesModel::~DescendantEntitiesModel()
{
  Q_D( DescendantEntitiesModel );
}

int DescendantEntitiesModel::columnCount( const QModelIndex & parent ) const
{
// TODO: Subscriptions? Statistics?
  if ( parent.isValid() && parent.column() != 0 )
    return 0;
  return 1;
}

QVariant DescendantEntitiesModel::data( const QModelIndex & index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();
  Q_D( const DescendantEntitiesModel );

  int entityType = d->m_clientSideEntityStorage->entityTypeForInternalIdentifier(index.internalId());
  if (entityType == ClientSideEntityStorage::CollectionType)
  {
    const Collection col = d->m_clientSideEntityStorage->getCollection( index.internalId() );
    if ( !col.isValid() )
      return QVariant();
    if ( index.column() == 0 && ( role == Qt::DisplayRole || role == Qt::EditRole ) ) {
      if ( col.hasAttribute<EntityDisplayAttribute>() &&
           !col.attribute<EntityDisplayAttribute>()->displayName().isEmpty() )
        return col.attribute<EntityDisplayAttribute>()->displayName();
      return col.name();
    }
    switch ( role ) {

    case Qt::DisplayRole:
    case Qt::EditRole:
      if ( index.column() == 0 ) {
        if ( col.hasAttribute<EntityDisplayAttribute>() &&
             !col.attribute<EntityDisplayAttribute>()->displayName().isEmpty() ) {
          return col.attribute<EntityDisplayAttribute>()->displayName();
        }
        return col.name();
      }
      break;

    case Qt::DecorationRole:
      if ( col.hasAttribute<EntityDisplayAttribute>() &&
           ! col.attribute<EntityDisplayAttribute>()->iconName().isEmpty() ) {
        return col.attribute<EntityDisplayAttribute>()->icon();
      }
      return KIcon( CollectionUtils::defaultIconName( col ) );

    case MimeTypeRole:
      return col.mimeType();

    case RemoteIdRole:
      return col.remoteId();

    case CollectionIdRole:
      return col.id();

    case CollectionRole: {
      return QVariant::fromValue( col );
    }
    default:
      break;
    }
  }
  else if ( entityType == ClientSideEntityStorage::ItemType)
  {
    const Item item = d->m_clientSideEntityStorage->getItem( index.internalId() );
    if ( !item.isValid() )
      return QVariant();

    switch ( role ) {
    case Qt::DisplayRole:
    case Qt::EditRole:
      if ( item.hasAttribute<EntityDisplayAttribute>() &&
           ! item.attribute<EntityDisplayAttribute>()->displayName().isEmpty() )
        return item.attribute<EntityDisplayAttribute>()->displayName();
      return item.remoteId();

    case Qt::DecorationRole:
      if ( item.hasAttribute<EntityDisplayAttribute>() &&
           ! item.attribute<EntityDisplayAttribute>()->iconName().isEmpty() )
        return item.attribute<EntityDisplayAttribute>()->icon();

    case MimeTypeRole:
      return item.mimeType();

    case RemoteIdRole:
      return item.remoteId();

    case ItemRole:
      return QVariant::fromValue( item );

    case ItemIdRole:
      return item.id();

    default:
      break;
    }

  }
  return QVariant();

}


Qt::ItemFlags DescendantEntitiesModel::flags( const QModelIndex & index ) const
{
  Q_D( const DescendantEntitiesModel );

  // Pass modeltest.
  // http://labs.trolltech.com/forums/topic/79
  if ( !index.isValid() )
    return 0;


  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

//   Only show and enable items in columns other than 0.
  if ( index.column() != 0 )
    return flags;

  int entityType = d->m_clientSideEntityStorage->entityTypeForInternalIdentifier(index.internalId());

  if (entityType == ClientSideEntityStorage::CollectionType)
  {
    const Collection col = d->m_clientSideEntityStorage->getCollection( index.internalId() );
    if ( col.isValid() ) {
      int rights = col.rights();
      if ( rights & Collection::CanChangeCollection ) {
        flags |= Qt::ItemIsEditable;
      }

      if ( rights & Collection::CanDeleteCollection ) {
        // If this collection is moved, it will need to be deleted
        flags |= Qt::ItemIsDragEnabled;
      }
    }
  } else if ( entityType == ClientSideEntityStorage::ItemType )
  {
    // Rights come from the parent collection.

    // TODO: Is this right for the root collection? I think so, but only by chance.
    // But will it work if m_rootCollection is different from Collection::root?
    // Should probably rely on index.parent().isValid() for that.
    const Collection parentCol = d->m_clientSideEntityStorage->getCollection( index.parent().internalId() );
    if ( parentCol.isValid() ) {
      int rights = parentCol.rights();
      // Can't drop onto items.
      if ( rights & Collection::CanChangeItem ) {
        flags = flags | Qt::ItemIsEditable;
      }
    }
  }
  return flags;
}

QStringList DescendantEntitiesModel::mimeTypes() const
{
  // TODO: Should this return the mimetypes that the items provide? Allow dragging a contact from here for example.
  return QStringList() << QLatin1String( "text/uri-list" );
}

QModelIndex DescendantEntitiesModel::index( int row, int column, const QModelIndex & parent ) const
{
  Q_D( const DescendantEntitiesModel );

//   kDebug() << row << column << parent;
  if ( (row < 0) || (column != 0) )
    return QModelIndex();

  // No valid row has any child items.
  if (parent.isValid())
    return QModelIndex();

  bool ok;
  qint64 id = d->findIdForRow(row, Collection::root().id(), &ok);
//   kDebug() << ok << id << createIndex(row, column, reinterpret_cast<void*>(id));
  if (ok)
    return createIndex(row, column, reinterpret_cast<void*>(id));
  return QModelIndex();

}

QModelIndex DescendantEntitiesModel::parent( const QModelIndex & index ) const
{
//   Q_D( const DescendantEntitiesModel );

  return QModelIndex();
}

int DescendantEntitiesModel::rowCount( const QModelIndex & parent ) const
{
  Q_D( const DescendantEntitiesModel );

  if (parent.isValid())
    return 0;

  int count = d->descendantCount( Collection::root().id() );
//   kDebug() << count;
  return count;
}

QVariant DescendantEntitiesModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return i18nc( "@title:column, name of a thing", "Name" );
  return QAbstractItemModel::headerData( section, orientation, role );
}

QMimeData *DescendantEntitiesModel::mimeData( const QModelIndexList &indexes ) const
{
  Q_D( const DescendantEntitiesModel );
  QMimeData *data = new QMimeData();
  KUrl::List urls;
  foreach( const QModelIndex &index, indexes ) {
    if ( index.column() != 0 )
      continue;

    int entityType = d->m_clientSideEntityStorage->entityTypeForInternalIdentifier( index.internalId() );

    if (entityType == ClientSideEntityStorage::CollectionType)
    {
      urls << d->m_clientSideEntityStorage->getCollection( index.internalId() ).url();
    } else if ( entityType == ClientSideEntityStorage::ItemType )
    {
      urls << d->m_clientSideEntityStorage->getItem( index.internalId() ).url( Item::UrlWithMimeType );
    }
  }
  urls.populateMimeData( data );

  return data;
}

// Always return false for actions which take place syncronously, eg via a Job.
bool DescendantEntitiesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_D( DescendantEntitiesModel );
  // Delegate to something? Akonadi updater, or the manager classes? I think akonadiUpdater. entityUpdateAdapter
  if ( index.column() == 0 && ( role & (Qt::EditRole | ItemRole | CollectionRole) ) ) {
    int entityType = d->m_clientSideEntityStorage->entityTypeForInternalIdentifier( index.internalId() );
    if ( ClientSideEntityStorage::CollectionType == entityType )
    {
      // rename collection
//       Collection col = d->collectionForIndex( index );
      Collection col = d->m_clientSideEntityStorage->getCollection( index.internalId() );
      if ( !col.isValid() || value.toString().isEmpty() )
        return false;

// //       if ( col.hasAttribute< EntityDisplayAttribute >() )
// //       {
//       EntityDisplayAttribute *displayAttribute = col.attribute<EntityDisplayAttribute>( Entity::AddIfMissing );
//       displayAttribute->setDisplayName( value.toString() );
//           col.addAttribute(displayAttribute);
// //       }

      d->entityUpdateAdapter->updateEntities( Collection::List() << col );
      return false;
    }
    if ( ClientSideEntityStorage::ItemType == entityType )
    {
      Item i = value.value<Item>();
//       Item item = d->m_items.value( index.internalId() );
//       if ( !item.isValid() || value.toString().isEmpty() )
//         return false;

//       if ( item.hasAttribute< EntityDisplayAttribute >() )
//       {
//       EntityDisplayAttribute *displayAttribute = item.attribute<EntityDisplayAttribute>( Entity::AddIfMissing );
//       displayAttribute->setDisplayName( value.toString() );
//       }
//           item.addAttribute(displayAttribute);
      d->entityUpdateAdapter->updateEntities( Item::List() << i );
//       d->entityUpdateAdapter->updateEntities( Item::List() << item );
        return false;
    }
  }

  return QAbstractItemModel::setData( index, value, role );
}

bool DescendantEntitiesModel::insertRows(int , int, const QModelIndex&)
{
    return false;
}

bool DescendantEntitiesModel::insertColumns(int, int, const QModelIndex&)
{
    return false;
}

bool DescendantEntitiesModel::removeRows(int, int, const QModelIndex&)
{
    return false;
}

bool DescendantEntitiesModel::removeColumns(int, int, const QModelIndex&)
{
    return false;
}



#include "descendantentitiesmodel.moc"
