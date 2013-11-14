/*
    This file is part of Akregator2.

    Copyright (C) 2013 Dan Vrátil <dvratil@redhat.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "searchproxymodel.h"

#include <KRss/Item>
#include <KRss/FeedItemModel>
#include <KRss/FeedCollection>

using namespace Akregator2;
using namespace Akonadi;

class SearchProxyModel::Private
{

  public:
    Private(SearchProxyModel *parent):
      q(parent)
    { }

    ~Private()
    { }

    QString buildFullPath( const Akonadi::Collection &collection ) const;
    QModelIndex sourceParent();

    Akonadi::Collection collection;
    mutable QHash<Akonadi::Entity::Id,QString> collectionsPathMap;

  private:
    SearchProxyModel *q;

};

SearchProxyModel::SearchProxyModel( QObject* parent ):
    EntityMimeTypeFilterModel( parent ),
    d( new Private( this ) )
{
    setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
    addMimeTypeInclusionFilter( KRss::Item::mimeType() );
    setSortRole( KRss::FeedItemModel::SortRole );
    setDynamicSortFilter( true );
}

SearchProxyModel::~SearchProxyModel()
{
    delete d;
}

void SearchProxyModel::setCollection( const Collection& collection )
{
    d->collection = collection;

    /* If the model is lazy-populated, we need to force fetching items in the collection */
    if (d->collection.isValid() && sourceModel()) {
      sourceModel()->fetchMore( d->sourceParent() );
    }
}

void SearchProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (sourceModel && d->collection.isValid()) {
      sourceModel->fetchMore( d->sourceParent() );
    }
}


QModelIndex SearchProxyModel::index( int row, int column, const QModelIndex& parent ) const
{
    /* This is a flat model */
    if ( parent.isValid() ) {
      return QModelIndex();
    }

    return createIndex( row, column, 0 );
}

QVariant SearchProxyModel::data( const QModelIndex& index, int role ) const
{
    QModelIndex sourceParent = d->sourceParent();
    if ( !sourceParent.isValid() ) {
      return QVariant();
    }

    QModelIndex sourceIndex = sourceModel()->index( index.row(), index.column(), sourceParent );
    if ( !sourceIndex.isValid() ) {
      return QVariant();
    }

    if ( index.column() == KRss::FeedItemModel::FeedTitleForItemColumn && role == Qt::DisplayRole ) {
      const Item item = sourceIndex.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      Collection parentCollection = item.parentCollection();

      if ( !d->collectionsPathMap.contains( parentCollection.id() ) ) {
        /* Get index of the collection that is parent of the original item (the index
          points to item linked to m_collection) */
        QModelIndex parentCollectionIndex = EntityTreeModel::modelIndexForCollection( sourceModel(), parentCollection );
        parentCollection = parentCollectionIndex.data( EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
        d->collectionsPathMap.insert( parentCollection.id(), d->buildFullPath(parentCollection) );
      }

      return d->collectionsPathMap[ parentCollection.id() ];
    }

    return sourceIndex.data( role );
}

QModelIndex SearchProxyModel::mapFromSource( const QModelIndex& sourceIndex ) const
{
    if ( !sourceIndex.isValid() ) {
      return QModelIndex();
    }

    return index( sourceIndex.row(), sourceIndex.column(), QModelIndex() );
}

QModelIndex SearchProxyModel::mapToSource( const QModelIndex& proxyIndex ) const
{
    if ( !proxyIndex.isValid() ) {
      return d->sourceParent();
    }

    return sourceModel()->index( proxyIndex.row(), proxyIndex.column(), d->sourceParent() );
}


QModelIndex SearchProxyModel::parent( const QModelIndex& child ) const
{
    Q_UNUSED( child )

    /* This is a flat, single-level model */
    return QModelIndex();
}

int SearchProxyModel::rowCount( const QModelIndex& parent ) const
{
    if ( !parent.isValid() ) {
      return sourceModel()->rowCount( d->sourceParent() );
    }

    return 0;
}

QString SearchProxyModel::Private::buildFullPath( const Collection& collection ) const
{
    QString path = KRss::FeedCollection(collection).title();
    if ( collection.parentCollection().isValid() ) {
        QModelIndex index = EntityTreeModel::modelIndexForCollection( q->sourceModel(), collection.parentCollection() );
        const Collection parentCollection = index.data( EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
        QString subpath = buildFullPath( parentCollection );
        if (! subpath.isEmpty() ) {
          path = subpath + QLatin1String(" / ") + path;
        }
    }

    return path;
}

QModelIndex SearchProxyModel::Private::sourceParent()
{
    if ( q->sourceModel() && collection.isValid() ) {
      return EntityTreeModel::modelIndexForCollection( q->sourceModel(), collection );
    }

    return QModelIndex();
}
