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

#ifndef KRSS_FEEDLIST_P_H
#define KRSS_FEEDLIST_P_H

#include "feed.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <boost/shared_ptr.hpp>

class KJob;

namespace KRss {

class FeedCollection;
class Resource;
class FeedList;

class FeedListPrivate
{
public:

    explicit FeedListPrivate( FeedList *qq )
        : q( qq )
    {
    }

    KRss::Feed::Id appendFeed( const FeedCollection& feedCollection );
    void dump();

    void slotFeedAdded( const KRss::Feed::Id& id );
    void slotFeedRemoved( const KRss::Feed::Id& id );
    void slotCollectionLoadDone( KJob *job );

public:

    FeedList * const q;

    // resource id => list of child feed ids
    QHash<QString, QList<Feed::Id> > m_feedsMap;
    // feed id => feed
    QHash<Feed::Id, boost::shared_ptr<Feed> > m_feeds;
    // resource id => resource
    QHash<QString, const Resource *> m_resources;

};

} // namespace KRss

#endif // KRSS_FEEDLIST_P_H
