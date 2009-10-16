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

#ifndef RSSSEARCHAGENT_CHECKABLEFEEDLISTMODEL_H
#define RSSSEARCHAGENT_CHECKABLEFEEDLISTMODEL_H

#include <krss/feedlistmodel.h>
#include <krss/feedlist.h>
#include <krss/tagprovider.h>
#include <krss/feed.h>

#include <boost/shared_ptr.hpp>

class CheckableFeedListModel : public KRss::FeedListModel
{
public:
    CheckableFeedListModel( const boost::shared_ptr<const KRss::FeedList>& feedlist,
                         const boost::shared_ptr<const KRss::TagProvider>& tagProvider,
                         QObject *parent = 0 );

    QList<KRss::Feed::Id> selectedFeeds() const;
    void setSelectedFeeds( const QList<KRss::Feed::Id>& feeds );

    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex& index ) const;
    bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );

private:
    QList<KRss::Feed::Id> m_selectedFeeds;
};

#endif // RSSSEARCHAGENT_CHECKABLEFEEDLISTMODEL_H
