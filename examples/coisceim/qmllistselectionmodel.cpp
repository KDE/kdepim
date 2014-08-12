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

#include "qmllistselectionmodel.h"

#include <QDebug>
#include <AkonadiCore/EntityTreeModel>

QMLListSelectionModel::QMLListSelectionModel(QItemSelectionModel *selectionModel, QObject* parent)
  : QObject(parent), m_selectionModel(selectionModel)
{
  connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(selectionChanged()));
}

QMLListSelectionModel::QMLListSelectionModel(QAbstractItemModel* model, QObject* parent)
  : QObject(parent), m_selectionModel(new QItemSelectionModel(model, this))
{
  connect(m_selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(selectionChanged()));
}

QItemSelectionModel* QMLListSelectionModel::selectionModel() const
{
  return m_selectionModel;
}

QList< int > QMLListSelectionModel::selection() const
{
  QList< int > list;
  const QModelIndexList indexes = m_selectionModel->selectedRows();
  foreach (const QModelIndex &index, indexes)
    list << index.row();
  return list;
}

bool QMLListSelectionModel::hasSelection() const
{
    return m_selectionModel->hasSelection();
}

int QMLListSelectionModel::currentRow() const
{
  const QModelIndexList indexes = m_selectionModel->selectedRows();
  if (indexes.size() != 1)
    return -1;
  Q_ASSERT(indexes.first().isValid());
  return indexes.first().row();
}

void QMLListSelectionModel::setCurrentRow(int row)
{
  select(row, QItemSelectionModel::ClearAndSelect);
}

qint64 QMLListSelectionModel::currentItemId() const
{
  const QModelIndexList indexes = m_selectionModel->selectedRows();
  if (indexes.size() != 1)
    return -1;
  Q_ASSERT(indexes.first().isValid());
  return indexes.first().data(Akonadi::EntityTreeModel::ItemIdRole).toLongLong();
}

void QMLListSelectionModel::setCurrentItemId(qint64 itemId)
{
  const QModelIndexList list = Akonadi::EntityTreeModel::modelIndexesForItem(m_selectionModel->model(), Akonadi::Item(itemId));
  if (list.size() == 1) {
    const QModelIndex idx = list.first();
    m_selectionModel->select(QItemSelection(idx, idx), QItemSelectionModel::ClearAndSelect);
  }
}

void QMLListSelectionModel::select(int row, int command)
{
  if (row < 0) {
    clearSelection();
    return;
  }
  static const int column = 0;
  const QModelIndex idx = m_selectionModel->model()->index(row, column);
  Q_ASSERT(idx.isValid());
  QItemSelection sel(idx, idx);
  QItemSelectionModel::SelectionFlags flags = static_cast<QItemSelectionModel::SelectionFlags>(command);
  m_selectionModel->select(sel, flags);
}

bool QMLListSelectionModel::requestNext()
{
  const QModelIndexList list = m_selectionModel->selectedRows();
  if (list.isEmpty() || list.size() != 1)
    return false;

  const QModelIndex idx = list.first();
  Q_ASSERT(idx.isValid());
  const QModelIndex next = idx.sibling(idx.row() + 1, idx.column());
  if (!next.isValid())
    return false;

  m_selectionModel->select(QItemSelection(next, next), QItemSelectionModel::ClearAndSelect);
  return true;
}

bool QMLListSelectionModel::requestPrevious()
{
  const QModelIndexList list = m_selectionModel->selectedRows();
  if (list.isEmpty() || list.size() != 1)
    return false;

  const QModelIndex idx = list.first();
  Q_ASSERT(idx.isValid());
  if (idx.row() == 0)
    return false;

  const QModelIndex previous = idx.sibling(idx.row() - 1, idx.column());
  m_selectionModel->select(QItemSelection(previous, previous), QItemSelectionModel::ClearAndSelect);
  return true;
}

void QMLListSelectionModel::clearSelection()
{
  // Don't call QItemSelectionModel::clearSelection. It is non-virtual so
  // item selection models in chains can't react to it properly.
  m_selectionModel->select(QItemSelection(), QItemSelectionModel::Clear);
}

