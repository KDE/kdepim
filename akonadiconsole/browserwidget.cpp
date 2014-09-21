/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "browserwidget.h"

#include "collectionattributespage.h"
#include "collectioninternalspage.h"
#include "collectionaclpage.h"
#include "dbaccess.h"
#include "akonadibrowsermodel.h"

#include <akonadi/attributefactory.h>
#include <akonadi/changerecorder.h>
#include <akonadi/control.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/job.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionpropertiesdialog.h>
#include <akonadi/selectionproxymodel.h>
#include <akonadi/session.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi/entitylistview.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi/favoritecollectionsmodel.h>
#include <akonadi_next/quotacolorproxymodel.h>
#include <akonadi/tagmodel.h>
#include <libkdepim/misc/statisticsproxymodel.h>
#include <kviewstatemaintainer.h>

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kcalcore/incidence.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kxmlguiwindow.h>
#include <KToggleAction>
#include <KActionCollection>

#include <QSplitter>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QSqlQuery>
#include <QTimer>
#include <QSqlError>

using namespace Akonadi;

AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionAttributePageFactory, CollectionAttributePage)
AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionInternalsPageFactory, CollectionInternalsPage)
AKONADI_COLLECTION_PROPERTIES_PAGE_FACTORY(CollectionAclPageFactory, CollectionAclPage)

Q_DECLARE_METATYPE( QSet<QByteArray> )

