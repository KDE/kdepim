/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"

#include <akonadi/entitytreemodel.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/kcal/calendar.h>
#include <ksystemtimezone.h>
#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityTreeModel>
#include <akonadi/entitymimetypefiltermodel.h>
#include <Akonadi/ItemFetchScope>

#include <QItemSelectionModel>
#include <QColumnView>

#include <akonadi_next/checkableitemproxymodel.h>
#include <QTreeView>
#include <kselectionproxymodel.h>
#include <QListView>
#include <akonadi_next/kproxyitemselectionmodel.h>

using namespace Akonadi;

MainView::MainView( QWidget *parent ) : KDeclarativeMainView( "korganizer-mobile", 0 /* TODO */, parent )
{
  addMimeType( IncidenceMimeTypeVisitor::eventMimeType() );

  Akonadi::Calendar* calendar = new Akonadi::Calendar( entityTreeModel(), itemModel(), KSystemTimeZones::local(), this );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( calendar ) ) );

  Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder( this );
  changeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  changeRecorder->itemFetchScope().fetchFullPayload(true);
  foreach ( const QString &mimeType, mimeTypes() )
    changeRecorder->setMimeTypeMonitored( mimeType );

  m_etm = new Akonadi::EntityTreeModel( changeRecorder, this );

  Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  collectionFilter->setSourceModel(m_etm);

  QItemSelectionModel *favSelection = new QItemSelectionModel(collectionFilter, this);

  // Need to proxy the selection because the favSelection operates on collectionFilter, but the
  // KSelectionProxyModel *list below operates on m_etm.
  Future::KProxyItemSelectionModel *selectionProxy = new Future::KProxyItemSelectionModel(m_etm, favSelection, this);

  // Show the list of currently selected items.
  KSelectionProxyModel *collectionList = new KSelectionProxyModel(selectionProxy, this);
  collectionList->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  collectionList->setSourceModel(m_etm);

  // Show the list of currently selected items.
  KSelectionProxyModel *list = new KSelectionProxyModel(selectionProxy, this);
  list->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
  list->setSourceModel(m_etm);

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel(this);
  itemFilter->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  itemFilter->setSourceModel(list);

  // Make it possible to uncheck currently selected items in the list
  CheckableItemProxyModel *currentSelectionCheckableProxyModel = new CheckableItemProxyModel(this);
  currentSelectionCheckableProxyModel->setSourceModel(collectionList);
  Future::KProxyItemSelectionModel *proxySelector = new Future::KProxyItemSelectionModel(collectionList, favSelection);
  currentSelectionCheckableProxyModel->setSelectionModel(proxySelector);

  // Make it possible to check/uncheck items in the column view.
  CheckableItemProxyModel *checkableSelectionModel = new CheckableItemProxyModel(this);
  checkableSelectionModel->setSelectionModel(favSelection);
  checkableSelectionModel->setSourceModel(collectionFilter);

#if 1
  QTreeView *etmView = new QTreeView;
  etmView->setModel(m_etm);
  etmView->show();
  etmView->setWindowTitle("ETM");

  QListView *currentlyCheckedView = new QListView;
  currentlyCheckedView->setModel(currentSelectionCheckableProxyModel);
  currentlyCheckedView->show();
  currentlyCheckedView->setWindowTitle("Currently checked collections");

  QColumnView *columnView = new QColumnView;
  columnView->setModel(checkableSelectionModel);
  columnView->show();
  columnView->setWindowTitle("All collections. Checkable");

  QListView *childItemsView = new QListView;
  childItemsView->setModel(itemFilter);
  childItemsView->show();
  childItemsView->setWindowTitle("List of items in checked collections");
#endif

  engine()->rootContext()->setContextProperty( "checkableSelectionModel", QVariant::fromValue( static_cast<QObject*>( checkableSelectionModel ) ) );

}

#include "mainview.moc"
