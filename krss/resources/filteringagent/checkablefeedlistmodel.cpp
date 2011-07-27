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

#include "checkablefeedlistmodel.h"

#include <krss/treenode.h>

using namespace KRss;
using boost::shared_ptr;

CheckableFeedListModel::CheckableFeedListModel( const shared_ptr<const FeedList>& feedList,
                                                const shared_ptr<const TagProvider>& tagProvider,
                                                QObject* parent )
    : FeedListModel( feedList, tagProvider, parent )
{
}

QList<Feed::Id> CheckableFeedListModel::selectedFeeds() const
{
    return m_selectedFeeds;
}

void CheckableFeedListModel::setSelectedFeeds( const QList<KRss::Feed::Id>& feeds )
{
    m_selectedFeeds = feeds;
    reset();
}

int CheckableFeedListModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED( parent );
    return 1;
}

QVariant CheckableFeedListModel::data(const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( role == Qt::CheckStateRole ) {
        const shared_ptr<const TreeNode> node = FeedListModel::data( index, FeedListModel::TreeNodeRole ).
                                                               value<shared_ptr<TreeNode> >();
        if ( !node )
            return QVariant();

        if ( node->tier() == TreeNode::FeedTier ) {
            const Feed::Id feedId = boost::static_pointer_cast<const FeedNode>( node )->feedId();
            return ( m_selectedFeeds.contains( feedId ) ? Qt::Checked : Qt::Unchecked );
        }
        else
            return QVariant();
    }

    return FeedListModel::data( index, role );
}

Qt::ItemFlags CheckableFeedListModel::flags( const QModelIndex& index ) const
{
    Qt::ItemFlags flags = FeedListModel::flags( index );
    if ( !FeedListModel::data( index, FeedListModel::IsTagRole ).toBool() )
        return flags | Qt::ItemIsUserCheckable;

    return flags;
}

bool CheckableFeedListModel::setData( const QModelIndex& index, const QVariant& value, int role )
{
    if ( !index.isValid() )
        return false;

    if ( role == Qt::CheckStateRole ) {
        const shared_ptr<const TreeNode> node = FeedListModel::data( index, FeedListModel::TreeNodeRole ).
                                                               value<shared_ptr<TreeNode> >();
        if ( !node )
            return false;

        if ( node->tier() == TreeNode::FeedTier ) {
            const Feed::Id feedId = boost::static_pointer_cast<const FeedNode>( node )->feedId();
            if ( value == QVariant( Qt::Checked ) ) {
                m_selectedFeeds.append( feedId );
                return true;
            }
            else if ( value == QVariant( Qt::Unchecked ) ) {
                m_selectedFeeds.removeOne( feedId );
                return true;
            }
        }
    }

    return FeedListModel::setData( index, value, role );
}
