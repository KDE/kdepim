/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

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

#include "agentstatusmonitor.h"
#include "akonadibreadcrumbnavigationfactory.h"
#include "declarativewidgetbase.h"
#include "exporthandlerbase.h"
#include "guistatemanager.h"
#include "importhandlerbase.h"
#include "kdepim-version.h"
#include "kresettingproxymodel.h"
#include "listproxy.h"
#include "qmlcheckableproxymodel.h"
#include "qmllistselectionmodel.h"
#include "statemachinebuilder.h"

#include <akonadi/agentactionmanager.h>
#include <akonadi/agentinstancemodel.h>
#include <akonadi/agentmanager.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/selectionproxymodel.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi_next/kviewstatemaintainer.h>
#include <kbreadcrumbselectionmodel.h>
#include <klinkitemselectionmodel.h>
#include <kselectionproxymodel.h>

#include <KDE/KAboutData>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KCmdLineArgs>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KFileDialog>
#include <KDE/KGlobal>
#include <KDE/KInputDialog>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KMessageBox>
#include <KDE/KProcess>
#include <KDE/KRun>
#include <KDE/KSharedConfig>
#include <KDE/KSharedConfigPtr>
#include <KDE/KStandardDirs>
#include <KDE/KToolInvocation>
#include "kdeversion.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <QtGui/QApplication>
#include <QtGui/QTreeView>

#include <QtDeclarative/QDeclarativeItem>

#include <sys/utsname.h>

#ifdef Q_OS_WINCE
#include <windows.h>
#include <Shellapi.h>
#endif

#define VIEW(model) {                        \
  QTreeView *view = new QTreeView( this );   \
  view->setWindowFlags( Qt::Window );        \
  view->setAttribute(Qt::WA_DeleteOnClose);  \
  view->setModel(model);                     \
  view->setWindowTitle(#model);              \
  view->show();                              \
}                                            \

class ActionImageProvider : public QDeclarativeImageProvider
{
  public:
    ActionImageProvider()
      : QDeclarativeImageProvider( QDeclarativeImageProvider::Pixmap )
    {
    }

    QPixmap requestPixmap( const QString &id, QSize *size, const QSize &requestedSize )
    {
      int width = 32;
      int height = 32;
      if ( requestedSize.isValid() ) {
        width = requestedSize.width();
        height = requestedSize.height();
      }

      if ( size )
        *size = QSize( width, height );

      const QIcon icon = KIconLoader::global()->loadIcon( id, KIconLoader::Dialog, KIconLoader::SizeHuge );
      return icon.pixmap( width, height );
    }
};

using namespace Akonadi;

typedef DeclarativeWidgetBase<KLineEdit, KDeclarativeMainView, &KDeclarativeMainView::setFilterLineEdit> DeclarativeFilterLineEdit;
typedef DeclarativeWidgetBase<KLineEdit, KDeclarativeMainView, &KDeclarativeMainView::setBulkActionFilterLineEdit> DeclarativeBulkActionFilterLineEdit;
QML_DECLARE_TYPE( DeclarativeFilterLineEdit )
QML_DECLARE_TYPE( DeclarativeBulkActionFilterLineEdit )
QML_DECLARE_TYPE( AgentStatusMonitor )
QML_DECLARE_TYPE( GuiStateManager )

KDeclarativeMainView::KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent )
  : KDeclarativeFullScreenView( appName, parent )
  , d( new KDeclarativeMainViewPrivate( this ) )
{
  d->mListProxy = listProxy;

  ActionImageProvider *provider = new ActionImageProvider;
  engine()->addImageProvider( QLatin1String( "action_images" ), provider );

  d->mSearchManager = new SearchManager( this );
  connect( d->mSearchManager, SIGNAL( searchStarted( const Akonadi::Collection& ) ),
           d, SLOT( searchStarted( const Akonadi::Collection& ) ) );
  connect( d->mSearchManager, SIGNAL( searchStopped() ),
           d, SLOT( searchStopped() ) );
}

