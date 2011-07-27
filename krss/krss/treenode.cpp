/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "treenode.h"
#include "feed.h"
#include "feedlist.h"
#include "itemlistjob.h"
#include "treenodevisitor.h"

using namespace KRss;
using namespace boost;

TreeNode::~TreeNode()
{
}

class RootNode::Private
{
public:
    Private() {}

    QList<shared_ptr<TagNode> > m_tagNodes;
};

RootNode::RootNode()
    : d( new Private )
{
}

RootNode::~RootNode()
{
    delete d;
}

TreeNode::Tier RootNode::tier() const
{
    return TreeNode::RootTier;
}

void RootNode::accept( TreeNodeVisitor *visitor )
{
    visitor->visit( static_pointer_cast<RootNode>( shared_from_this() ) );
}

void RootNode::accept( ConstTreeNodeVisitor *visitor ) const
{
    visitor->visit( static_pointer_cast<const RootNode>( shared_from_this() ) );
}

QList<shared_ptr<TagNode> > RootNode::tagNodes()
{
    return d->m_tagNodes;
}

QList<shared_ptr<const TagNode> > RootNode::tagNodes() const
{
    QList<shared_ptr<const TagNode> > constTagNodes;
    Q_FOREACH( const shared_ptr<const TagNode>& tagNode, d->m_tagNodes )
        constTagNodes.append( tagNode );

    return constTagNodes;
}

int RootNode::tagNodesCount() const
{
    return d->m_tagNodes.count();
}

shared_ptr<TagNode> RootNode::tagNodeAt( int row )
{
    return d->m_tagNodes.value( row );
}

shared_ptr<const TagNode> RootNode::tagNodeAt( int row ) const
{
    return d->m_tagNodes.value( row );
}

int RootNode::tagNodeRow( const shared_ptr<const TagNode>& tagNode ) const
{
    return tagNodes().indexOf( tagNode );
}

void RootNode::appendTagNode( const shared_ptr<TagNode>& tagNode )
{
    d->m_tagNodes.append( tagNode );
}

void RootNode::removeTagNodeAt( int row )
{
    d->m_tagNodes.removeAt( row );
}

void RootNode::removeTagNodes()
{
    d->m_tagNodes.clear();
}

ItemListJob* RootNode::createItemListJob( const shared_ptr<FeedList>& feedList ) const {
    //TODO: return All Feeds list job
    return 0;
}

QString RootNode::title( const shared_ptr<const FeedList>& feedList ) const {
    return QString();
}

int RootNode::unreadCount( const shared_ptr<const FeedList>& feedList ) const {
    int unread = 0;
    //TODO: return sum of all feeds, or all feeds tag node (but not sum of all tag nodes!)
    return unread;
}

int RootNode::totalCount( const shared_ptr<const FeedList>& feedList ) const {
    int total = 0;
    //TODO: return sum of all feeds, or all feeds tag node (but not sum of all tag nodes!)
    return total;
}

int RootNode::childCount() const {
    return tagNodesCount();
}


class TagNode::Private
{
public:
    Private( const shared_ptr<RootNode>& parent )
        : m_parent( parent ) {}

    const shared_ptr<RootNode> m_parent;
    Tag m_tag;
    QList<shared_ptr<FeedNode> > m_feedNodes;
};

TagNode::TagNode( const shared_ptr<RootNode>& parent )
    : d( new Private( parent ) )
{
}

TagNode::~TagNode()
{
    delete d;
}

TreeNode::Tier TagNode::tier() const
{
    return TreeNode::TagTier;
}

void TagNode::accept( TreeNodeVisitor *visitor )
{
    visitor->visit( static_pointer_cast<TagNode>( shared_from_this() ) );
}

void TagNode::accept( ConstTreeNodeVisitor *visitor ) const
{
    visitor->visit( static_pointer_cast<const TagNode>( shared_from_this() ) );
}

void TagNode::setTag( const Tag& tag )
{
    d->m_tag = tag;
}

Tag TagNode::tag() const
{
    return d->m_tag;
}

shared_ptr<RootNode> TagNode::parent()
{
    return d->m_parent;
}

shared_ptr<const RootNode> TagNode::parent() const
{
    return d->m_parent;
}

int TagNode::row() const
{
    const QList<shared_ptr<TagNode> > siblings = d->m_parent->tagNodes();
    const int size = siblings.count();
    for( int i = 0; i < size; ++i ) {
        if ( siblings.at( i ).get() == this )
            return i;
    }

    Q_ASSERT_X( false, "TagNode::row", "This tag node was not found among the parent's child nodes" );
    return 0;
}

//PENDING(frank) don't hardcode AllFeeds URI all over the place
bool TagNode::isDeletable() const {
    return d->m_tag.id() != TagId("http://akregator.kde.org/defaultTags/AllFeeds");
}

bool TagNode::taggedFeedsUserEditable() const {
    return d->m_tag.id() != TagId("http://akregator.kde.org/defaultTags/AllFeeds");
}

