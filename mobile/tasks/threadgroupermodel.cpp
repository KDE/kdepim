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

#include "../lib/hierarchyresolver.h"
#include "settings.h"

#include <calendarsupport/utils.h>
#include <kcalcore/todo.h>

#include <map>

class ThreadGrouperModelPrivate
{
  public:
    ThreadGrouperModelPrivate( ThreadGrouperModel *qq )
      : q_ptr( qq )
    {
    }

    Q_DECLARE_PUBLIC( ThreadGrouperModel )
    ThreadGrouperModel* const q_ptr;

    Akonadi::Item getThreadItem( const Akonadi::Item &item ) const;

    Akonadi::Item threadRoot( const QModelIndex &index ) const;

    void populateThreadGrouperModel() const;

    mutable QHash<QByteArray, QByteArray> m_threads;
    mutable QHash<QByteArray, Akonadi::Item> m_items;
};

struct ItemLessThanComparator
{
  ThreadGrouperModelPrivate const *m_grouper;
  ItemLessThanComparator( const ThreadGrouperModelPrivate *grouper );
  ItemLessThanComparator( const ItemLessThanComparator &other );
  ItemLessThanComparator &operator=( const ItemLessThanComparator &other );

  bool operator()( const Akonadi::Item &left, const Akonadi::Item &right ) const;
};

ItemLessThanComparator::ItemLessThanComparator( const ThreadGrouperModelPrivate *grouper )
  : m_grouper( grouper )
{
}

ItemLessThanComparator::ItemLessThanComparator( const ItemLessThanComparator &other )
{
  m_grouper = other.m_grouper;
}

ItemLessThanComparator &ItemLessThanComparator::operator=( const ItemLessThanComparator &other )
{
  m_grouper = other.m_grouper;
  return *this;
}

bool ItemLessThanComparator::operator()( const Akonadi::Item &leftItem, const Akonadi::Item &rightItem ) const
{
  Q_ASSERT( leftItem.isValid() );
  Q_ASSERT( rightItem.isValid() );

  const Akonadi::Item leftThreadRootItem = m_grouper->getThreadItem( leftItem );
  const Akonadi::Item rightThreadRootItem = m_grouper->getThreadItem( rightItem );

  Q_ASSERT( rightThreadRootItem.isValid() );
  Q_ASSERT( leftThreadRootItem.isValid() );

  if ( leftThreadRootItem != rightThreadRootItem ) {
    const KCalCore::Todo::Ptr leftTodo = CalendarSupport::todo( leftThreadRootItem );
    const KCalCore::Todo::Ptr rightTodo = CalendarSupport::todo( rightThreadRootItem );

    if ( !leftTodo || !rightTodo ) {
      kDebug() << "This shouldn't happen, but i didn't check. Better safe than sorry.";
      return false;
    }

    const bool leftCompleted = leftTodo->isCompleted();
    const bool rightCompleted = rightTodo->isCompleted();
    const int leftPriority = leftTodo->priority();
    const int rightPriority = rightTodo->priority();

    if ( Settings::self()->showCompletedTodosAtBottom() && leftCompleted != rightCompleted ) {
      return rightCompleted;
    }

    if ( leftPriority != rightPriority ) {
      // higher priority first. ( Also note that 9 is low, and 1 is high )
      return leftPriority < rightPriority;
    } else {
      // lower id first
      return leftItem.id() < rightItem.id();
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();
  }

  if ( leftThreadRootItem == leftItem )
    return true;

  if ( rightThreadRootItem == rightItem )
    return false;

  return leftItem.id() < rightItem.id();
}

static QByteArray identifierForTask( const KCalCore::Todo::Ptr &todo, Akonadi::Item::Id id )
{
  QByteArray identifier = todo->uid().toLatin1();

  if ( identifier.isEmpty() )
    identifier = QByteArray::number( id );

  return identifier;
}

Akonadi::Item ThreadGrouperModelPrivate::getThreadItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KCalCore::Todo::Ptr>() );
  const KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
  const QByteArray identifier = identifierForTask( todo, item.id() );

  const QByteArray parentIdentifier = m_threads.value( identifier );

  Q_ASSERT( m_items.contains( parentIdentifier ) );
  return m_items.value( parentIdentifier );
}

