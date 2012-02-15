/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2010 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "entitycollectionorderproxymodel.h"
#include "mailkernel.h"

#include <Akonadi/Collection>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/KMime/SpecialMailCollections>

namespace MailCommon {

class EntityCollectionOrderProxyModel::EntityCollectionOrderProxyModelPrivate
{
  public:
    EntityCollectionOrderProxyModelPrivate()
      : manualSortingActive( false )
    {
    }

    int collectionRank( const Akonadi::Collection &collection )
    {
      const Akonadi::Collection::Id id = collection.id();
      if ( collectionRanks.contains( id ) ) {
        return collectionRanks[ id ];
      }

      int rank = 100;
      if ( Kernel::folderIsInbox( collection, true ) ) {
        rank = 1;
      } else if ( Kernel::self()->folderIsDraftOrOutbox( collection ) ) {
        if ( Kernel::self()->folderIsDrafts( collection ) ) {
          rank = 5;
        } else {
          rank = 2;
        }
      } else if ( Kernel::self()->folderIsSentMailFolder( collection ) ) {
        rank = 3;
      } else if ( Kernel::self()->folderIsTrash( collection ) ) {
        rank = 4;
      } else if ( Kernel::self()->folderIsTemplates( collection ) ) {
        rank = 6;
      }
      collectionRanks.insert( id, rank );
      return rank;
    }

    bool manualSortingActive;
    QMap<Akonadi::Collection::Id, int> collectionRanks;
};

EntityCollectionOrderProxyModel::EntityCollectionOrderProxyModel( QObject *parent )
  : EntityOrderProxyModel( parent ), d( new EntityCollectionOrderProxyModelPrivate() )
{
  setDynamicSortFilter( true );
  setSortCaseSensitivity( Qt::CaseInsensitive );
  connect( Akonadi::SpecialMailCollections::self(), SIGNAL(defaultCollectionsChanged()),
           this, SLOT(slotDefaultCollectionsChanged()) );

}

EntityCollectionOrderProxyModel::~EntityCollectionOrderProxyModel()
{
  if ( d->manualSortingActive ) {
    saveOrder();
  }
  delete d;
}

void EntityCollectionOrderProxyModel::slotDefaultCollectionsChanged()
{
  if ( !d->manualSortingActive ) {
    d->collectionRanks.clear();
    invalidate();
  }
}

bool EntityCollectionOrderProxyModel::lessThan( const QModelIndex &left,
                                                const QModelIndex &right ) const
{
  if ( !d->manualSortingActive ) {

    Akonadi::Collection leftData =
      left.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    Akonadi::Collection rightData =
      right.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();

    int rankLeft = d->collectionRank( leftData );
    int rankRight = d->collectionRank( rightData );

    if ( rankLeft < rankRight ) {
      return true;
    } else if ( rankLeft > rankRight ) {
      return false;
    }

    return QSortFilterProxyModel::lessThan( left, right );
  }
  return EntityOrderProxyModel::lessThan( left, right );
}

void EntityCollectionOrderProxyModel::setManualSortingActive( bool active )
{
  if ( d->manualSortingActive == active ) {
    return;
  }

  d->manualSortingActive = active;
  d->collectionRanks.clear();
  invalidate();
}

bool EntityCollectionOrderProxyModel::isManualSortingActive() const
{
  return d->manualSortingActive;
}

}

#include "entitycollectionorderproxymodel.moc"