int TagNode::feedNodesCount() const
{
    return d->m_feedNodes.count();
}

QList<shared_ptr<FeedNode> > TagNode::feedNodes()
{
    return d->m_feedNodes;
}

QList<shared_ptr<const FeedNode> > TagNode::feedNodes() const
{
    QList<shared_ptr<const FeedNode> > constFeedNodes;
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, d->m_feedNodes )
        constFeedNodes.append( feedNode );

    return constFeedNodes;
}

shared_ptr<FeedNode> TagNode::feedNodeAt( int row )
{
    return d->m_feedNodes.value( row );
}

shared_ptr<const FeedNode> TagNode::feedNodeAt( int row ) const
{
    return d->m_feedNodes.value( row );
}

int TagNode::feedNodeRow( const shared_ptr<const FeedNode>& feedNode ) const
{
    return feedNodes().indexOf( feedNode );
}

void TagNode::appendFeedNode( const shared_ptr<FeedNode>& feedNode )
{
    d->m_feedNodes.append( feedNode );
}

void TagNode::removeFeedNodeAt( int row )
{
    d->m_feedNodes.removeAt( row );
}

void TagNode::removeFeedNodes()
{
    d->m_feedNodes.clear();
}

ItemListJob* TagNode::createItemListJob( const shared_ptr<FeedList>& feedList ) const {
    CompositeItemListJob* job = new CompositeItemListJob;
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes() ) {
        ItemListJob* const sj = feedNode->createItemListJob( feedList );
        if ( sj )
            job->addSubJob( sj );
    }
    return job;
}

QString TagNode::title( const shared_ptr<const FeedList>& ) const {
    return d->m_tag.label();
}

int TagNode::unreadCount( const shared_ptr<const FeedList>& feedList ) const {
    int sum = 0;
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes() )
            sum += feedNode->unreadCount( feedList );
    return sum;
}

int TagNode::totalCount( const shared_ptr<const FeedList>& feedList ) const {
    int sum = 0;
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes() )
            sum += feedNode->totalCount( feedList );
    return sum;
}

int TagNode::childCount() const {
    return feedNodesCount();
}


class FeedNode::Private
{
public:
    Private( const shared_ptr<TagNode>& parent )
        : m_parent( parent ) {}

    const shared_ptr<TagNode> m_parent;
    Feed::Id m_id;

    shared_ptr<const Feed> feed( const shared_ptr<const FeedList>& fl ) const {
        return fl ? fl->constFeedById( m_id ) : shared_ptr<const Feed>();
    }

    const shared_ptr<Feed> feed( const shared_ptr<FeedList>& fl ) const {
        return fl ? fl->feedById( m_id ) : shared_ptr<Feed>();
    }

};

FeedNode::FeedNode( const shared_ptr<TagNode>& parent )
    : d( new Private( parent ) )
{
}

FeedNode::~FeedNode()
{
    delete d;
}

TreeNode::Tier FeedNode::tier() const
{
    return TreeNode::FeedTier;
}

void FeedNode::accept( TreeNodeVisitor *visitor )
{
    visitor->visit( static_pointer_cast<FeedNode>( shared_from_this() ) );
}

void FeedNode::accept( ConstTreeNodeVisitor *visitor ) const
{
    visitor->visit( static_pointer_cast<const FeedNode>( shared_from_this() ) );
}

void FeedNode::setFeedId( const Feed::Id& id )
{
    d->m_id = id;
}

Feed::Id FeedNode::feedId() const
{
    return d->m_id;
}

shared_ptr<TagNode> FeedNode::parent()
{
    return d->m_parent;
}

shared_ptr<const TagNode> FeedNode::parent() const
{
    return d->m_parent;
}

int FeedNode::row() const
{
    const QList<shared_ptr<FeedNode> > siblings = d->m_parent->feedNodes();
    const int size = siblings.count();
    for( int i = 0; i < size; ++i ) {
        if ( siblings.at( i ).get() == this )
            return i;
    }

    Q_ASSERT_X( false, "FeedNode::row", "This feed node was not found among the parent's child nodes" );
    return 0;
}


ItemListJob* FeedNode::createItemListJob( const shared_ptr<FeedList>& feedList ) const {
    const shared_ptr<Feed> f = d->feed( feedList );
    return f ? f->itemListJob() : 0;
}

QString FeedNode::title( const shared_ptr<const FeedList>& feedList ) const {
    const shared_ptr<const Feed> f = d->feed( feedList );
    return f ? f->title() : QString();
}

int FeedNode::unreadCount( const shared_ptr<const FeedList>& feedList ) const {
    const shared_ptr<const Feed> f = d->feed( feedList );
    return f ? f->unread() : 0;
}

int FeedNode::totalCount( const shared_ptr<const FeedList>& feedList ) const {
    const shared_ptr<const Feed> f = d->feed( feedList );
    return f ? f->total() : 0;
}

int FeedNode::childCount() const {
    return 0;
}
