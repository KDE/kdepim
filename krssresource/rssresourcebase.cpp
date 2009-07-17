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

#include "rssresourcebase.h"
#include "krssadaptor.h"
#include "rssbackendjobs.h"
#include "rssresourcebasejobs.h"
#include "importopmljob.h"
#include "exportopmljob.h"
#include "importitemsjob.h"
#include "util.h"

#include <krss/resourcemanager.h>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ItemFetchScope>

#include <KDebug>
#include <KLocale>

#include <QtDBus/QDBusConnection>

#include <boost/bind.hpp>

using namespace KRssResource;
using namespace KRss;

static void addSubscriptionLabel( KRss::FeedCollection& collection, const QString& label )
{
    collection.addSubscriptionLabel( label );
}

static void removeSubscriptionLabel( KRss::FeedCollection& collection, const QString& label )
{
    collection.removeSubscriptionLabel( label );
}

class RssResourceBase::Private
{
public:
    explicit Private() {};

    QHash<KJob*, KRss::FeedCollection> m_feedsInProcess;
    QHash<const KJob*, KRss::Feed::Id> m_fetchJobs;
    QHash<KJob*, QDBusMessage> m_replies;
};

RssResourceBase::RssResourceBase( const QString &id )
    : ResourceBase( id ), d( new Private() )
{
    KRss::ResourceManager::registerAttributes();

    changeRecorder()->fetchCollection( true );
    changeRecorder()->itemFetchScope().fetchFullPayload( false );
    changeRecorder()->itemFetchScope().fetchAllAttributes( true );  // in order to fetch the flags

    new KrssAdaptor( this );
    if ( !QDBusConnection::sessionBus().registerObject( QLatin1String( "/KRss" ),
         this, QDBusConnection::ExportAdaptors ) ) {
        kWarning() << "Couldn't register a D-Bus service org.kde.krss";
        kWarning() << QDBusConnection::sessionBus().lastError().message();
    }
}

RssResourceBase::~RssResourceBase()
{
    delete d;
}

void RssResourceBase::collectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection &parent )
{
    changeCommitted( collection );
}

void RssResourceBase::collectionChanged( const Akonadi::Collection &collection )
{
    FeedModifyJob * const backendJob = feedModifyJob();
    if ( !backendJob ) {
        emit warning( i18n( "Modifying a feed: not implemented" ) );
        changeCommitted( collection );
        return;
    }

    backendJob->setFeed( collection );
    connect( backendJob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionChanged( KJob* ) ) );
    backendJob->start();
}

void RssResourceBase::collectionRemoved( const Akonadi::Collection &collection )
{
    FeedDeleteJob * const backendJob = feedDeleteJob();
    if ( !backendJob ) {
        emit warning( i18n( "Deleting a feed: not implemented" ) );
        changeCommitted( collection );
        return;
    }

    backendJob->setFeed( collection );
    connect( backendJob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionDeleted( KJob* ) ) );
    backendJob->start();
}

void RssResourceBase::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
    changeCommitted( item );
}

void RssResourceBase::itemChanged( const Akonadi::Item &item, const QSet<QByteArray> &partIdentifiers )
{
    ItemModifyJob * const backendJob = itemModifyJob();
    if ( !backendJob ) {
        emit warning( i18n( "Modifying an item: not implemented" ) );
        changeCommitted( item );
        return;
    }

    backendJob->setItem( item );
    connect( backendJob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemChanged( KJob* ) ) );
    backendJob->start();
}

void RssResourceBase::itemRemoved( const Akonadi::Item &item )
{
    changeCommitted( item );
}

