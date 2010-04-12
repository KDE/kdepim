/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "mainview.h"
#include "messagelistproxy.h"
#include "breadcrumbnavigation.h"

#include <QApplication>

#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>

#include <libkdepim/kdescendantsproxymodel_p.h>

#include <akonadi/changerecorder.h>
#include <akonadi/entitytreeview.h>
#include <Akonadi/ItemFetchScope>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/selectionproxymodel.h>

#include <KMime/Message>

#include "kdebug.h"
#include <kselectionproxymodel.h>

#include <akonadi_next/kbreadcrumbselectionmodel.h>
#include <akonadi_next/kproxyitemselectionmodel.h>

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeComponent>
#include <akonadi_next/etmstatesaver.h>

#define SON(o) o->setObjectName(#o)

MainView::MainView(QWidget* parent) :
  QDeclarativeView(parent),
  m_childCollectionSelection( 0 )
{
  Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder( this );
  changeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );
  changeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  changeRecorder->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Header );

  Akonadi::EntityTreeModel *etm = new Akonadi::EntityTreeModel( changeRecorder, this );
  etm->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

  m_collectionSelection = new QItemSelectionModel( etm, this );

  m_selectedSubTree = new KSelectionProxyModel( m_collectionSelection );
  m_selectedSubTree->setSourceModel( etm );

  m_collectionFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  m_collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  m_collectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  m_collectionFilter->setSourceModel( m_selectedSubTree );

  KSelectionProxyModel *currentCollectionSelectionModel = new KSelectionProxyModel( m_collectionSelection, this );
  currentCollectionSelectionModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  currentCollectionSelectionModel->setSourceModel( etm );

  Future::KBreadcrumbSelectionModel *breadcrumbCollectionSelection = new Future::KBreadcrumbSelectionModel( m_collectionSelection, Future::KBreadcrumbSelectionModel::Forward, this );
  breadcrumbCollectionSelection->setIncludeActualSelection(false);
  breadcrumbCollectionSelection->setSelectionDepth( 2 );

  KBreadcrumbNavigationProxyModel *breadcrumbNavigationModel = new KBreadcrumbNavigationProxyModel( breadcrumbCollectionSelection, this );
  breadcrumbNavigationModel->setSourceModel( etm );
  breadcrumbNavigationModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  KForwardingItemSelectionModel *oneway = new KForwardingItemSelectionModel( etm, m_collectionSelection, this );

  m_childEntitiesModel = new KNavigatingProxyModel( oneway, this );
  m_childEntitiesModel->setSourceModel( etm );

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel();
  itemFilter->setSourceModel( m_childEntitiesModel );
  itemFilter->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );

  m_childCollectionFilter = new Akonadi::EntityMimeTypeFilterModel();
  m_childCollectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  m_childCollectionFilter->setSourceModel( m_childEntitiesModel );
  m_childCollectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );

  Future::KProxyItemSelectionModel *proxyBreadcrumbCollectionSelection = new Future::KProxyItemSelectionModel( breadcrumbNavigationModel, m_collectionSelection, this );

  m_breadcrumbCollectionSelection = new KForwardingItemSelectionModel( breadcrumbNavigationModel, proxyBreadcrumbCollectionSelection, KForwardingItemSelectionModel::Reverse, this );

  m_childCollectionSelection = new Future::KProxyItemSelectionModel( m_childCollectionFilter, m_collectionSelection, this );

  MessageListProxy *messageProxy = new MessageListProxy( this );
  messageProxy->setSourceModel( itemFilter );

  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );

#if 0
  QTreeView *viewetm = new QTreeView;
  viewetm->setModel( etm );
  viewetm->setSelectionModel( m_collectionSelection );
  viewetm->show();

  QTreeView *viewCollectionFilter = new QTreeView;
  viewCollectionFilter->setModel( m_collectionFilter );
  viewCollectionFilter->show();

  QTreeView *view1 = new QTreeView;
  view1->setModel( currentCollectionSelectionModel );
  view1->show();

  QTreeView *view2 = new QTreeView;
  view2->setModel( breadcrumbNavigationModel );
  view2->setSelectionModel( m_breadcrumbCollectionSelection );
  view2->show();

  QTreeView *view3 = new QTreeView;
  view3->setModel( m_childCollectionFilter );
  view3->setSelectionModel(m_childCollectionSelection);
  view3->show();

  QTreeView *view4 = new QTreeView;
  view4->setModel( itemFilter );
  view4->setSelectionModel(m_childCollectionSelection);
  view4->show();
#endif

  // It shouldn't be necessary to have three of these once I've
  // written KReaggregationProxyModel :)
  engine()->rootContext()->setContextProperty( "selectedCollectionModel", QVariant::fromValue( static_cast<QObject*>( currentCollectionSelectionModel ) ) );
  engine()->rootContext()->setContextProperty( "breadcrumbCollectionsModel", QVariant::fromValue( static_cast<QObject*>( breadcrumbNavigationModel ) ) );
  engine()->rootContext()->setContextProperty( "childCollectionsModel", QVariant::fromValue( static_cast<QObject*>( m_childCollectionFilter ) ) );
  engine()->rootContext()->setContextProperty( "itemModel", QVariant::fromValue( static_cast<QObject*>( messageProxy ) ) );
  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  const QString qmlPath = KStandardDirs::locate( "appdata", "kmail-mobile.qml" );
  setSource( qmlPath );

  connect( etm, SIGNAL(modelAboutToBeReset()), SLOT(saveState()) );
  connect( etm, SIGNAL(modelReset()), SLOT(restoreState()) );
  connect( qApp, SIGNAL(aboutToQuit()), SLOT(saveState()) );

  restoreState();
}

void MainView::saveState()
{
  ETMStateSaver saver;
  saver.setSelectionModel(m_collectionSelection);

  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver.saveState( cfg );
  cfg.sync();
}

void MainView::restoreState()
{
  ETMStateSaver *saver = new ETMStateSaver;
  saver->setSelectionModel(m_collectionSelection);
  KConfigGroup cfg( KGlobal::config(), "SelectionState" );
  saver->restoreState( cfg );
}

void MainView::setSelectedChildCollectionRow(int row)
{
  if ( row < 0 )
  {
    m_collectionSelection->clearSelection();
    return;
  }
  QModelIndex index = m_childCollectionSelection->model()->index( row, 0 );
  m_childCollectionSelection->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void MainView::setSelectedBreadcrumbCollectionRow(int row)
{
  if ( row < 0 )
  {
    m_collectionSelection->clearSelection();
    return;
  }
  QModelIndex index = m_breadcrumbCollectionSelection->model()->index( row, 0 );
  m_breadcrumbCollectionSelection->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

bool MainView::childCollectionHasChildren( int row )
{
  if ( row < 0)
    return false;
  QModelIndex idx = m_childCollectionFilter->index( row, 0 );
  QModelIndex idx2 = m_childCollectionFilter->mapToSource( idx );
  QModelIndex idx3 = m_childEntitiesModel->mapToSource( idx2 );
  QModelIndex idx4 = m_selectedSubTree->mapFromSource( idx3 );
  QModelIndex idx5 = m_collectionFilter->mapFromSource( idx4 );
  if ( !idx5.isValid() )
    return false;

  return idx5.model()->rowCount( idx5 ) > 0;
}

#include "mainview.moc"

