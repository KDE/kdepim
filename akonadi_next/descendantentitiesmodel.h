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

#ifndef AKONADI_DESCENDANTENTITIESMODEL_H
#define AKONADI_DESCENDANTENTITIESMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>

#include <akonadi/collection.h>


namespace Akonadi
{

class Collection;
class Item;
class EntityUpdateAdapter;
class ClientSideEntityStorage;

class DescendantEntitiesModelPrivate;

/**
 * @short A model for showing all descendant items below a single parent.
 *
 * This class uses pre-order traversal to expand a tree of Entities in a model
 * to the form of a list.
 *
 * Because pre-order traversal is used, items appear in the same order as they would appear in a
 * fully expanded tree.
 *
 * The resulting model includes Collections. They can be filtered out if necessary
 * using an EntityFilterProxyModel:
 *
 * @code
 *
 *   DescendantEntitiesModel *model = new DescendantEntitiesModel( entityUpdateAdapter,
 *                                                                 clientSideEntityStorage, this );
 *   EntityFilterProxyModel *proxy = new EntityFilterProxyModel();
 *   proxy->setSourceModel(model);
 *   proxy->addMimeTypeExclusionFilter( Collection::mimeType() );
 *   EntityTreeView *view = new EntityTreeView( this );
 *   view->setModel( proxy );
 *
 * @endcode
 *
 *  \image html howtousedem.png "How To Use DescendantEntitiesModel"
 *
 * It is not possible to drop items onto this model.
 *
 * @author Stephen Kelly <steveire@gmail.com>
 * @since 4.3
 */
class DescendantEntitiesModel : public QAbstractItemModel
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

  /**
   * Creates a new collection and item model.
   *
   * @param parent The parent object.
   * @param mimeTypes The list of mimetypes to be retrieved in the model.
   */
  DescendantEntitiesModel( EntityUpdateAdapter *entityUpdateAdapter,
                            ClientSideEntityStorage *clientSideEntityStorage,
                            QObject *parent = 0 );

  /**
   * Destroys the collection and item model.
   */
  virtual ~DescendantEntitiesModel();

  virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;
  virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;

  virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
  virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

  virtual Qt::ItemFlags flags( const QModelIndex &index ) const;
  virtual QStringList mimeTypes() const;

  virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;
  virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

  virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
  virtual QModelIndex parent( const QModelIndex & index ) const;

private:
  Q_DECLARE_PRIVATE( DescendantEntitiesModel )
  //@cond PRIVATE
  DescendantEntitiesModelPrivate *d_ptr;

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
