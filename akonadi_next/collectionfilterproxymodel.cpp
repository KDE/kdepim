/*
    Copyright (c) 2007 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "collectionfilterproxymodel.h"

#include "collectionmodel.h"

#include <kdebug.h>
#include <kmimetype.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

using namespace Akonadi;

/**
 * @internal
 */
class CollectionFilterProxyModel::Private
{
  public:
    Private( CollectionFilterProxyModel *parent )
      : mParent( parent )
    {
      mimeTypes << QLatin1String( "text/uri-list" );
    }

    bool collectionAccepted( const QModelIndex &index, bool checkResourceVisibility = true );

    QList< QModelIndex > acceptedResources;
    CollectionFilterProxyModel *mParent;
    QStringList mimeTypes;
};

bool CollectionFilterProxyModel::Private::collectionAccepted( const QModelIndex &index, bool checkResourceVisibility )
{
  // Retrieve supported mimetypes
  QStringList collectionMimeTypes = mParent->sourceModel()->data( index, CollectionModel::CollectionRole ).value<Collection>().contentMimeTypes();

  // If this collection directly contains one valid mimetype, it is accepted
  foreach ( const QString &type, collectionMimeTypes ) {
    if ( mimeTypes.contains( type ) ) {

      // The folder will be accepted, but we need to make sure the resource is visible too.
      if ( checkResourceVisibility ) {

        // find the resource
        QModelIndex resource = index;
        while ( resource.parent().isValid() )
          resource = resource.parent();

        // See if that resource is visible, if not, reset the model.
        if ( resource != index && !acceptedResources.contains( resource ) ) {
          kDebug() << "We got a new collection:" << mParent->sourceModel()->data( index ).toString()
                   << "but the resource is not visible:" << mParent->sourceModel()->data( resource ).toString();
          acceptedResources.clear();
          mParent->reset();
          return true;
        }
      }

      // Keep track of all the resources that are visible.
      if ( !index.parent().isValid() )
        acceptedResources.append( index );

      return true;
    }

    KMimeType::Ptr mimeType = KMimeType::mimeType( type, KMimeType::ResolveAliases );
    if ( !mimeType.isNull() ) {
      foreach ( const QString &mt, mimeTypes ) {
        if ( mimeType->is( mt ) ) {

          // Keep track of all the resources that are visible.
          if ( !index.parent().isValid() )
            acceptedResources.append( index );

          return true;
        }
      }
    }
  }

  // If this collection has a child which contains valid mimetypes, it is accepted
  QModelIndex childIndex = index.child( 0, 0 );
  while ( childIndex.isValid() ) {
    if ( collectionAccepted( childIndex, false /* don't check visibility of the parent, as we are checking the child now */ ) ) {

      // Keep track of all the resources that are visible.
      if ( !index.parent().isValid() )
        acceptedResources.append( index );

      return true;
    }
    childIndex = childIndex.sibling( childIndex.row() + 1, 0 );
  }

  // Or else, no reason to keep this collection.
  return false;
}


CollectionFilterProxyModel::CollectionFilterProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent ),
    d( new Private( this ) )
{
  setSupportedDragActions( Qt::CopyAction | Qt::MoveAction );
}

CollectionFilterProxyModel::~CollectionFilterProxyModel()
{
  delete d;
}

void CollectionFilterProxyModel::addMimeTypeFilters(const QStringList &typeList)
{
  d->mimeTypes << typeList;
  invalidateFilter();
}

void CollectionFilterProxyModel::addMimeTypeFilter(const QString &type)
{
  d->mimeTypes << type;
  invalidateFilter();
}

bool CollectionFilterProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent) const
{
  return d->collectionAccepted( sourceModel()->index( sourceRow, 0, sourceParent ) );
}

QStringList CollectionFilterProxyModel::mimeTypeFilters() const
{
  return d->mimeTypes;
}

void CollectionFilterProxyModel::clearFilters()
{
  d->mimeTypes.clear();
  invalidateFilter();
}

#include "collectionfilterproxymodel.moc"
