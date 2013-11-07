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

#include "threadgroupermodel.h"

#include "hierarchyresolver.h"

class ThreadGrouperModelPrivate
{
  public:
    ThreadGrouperModelPrivate( ThreadGrouperComparator *comparator, ThreadGrouperModel *qq )
      : q_ptr( qq ), m_comparator( comparator ), m_threadingEnabled( true ), m_dynamicModelRepopulation( false )
    {
      Q_ASSERT( m_comparator );

      m_comparator->m_grouper = this;
    }

    Q_DECLARE_PUBLIC( ThreadGrouperModel )
    ThreadGrouperModel* const q_ptr;

    Akonadi::Item getThreadItem( const Akonadi::Item &item ) const;

    Akonadi::Item threadRoot( const QModelIndex &index ) const;

    void populateThreadGrouperModel() const;
    void resort();

    mutable QHash<QByteArray, QByteArray> m_childParentMap; // maps an item to its thread leader item
    mutable QHash<QByteArray, QSet<QByteArray> > m_parentChildrenMap; // maps a thread leader item to all its descendant items
    mutable QHash<QByteArray, Akonadi::Item> m_items;

    ThreadGrouperComparator *m_comparator;
    bool m_threadingEnabled;
    bool m_dynamicModelRepopulation;
};

ThreadGrouperComparator::ThreadGrouperComparator()
{
}

ThreadGrouperComparator::~ThreadGrouperComparator()
{
}

QString ThreadGrouperComparator::grouperString( const Akonadi::Item& ) const
{
  return QString();
}

Akonadi::Item ThreadGrouperComparator::threadItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( m_grouper );

  return m_grouper->getThreadItem( item );
}

Akonadi::Item ThreadGrouperComparator::itemForIdentifier( const QByteArray &identifier ) const
{
  Q_ASSERT( m_grouper );

  return m_grouper->m_items.value( identifier );
}

QSet<QByteArray> ThreadGrouperComparator::threadDescendants( const QByteArray &identifier ) const
{
  Q_ASSERT( m_grouper );

  return m_grouper->m_parentChildrenMap.value( identifier );
}

void ThreadGrouperComparator::invalidate()
{
  Q_ASSERT( m_grouper );

  m_grouper->q_ptr->invalidate();
}

void ThreadGrouperComparator::resetCaches()
{
}


Akonadi::Item ThreadGrouperModelPrivate::getThreadItem( const Akonadi::Item &item ) const
{
  const QByteArray identifier = m_comparator->identifierForItem( item );
  const QByteArray parentIdentifier = m_childParentMap.value( identifier );

  /**
   * If threading is disabled, we treat each item like it is its own thread leader.
   */
  if ( !m_threadingEnabled )
    return item;

  if ( !m_items.contains( parentIdentifier ) ) {
    /**
     * The model knows nothing about the referenced parent item, this can happen
     * when importing and viewing only a part of a mail thread for example.
     * In this case we handle the item as standalone thread top node.
     */
    return m_items.value( identifier );
  }

  return m_items.value( parentIdentifier );
}

void ThreadGrouperModelPrivate::populateThreadGrouperModel() const
{
  Q_Q( const ThreadGrouperModel );
  m_childParentMap.clear();
  m_parentChildrenMap.clear();
  m_items.clear();

  if ( !q->sourceModel() )
    return;

  HierarchyResolver resolver;

  const int rowCount = q->sourceModel()->rowCount();

  for ( int row = 0; row < rowCount; ++row ) {
    const QModelIndex index = q->sourceModel()->index( row, 0 );
    Q_ASSERT( index.isValid() );

    const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    Q_ASSERT( item.isValid() );

    const QByteArray identifier = m_comparator->identifierForItem( item );

    m_items[ identifier ] = item;

    const QByteArray parentIdentifier = m_comparator->parentIdentifierForItem( item );
    if ( parentIdentifier.isEmpty() )
      resolver.addNode( identifier );
    else
      resolver.addRelation( identifier, parentIdentifier );

  }

  resolver.resolve( m_items.keys().toSet() );

  m_childParentMap = resolver.childParentMap();
  m_parentChildrenMap = resolver.parentChildrenMap();

  m_comparator->resetCaches();
}

void ThreadGrouperModelPrivate::resort()
{
  Q_Q( ThreadGrouperModel );

  q->sort( 0, q->sortOrder() );
}

ThreadGrouperModel::ThreadGrouperModel( ThreadGrouperComparator *comparator, QObject *parent )
  : QSortFilterProxyModel( parent ), d_ptr( new ThreadGrouperModelPrivate( comparator, this ) )
{
  setDynamicSortFilter( true );
  sort( 0, Qt::AscendingOrder );
}

ThreadGrouperModel::~ThreadGrouperModel()
{
  delete d_ptr;
}

Akonadi::Item ThreadGrouperModelPrivate::threadRoot( const QModelIndex &index ) const
{
  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  Q_ASSERT( item.isValid() );
  return getThreadItem( item );
}

QVariant ThreadGrouperModel::data( const QModelIndex &index, int role ) const
{
  Q_D( const ThreadGrouperModel );

  if ( !index.isValid() )
    return QVariant();

  if ( role == ThreadIdRole )
    return d->threadRoot( index ).id();
  else if ( role == GrouperRole ) {
    return d->m_comparator->grouperString( index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>() );
  }

  return QSortFilterProxyModel::data( index, role );
}

void ThreadGrouperModel::setSourceModel( QAbstractItemModel *sourceModel )
{
  Q_D( ThreadGrouperModel );
  d->populateThreadGrouperModel();

  connect( sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(populateThreadGrouperModel()) );
  connect( sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(resort()) );

  connect( sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
           this, SLOT(populateThreadGrouperModel()) );

  connect( sourceModel, SIGNAL(layoutChanged()),
           this, SLOT(populateThreadGrouperModel()) );

  QSortFilterProxyModel::setSourceModel( sourceModel );

  if ( d->m_dynamicModelRepopulation )
    connect( sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
             this, SLOT(populateThreadGrouperModel()) );

  QHash<int, QByteArray> names = roleNames();
  names.insert( GrouperRole, "grouperString" );
  setRoleNames( names );
}

bool ThreadGrouperModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  Q_D( const ThreadGrouperModel );

  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  return d->m_comparator->lessThan( leftItem, rightItem );
}

void ThreadGrouperModel::setThreadingEnabled( bool enabled )
{
  Q_D( ThreadGrouperModel );

  d->m_threadingEnabled = enabled;

  d->populateThreadGrouperModel();
  invalidate();
}

bool ThreadGrouperModel::threadingEnabled() const
{
  Q_D( const ThreadGrouperModel );

  return d->m_threadingEnabled;
}

void ThreadGrouperModel::setDynamicModelRepopulation( bool enabled )
{
  Q_D( ThreadGrouperModel );

  d->m_dynamicModelRepopulation = enabled;
}

#include "moc_threadgroupermodel.cpp"
