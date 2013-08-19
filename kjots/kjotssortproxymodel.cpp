/*
    This file is part of KJots.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

#include "kjotssortproxymodel.h"
#include <Akonadi/EntityTreeModel>
#include <KMime/KMimeMessage>


KJotsSortProxyModel::KJotsSortProxyModel( QObject* parent )
  : QSortFilterProxyModel( parent )
{
  setDynamicSortFilter(true);
}

KJotsSortProxyModel::~KJotsSortProxyModel()
{

}

bool KJotsSortProxyModel::lessThan( const QModelIndex& left, const QModelIndex& right ) const
{
  const Akonadi::Collection::Id colId = left.data( Akonadi::EntityTreeModel::ParentCollectionRole ).value<Akonadi::Collection>().id();

  if ( colId < 0 || m_alphaSorted.contains( colId ) || !m_dateTimeSorted.contains( colId ) )
    return QSortFilterProxyModel::lessThan( left, right );

  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( !leftItem.isValid() || !rightItem.isValid() )
    return true;

  const KMime::Message::Ptr leftNote = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightNote = rightItem.payload<KMime::Message::Ptr>();

  return leftNote->date()->dateTime() < rightNote->date()->dateTime();
}

Akonadi::Collection::Id KJotsSortProxyModel::collectionId( const QModelIndex& parent ) const
{
  const QModelIndex childIndex = index( 0, 0, parent );
  if ( !childIndex.isValid() )
    return -1;

  const Akonadi::Collection collection = childIndex.data( Akonadi::EntityTreeModel::ParentCollectionRole ).value<Akonadi::Collection>();

  if ( !collection.isValid() )
    return -1;

  return collection.id();
}

void KJotsSortProxyModel::sortChildrenAlphabetically( const QModelIndex& parent )
{
  const Akonadi::Collection::Id id = collectionId( parent );
  if ( id < 0 )
    return;

  m_dateTimeSorted.remove( id );
  m_alphaSorted.insert( id );
  invalidate();
}

void KJotsSortProxyModel::sortChildrenByCreationTime( const QModelIndex& parent )
{
  const Akonadi::Collection::Id id = collectionId( parent );
  if ( id < 0 )
    return;

  m_alphaSorted.remove( id );
  m_dateTimeSorted.insert( id );
  invalidate();
}

#include "kjotssortproxymodel.moc"