void KDeclarativeMainView::doDelayedInitInternal()
{
  qmlRegisterType<DeclarativeFilterLineEdit>( "org.kde.akonadi", 4, 5, "FilterLineEdit" );
  qmlRegisterType<DeclarativeBulkActionFilterLineEdit>( "org.kde.akonadi", 4, 5, "BulkActionFilterLineEdit" );
  qmlRegisterUncreatableType<GuiStateManager>( "org.kde.pim.mobileui", 4, 5, "GuiStateManager", QLatin1String( "This type is only exported for its enums" ) );

  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet( "timeit" );

  QTime time;
  if ( debugTiming ) {
    time.start();
    kWarning() << "Start KDeclarativeMainView ctor" << &time << " - " << QDateTime::currentDateTime();
  }

  KGlobal::locale()->insertCatalog( QLatin1String( "libkdepimmobileui" ) );
  KGlobal::locale()->insertCatalog( QLatin1String( "libincidenceeditors" ) ); // for category dialog

  if ( debugTiming ) {
    kWarning() << "Catalog inserted" << time.elapsed() << &time;
  }

  d->mChangeRecorder = new Akonadi::ChangeRecorder( this );
  d->mChangeRecorder->fetchCollection( true );
  d->mChangeRecorder->setCollectionMonitored( Akonadi::Collection::root() );
  d->mChangeRecorder->itemFetchScope().setFetchModificationTime( false );

  d->mEtm = new Akonadi::EntityTreeModel( d->mChangeRecorder, this );
  d->mEtm->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );
  d->mEtm->setIncludeUnsubscribed( false );

  if ( debugTiming ) {
    kWarning() << "ETM created" << time.elapsed() << &time;
  }

  QAbstractItemModel *mainModel = d->mEtm;

  QAbstractProxyModel *mainProxyModel = createMainProxyModel();
  if ( mainProxyModel ) {
    mainProxyModel->setSourceModel( mainModel );
    mainModel = mainProxyModel;
  }

  d->mBnf = new Akonadi::BreadcrumbNavigationFactory( this );
  d->mBnf->createBreadcrumbContext( mainModel, this );

  if ( debugTiming ) {
    kWarning() << "BreadcrumbNavigation factory created" << time.elapsed() << &time;
  }

  QDeclarativeContext *context = engine()->rootContext();
  context->setContextProperty( "searchManager", d->mSearchManager );

  context->setContextProperty( "_breadcrumbNavigationFactory", d->mBnf );

  d->mMultiBnf = new Akonadi::BreadcrumbNavigationFactory( this );
  d->mMultiBnf->createCheckableBreadcrumbContext( mainModel, this );

  context->setContextProperty( "_multiSelectionComponentFactory", d->mMultiBnf );

  context->setContextProperty( "accountsModel", QVariant::fromValue( static_cast<QObject*>( mainModel ) ) );

  Akonadi::EntityMimeTypeFilterModel *filterModel = new Akonadi::EntityMimeTypeFilterModel( this );
  filterModel->setSourceModel( d->mBnf->unfilteredChildItemModel() );
  filterModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );

  d->mItemModel = createItemModelContext( context, filterModel );

  context->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  // The global screen manager
  d->mGuiStateManager = createGuiStateManager();
  context->setContextProperty( "guiStateManager", QVariant::fromValue( static_cast<QObject*>( d->mGuiStateManager ) ) );
  connect( d->mGuiStateManager, SIGNAL( guiStateChanged( int, int ) ), d, SLOT( guiStateChanged( int, int ) ) );

  // A list of available favorites
  d->mFavoritesEditor = new FavoritesEditor( actionCollection(), KGlobal::config(), this );
  d->mFavoritesEditor->setCollectionSelectionModel( d->mBnf->selectionModel() );

  context->setContextProperty( "favoritesEditor", d->mFavoritesEditor );
  context->setContextProperty( "favoritesList", d->mFavoritesEditor->model() );

  // A list of agent instances
  Akonadi::AgentInstanceModel *agentInstanceModel = new Akonadi::AgentInstanceModel( this );
  d->mAgentInstanceFilterModel = new Akonadi::AgentFilterProxyModel( this );
  d->mAgentInstanceFilterModel->addCapabilityFilter( QLatin1String( "Resource" ) );
  d->mAgentInstanceFilterModel->setSourceModel( agentInstanceModel );

  context->setContextProperty( "agentInstanceList", QVariant::fromValue( static_cast<QObject*>( d->mAgentInstanceFilterModel ) ) );
  d->mAgentInstanceSelectionModel = new QItemSelectionModel( d->mAgentInstanceFilterModel, this );

  setupAgentActionManager( d->mAgentInstanceSelectionModel );

  KAction *action = KStandardAction::quit( qApp, SLOT( quit() ), this );
  actionCollection()->addAction( QLatin1String( "quit" ), action );

  action = new KAction( i18n( "Synchronize all" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( synchronizeAllItems() ) );
  actionCollection()->addAction( QLatin1String( "synchronize_all_items" ), action );

  action = new KAction( i18n( "Report Bug Or Request Feature" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( reportBug() ) );
  actionCollection()->addAction( QLatin1String( "report_bug" ), action );

  action = new KAction( i18n( "Manual" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( openManual() ) );
  actionCollection()->addAction( QLatin1String( "open_manual" ), action );

  setupStandardActionManager( regularSelectionModel(), d->mItemActionSelectionModel );

  connect( d->mEtm, SIGNAL( modelAboutToBeReset() ), d, SLOT( saveState() ) );
  connect( d->mEtm, SIGNAL( modelReset() ), d, SLOT( restoreState() ) );
  connect( qApp, SIGNAL( aboutToQuit() ), d, SLOT( saveState() ) );

  connect( d->mBnf->selectedItemModel(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ), SIGNAL( isLoadingSelectedChanged() ) );
  connect( d->mBnf->selectedItemModel(), SIGNAL( rowsInserted( QModelIndex, int, int ) ), SIGNAL( isLoadingSelectedChanged() ) );
  connect( d->mBnf->selectedItemModel(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), SIGNAL( isLoadingSelectedChanged() ) );

  connect( d->mBnf->qmlBreadcrumbsModel(), SIGNAL( rowsInserted( QModelIndex, int, int ) ), SLOT( breadcrumbsSelectionChanged() ) );
  connect( d->mBnf->qmlBreadcrumbsModel(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), SLOT( breadcrumbsSelectionChanged() ) );
  connect( d->mBnf->qmlSelectedItemModel(), SIGNAL( rowsInserted( QModelIndex, int, int ) ), SLOT( breadcrumbsSelectionChanged() ) );
  connect( d->mBnf->qmlSelectedItemModel(), SIGNAL( rowsRemoved( QModelIndex, int, int ) ), SLOT( breadcrumbsSelectionChanged() ) );

  if ( debugTiming ) {
    kWarning() << "Restoring state" << time.elapsed() << &time;
  }

  d->restoreState();

  if ( debugTiming ) {
    kWarning() << "restore state done" << time.elapsed() << &time;
  }

  connect( d->mBnf->selectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SIGNAL( numSelectedAccountsChanged() ) );

  if ( debugTiming ) {
    time.start();
    kWarning() << "Finished KDeclarativeMainView ctor: " << time.elapsed() << " - " << &time;
  }

  qmlRegisterUncreatableType<AgentStatusMonitor>( "org.kde.pim.mobileui", 4, 5, "AgentStatusMonitor", QLatin1String( "This type is only exported for its enums" ) );
  d->mAgentStatusMonitor = new  AgentStatusMonitor( this );
  d->mAgentStatusMonitor->setMimeTypeFilter( d->mChangeRecorder->mimeTypesMonitored() );
  context->setContextProperty( "agentStatusMonitor", QVariant::fromValue<QObject*>( d->mAgentStatusMonitor ) );

  connect( itemSelectionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ), SLOT( itemSelectionChanged() ) );
}

void KDeclarativeMainView::itemSelectionChanged()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if ( list.size() != 1 ) {
    // TODO Clear messageViewerItem
    return;
  }

  const QModelIndex itemIndex = list.first();
  const Akonadi::Collection parentCollection = itemIndex.data( Akonadi::EntityTreeModel::ParentCollectionRole ).value<Akonadi::Collection>();
  Q_ASSERT( parentCollection.isValid() );
  const QModelIndex index = EntityTreeModel::modelIndexForCollection( entityTreeModel(), parentCollection );
  Q_ASSERT( index.isValid() );

  const Akonadi::Item item = itemIndex.data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  viewSingleItem( item );
}

