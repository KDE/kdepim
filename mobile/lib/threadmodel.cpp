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

#include "threadmodel.h"

#include "rangemanager_p.h"
#include "threadgroupermodel.h"

#include <Akonadi/KMime/messagestatus.h>
#include <kmime/kmime_message.h>

class ThreadModelPrivate
{
  ThreadModelPrivate( QAbstractItemModel *emailModel, ThreadModel *qq )
    : q_ptr( qq ), m_emailModel( emailModel )
  {
  }

  Q_DECLARE_PUBLIC( ThreadModel )
  ThreadModel* const q_ptr;

  void slotRowsInserted( const QModelIndex&, int, int );
  void slotRowsRemoved( const QModelIndex&, int, int );
  void slotResetModel();
  void populateThreadModel();

  QAbstractItemModel *m_emailModel;
  RangeManager m_rangeManager;
};

void ThreadModelPrivate::slotRowsInserted( const QModelIndex&, int start, int end )
{
  /**
   * At this point the m_emailModel contains the new rows already, but the range manager
   * is not updated yet.
   *
   * Example:
   *   The m_emailModel contained two threads each with two messages
   *     EM[t1 t1 t2 t2]
   *   so the range manager contains the same information
   *     RM[t1 t1 t2 t2]
   *   Now 4 new rows are inserted into the m_emailModel with one new message belonging to t1,
   *   one new message belonging to t2 and 2 new messages that are thread leaders themself.
   *   The m_emailModel will look like the following now
   *     EM[t1 t1 t1 t2 t3 t4 t4 t4]
   *   and we have to adapt the range manager to match this.
   */

  // At first find out if there exists a row before 'start' and what its thread id and associated range is
  Akonadi::Item::Id previousThreadId = -1;
  if ( start != 0 ) {
    const QModelIndex index = m_emailModel->index( start - 1, 0 );
    Q_ASSERT( index.isValid() );
    previousThreadId = index.data( ThreadGrouperModel::ThreadIdRole ).toLongLong();
  }

  // check if there exists a row after 'end' and what its thread id and associated range is
  Akonadi::Item::Id nextThreadId = -1;
  if ( end != (m_emailModel->rowCount() - 1) ) {
    const QModelIndex index = m_emailModel->index( end + 1, 0 );
    Q_ASSERT( index.isValid() );
    nextThreadId = index.data( ThreadGrouperModel::ThreadIdRole ).toLongLong();
  }

  // iterate over the new rows
  Akonadi::Item::Id currentThreadId = previousThreadId;
  int currentRange = (start == 0 ? -1 : m_rangeManager.rangeForPosition( start - 1 ));

  for ( int row = start; row <= end; ++row ) {
    const QModelIndex index = m_emailModel->index( row, 0 );
    Q_ASSERT( index.isValid() );

    const Akonadi::Item::Id threadRootId = index.data( ThreadGrouperModel::ThreadIdRole ).toLongLong();
    if ( threadRootId == currentThreadId ) { // belongs to the current thread
      m_rangeManager.increaseRange( currentRange, 1 );

      const QModelIndex threadIndex = q_ptr->index( currentRange, 0 ); //TODO: cache it
      emit q_ptr->dataChanged( threadIndex, threadIndex );
    } else { // threadRootId != currentThreadId
      if ( (row == end) && (threadRootId == nextThreadId) ) {
        const int nextRange = currentRange + 1;
        m_rangeManager.increaseRange( nextRange, 1 );

        const QModelIndex threadIndex = q_ptr->index( nextRange, 0 );
        emit q_ptr->dataChanged( threadIndex, threadIndex );
      } else {
        currentRange++;
        q_ptr->beginInsertRows( QModelIndex(), currentRange, currentRange );
        m_rangeManager.insertRange( currentRange, 1 );
        q_ptr->endInsertRows();
      }
    }

    currentThreadId = threadRootId;
  }
}

