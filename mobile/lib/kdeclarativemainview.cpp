/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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
#include "kdeclarativemainview.h"
#include "kdeclarativemainview_p.h"

#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtGui/QApplication>
#include <QtGui/QColumnView>
#include <QtGui/QTreeView>
#include <QtGui/QListView>
#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusmessage.h>

#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KConfigGroup>
#include <KDE/KSharedConfig>
#include <KDE/KSharedConfigPtr>
#include <KDE/KStandardDirs>
#include <KDE/KProcess>

#include <kselectionproxymodel.h>

#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>

#include <akonadi_next/kbreadcrumbselectionmodel.h>
#include <akonadi_next/kproxyitemselectionmodel.h>
#include <akonadi_next/etmstatesaver.h>
#include <akonadi_next/checkableitemproxymodel.h>

#include "listproxy.h"

using namespace Akonadi;

KDeclarativeMainView::KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent )
  : QDeclarativeView( parent )
  , d( new KDeclarativeMainViewPrivate )
{
  setResizeMode( QDeclarativeView::SizeRootObjectToView );
#ifdef Q_WS_MAEMO_5
  setWindowState( Qt::WindowFullScreen );
#endif

  d->mChangeRecorder = new Akonadi::ChangeRecorder( this );
  d->mChangeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  d->mChangeRecorder->itemFetchScope().fetchFullPayload(); // By default fetch the full payload

  d->mEtm = new Akonadi::EntityTreeModel( d->mChangeRecorder, this );
  d->mEtm->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );

  d->mCollectionSelection = new QItemSelectionModel( d->mEtm, this );

  d->mSelectedSubTree = new Akonadi::SelectionProxyModel( d->mCollectionSelection );
  d->mSelectedSubTree->setSourceModel( d->mEtm );

  d->mCollectionFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  d->mCollectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  d->mCollectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  d->mCollectionFilter->setSourceModel( d->mSelectedSubTree );

  KSelectionProxyModel *currentCollectionSelectionModel // Deleted by ~QObect
      = new KSelectionProxyModel( d->mCollectionSelection, this );
  currentCollectionSelectionModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  currentCollectionSelectionModel->setSourceModel( d->mEtm );

  Future::KBreadcrumbSelectionModel *breadcrumbCollectionSelection
      = new Future::KBreadcrumbSelectionModel( d->mCollectionSelection, Future::KBreadcrumbSelectionModel::Forward, this );
  breadcrumbCollectionSelection->setIncludeActualSelection(false);
  breadcrumbCollectionSelection->setSelectionDepth( 2 );

  KBreadcrumbNavigationProxyModel *breadcrumbNavigationModel
      = new KBreadcrumbNavigationProxyModel( breadcrumbCollectionSelection, this );
  breadcrumbNavigationModel->setSourceModel( d->mEtm );
  breadcrumbNavigationModel->setFilterBehavior( KSelectionProxyModel::ExactSelection );

  KForwardingItemSelectionModel *oneway
      = new KForwardingItemSelectionModel( d->mEtm, d->mCollectionSelection, this );

  d->mChildEntitiesModel = new KNavigatingProxyModel( oneway, this );
  d->mChildEntitiesModel->setSourceModel( d->mEtm );

  d->mItemFilter = new Akonadi::EntityMimeTypeFilterModel();
  d->mItemFilter->setSourceModel( d->mChildEntitiesModel );
  d->mItemFilter->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );

  d->mChildCollectionFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  d->mChildCollectionFilter->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
  d->mChildCollectionFilter->setSourceModel( d->mChildEntitiesModel );
  d->mChildCollectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );

  Future::KProxyItemSelectionModel *proxyBreadcrumbCollectionSelection
      = new Future::KProxyItemSelectionModel( breadcrumbNavigationModel, d->mCollectionSelection, this );

  d->mBreadcrumbCollectionSelection = new KForwardingItemSelectionModel( breadcrumbNavigationModel,
                                                                         proxyBreadcrumbCollectionSelection,
                                                                         KForwardingItemSelectionModel::Reverse,
                                                                         this );

  d->mChildCollectionSelection = new Future::KProxyItemSelectionModel( d->mChildCollectionFilter, d->mCollectionSelection, this );

  d->mListProxy = listProxy;
  if ( listProxy ) {
    listProxy->setParent( this ); // Make sure the proxy gets deleted when this gets deleted.
    listProxy->setSourceModel( d->mItemFilter );
  }

  // It shouldn't be necessary to have three of these once I've written KReaggregationProxyModel :)
  engine()->rootContext()->setContextProperty( "accountsModel", QVariant::fromValue( static_cast<QObject*>( d->mEtm ) ) );
  engine()->rootContext()->setContextProperty( "selectedCollectionModel", QVariant::fromValue( static_cast<QObject*>( currentCollectionSelectionModel ) ) );
  engine()->rootContext()->setContextProperty( "breadcrumbCollectionsModel", QVariant::fromValue( static_cast<QObject*>( breadcrumbNavigationModel ) ) );
  engine()->rootContext()->setContextProperty( "childCollectionsModel", QVariant::fromValue( static_cast<QObject*>( d->mChildCollectionFilter ) ) );
  if ( listProxy )
    engine()->rootContext()->setContextProperty( "itemModel", QVariant::fromValue( static_cast<QObject*>( listProxy ) ) );
  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  Akonadi::EntityMimeTypeFilterModel *favCollectionFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  favCollectionFilter->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  favCollectionFilter->setSourceModel( d->mEtm );

  d->mFavSelection = new QItemSelectionModel( favCollectionFilter, this );

  // Need to proxy the selection because the favSelection operates on collectionFilter, but the
  // KSelectionProxyModel *list below operates on mEtm.
  Future::KProxyItemSelectionModel *selectionProxy = new Future::KProxyItemSelectionModel( d->mEtm, d->mFavSelection, this );

  // Show the list of currently selected items.
  Akonadi::SelectionProxyModel *favCollectionList = new Akonadi::SelectionProxyModel( selectionProxy, this );
  favCollectionList->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  favCollectionList->setSourceModel( d->mEtm );

  // Show the list of currently selected items.
  KSelectionProxyModel *favSelectedChildren = new KSelectionProxyModel(selectionProxy, this);
  favSelectedChildren->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
  favSelectedChildren->setSourceModel( d->mEtm );

  Akonadi::EntityMimeTypeFilterModel *favItemFilter = new Akonadi::EntityMimeTypeFilterModel( this );
  favItemFilter->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  favItemFilter->setSourceModel( favSelectedChildren );

  // Make it possible to uncheck currently selected items in the list
  CheckableItemProxyModel *currentSelectionCheckableProxyModel = new CheckableItemProxyModel( this );
  currentSelectionCheckableProxyModel->setSourceModel( favCollectionList );
  Future::KProxyItemSelectionModel *proxySelector = new Future::KProxyItemSelectionModel( favCollectionList, d->mFavSelection );
  currentSelectionCheckableProxyModel->setSelectionModel( proxySelector );

  // Make it possible to check/uncheck items in the column view.
  CheckableItemProxyModel *favoriteSelectionModel = new CheckableItemProxyModel( this );
  favoriteSelectionModel->setSelectionModel( d->mFavSelection );
  favoriteSelectionModel->setSourceModel( favCollectionFilter );

  QAbstractItemModel *favsList = d->getFavoritesListModel();

