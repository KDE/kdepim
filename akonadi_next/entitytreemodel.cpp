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

#include "entitytreemodel.h"
#include "entitytreemodel_p.h"

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
#include <akonadi/monitor.h>

#include "kdebug.h"

using namespace Akonadi;

EntityTreeModel::EntityTreeModel( EntityUpdateAdapter *entityUpdateAdapter,
                                  Monitor *monitor,
                                  QStringList mimetypes,
                                  QObject *parent,
                                  Collection rootCollection,
                                  int entitiesToFetch,
                                  int showStats
                                )
    : QAbstractItemModel( parent ),
    d_ptr( new EntityTreeModelPrivate( this ) )
{
  Q_D( EntityTreeModel );
  d->m_mimeTypeFilter = mimetypes;

  d->entityUpdateAdapter = entityUpdateAdapter;
  d->monitor = monitor;
  d->m_rootCollection = rootCollection;
  d->m_showStats = ( showStats == ShowStatistics ) ? true : false;
  d->m_entitiesToFetch = entitiesToFetch;

  AttributeFactory::registerAttribute<CollectionChildOrderAttribute>();

  d->init();
}

EntityTreeModel::~EntityTreeModel()
{
  Q_D( EntityTreeModel );
  d->collections.clear();
  d->m_items.clear();
  d->m_childEntities.clear();
}

int EntityTreeModel::columnCount( const QModelIndex & parent ) const
{
// TODO: Subscriptions? Statistics?
  if ( parent.isValid() && parent.column() != 0 )
    return 0;
  return 1;
}

