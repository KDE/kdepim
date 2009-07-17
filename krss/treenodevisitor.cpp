/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

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

#include "treenodevisitor.h"
#include "treenode.h"
#include "itemlisting.h"
#include "itemlistjob.h"
#include "feedlist.h"
#include "feed.h"
#include "statusmodifyjob.h"

#include <Akonadi/ItemFetchScope>

using namespace KRss;
using boost::shared_ptr;

TreeNodeVisitor::~TreeNodeVisitor()
{
}

ConstTreeNodeVisitor::~ConstTreeNodeVisitor()
{
}

class ConnectToItemListingVisitor::Private
{
public:
    explicit Private( const boost::shared_ptr<const FeedList>& feedList,
                      const shared_ptr<ItemListing>& itemListing )
        : m_feedList( feedList ), m_itemListing( itemListing ) {}

    const boost::shared_ptr<const FeedList> m_feedList;
    const shared_ptr<ItemListing> m_itemListing;
};

ConnectToItemListingVisitor::ConnectToItemListingVisitor( const boost::shared_ptr<const FeedList>& feedList,
                                                          const shared_ptr<ItemListing>& itemListing )
    : d( new Private( feedList, itemListing ) )
{
}

ConnectToItemListingVisitor::~ConnectToItemListingVisitor()
{
    delete d;
}

void ConnectToItemListingVisitor::visit( const shared_ptr<const RootNode>& rootNode )
{
    const QList<shared_ptr<const TagNode> > tagNodes = rootNode->tagNodes();
    Q_FOREACH( const shared_ptr<const TagNode>& tagNode, tagNodes ) {
        tagNode->accept( this );
    }
}

void ConnectToItemListingVisitor::visit( const shared_ptr<const TagNode>& tagNode )
{
    const QList<shared_ptr<const FeedNode> > feedNodes = tagNode->feedNodes();
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes ) {
        feedNode->accept( this );
    }
}

void ConnectToItemListingVisitor::visit( const shared_ptr<const FeedNode>& feedNode )
{
    d->m_itemListing->connectToFeed( d->m_feedList->constFeedById( feedNode->feedId() ) );
}

class CreateStatusModifyJobVisitor::Private
{
public:
    explicit Private( const boost::shared_ptr<const FeedList>& feedList )
        : m_feedList( feedList ), m_job( new CompositeStatusModifyJob ) {}

    const boost::shared_ptr<const FeedList> m_feedList;
    CompositeStatusModifyJob * const m_job;
};

CreateStatusModifyJobVisitor::CreateStatusModifyJobVisitor( const boost::shared_ptr<const FeedList>& feedList )
    : d( new Private( feedList ) )
{
}

CreateStatusModifyJobVisitor::~CreateStatusModifyJobVisitor()
{
    delete d;
}

StatusModifyJob* CreateStatusModifyJobVisitor::statusModifyJob() const
{
    return d->m_job;
}

void CreateStatusModifyJobVisitor::visit( const shared_ptr<const RootNode>& rootNode )
{
    const QList<shared_ptr<const TagNode> > tagNodes = rootNode->tagNodes();
    Q_FOREACH( const shared_ptr<const TagNode>& tagNode, tagNodes ) {
        tagNode->accept( this );
    }
}

void CreateStatusModifyJobVisitor::visit( const shared_ptr<const TagNode>& tagNode )
{
    const QList<shared_ptr<const FeedNode> > feedNodes = tagNode->feedNodes();
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes ) {
        feedNode->accept( this );
    }
}

void CreateStatusModifyJobVisitor::visit( const shared_ptr<const FeedNode>& feedNode )
{
    d->m_job->addSubJob( d->m_feedList->feedById( feedNode->feedId() )->statusModifyJob() );
}

class FetchVisitor::Private
{
public:
    explicit Private( const boost::shared_ptr<const FeedList>& feedList )
        : m_feedList( feedList ) {}

    const boost::shared_ptr<const FeedList> m_feedList;
};

FetchVisitor::FetchVisitor( const boost::shared_ptr<const FeedList>& feedList )
    : d( new Private( feedList ) )
{
}

FetchVisitor::~FetchVisitor()
{
    delete d;
}

void FetchVisitor::visit( const shared_ptr<const RootNode>& rootNode )
{
    const QList<shared_ptr<const TagNode> > tagNodes = rootNode->tagNodes();
    Q_FOREACH( const shared_ptr<const TagNode>& tagNode, tagNodes ) {
        tagNode->accept( this );
    }
}

void FetchVisitor::visit( const shared_ptr<const TagNode>& tagNode )
{
    const QList<shared_ptr<const FeedNode> > feedNodes = tagNode->feedNodes();
    Q_FOREACH( const shared_ptr<const FeedNode>& feedNode, feedNodes ) {
        feedNode->accept( this );
    }
}

void FetchVisitor::visit( const shared_ptr<const FeedNode>& feedNode )
{
    d->m_feedList->feedById( feedNode->feedId() )->fetch();
}
