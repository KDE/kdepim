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

#include "kdepim-version.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QPluginLoader>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <QtGui/QApplication>
#include <QtDBus/qdbusconnection.h>
#include <QtDBus/qdbusmessage.h>

#include <KDE/KDebug>
#include <KDE/KGlobal>
#include <KDE/KConfigGroup>
#include <KDE/KLineEdit>
#include <KDE/KLocale>
#include <KDE/KSharedConfig>
#include <KDE/KSharedConfigPtr>
#include <KDE/KStandardDirs>
#include <KDE/KProcess>

#include <kselectionproxymodel.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/agentinstancemodel.h>
#include <akonadi/agentmanager.h>
#include <akonadi/changerecorder.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/selectionproxymodel.h>
#include <akonadi_next/kviewstatemaintainer.h>

#include <kbreadcrumbselectionmodel.h>
#include <klinkitemselectionmodel.h>

#include "akonadibreadcrumbnavigationfactory.h"
#include "declarativewidgetbase.h"
#include "exporthandlerbase.h"
#include "importhandlerbase.h"
#include "listproxy.h"
#include <KActionCollection>
#include <akonadi/standardactionmanager.h>
#include <KAction>
#include <KCmdLineArgs>
#include <KInputDialog>

#include "kresettingproxymodel.h"
#include "qmllistselectionmodel.h"
#include "qmlcheckableproxymodel.h"
#include "agentstatusmonitor.h"

#include <QtGui/QTreeView>

#define VIEW(model) {                        \
  QTreeView *view = new QTreeView;           \
  view->setAttribute(Qt::WA_DeleteOnClose);  \
  view->setModel(model);                     \
  view->setWindowTitle(#model);              \
  view->show();                              \
}

ItemSelectHook::ItemSelectHook(QItemSelectionModel *selectionModel, QObject* parent)
  : QObject(parent), m_selectionModel(selectionModel)
{
  connect(selectionModel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged()));
}

void ItemSelectHook::selectionChanged()
{
  const QModelIndexList list = m_selectionModel->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Q_ASSERT(idx.isValid());
  Akonadi::Item::Id itemId = idx.data( Akonadi::EntityTreeModel::ItemIdRole ).toLongLong();
  rowSelected(idx.row(), itemId);
}

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

KDeclarativeMainView::KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent )
  : KDeclarativeFullScreenView( appName, parent )
  , d( new KDeclarativeMainViewPrivate )
{
  d->mListProxy = listProxy;

  ActionImageProvider *provider = new ActionImageProvider;
  engine()->addImageProvider( QLatin1String( "action_images" ), provider );
}