void KDeclarativeMainView::viewSingleItem( const Akonadi::Item& )
{
}

bool KDeclarativeMainView::useFilterLineEditInCurrentState() const
{
  return false;
}

KDeclarativeMainView::~KDeclarativeMainView()
{
  delete d;
}

void KDeclarativeMainView::setItemNaigationAndActionSelectionModels( QItemSelectionModel *itemNavigationSelectionModel, QItemSelectionModel *itemActionSelectionModel )
{
  d->mItemNavigationSelectionModel = itemNavigationSelectionModel;

  d->mItemViewStateMaintainer = new Future::KViewStateMaintainer<ETMViewStateSaver>( KGlobal::config()->group( QLatin1String( "ItemSelectionState" ) ), this );
  d->mItemViewStateMaintainer->setSelectionModel( d->mItemNavigationSelectionModel );

  d->mItemActionSelectionModel = itemActionSelectionModel;
}

QAbstractItemModel* KDeclarativeMainView::createItemModelContext( QDeclarativeContext *context, QAbstractItemModel *model )
{
  d->mItemFilterModel = createItemFilterModel();
  if ( d->mItemFilterModel ) {
    d->mItemFilterModel->setSourceModel( model );
    model = d->mItemFilterModel;
  }

  QMLCheckableItemProxyModel *qmlCheckable = new QMLCheckableItemProxyModel( this );
  qmlCheckable->setSourceModel( model );

  QItemSelectionModel *itemActionCheckModel = new QItemSelectionModel( model, this );
  qmlCheckable->setSelectionModel( itemActionCheckModel );

  KSelectionProxyModel *checkedItems = new KSelectionProxyModel( itemActionCheckModel, this );
  checkedItems->setFilterBehavior( KSelectionProxyModel::ExactSelection );
  checkedItems->setSourceModel( model );

  QItemSelectionModel *itemSelectionModel = new QItemSelectionModel( model, this );

  if ( d->mListProxy ) {
    d->mListProxy->setParent( this ); // Make sure the proxy gets deleted when this gets deleted.

    d->mListProxy->setSourceModel( qmlCheckable );
  }

  KLinkItemSelectionModel *itemNavigationSelectionModel = new KLinkItemSelectionModel( d->mListProxy, itemSelectionModel, this );
  KLinkItemSelectionModel *itemActionSelectionModel = new KLinkItemSelectionModel( d->mListProxy, itemActionCheckModel, this );

  setItemNaigationAndActionSelectionModels( itemNavigationSelectionModel, itemActionSelectionModel );

  if ( d->mListProxy ) {
    context->setContextProperty( "itemModel", d->mListProxy );

    QMLListSelectionModel *qmlItemNavigationSelectionModel = new QMLListSelectionModel( d->mItemNavigationSelectionModel, this );
    QMLListSelectionModel *qmlItemActionSelectionModel = new QMLListSelectionModel( d->mItemActionSelectionModel, this );

    context->setContextProperty( "_itemNavigationModel", QVariant::fromValue( static_cast<QObject*>( qmlItemNavigationSelectionModel ) ) );
    context->setContextProperty( "_itemActionModel", QVariant::fromValue( static_cast<QObject*>( qmlItemActionSelectionModel ) ) );

    Akonadi::BreadcrumbNavigationFactory *bulkActionBnf = new Akonadi::BreadcrumbNavigationFactory( this );
    bulkActionBnf->createCheckableBreadcrumbContext( d->mEtm, this );
    context->setContextProperty( "_bulkActionBnf", QVariant::fromValue( static_cast<QObject*>( bulkActionBnf ) ) );
  }

  StateMachineBuilder *builder = new StateMachineBuilder;
  builder->setItemSelectionModel( itemNavigationSelectionModel );
  builder->setNavigationModel( d->mBnf->selectionModel() );
  d->mStateMachine = builder->getMachine( this );
  Q_ASSERT( d->mStateMachine );
  connect( d->mStateMachine, SIGNAL( stateChanged() ), SIGNAL( stateChanged() ) );
  d->mStateMachine->start();
  delete builder;

  return model;
}