QVariantMap RssResourceBase::addFeed( const QString &xmlUrl, const QString &subscriptionLabel )
{
    FeedCreateJob * const backendJob = feedCreateJob();
    if ( !backendJob ) {
        emit warning( i18n( "Creating a feed: not implemented" ) );
        QVariantMap res;
        res.insert( QLatin1String("errorString"),
                    i18n( "Creating a feed: not implemented" ) );
        return res;
    }

    FeedCollectionCreateJob * const job = new FeedCollectionCreateJob( xmlUrl, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedAdded( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return QVariantMap();
}

void RssResourceBase::fetchFeed( const KRss::Feed::Id& id )
{
    FeedFetchJob * const backendJob = feedFetchJob();
    if ( !backendJob ) {
        emit warning( i18n( "Fetching a feed: not implemented" ) );
        return;
    }

    FeedCollectionFetchJob * const job = new FeedCollectionFetchJob( id, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    job->setSynchronizeFlags( flagsSynchronizable() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFetchResult( KJob* ) ) );
    connect( job, SIGNAL( percent( KJob*, unsigned long) ),
             this, SLOT( slotFetchPercent( KJob*, unsigned long ) ) );
    d->m_fetchJobs.insert( job, id );
    job->start();
    emit fetchStarted( id );
    if ( d->m_fetchJobs.count() == 1 )
        emit fetchQueueStarted();
    return;
}

void RssResourceBase::abortFetch( const KRss::Feed::Id& id )
{
    return;
}

bool RssResourceBase::removeFeed( const KRss::Feed::Id& id )
{
    FeedDeleteJob * const backendJob = feedDeleteJob();
    if ( !backendJob ) {
        emit warning( i18n( "Deleting a feed: not implemented" ) );
        return false;
    }

    FeedCollectionDeleteJob * const job = new FeedCollectionDeleteJob( id, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedDeleted( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return false;
}

bool RssResourceBase::subscribe( const KRss::Feed::Id& id, const QString &subscriptionLabel )
{
    FeedModifyJob * const backendJob = feedModifyJob();
    if ( !backendJob ) {
        emit warning( i18n( "Modifying a feed: not implemented" ) );
        return false;
    }

    FeedCollectionModifyJob * const job = new FeedCollectionModifyJob( id, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    job->setModifier( boost::bind( &addSubscriptionLabel, _1, subscriptionLabel ) );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedModified( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return false;
}

bool RssResourceBase::unsubscribe( const KRss::Feed::Id& id, const QString &subscriptionLabel )
{
    FeedModifyJob * const backendJob = feedModifyJob();
    if ( !backendJob ) {
        emit warning( i18n( "Modifying a feed: not implemented" ) );
        return false;
    }

    FeedCollectionModifyJob * const job = new FeedCollectionModifyJob( id, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    job->setModifier( boost::bind( &removeSubscriptionLabel, _1, subscriptionLabel ) );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedModified( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return false;
}

QStringList RssResourceBase::subscriptions( const KRss::Feed::Id& id )
{
    FeedCollectionRetrieveJob * const job = new FeedCollectionRetrieveJob( id, this );
    job->setResourceId( identifier() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotSubscriptionsRetrieved( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return QStringList();
}

QVariantMap RssResourceBase::importOpml( const QString &path, const QString &defaultTag )
{
    FeedsImportJob * const backendJob = feedsImportJob();
    if ( !backendJob ) {
        emit warning( i18n( "Importing feeds: not implemented" ) );
        QVariantMap result;
        result.insert( "error", 1 );
        result.insert( "errorString", i18n( "Not implemented" ) );
        return result;
    }

    const KUrl url = KUrl::fromPathOrUrl( path );
    ImportOpmlJob * const job = new ImportOpmlJob( url, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    job->setDefaultTag( defaultTag );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotOpmlImported( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return QVariantMap();
}

QVariantMap RssResourceBase::exportOpml( const QString &path )
{
    ExportOpmlJob * const job = new ExportOpmlJob( KUrl( path ), this );
    job->setResourceId( identifier() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotOpmlExported( KJob* ) ) );
    job->start();

    // send the reply later
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    return QVariantMap();
}

QVariantMap RssResourceBase::importItems( const QString& xmlUrl, const QString& path )
{
    ImportItemsJob *job = new ImportItemsJob( xmlUrl, this );
    job->setResourceId( identifier() );
    job->setFlagsSynchronizable( flagsSynchronizable() );
    job->setSourceFile( path );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemsImported( KJob* ) ) );
    setDelayedReply( true );
    d->m_replies.insert( job, QDBusContext::message().createReply() );
    job->start();
    return QVariantMap();
}

bool RssResourceBase::retrieveItem( const Akonadi::Item &item, const QSet<QByteArray> &parts )
{
    return false;
}

void RssResourceBase::configure( WId windowId )
{
    Q_UNUSED( windowId )

    // don't do anything
    return;
}

void RssResourceBase::retrieveCollections()
{
    FeedsRetrieveJob * const job = feedsRetrieveJob();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsRetrieved( KJob* ) ) );
    job->start();
}

void RssResourceBase::retrieveItems( const Akonadi::Collection& collection )
{
    FeedFetchJob * const backendJob = feedFetchJob();
    if ( !backendJob ) {
        emit warning( i18n( "Retrieving items: not implemented" ) );
        itemsRetrievalDone();
        return;
    }

    FeedCollectionFetchJob * const job = new FeedCollectionFetchJob( collection, this );
    job->setResourceId( identifier() );
    job->setBackendJob( backendJob );
    job->setSynchronizeFlags( flagsSynchronizable() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotItemsRetrieved( KJob* ) ) );
    connect( job, SIGNAL( percent( KJob*, unsigned long) ),
             this, SLOT( slotFetchPercent( KJob*, unsigned long ) ) );
    d->m_fetchJobs.insert( job, FeedCollection::feedIdFromAkonadi( collection.id() ) );
    job->start();
    emit fetchStarted( FeedCollection::feedIdFromAkonadi( collection.id() ) );
    if ( d->m_fetchJobs.count() == 1 )
        emit fetchQueueStarted();
    return;
}

void RssResourceBase::slotCollectionsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        collectionsRetrievedIncremental( Akonadi::Collection::List(), Akonadi::Collection::List() );
        return;
    }

    FeedsRetrieveJob *fjob = dynamic_cast<FeedsRetrieveJob*>( job );
    Q_ASSERT( fjob );

    QList<Akonadi::Collection> feeds = fjob->feeds();
    const int size = feeds.size();
    for( int i = 0; i < size; ++i ) {
        feeds[ i ].setName( generateCollectionName( feeds.at( i ) ) );
    }

    collectionsRetrieved( feeds );
}

void RssResourceBase::slotCollectionChanged( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
    }

    const FeedModifyJob * const mjob = qobject_cast<const FeedModifyJob*>( job );
    Q_ASSERT( mjob );
    changeCommitted( mjob->feed() );

    // clear the cache as that collection has been changed in Akonadi
    FeedCollectionsCache::clear();
}

void RssResourceBase::slotCollectionDeleted( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
    }

    const FeedDeleteJob * const djob = qobject_cast<const FeedDeleteJob*>( job );
    Q_ASSERT( djob );
    changeCommitted( djob->feed() );

    // clear the cache as that collection has been removed from Akonadi
    FeedCollectionsCache::clear();
}

void RssResourceBase::slotItemChanged( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
    }

    const ItemModifyJob * const mjob = qobject_cast<const ItemModifyJob*>( job );
    Q_ASSERT( mjob );
    changeCommitted( mjob->item() );
}

void RssResourceBase::slotFeedAdded( KJob *job )
{
    // return the remote id to the caller
    Q_ASSERT( d->m_replies.contains( job ) );
    QDBusMessage replyMessage = d->m_replies.take( job );
    QVariantMap res;
    if ( job->error() ) {
        kWarning() << "Failed to create a new feed: "<< job->errorString();
        res.insert( "feedId", Feed::Id() );
        res.insert( "errorString", job->errorString() );
        res.insert( "error", job->error() );
    } else {
        const FeedCollectionCreateJob * const cjob = qobject_cast<const FeedCollectionCreateJob*>( job );
        Q_ASSERT( cjob );
        res.insert( "feedId", cjob->feedCollection().id() );
    }
    replyMessage << res;
    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotFeedModified( KJob *job )
{
    Q_ASSERT( d->m_replies.contains( job ) );
    QDBusMessage replyMessage = d->m_replies.take( job );

    if ( job->error() ) {
        kWarning() << "Failed to modify the feed";
        kWarning() << job->errorString();
        replyMessage << false;
    }
    else {
        replyMessage << true;
    }

    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotFeedDeleted( KJob *job )
{
    Q_ASSERT( d->m_replies.contains( job ) );
    QDBusMessage replyMessage = d->m_replies.take( job );

    if ( job->error() ) {
        kWarning() << "Failed to delete the feed";
        kWarning() << job->errorString();
        replyMessage << false;
    }
    else {
        replyMessage << true;
    }

    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotSubscriptionsRetrieved( KJob *job )
{
    Q_ASSERT( d->m_replies.contains( job ) );
    QDBusMessage replyMessage = d->m_replies.take( job );

    if ( job->error() ) {
        kWarning() << "Failed to retrieve the collection";
        kWarning() << job->errorString();
        replyMessage << QStringList();
    }
    else {
        const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
        Q_ASSERT( rjob );
        replyMessage << rjob->feedCollection().subscriptionLabels();
    }

    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotItemsRetrieved( KJob * job )
{
    Q_ASSERT( d->m_fetchJobs.contains( job ) );
    const KRss::Feed::Id id = d->m_fetchJobs.take( job );

    if ( job->error() ) {
        kWarning() << "Failed to retrieve items";
        kWarning() << job->errorString();
        ( job->error() == KJob::KilledJobError ? emit fetchAborted( id ) :
                                                 emit fetchFailed( id, job->errorString() ) );
    }
    else {
        emit fetchFinished( id );
    }

    if ( d->m_fetchJobs.count() == 0 )
        emit fetchQueueFinished();

    itemsRetrievalDone();
}

void RssResourceBase::slotFetchResult( KJob * job )
{
    Q_ASSERT( d->m_fetchJobs.contains( job ) );
    const KRss::Feed::Id id = d->m_fetchJobs.take( job );

    if ( job->error() ) {
        kWarning() << "Failed to fetch the feed";
        kWarning() << job->errorString();
        ( job->error() == KJob::KilledJobError ? emit fetchAborted( id ) :
                                                 emit fetchFailed( id, job->errorString() ) );
    }
    else {
        emit fetchFinished( id );
    }

    if ( d->m_fetchJobs.count() == 0 )
        emit fetchQueueFinished();
}

void RssResourceBase::slotFetchPercent( KJob *job, unsigned long percentage )
{
    Q_ASSERT( d->m_fetchJobs.contains( job ) );
    emit fetchPercent( d->m_fetchJobs.value( job ), percentage );
}

void RssResourceBase::slotOpmlImported( KJob *job )
{
    if ( job->error() ) {
        kWarning() << "Failed to import the feeds";
        kWarning() << job->errorString();
    }

    const ImportOpmlJob* const ijob = qobject_cast<ImportOpmlJob*>( job );
    Q_ASSERT( ijob );
    Q_ASSERT( d->m_replies.contains( job ) );

    QDBusMessage replyMessage = d->m_replies.take( job );
    QVariantMap result;
    const QList<Akonadi::Collection> feeds = ijob->importedFeeds();
    QStringList feedUrls, feedTitles;
    Q_FOREACH( const FeedCollection& feed, feeds ) {
        feedUrls.append( feed.xmlUrl() );
        feedTitles.append( feed.title() );
    }
    result.insert( "error", ijob->error() );
    result.insert( "errorString", ijob->errorString() );
    result.insert( "feedUrls", feedUrls );
    result.insert( "feedTitles", feedTitles );
    replyMessage << result;
    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotOpmlExported( KJob *job )
{
    if ( job->error() ) {
        kWarning() << "Failed to export the feeds";
        kWarning() << job->errorString();
    }

    const ExportOpmlJob* const ejob = qobject_cast<ExportOpmlJob*>( job );
    Q_ASSERT( ejob );
    Q_ASSERT( d->m_replies.contains( job ) );

    QDBusMessage replyMessage = d->m_replies.take( job );
    QVariantMap result;
    result.insert( "error", ejob->error() );
    result.insert( "errorString", ejob->errorString() );
    replyMessage << result;
    QDBusConnection::sessionBus().send( replyMessage );
}

void RssResourceBase::slotItemsImported( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
    }

    Q_ASSERT( d->m_replies.contains( job ) );
    QDBusMessage replyMessage = d->m_replies.take( job );
    QVariantMap result;
    result.insert( "error", job->error() );
    result.insert( "errorString", job->errorString() );
    replyMessage << result;
    QDBusConnection::sessionBus().send( replyMessage );
}

#include "rssresourcebase.moc"
