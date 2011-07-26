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

#ifndef KRSS_TREENODE_H
#define KRSS_TREENODE_H

#include "krss_export.h"
#include "tag.h"
#include "feed.h"

#include <QtCore/QString>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

template<class T> class QList;

namespace Akonadi {
class ItemFetchScope;
}

namespace KRss {

class Feed;
class FeedList;
class ItemListJob;
class RootNode;
class TagNode;
class FeedNode;
class TreeNodeVisitor;
class ConstTreeNodeVisitor;

class KRSS_EXPORT TreeNode
{
public:
    enum Tier {
        RootTier,
        TagTier,
        FeedTier
    };

    virtual ~TreeNode();

    virtual Tier tier() const = 0;
    virtual void accept( TreeNodeVisitor *visitor ) = 0;
    virtual void accept( ConstTreeNodeVisitor *visitor ) const = 0;

    virtual ItemListJob* createItemListJob( const boost::shared_ptr<FeedList>& feedList ) const = 0;
    virtual QString title( const boost::shared_ptr<const FeedList>& feedList ) const = 0;
    virtual int unreadCount( const boost::shared_ptr<const FeedList>& feedList ) const = 0;
    virtual int totalCount( const boost::shared_ptr<const FeedList>& feedList ) const = 0;
    virtual int childCount() const = 0;
};

class KRSS_EXPORT RootNode : public TreeNode, public boost::enable_shared_from_this<RootNode>
{
public:
    RootNode();
    ~RootNode();

    TreeNode::Tier tier() const;
    void accept( TreeNodeVisitor *visitor );
    void accept( ConstTreeNodeVisitor *visitor ) const;

    int tagNodesCount() const;
    QList<boost::shared_ptr<TagNode> > tagNodes();
    QList<boost::shared_ptr<const TagNode> > tagNodes() const;
    boost::shared_ptr<TagNode> tagNodeAt( int row );
    boost::shared_ptr<const TagNode> tagNodeAt( int row ) const;
    int tagNodeRow( const boost::shared_ptr<const TagNode>& tagNode ) const;
    void appendTagNode( const boost::shared_ptr<TagNode>& tagNode );
    void removeTagNodeAt( int row );
    void removeTagNodes();


    /* reimp */ ItemListJob* createItemListJob( const boost::shared_ptr<FeedList>& feedList ) const;
    /* reimp */ QString title( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int unreadCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int totalCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int childCount() const;

private:
    class Private;
    Private * const d;
};

class KRSS_EXPORT TagNode : public TreeNode, public boost::enable_shared_from_this<TagNode>
{
public:
    explicit TagNode( const boost::shared_ptr<RootNode>& parent );
    ~TagNode();

    TreeNode::Tier tier() const;
    void accept( TreeNodeVisitor *visitor );
    void accept( ConstTreeNodeVisitor *visitor ) const;

    void setTag( const Tag& tag );
    Tag tag() const;

    bool isDeletable() const;
    bool taggedFeedsUserEditable() const;

    boost::shared_ptr<RootNode> parent();
    boost::shared_ptr<const RootNode> parent() const;
    int row() const;
    int feedNodesCount() const;
    QList<boost::shared_ptr<FeedNode> > feedNodes();
    QList<boost::shared_ptr<const FeedNode> > feedNodes() const;
    boost::shared_ptr<FeedNode> feedNodeAt( int row );
    boost::shared_ptr<const FeedNode> feedNodeAt( int row ) const;
    int feedNodeRow( const boost::shared_ptr<const FeedNode>& feedNode ) const;
    void appendFeedNode( const boost::shared_ptr<FeedNode>& feedNode );
    void removeFeedNodeAt( int row );
    void removeFeedNodes();

    /* reimp */ ItemListJob* createItemListJob( const boost::shared_ptr<FeedList>& feedList ) const;
    /* reimp */ QString title( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int unreadCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int totalCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int childCount() const;

private:
    class Private;
    Private * const d;
};

class KRSS_EXPORT FeedNode : public TreeNode, public boost::enable_shared_from_this<FeedNode>
{
public:
    explicit FeedNode( const boost::shared_ptr<TagNode>& parent );
    ~FeedNode();

    TreeNode::Tier tier() const;
    void accept( TreeNodeVisitor *visitor );
    void accept( ConstTreeNodeVisitor *visitor ) const;

    Feed::Id feedId() const;
    void setFeedId( const Feed::Id& id );

    boost::shared_ptr<TagNode> parent();
    boost::shared_ptr<const TagNode> parent() const;
    int row() const;

    /* reimp */ ItemListJob* createItemListJob( const boost::shared_ptr<FeedList>& feedList ) const;
    /* reimp */ QString title( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int unreadCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int totalCount( const boost::shared_ptr<const FeedList>& feedList ) const;
    /* reimp */ int childCount() const;

private:
    class Private;
    Private * const d;
};

} // namespace KRss

Q_DECLARE_METATYPE( boost::shared_ptr<KRss::TreeNode> )
Q_DECLARE_METATYPE( boost::shared_ptr<KRss::RootNode> )
Q_DECLARE_METATYPE( boost::shared_ptr<KRss::TagNode> )
Q_DECLARE_METATYPE( boost::shared_ptr<KRss::FeedNode> )

#endif // KRSS_TREENODE_H
