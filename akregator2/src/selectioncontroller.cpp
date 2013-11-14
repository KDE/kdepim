/*
    This file is part of Akregator2.

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

#include <krss/feedcollection.h>
#include <krss/feeditemmodel.h>
#include <krss/item.h>

#include <Akonadi/ChangeRecorder>
#include <Akonadi/CollectionFetchScope>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/EntityMimeTypeFilterModel>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <akonadi/selectionproxymodel.h>
#include <Akonadi/Session>

#include <KDebug>

#include <QAbstractItemView>
#include <QMenu>
#include <QtCore/QTimer>

#include <cassert>

using namespace boost;
using namespace Akregator2;
using namespace KRss;

TotalUnreadCountWatcher::TotalUnreadCountWatcher( QObject* parent )
    : QObject( parent )
    , m_unreadCount( 0 )
    , m_timer( new QTimer( this ) )
{
    m_timer->setInterval( 250 );
    m_timer->setSingleShot( true );
    connect( m_timer, SIGNAL(timeout()), this, SLOT(updateUnreadCount()) );
}

void TotalUnreadCountWatcher::setModel( QAbstractItemModel* model ) {
    if ( m_model )
        m_model->disconnect( this );

    m_model = model;

    if ( m_model )
        connect( m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)) );

    updateUnreadCount();
}

int TotalUnreadCountWatcher::unreadCount() const {
    return m_unreadCount;
}

void TotalUnreadCountWatcher::dataChanged( const QModelIndex&, const QModelIndex& ) {
    if ( !m_timer->isActive() )
        m_timer->start();
}

static int recursiveUnreadCount( const QModelIndex& index ) {
    const int childCount = index.model()->rowCount( index );
    int unread = index.data( Akonadi::EntityTreeModel::UnreadCountRole ).toInt();

    for ( int i = 0; i < childCount; ++i ) {
        const QModelIndex child = index.child( i, 0 );
        unread += recursiveUnreadCount( child );
    }

    return unread;
}

void TotalUnreadCountWatcher::updateUnreadCount() {
    int count = 0;
    if ( m_model ) {
        const int rowCount = m_model->rowCount();
        for ( int i = 0; i < rowCount; ++i ) {
            const QModelIndex idx = m_model->index( i, 0 );
            count += recursiveUnreadCount( idx );
        }
    }
    if ( m_unreadCount == count )
        return;
    m_unreadCount = count;
    emit unreadCountChanged( count );
}

static Akonadi::Item itemForIndex( const QModelIndex& index )
{
    return index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
}

static Akonadi::Item::List itemsForIndexes( const QModelIndexList& indexes )
{
    Akonadi::Item::List items;
    Q_FOREACH ( const QModelIndex& i, indexes )
        items.append( itemForIndex( i ) );
    return items;
}

Akregator2::SelectionController::SelectionController( Akonadi::Session* session, KRss::FeedItemModel *itemModel, QObject* parent )
    : AbstractSelectionController( parent ),
    m_feedSelector(),
    m_articleLister( 0 ),
    m_singleDisplay( 0 ),
    m_folderExpansionHandler( 0 ),
    m_itemModel( itemModel ),
    m_feedSelectionResolved( 0 ),
    m_session( session ),
    m_collectionFilterModel( 0 ),
    m_unreadWatcher( new TotalUnreadCountWatcher( this ) )
{
    Akonadi::EntityMimeTypeFilterModel* filterProxy = new Akonadi::EntityMimeTypeFilterModel( this );
    filterProxy->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
    filterProxy->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );
    filterProxy->setSupportedDragActions( Qt::MoveAction );
    filterProxy->setSourceModel( m_itemModel );
    filterProxy->setDynamicSortFilter( true );
    m_collectionFilterModel = filterProxy;

    connect( m_unreadWatcher, SIGNAL(unreadCountChanged(int)), this, SIGNAL(totalUnreadCountChanged(int)) );
    m_unreadWatcher->setModel( m_collectionFilterModel );
}


void Akregator2::SelectionController::setFeedSelector( QAbstractItemView* feedSelector )
{
    Q_ASSERT( !m_feedSelector );
    m_feedSelector = feedSelector;
    init();
}

void Akregator2::SelectionController::setArticleLister( Akregator2::ArticleLister* lister )
{
    Q_ASSERT( !m_articleLister );
    m_articleLister = lister;
    init();
}

void Akregator2::SelectionController::init() {
    Q_ASSERT( !m_feedSelectionResolved );
    if (  !m_feedSelector || !m_articleLister )
        return;

    Q_ASSERT( m_articleLister->itemView() );
    Q_ASSERT( !m_feedSelector->model() );


    connect( m_feedSelector, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(subscriptionContextMenuRequested(QPoint)) );

    m_feedSelector->setModel( m_collectionFilterModel );

    connect( m_feedSelector->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
             this, SLOT(feedSelectionChanged(QItemSelection,QItemSelection)) );

    m_feedSelectionResolved = new QItemSelectionModel( m_collectionFilterModel, this );
    Akonadi::SelectionProxyModel* selectionProxy = new Akonadi::SelectionProxyModel( m_feedSelectionResolved );
    selectionProxy->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );
    selectionProxy->setSourceModel( m_itemModel );

    Akonadi::EntityMimeTypeFilterModel* filterProxy2 = new Akonadi::EntityMimeTypeFilterModel;
    filterProxy2->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
    filterProxy2->addMimeTypeInclusionFilter( KRss::Item::mimeType() );
    filterProxy2->setSortRole( FeedItemModel::SortRole );
    filterProxy2->setDynamicSortFilter( true );
    filterProxy2->setSourceModel( selectionProxy );

    m_articleLister->setItemModel( filterProxy2 );
    connect( m_articleLister->articleSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                this, SLOT(itemSelectionChanged()) );
    connect( m_articleLister->itemView(), SIGNAL(doubleClicked(QModelIndex)),
             this, SLOT(itemIndexDoubleClicked(QModelIndex))  );
}

static QModelIndexList collectLeaves( const QModelIndex& idx ) {
    const int cc = idx.model()->rowCount( idx );
    if (  cc == 0 )
        return QModelIndexList() << idx;
    QModelIndexList l;
    for ( int i = 0; i < cc; ++i )
        l << collectLeaves( idx.child( i, 0 ) );
    return l;
}

void Akregator2::SelectionController::feedSelectionChanged ( const QItemSelection & selected, const QItemSelection & deselected )
{
    m_feedSelectionResolved->clear();
    const QModelIndexList sel = selected.indexes();
    if ( sel.isEmpty() ) {
        emit currentCollectionChanged( selectedCollection() );
        return;
    }
    Q_FOREACH( const QModelIndex& i, sel )
        Q_FOREACH( const QModelIndex& j, collectLeaves( i ) )
            m_feedSelectionResolved->select( j, QItemSelectionModel::Select|QItemSelectionModel::Rows );
    emit currentCollectionChanged( selectedCollection() );
}


void Akregator2::SelectionController::setSingleArticleDisplay( Akregator2::SingleArticleDisplay* display )
{
    m_singleDisplay = display;
}

Akonadi::Item Akregator2::SelectionController::currentItem() const
{
    return ::itemForIndex( m_articleLister->articleSelectionModel()->currentIndex() );
}

Akonadi::Item::List Akregator2::SelectionController::selectedItems() const
{
    return ::itemsForIndexes( m_articleLister->articleSelectionModel()->selectedRows() );
}

QModelIndex Akregator2::SelectionController::selectedCollectionIndex() const {
    return m_feedSelector->selectionModel()->currentIndex();
}

Akonadi::Collection::List SelectionController::resourceRootCollections() const {
    Akonadi::Collection::List l;
    const int rows = m_collectionFilterModel->rowCount();
    l.reserve( rows );
    for ( int i = 0; i < rows; ++i )
        l.push_back( m_collectionFilterModel->index( i, 0 ).data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>() );
    //PENDING(frank) filter out search folders etc.
    return l;
}

Akonadi::Collection Akregator2::SelectionController::selectedCollection() const
{
    return m_feedSelector->selectionModel()->currentIndex().data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
}

void Akregator2::SelectionController::setFolderExpansionHandler( Akregator2::FolderExpansionHandler* handler )
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



void Akregator2::SelectionController::subscriptionContextMenuRequested( const QPoint& point )
{
    Q_ASSERT( m_feedSelector );
    const QModelIndex index = m_feedSelector->indexAt( point );
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( !collection.isValid() )
        return;
    const bool isFolder = FeedCollection( collection ).isFolder();
    QWidget* w = ActionManager::getInstance()->container( isFolder ? "feedgroup_popup" : "feeds_popup" );
    QMenu* popup = qobject_cast<QMenu*>( w );
    if ( popup ) {
        const QPoint globalPos = m_feedSelector->viewport()->mapToGlobal( point );
        popup->exec( globalPos );
    }
}

void Akregator2::SelectionController::itemSelectionChanged()
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

void Akregator2::SelectionController::fullItemFetched( KJob* j )
{
    Akonadi::ItemFetchJob* job = qobject_cast<Akonadi::ItemFetchJob*>( j );
    assert( job );
    if ( job->error() ) {
        //PENDING(frank) TODO handle error
        kDebug() << job->errorString();
        return;
    }

    Q_ASSERT( job->items().size() == 1 );

    const Akonadi::Item aitem = job->items().first();
    if ( !aitem.hasPayload<KRss::Item>() )
        return;
    if ( m_singleDisplay )
        m_singleDisplay->showItem( aitem );

    emit currentItemChanged( aitem );
}

void Akregator2::SelectionController::itemIndexDoubleClicked( const QModelIndex& index )
{
    const Akonadi::Item item = ::itemForIndex( index );
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

