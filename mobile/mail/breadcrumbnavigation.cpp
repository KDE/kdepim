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

#include "breadcrumbnavigation.h"

#include <QStringList>
#include "kdebug.h"

KBreadcrumbNavigationProxyModel::KBreadcrumbNavigationProxyModel(QItemSelectionModel* selectionModel, QObject* parent)
  : KSelectionProxyModel(selectionModel, parent)
{

}

QVariant KBreadcrumbNavigationProxyModel::data(const QModelIndex& index, int role) const
{
  if (rowCount() > 2 && index.row() == 0 && role == Qt::DisplayRole)
  {
    QModelIndex sourceIndex = mapToSource(index);
    QStringList dataList;
    while (sourceIndex.isValid())
    {
      dataList.prepend(sourceIndex.data().toString());
      sourceIndex = sourceIndex.parent();
    }
    return dataList.join(" > ");
  }
  return KSelectionProxyModel::data(index, role);
}

void KBreadcrumbNavigationProxyModel::setShowHiddenAscendantData(bool showHiddenAscendantData)
{
  m_showHiddenAscendantData = showHiddenAscendantData;
}

bool KBreadcrumbNavigationProxyModel::showHiddenAscendantData() const
{
  return m_showHiddenAscendantData;
}

KNavigatingProxyModel::KNavigatingProxyModel(QItemSelectionModel* selectionModel, QObject* parent)
  : KSelectionProxyModel(selectionModel, parent), m_selectionModel(selectionModel)
{

}

void KNavigatingProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  connect( m_selectionModel, SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
      SLOT( navigationSelectionChanged( const QItemSelection &, const QItemSelection & ) ) );

  KSelectionProxyModel::setSourceModel(sourceModel);
  updateNavigation();

  connect(sourceModel, SIGNAL(modelReset()), SLOT(updateNavigation()));
  connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(updateNavigation()));
  connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(updateNavigation()));


}

void KNavigatingProxyModel::navigationSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  updateNavigation();
}

void KNavigatingProxyModel::updateNavigation()
{
  kDebug() << sourceModel();
  beginResetModel();
  if (!sourceModel())
  {
    endResetModel();
    return;
  }

  if (m_selectionModel->selection().isEmpty())
  {
    setFilterBehavior(KSelectionProxyModel::ExactSelection);
    QModelIndex top = sourceModel()->index(0, 0);
    QModelIndex bottom = sourceModel()->index(sourceModel()->rowCount() - 1, 0);

    disconnect( m_selectionModel, SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
        this, SLOT( navigationSelectionChanged( const QItemSelection &, const QItemSelection & ) ) );
    m_selectionModel->select(QItemSelection(top, bottom), QItemSelectionModel::Select);
    connect( m_selectionModel, SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection & ) ),
        SLOT( navigationSelectionChanged( const QItemSelection &, const QItemSelection & ) ) );
  } else if (filterBehavior() != KSelectionProxyModel::ChildrenOfExactSelection) {
    setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  }
    endResetModel();
}

QVariant KNavigatingProxyModel::data(const QModelIndex& index, int role) const
{
  if ( role == Qt::DisplayRole && sourceModel()->hasChildren(mapToSource(index)))
  {
    return "+ " + KSelectionProxyModel::data(index, role).toString();
  }
  return KSelectionProxyModel::data(index, role);
}


KForwardingItemSelectionModel::KForwardingItemSelectionModel(QItemSelectionModel* selectionModel, QAbstractItemModel* model, QObject *parent)
  : QItemSelectionModel(model, parent), m_selectionModel(selectionModel)
{
  Q_ASSERT(model == selectionModel->model());
  connect(selectionModel, SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
          SLOT(navigationSelectionChanged(const QItemSelection&,const QItemSelection&)));
}

void KForwardingItemSelectionModel::navigationSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  select(selected, ClearAndSelect);
}