QVariant EntityTreeModel::data( const QModelIndex & index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();
  Q_D( const EntityTreeModel );
//   return d->modelEntityManager.data(index, role);

  if ( d->isItem( index ) ) {
//     Item item = modelEntityManager.getItem(index.internalId());
//     return itemManager.data(index, role);


    Item item = d->m_items.value( index.internalId() );

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

//   modelEntityManager->isCollection(index.internalId());
  if ( d->isCollection( index ) ) {
//     Collection col = modelEntityManager.getCollection(index.internalId());
//     return collectionManager.data(index, role);
    const Collection col = d->collectionForIndex( index );
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
  return QVariant();
}


Qt::ItemFlags EntityTreeModel::flags( const QModelIndex & index ) const
{
  Q_D( const EntityTreeModel );

  // Pass modeltest.
  // http://labs.trolltech.com/forums/topic/79
  if ( !index.isValid() )
    return 0;


  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

//   Only show and enable items in columns other than 0.
  if ( index.column() != 0 )
    return flags;

  if ( d->isCollection( index ) ) {
    const Collection col = d->collectionForIndex( index );
    if ( col.isValid() ) {
      int rights = col.rights();
      if ( rights & Collection::CanChangeCollection ) {
        flags |= Qt::ItemIsEditable;
        // Changing the collection includes changing the metadata (child entityordering).
        // Need to allow this by drag and drop.
        flags |= Qt::ItemIsDropEnabled;
      }

      if ( rights & Collection::CanDeleteCollection ) {
        // If this collection is moved, it will need to be deleted
        flags |= Qt::ItemIsDragEnabled;
      }
      if ( rights & ( Collection::CanCreateCollection | Collection::CanCreateItem ) ) {
//           Can we drop new collections and items into this collection?
        flags |= Qt::ItemIsDropEnabled;
      }
    }
  } else if ( d->isItem( index ) ) {
    // Rights come from the parent collection.

    // TODO: Is this right for the root collection? I think so, but only by chance.
    // But will it work if m_rootCollection is different from Collection::root?
    // Should probably rely on index.parent().isValid() for that.
    const Collection parentCol = d->collectionForIndex( index.parent() );
    if ( parentCol.isValid() ) {
      int rights = parentCol.rights();
      // Can't drop onto items.
      if ( rights & Collection::CanChangeItem ) {
        flags = flags | Qt::ItemIsEditable;
      }
      if ( rights & Collection::CanDeleteItem ) {
        // If this item is moved, it will need to be deleted from its parent.
        flags = flags | Qt::ItemIsDragEnabled;
      }
    }
  }
  return flags;
}

Qt::DropActions EntityTreeModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

QStringList EntityTreeModel::mimeTypes() const
{
  // TODO: Should this return the mimetypes that the items provide? Allow dragging a contact from here for example.
  return QStringList() << QLatin1String( "text/uri-list" );
}

bool EntityTreeModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
{
  Q_D( EntityTreeModel );

  // TODO Use action and collection rights and return false if neccessary

// if row and column are -1, then the drop was on parent directly.
// data should then be appended on the end of the items of the collections as appropriate.
// That will mean begin insert rows etc.
// Otherwise it was a sibling of the row^th item of parent.
// That will need to be handled by a proxy model. This one can't handle ordering.
// if parent is invalid the drop occured somewhere on the view that is no model, and corresponds to the root.
  kDebug() << "ismove" << ( action == Qt::MoveAction );
  if ( action == Qt::IgnoreAction )
    return true;

// Shouldn't do this. Need to be able to drop vcards for example.
//   if (!data->hasFormat("text/uri-list"))
//       return false;

// This is probably wrong and unneccessary.
  if ( column > 0 )
    return false;

  kDebug() << "isItem" << d->isItem( parent );
  if ( d->isItem( parent ) ) {
    // Can't drop data onto an item, although we can drop data between items.
    return false;
    // TODO: Maybe if it's a drop on an item I should drop below the item instead?
    // Find out what others do.
  }

  kDebug() << "d->isCollection" << d->isCollection( parent );
  if ( d->isCollection( parent ) ) {
    Collection destCol = d->collectionForIndex( parent );

    if ( data->hasFormat( "text/uri-list" ) ) {
      QHash<Collection::Id, Collection::List> dropped_cols;
      QHash<Collection::Id, Item::List> dropped_items;

      KUrl::List urls = KUrl::List::fromMimeData( data );
      foreach( const KUrl &url, urls ) {
        Collection col = d->collections.value( Collection::fromUrl( url ).id() );
        kDebug() << "url" << url << col.mimeType();
        if ( col.isValid() ) {
          if ( !d->mimetypeMatches( destCol.contentMimeTypes(), col.contentMimeTypes() ) )
            return false;

          dropped_cols[ col.parent()].append( col );
        } else {
          Item item = d->m_items.value( Item::fromUrl( url ).id() * -1 );
          kDebug() << item.remoteId();
          if ( item.isValid() ) {
            Collection col = d->findParentCollection( item );
            kDebug() << col.id() << item.remoteId();
            dropped_items[ col.id()].append( item );
          } else {
            // A uri, but not an akonadi url. What to do?
            // Should handle known mimetypes like vcards first.
            // That should make any remaining uris meaningless at this point.
          }
        }
      }
      QHashIterator<Collection::Id, Item::List> item_iter( dropped_items );
      d->entityUpdateAdapter->beginTransaction();
      while ( item_iter.hasNext() ) {
        item_iter.next();
//         Collection srcCol = collectionForIndex( indexForId( item_iter.key() ) );
        Collection srcCol = d->collections.value( item_iter.key() );
        if ( action == Qt::MoveAction ) {
          d->entityUpdateAdapter->moveEntities( item_iter.value(), dropped_cols.value( item_iter.key() ), srcCol, destCol, row );
        } else if ( action == Qt::CopyAction ) {
          d->entityUpdateAdapter->addEntities( item_iter.value(), dropped_cols.value( item_iter.key() ), destCol, row );
        }
        dropped_cols.remove( item_iter.key() );
      }
      QHashIterator<Collection::Id, Collection::List> col_iter( dropped_cols );
      while ( col_iter.hasNext() ) {
        col_iter.next();
//         Collection srcCol = collectionForIndex( indexForId( col_iter.key() ) );
        Collection srcCol = d->collections.value( col_iter.key() );
        if ( action == Qt::MoveAction ) {
          // Item::List() because I know I've already dealt with the items of this parent.
          d->entityUpdateAdapter->moveEntities( Item::List(), col_iter.value(), srcCol, destCol, row );
        } else if ( action == Qt::CopyAction ) {
          d->entityUpdateAdapter->addEntities( Item::List(), col_iter.value(), destCol, row );
        }
      }
      d->entityUpdateAdapter->endTransaction();
      return false; // ### Return false so that the view does not update with the dropped
      // in place where they were dropped. That will be done when the monitor notifies the model
      // through collectionsReceived that the move was successful.
    } else {
//       not a set of uris. Maybe vcards etc. Check if the parent supports them, and maybe do
      // fromMimeData for them. Hmm, put it in the same transaction with the above?
      // TODO: This should be handled first, not last.
    }
  }
  return false;
}

QModelIndex EntityTreeModel::index( int row, int column, const QModelIndex & parent ) const
{

  Q_D( const EntityTreeModel );

  //TODO: don't use column count here. Use some d-> func.
  if ( column >= columnCount() || column < 0 )
    return QModelIndex();

  Collection col = d->collectionForIndex( parent );

  QList<qint64> idList = d->m_childEntities.value( col.id() );

  if ( row < 0 || row >= idList.size() )
    return QModelIndex();

  return createIndex( row, column, reinterpret_cast<void*>( idList.at( row ) ) );
}

QModelIndex EntityTreeModel::parent( const QModelIndex & index ) const
{
  Q_D( const EntityTreeModel );

  if ( !index.isValid() )
    return QModelIndex();

  Collection col = d->findParentCollection( index.internalId() );
  if ( !col.isValid() || col.id() == d->m_rootCollection.id() )
    return QModelIndex();

  int row = d->m_childEntities[ col.parent()].indexOf( col.id() );
  return createIndex( row, 0, reinterpret_cast<void*>( col.id() ) );

}

int EntityTreeModel::rowCount( const QModelIndex & parent ) const
{
  Q_D( const EntityTreeModel );
  return d->childEntitiesCount( parent );
}

QVariant EntityTreeModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole )
    return i18nc( "@title:column, name of a thing", "Name" );
  return QAbstractItemModel::headerData( section, orientation, role );
}

