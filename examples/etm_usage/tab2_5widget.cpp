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

#include "tab2_5widget.h"

#include <QHBoxLayout>
#include <QTreeView>

#include "entitytreewidget.h"
#include "itemviewerwidget.h"
#include "mixedtreemodel.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/entitymimetypefiltermodel.h>

class Tab2_5TreeWidget : public EntityTreeWidget
{
public:
  Tab2_5TreeWidget(QWidget* parent = 0)
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

  /* reimp */ Akonadi::EntityTreeModel* getETM()
  {
    return new MixedTreeModel(changeRecorder(), this);
  }

private:
  Akonadi::EntityMimeTypeFilterModel *m_collectionFilter;

};

Tab2_5Widget::Tab2_5Widget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);

  m_etw = new Tab2_5TreeWidget(this);
  m_etw->init();

  layout->addWidget(m_etw);
  QWidget *rhsContainer = new QWidget(this);
  QVBoxLayout *rhsLayout = new QVBoxLayout(rhsContainer);

  m_itemView = new QTreeView(this);

  m_itemView->setModel(m_etw->model());

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), this);
  rhsLayout->addWidget(m_itemView);
  rhsLayout->addWidget(viewerWidget);
  layout->addWidget(rhsContainer);

  connect( m_etw->view(), SIGNAL(activated(QModelIndex)), SLOT(setMappedRootIndex(QModelIndex)) );
}

void Tab2_5Widget::setMappedRootIndex(const QModelIndex& index)
{
  m_itemView->setRootIndex(m_etw->mapToSource(index));
}


