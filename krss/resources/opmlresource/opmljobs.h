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

#ifndef KRSS_OPMLRESOURCE_OPMLJOBS_H
#define KRSS_OPMLRESOURCE_OPMLJOBS_H

#include <krssresource/rssbackendjobs.h>
#include <krss/feedcollection.h>

#include <Syndication/Syndication>
#include <Akonadi/Collection>
#include <KUrl>
#include <KJob>

class OpmlJob
{
public:
    void setPath( const KUrl& path );

protected:
    KUrl m_path;
};

class OpmlFeedsRetrieveJob : public KRssResource::FeedsRetrieveJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedsRetrieveJob( QObject *parent = 0 );

    QList<Akonadi::Collection> feeds() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void emitCachedResult();
    void slotGetFinished( KJob *job );
    void slotNewPutFinished( KJob *job );
    void slotPutFinished( KJob *job );

private:
    friend class OpmlFeedCreateJob;
    friend class OpmlFeedModifyJob;
    friend class OpmlFeedDeleteJob;
    friend class OpmlFeedsImportJob;

    // remote id => Akonadi::Collection
    static QHash<QString, Akonadi::Collection> m_feedsCache;
    static bool m_isCached;

    Q_DISABLE_COPY( OpmlFeedsRetrieveJob )
};

class OpmlFeedsImportJob : public KRssResource::FeedsImportJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedsImportJob( QObject *parent = 0 );

    void setFeeds( const QList<Akonadi::Collection>& feeds );
    QList<Akonadi::Collection> feeds() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotFeedsRetrieved( KJob *job );
    void slotPutFinished( KJob *job );

private:
    QList<Akonadi::Collection> m_feeds;
};

class OpmlFeedCreateJob : public KRssResource::FeedCreateJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedCreateJob( QObject *parent = 0 );

    void setXmlUrl( const QString& xmlUrl );
    Akonadi::Collection feed() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotFeedsRetrieved( KJob *job );
    void slotFeedFetched( Syndication::Loader *loader, Syndication::FeedPtr feedptr, Syndication::ErrorCode status );
    void slotPutFinished( KJob *job );

private:
    KUrl m_xmlUrl;
    KRss::FeedCollection m_feed;
    int m_fetchAttempts;
    Q_DISABLE_COPY( OpmlFeedCreateJob )
};

class OpmlFeedModifyJob : public KRssResource::FeedModifyJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedModifyJob( QObject *parent = 0 );

    void setFeed( const Akonadi::Collection& feed );
    Akonadi::Collection feed() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotFeedsRetrieved( KJob *job );
    void slotPutFinished( KJob *job );

private:
    KRss::FeedCollection m_feed;
    Q_DISABLE_COPY( OpmlFeedModifyJob )
};

class OpmlFeedDeleteJob : public KRssResource::FeedDeleteJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedDeleteJob( QObject *parent = 0 );

    void setFeed( const Akonadi::Collection& feed );
    Akonadi::Collection feed() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotFeedsRetrieved( KJob *job );
    void slotPutFinished( KJob *job );

private:
    KRss::FeedCollection m_feed;
    Q_DISABLE_COPY( OpmlFeedDeleteJob )
};

class OpmlFeedFetchJob : public KRssResource::FeedFetchJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlFeedFetchJob( QObject *parent = 0 );

    void setFeed( const Akonadi::Collection& feed );
    Akonadi::Collection feed() const;
    QList<Akonadi::Item> items() const;

    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotFeedFetched( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode );

private:
    KRss::FeedCollection m_feed;
    QList<Akonadi::Item> m_items;
    Q_DISABLE_COPY( OpmlFeedFetchJob )
};

class OpmlItemModifyJob : public KRssResource::ItemModifyJob, public OpmlJob
{
    Q_OBJECT
public:
    explicit OpmlItemModifyJob( QObject *parent = 0 );

    void setItem( const Akonadi::Item& item );
    Akonadi::Item item() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void doStart();

private:
    Akonadi::Item m_item;
    Q_DISABLE_COPY( OpmlItemModifyJob )
};

#endif // KRSS_OPMLRESOURCE_OPMLJOBS_H
