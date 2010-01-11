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

#include "tab2widget.h"

#include <QHBoxLayout>
#include <QTreeView>

#include "entitytreewidget.h"
#include "itemviewerwidget.h"

#include <akonadi/entitymimetypefiltermodel.h>

class Tab2TreeWidget : public EntityTreeWidget
{
public:
  Tab2TreeWidget(QWidget* parent = 0)
    : EntityTreeWidget(parent)
  {

  }

  virtual void connectTreeToModel(QTreeView* tree, Akonadi::EntityTreeModel* model)
  {
    Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);
    collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
    collectionFilter->setSourceModel(model);
    tree->setModel(collectionFilter);
  }

};

Tab2Widget::Tab2Widget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);

  EntityTreeWidget *etw = new Tab2TreeWidget(this);

  layout->addWidget(etw);
  QWidget *rhsContainer = new QWidget(this);
  QVBoxLayout *rhsLayout = new QVBoxLayout(rhsContainer);

  QTreeView *itemView = new QTreeView(this);

  itemView->setModel(etw->model());

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(itemView->selectionModel(), this);
  rhsLayout->addWidget(itemView);
  rhsLayout->addWidget(viewerWidget);
  layout->addWidget(rhsContainer);

  connect( etw->view(), SIGNAL(activated(QModelIndex)), itemView, SLOT(setRootIndex(QModelIndex)) );

}