void KDeclarativeMainView::setApplicationState( const QString &state )
{
  d->mStateMachine->requestState( state );
}

QString KDeclarativeMainView::applicationState() const
{
  if ( !d->mStateMachine )
    return QString();

  const QSet<QAbstractState*> set = d->mStateMachine->configuration();
  if ( set.isEmpty() )
    return QString();
  Q_ASSERT( !set.isEmpty() );

  QSet<QAbstractState*>::const_iterator it = set.begin();
  const QSet<QAbstractState*>::const_iterator end = set.end();
  QObject *top = *it;
  ++it;
  for ( ; it != end; ++it ) {
    QObject *state = *it;
    QObject *parent = state->parent();
    while ( parent ) {
      if ( parent == top )
        top = state;
      parent = parent->parent();
    }
  }

  return top->objectName();
}

void KDeclarativeMainView::breadcrumbsSelectionChanged()
{
  const int numBreadcrumbs = qobject_cast<QAbstractItemModel*>( d->mBnf->qmlBreadcrumbsModel() )->rowCount();
  const int numSelectedItems = qobject_cast<QAbstractItemModel*>( d->mBnf->qmlSelectedItemModel() )->rowCount();

  if ( d->mGuiStateManager->inSearchScreenState() ||
       d->mGuiStateManager->inSearchResultScreenState() ||
       d->mGuiStateManager->inViewSingleItemState() )
    return;

  if ( numBreadcrumbs == 0 && numSelectedItems == 0) {
    d->mGuiStateManager->switchState( GuiStateManager::HomeScreenState );
  } else if ( numBreadcrumbs == 0 && numSelectedItems == 1 ) {
    d->mGuiStateManager->switchState( GuiStateManager::AccountScreenState );
  } else if ( numSelectedItems > 1 ) {
    d->mGuiStateManager->switchState( GuiStateManager::MultipleFolderScreenState );
  } else {
    d->mGuiStateManager->switchState( GuiStateManager::SingleFolderScreenState );
  }
}