void KDeclarativeMainView::delayedInit()
{
  kDebug();
  qmlRegisterType<DeclarativeFilterLineEdit>( "org.kde.akonadi", 4, 5, "FilterLineEdit" );
  qmlRegisterType<DeclarativeBulkActionFilterLineEdit>( "org.kde.akonadi", 4, 5, "BulkActionFilterLineEdit" );

  KDeclarativeFullScreenView::delayedInit();

  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");

  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "Start KDeclarativeMainView ctor" << &t << " - " << QDateTime::currentDateTime();
  }

  KGlobal::locale()->insertCatalog( QLatin1String( "libkdepimmobileui" ) );

  if ( debugTiming ) {
    kWarning() << "Catalog inserted" << t.elapsed() << &t;
  }

  d->mChangeRecorder = new Akonadi::ChangeRecorder( this );
  d->mChangeRecorder->fetchCollection( true );
  d->mChangeRecorder->setCollectionMonitored( Akonadi::Collection::root() );

  d->mEtm = new Akonadi::EntityTreeModel( d->mChangeRecorder, this );
  d->mEtm->setItemPopulationStrategy( Akonadi::EntityTreeModel::LazyPopulation );
  d->mEtm->setIncludeUnsubscribed( false );

  if ( debugTiming ) {
    kWarning() << "ETM created" << t.elapsed() << &t;
  }

  d->mBnf = new Akonadi::BreadcrumbNavigationFactory(this);
  d->mBnf->createBreadcrumbContext(d->mEtm, this);

  if ( debugTiming ) {
    kWarning() << "BreadcrumbNavigation factory created" << t.elapsed() << &t;
  }

  Akonadi::EntityMimeTypeFilterModel *filterModel = new Akonadi::EntityMimeTypeFilterModel(this);
  filterModel->setSourceModel( d->mBnf->unfilteredChildItemModel() );
  filterModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );

  d->mItemFilter = filterModel;

  d->mItemFilterModel = itemFilterModel();
  if ( d->mItemFilterModel ) {
    d->mItemFilterModel->setSourceModel( filterModel );
    d->mItemFilter = d->mItemFilterModel;
  }

  QMLCheckableItemProxyModel *qmlCheckable = new QMLCheckableItemProxyModel( this );
  qmlCheckable->setSourceModel( d->mItemFilter );

  QItemSelectionModel *itemActionCheckModel = new QItemSelectionModel( d->mItemFilter, this );
  qmlCheckable->setSelectionModel( itemActionCheckModel );

  KSelectionProxyModel *checkedItems = new KSelectionProxyModel(itemActionCheckModel, this);
  checkedItems->setFilterBehavior(KSelectionProxyModel::ExactSelection);
  checkedItems->setSourceModel(d->mItemFilter);

  QItemSelectionModel *itemSelectionModel = new QItemSelectionModel( d->mItemFilter, this );

  if ( d->mListProxy ) {
    d->mListProxy->setParent( this ); // Make sure the proxy gets deleted when this gets deleted.

    d->mListProxy->setSourceModel( qmlCheckable);
  }
  d->mItemNavigationSelectionModel = new KLinkItemSelectionModel( d->mListProxy, itemSelectionModel, this);

  d->mItemViewStateMaintainer = new Future::KViewStateMaintainer<ETMViewStateSaver>( KGlobal::config()->group( QLatin1String( "ItemSelectionState" ) ), this);
  d->mItemViewStateMaintainer->setSelectionModel( d->mItemNavigationSelectionModel );

  d->mItemActionSelectionModel = new KLinkItemSelectionModel( d->mListProxy, itemActionCheckModel, this);

  if ( debugTiming ) {
    kWarning() << "Begin inserting QML context" << t.elapsed() << &t;
  }

  QDeclarativeContext *context = engine()->rootContext();

  context->setContextProperty( "_breadcrumbNavigationFactory", d->mBnf );

  d->mMultiBnf = new Akonadi::BreadcrumbNavigationFactory(this);
  d->mMultiBnf->createCheckableBreadcrumbContext( d->mEtm, this);

  context->setContextProperty( "_multiSelectionComponentFactory", d->mMultiBnf );

  context->setContextProperty( "accountsModel", QVariant::fromValue( static_cast<QObject*>( d->mEtm ) ) );

  if ( d->mListProxy ) {
    context->setContextProperty( "itemModel", QVariant::fromValue( static_cast<QObject*>( d->mListProxy ) ) );

    QMLListSelectionModel *qmlItemNavigationSelectionModel = new QMLListSelectionModel(d->mItemNavigationSelectionModel, this);
    QMLListSelectionModel *qmlItemActionSelectionModel = new QMLListSelectionModel(d->mItemActionSelectionModel, this);

    d->m_hook = new ItemSelectHook(d->mItemNavigationSelectionModel, this);
    context->setContextProperty( "_itemSelectHook", QVariant::fromValue( static_cast<QObject*>( d->m_hook ) ) );

    context->setContextProperty( "_itemCheckModel", QVariant::fromValue( static_cast<QObject*>( qmlItemNavigationSelectionModel ) ) );
    context->setContextProperty( "_itemActionModel", QVariant::fromValue( static_cast<QObject*>( qmlItemActionSelectionModel ) ) );

    Akonadi::BreadcrumbNavigationFactory *bulkActionBnf = new Akonadi::BreadcrumbNavigationFactory(this);
    bulkActionBnf->createCheckableBreadcrumbContext( d->mEtm, this);
    context->setContextProperty( "_bulkActionBnf", QVariant::fromValue( static_cast<QObject*>( bulkActionBnf ) ) );
  }

  context->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

  // A list of available favorites
  QAbstractItemModel *favsList = d->getFavoritesListModel();
  favsList->setParent(this);

  context->setContextProperty( "favoritesList", QVariant::fromValue( static_cast<QObject*>( favsList ) ) );

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

  setupStandardActionManager( regularSelectionModel(), d->mItemActionSelectionModel );

  connect( d->mEtm, SIGNAL(modelAboutToBeReset()), d, SLOT(saveState()) );
  connect( d->mEtm, SIGNAL(modelReset()), d, SLOT(restoreState()) );
  connect( qApp, SIGNAL(aboutToQuit()), d, SLOT(saveState()) );

  connect( d->mBnf->selectedItemModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SIGNAL(isLoadingSelectedChanged()));
  connect( d->mBnf->selectedItemModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(isLoadingSelectedChanged()));
  connect( d->mBnf->selectedItemModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(isLoadingSelectedChanged()));


  if ( debugTiming ) {
    kWarning() << "Restoring state" << t.elapsed() << &t;
  }

  d->restoreState();

  if ( debugTiming ) {
    kWarning() << "restore state done" << t.elapsed() << &t;
  }

  connect( d->mBnf->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SIGNAL(numSelectedAccountsChanged()));

  if ( debugTiming ) {
    t.start();
    kWarning() << "Finished KDeclarativeMainView ctor: " << t.elapsed() << " - " << &t;
  }

  qmlRegisterUncreatableType<AgentStatusMonitor>( "org.kde.pim.mobileui", 4, 5, "AgentStatusMonitor", QLatin1String( "This type is only exported for its enums" ) );
  context->setContextProperty( "agentStatusMonitor", QVariant::fromValue<QObject*>( new AgentStatusMonitor( this ) ) );
}

