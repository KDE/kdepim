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

#include "tab7widget.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QTreeView>

#include "entitytreewidget.h"
#include "entitytreemodelfactory.h"
#include "itemviewerwidget.h"
#include "mixedtreemodel.h"
#include "categorizedentitymodel.h"

#include <kselectionproxymodel.h>
#include <kcategorizedsortfilterproxymodel.h>
#include <kcategorizedview.h>
#include <kcategorydrawer.h>

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>

class CategorisedEntityModelFactory : public EntityTreeModelFactory
{
public:
  CategorisedEntityModelFactory(QObject* parent = 0)
    : EntityTreeModelFactory(parent)
  {

  }

  virtual Akonadi::EntityTreeModel* getModel(Akonadi::ChangeRecorder* changeRecorder, QObject* parent)
  {
    return new CategorisedEntityModel(changeRecorder, parent);
  }

};

class Tab7TreeWidget : public EntityTreeWidget
{
public:
  Tab7TreeWidget(QWidget* parent = 0)
    : EntityTreeWidget(parent), m_model(0)
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
    return m_model;
  }

  void setModel(Akonadi::EntityTreeModel *model)
  {
    m_model = model;
  }


private:
  Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;
  Akonadi::EntityTreeModel *m_model;

};

Tab7Widget::Tab7Widget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  EntityTreeModelFactory *modelFactory = new CategorisedEntityModelFactory(this);
  modelFactory->createFromRemoteId(QLatin1String("nepomuktags"));
  connect(modelFactory, &CategorisedEntityModelFactory::modelCreated, this, &Tab7Widget::initModel);
}

void Tab7Widget::initModel(Akonadi::EntityTreeModel *model)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);

  Tab7TreeWidget *etw = new Tab7TreeWidget(splitter);
  etw->setModel(model);
  m_etw = etw;
  m_etw->init();

  QSplitter *rhsContainer = new QSplitter(Qt::Vertical, splitter);

  m_itemView = new KCategorizedView(rhsContainer);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(m_etw->view()->selectionModel(), this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selectionProxy->setSourceModel(m_etw->model());

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
  itemFilter->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
  itemFilter->setSourceModel(selectionProxy);

  KCategorizedSortFilterProxyModel *categorizedModel = new KCategorizedSortFilterProxyModel(this);
  categorizedModel->setSourceModel(itemFilter);
  categorizedModel->setCategorizedModel(true);

  m_itemView->setModel(categorizedModel);
  m_itemView->setCategoryDrawer(new KCategoryDrawer(m_itemView));

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), rhsContainer);
}