ItemFetchScope& KDeclarativeMainView::itemFetchScope()
{
  return d->mChangeRecorder->itemFetchScope();
}

void KDeclarativeMainView::addMimeType( const QString &mimeType )
{
  d->mChangeRecorder->setMimeTypeMonitored( mimeType );
  d->mAgentInstanceFilterModel->addMimeTypeFilter( mimeType );
  d->mAgentStatusMonitor->setMimeTypeFilter( d->mChangeRecorder->mimeTypesMonitored() );
}

QStringList KDeclarativeMainView::mimeTypes() const
{
  return d->mChangeRecorder->mimeTypesMonitored();
}

void KDeclarativeMainView::setAgentInstanceListSelectedRow( int row )
{
  static const int column = 0;
  const QModelIndex idx = d->mAgentInstanceSelectionModel->model()->index( row, column );
  d->mAgentInstanceSelectionModel->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
}

void KDeclarativeMainView::setSelectedAccount( int row )
{
  d->mBnf->selectionModel()->clearSelection();
  if ( row < 0 )
    return;

  d->mBnf->selectChild( row );
}

Akonadi::EntityTreeModel* KDeclarativeMainView::entityTreeModel() const
{
  return d->mEtm;
}

QAbstractItemModel* KDeclarativeMainView::itemModel() const
{
  return d->mListProxy ? static_cast<QAbstractItemModel*>( d->mListProxy ) : static_cast<QAbstractItemModel*>( d->mItemModel );
}

void KDeclarativeMainView::launchAccountWizard()
{
#ifdef Q_OS_UNIX
  const QString inProcessAccountWizard = KStandardDirs::locate( "module", "accountwizard_plugin.so" );
  kDebug() << inProcessAccountWizard;
  if ( !inProcessAccountWizard.isEmpty() ) {
    QPluginLoader loader( inProcessAccountWizard );
    if ( loader.load() ) {
      QObject *instance = loader.instance();
      // TODO error handling
      QMetaObject::invokeMethod( instance, "run", Qt::DirectConnection, Q_ARG( QStringList, d->mChangeRecorder->mimeTypesMonitored() ) );
      loader.unload();
      return;
    } else {
      kDebug() << loader.fileName() << loader.errorString();
    }
  }
#endif

  QStringList args;
  args << QLatin1String( "--type" ) << d->mChangeRecorder->mimeTypesMonitored().join( "," );

  int pid = KProcess::startDetached( QLatin1String( "accountwizard" ), args );
  if ( !pid ) {
    // Handle error
    kDebug() << "error creating accountwizard";
  }
}

void KDeclarativeMainView::synchronizeAllItems()
{
  if ( !d->mAgentInstanceFilterModel )
    return;

  for ( int row = 0; row < d->mAgentInstanceFilterModel->rowCount(); ++row ) {
    const QModelIndex index = d->mAgentInstanceFilterModel->index( row, 0 );
    if ( !index.isValid() )
      continue;

    Akonadi::AgentInstance instance = index.data( Akonadi::AgentInstanceModel::InstanceRole ).value<Akonadi::AgentInstance>();
    if ( !instance.isValid() )
      continue;

    instance.synchronize();
  }
}

void KDeclarativeMainView::saveFavorite()
{
  bool ok;
  const QString name = KInputDialog::getText( i18n( "Select name for favorite" ),
                                              i18n( "Favorite name" ),
                                              QString(), &ok, this );

  if ( !ok || name.isEmpty() )
    return;

  d->mFavoritesEditor->saveFavorite( name );
}

void KDeclarativeMainView::loadFavorite( const QString &name )
{
  d->mFavoritesEditor->loadFavorite( name );
}

void KDeclarativeMainView::multipleSelectionFinished()
{
  const QModelIndexList list = d->mMultiBnf->checkModel()->selectedRows();

  QItemSelection selection;
  foreach ( const QModelIndex &index, list )
    selection.select( index, index );

  d->mBnf->selectionModel()->select( selection, QItemSelectionModel::ClearAndSelect );
}

QItemSelectionModel* KDeclarativeMainView::regularSelectionModel() const
{
  return d->mBnf->selectionModel();
}

Akonadi::Item KDeclarativeMainView::itemFromId( quint64 id ) const
{
  const QModelIndexList list = EntityTreeModel::modelIndexesForItem( d->mEtm, Item( id ) );
  if ( list.isEmpty() )
    return Akonadi::Item();

  return list.first().data( EntityTreeModel::ItemRole ).value<Akonadi::Item>();
}