BrowserWidget::BrowserWidget(KXmlGuiWindow *xmlGuiWindow, QWidget * parent) :
    QWidget( parent ),
    mAttrModel( 0 ),
    mStdActionManager( 0 ),
    mMonitor( 0 ),
    mCacheOnlyAction( 0 )
{
  Q_ASSERT( xmlGuiWindow );
  QVBoxLayout *layout = new QVBoxLayout( this );

  QSplitter *splitter = new QSplitter( Qt::Horizontal, this );
  splitter->setObjectName( "collectionSplitter" );
  layout->addWidget( splitter );

  QSplitter *splitter2 = new QSplitter( Qt::Vertical, this );
  splitter2->setObjectName( "ffvSplitter" );

  mCollectionView = new Akonadi::EntityTreeView( xmlGuiWindow, this );
  mCollectionView->setObjectName( "CollectionView" );
  mCollectionView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  splitter2->addWidget( mCollectionView );

  EntityListView *favoritesView = new EntityListView( xmlGuiWindow, this );
  //favoritesView->setViewMode( QListView::IconMode );
  splitter2->addWidget( favoritesView );

  splitter->addWidget( splitter2 );

  ChangeRecorder *tagRecorder = new ChangeRecorder( this );
  tagRecorder->setTypeMonitored( Monitor::Tags );
  tagRecorder->setChangeRecordingEnabled( false );
  QTreeView *tagView = new QTreeView( this );
  TagModel *tagModel = new Akonadi::TagModel( tagRecorder, this );
  tagView->setModel( tagModel );
  splitter2->addWidget( tagView );

  Session *session = new Session( "AkonadiConsole Browser Widget", this );

  // monitor collection changes
  mBrowserMonitor = new ChangeRecorder( this );
  mBrowserMonitor->setSession(session);
  mBrowserMonitor->setCollectionMonitored( Collection::root() );
  mBrowserMonitor->fetchCollection( true );
  mBrowserMonitor->setAllMonitored( true );
  // TODO: Only fetch the envelope etc if possible.
  mBrowserMonitor->itemFetchScope().fetchFullPayload( true );
  mBrowserMonitor->itemFetchScope().setCacheOnly( true );

  mBrowserModel = new AkonadiBrowserModel( mBrowserMonitor, this );
  mBrowserModel->setItemPopulationStrategy( EntityTreeModel::LazyPopulation );
  mBrowserModel->setShowSystemEntities( true );

//   new ModelTest( mBrowserModel );

  EntityMimeTypeFilterModel *collectionFilter = new EntityMimeTypeFilterModel( this );
  collectionFilter->setSourceModel( mBrowserModel );
  collectionFilter->addMimeTypeInclusionFilter( Collection::mimeType() );
  collectionFilter->setHeaderGroup( EntityTreeModel::CollectionTreeHeaders );
  collectionFilter->setDynamicSortFilter( true );
  collectionFilter->setSortCaseSensitivity( Qt::CaseInsensitive );

  statisticsProxyModel = new KPIM::StatisticsProxyModel( this );
  statisticsProxyModel->setToolTipEnabled( true );
  statisticsProxyModel->setSourceModel( collectionFilter );

  QuotaColorProxyModel *quotaProxyModel = new QuotaColorProxyModel( this );
  quotaProxyModel->setWarningThreshold( 50.0 );
  quotaProxyModel->setSourceModel( statisticsProxyModel );

  mCollectionView->setModel( quotaProxyModel );

  Akonadi::SelectionProxyModel *selectionProxyModel = new Akonadi::SelectionProxyModel( mCollectionView->selectionModel(), this );
  selectionProxyModel->setSourceModel( mBrowserModel );
  selectionProxyModel->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

  EntityMimeTypeFilterModel *itemFilter = new EntityMimeTypeFilterModel( this );
  itemFilter->setSourceModel( selectionProxyModel );
  itemFilter->addMimeTypeExclusionFilter( Collection::mimeType() );
  itemFilter->setHeaderGroup( EntityTreeModel::ItemListHeaders );

  connect( mBrowserModel, SIGNAL(columnsChanged()), itemFilter, SLOT(invalidate()) );

  AkonadiBrowserSortModel *sortModel = new AkonadiBrowserSortModel( mBrowserModel, this );
  sortModel->setDynamicSortFilter( true );
  sortModel->setSourceModel( itemFilter );

  const KConfigGroup group = KGlobal::config()->group( "FavoriteCollectionsModel" );
  FavoriteCollectionsModel *favoritesModel = new FavoriteCollectionsModel( mBrowserModel, group, this );
  favoritesView->setModel( favoritesModel );


  QSplitter *splitter3 = new QSplitter( Qt::Vertical, this );
  splitter3->setObjectName( "itemSplitter" );
  splitter->addWidget( splitter3 );

  QWidget *itemViewParent = new QWidget( this );
  itemUi.setupUi( itemViewParent );

  itemUi.modelBox->addItem( "Generic" );
  itemUi.modelBox->addItem( "Mail" );
  itemUi.modelBox->addItem( "Contacts" );
  itemUi.modelBox->addItem( "Calendar/Tasks" );
  connect( itemUi.modelBox, SIGNAL(activated(int)), SLOT(modelChanged()) );
  QTimer::singleShot( 0, this, SLOT(modelChanged()) );

  itemUi.itemView->setXmlGuiClient( xmlGuiWindow );
  itemUi.itemView->setModel( sortModel );
  itemUi.itemView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  connect( itemUi.itemView, SIGNAL(activated(QModelIndex)), SLOT(itemActivated(QModelIndex)) );
  connect( itemUi.itemView, SIGNAL(clicked(QModelIndex)), SLOT(itemActivated(QModelIndex)) );
  splitter3->addWidget( itemViewParent );
  itemViewParent->layout()->setMargin( 0 );

  QWidget *contentViewParent = new QWidget( this );
  contentUi.setupUi( contentViewParent );
  contentUi.saveButton->setEnabled( false );
  connect( contentUi.saveButton, SIGNAL(clicked()), SLOT(save()) );
  splitter3->addWidget( contentViewParent );

  connect( contentUi.attrAddButton, SIGNAL(clicked()), SLOT(addAttribute()) );
  connect( contentUi.attrDeleteButton, SIGNAL(clicked()), SLOT(delAttribute()) );

  CollectionPropertiesDialog::registerPage( new CollectionAclPageFactory() );
  CollectionPropertiesDialog::registerPage( new CollectionAttributePageFactory() );
  CollectionPropertiesDialog::registerPage( new CollectionInternalsPageFactory() );

  Control::widgetNeedsAkonadi( this );

  mStdActionManager = new StandardActionManager( xmlGuiWindow->actionCollection(), xmlGuiWindow );
  mStdActionManager->setCollectionSelectionModel( mCollectionView->selectionModel() );
  mStdActionManager->setItemSelectionModel( itemUi.itemView->selectionModel() );
  mStdActionManager->setFavoriteCollectionsModel( favoritesModel );
  mStdActionManager->setFavoriteSelectionModel( favoritesView->selectionModel() );
  mStdActionManager->createAllActions();

  mCacheOnlyAction = new KToggleAction( i18n("Cache only retrieval"), xmlGuiWindow );
  mCacheOnlyAction->setChecked( true );
  xmlGuiWindow->actionCollection()->addAction( "akonadiconsole_cacheonly", mCacheOnlyAction );
  connect( mCacheOnlyAction, SIGNAL(toggled(bool)), SLOT(updateItemFetchScope()) );

  m_stateMaintainer = new KViewStateMaintainer<ETMViewStateSaver>( KGlobal::config()->group("CollectionViewState"), this );
  m_stateMaintainer->setView( mCollectionView );

  m_stateMaintainer->restoreState();
}

BrowserWidget::~BrowserWidget()
{
  m_stateMaintainer->saveState();
}

void BrowserWidget::clear()
{
  contentUi.stack->setCurrentWidget( contentUi.unsupportedTypePage );
  contentUi.dataView->clear();
  contentUi.id->clear();
  contentUi.remoteId->clear();
  contentUi.mimeType->clear();
  contentUi.revision->clear();
  contentUi.size->clear();
  contentUi.modificationtime->clear();
  contentUi.flags->clear();
  contentUi.tags->clear();
  contentUi.attrView->setModel( 0 );
}