#if 0
  QTreeView *etmView = new QTreeView;
  etmView->setModel( d->mEtm );
  etmView->show();
  etmView->setWindowTitle( "ETM" );

  QListView *currentlyCheckedView = new QListView;
  currentlyCheckedView->setModel( currentSelectionCheckableProxyModel );
  currentlyCheckedView->show();
  currentlyCheckedView->setWindowTitle( "Currently checked collections" );

  QColumnView *columnView = new QColumnView;
  columnView->setModel( favoriteSelectionModel );
  columnView->show();
  columnView->setWindowTitle( "All collections. Checkable" );

  QListView *childItemsView = new QListView;
  childItemsView->setModel( favItemFilter );
  childItemsView->show();
  childItemsView->setWindowTitle( "List of items in checked collections" );

  QListView *favoritesView = new QListView;
  favoritesView->setModel( favsList );
  favoritesView->show();
  favoritesView->setWindowTitle( "Available Favorites" );
#endif

  engine()->rootContext()->setContextProperty( "favoritesList", QVariant::fromValue( static_cast<QObject*>( favsList ) ) );
  engine()->rootContext()->setContextProperty( "selectedFavoriteItems", QVariant::fromValue( static_cast<QObject*>( favItemFilter ) ) );
  engine()->rootContext()->setContextProperty( "favoriteSelectionModel", QVariant::fromValue( static_cast<QObject*>( favoriteSelectionModel ) ) );

  connect( d->mEtm, SIGNAL(modelAboutToBeReset()), d, SLOT(saveState()) );
  connect( d->mEtm, SIGNAL(modelReset()), d, SLOT(restoreState()) );
  connect( qApp, SIGNAL(aboutToQuit()), d, SLOT(saveState()) );

  d->restoreState();

  foreach ( const QString &importPath, KGlobal::dirs()->findDirs( "module", "imports" ) )
    engine()->addImportPath( importPath );

  const QString qmlPath = KStandardDirs::locate( "appdata", appName + ".qml" );
  setSource( qmlPath );
}

