/*
    Copyright (C) 2008,2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSS_FEED_P_H
#define KRSS_FEED_P_H

#include "item.h"
#include "itemlistjob.h"
#include "statusmodifyjob.h"
#include "feedcollection.h"

#include <Akonadi/Item>
#include <akonadi/itemfetchscope.h>

#include <QtGui/QIcon>
#include <QtCore/QList>

namespace Akonadi {
    class ItemFetchJob;
}

namespace KRss {

class Resource;

class FeedPrivate
{
public:

    FeedPrivate( const FeedCollection& feedCollection, const Resource *resource, Feed *qq )
        : q( qq ), m_feedCollection( feedCollection ), m_resource( resource ),
          m_unreadCount( 0 ), m_totalCount( 0 ), m_fetching( false )
    {
    }

    void updateFromStatistics( const Akonadi::CollectionStatistics& stats );
    void slotCollectionLoadDone( KJob *job );
    void slotStatisticsFetchDone( KJob *job );
    void slotFeedChanged( const KRss::Feed::Id& feedId );
    void slotFeedRemoved( const KRss::Feed::Id& feedId );
    void slotFetchStarted( const KRss::Feed::Id& feedId );
    void slotFetchPercent( const KRss::Feed::Id& feedId, uint percentage );
    void slotFetchFinished( const KRss::Feed::Id& feedId );
    void slotFetchFailed( const KRss::Feed::Id& feedId, const QString &errorMessage );
    void slotFetchAborted( const KRss::Feed::Id& feedId );
    void slotStatisticsChanged( const KRss::Feed::Id& feedId, const Akonadi::CollectionStatistics& stats );

    Feed* const q;
    FeedCollection m_feedCollection;
    const Resource *m_resource;
    int m_unreadCount;
    int m_totalCount;
    bool m_fetching;
    QIcon m_icon;
};

class ItemListJobImpl : public ItemListJob {
    Q_OBJECT
public:
    explicit ItemListJobImpl( Feed* parent = 0 ) : ItemListJob( parent ), m_feed( parent ), m_job( 0 ) {}

    /* reimp */ void setFetchScope( const Akonadi::ItemFetchScope &fetchScope ) { m_fetchScope = fetchScope; }

    /* reimp */ Akonadi::ItemFetchScope &fetchScope() { return m_fetchScope; }

    /* reimp */  QList<KRss::Item> items() const { return m_items; }

    /* reimp */ void start();

private Q_SLOTS:
    void doStart();
    void slotItemsReceived( const Akonadi::Item::List& items );
    void jobDone( KJob* job );

private:
    const Feed* m_feed;
    Akonadi::ItemFetchJob* m_job;
    QList<KRss::Item> m_items;
    Akonadi::ItemFetchScope m_fetchScope;
};

class StatusModifyJobImpl : public StatusModifyJob
{
    Q_OBJECT
public:
    explicit StatusModifyJobImpl( Feed* parent );

    enum Error {
        CouldNotLoadItems = KJob::UserDefinedError,
        CouldNotModifyItem,
        UserDefinedError = CouldNotModifyItem + 100
    };

    void clearFlags( const QList<KRss::Item::StatusFlag>& flags );
    void setFlags( const QList<KRss::Item::StatusFlag>& flags );

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotItemsLoaded( KJob *job );
    void slotItemModified( KJob *job );

private:
    Feed* m_feed;
    QList<Item::StatusFlag> m_setFlags;
    QList<Item::StatusFlag> m_clearFlags;
    int m_jobs;
    int m_totalJobs;
};

} // namespace KRss

#endif // KRSS_FEED_P_H
