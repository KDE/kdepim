/*
    This file is part of Akregator.

    Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "selectioncontroller.h"
#include "actionmanager.h"
#include "progressmanager.h"

#include <krss/feeditemmodel.h>
#include <krss/feedpropertiescollectionattribute.h>
#include <krss/rssitem.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/Session>

#include <KSelectionProxyModel>
#include <KDebug>

#include <QAbstractItemView>
#include <QMenu>

#include <cassert>

using namespace boost;
using namespace Akregator;
using namespace KRss;

static KRss::Item itemForIndex( const QModelIndex& index )
{
    return KRss::Item( index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>() );
}

static QList<KRss::Item> itemsForIndexes( const QModelIndexList& indexes )
{
    QList<KRss::Item> items;
    Q_FOREACH ( const QModelIndex& i, indexes )
        items.append( itemForIndex( i ) );

    return items;
}

Akregator::SelectionController::SelectionController( Akonadi::Session* session, QObject* parent )
    : AbstractSelectionController( parent ),
    m_feedSelector(),
    m_articleLister( 0 ),
    m_singleDisplay( 0 ),
    m_folderExpansionHandler( 0 ),
    m_itemModel( 0 ),
    m_session( session )
{
    Akonadi::ItemFetchScope iscope;
    iscope.fetchFullPayload( true );
    iscope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    Akonadi::CollectionFetchScope cscope;
    cscope.setIncludeStatistics( true );
    cscope.setContentMimeTypes( QStringList() << KRss::Item::mimeType() );
    Akonadi::ChangeRecorder* recorder = new Akonadi::ChangeRecorder( this );
    recorder->setSession( m_session );
    recorder->fetchCollection( true );
    recorder->setCollectionFetchScope( cscope );
    recorder->setItemFetchScope( iscope );
    recorder->setCollectionMonitored( Akonadi::Collection::root() );
    recorder->setMimeTypeMonitored( KRss::Item::mimeType() );
    m_itemModel = new FeedItemModel( recorder, this );
}


void Akregator::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    Q_ASSERT( !m_feedSelector );
    m_feedSelector = feedSelector;
    init();
}

void Akregator::SelectionController::setArticleLister( Akregator::ArticleLister* lister )
{
    Q_ASSERT( !m_articleLister );
    m_articleLister = lister;
    init();
}

void Akregator::SelectionController::init() {
    if (  !m_feedSelector || !m_articleLister )
        return;

    Q_ASSERT( m_articleLister->itemView() );
    Q_ASSERT( !m_feedSelector->model() );

    Akonadi::EntityMimeTypeFilterModel* filterProxy = new Akonadi::EntityMimeTypeFilterModel( this );
    filterProxy->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
    filterProxy->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
    filterProxy->setSourceModel( m_itemModel );
    filterProxy->setDynamicSortFilter( true );

    connect( m_feedSelector, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(subscriptionContextMenuRequested(QPoint)) );
    connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(selectedSubscriptionChanged(QModelIndex)) );

    m_feedSelector->setModel( filterProxy );

    KSelectionProxyModel* selectionProxy = new KSelectionProxyModel( m_feedSelector->selectionModel() );
    selectionProxy->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

    selectionProxy->setSourceModel( m_itemModel );

    Akonadi::EntityMimeTypeFilterModel* filterProxy2 = new Akonadi::EntityMimeTypeFilterModel;
    filterProxy2->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
    filterProxy2->addMimeTypeInclusionFilter( QLatin1String("application/rss+xml") );
    filterProxy2->setSortRole( FeedItemModel::SortRole );
    filterProxy2->setDynamicSortFilter( true );
    filterProxy2->setSourceModel( selectionProxy );
    m_articleLister->setItemModel( filterProxy2 );
    connect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(itemSelectionChanged()) );
    connect( m_articleLister->itemView(), SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(itemIndexDoubleClicked(QModelIndex))  );
}

void Akregator::SelectionController::selectedSubscriptionChanged( const QModelIndex& ) {
    emit currentCollectionChanged( selectedCollection() );
}

void Akregator::SelectionController::setSingleArticleDisplay( Akregator::SingleArticleDisplay* display )
{
    m_singleDisplay = display;
}

KRss::Item Akregator::SelectionController::currentItem() const
{
    return ::itemForIndex( m_articleLister->articleSelectionModel()->currentIndex() );
}

QList<KRss::Item> Akregator::SelectionController::selectedItems() const
{
    return ::itemsForIndexes( m_articleLister->articleSelectionModel()->selectedRows() );
}

Akonadi::Collection Akregator::SelectionController::selectedCollection() const
{
    return m_feedSelector->selectionModel()->currentIndex().data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

#ifdef KRSS_PORT_DISABLED
void Akregator::SelectionController::setFeedList( const shared_ptr<KRss::FeedList>& feedList )
{

    if ( m_feedList == feedList )
        return;

    m_feedList = feedList;


    if ( m_folderExpansionHandler ) {
        m_folderExpansionHandler->setFeedList( m_feedList );
        m_folderExpansionHandler->setModel( m_subscriptionModel );
    }

    if ( m_feedSelector ) {
        Akonadi::EntityMimeTypeFilterModel* filterProxy = new Akonadi::EntityMimeTypeFilterModel( m_feedSelector );
        filterProxy->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
        filterProxy->setSourceModel( m_itemModel );
        m_feedSelector->setModel( filterProxy );
        connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                 this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
    }
}
#endif

void Akregator::SelectionController::setFolderExpansionHandler( Akregator::FolderExpansionHandler* handler )
{
#ifdef KRSS_PORT_DISABLED
    if ( handler == m_folderExpansionHandler )
        return;
    m_folderExpansionHandler = handler;
    if ( !m_folderExpansionHandler )
        return;
    handler->setFeedList( m_feedList );
    handler->setModel( m_subscriptionModel );
#endif
}

#ifdef KRSS_PORT_DISABLED
void Akregator::SelectionController::articleHeadersAvailable( KJob* job )
{
    assert( job );
    assert( job == m_listJob );


    if ( job->error() ) {
        kWarning() << job->errorText();
        return;
    }

    kDebug() << "article listing took (ms):" << m_time.elapsed() << " for items:" << m_listJob->items().count();

    m_itemListing.reset( new KRss::ItemListing( m_listJob->items(), m_listJob->fetchScope() ) );
    KRss::ConnectToItemListingVisitor visitor ( m_feedList, m_itemListing );
    selectedSubscription()->accept( &visitor );
    KRss::ItemModel* const newModel = new KRss::ItemModel( m_itemListing, this );

    m_articleLister->setIsAggregation( selectedSubscription()->tier() == KRss::TreeNode::TagTier );
    m_articleLister->setItemModel( newModel );
    delete m_itemModel; //order is important: do not delete the old model before the new model is set in the view
    m_itemModel = newModel;

    disconnect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this, SLOT(itemSelectionChanged()) );
    connect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
             this, SLOT(itemSelectionChanged()) );

    m_articleLister->setScrollBarPositions( m_scrollBarPositions.value( m_selectedSubscription ) );
}
#endif //KRSS_PORT_DISABLED


void Akregator::SelectionController::subscriptionContextMenuRequested( const QPoint& point )
{
#ifdef KRSS_PORT_DISABLED
    Q_ASSERT( m_feedSelector );
    const shared_ptr<const KRss::TreeNode> treeNode = ::subscriptionForIndex( m_feedSelector->indexAt( point ) );
    if ( !treeNode )
        return;

    QWidget* w = ActionManager::getInstance()->container( treeNode->tier() == KRss::TreeNode::TagTier ?
                                                          "feedgroup_popup" : "feeds_popup" );
    QMenu* popup = qobject_cast<QMenu*>( w );
    if ( popup ) {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal( point );
        popup->exec( globalPos );
    }
#endif
}

void Akregator::SelectionController::itemSelectionChanged()
{
    const Akonadi::Item item = m_articleLister->articleSelectionModel()->currentIndex().data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( !item.isValid() )
        return;
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload();
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();
    Akonadi::ItemFetchJob* job = new Akonadi::ItemFetchJob( item, m_session );
    job->setFetchScope( scope );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(fullItemFetched(KJob*)) );
    job->start();
}

void Akregator::SelectionController::fullItemFetched( KJob* j )
{
    Akonadi::ItemFetchJob* job = qobject_cast<Akonadi::ItemFetchJob*>( j );
    assert( job );
    if ( job->error() ) {
        //PENDING(frank) TODO handle error
        return;
    }

    Q_ASSERT( job->items().size() == 1 );

    const Akonadi::Item aitem = job->items().first();
    if ( !aitem.hasPayload<KRss::RssItem>() )
        return;
    const KRss::Item item( aitem );
    if ( m_singleDisplay )
        m_singleDisplay->showItem( item );

    emit currentItemChanged( item );
}

void Akregator::SelectionController::itemIndexDoubleClicked( const QModelIndex& index )
{
    const KRss::Item item = ::itemForIndex( index );
    emit itemDoubleClicked( item );
}

void SelectionController::setFilters( const std::vector<boost::shared_ptr<const Filters::AbstractMatcher> >& matchers )
{
    Q_ASSERT( m_articleLister );
    m_articleLister->setFilters( matchers );
}

void SelectionController::forceFilterUpdate()
{
    Q_ASSERT( m_articleLister );
    m_articleLister->forceFilterUpdate();
}

#include "selectioncontroller.moc"
