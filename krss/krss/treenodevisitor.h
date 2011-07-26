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

#ifndef KRSS_TREENODEVISITOR_H
#define KRSS_TREENODEVISITOR_H

#include "krss_export.h"

#include <boost/shared_ptr.hpp>

namespace Akonadi {
class ItemFetchScope;
}

namespace KRss {

class TreeNode;
class RootNode;
class TagNode;
class FeedNode;
class FeedList;
class ItemListing;
class CompositeItemListJob;
class StatusModifyJob;

class KRSS_EXPORT TreeNodeVisitor
{
public:
    virtual ~TreeNodeVisitor();

    virtual void visit( const boost::shared_ptr<RootNode>& rootNode ) = 0;
    virtual void visit( const boost::shared_ptr<TagNode>& tagNode ) = 0;
    virtual void visit( const boost::shared_ptr<FeedNode>& feedNode ) = 0;
};

class KRSS_EXPORT ConstTreeNodeVisitor
{
    public:
        virtual ~ConstTreeNodeVisitor();

        virtual void visit( const boost::shared_ptr<const RootNode>& rootNode ) = 0;
        virtual void visit( const boost::shared_ptr<const TagNode>& tagNode ) = 0;
        virtual void visit( const boost::shared_ptr<const FeedNode>& feedNode ) = 0;
};


class KRSS_EXPORT ConnectToItemListingVisitor : public ConstTreeNodeVisitor
{
public:
    explicit ConnectToItemListingVisitor( const boost::shared_ptr<const FeedList>& feedList,
                                          const boost::shared_ptr<ItemListing>& itemListing );
    ~ConnectToItemListingVisitor();

    void visit( const boost::shared_ptr<const RootNode>& rootNode );
    void visit( const boost::shared_ptr<const TagNode>& tagNode );
    void visit( const boost::shared_ptr<const FeedNode>& feedNode );

private:
    class Private;
    Private * const d;
};

class KRSS_EXPORT CreateStatusModifyJobVisitor : public ConstTreeNodeVisitor
{
public:
    explicit CreateStatusModifyJobVisitor( const boost::shared_ptr<const FeedList>& feedList );
    ~CreateStatusModifyJobVisitor();

    StatusModifyJob* statusModifyJob() const;

    void visit( const boost::shared_ptr<const RootNode>& rootNode );
    void visit( const boost::shared_ptr<const TagNode>& tagNode );
    void visit( const boost::shared_ptr<const FeedNode>& feedNode );

private:
    class Private;
    Private * const d;
};

class KRSS_EXPORT FetchVisitor : public ConstTreeNodeVisitor
{
public:
    explicit FetchVisitor( const boost::shared_ptr<const FeedList>& feedList );
    ~FetchVisitor();

    void visit( const boost::shared_ptr<const RootNode>& rootNode );
    void visit( const boost::shared_ptr<const TagNode>& tagNode );
    void visit( const boost::shared_ptr<const FeedNode>& feedNode );

private:
    class Private;
    Private * const d;
};

} // namespace KRss

#endif // KRSS_TREENODEVISITOR_H
