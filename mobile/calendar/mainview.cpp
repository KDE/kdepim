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
  foreach ( const QString &mimeType, mimeTypes() )
    changeRecorder->setMimeTypeMonitored( mimeType );

  m_etm = new Akonadi::EntityTreeModel( changeRecorder, this );
  m_etm->setItemPopulationStrategy( Akonadi::EntityTreeModel::NoItemPopulation );

  QItemSelectionModel *favSelection = new QItemSelectionModel(m_etm, this);

  // Show the list of currently selected items.
  KSelectionProxyModel *list = new KSelectionProxyModel(favSelection, this);
  list->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  list->setSourceModel(m_etm);

  // Make it possible to uncheck currently selected items in the list
  CheckableItemProxyModel *currentSelectionCheckableProxyModel = new CheckableItemProxyModel(this);
  currentSelectionCheckableProxyModel->setSourceModel(list);
  Future::KProxyItemSelectionModel *proxySelector = new Future::KProxyItemSelectionModel(list, favSelection);
  currentSelectionCheckableProxyModel->setSelectionModel( proxySelector );

  // Make it possible to check/uncheck items in the column view.
  CheckableItemProxyModel *checkableSelectionModel = new CheckableItemProxyModel(this);
  checkableSelectionModel->setSelectionModel(favSelection);
  checkableSelectionModel->setSourceModel(m_etm);

#if 1
  QListView *listView = new QListView;
  listView->setModel(currentSelectionCheckableProxyModel);
  listView->show();

  QColumnView *colView = new QColumnView;
  colView->setModel(checkableSelectionModel);
  colView->show();

  QTreeView *tree = new QTreeView;
  tree->setModel(checkableSelectionModel);
  tree->show();
#endif

  engine()->rootContext()->setContextProperty( "checkableSelectionModel", QVariant::fromValue( static_cast<QObject*>( checkableSelectionModel ) ) );

}

#include "mainview.moc"
