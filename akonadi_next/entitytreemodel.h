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
class EntityUpdateAdapter;
class ClientSideEntityStorage;

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

  // EntityTreeModel( EntityUpdateAdapter,
  //                  MonitorAdapter,
  //                  QStringList mimeFilter = QStringList(), QObject *parent = 0);

  /**
   * Creates a new collection and item model.
   *
   * @param parent The parent object.
   * @param mimeTypes The list of mimetypes to be retrieved in the model.
   */
  EntityTreeModel( EntityUpdateAdapter *entityUpdateAdapter,
                            ClientSideEntityStorage *clientSideEntityStorage,
                            QObject *parent = 0
// TODO: figure out what to do about this:
// I think if you want to show stats, you fetch them in the monitor.
// This model should show them if they are fetched.
//                             int showStats = EntityTreeModel::DoNotShowStatistics
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
  virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

  virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex parent( const QModelIndex & index ) const;

private:
  Q_DECLARE_PRIVATE( EntityTreeModel )
  //@cond PRIVATE
  EntityTreeModelPrivate *d_ptr;

  // Make these private, they shouldn't be called by applications
  virtual bool insertRows(int , int, const QModelIndex & = QModelIndex());
  virtual bool insertColumns(int, int, const QModelIndex & = QModelIndex());
  virtual bool removeRows(int, int, const QModelIndex & = QModelIndex());
  virtual bool removeColumns(int, int, const QModelIndex & = QModelIndex());

  Q_PRIVATE_SLOT( d_func(), void rowsAboutToBeInserted( Collection::Id colId, int start, int end ) )
  Q_PRIVATE_SLOT( d_func(), void rowsAboutToBeRemoved( Collection::Id colId, int start, int end ) )
  Q_PRIVATE_SLOT( d_func(), void rowsInserted() )
  Q_PRIVATE_SLOT( d_func(), void rowsRemoved() )

  //@endcond


};

} // namespace

#endif