KDeclarativeMainView::~KDeclarativeMainView()
{
  delete d;
}

QString KDeclarativeMainView::pathToItem( Entity::Id id )
{
  QString path;
  const QModelIndexList list = EntityTreeModel::modelIndexesForItem( d->mEtm, Item( id ) );
  if ( list.isEmpty() )
    return QString();

  QModelIndex idx = list.first().parent();
  while ( idx.isValid() )
  {
    path.prepend( idx.data().toString() );
    idx = idx.parent();
    if ( idx.isValid() )
      path.prepend( " / " );
  }
  return path;
}

ItemFetchScope& KDeclarativeMainView::itemFetchScope()
{
  return d->mChangeRecorder->itemFetchScope();
}

void KDeclarativeMainView::addMimeType( const QString &mimeType )
{
  d->mChangeRecorder->setMimeTypeMonitored( mimeType );
  d->mAgentInstanceFilterModel->addMimeTypeFilter( mimeType );
}

QStringList KDeclarativeMainView::mimeTypes() const
{
  return d->mChangeRecorder->mimeTypesMonitored();
}

bool KDeclarativeMainView::blockHook()
{
  return d->m_hook->blockSignals( true );
}

void KDeclarativeMainView::unblockHook( bool block )
{
  d->m_hook->blockSignals( block );
}

void KDeclarativeMainView::setListSelectedRow( int row )
{
  static const int column = 0;
  const QModelIndex idx = d->mItemNavigationSelectionModel->model()->index( row, column );
  const bool blocked = d->m_hook->blockSignals( true );
  d->mItemNavigationSelectionModel->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  d->mItemActionSelectionModel->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );

  d->m_hook->blockSignals( blocked );
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
  {
    return;
  }
  d->mBnf->selectChild( row );
}

