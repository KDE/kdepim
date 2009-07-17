/*
    Copyright (C) 2008,2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "feedlistmodel.h"
#include "tag.h"
#include "tagprovider.h"
#include "treenode.h"
#include "treenodevisitor.h"
#include "feedlist.h"
#include "feedjobs.h"

#include <KLocale>
#include <KIcon>
#include <KIconLoader>

#include <QtCore/QPointer>
#include <QtCore/QMimeData>
#include <QtCore/QHashIterator>

#include <QtGui/QPixmap>

#include <cassert>

using namespace KRss;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

const char KRSS_TREENODE_MIMETYPE[] = "krss/treenode-id";

namespace KRss {

class FeedListModelPrivate
{
public:
    FeedListModelPrivate( const shared_ptr<const FeedList>& feedList,
                          const shared_ptr<const TagProvider>& tagProvider,
                          FeedListModel * const qq );

    shared_ptr<TagNode> addTagNode( const Tag& tag );
    void addFeed( const Feed::Id& feedId );
    void addFeedNode( const Feed::Id& feedId, const TagId& tagId );
    void removeFeedNode( const shared_ptr<FeedNode>& feedNode );
    QList<TagId> lookupTags( const Feed::Id& feedId ) const;
    shared_ptr<FeedNode> lookupFeedNode( const Feed::Id& feedId, const TagId &tagId ) const;
    shared_ptr<TreeNode> lookupTreeNode( const TreeNode *treeNode ) const;
    QModelIndex createIndex( int row, int column, const boost::shared_ptr<TreeNode>& treeNode ) const;

    void slotTagCreated( const Tag& tag );
    void slotTagModified( const Tag& tag );
    void slotTagDeleted( const TagId& id );
    void slotFeedAdded( const Feed::Id& feedId );
    void slotFeedChanged( const Feed::Id& feedId );
    void slotFeedRemoved( const Feed::Id& feedId );
    void slotFeedItemCountChanged( const Feed::Id& feedId, int count );
    void reset();

public:
    FeedListModel * const q;

    shared_ptr<const FeedList> m_feedList;
    const shared_ptr<const TagProvider> m_tagProvider;
    Tag m_allFeedsTag;

    shared_ptr<RootNode> m_rootNode;
    QHash<Feed::Id, QList<shared_ptr<FeedNode> > > m_feedNodes;
    QHash<TagId, shared_ptr<TagNode> > m_tagNodes;
};

} // namespace KRss

namespace {
    class SignalBlocker
    {
    public:
        explicit SignalBlocker( QObject* o ) : object( o ), wasBlocked( o->blockSignals( true ) ) {}
        ~SignalBlocker() { if ( object ) object->blockSignals( wasBlocked ); }
    private:
        const QPointer<QObject> object;
        const bool wasBlocked;
    };

    class GetIconVisitor : public ConstTreeNodeVisitor
    {
    public:
        GetIconVisitor( const shared_ptr<const FeedList> feedList )
            : m_feedList( feedList ) {}

        void visit( const boost::shared_ptr<const RootNode>& ) {
        }

        void visit( const boost::shared_ptr<const TagNode>& tagNode ) {
            m_icon = KIcon( "folder" ).pixmap( KIconLoader::SizeSmall );
        }

        void visit( const boost::shared_ptr<const FeedNode>& feedNode ) {
            const boost::shared_ptr<const Feed> feed = m_feedList->constFeedById( feedNode->feedId() );
            m_icon = feed->icon().pixmap( KIconLoader::SizeSmall, feed->isFetching() ? QIcon::Active : QIcon::Normal );
        }

        const shared_ptr<const FeedList> m_feedList;
        QPixmap m_icon;
    };

    class CreateChildIndexVisitor : public TreeNodeVisitor
    {
    public:
        CreateChildIndexVisitor( const FeedListModelPrivate *model, int row, int column )
            : m_model( model ), m_row( row ), m_column( column ) {}

        QModelIndex childIndex() const { return m_childIndex; }

        void visit( const boost::shared_ptr<RootNode>& rootNode ) {
            Q_UNUSED( rootNode )
            Q_ASSERT_X( false, "CreateChildIndexVisitor::visit()", "Called for the root node" );
        }

        void visit( const boost::shared_ptr<TagNode>& tagNode ) {
            m_childIndex = ( m_row < tagNode->feedNodesCount() ?
                             m_model->createIndex( m_row, m_column, tagNode->feedNodeAt( m_row ) ) : QModelIndex() );
        }

        void visit( const boost::shared_ptr<FeedNode>& feedNode ) {
            Q_UNUSED( feedNode )
            Q_ASSERT_X( false, "CreateChildIndexVisitor::visit()", "Called for the root node" );
        }

        private:
            const FeedListModelPrivate* const m_model;
            const int m_row;
            const int m_column;
            QModelIndex m_childIndex;
    };

    class CreateParentIndexVisitor : public TreeNodeVisitor
    {
    public:
        CreateParentIndexVisitor( const FeedListModelPrivate *model )
            : m_model( model ) {}

        QModelIndex parentIndex() const { return m_parentIndex; }

        void visit( const boost::shared_ptr<RootNode>& rootNode ) {
            Q_UNUSED( rootNode )
            Q_ASSERT_X( false, "CreateParentIndexVisitor::visit()", "Called for the root node" );
        }

        void visit( const boost::shared_ptr<TagNode>& tagNode ) {
            Q_UNUSED( tagNode )
            m_parentIndex = QModelIndex();
        }

        void visit( const boost::shared_ptr<FeedNode>& feedNode ) {
            const shared_ptr<TagNode> tagNode = feedNode->parent();
            m_parentIndex = m_model->createIndex( tagNode->row(), 0, tagNode );
        }

        private:
            const FeedListModelPrivate* const m_model;
            QModelIndex m_parentIndex;
    };
}


// private implementation
FeedListModelPrivate::FeedListModelPrivate( const shared_ptr<const FeedList>& feedList,
                                            const shared_ptr<const TagProvider>& tagProvider,
                                            FeedListModel * const qq )
    : q( qq ), m_feedList( feedList ), m_tagProvider( tagProvider )
{
    // construct the root node
    m_rootNode = shared_ptr<RootNode>( new RootNode );

    // construct the "All feeds" node
    m_allFeedsTag = Tag( TagId( "http://akregator.kde.org/defaultTags/AllFeeds" ) );
    m_allFeedsTag.setLabel( i18n( "All Feeds" ) );
    m_allFeedsTag.setDescription( i18n( "Contains all your feeds" ) );
    addTagNode( m_allFeedsTag );

    // walk over the TagProvider and construct tag nodes for all tags
    const QList<Tag> allTags = m_tagProvider->tags().values();
    Q_FOREACH( const Tag& tag, allTags ) {
        addTagNode( tag );
    }
}

shared_ptr<TagNode> FeedListModelPrivate::addTagNode( const Tag& tag )
{
    const shared_ptr<TagNode> tagNode( new TagNode( m_rootNode ) );
    tagNode->setTag( tag );

    m_rootNode->appendTagNode( tagNode );
    m_tagNodes.insert( tag.id(), tagNode );
    return tagNode;
}

void FeedListModelPrivate::addFeed( const Feed::Id& feedId ) {
    const shared_ptr<const Feed> feed = m_feedList->constFeedById( feedId );
    const QList<TagId> feedTags = feed->tags() << m_allFeedsTag.id();

    Q_FOREACH( const TagId &tag, feedTags )
        addFeedNode( feedId, tag );
}

void FeedListModelPrivate::addFeedNode( const Feed::Id& feedId, const TagId& tagId )
{
    if ( !m_tagNodes.contains( tagId ) )
        return;

    const shared_ptr<TagNode> tagNode = m_tagNodes.value( tagId );

    // create the feed node
    QModelIndex tagIndex = q->createIndex( tagNode->row(), 0, tagNode.get() );
    const int feedrow = tagNode->feedNodesCount();
    q->beginInsertRows( tagIndex, feedrow, feedrow );
    shared_ptr<FeedNode> feedNode( new FeedNode( tagNode ) );
    feedNode->setFeedId( feedId );
    tagNode->appendFeedNode( feedNode );
    m_feedNodes[ feedId ].append( feedNode );
    q->endInsertRows();
}

void FeedListModelPrivate::removeFeedNode( const shared_ptr<FeedNode>& feedNode )
{
    const Feed::Id feedId = feedNode->feedId();

    // remove the feed node
    const shared_ptr<TagNode> tagNode = feedNode->parent();
    Q_ASSERT( tagNode );

    const int tagrow = tagNode->row();
    QModelIndex tagIndex = q->createIndex( tagrow, 0, tagNode.get() );
    const int feedrow = feedNode->row();
    q->beginRemoveRows( tagIndex, feedrow, feedrow );
    m_feedNodes[ feedId ].removeOne( feedNode );
    tagNode->removeFeedNodeAt( feedrow );
    q->endRemoveRows();
}

QList<TagId> FeedListModelPrivate::lookupTags( const Feed::Id& feedId ) const
{
    const QList<shared_ptr<FeedNode> > feedNodes = m_feedNodes.value( feedId );
    QList<TagId> tags;
    Q_FOREACH( const shared_ptr<FeedNode>& feedNode, feedNodes ) {
        const shared_ptr<TagNode> tagNode = feedNode->parent();
        Q_ASSERT( tagNode );
        const Tag tag = tagNode->tag();
        tags.append( tag.id() );
    }

    return tags;
}

shared_ptr<FeedNode> FeedListModelPrivate::lookupFeedNode( const Feed::Id& feedId, const TagId& tagId ) const
{
    const QList<shared_ptr<FeedNode> > feedNodes = m_feedNodes.value( feedId );
    Q_FOREACH( const shared_ptr<FeedNode>& feedNode, feedNodes ) {
        if ( feedNode->parent()->tag().id() == tagId )
            return feedNode;
    }

    Q_ASSERT_X( false, "FeedListModelPrivate::lookupFeedNode", "No such feed node" );
    return shared_ptr<FeedNode>();
}

shared_ptr<TreeNode> FeedListModelPrivate::lookupTreeNode( const TreeNode *treeNode ) const
{
    switch( treeNode->tier() ) {
        case TreeNode::RootTier:
            return m_rootNode;
        case TreeNode::TagTier:
        {
            const TagNode * const tagNodePtr = dynamic_cast<const TagNode*>( treeNode );
            Q_ASSERT( tagNodePtr );
            const shared_ptr<TreeNode> tagNode = m_tagNodes.value( tagNodePtr->tag().id() );
            Q_ASSERT( tagNode );
            return tagNode;
        }
        case TreeNode::FeedTier:
        {
            const FeedNode* const feedNodePtr = dynamic_cast<const FeedNode*>( treeNode );
            Q_ASSERT( feedNodePtr );
            const shared_ptr<TreeNode> feedNode = lookupFeedNode( feedNodePtr->feedId(),
                                                                  feedNodePtr->parent()->tag().id() );
            Q_ASSERT( feedNode );
            return feedNode;
        }
        default:
            Q_ASSERT_X( false, "FeedListModelPrivate::lookupTreeNode", "Unknown node tier" );
    }

    return shared_ptr<TreeNode>();
}

QModelIndex FeedListModelPrivate::createIndex( int row, int column, const shared_ptr<TreeNode>& treeNode ) const
{
    return q->createIndex( row, column, treeNode.get() );
}

void FeedListModelPrivate::reset()
{
    // we block signals and then call reset() to update
    // the model in one sitting
    if ( m_feedList ) {
        const SignalBlocker blocker( q );
        Q_FOREACH( const Feed::Id& feedId, m_feedList->feedIds() )
            addFeed( feedId );
    }
    q->reset();
}

void FeedListModelPrivate::slotTagCreated( const KRss::Tag& tag )
{
    const shared_ptr<TagNode> tagNode = addTagNode( tag );
    const int row = tagNode->row();
    q->beginInsertRows( QModelIndex(), row, row );
    q->endInsertRows();
}

void FeedListModelPrivate::slotTagModified( const KRss::Tag& tag )
{
    Q_ASSERT( m_tagNodes.contains( tag.id() ) );

    const shared_ptr<TagNode> tagNode = m_tagNodes.value( tag.id() );
    const QModelIndex tagIndex = q->createIndex( tagNode->row(), 0, tagNode.get() );
    tagNode->setTag( tag );
    emit q->dataChanged( tagIndex, tagIndex );
}

void FeedListModelPrivate::slotTagDeleted( const KRss::TagId& id )
{
    Q_ASSERT( m_tagNodes.value( id )->feedNodesCount() == 0 );

    const shared_ptr<TagNode> tagNode = m_tagNodes.value( id );
    const int row = tagNode->row();
    q->beginRemoveRows( QModelIndex(), row, row );
    m_tagNodes.remove( id );
    m_rootNode->removeTagNodeAt( row );
    q->endRemoveRows();
}

void FeedListModelPrivate::slotFeedAdded( const Feed::Id& feedId )
{
    addFeed( feedId );
}

void FeedListModelPrivate::slotFeedChanged( const Feed::Id& feedId )
{
    const shared_ptr<const Feed> feed = m_feedList->feedById( feedId );
    const QList<TagId> newTags = feed->tags() << m_allFeedsTag.id();

    const QList<TagId> oldTags = lookupTags( feedId );
    const QSet<TagId> common = oldTags.toSet().intersect( newTags.toSet() );
    const QSet<TagId> toRemove = oldTags.toSet().subtract( newTags.toSet() );
    const QSet<TagId> toAdd = newTags.toSet().subtract( oldTags.toSet() );

    // updated the feed nodes under the 'common' tags
    Q_FOREACH( const TagId &tag, common ) {
        const shared_ptr<FeedNode> feedNode = lookupFeedNode( feedId, tag );
        const QModelIndex feedIndex = q->createIndex( feedNode->row(), 0, feedNode.get() );
        emit q->dataChanged( feedIndex, feedIndex );
    }

    // remove the feed nodes from the 'toRemove' tags
    Q_FOREACH( const TagId &tag, toRemove ) {
        const shared_ptr<FeedNode> feedNode = lookupFeedNode( feedId, tag );
        removeFeedNode( feedNode );
    }

    // add new feed nodes under the 'toAdd' tags
    Q_FOREACH( const TagId &tag, toAdd ) {
        addFeedNode( feedId, tag );
    }
}

void FeedListModelPrivate::slotFeedItemCountChanged( const Feed::Id& feedId, int ) {
    //TODO: make this more efficient
    slotFeedChanged( feedId );
}

void FeedListModelPrivate::slotFeedRemoved( const Feed::Id& feedId )
{
    const QList<shared_ptr<FeedNode> > feedNodes = m_feedNodes.value( feedId );
    Q_FOREACH( const shared_ptr<FeedNode> feedNode, feedNodes ) {
        removeFeedNode( feedNode );
    }
}

// public interface implementation
FeedListModel::FeedListModel( const shared_ptr<const FeedList>& feedList,
                              const shared_ptr<const TagProvider>& tagProvider,
                              QObject *parent )
    : QAbstractItemModel( parent ), d( new FeedListModelPrivate( feedList, tagProvider, this ) )
{
    if ( d->m_feedList ) {
        connect( d->m_feedList.get(), SIGNAL( feedAdded( const KRss::Feed::Id& ) ),
                    this, SLOT( slotFeedAdded( const KRss::Feed::Id& ) ) );
        connect( d->m_feedList.get(), SIGNAL( feedChanged( const KRss::Feed::Id& ) ),
                    this, SLOT( slotFeedChanged( const KRss::Feed::Id& ) ) );
        connect( d->m_feedList.get(), SIGNAL( feedRemoved( const KRss::Feed::Id& ) ),
                    this, SLOT( slotFeedRemoved( const KRss::Feed::Id& ) ) );
        connect( d->m_feedList.get(), SIGNAL( unreadCountChanged( const KRss::Feed::Id&, int ) ),
                    this, SLOT( slotFeedItemCountChanged( const KRss::Feed::Id&, int ) ) );
        connect( d->m_feedList.get(), SIGNAL( totalCountChanged( const KRss::Feed::Id&, int ) ),
                    this, SLOT( slotFeedItemCountChanged( const KRss::Feed::Id&, int ) ) );

        Q_FOREACH( const Feed::Id& feedId, d->m_feedList->feedIds() )
            d->addFeed( feedId );
    }

    connect( d->m_tagProvider.get(), SIGNAL( tagCreated( const KRss::Tag& ) ),
             this, SLOT( slotTagCreated( const KRss::Tag& ) ) );
    connect( d->m_tagProvider.get(), SIGNAL( tagModified( const KRss::Tag& ) ),
             this, SLOT( slotTagModified( const KRss::Tag& ) ) );
    connect( d->m_tagProvider.get(), SIGNAL( tagDeleted( const KRss::TagId& ) ),
             this, SLOT( slotTagDeleted( const KRss::TagId& ) ) );
}

FeedListModel::~FeedListModel()
{
    delete d;
}

shared_ptr<const FeedList> FeedListModel::feedList() const
{
    return d->m_feedList;
}

QVariant FeedListModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();

    switch ( section ) {
        case TitleColumn: return i18n( "Feeds" );
        case UnreadCountColumn: return i18n( "Unread" );
        case TotalCountColumn: return i18n( "Total" );
        default:
            break;
    }

    return QVariant();
}

int FeedListModel::columnCount( const QModelIndex &parent ) const
{
    Q_UNUSED( parent );
    return ColumnCount;
}

int FeedListModel::rowCount( const QModelIndex &parent ) const
{
    if ( !parent.isValid() )
        return d->m_rootNode->tagNodesCount();

    const TreeNode* const node = static_cast<const TreeNode*>( parent.internalPointer() );
    const shared_ptr<const TreeNode> treeNode = d->lookupTreeNode( node );
    return treeNode->childCount();
}

QVariant FeedListModel::data(const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    const TreeNode* const node = static_cast<const TreeNode*>( index.internalPointer() );
    const shared_ptr<TreeNode> treeNode = d->lookupTreeNode( node );

    switch ( role ) {
        case Qt::EditRole:
        case Qt::DisplayRole:
        {
            switch ( index.column() )
            {
                case TitleColumn:
                    return treeNode->title( d->m_feedList );
                case UnreadCountColumn:
                    return treeNode->unreadCount( d->m_feedList );
                case TotalCountColumn:
                    return treeNode->totalCount( d->m_feedList );
            }
            break;
        }
        case Qt::DecorationRole:
        {
            if ( index.column() != TitleColumn )
                return QVariant();

            GetIconVisitor visitor( d->m_feedList );
            treeNode->accept( &visitor );
            return visitor.m_icon;
        }
        case Qt::ToolTipRole:   return treeNode->title( d->m_feedList );
        case HasUnreadRole:     return treeNode->unreadCount( d->m_feedList ) > 0;
        case TreeNodeRole:      return QVariant::fromValue( treeNode );
        case IsTagRole:         return treeNode->tier() == TreeNode::TagTier;

        default:                return QVariant();
    }

    return QVariant();
}

QModelIndex FeedListModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( row < 0 || column < 0 || column > ColumnCount )
        return QModelIndex();

    // create index for a tag node
    if ( !parent.isValid() ) {
        if ( row < d->m_rootNode->tagNodesCount() )
            return createIndex( row, column, d->m_rootNode->tagNodeAt( row ).get() );
        else
            return QModelIndex();
    }

    // node is a Tag node, CreateChildIndexVisitor checks this
    const TreeNode* const node = static_cast<const TreeNode*>( parent.internalPointer() );
    const shared_ptr<TreeNode> treeNode = d->lookupTreeNode( node );
    CreateChildIndexVisitor visitor( d, row, column );
    treeNode->accept( &visitor );
    return visitor.childIndex();
}

QModelIndex FeedListModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    // node is either a Tag node or Feed node, CreateParentIndexVisitor checks this
    const TreeNode* const node = static_cast<const TreeNode*>( index.internalPointer() );
    const shared_ptr<TreeNode> treeNode = d->lookupTreeNode( node );
    CreateParentIndexVisitor visitor( d );
    treeNode->accept( &visitor );
    return visitor.parentIndex();
}

Qt::ItemFlags FeedListModel::flags( const QModelIndex& index ) const
{
    const Qt::ItemFlags flags = QAbstractItemModel::flags( index );
    if ( !index.isValid() || ( index.column() != TitleColumn ) )
        return flags;

    const shared_ptr<const TreeNode> treeNode = FeedListModel::data( index, FeedListModel::TreeNodeRole )
                                                                    .value<shared_ptr<TreeNode> >();
    if ( treeNode->tier() == TreeNode::TagTier )
        return flags | Qt::ItemIsDropEnabled;
    else
        return flags | Qt::ItemIsDragEnabled;
}

QStringList FeedListModel::mimeTypes() const
{
    QStringList types;
    types << "text/uri-list" << KRSS_TREENODE_MIMETYPE;
    return types;
}

QMimeData* FeedListModel::mimeData( const QModelIndexList& indexes ) const
{
    QMimeData* mimeData = new QMimeData;
    QList<QUrl> urls;
    QByteArray idList;
    QDataStream idStream( &idList, QIODevice::WriteOnly );

    Q_FOREACH ( const QModelIndex& index, indexes ) {
        const shared_ptr<const TreeNode> treeNode = FeedListModel::data( index, FeedListModel::TreeNodeRole )
                                                                       .value<shared_ptr<TreeNode> >();
        switch( treeNode->tier() ) {
            case TreeNode::FeedTier:
            {
                const shared_ptr<const FeedNode> feedNode = dynamic_pointer_cast<const FeedNode, const TreeNode>( treeNode );
                // TODO: figure out how to handle virtual search feeds
                //urls.append( d->m_feedList->constFeedById( feedNode->feedId() )->xmlUrl() );
                idStream << feedNode->feedId();
                break;
            }
            default:
                Q_ASSERT_X( false, "FeedListModel::mimeData()", "Default path reached" );
        }
    }

    mimeData->setUrls( urls );
    mimeData->setData( KRSS_TREENODE_MIMETYPE, idList );
    return mimeData;
}

bool FeedListModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column,
                                  const QModelIndex& parent )
{
    // TODO: support rearranging order of the tags and feeds
    Q_UNUSED( column )
    Q_UNUSED( row )

    if ( action == Qt::IgnoreAction )
        return true;

    if ( !data->hasFormat( KRSS_TREENODE_MIMETYPE ) )
        return false;

    const shared_ptr<const TreeNode> droppedOnNode = FeedListModel::data( parent, FeedListModel::TreeNodeRole )
                                                                        .value<shared_ptr<TreeNode> >();
    if ( !droppedOnNode )
        return false;

    if ( droppedOnNode->tier() != TreeNode::TagTier )
        return false;

    const shared_ptr<const TagNode> droppedOnTag = dynamic_pointer_cast<const TagNode, const TreeNode>( droppedOnNode );
    assert( droppedOnTag );
    if ( droppedOnTag->tag() == d->m_allFeedsTag )
        return false;

    QByteArray idData = data->data( KRSS_TREENODE_MIMETYPE );
    QDataStream stream( &idData, QIODevice::ReadOnly );
    while ( !stream.atEnd() ) {
        KRss::Feed::Id id;
        stream >> id;
        const shared_ptr<Feed> feed = d->m_feedList->feedById( id );
        Q_ASSERT( feed );
        feed->addTag( droppedOnTag->tag().id() );
        // TODO: figure out how to pass the error message up to the main window
        FeedModifyJob * const job = new FeedModifyJob( feed );
        job->start();
    }

    return true;
}

QList<QModelIndex> FeedListModel::feedIndexes( const Feed::Id& feedId ) const
{
    QList<QModelIndex> feedIndexesList;

    QList<shared_ptr<FeedNode> > feedNodes = d->m_feedNodes.value( feedId );
    Q_FOREACH( const shared_ptr<FeedNode> feedNode, feedNodes ) {
        feedIndexesList.append( createIndex ( feedNode->row(), 0, feedNode.get() ) );
    }

    return feedIndexesList;
}

#include "feedlistmodel.moc"