QMimeData *EntityTreeModel::mimeData( const QModelIndexList &indexes ) const
{
  Q_D( const EntityTreeModel );
  QMimeData *data = new QMimeData();
  KUrl::List urls;
  foreach( const QModelIndex &index, indexes ) {
    if ( index.column() != 0 )
      continue;

    if ( d->isCollection( index ) ) {
      urls << d->collectionForIndex( index ).url();
    }

    if ( d->isItem( index ) ) {
      urls << d->m_items.value( index.internalId() ).url( Item::UrlWithMimeType );
    }
  }
  urls.populateMimeData( data );

  return data;
}

// Always return false for actions which take place syncronously, eg via a Job.
bool EntityTreeModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  Q_D( EntityTreeModel );
  kDebug() << index.column()
      << (role & (Qt::EditRole | ItemRole | CollectionRole))
      << role << (Qt::EditRole | ItemRole | CollectionRole) ;
  // Delegate to something? Akonadi updater, or the manager classes? I think akonadiUpdater. entityUpdateAdapter
  if ( index.column() == 0 && role & (Qt::EditRole | ItemRole | CollectionRole) ) {
    kDebug() << "ok";
    if ( d->isCollection( index ) ) {
      // rename collection
      Collection col = d->collectionForIndex( index );
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
    if ( d->isItem( index ) ) {
      kDebug() << "item";
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

#include "entitytreemodel.moc"