int KDeclarativeMainView::selectedCollectionRow()
{
  const QModelIndexList list = d->mBnf->selectionModel()->selectedRows();
  if (list.size() != 1)
    return -1;
  return list.first().row();
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
  if ( !pid )
  {
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
  QString name = KInputDialog::getText(i18n("Select name for favorite"), i18n("Favorite name"),
                                       QString(), &ok, this);

  if (!ok || name.isEmpty())
    return;

  ETMViewStateSaver saver;
  saver.setSelectionModel( d->mBnf->selectionModel() );

  KConfigGroup cfg( KGlobal::config(), sFavoritePrefix + name );
  saver.saveState( cfg );
  cfg.sync();
  d->mFavsListModel->setStringList( d->getFavoritesList() );
}

void KDeclarativeMainView::loadFavorite(const QString& name)
{
  ETMViewStateSaver *saver = new ETMViewStateSaver;
  saver->setSelectionModel( d->mBnf->selectionModel() );
  KConfigGroup cfg( KGlobal::config(), sFavoritePrefix + name );
  if ( !cfg.isValid() )
  {
    delete saver;
    return;
  }
  saver->restoreState( cfg );
}

void KDeclarativeMainView::multipleSelectionFinished()
{
  QModelIndexList list = d->mMultiBnf->checkModel()->selectedRows();
  QItemSelection selection;
  foreach(const QModelIndex &idx, list)
    selection.select(idx, idx);
  d->mBnf->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
}

QItemSelectionModel* KDeclarativeMainView::regularSelectionModel() const
{
  return d->mBnf->selectionModel();
}

QAbstractItemModel* KDeclarativeMainView::regularSelectedItems() const
{
  return d->mItemFilter;
}

Akonadi::Item KDeclarativeMainView::itemFromId(quint64 id) const
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

void KDeclarativeMainView::persistCurrentSelection(const QString& key)
{
  ETMViewStateSaver saver;
  saver.setSelectionModel(d->mBnf->selectionModel());
  QStringList selection = saver.selectionKeys();
  if (selection.isEmpty())
    return;

  d->mPersistedSelections.insert(key, selection);
}

void KDeclarativeMainView::clearPersistedSelection(const QString& key)
{
  d->mPersistedSelections.remove(key);
}

void KDeclarativeMainView::restorePersistedSelection(const QString& key)
{
  if (!d->mPersistedSelections.contains(key))
    return;

  QStringList selection = d->mPersistedSelections.take(key);
  ETMViewStateSaver *restorer = new ETMViewStateSaver;

  QItemSelectionModel *selectionModel = d->mBnf->selectionModel();
  selectionModel->clearSelection();

  restorer->setSelectionModel(selectionModel);
  restorer->restoreSelection(selection);
}

void KDeclarativeMainView::setBulkActionScreenSelected( bool selected )
{
  d->mIsBulkActionScreenSelected = selected;
  emit isBulkActionScreenSelectedChanged();
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

void KDeclarativeMainView::setScreenVisibilityState( ScreenStates state )
{
  d->mScreenState = state;
  emit screenVisibilityChanged();
}

bool KDeclarativeMainView::isHomeScreenVisible() const
{
  return (d->mScreenState == HomeScreen);
}

bool KDeclarativeMainView::isAccountScreenVisible() const
{
  return (d->mScreenState == AccountScreen);
}

bool KDeclarativeMainView::isSingleFolderScreenVisible() const
{
  return (d->mScreenState == SingleFolderScreen);
}

bool KDeclarativeMainView::isMultiFolderScreenVisible() const
{
  return (d->mScreenState == MultiFolderScreen);
}

int KDeclarativeMainView::numSelectedAccounts()
{
  const QModelIndexList list = d->mBnf->selectionModel()->selectedRows();
  if (list.isEmpty())
    return 0;

  QSet<QString> resources;

  foreach (const QModelIndex &index, list)
  {
    const Collection col = index.data(EntityTreeModel::CollectionRole).value<Collection>();
    if (!col.isValid())
      continue;
    resources.insert(col.resource());
  }
  return resources.size();
}

QAbstractItemModel* KDeclarativeMainView::selectedItemsModel() const
{
  return d->mBnf->selectedItemModel();
}

bool KDeclarativeMainView::isLoadingSelected()
{
  const QModelIndex idx = d->mBnf->selectedItemModel()->index(0, 0);
  if (!idx.isValid())
    return false;

  const QVariant fetchStateData = idx.data(EntityTreeModel::FetchStateRole);
  Q_ASSERT(fetchStateData.isValid());
  const EntityTreeModel::FetchState fetchState = static_cast<EntityTreeModel::FetchState>(fetchStateData.toInt());
  return fetchState == EntityTreeModel::FetchingState;
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
  return 0;
}

ImportHandlerBase* KDeclarativeMainView::importHandler() const
{
  return 0;
}

ExportHandlerBase* KDeclarativeMainView::exportHandler() const
{
  return 0;
}

QString KDeclarativeMainView::version() const
{
  return i18n( "Version: %1 (%2)\nLast change: %3", QLatin1String( KDEPIM_VERSION ), KDEPIM_SVN_REVISION_STRING, KDEPIM_SVN_LAST_CHANGE );
}

bool KDeclarativeMainView::isBulkActionScreenSelected() const
{
  return d->mIsBulkActionScreenSelected;
}

Akonadi::ChangeRecorder* KDeclarativeMainView::monitor() const
{
  return d->mChangeRecorder;
}

void KDeclarativeMainView::setFilterLineEdit( KLineEdit *lineEdit )
{
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

  KLineEdit *lineEdit = (d->mIsBulkActionScreenSelected ? d->mBulkActionFilterLineEdit : d->mFilterLineEdit);

  if ( !isSendingEvent && // do not end up in a recursion
       d->mScreenState != HomeScreen && // we are not showing the HomeScreen
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

#include "kdeclarativemainview.moc"