QItemSelectionModel* KDeclarativeMainView::itemSelectionModel() const
{
  return d->mItemNavigationSelectionModel;
}

QItemSelectionModel* KDeclarativeMainView::itemActionModel() const
{
  return d->mItemActionSelectionModel;
}

void KDeclarativeMainView::persistCurrentSelection( const QString &key )
{
  ETMViewStateSaver saver;
  saver.setSelectionModel( d->mBnf->selectionModel() );

  const QStringList selection = saver.selectionKeys();
  d->mPersistedSelections.insert( key, selection );
}

void KDeclarativeMainView::clearPersistedSelection( const QString &key )
{
  d->mPersistedSelections.remove( key );
}

void KDeclarativeMainView::restorePersistedSelection( const QString &key )
{
  if ( !d->mPersistedSelections.contains( key ) )
    return;

  const QStringList selection = d->mPersistedSelections.take( key );
  ETMViewStateSaver *restorer = new ETMViewStateSaver;

  QItemSelectionModel *selectionModel = d->mBnf->selectionModel();
  selectionModel->clearSelection();

  restorer->setSelectionModel( selectionModel );
  restorer->restoreSelection( selection );
}

void KDeclarativeMainView::importItems()
{
  ImportHandlerBase *handler = importHandler();
  if ( !handler )
    return;

  handler->setSelectionModel( regularSelectionModel() );
  handler->exec();
}

void KDeclarativeMainView::exportItems()
{
  ExportHandlerBase *handler = exportHandler();
  if ( !handler )
    return;

  handler->setSelectionModel( regularSelectionModel() );
  handler->exec();
}

void KDeclarativeMainView::exportSingleItem()
{
  ExportHandlerBase *handler = exportHandler();
  if ( !handler )
    return;

  handler->setItemSelectionModel( itemSelectionModel() );
  handler->exec();
}

/*
 * Copied from kdelibs/kdoctools/kio_help.cpp
 */
static QString lookupDocumentation( const QString &fileName )
{
  QStringList searches;

  // assemble the local search paths
  const QStringList localDirectories = KGlobal::dirs()->resourceDirs( "html" );

  qDebug() << "localDirectories:" << localDirectories;

  QStringList languages = KGlobal::locale()->languageList();
  languages.append( "en" );
  languages.removeAll( "C" );

  // this is kind of compat hack as we install our docs in en/ but the
  // default language is en_US
  for ( QStringList::Iterator it = languages.begin(); it != languages.end(); ++it ) {
    if ( *it == "en_US" )
      *it = "en";
  }

  qDebug() << "languages:" << languages;

  // look up the different languages
  foreach ( const QString &directory, localDirectories ) {
    foreach ( const QString &language, languages ) {
      searches.append( QString( "%1%2/%3" ).arg( directory, language, fileName ) );
    }
  }

  foreach ( const QString &search, searches ) {
    qDebug() << "check search:" << search;
    const QFileInfo info( search );
    qDebug() << "  exists=" << info.exists() << "isFile=" << info.isFile() << "isReadable=" << info.isReadable();
    if ( info.exists() && info.isFile() && info.isReadable() )
      return search;
  }

  return QString();
}

void KDeclarativeMainView::openManual()
{
  const QString path = lookupDocumentation( "kontact-touch/manual/index.html" );
  const KUrl url = path;
  const bool isValid = url.isValid();
  
  qDebug() << "manual path:  " << path;
  qDebug() << "manual url:   " << url;
  qDebug() << "url is valid: " << isValid;
  
  if ( !isValid ) {
    KMessageBox::error( this,
                        i18n( "The manual could not be found on your system." ),
                        i18n( "Manual not found" ) );
    return;
  }

  openAttachment( path, QLatin1String( "text/html" ) );
}

void KDeclarativeMainView::openLicenses()
{
  KDeclarativeMainView::openAttachment( KGlobal::dirs()->findResource( "data", "kontact-touch/licenses.pdf" ),
                                        QLatin1String( "application/pdf" ) );
}

void KDeclarativeMainView::openAttachment( const QString &url, const QString &mimeType )
{
  qDebug() << "opening attachment: " << url;
#ifndef Q_OS_WINCE
   KRun::runUrl( KUrl( url ), mimeType, this );
#else
   SHELLEXECUTEINFO execinfo;
   WCHAR wfile[2048];
   int num = KUrl(url).toLocalFile().toWCharArray(wfile);
   wfile[num] = '\0';

   memset(&execinfo, 0, sizeof(SHELLEXECUTEINFO));
   execinfo.cbSize = sizeof(SHELLEXECUTEINFO);
   execinfo.lpVerb = L"open";
   execinfo.lpFile = wfile;
   execinfo.lpParameters = L"";
   execinfo.lpDirectory = L"";
   execinfo.nShow = SW_SHOWNORMAL;
   execinfo.fMask = SEE_MASK_NOCLOSEPROCESS; // don't close process after ShellExecuteEx function exits
   ShellExecuteEx(&execinfo);
#endif
}

