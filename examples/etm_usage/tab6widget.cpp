/*
    This file is part of Akonadi.

    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "tab6widget.h"

#include <QHBoxLayout>
#include <QTreeView>

#include "entitytreewidget.h"
#include "itemviewerwidget.h"

#include <kselectionproxymodel.h>
#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>

class NoncheckableFilterModel : public Akonadi::EntityMimeTypeFilterModel
{
public:
  NoncheckableFilterModel(QObject* parent = 0)
    : Akonadi::EntityMimeTypeFilterModel(parent)
  {

  }

  /* reimp */ Qt::ItemFlags flags(const QModelIndex& index) const
  {
    return Akonadi::EntityMimeTypeFilterModel::flags(index) & (~Qt::ItemIsUserCheckable);
  }

  /* reimp */ QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
  {
    if ( role == Qt::CheckStateRole )
      return QVariant();
    return Akonadi::EntityMimeTypeFilterModel::data(index, role);
  }

};

class CheckedSelectionEntityModel : public Akonadi::EntityTreeModel
{
public:
  CheckedSelectionEntityModel(Akonadi::ChangeRecorder* monitor, QObject* parent = 0)
    : Akonadi::EntityTreeModel(monitor, parent), m_itemSelectionModel(0)
  {
  }

  void setSelectionModel(QItemSelectionModel *itemSelectionModel)
  {
    m_itemSelectionModel = itemSelectionModel;
  }

  /* reimp */ Qt::ItemFlags flags(const QModelIndex& index) const
  {
    return EntityTreeModel::flags(index) | Qt::ItemIsUserCheckable;
  }

  /* reimp */ QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
  {
    if (role == Qt::CheckStateRole)
    {
      if (!m_itemSelectionModel)
        return Qt::Unchecked;
      return m_itemSelectionModel->selectedRows().contains(index) ? Qt::Checked : Qt::Unchecked;
    }
    return Akonadi::EntityTreeModel::data(index, role);
  }

  /* reimp */ bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)
  {
    if (role == Qt::CheckStateRole)
    {
      if (!m_itemSelectionModel)
        return false;
      m_itemSelectionModel->select(index, QItemSelectionModel::SelectCurrent);
      return true;
    }
    return Akonadi::EntityTreeModel::setData(index, value, role);
  }
private:
  QItemSelectionModel *m_itemSelectionModel;
};

class Tab6TreeWidget : public EntityTreeWidget
{
public:
  Tab6TreeWidget(QWidget* parent = 0)
    : EntityTreeWidget(parent)
  {
  }

  /* reimp */ void connectTreeToModel(QTreeView* tree, Akonadi::EntityTreeModel* model)
  {
    m_collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);
    m_collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    m_collectionFilter->setSourceModel(model);
    m_collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);
    tree->setModel(m_collectionFilter);
  }

  /* reimp */ QModelIndex mapToSource(const QModelIndex &idx)
  {
    return m_collectionFilter->mapToSource(idx);
  }

  virtual Akonadi::EntityTreeModel* getETM()
  {
    CheckedSelectionEntityModel* model = new CheckedSelectionEntityModel(changeRecorder(), this);
    m_itemSelectionModel = new QItemSelectionModel(model);
    return model;
  }

  QItemSelectionModel* itemSelectionModel() const { return m_itemSelectionModel; }


private:
  Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;
  QItemSelectionModel *m_itemSelectionModel;

};

Tab6Widget::Tab6Widget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);

  Tab6TreeWidget *etw = new Tab6TreeWidget(this);
  m_etw = etw;
  m_etw->init();

  layout->addWidget(m_etw);
  QWidget *rhsContainer = new QWidget(this);
  QVBoxLayout *rhsLayout = new QVBoxLayout(rhsContainer);

  m_itemView = new QTreeView(this);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(etw->itemSelectionModel(), this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selectionProxy->setSourceModel(m_etw->model());

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new NoncheckableFilterModel(this);
  itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
  itemFilter->setSourceModel(selectionProxy);

  m_itemView->setModel(itemFilter);

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), this);
  rhsLayout->addWidget(m_itemView);
  rhsLayout->addWidget(viewerWidget);
  layout->addWidget(rhsContainer);
}


