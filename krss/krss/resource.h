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

#ifndef KRSS_RESOURCE_H
#define KRSS_RESOURCE_H

#include "krss_export.h"
#include "feed.h"

#include <boost/shared_ptr.hpp>

namespace Akonadi {
class CollectionStatistics;
}

namespace KRss {

class RetrieveResourceCollectionsJob;
class ResourcePrivate;

class KRSS_EXPORT Resource : public QObject
{
    Q_OBJECT
public:
    ~Resource();
    QString id() const;
    QString name() const;

public Q_SLOTS:
    virtual void fetchFeed( const KRss::Feed::Id& feedId ) const = 0;
    virtual void abortFetch( const KRss::Feed::Id& feedId ) const = 0;

Q_SIGNALS:
    void feedAdded( const QString& resourceId, const KRss::Feed::Id& feedId );
    void feedChanged( const QString& resourceId, const KRss::Feed::Id& feedId );
    void feedRemoved( const QString& resourceId, const KRss::Feed::Id& feedId );

    void statisticsChanged( const QString& resourceId, const KRss::Feed::Id& feedId,
                            const Akonadi::CollectionStatistics& statistics );

    void fetchStarted( const QString& resourceId, const KRss::Feed::Id& feedId );
    void fetchPercent( const QString& resourceId, const KRss::Feed::Id& feedId, uint percent );
    void fetchFinished( const QString& resourceId, const KRss::Feed::Id& feedId );
    void fetchFailed( const QString& resourceId, const KRss::Feed::Id& feedId, const QString& errorMessage );
    void fetchAborted( const QString& resourceId, const KRss::Feed::Id& feedId );
    void fetchQueueStarted( const QString& resourceId );
    void fetchQueueFinished( const QString& resourceId );

public:
    void registerListeningFeed( Feed* feed );
    void unregisterListeningFeed( Feed* feed );
    virtual RetrieveResourceCollectionsJob* retrieveResourceCollectionsJob() const = 0;

protected:
    Resource( const QString& resourceId, const QString& name, QObject* parent = 0 );
    explicit Resource( ResourcePrivate& dd, QObject* parent = 0 );

    void triggerFeedChanged( const KRss::Feed::Id& feedId );
    void triggerFeedRemoved( const KRss::Feed::Id& feedId );
    void triggerStatisticsChanged( const KRss::Feed::Id& feedId,
                                   const Akonadi::CollectionStatistics &statistics );
    void triggerFetchStarted( const KRss::Feed::Id& feedId );
    void triggerFetchPercent( const KRss::Feed::Id& feedId, uint percentage );
    void triggerFetchFinished( const KRss::Feed::Id& feedId );
    void triggerFetchFailed( const KRss::Feed::Id& feedId, const QString &errorMessage );
    void triggerFetchAborted( const KRss::Feed::Id& feedId );

protected:
    ResourcePrivate* const d_ptr;

private:
    friend class Feed;  // for register/unregisterListeningFeed
    friend class RetrieveFeedListJobPrivate; // for retrieveResourceCollectionsJob
    Q_DECLARE_PRIVATE( Resource );
    Q_DISABLE_COPY( Resource );
};

} //namespace KRss

#endif // KRSS_RESOURCE_H