void KDeclarativeMainView::saveAttachment( const QString &url )
{
  QString fileName = KUrl( url ).fileName();
  if ( fileName.isEmpty() ) {
    fileName = i18nc( "filename for an unnamed attachment", "attachment.1" );
  }

  const QString targetFile = KFileDialog::getSaveFileName( KUrl( "kfiledialog:///saveAttachment/" + fileName ),
                                                           QString(),
                                                           this,
                                                           i18n( "Save Attachment" ) );
  if ( targetFile.isEmpty() ) {
    return;
  }

  if ( QFile::exists( targetFile ) ) {
    if ( KMessageBox::warningContinueCancel( this,
            i18n( "A file named <br><filename>%1</filename><br>already exists.<br><br>Do you want to overwrite it?",
                  targetFile ),
            i18n( "File Already Exists" ), KGuiItem(i18n("&Overwrite")) ) == KMessageBox::Cancel) {
        return;
    }

    QFile::remove( targetFile );
  }

  QFile file( url );
  bool success = file.open( QFile::ReadOnly );
  if ( success )
    success = file.copy( targetFile );

  if ( !success ) {
    KMessageBox::error( this,
                        i18nc( "1 = file name, 2 = error string",
                               "<qt>Could not write to the file<br><filename>%1</filename><br><br>%2",
                               targetFile,
                               file.errorString() ),
                        i18n( "Error saving attachment" ) );
  }

  file.close();
}


int KDeclarativeMainView::numSelectedAccounts()
{
  const QModelIndexList list = d->mBnf->selectionModel()->selectedRows();
  if ( list.isEmpty() )
    return 0;

  QSet<QString> resources;

  foreach ( const QModelIndex &index, list ) {
    const Collection collection = index.data( EntityTreeModel::CollectionRole ).value<Collection>();
    if ( !collection.isValid() )
      continue;

    resources.insert( collection.resource() );
  }

  return resources.size();
}

QAbstractItemModel* KDeclarativeMainView::selectedItemsModel() const
{
  return d->mBnf->selectedItemModel();
}

bool KDeclarativeMainView::isLoadingSelected()
{
  const QModelIndex index = d->mBnf->selectedItemModel()->index( 0, 0 );
  if ( !index.isValid() )
    return false;

  const QVariant fetchStateData = index.data( EntityTreeModel::FetchStateRole );
  Q_ASSERT( fetchStateData.isValid() );

  const EntityTreeModel::FetchState fetchState = static_cast<EntityTreeModel::FetchState>( fetchStateData.toInt() );
  return (fetchState == EntityTreeModel::FetchingState);
}

void KDeclarativeMainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                                       QItemSelectionModel *itemSelectionModel )
{
  Akonadi::StandardActionManager *standardActionManager = new Akonadi::StandardActionManager( actionCollection(), this );
  standardActionManager->setItemSelectionModel( itemSelectionModel );
  standardActionManager->setCollectionSelectionModel( collectionSelectionModel );
  standardActionManager->createAllActions();
}

void KDeclarativeMainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = new Akonadi::AgentActionManager( actionCollection(), this );
  manager->setSelectionModel( selectionModel );
  manager->createAllActions();
}

QAbstractProxyModel* KDeclarativeMainView::itemFilterModel() const
{
  return d->mItemFilterModel;
}

QAbstractProxyModel* KDeclarativeMainView::listProxy() const
{
  return d->mListProxy;
}

QAbstractProxyModel* KDeclarativeMainView::createMainProxyModel() const
{
  return 0;
}

QAbstractProxyModel* KDeclarativeMainView::createItemFilterModel() const
{
  return 0;
}

void KDeclarativeMainView::setItemFilterModel(QAbstractProxyModel* model)
{
  d->mItemFilterModel = model;
}

ImportHandlerBase* KDeclarativeMainView::importHandler() const
{
  return 0;
}

ExportHandlerBase* KDeclarativeMainView::exportHandler() const
{
  return 0;
}

GuiStateManager* KDeclarativeMainView::createGuiStateManager() const
{
  return new GuiStateManager();
}