KDeclarativeMainView::~KDeclarativeMainView()
{
  delete d;
}

bool KDeclarativeMainView::childCollectionHasChildren( int row )
{
  if ( row < 0 )
    return false;

  QModelIndex idx = d->mChildCollectionFilter->index( row, 0 );
  QModelIndex idx2 = d->mChildCollectionFilter->mapToSource( idx );
  QModelIndex idx3 = d->mChildEntitiesModel->mapToSource( idx2 );
  QModelIndex idx4 = d->mSelectedSubTree->mapFromSource( idx3 );
  QModelIndex idx5 = d->mCollectionFilter->mapFromSource( idx4 );

  if ( !idx5.isValid() )
    return false;

  return idx5.model()->rowCount( idx5 ) > 0;
}

void KDeclarativeMainView::setListPayloadPart( const QByteArray &payloadPart )
{
  d->mChangeRecorder->itemFetchScope().fetchPayloadPart( payloadPart );
}

void KDeclarativeMainView::addMimeType( const QString &mimeType )
{
  d->mChangeRecorder->setMimeTypeMonitored( mimeType );
}

QStringList KDeclarativeMainView::mimeTypes() const
{
  return d->mChangeRecorder->mimeTypesMonitored();
}

void KDeclarativeMainView::setSelectedChildCollectionRow( int row )
{
  if ( row < 0 )
  {
    d->mCollectionSelection->clearSelection();
    return;
  }
  QModelIndex index = d->mChildCollectionSelection->model()->index( row, 0 );
  d->mChildCollectionSelection->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void KDeclarativeMainView::setSelectedBreadcrumbCollectionRow( int row )
{
  if ( row < 0 )
  {
    d->mCollectionSelection->clearSelection();
    return;
  }
  QModelIndex index = d->mBreadcrumbCollectionSelection->model()->index( row, 0 );
  d->mBreadcrumbCollectionSelection->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void KDeclarativeMainView::setSelectedAccount( int row )
{
  if ( row < 0 )
  {
    d->mCollectionSelection->clearSelection();
    return;
  }
  QModelIndex index = d->mCollectionSelection->model()->index( row, 0 );
  d->mCollectionSelection->select( QItemSelection(index, index), QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
}

void KDeclarativeMainView::triggerTaskSwitcher()
{
#ifdef Q_WS_MAEMO_5
  QDBusConnection::sessionBus().call( QDBusMessage::createSignal( QLatin1String( "/" ), QLatin1String( "com.nokia.hildon_desktop" ), QLatin1String( "exit_app_view" ) ), QDBus::NoBlock );
#else
  kDebug() << "not implemented for this platform";
#endif
}

Akonadi::EntityTreeModel* KDeclarativeMainView::entityTreeModel() const
{
  return d->mEtm;
}

QAbstractItemModel* KDeclarativeMainView::itemModel() const
{
  return d->mListProxy ? static_cast<QAbstractItemModel*>( d->mListProxy ) : static_cast<QAbstractItemModel*>( d->mItemFilter );
}

void KDeclarativeMainView::launchAccountWizard()
{
  QStringList args;

  foreach ( const QString &mimetype, d->mChangeRecorder->mimeTypesMonitored() )
   args << QLatin1String( "--type" ) << mimetype;

  int pid = KProcess::startDetached( QLatin1String( "accountwizard" ), args );
  if ( !pid )
  {
    // Handle error
    kDebug() << "error creating accountwizard";
  }
}

void KDeclarativeMainView::saveFavorite(const QString& name)
{
  ETMStateSaver saver;
  saver.setSelectionModel( d->mFavSelection );

  KConfigGroup cfg( KGlobal::config(), sFavoritePrefix + name );
  saver.saveState( cfg );
  cfg.sync();
  d->mFavsListModel->setStringList( d->getFavoritesList() );
}

void KDeclarativeMainView::loadFavorite(const QString& name)
{
  ETMStateSaver *saver = new ETMStateSaver;
  saver->setSelectionModel( d->mFavSelection );
  KConfigGroup cfg( KGlobal::config(), sFavoritePrefix + name );
  if ( !cfg.isValid() )
  {
    delete saver;
    return;
  }
  saver->restoreState( cfg );
}

