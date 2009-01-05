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

#ifndef AKONADI_ENTITYTREEMODEL_H
#define AKONADI_ENTITYTREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>

#include <akonadi/collection.h>

namespace Akonadi
{

class Collection;
class Item;
class Monitor;
class EntityUpdateAdapter;

class EntityTreeModelPrivate;

/**
 * @short A model for collections and items together.
 *
 * This class provides the interface of QAbstractItemModel for the
 * collection and item tree of the Akonadi storage.
 *
 * Child elements of a collection consist of the child collections
 * followed by the items. This arrangement can be modified using a proxy model.
 *
 * @code
 *
 *   EntityTreeModel *model = new EntityTreeModel( this, QStringList() << FooMimeType << BarMimeType );
 *
 *   QTreeView *view = new QTreeView( this );
 *   view->setModel( model );
 *
 * @endcode
 *
 * Only collections and items matching @p mimeTypes will be shown. This way,
 * retrieving every item in Akonadi is avoided.
 *
 * @author Stephen Kelly <steveire@gmail.com>
 * @since 4.3
 */
class EntityTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  /**
   * Describes the roles for items. Roles for collections are defined by the superclass.
   */
  enum Roles {
    CollectionRole = Qt::UserRole,          ///< The collection.
    CollectionIdRole,                       ///< The collection id.
    ItemRole,                               ///< The Item
    ItemIdRole,                             ///< The item id
    MimeTypeRole,                           ///< The mimetype of the entity
    RemoteIdRole,                           ///< The remoteId of the entity
    CollectionChildOrderRole,               ///< Ordered list of child items if available
    UserRole = Qt::UserRole + 1000          ///< Role for user extensions.
  };

  enum ShouldShowStatistics {
    ShowStatistics,            ///< Show collection statistics
    DoNotShowStatistics        ///< Do not show collection statistics.
  };

  // EntityTreeModel( EntityUpdateAdapter,
  //                  MonitorAdapter,
  //                  QStringList mimeFilter = QStringList(), QObject *parent = 0);

  /**
  What to fetch and represent in the model.
  */
  enum EntitiesToFetch {
    FetchNothing = 0,                     /// Fetch nothing. This creates an empty model.
    FetchItems = 1,                       /// Fetch items in the rootCollection
    FetchFirstLevelChildCollections = 2,  /// Fetch first level collections in the root collection.
    FetchCollectionsRecursive = 4    /// Fetch collections in the root collection recursively. This implies FetchFirstLevelChildCollections.
  };

  /**
   * Creates a new collection and item model.
   *
   * @param parent The parent object.
   * @param mimeTypes The list of mimetypes to be retrieved in the model.
   */
  explicit EntityTreeModel( EntityUpdateAdapter *entityUpdateAdapter,
                            Monitor *monitor,
                            QStringList mimeTypes = QStringList(),
                            QObject *parent = 0,
                            Collection rootCollection = Collection::root(),
// TODO: figure out what to do about this:
                            int entitiesToFetch = EntityTreeModel::FetchCollectionsRecursive,
                            int showStats = EntityTreeModel::DoNotShowStatistics
                          );

  /**
   * Destroys the collection and item model.
   */
  virtual ~EntityTreeModel();

  virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
  virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

  virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
  virtual QStringList mimeTypes() const;

  virtual Qt::DropActions supportedDropActions() const;
  virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;
  virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
//     virtual void moveEntities(QModelIndexList sourceIndexes, QModelIndex destParentIndex);
  virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

  virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex parent( const QModelIndex & index ) const;

private:
  Q_DECLARE_PRIVATE( EntityTreeModel )
  //@cond PRIVATE
  EntityTreeModelPrivate *d_ptr;

  Q_PRIVATE_SLOT( d_func(), void onRowsInserted( const QModelIndex &parent, int start, int end ) )
  Q_PRIVATE_SLOT( d_func(), void collectionsReceived( const Akonadi::Collection::List& ) )
  Q_PRIVATE_SLOT( d_func(), void itemsReceived( const Akonadi::Item::List&, Collection::Id ) )

  Q_PRIVATE_SLOT( d_func(), void collectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void collectionRemoved( const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void collectionChanged( const Akonadi::Collection& ) )

  Q_PRIVATE_SLOT( d_func(), void itemAdded( const Akonadi::Item&, const Akonadi::Collection& ) )
  Q_PRIVATE_SLOT( d_func(), void itemRemoved( const Akonadi::Item& ) )
  Q_PRIVATE_SLOT( d_func(), void itemChanged( const Akonadi::Item&, const QSet<QByteArray>& ) )
  Q_PRIVATE_SLOT( d_func(), void itemMoved( const Akonadi::Item&,
                  const Akonadi::Collection&, const Akonadi::Collection& ) )

  Q_PRIVATE_SLOT( d_func(), void collectionStatisticsChanged(
                    Akonadi::Collection::Id, const Akonadi::CollectionStatistics& ) )
  //@endcond


};

} // namespace

#endif