void ThreadModelPrivate::slotRowsRemoved( const QModelIndex&, int start, int end )
{
  const int startRange = m_rangeManager.rangeForPosition( start );
  const int endRange = m_rangeManager.rangeForPosition( end );

  const int startRangeSize = m_rangeManager.rangeSize( startRange );
  const int endRangeSize = m_rangeManager.rangeSize( endRange );

  if ( startRange == endRange ) {
    // the rows to be removed are covered by one range

    const int rowCount = (end - start + 1);
    if ( (rowCount - startRangeSize) == 0 ) { // all messages of this thread are removed -> remove the thread
      q_ptr->beginRemoveRows( QModelIndex(), startRange, startRange );
      m_rangeManager.removeRange( startRange );
      q_ptr->endRemoveRows();
    } else { // some messages are left in the thread -> adapt thread size
      m_rangeManager.decreaseRange( startRange, rowCount );

      const QModelIndex index = q_ptr->index( startRange, 0 );
      emit q_ptr->dataChanged( index, index ); // the number of thread children has changed -> trigger view update
    }
  } else {
    // the rows to be removed are covered by two or more ranges

    // first check how many rows of the start range are affected
    const int startRangeStart = m_rangeManager.rangeStart( startRange + 1 );
    const int affectedStartRows = (startRangeStart - start);

    // check how many rows of the end range are affected
    const int endRangeStart = m_rangeManager.rangeStart( endRange );
    const int affectedEndRows = (end - endRangeStart + 1);

    // we can't delete the ranges one by one, but have to remove them in one go,
    // so store which is the first and last range to be deleted
    int startRangeToDelete = startRange + 1;
    int endRangeToDelete = endRange - 1;

    // we can't update the indexes before the rows are removed, so delay it
    bool updateStartRange = false;
    bool updateEndRange = false;

    if ( (affectedStartRows - startRangeSize) == 0 ) { // all messages of this thread are removed -> remove the thread
      startRangeToDelete = startRange;
    } else { // some messages are left in the thread -> adapt thread size
      updateStartRange = true;
    }

    if ( (affectedEndRows - endRangeSize) == 0 ) { // all messages of this thread are removed -> remove the thread
      endRangeToDelete = endRange;
    } else { // some messages are left in the thread -> adapt thread size
      updateEndRange = true;
    }

    // check if there are ranges that must be removed
    if ( (endRangeToDelete - startRangeToDelete) >= 0 ) {
      q_ptr->beginRemoveRows( QModelIndex(), startRangeToDelete, endRangeToDelete );
      for ( int range = startRangeToDelete; range <= endRangeToDelete; ++range ) {
        m_rangeManager.removeRange( startRangeToDelete );
      }
      q_ptr->endRemoveRows();
    }

    // no update the start range and end range as well
    if ( updateStartRange ) {
      m_rangeManager.decreaseRange( startRange, affectedStartRows );

      const QModelIndex index = q_ptr->index( startRange, 0 );
      emit q_ptr->dataChanged( index, index ); // the number of thread children has changed -> trigger view update
    }

    if ( updateEndRange ) {
      // we need to update the end range here, since the ranges between start and end have been removed already
      const int updatedEndRange = (endRange - (endRangeToDelete - startRangeToDelete) - 1);
      m_rangeManager.decreaseRange( updatedEndRange, affectedEndRows );

      const QModelIndex index = q_ptr->index( updatedEndRange, 0 );
      emit q_ptr->dataChanged( index, index ); // the number of thread children has changed -> trigger view update
    }
  }
}

void ThreadModelPrivate::slotResetModel()
{
  populateThreadModel();
}

