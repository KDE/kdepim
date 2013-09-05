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

// READ THE README FILE

#include "tab6widget.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QTreeView>

#include "entitytreewidget.h"
#include "itemviewerwidget.h"
#include "mixedtreemodel.h"

#include <kselectionproxymodel.h>
#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>
#include <kcheckableproxymodel.h>

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>

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

    m_checkableProxy = new KCheckableProxyModel(this);
    m_checkableProxy->setSourceModel(m_collectionFilter);

    m_itemSelectionModel = new QItemSelectionModel(m_collectionFilter);
    m_checkableProxy->setSelectionModel(m_itemSelectionModel);

    tree->setModel(m_checkableProxy);
  }

  /* reimp */ QModelIndex mapToSource(const QModelIndex &idx)
  {
    return m_collectionFilter->mapToSource(idx);
  }

  virtual Akonadi::EntityTreeModel* getETM()
  {
    MixedTreeModel* model = new MixedTreeModel(changeRecorder(), this);
    return model;
  }

  QItemSelectionModel* itemSelectionModel() const { return m_itemSelectionModel; }


private:
  Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;
  KCheckableProxyModel *m_checkableProxy;
  QItemSelectionModel *m_itemSelectionModel;

};

Tab6Widget::Tab6Widget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);

  Tab6TreeWidget *etw = new Tab6TreeWidget(splitter);
  m_etw = etw;
  m_etw->init();

  QSplitter *rhsContainer = new QSplitter(Qt::Vertical, splitter);

  m_itemView = new QTreeView(rhsContainer);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(etw->itemSelectionModel(), this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selectionProxy->setSourceModel(m_etw->model());

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  itemFilter->setObjectName(QLatin1String("itemFilter"));
  itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
  itemFilter->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
  itemFilter->setSourceModel(selectionProxy);

  m_itemView->setModel(itemFilter);

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), rhsContainer);
}


