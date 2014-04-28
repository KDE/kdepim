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

// READ THE README FILE

#include "entityfindingwidget.h"

#include <AkonadiCore/entitytreemodel.h>

#include <QLineEdit>
#include <QGridLayout>
#include <QSplitter>
#include <QListView>
#include <QTreeView>
#include <kselectionproxymodel.h>
#include <QTreeView>
#include <QLabel>
#include <QSortFilterProxyModel>

FindingETW::FindingETW(QWidget* parent)
  : EntityTreeWidget(parent)
{

}

void FindingETW::connectTreeToModel(QTreeView* tree, Akonadi::EntityTreeModel* model)
{
  QSortFilterProxyModel *proxy1 = new QSortFilterProxyModel(this);
  proxy1->setSourceModel( model );
  QSortFilterProxyModel *proxy2 = new QSortFilterProxyModel(this);
  proxy2->setSourceModel( proxy1 );
  QSortFilterProxyModel *proxy3 = new QSortFilterProxyModel(this);
  proxy3->setSourceModel( proxy2 );
  tree->setModel(proxy3);
  emit initialized();
}

EntityFindingWidget::EntityFindingWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
{
  QGridLayout *gridLayout = new QGridLayout( this );

  QLabel *collectionIdLabel = new QLabel(QLatin1String("Collection Id :"));
  QLabel *itemIdLabel = new QLabel(QLatin1String("Item Id :"));

  m_collectionIdInput = new QLineEdit;
  m_itemIdInput = new QLineEdit;

  collectionIdLabel->setBuddy( m_collectionIdInput );
  itemIdLabel->setBuddy( m_itemIdInput );

  connect( m_collectionIdInput, SIGNAL(returnPressed()), SLOT(findCollection()) );
  connect( m_itemIdInput, SIGNAL(returnPressed()), SLOT(findItem()) );

  QSplitter *splitter = new QSplitter;
  m_etw = new FindingETW();

  m_etw->init();

  connect( m_etw, SIGNAL(initialized()), SLOT(initWidget()) );

  m_selectionView = new QListView;

  splitter->addWidget( m_etw );
  splitter->addWidget( m_selectionView );

  gridLayout->addWidget( collectionIdLabel, 0, 0 );
  gridLayout->addWidget( m_collectionIdInput, 0, 1 );
  gridLayout->addWidget( itemIdLabel, 0, 2 );
  gridLayout->addWidget( m_itemIdInput, 0, 3 );
  gridLayout->addWidget( splitter, 1, 0, 1, 4 );
}

void EntityFindingWidget::initWidget()
{
  Akonadi::EntityTreeModel *etm = m_etw->model();
  QItemSelectionModel *itemSelectionModel = new QItemSelectionModel( etm );
  m_etw->view()->setSelectionModel( itemSelectionModel );

  KSelectionProxyModel *selProxy = new KSelectionProxyModel( itemSelectionModel );
  selProxy->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  selProxy->setSourceModel( m_etw->model() );

  m_selectionView->setModel( selProxy );
}

void EntityFindingWidget::findCollection()
{
  bool ok;
  Akonadi::Entity::Id id =  m_collectionIdInput->text().toULongLong( &ok );
  if ( !ok )
    return;
  QModelIndex idx = Akonadi::EntityTreeModel::modelIndexForCollection( m_etw->view()->model(), Akonadi::Collection( id ) );
  qDebug() << idx.data();
  if ( !idx.isValid() )
    return;

  m_etw->view()->selectionModel()->select( idx, QItemSelectionModel::ClearAndSelect );
  m_etw->view()->expandAll();
  m_etw->view()->scrollTo( idx );
}

void EntityFindingWidget::findItem()
{
  bool ok;
  Akonadi::Entity::Id id =  m_itemIdInput->text().toULongLong( &ok );
  if ( !ok )
    return;

  QModelIndexList list = Akonadi::EntityTreeModel::modelIndexesForItem( m_etw->view()->model(), Akonadi::Item( id ) );
  if ( list.isEmpty() )
    return;
  QModelIndex idx = list.first();
  if ( !idx.isValid() )
    return;

  m_etw->view()->selectionModel()->select( idx, QItemSelectionModel::ClearAndSelect );
  m_etw->view()->expandAll();
  m_etw->view()->scrollTo( idx );
}


