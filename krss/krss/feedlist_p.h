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

#include <Akonadi/Collection>

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
        : q( qq ) {}

    void appendResourceCollections( const QList<Akonadi::Collection>& collections,
                                    const boost::shared_ptr<Resource>& resource );
    void appendFeedCollection( const FeedCollection& feedCollection,
                               const boost::shared_ptr<Resource>& resource );

    void slotFeedAdded( const QString& resourceId, const KRss::Feed::Id& id );
    void slotFeedRemoved( const KRss::Feed::Id& id );
    void slotCollectionLoadDone( KJob *job );

public:
    FeedList * const q;
    QHash<Feed::Id, boost::shared_ptr<Feed> > m_feeds;
    QHash<QString, boost::shared_ptr<Resource> > m_resources;
    QHash<const KJob*, QString> m_pendingJobs;
};

} // namespace KRss

#endif // KRSS_FEEDLIST_P_H
