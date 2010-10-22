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

#include "threadgroupermodel.h"

#include <KMime/Message>
#include <akonadi/kmime/messagestatus.h>


struct ThreadModelNode {
  ThreadModelNode(const QModelIndex &top, const QModelIndex &bottom)
    : range(top, bottom)
  {

  }
  QItemSelectionRange range;
};

class ThreadModelPrivate
{
  ThreadModelPrivate(QAbstractItemModel *emailModel, ThreadModel *qq)
    : q_ptr(qq), m_emailModel(emailModel)
  {
    populateThreadModel();
  }
  Q_DECLARE_PUBLIC(ThreadModel)
  ThreadModel * const q_ptr;

  void populateThreadModel();

  QAbstractItemModel *m_emailModel;

  QVector<ThreadModelNode*> m_threads;

};

void ThreadModelPrivate::populateThreadModel()
{
  Q_Q(ThreadModel);
  q->beginResetModel();
  m_threads.clear();
  const int rowCount = m_emailModel->rowCount();
  if (rowCount == 0)
    return;
  const QModelIndex firstIdx = m_emailModel->index(0, 0);
  Akonadi::Item::Id currentThreadId = firstIdx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
  int startRow = 0;
  int row = 1;
  static const int column = 0;
  for ( ; row < rowCount; ++row) {
    const QModelIndex idx = m_emailModel->index(row, column);
    Q_ASSERT(idx.isValid());
    const Akonadi::Item::Id threadRoot = idx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
    if (threadRoot != currentThreadId)
    {
      const QModelIndex top = m_emailModel->index(startRow, column);
      const QModelIndex bottom = m_emailModel->index(row - 1, column);
      Q_ASSERT(top.isValid());
      Q_ASSERT(bottom.isValid());
      m_threads.push_back(new ThreadModelNode(top, bottom));
      startRow = row;
    }
    currentThreadId = threadRoot;
  }
  const QModelIndex idx = m_emailModel->index(row - 1, column);
  Q_ASSERT(idx.isValid());
  const Akonadi::Item::Id threadRoot = idx.data(ThreadGrouperModel::ThreadIdRole).toLongLong();
  if (threadRoot != currentThreadId)
  {
    const QModelIndex top = m_emailModel->index(startRow, column);
    const QModelIndex bottom = idx;
    Q_ASSERT(top.isValid());
    Q_ASSERT(bottom.isValid());
    m_threads.push_back(new ThreadModelNode(top, bottom));
    startRow = row;
  }
  q->endResetModel();
}

ThreadModel::ThreadModel(QAbstractItemModel *emailModel, QObject* parent)
  : QAbstractListModel(parent), d_ptr(new ThreadModelPrivate(emailModel, this))
{
  connect(emailModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(layoutChanged()),
      this, SLOT(populateThreadModel()));

  connect(emailModel, SIGNAL(modelReset()),
      this, SLOT(populateThreadModel()));

  QHash<int, QByteArray> roleNames = emailModel->roleNames();
  roleNames.insert(ThreadSizeRole, "threadSize");
  roleNames.insert(ThreadUnreadCountRole, "threadUnreadCount");
  setRoleNames(roleNames);

}

ThreadModel::~ThreadModel()
{
  qDeleteAll(d_func()->m_threads);
  delete d_ptr;
}

QVariant ThreadModel::data(const QModelIndex& index, int role) const
{
  Q_D(const ThreadModel);
  const int idx = index.row();
  if (idx < d->m_threads.size()) {
    ThreadModelNode *node = d->m_threads.at(idx);
    const QModelIndex firstMailIndex = node->range.topLeft();
    Q_ASSERT(firstMailIndex.isValid());

    if (role == ThreadRangeStartRole)
      return node->range.top();
    if (role == ThreadRangeEndRole)
      return node->range.bottom();
    if (role == ThreadSizeRole)
      return node->range.height();
    if (role == ThreadUnreadCountRole) {
      int unreadCount = 0;
      for (int row = node->range.top(); row <= node->range.bottom(); ++row) {
        static const int column = 0;
        const QModelIndex idx = node->range.model()->index(row, column);
        Q_ASSERT(idx.isValid());
        const Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
        Q_ASSERT(item.isValid());
        Q_ASSERT(item.hasPayload<KMime::Message::Ptr>());
        const KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
        Akonadi::MessageStatus status;
        status.setStatusFromFlags( item.flags() );
        if (status.isUnread())
          ++unreadCount;
      }
      return unreadCount;
    }

    if (role == Qt::DisplayRole) {
      QString displ = firstMailIndex.data(role).toString();
      return "(" + QString::number( node->range.height() ) + ")" + displ;
    }
    return firstMailIndex.data(role);
  }

  return QVariant();
}

int ThreadModel::rowCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  Q_D(const ThreadModel);
  return d->m_threads.size();
}

#include "threadmodel.moc"
