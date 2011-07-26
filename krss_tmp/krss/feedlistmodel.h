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

#ifndef KRSS_FEEDLISTMODEL_H
#define KRSS_FEEDLISTMODEL_H

#include "krss_export.h"
#include "feed.h"

#include <QtCore/QAbstractItemModel>

#include <boost/shared_ptr.hpp>

namespace KRss {

class Feed;
class FeedList;
class TagProvider;

class KRSS_EXPORT FeedListModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Role {
        TreeNodeRole = Qt::UserRole,
        HasUnreadRole,
        IsTagRole,
        UserRole = HasUnreadRole+100
    };

    enum Column {
        TitleColumn=0,
        UnreadCountColumn,
        TotalCountColumn,
        ColumnCount
    };
public:

    FeedListModel( const boost::shared_ptr<const FeedList>& feedlist,
                   const boost::shared_ptr<const TagProvider>& tagProvider,
                   QObject *parent = 0 );
    ~FeedListModel();

    boost::shared_ptr<const FeedList> feedList() const;

    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &index ) const;
    Qt::ItemFlags flags( const QModelIndex& index ) const;
    QStringList mimeTypes() const;
    QMimeData* mimeData( const QModelIndexList& indexes ) const;
    bool dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent );

protected:

    // helper methods
    QList<QModelIndex> feedIndexes( const Feed::Id& id ) const;

private:

    friend class FeedListModelPrivate;
    class FeedListModelPrivate * const d;

    Q_DISABLE_COPY( FeedListModel )
    Q_PRIVATE_SLOT( d, void slotTagCreated( const KRss::Tag& tag ) )
    Q_PRIVATE_SLOT( d, void slotTagModified( const KRss::Tag& tag ) )
    Q_PRIVATE_SLOT( d, void slotTagDeleted( const KRss::TagId& id ) )
    Q_PRIVATE_SLOT( d, void slotFeedAdded( const KRss::Feed::Id& id ) )
    Q_PRIVATE_SLOT( d, void slotFeedChanged( const KRss::Feed::Id& id ) )
    Q_PRIVATE_SLOT( d, void slotFeedRemoved( const KRss::Feed::Id& id) )
    Q_PRIVATE_SLOT( d, void slotFeedItemCountChanged( const KRss::Feed::Id& id, int count ) )
};

} // namespace KRss

#endif // KRSS_FEEDLISTMODEL_H
