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

#ifndef KRSSRESOURCE_RSSRESOURCEBASEJOBS_H
#define KRSSRESOURCE_RSSRESOURCEBASEJOBS_H

#include <krss/feed.h>
#include <krss/feedcollection.h>

#include <Akonadi/Collection>
#include <KJob>
#include <boost/function.hpp>

namespace KRssResource
{

class FeedCreateJob;
class FeedModifyJob;
class FeedDeleteJob;
class FeedFetchJob;

namespace FeedCollectionsCache
{
    QList<Akonadi::Collection> feedCollections();
    void insertFeedCollection( const Akonadi::Collection& feed );
    void clear();
} // namespace FeedCollectionsCache

class FeedCollectionRetrieveJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCollectionRetrieveJob( const KRss::Feed::Id& id, QObject *parent = 0 );
    explicit FeedCollectionRetrieveJob( QObject *parent = 0 );
    ~FeedCollectionRetrieveJob();

    void setResourceId( const QString& resourceId );
    KRss::FeedCollection feedCollection() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotFetchFeedCollection = KJob::UserDefinedError,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( FeedCollectionRetrieveJob )
    Q_PRIVATE_SLOT( d, void emitCachedResult() )
    Q_PRIVATE_SLOT( d, void slotCollectionsFetched( KJob* ) )
};

class FeedCollectionCreateJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCollectionCreateJob( const QString& xmlUrl, QObject *parent = 0 );
    ~FeedCollectionCreateJob();

    void setResourceId( const QString& resourceId );
    void setBackendJob( KRssResource::FeedCreateJob *job );
    KRss::FeedCollection feedCollection() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveRootCollection = KJob::UserDefinedError,
        CouldNotCreateBackendCollection,
        CouldNotCreateAkonadiCollection,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( FeedCollectionCreateJob )
    Q_PRIVATE_SLOT( d, void slotRootCollectionRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotBackendCollectionCreated( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionCreated( KJob* ) )
};

class FeedCollectionModifyJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCollectionModifyJob( const KRss::Feed::Id& id, QObject *parent = 0 );
    ~FeedCollectionModifyJob();

    void setResourceId( const QString& resourceId );
    void setBackendJob( KRssResource::FeedModifyJob *job );
    void setModifier( const boost::function1<void, KRss::FeedCollection&>& modifier );

    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveAkonadiCollection = KJob::UserDefinedError,
        CouldNotModifyAkonadiCollection,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( FeedCollectionModifyJob )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionModified( KJob* ) )
};

class FeedCollectionDeleteJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCollectionDeleteJob( const KRss::Feed::Id& id, QObject *parent = 0 );
    ~FeedCollectionDeleteJob();

    void setResourceId( const QString& resourceId );
    void setBackendJob( KRssResource::FeedDeleteJob *job );
    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveAkonadiCollection = KJob::UserDefinedError,
        CouldNotDeleteBackendCollection,
        CouldNotDeleteAkonadiCollection,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( FeedCollectionDeleteJob )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotBackendCollectionDeleted( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionDeleted( KJob* ) )
};

class FeedCollectionFetchJob : public KJob
{
    Q_OBJECT
public:
    explicit FeedCollectionFetchJob( const KRss::FeedCollection& collection, QObject *parent = 0 );
    explicit FeedCollectionFetchJob( const KRss::Feed::Id& id, QObject *parent = 0 );
    ~FeedCollectionFetchJob();

    void setResourceId( const QString& resourceId );
    void setBackendJob( KRssResource::FeedFetchJob *job );
    void setSynchronizeFlags( bool synchronizeFlags );
    bool synchronizeFlags() const;
    KRss::FeedCollection feedCollection() const;

    void start();
    QString errorString() const;

    enum Error {
        CouldNotRetrieveAkonadiCollection = KJob::UserDefinedError,
        CouldNotMarkOldItems,
        CouldNotFetchFeed,
        CouldNotSyncItems,
        UserDefinedError
    };

private:
    class Private;
    Private * const d;
    Q_DISABLE_COPY( FeedCollectionFetchJob )
    Q_PRIVATE_SLOT( d, void doStart() )
    Q_PRIVATE_SLOT( d, void slotAkonadiCollectionRetrieved( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotOldItemsMarked( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotFeedFetched( KJob* ) )
    Q_PRIVATE_SLOT( d, void slotItemsSynced( KJob *job ) )
};

} // namespace KRssResource

#endif // KRSSRESOURCE_RSSRESOURCEBASEJOBS_H
