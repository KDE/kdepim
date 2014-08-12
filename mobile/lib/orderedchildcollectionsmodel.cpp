/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "orderedchildcollectionsmodel.h"
#include <Akonadi/KMime/SpecialMailCollections>
#include <AkonadiCore/Entity>
#include <AkonadiCore/EntityTreeModel>

OrderedChildCollectionsModel::OrderedChildCollectionsModel( QObject* parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter( true );
  setSortCaseSensitivity( Qt::CaseInsensitive );
  setSortLocaleAware( true );
  // TODO: This does not actually work. The id() of each special collection is -1.
  specialCollectionOrder <<
          Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Inbox ).id()
      <<  Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Outbox ).id()
      <<  Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Trash ).id()
      <<  Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Drafts ).id()
      <<  Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::Templates ).id()
      <<  Akonadi::SpecialMailCollections::self()->defaultCollection( Akonadi::SpecialMailCollections::SentMail ).id();
}

bool OrderedChildCollectionsModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  const Akonadi::Entity::Id leftId = left.data( Akonadi::EntityTreeModel::CollectionIdRole ).toLongLong();
  const Akonadi::Entity::Id rightId = right.data( Akonadi::EntityTreeModel::CollectionIdRole ).toLongLong();
  if ( const int leftIndex = specialCollectionOrder.indexOf( leftId ) >= 0 ) {
    if ( const int rightIndex = specialCollectionOrder.indexOf( rightId ) >= 0 ) {
      return leftIndex < rightIndex;
    }
    // Left is a special collection, right is not.
    return true;
  } else {
    if ( const int rightIndex = specialCollectionOrder.indexOf( rightId ) >= 0 ) {
      // Right is a special collection, left is not.
      return false;
    }
  }
  // Neither is special.

  // First put inbox on top,

  const Akonadi::Collection leftCol = left.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
  if ( leftCol.name().compare( QLatin1String( "inbox" ), Qt::CaseInsensitive ) == 0 ) {
    // If we have two collections called inbox make sure they have total order.
    const Akonadi::Collection rightCol = right.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( rightCol.name().compare( QLatin1String( "inbox" ), Qt::CaseInsensitive ) == 0 )
      return leftId < rightId;
    return true;
  } else {
    const Akonadi::Collection rightCol = right.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( rightCol.name().compare( QLatin1String( "inbox" ), Qt::CaseInsensitive ) == 0 )
      return false;
  }
  // ... then let QSFPM sort by display data
  return QSortFilterProxyModel::lessThan( left, right );
}