void ThreadGrouperModelPrivate::populateThreadGrouperModel() const
{
  Q_Q( const ThreadGrouperModel );
  m_threads.clear();
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
    Q_ASSERT( item.hasPayload<KCalCore::Todo::Ptr>());

    const KCalCore::Todo::Ptr todo = item.payload<KCalCore::Todo::Ptr>();
    const QByteArray identifier = identifierForTask( todo, item.id() );

    m_items.insert( identifier, item );

    if ( !todo->relatedTo( KCalCore::Todo::RelTypeParent ).isEmpty() ) {
      const QByteArray parentIdentifier = todo->relatedTo( KCalCore::Todo::RelTypeParent ).toLatin1();

      resolver.addRelation( identifier, parentIdentifier );
    } else {
      resolver.addNode( identifier );
    }
  }

  m_threads = resolver.resolve();
}

ThreadGrouperModel::ThreadGrouperModel( QObject *parent )
  : QSortFilterProxyModel( parent ), d_ptr( new ThreadGrouperModelPrivate( this ) )
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

  if ( role == ThreadIdRole ) {
    return d->threadRoot( index ).id();
  }

  return QSortFilterProxyModel::data( index, role );
}

void ThreadGrouperModel::setSourceModel( QAbstractItemModel *sourceModel )
{
  Q_D( ThreadGrouperModel );
  d->populateThreadGrouperModel();
  QSortFilterProxyModel::setSourceModel( sourceModel );

  disconnect( sourceModel, SIGNAL( rowsAboutToBeInserted( QModelIndex, int, int ) ),
              this, SIGNAL( _q_sourceRowsAboutToBeInserted( QModelIndex, int, int ) ) );

  disconnect( sourceModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ),
              this, SLOT( _q_sourceRowsInserted( QModelIndex, int, int ) ) );

  disconnect( sourceModel, SIGNAL( rowsAboutToBeRemoved( QModelIndex, int, int ) ),
              this, SLOT( _q_sourceRowsAboutToBeRemoved( QModelIndex, int, int ) ) );

  disconnect( sourceModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),
              this, SLOT( _q_sourceRowsRemoved( QModelIndex, int, int ) ) );

  disconnect( sourceModel, SIGNAL( layoutAboutToBeChanged() ),
              this, SLOT( _q_sourceLayoutAboutToBeChanged() ) );

  disconnect( sourceModel, SIGNAL( layoutChanged() ),
              this, SLOT( _q_sourceLayoutChanged() ) );

  connect( sourceModel, SIGNAL( rowsAboutToBeInserted( QModelIndex, int, int ) ),
           this, SLOT( _q_sourceAboutToBeReset() ) );

  connect( sourceModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ),
           this, SLOT( populateThreadGrouperModel() ) );

  connect( sourceModel, SIGNAL( rowsInserted( QModelIndex, int, int ) ),
           this, SLOT( _q_sourceReset() ) );

  connect( sourceModel, SIGNAL( rowsAboutToBeRemoved( QModelIndex, int, int ) ),
           this, SLOT( _q_sourceAboutToBeReset() ) );

  connect( sourceModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),
           this, SLOT( populateThreadGrouperModel() ) );

  connect( sourceModel, SIGNAL( rowsRemoved( QModelIndex, int, int ) ),
           this, SLOT( _q_sourceReset() ) );

  connect( sourceModel, SIGNAL( layoutAboutToBeChanged() ),
           this, SLOT( _q_sourceAboutToBeReset() ) );

  connect( sourceModel, SIGNAL( layoutChanged() ),
           this, SLOT( populateThreadGrouperModel() ) );

  connect( sourceModel, SIGNAL( layoutChanged() ),
           this, SLOT( _q_sourceReset() ) );
}

bool ThreadGrouperModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  Q_D( const ThreadGrouperModel );
  static ItemLessThanComparator lt( d );
  const Akonadi::Item leftItem = left.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  const Akonadi::Item rightItem = right.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  return lt( leftItem, rightItem );
}

#include "threadgroupermodel.moc"
