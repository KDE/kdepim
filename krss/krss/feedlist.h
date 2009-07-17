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

#ifndef KRSS_FEEDLIST_H
#define KRSS_FEEDLIST_H

#include "krss_export.h"
#include "feed.h"

#include <boost/shared_ptr.hpp>

#include <KJob>

class QStringList;

namespace KRss {

class NetFeed;
class SearchFeed;
class RetrieveFeedListJob;
class RetrieveFeedListJobPrivate;
class NetFeedCreateJob;

class FeedListPrivate;

class KRSS_EXPORT FeedList : public QObject
{
    Q_OBJECT
    friend class ::KRss::RetrieveFeedListJob;
    friend class ::KRss::RetrieveFeedListJobPrivate;
    friend class ::KRss::NetFeedCreateJob;

public:

    explicit FeedList( const QStringList &resourceIdentifiers, QObject *parent = 0 );
    ~FeedList();

    QList<boost::shared_ptr<Feed> > feeds() const;
    QList<boost::shared_ptr<const Feed> > constFeeds() const;
    QList<boost::shared_ptr<NetFeed> > netFeeds() const;
    QList<boost::shared_ptr<const NetFeed> > constNetFeeds() const;
    QList<boost::shared_ptr<SearchFeed> > searchFeeds() const;
    QList<boost::shared_ptr<const SearchFeed> > constSearchFeeds() const;

    QList<Feed::Id> feedIds() const;
    QStringList feedUrls() const;

    boost::shared_ptr<Feed> feedById( const Feed::Id& id ) const;
    boost::shared_ptr<const Feed> constFeedById( const Feed::Id& id ) const;
    QList<boost::shared_ptr<Feed> > feedByTitle( const QString& title ) const;
    QList<boost::shared_ptr<const Feed> > constFeedByTitle( const QString& title ) const;

Q_SIGNALS:

    void feedAdded( const KRss::Feed::Id& id );
    void feedChanged( const KRss::Feed::Id& id );
    void feedRemoved( const KRss::Feed::Id& id );
    void totalCountChanged( const KRss::Feed::Id& id, int count );
    void unreadCountChanged( const KRss::Feed::Id& id, int count );

    void fetchStarted( const KRss::Feed::Id& id );
    void fetchPercent( const KRss::Feed::Id& id, uint percent );
    void fetchFinished( const KRss::Feed::Id& id );
    void fetchFailed( const KRss::Feed::Id& id, const QString& errorMessage );

private:

    friend class FeedListPrivate;
    FeedListPrivate * const d;

    Q_DISABLE_COPY( FeedList )
    Q_PRIVATE_SLOT( d, void slotFeedAdded( const KRss::Feed::Id& id ) )
    Q_PRIVATE_SLOT( d, void slotFeedRemoved( const KRss::Feed::Id& id ) )
    Q_PRIVATE_SLOT( d, void slotCollectionLoadDone( KJob *job ) )
};



class KRSS_EXPORT RetrieveFeedListJob : public KJob
{
    Q_OBJECT
    friend class ::KRss::RetrieveFeedListJobPrivate;

public:
    enum Error {
        CouldNotRetrieveFeedListError=KJob::UserDefinedError //TODO ...
    };

    explicit RetrieveFeedListJob( QObject* parent=0 );
    ~RetrieveFeedListJob();

    void setResourceIdentifiers( const QStringList& resourceIdentifiers );
    QStringList resourceIdentifiers() const;

    /* impl */ void start();

    boost::shared_ptr<FeedList> feedList() const;

private:
    RetrieveFeedListJobPrivate* const d;
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void loadDone(KJob*) )
};

} // namespace KRss

#endif // KRSS_FEEDLIST_H
