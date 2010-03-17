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

#include <libkdepim/kdescendantsproxymodel_p.h>

#include <akonadi/changerecorder.h>
#include <akonadi/entitytreeview.h>
#include <Akonadi/ItemFetchScope>
#include <akonadi/entitymimetypefiltermodel.h>

#include <KMime/Message>

#include "kdebug.h"
#include <kselectionproxymodel.h>
#include <KStandardDirs>

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeComponent>

MainView::MainView(QWidget* parent) :
  QDeclarativeView(parent),
  m_collectionSelection( 0 )
{
  Akonadi::ChangeRecorder *changeRecorder = new Akonadi::ChangeRecorder( this );
  changeRecorder->setMimeTypeMonitored( KMime::Message::mimeType() );
  changeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  changeRecorder->itemFetchScope().fetchFullPayload();

  Akonadi::EntityTreeModel *etm = new Akonadi::EntityTreeModel( changeRecorder, this );
  Akonadi::EntityMimeTypeFilterModel *collectionFilter = new Akonadi::EntityMimeTypeFilterModel();
  collectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  collectionFilter->setSourceModel( etm );
  collectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );

  KDescendantsProxyModel *flatProxy = new KDescendantsProxyModel( this );
  flatProxy->setSourceModel( collectionFilter );
  flatProxy->setAncestorSeparator( QLatin1String(" / ") );
  flatProxy->setDisplayAncestorData( true );

  m_collectionSelection = new QItemSelectionModel( flatProxy, this );
  KSelectionProxyModel *selectionProxyModel = new KSelectionProxyModel( m_collectionSelection );
  selectionProxyModel->setSourceModel( etm );
  selectionProxyModel->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

  Akonadi::EntityMimeTypeFilterModel *itemFilter = new Akonadi::EntityMimeTypeFilterModel();
  itemFilter->setSourceModel( selectionProxyModel );
  itemFilter->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );

  engine()->rootContext()->setContextProperty( "collectionModel", QVariant::fromValue( static_cast<QObject*>( flatProxy ) ) );
  engine()->rootContext()->setContextProperty( "itemModel", QVariant::fromValue( static_cast<QObject*>( itemFilter ) ) );
  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  const QString qmlPath = KStandardDirs::locate( "appdata", "kmail-mobile.qml" );
  setSource( qmlPath );
}

int MainView::selectedCollectionRow() const
{
  const QModelIndexList indexes = m_collectionSelection->selectedRows();
  Q_ASSERT( indexes.size() <= 1 );
  if ( !indexes.isEmpty() )
    return indexes.first().row();
  return 0;
}

void MainView::setSelectedCollectionRow(int row)
{
  kDebug() << row;
  m_collectionSelection->select( m_collectionSelection->model()->index( row, 0 ), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

#include "mainview.moc"

