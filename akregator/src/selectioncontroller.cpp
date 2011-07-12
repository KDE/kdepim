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
#include <krss/feedlist.h>
#include <krss/itemjobs.h>
#include <krss/itemlisting.h>
#include <krss/itemlistjob.h>
#include <krss/itemmodel.h>
#include <krss/tagprovider.h>
#include <krss/treenode.h>
#include <krss/treenodevisitor.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/EntityTreeModel>
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
    return index.data( KRss::ItemModel::ItemRole ).value<KRss::Item>();
}

static QList<KRss::Item> itemsForIndexes( const QModelIndexList& indexes )
{
    QList<KRss::Item> items;
    Q_FOREACH ( const QModelIndex& i, indexes )
        items.append( itemForIndex( i ) );

    return items;
}
static shared_ptr<KRss::TreeNode> subscriptionForIndex( const QModelIndex& index )
{
#ifdef KRSS_PORT_DISABLED
    return index.data( FeedListModel::TreeNodeRole ).value<shared_ptr<KRss::TreeNode> >();
#endif
    return shared_ptr<KRss::TreeNode>();
}

Akregator::SelectionController::SelectionController( QObject* parent )
    : AbstractSelectionController( parent ),
    m_feedList(),
    m_feedSelector(),
    m_articleLister( 0 ),
    m_singleDisplay( 0 ),
    m_folderExpansionHandler( 0 ),
    m_itemModel( 0 ),
    m_selectedSubscription()
{
    Akonadi::ItemFetchScope scope;
    scope.fetchFullPayload( true );
    scope.fetchAttribute<Akonadi::EntityDisplayAttribute>();

    Akonadi::Session* session = new Akonadi::Session( QByteArray( "Akregator-" ) + QByteArray::number( qrand() ) );
    Akonadi::ChangeRecorder* recorder = new Akonadi::ChangeRecorder;
    recorder->setSession( session );
    recorder->fetchCollection( true );
    recorder->setItemFetchScope( scope );
    recorder->setCollectionMonitored( Akonadi::Collection::root() );
    recorder->setMimeTypeMonitored( QLatin1String( "application/rss+xml" ) );
    m_itemModel = new FeedItemModel( recorder );
}


void Akregator::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    if ( m_feedSelector == feedSelector )
        return;

    if ( m_feedSelector ) {
        m_feedSelector->disconnect( this );
        m_feedSelector->selectionModel()->disconnect( this );
    }

    m_feedSelector = feedSelector;

    init();
}

void Akregator::SelectionController::setArticleLister( Akregator::ArticleLister* lister )
{
    if ( m_articleLister == lister )
        return;

    if ( m_articleLister )
        m_articleLister->articleSelectionModel()->disconnect( this );
    if ( m_articleLister && m_articleLister->itemView() )
        m_articleLister->itemView()->disconnect( this );

    m_articleLister = lister;
    init();
}

void Akregator::SelectionController::init() {

    if (  !m_feedSelector || !m_articleLister || !m_articleLister->itemView() )
        return;

    Akonadi::EntityMimeTypeFilterModel* filterProxy = new Akonadi::EntityMimeTypeFilterModel( m_feedSelector );
    filterProxy->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
    filterProxy->setSourceModel( m_itemModel );

    m_feedSelector->setModel( filterProxy );

    connect( m_feedSelector, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(subscriptionContextMenuRequested(QPoint)) );
    connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
             this, SLOT(selectedSubscriptionChanged(QModelIndex)) );

    KSelectionProxyModel* selectionProxy = new KSelectionProxyModel( m_feedSelector->selectionModel() );
    selectionProxy->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
    selectionProxy->setSourceModel( m_itemModel );

    Akonadi::EntityMimeTypeFilterModel* filterProxy2 = new Akonadi::EntityMimeTypeFilterModel;
    filterProxy2->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
    filterProxy2->setSourceModel( selectionProxy );
    filterProxy2->setSortRole( FeedItemModel::SortRole );
    m_articleLister->setItemModel( filterProxy2 );

    connect( m_articleLister->itemView(), SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(itemIndexDoubleClicked(QModelIndex))  );
}

void Akregator::SelectionController::selectedSubscriptionChanged( const QModelIndex& ) {
    emit currentSubscriptionChanged( selectedSubscription() );
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

shared_ptr<KRss::TreeNode> Akregator::SelectionController::selectedSubscription() const
{
    return ::subscriptionForIndex( m_feedSelector->selectionModel()->currentIndex() );
}

void Akregator::SelectionController::setFeedList( const shared_ptr<KRss::FeedList>& feedList )
{
    if ( m_feedList == feedList )
        return;

    m_feedList = feedList;


#ifdef KRSS_PORT_DISABLED
    if ( m_folderExpansionHandler ) {
        m_folderExpansionHandler->setFeedList( m_feedList );
        m_folderExpansionHandler->setModel( m_subscriptionModel );
    }
#endif

    if ( m_feedSelector ) {
        Akonadi::EntityMimeTypeFilterModel* filterProxy = new Akonadi::EntityMimeTypeFilterModel( m_feedSelector );
        filterProxy->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
        filterProxy->setSourceModel( m_itemModel );
        m_feedSelector->setModel( filterProxy );
        disconnect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                    this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
        connect( m_feedSelector->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                 this, SLOT(selectedSubscriptionChanged(QModelIndex)) );
    }
}

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
    const KRss::Item item = currentItem();
    Akonadi::ItemFetchScope scope;
    scope.fetchPayloadPart( KRss::Item::HeadersPart );
    scope.fetchPayloadPart( KRss::Item::ContentPart );
    KRss::ItemFetchJob* job = new KRss::ItemFetchJob( this );
    job->setFetchScope( scope );
    job->setItem( item );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(fullItemFetched(KJob*)) );
    job->start();
}

void Akregator::SelectionController::fullItemFetched( KJob* j )
{
    KRss::ItemFetchJob* job = qobject_cast<KRss::ItemFetchJob*>( j );
    assert( job );
    if ( job->error() ) {
        //PENDING(frank) TODO handle error
    }

    const KRss::Item item = job->item();

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