void ThreadModelPrivate::populateThreadModel()
{
  Q_Q( ThreadModel );
  q->beginResetModel();

  m_rangeManager.clear();
  const int rowCount = m_emailModel->rowCount();
  if ( rowCount == 0 ) {
    q->endResetModel();
    return;
  }

  const QModelIndex firstIndex = m_emailModel->index( 0, 0 );
  Akonadi::Item::Id currentThreadId = firstIndex.data( ThreadGrouperModel::ThreadIdRole ).toLongLong();

  int startRow = 0;
  static const int column = 0;
  for ( int row = 1; row < rowCount; ++row ) {
    const QModelIndex index = m_emailModel->index( row, column );
    Q_ASSERT( index.isValid() );

    const Akonadi::Item::Id threadRoot = index.data( ThreadGrouperModel::ThreadIdRole ).toLongLong();
    if ( threadRoot != currentThreadId ) {
      m_rangeManager.insertRange( m_rangeManager.count(), row - startRow );
      startRow = row;
    }

    currentThreadId = threadRoot;
  }

  m_rangeManager.insertRange( m_rangeManager.count(), rowCount - startRow );
  q->endResetModel();
}

ThreadModel::ThreadModel( QAbstractItemModel *emailModel, QObject *parent )
  : QAbstractListModel( parent ), d_ptr( new ThreadModelPrivate( emailModel, this ) )
{
  connect( emailModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
           this, SLOT(slotRowsInserted(QModelIndex,int,int)) );

  connect( emailModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
           this, SLOT(slotRowsRemoved(QModelIndex,int,int)) );

  connect( emailModel, SIGNAL(layoutChanged()),
           this, SLOT(slotResetModel()) );

  connect( emailModel, SIGNAL(modelReset()),
           this, SLOT(slotResetModel()) );

  QHash<int, QByteArray> roleNames = emailModel->roleNames();
  roleNames.insert( ThreadSizeRole, "threadSize" );
  roleNames.insert( ThreadUnreadCountRole, "threadUnreadCount" );
  setRoleNames( roleNames );
}

ThreadModel::~ThreadModel()
{
  delete d_ptr;
}

QVariant ThreadModel::data( const QModelIndex &index, int role ) const
{
  Q_D( const ThreadModel );

  if ( !index.isValid() )
    return QVariant();

  const int indexRow = index.row();
  if ( indexRow < d->m_rangeManager.count() ) {
    const int range = indexRow;

    const int rangeStartRow = d->m_rangeManager.rangeStart( range );
    const int rangeSize = d->m_rangeManager.rangeSize( range );
    const QModelIndex firstMailIndex = d->m_emailModel->index( rangeStartRow, 0 );
    Q_ASSERT( firstMailIndex.isValid() );

    if ( role == ThreadRangeStartRole )
      return rangeStartRow;
    if ( role == ThreadRangeEndRole )
      return (rangeStartRow + rangeSize - 1);
    if ( role == ThreadSizeRole )
      return rangeSize;
    if ( role == ThreadUnreadCountRole ) {
      int unreadCount = 0;
      for ( int row = rangeStartRow; row <= (rangeStartRow + rangeSize - 1); ++row ) {
        static const int column = 0;

        const QModelIndex index = d->m_emailModel->index( row, column );
        Q_ASSERT( index.isValid() );

        const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
        Q_ASSERT( item.isValid() );
        Q_ASSERT( item.hasPayload<KMime::Message::Ptr>() );

        const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
        Akonadi::MessageStatus status;
        status.setStatusFromFlags( item.flags() );
        if ( !status.isRead() )
          ++unreadCount;
      }

      return unreadCount;
    }

    if ( role == Qt::DisplayRole ) {
      const QString displayString = firstMailIndex.data( role ).toString();
      return QString(QLatin1Char('(') + QString::number( rangeSize ) + QLatin1Char(')') + displayString);
    }

    return firstMailIndex.data( role );
  }

  return QVariant();
}

int ThreadModel::rowCount( const QModelIndex& ) const
{
  Q_D( const ThreadModel );

  return d->m_rangeManager.count();
}

#include "moc_threadmodel.cpp"
