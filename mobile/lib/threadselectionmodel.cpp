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

#include "threadselectionmodel.h"

#include "threadmodel.h"

class ThreadSelectionModelPrivate
{
  ThreadSelectionModelPrivate(ThreadSelectionModel *qq, QItemSelectionModel *contentSelectionModel, QItemSelectionModel* navigationModel)
    : q_ptr(qq), m_contentSelectionModel(contentSelectionModel), m_navigationModel(navigationModel)
  {

  }
  Q_DECLARE_PUBLIC(ThreadSelectionModel)
  ThreadSelectionModel * const q_ptr;

  void contentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  QItemSelectionModel * const m_contentSelectionModel;
  QItemSelectionModel * const m_navigationModel;

};

ThreadSelectionModel::ThreadSelectionModel(QAbstractItemModel* model, QItemSelectionModel* contentSelectionModel, QItemSelectionModel* navigationModel, QObject *parent)
  : QItemSelectionModel(model, parent),
    d_ptr(new ThreadSelectionModelPrivate(this, contentSelectionModel, navigationModel))
{
  connect(contentSelectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          SLOT(contentSelectionChanged(QItemSelection,QItemSelection)));
}

ThreadSelectionModel::~ThreadSelectionModel()
{
  delete d_ptr;
}

void ThreadSelectionModel::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
{
  select(QItemSelection(index, index), command);
}

void ThreadSelectionModel::select(const QItemSelection& selection, QItemSelectionModel::SelectionFlags command)
{
  Q_D(ThreadSelectionModel);
  QItemSelectionModel::select(selection, command);
  QItemSelection thread;
  foreach(const QItemSelectionRange &range, selection) {
    for (int row = range.top(); row <= range.bottom(); ++row) {
      static const int column = 0;
      const QModelIndex idx = model()->index(row, column);
      const int threadStartRow = idx.data(ThreadModel::ThreadRangeStartRole).toInt();
      const int threadEndRow = idx.data(ThreadModel::ThreadRangeEndRole).toInt();
      const QModelIndex threadStart = d->m_contentSelectionModel->model()->index(threadStartRow, column);
      const QModelIndex threadEnd = d->m_contentSelectionModel->model()->index(threadEndRow, column);
      Q_ASSERT(threadStart.isValid());
      Q_ASSERT(threadEnd.isValid());
      thread.select(threadStart, threadEnd);
    }
  }
  d->m_contentSelectionModel->select(thread, ClearAndSelect);
}

void ThreadSelectionModelPrivate::contentSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  Q_UNUSED(selected)
  Q_UNUSED(deselected)

  const QModelIndexList list = m_contentSelectionModel->selectedRows();
  if (list.isEmpty())
    m_navigationModel->clearSelection();
  if (list.size() == 1) {
    m_navigationModel->select(list.first(), QItemSelectionModel::ClearAndSelect);
  }
}

#include "moc_threadselectionmodel.cpp"