QString KDeclarativeMainView::version() const
{
  const static QString svn_rev = QLatin1String( KDEPIM_GIT_REVISION_STRING );
  if ( svn_rev.isEmpty() ) {
    return i18n( "Version: %1", QLatin1String( KDEPIM_VERSION ) );
  } else {
    return i18n( "Version: %1 (%2)\nLast change: %3", QLatin1String( KDEPIM_VERSION ), KDEPIM_GIT_REVISION_STRING, KDEPIM_GIT_LAST_CHANGE );
  }
}

QString KDeclarativeMainView::name() const
{
  const static QString app_name = QString( KGlobal::mainComponent().aboutData()->programName() );
  return app_name;
}

Akonadi::ChangeRecorder* KDeclarativeMainView::monitor() const
{
  return d->mChangeRecorder;
}

GuiStateManager* KDeclarativeMainView::guiStateManager() const
{
  return d->mGuiStateManager;
}

void KDeclarativeMainView::setFilterLineEdit( KLineEdit *lineEdit )
{
  Q_ASSERT( !d->mFilterLineEdit );

  d->mFilterLineEdit = lineEdit;
  d->mFilterLineEdit->setFixedHeight( 0 );
  d->mFilterLineEdit->setClearButtonShown( true );
  connect( d->mFilterLineEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( filterLineEditChanged( const QString& ) ) );
  connect( d->mFilterLineEdit, SIGNAL( textChanged( const QString& ) ),
           d->mItemFilterModel, SLOT( setFilterString( const QString& ) ) );
}

void KDeclarativeMainView::setBulkActionFilterLineEdit( KLineEdit *lineEdit )
{
  Q_ASSERT( !d->mBulkActionFilterLineEdit );

  d->mBulkActionFilterLineEdit = lineEdit;
  d->mBulkActionFilterLineEdit->setFixedHeight( 0 );
  d->mBulkActionFilterLineEdit->setClearButtonShown( true );
  connect( d->mBulkActionFilterLineEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( bulkActionFilterLineEditChanged( const QString& ) ) );
  connect( d->mBulkActionFilterLineEdit, SIGNAL( textChanged( const QString& ) ),
           d->mItemFilterModel, SLOT( setFilterString( const QString& ) ) );
}

void KDeclarativeMainView::keyPressEvent( QKeyEvent *event )
{
  static bool isSendingEvent = false;

  KLineEdit *lineEdit = (d->mGuiStateManager->inBulkActionScreenState() ? d->mBulkActionFilterLineEdit.data() : d->mFilterLineEdit.data());

  if ( !isSendingEvent && // do not end up in a recursion
       (d->mGuiStateManager->inAccountScreenState() ||
        d->mGuiStateManager->inSingleFolderScreenState() ||
        d->mGuiStateManager->inMultipleFolderScreenState() ||
        d->mGuiStateManager->inBulkActionScreenState() ||
        useFilterLineEditInCurrentState()) && // only in the right state
       !event->text().isEmpty() && // only react on character input
       lineEdit && // only if a filter line edit has been set
       d->mItemFilterModel ) { // and a filter model is used
    isSendingEvent = true;
    QCoreApplication::sendEvent( lineEdit, event );
    isSendingEvent = false;
  } else {
    KDeclarativeFullScreenView::keyPressEvent( event );
  }
}

void KDeclarativeMainView::reportBug()
{
    QString kde_version = QString::fromLatin1( KDE_VERSION_STRING );

    struct utsname unameBuf;
    uname( &unameBuf );
    QString os = QString::fromLatin1( unameBuf.sysname ) +
          " (" + QString::fromLatin1( unameBuf.machine ) + ") "
          "release " + QString::fromLatin1( unameBuf.release );
    KUrl url = KUrl( "https://bugs.kde.org/wizard.cgi" );
    url.addQueryItem( "os", os );
    url.addQueryItem( "kdeVersion", kde_version );
    url.addQueryItem( "appVersion", KGlobal::mainComponent().aboutData()->version() );
    url.addQueryItem( "package",  KGlobal::mainComponent().aboutData()->productName() );
    url.addQueryItem( "kbugreport", "1" );

    KToolInvocation::invokeBrowser( url.url() );
}

void KDeclarativeMainView::checkAllBulkActionItems( bool check )
{
  if ( check ) {
    d->mItemActionSelectionModel->select( QItemSelection( d->mListProxy->index( 0, 0 ),
                                                          d->mListProxy->index( d->mListProxy->rowCount() - 1, 0 ) ),
                                          QItemSelectionModel::Select );
  } else {
    d->mItemActionSelectionModel->select( QItemSelection( d->mListProxy->index( 0, 0 ),
                                                          d->mListProxy->index( d->mListProxy->rowCount() - 1, 0 ) ),
                                          QItemSelectionModel::Deselect );
  }
}

#include "kdeclarativemainview.moc"
