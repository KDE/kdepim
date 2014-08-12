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

#include "checkable2.h"

#include <QSplitter>
#include <QHBoxLayout>
#include <QTreeView>

#include "itemviewerwidget.h"

#include <kselectionproxymodel.h>
#include <kcheckableproxymodel.h>

#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/changerecorder.h>
#include <AkonadiCore/itemfetchscope.h>
#include <AkonadiCore/entitymimetypefiltermodel.h>

using namespace Akonadi;

Checkable2::Checkable2(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  QSplitter *splitter = new QSplitter(this);
  layout->addWidget(splitter);

  ChangeRecorder *changeRecorder = new ChangeRecorder( this );

  changeRecorder->setCollectionMonitored( Collection::root() );
  changeRecorder->fetchCollection( true );
  changeRecorder->setAllMonitored( true );
  changeRecorder->itemFetchScope().fetchFullPayload( true );
  changeRecorder->itemFetchScope().fetchAllAttributes( true );

  EntityTreeModel *etm = new EntityTreeModel( changeRecorder, this );

  Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);

  collectionFilter->addMimeTypeInclusionFilter(Akonadi::Collection::mimeType());
  collectionFilter->setSourceModel(etm);
  collectionFilter->setHeaderGroup(Akonadi::EntityTreeModel::CollectionTreeHeaders);

  KCheckableProxyModel *checkablePM = new KCheckableProxyModel(this);

  QItemSelectionModel *checkSelection = new QItemSelectionModel(collectionFilter, this);

  checkablePM->setSelectionModel(checkSelection);
  checkablePM->setSourceModel(collectionFilter);

  QTreeView *treeView = new QTreeView(splitter);
  treeView->setModel(checkablePM);

  KSelectionProxyModel *selectionProxy = new KSelectionProxyModel(checkSelection, this);
  selectionProxy->setFilterBehavior(KSelectionProxyModel::ChildrenOfExactSelection);
  selectionProxy->setSourceModel(etm);

  QTreeView *allChildrenView = new QTreeView(splitter);
  allChildrenView->setModel(selectionProxy);

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  itemFilter->setObjectName(QLatin1String("itemFilter"));
  itemFilter->addMimeTypeExclusionFilter(Akonadi::Collection::mimeType());
  itemFilter->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
  itemFilter->setSourceModel(selectionProxy);

  QSplitter *rhsContainer = new QSplitter(Qt::Vertical, splitter);

  m_itemView = new QTreeView(rhsContainer);

  m_itemView->setModel(itemFilter);

  ItemViewerWidget *viewerWidget = new ItemViewerWidget(m_itemView->selectionModel(), rhsContainer);
}