void BrowserWidget::itemActivated(const QModelIndex & index)
{
  const Item item = index.sibling( index.row(), 0 ).data( EntityTreeModel::ItemRole ).value< Item >();
  if ( !item.isValid() ) {
    clear();
    return;
  }

  ItemFetchJob *job = new ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  job->fetchScope().fetchAllAttributes();
  job->fetchScope().setFetchTags( true );
  connect( job, SIGNAL(result(KJob*)), SLOT(itemFetchDone(KJob*)), Qt::QueuedConnection );
}

void BrowserWidget::itemFetchDone(KJob * job)
{
  ItemFetchJob *fetch = static_cast<ItemFetchJob*>( job );
  if ( job->error() ) {
    kWarning( 5265 ) << "Item fetch failed: " << job->errorString();
  } else if ( fetch->items().isEmpty() ) {
    kWarning( 5265 ) << "No item found!";
  } else {
    const Item item = fetch->items().first();
    setItem( item );
  }
}

void BrowserWidget::setItem( const Akonadi::Item &item )
{
  mCurrentItem = item;
  if ( item.hasPayload<KABC::Addressee>() ) {
    contentUi.contactView->setItem( item );
    contentUi.stack->setCurrentWidget( contentUi.contactViewPage );
  } else if ( item.hasPayload<KABC::ContactGroup>() ) {
    contentUi.contactGroupView->setItem( item );
    contentUi.stack->setCurrentWidget( contentUi.contactGroupViewPage );
  } else if ( item.hasPayload<KCalCore::Incidence::Ptr>() ) {
    contentUi.incidenceView->setItem( item );
    contentUi.stack->setCurrentWidget( contentUi.incidenceViewPage );
  } else if ( item.mimeType() == "message/rfc822"
              || item.mimeType() == "message/news" ) {
    contentUi.mailView->setMessageItem( item, MessageViewer::Viewer::Force );
    contentUi.stack->setCurrentWidget( contentUi.mailViewPage );
  } else if ( item.hasPayload<QPixmap>() ) {
    contentUi.imageView->setPixmap( item.payload<QPixmap>() );
    contentUi.stack->setCurrentWidget( contentUi.imageViewPage );
  } else {
    contentUi.stack->setCurrentWidget( contentUi.unsupportedTypePage );
  }

  QByteArray data = item.payloadData();
  contentUi.dataView->setPlainText( data );

  contentUi.id->setText( QString::number( item.id() ) );
  contentUi.remoteId->setText( item.remoteId() );
  contentUi.mimeType->setText( item.mimeType() );
  contentUi.revision->setText( QString::number( item.revision() ) );
  contentUi.size->setText( QString::number( item.size() ) );
  contentUi.modificationtime->setText( item.modificationTime().toString() + ( " UTC" ) );
  QStringList flags;
  foreach ( const Item::Flag &f, item.flags() )
    flags << QString::fromUtf8( f );
  contentUi.flags->setItems( flags );

  QStringList tags;
  foreach ( const Tag &tag, item.tags() )
    tags << tag.url().url();
  contentUi.tags->setItems( tags );

  Attribute::List list = item.attributes();
  delete mAttrModel;
  mAttrModel = new QStandardItemModel( list.count(), 2 );
  QStringList labels;
  labels << i18n( "Attribute" ) << i18n( "Value" );
  mAttrModel->setHorizontalHeaderLabels( labels );
  for ( int i = 0; i < list.count(); ++i ) {
    QModelIndex index = mAttrModel->index( i, 0 );
    Q_ASSERT( index.isValid() );
    mAttrModel->setData( index, QString::fromLatin1( list[i]->type() ) );
    index = mAttrModel->index( i, 1 );
    Q_ASSERT( index.isValid() );
    mAttrModel->setData( index, QString::fromLatin1( list[i]->serialized() ) );
    mAttrModel->itemFromIndex( index )->setFlags( Qt::ItemIsEditable | mAttrModel->flags( index ) );
  }
  contentUi.attrView->setModel( mAttrModel );

  if ( mMonitor )
    mMonitor->deleteLater(); // might be the one calling us
  mMonitor = new Monitor( this );
  mMonitor->setItemMonitored( item );
  mMonitor->itemFetchScope().fetchFullPayload();
  mMonitor->itemFetchScope().fetchAllAttributes();
  qRegisterMetaType<QSet<QByteArray> >();
  connect( mMonitor, SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), SLOT(setItem(Akonadi::Item)), Qt::QueuedConnection );
}

