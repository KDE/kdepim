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
#include <QtCore/QTimer>
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
        : q( qq ), m_totalCount( 0 ), m_unreadCount( 0 ) {
        m_emitItemCountTimer.setSingleShot( true );
        m_emitItemCountTimer.setInterval( 400 );
        q->connect( &m_emitItemCountTimer, SIGNAL(timeout()), q, SLOT(slotEmitItemCountsDelayed()) );
    }

    void appendResourceCollections( const QList<Akonadi::Collection>& collections,
                                    const boost::shared_ptr<Resource>& resource );
    void appendFeedCollection( const FeedCollection& feedCollection,
                               const boost::shared_ptr<Resource>& resource );

    void slotFeedAdded( const QString& resourceId, const Feed::Id& id );
    void slotFeedRemoved( const Feed::Id& id );
    void slotUnreadCountChanged( const Feed::Id& id, int count );
    void slotTotalCountChanged( const Feed::Id& id, int count );
    void slotCollectionLoadDone( KJob *job );
    void emitItemCounts();
    void slotEmitItemCountsDelayed();

public:
    FeedList * const q;
    QHash<Feed::Id, boost::shared_ptr<Feed> > m_feeds;
    QHash<QString, boost::shared_ptr<Resource> > m_resources;
    QHash<const KJob*, QString> m_pendingJobs;
    int m_totalCount;
    int m_unreadCount;
    QTimer m_emitItemCountTimer;
};

} // namespace KRss

#endif // KRSS_FEEDLIST_P_H
