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

#include <KMime/Message>

#include <map>

struct ItemLessThanComparator
{
  ThreadGrouperModelPrivate const *m_grouper;
  ItemLessThanComparator( const ThreadGrouperModelPrivate *grouper );
  ItemLessThanComparator( const ItemLessThanComparator &other );
  ItemLessThanComparator &operator=( const ItemLessThanComparator &other );

  bool operator()( const Akonadi::Item &left, const Akonadi::Item &right ) const;
};

class ThreadGrouperModelPrivate
{
  public:
    ThreadGrouperModelPrivate( ThreadGrouperModel *qq )
      : q_ptr( qq ), m_sortingOption( ThreadGrouperModel::SortByDateTimeMostRecent )
    {
    }

    Q_DECLARE_PUBLIC( ThreadGrouperModel )
    ThreadGrouperModel* const q_ptr;

    Akonadi::Item getThreadItem( const Akonadi::Item &item ) const;

    Akonadi::Item threadRoot( const QModelIndex &index ) const;

    KDateTime getMostRecentUpdate( KMime::Message::Ptr threadRoot, Akonadi::Entity::Id itemId ) const;

    void populateThreadGrouperModel() const;

    mutable QHash<QByteArray, QByteArray> m_childParentMap; // maps an item to its thread leader item
    mutable QHash<QByteArray, QSet<QByteArray> > m_parentChildrenMap; // maps a thread leader item to all its descendant items
    mutable QHash<QByteArray, Akonadi::Item> m_items;

    ThreadGrouperModel::SortingOption m_sortingOption;
};

static QByteArray identifierForMessage( const KMime::Message::Ptr &message, Akonadi::Item::Id id )
{
  QByteArray identifier = message->messageID()->identifier();
  if ( identifier.isEmpty() )
    identifier = QByteArray::number( id );

  return identifier;
}

Akonadi::Item ThreadGrouperModelPrivate::getThreadItem( const Akonadi::Item &item ) const
{
  Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );
  const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
  const QByteArray identifier = identifierForMessage( message, item.id() );

  const QByteArray parentIdentifier = m_childParentMap.value( identifier );

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

KDateTime ThreadGrouperModelPrivate::getMostRecentUpdate( KMime::Message::Ptr threadRoot, Akonadi::Item::Id itemId ) const
{
  const QSet<QByteArray> messageIds = m_parentChildrenMap.value( identifierForMessage( threadRoot, itemId ) );

  KDateTime newest = threadRoot->date()->dateTime();

  if ( messageIds.isEmpty() )
    return newest;

  foreach ( const QByteArray &messageId, messageIds ) {
    const Akonadi::Item item = m_items.value( messageId );
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    const KDateTime messageDateTime = message->date()->dateTime();
    if ( messageDateTime > newest )
      newest = messageDateTime;
  }

  return newest;
}

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
  if ( this != &other )
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
    Q_ASSERT( leftThreadRootItem.hasPayload<KMime::Message::Ptr>() );
    Q_ASSERT( rightThreadRootItem.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr leftThreadRootMessage = leftThreadRootItem.payload<KMime::Message::Ptr>();
    const KMime::Message::Ptr rightThreadRootMessage = rightThreadRootItem.payload<KMime::Message::Ptr>();

    switch ( m_grouper->m_sortingOption ) {
      case ThreadGrouperModel::SortByDateTime:
        {
          const KDateTime leftThreadRootDateTime = leftThreadRootMessage->date()->dateTime();
          const KDateTime rightThreadRootDateTime = rightThreadRootMessage->date()->dateTime();
          if ( leftThreadRootDateTime != rightThreadRootDateTime ) {
            return leftThreadRootDateTime > rightThreadRootDateTime;
          }
        }
        break;
      case ThreadGrouperModel::SortByDateTimeMostRecent:
        {
          const KDateTime leftNewest = m_grouper->getMostRecentUpdate( leftThreadRootMessage, leftThreadRootItem.id() );
          const KDateTime rightNewest = m_grouper->getMostRecentUpdate( rightThreadRootMessage, rightThreadRootItem.id() );

          if ( leftNewest != rightNewest ) {
            return leftNewest > rightNewest;
          }
        }
        break;
      case ThreadGrouperModel::SortBySenderReceiver:
        {
          const QString leftSender = leftThreadRootMessage->sender()->asUnicodeString();
          const QString rightSender = rightThreadRootMessage->sender()->asUnicodeString();

          if ( leftSender != rightSender )
            return (leftSender.localeAwareCompare( rightSender ) < 0);
        }
        break;
      case ThreadGrouperModel::SortBySubject:
        {
          const QString leftSubject = leftThreadRootMessage->subject()->asUnicodeString();
          const QString rightSubject = rightThreadRootMessage->subject()->asUnicodeString();

          if ( leftSubject != rightSubject )
            return (leftSubject.localeAwareCompare( rightSubject ) < 0);
        }
        break;
      case ThreadGrouperModel::SortBySize:
        {
          const qint64 leftSize = leftThreadRootItem.size();
          const qint64 rightSize = rightThreadRootItem.size();

          if ( leftSize != rightSize )
            return leftSize < rightSize;
        }
        break;
    }

    return leftThreadRootItem.id() < rightThreadRootItem.id();
  }

  if ( leftThreadRootItem == leftItem )
    return true;

  if ( rightThreadRootItem == rightItem )
    return false;

  Q_ASSERT( leftItem.hasPayload<KMime::Message::Ptr>() );
  Q_ASSERT( rightItem.hasPayload<KMime::Message::Ptr>() );

  const KMime::Message::Ptr leftMessage = leftItem.payload<KMime::Message::Ptr>();
  const KMime::Message::Ptr rightMessage = rightItem.payload<KMime::Message::Ptr>();

  const KDateTime leftDateTime = leftMessage->date()->dateTime();
  const KDateTime rightDateTime = rightMessage->date()->dateTime();

  // Messages in the same thread are ordered most recent last.
  if ( leftDateTime != rightDateTime ) {
    return leftDateTime < rightDateTime;
  }

  return leftItem.id() < rightItem.id();
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
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    const QByteArray identifier = identifierForMessage( message, item.id() );

    m_items[ identifier ] = item;

    if ( !message->inReplyTo()->isEmpty() ) {
      const QByteArray inReplyTo = message->inReplyTo()->as7BitString( false );
      const QByteArray parentIdentifier = inReplyTo.mid( 1, inReplyTo.size() -2 ); // strip '<' and '>'

      resolver.addRelation( identifier, parentIdentifier );
    } else {
      resolver.addNode( identifier );
    }
  }

  resolver.resolve( m_items.keys().toSet() );

  m_childParentMap = resolver.childParentMap();
  m_parentChildrenMap = resolver.parentChildrenMap();
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

void ThreadGrouperModel::setSortingOption( ThreadGrouperModel::SortingOption option )
{
  Q_D( ThreadGrouperModel );
  d->m_sortingOption = option;

  invalidate();
}

ThreadGrouperModel::SortingOption ThreadGrouperModel::sortingOption() const
{
  Q_D( const ThreadGrouperModel );
  return d->m_sortingOption;
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

  if ( role == Qt::DisplayRole ) {
    const Akonadi::Item item = QSortFilterProxyModel::data( index, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    Q_ASSERT( item.isValid() );
    Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

    const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
    return  QSortFilterProxyModel::data( index, role ).toString() + message->subject()->asUnicodeString() + " - " + message->date()->asUnicodeString();
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
