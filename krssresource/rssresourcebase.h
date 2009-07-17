/*
    Copyright (C) 2008, 2009    Dmitry Ivanov <vonami@gmail.com>

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

#ifndef KRSSRESOURCE_RSSRESOURCEBASE_H
#define KRSSRESOURCE_RSSRESOURCEBASE_H

#include "krssresource_export.h"

#include <krss/feedcollection.h>
#include <krss/feed.h>

#include <Akonadi/ResourceBase>

#include <QtCore/QHash>
#include <QtDBus/QDBusMessage>

class KJob;

namespace KRssResource
{

class FeedsRetrieveJob;
class FeedsImportJob;
class FeedCreateJob;
class FeedModifyJob;
class FeedDeleteJob;
class FeedFetchJob;
class ItemModifyJob;

class KRSSRESOURCE_EXPORT RssResourceBase : public Akonadi::ResourceBase, public Akonadi::AgentBase::Observer
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.kde.krss")

public:
    explicit RssResourceBase( const QString& id );
    ~RssResourceBase();

    void collectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent );
    void collectionChanged( const Akonadi::Collection& collection );
    void collectionRemoved( const Akonadi::Collection& collection );
    void itemAdded( const Akonadi::Item& item, const Akonadi::Collection& collection );
    void itemChanged( const Akonadi::Item& item, const QSet<QByteArray>& partIdentifiers );
    void itemRemoved( const Akonadi::Item& item );

public Q_SLOTS:
    void configure( WId windowId );

    // D-Bus interface
    QVariantMap addFeed( const QString& xmlUrl, const QString& subscriptionLabel );
    void fetchFeed( const KRss::Feed::Id& id );
    void abortFetch( const KRss::Feed::Id& id );
    bool removeFeed( const KRss::Feed::Id& id );
    bool subscribe( const KRss::Feed::Id& id, const QString& subscriptionLabel );
    bool unsubscribe( const KRss::Feed::Id& id, const QString& subscriptionLabel );
    QStringList subscriptions( const KRss::Feed::Id& id );
    QVariantMap importOpml( const QString &path, const QString &defaultTag );
    QVariantMap exportOpml( const QString &path );
    QVariantMap importItems( const QString& xmlUrl, const QString& path );

Q_SIGNALS:
    // D-Bus interface
    // NOTE: 'qlonglong' means here 'const KRss::Feed::Id&'
    // needed for QDBusAbstractAdaptor::autoRelaySignals to work
    // (the signatures must match)
    void fetchStarted( qlonglong id );
    void fetchPercent( qlonglong id , uint percentage );
    void fetchFinished( qlonglong id );
    void fetchFailed( qlonglong id, const QString& errorMessage );
    void fetchAborted( qlonglong id );
    void fetchQueueStarted();
    void fetchQueueFinished();

protected Q_SLOTS:
    void retrieveCollections();
    void retrieveItems( const Akonadi::Collection& collection );
    bool retrieveItem( const Akonadi::Item& item, const QSet<QByteArray>& parts );

private Q_SLOTS:
    void slotCollectionsRetrieved( KJob *job );
    void slotCollectionChanged( KJob *job );
    void slotCollectionDeleted( KJob *job );
    void slotItemChanged( KJob *job );
    void slotFeedAdded( KJob *job );
    void slotFeedModified( KJob *job );
    void slotFeedDeleted( KJob *job );
    void slotSubscriptionsRetrieved( KJob *job );
    void slotItemsRetrieved( KJob *job );
    void slotFetchResult( KJob *job );
    void slotFetchPercent( KJob *job, unsigned long percentage );
    void slotOpmlImported( KJob *job );
    void slotOpmlExported( KJob *job );
    void slotItemsImported( KJob *job );

private:
    virtual bool flagsSynchronizable() const = 0;
    virtual FeedsRetrieveJob* feedsRetrieveJob() const = 0;
    virtual FeedsImportJob* feedsImportJob() const = 0;
    virtual FeedCreateJob* feedCreateJob() const = 0;
    virtual FeedModifyJob* feedModifyJob() const = 0;
    virtual FeedDeleteJob* feedDeleteJob() const = 0;
    virtual FeedFetchJob* feedFetchJob() const = 0;
    virtual ItemModifyJob* itemModifyJob() const = 0;

private:
    class Private;
    Private * const d;
};

} // namespace KRssResource

#endif // KRSS_RSSRESOURCEBASE_H