void BrowserWidget::modelChanged()
{
  switch ( itemUi.modelBox->currentIndex() ) {
    case 1:
      mBrowserModel->setItemDisplayMode(AkonadiBrowserModel::MailMode);
      break;
    case 2:
      mBrowserModel->setItemDisplayMode(AkonadiBrowserModel::ContactsMode);
      break;
    case 3:
      mBrowserModel->setItemDisplayMode(AkonadiBrowserModel::CalendarMode);
      break;
    default:
      mBrowserModel->setItemDisplayMode(AkonadiBrowserModel::GenericMode);
  }
}

void BrowserWidget::save()
{
  Q_ASSERT( mAttrModel );

  const QByteArray data = contentUi.dataView->toPlainText().toUtf8();
  Item item = mCurrentItem;
  item.setRemoteId( contentUi.remoteId->text() );
  foreach ( const Item::Flag &f, mCurrentItem.flags() )
    item.clearFlag( f );
  foreach ( const QString &s, contentUi.flags->items() )
    item.setFlag( s.toUtf8() );
  foreach ( const Tag &tag, mCurrentItem.tags() )
    item.clearTag( tag );
  foreach ( const QString &s, contentUi.tags->items() )
    item.setTag( Tag::fromUrl( s ) );
  item.setPayloadFromData( data );

  item.clearAttributes();
  for ( int i = 0; i < mAttrModel->rowCount(); ++i ) {
    const QModelIndex typeIndex = mAttrModel->index( i, 0 );
    Q_ASSERT( typeIndex.isValid() );
    const QModelIndex valueIndex = mAttrModel->index( i, 1 );
    Q_ASSERT( valueIndex.isValid() );
    Attribute* attr = AttributeFactory::createAttribute( mAttrModel->data( typeIndex ).toString().toLatin1() );
    Q_ASSERT( attr );
    attr->deserialize( mAttrModel->data( valueIndex ).toString().toLatin1() );
    item.addAttribute( attr );
  }

  ItemModifyJob *store = new ItemModifyJob( item, this );
  connect( store, SIGNAL(result(KJob*)), SLOT(saveResult(KJob*)) );
}

void BrowserWidget::saveResult(KJob * job)
{
  if ( job->error() ) {
    KMessageBox::error( this, i18n( "Failed to save changes: %1", job->errorString() ) );
  }
}

void BrowserWidget::addAttribute()
{
  if ( !mAttrModel || contentUi.attrName->text().isEmpty() )
    return;
  const int row = mAttrModel->rowCount();
  mAttrModel->insertRow( row );
  QModelIndex index = mAttrModel->index( row, 0 );
  Q_ASSERT( index.isValid() );
  mAttrModel->setData( index, contentUi.attrName->text() );
  contentUi.attrName->clear();
  contentUi.saveButton->setEnabled( true );
}

void BrowserWidget::delAttribute()
{
  if ( !mAttrModel )
    return;
  QModelIndexList selection = contentUi.attrView->selectionModel()->selectedRows();
  if ( selection.count() != 1 )
    return;
  mAttrModel->removeRow( selection.first().row() );
  if ( mAttrModel->rowCount() == 0 ) {
    contentUi.saveButton->setEnabled( false );
  }
}

void BrowserWidget::dumpToXml()
{
  const Collection root = currentCollection();
  if ( !root.isValid() )
    return;
  const QString fileName = KFileDialog::getSaveFileName( KUrl(), "*.xml", this, i18n( "Select XML file" ) );
  if ( fileName.isEmpty() )
    return;
#if 0 // TODO: port me, can't use XmlWriteJob here, it's in runtime, call the akonadi2xml cli tool instead
  XmlWriteJob *job = new XmlWriteJob( root, fileName, this );
  connect( job, SIGNAL(result(KJob*)), SLOT(dumpToXmlResult(KJob*)) );
#endif
}

void BrowserWidget::dumpToXmlResult( KJob* job )
{
  if ( job->error() )
    KMessageBox::error( this, job->errorString() );
}

void BrowserWidget::clearCache()
{
  const Collection coll = currentCollection();
  if ( !coll.isValid() )
    return;
  QString str = QString("DELETE FROM PimItemTable WHERE collectionId=%1").arg(coll.id());
  qDebug() << str;
  QSqlQuery query( str, DbAccess::database() );
  if (query.exec() ) {
    if (query.lastError().isValid()) {
       qDebug() << query.lastError();
       KMessageBox::error( this, query.lastError().text() );
     }
  }
}

Akonadi::Collection BrowserWidget::currentCollection() const
{
  return mCollectionView->currentIndex().data( EntityTreeModel::CollectionRole ).value<Collection>();
}

void BrowserWidget::updateItemFetchScope()
{
  mBrowserMonitor->itemFetchScope().setCacheOnly( mCacheOnlyAction->isChecked() );
}

