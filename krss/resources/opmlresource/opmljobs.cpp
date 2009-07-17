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

#include "opmljobs.h"

#include <krss/rssitem.h>
#include <krss/item.h>
#include <krssresource/opmlparser.h>
#include <krssresource/util.h>

#include <KIO/Job>
#include <KLocale>
#include <KDebug>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QUuid>

using namespace KRss;
using namespace KRssResource;

namespace {

static QList<ParsedFeed> toParsedFeedList( const QList<Akonadi::Collection>& feeds )
{
    QList<ParsedFeed> parsedFeeds;
    Q_FOREACH( const Akonadi::Collection& feed, feeds ) {
        if ( feed.parent() != Akonadi::Collection::root().id() )
            parsedFeeds.append( ParsedFeed::fromAkonadiCollection( feed ) );
    }
    return parsedFeeds;
}

} // namespace

void OpmlJob::setPath( const KUrl& path )
{
    m_path = path;
}

// cache to store the feeds read from OPML
QHash<QString, Akonadi::Collection> OpmlFeedsRetrieveJob::m_feedsCache = QHash<QString, Akonadi::Collection>();
bool OpmlFeedsRetrieveJob::m_isCached = false;

OpmlFeedsRetrieveJob::OpmlFeedsRetrieveJob( QObject *parent )
    : FeedsRetrieveJob( parent )
{
}

QList<Akonadi::Collection> OpmlFeedsRetrieveJob::feeds() const
{
    return m_feedsCache.values();
}

QString OpmlFeedsRetrieveJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedsRetrieveJob::CouldNotContactBackend:
            result = i18n( "Couldn't read the OPML document.\n%1", errorText() );
            break;
        case FeedsRetrieveJob::CouldNotRetrieveFeeds:
            result = i18n( "Couldn't retrieve feeds.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedsRetrieveJob::start()
{
    if ( m_isCached )
        QMetaObject::invokeMethod( this, "emitCachedResult", Qt::QueuedConnection );
    else
        QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedsRetrieveJob::doStart()
{
    KIO::TransferJob * const job = KIO::storedGet( m_path, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotGetFinished( KJob* ) ) );
    job->start();
}

void OpmlFeedsRetrieveJob::emitCachedResult()
{
    emitResult();
}

void OpmlFeedsRetrieveJob::slotGetFinished( KJob *job )
{
    if ( job->error() == KIO::ERR_DOES_NOT_EXIST ) {
        const char emptyOpml[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<opml version=\"2.0\">\n"
                                 "    <head/>\n    <body/>\n</opml>\n";
        KIO::TransferJob * const njob = KIO::storedPut( emptyOpml, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
        connect( njob, SIGNAL( result( KJob* ) ), this, SLOT( slotNewPutFinished( KJob* ) ) );
        njob->start();
        return;
    }
    else if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const KIO::StoredTransferJob * const gjob = qobject_cast<const KIO::StoredTransferJob*>( job );
    Q_ASSERT( gjob );

    QXmlStreamReader reader( gjob->data() );
    OpmlReader opmlReader;
    while ( !reader.atEnd() ) {
        reader.readNext();

        if ( reader.isStartElement() ) {
            if ( reader.name().toString().toLower() == "opml" ) {
                kDebug() << "OPML version" << reader.attributes().value( "version" ).toString();
                opmlReader.readOpml( reader );
            }
            else {
                reader.raiseError( i18n ( "The file is not an valid OPML document." ) );
            }
        }
    }

    // create a top-level collection
    Akonadi::Collection top;
    top.setRemoteId( m_path.url() );
    top.setName( m_path.url() );
    top.setParent( Akonadi::Collection::root() );
    top.setContentMimeTypes( QStringList( Akonadi::Collection::mimeType() ) );
    m_feedsCache.insert( top.remoteId(), top );

    bool needsPut = false;
    const QList<ParsedFeed> parsedFeeds = opmlReader.feeds();
    Q_FOREACH( const ParsedFeed& parsedFeed, parsedFeeds ) {
        FeedCollection feed = parsedFeed.toAkonadiCollection();
        if ( feed.remoteId().isEmpty() ) {
            feed.setRemoteId( QUuid::createUuid().toString() );
            needsPut = true;
        }
        feed.setParent( top );

        m_feedsCache.insert( feed.remoteId(), feed );
    }

    m_isCached = true;

    if ( !needsPut ) {
        emitResult();
        return;
    }

    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, toParsedFeedList( m_feedsCache.values() ) );
    writer.writeEndDocument();

    kDebug() << "I'd write:" << data;
    KIO::TransferJob * const  sjob = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    sjob->start();
}

void OpmlFeedsRetrieveJob::slotNewPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    doStart();
}

void OpmlFeedsRetrieveJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

OpmlFeedsImportJob::OpmlFeedsImportJob( QObject *parent )
    : FeedsImportJob( parent )
{
}

void OpmlFeedsImportJob::setFeeds( const QList<Akonadi::Collection>& feeds )
{
    m_feeds = feeds;
}

QList<Akonadi::Collection> OpmlFeedsImportJob::feeds() const
{
    return m_feeds;
}

QString OpmlFeedsImportJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case OpmlFeedsImportJob::CouldNotContactBackend:
            result = i18n( "Couldn't read the OPML document.\n%1", errorText() );
            break;
        case OpmlFeedsImportJob::CouldNotImportFeeds:
            result = i18n( "Couldn't import feeds.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedsImportJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedsImportJob::doStart()
{
    OpmlFeedsRetrieveJob * const job = new OpmlFeedsRetrieveJob();
    job->setPath( m_path );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsRetrieved( KJob* ) ) );
    job->start();
}

void OpmlFeedsImportJob::slotFeedsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const int listSize = m_feeds.count();
    for( int i = 0; i < listSize; ++i ) {
        m_feeds[ i ].setRemoteId( QUuid::createUuid().toString() );
        OpmlFeedsRetrieveJob::m_feedsCache.insert( m_feeds.at( i ).remoteId(), m_feeds.at( i ) );
    }

    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, toParsedFeedList( OpmlFeedsRetrieveJob::m_feedsCache.values() ) );
    writer.writeEndDocument();

    KIO::TransferJob* pjob = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( pjob, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    pjob->start();
}

void OpmlFeedsImportJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotImportFeeds );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

OpmlFeedCreateJob::OpmlFeedCreateJob( QObject *parent )
    : FeedCreateJob( parent ), m_fetchAttempts( 0 )
{
}

void OpmlFeedCreateJob::setXmlUrl( const QString& xmlUrl )
{
    m_xmlUrl = KUrl( xmlUrl );
}

Akonadi::Collection OpmlFeedCreateJob::feed() const
{
    return m_feed;
}

QString OpmlFeedCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCreateJob::CouldNotContactBackend:
            result = i18n( "Couldn't retrieve feeds.\n%1", errorText() );
            break;
        case FeedCreateJob::CouldNotCreateFeed:
            result = i18n( "Couldn't create new feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedCreateJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedCreateJob::doStart()
{
    OpmlFeedsRetrieveJob * const job = new OpmlFeedsRetrieveJob();
    job->setPath( m_path );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsRetrieved( KJob* ) ) );
    job->start();
}

void OpmlFeedCreateJob::slotFeedsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    Syndication::Loader *loader = Syndication::Loader::create();
    connect( loader, SIGNAL( loadingComplete( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ),
             this, SLOT( slotFeedFetched( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ) );
    loader->loadFrom( m_xmlUrl );
}

void OpmlFeedCreateJob::slotFeedFetched( Syndication::Loader *loader, Syndication::FeedPtr feedPtr,
                                         Syndication::ErrorCode status )
{
    kDebug() << "Status:" << status;

    // discovered a feed url inside the loaded data?
    if ( ( status == Syndication::InvalidXml ) && ( m_fetchAttempts < 3 ) &&
         ( loader->discoveredFeedURL().isValid() ) ) {
        m_xmlUrl = loader->discoveredFeedURL();
        kDebug() << "Refetching by discovered feed url:" << m_xmlUrl;
        ++m_fetchAttempts;
        doStart();
        return;
    }

    m_feed.setRemoteId( QUuid::createUuid().toString() );
    m_feed.setFeedType( "rss" );
    m_feed.setXmlUrl( m_xmlUrl.url() );

    if ( status == Syndication::Success ) {
        m_feed.setName( feedPtr->title() );
        m_feed.setTitle( feedPtr->title() );
        m_feed.setHtmlUrl( feedPtr->link() );
        m_feed.setDescription( feedPtr->description() );
    } else {
        //PENDING(frank) review this, ideally one could use the properties from the opml in the opml import case...
        m_feed.setName( m_xmlUrl.url() );
        m_feed.setTitle( m_xmlUrl.url() );
    }
    OpmlFeedsRetrieveJob::m_feedsCache.insert( m_feed.remoteId(), m_feed );
    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, toParsedFeedList( OpmlFeedsRetrieveJob::m_feedsCache.values() ) );
    writer.writeEndDocument();

    kDebug() << "I'd write:" << data;
    KIO::TransferJob* job = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    job->start();


#if 0 //PENDING(frank) it should be possible to add a feed even if offline, so i think we should not report this as error
    // we failed to fetch feed
    // Error strings shamelessly taken from Syndication API docs
    setError( CouldNotCreateFeed );
    switch ( status ) {
        case Syndication::Aborted:
                setErrorText( i18n( "File downloading/parsing was aborted." ) );
                break;
        case Syndication::Timeout:
                setErrorText( i18n( "File download timed out." ) );
                break;
        case Syndication::UnknownHost:
                setErrorText( i18n( "The hostname couldn't get resolved to an IP address." ) );
                break;
        case Syndication::FileNotFound:
                setErrorText( i18n( "The host was contacted successfully, but reported a 404 error." ) );
                break;
        case Syndication::OtherRetrieverError:
                setErrorText( i18n( "Retriever error not covered by the error codes above." ) );
                break;
        case Syndication::InvalidXml:
                setErrorText( i18n( "The XML is invalid." ) );
                break;
        case Syndication::XmlNotAccepted:
                setErrorText( i18n( "The source is valid XML, but no parser accepted it." ) );
                break;
        case Syndication::InvalidFormat:
                setErrorText( i18n( "The source was accepted by a parser, but the actual parsing failed." ) );
                break;
        default:
                setErrorText( i18n( "Unknown error occured when fetching the feed" ) );
    }

    kWarning() << errorText();
    emitResult();
#endif
}

void OpmlFeedCreateJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotCreateFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

OpmlFeedModifyJob::OpmlFeedModifyJob( QObject *parent )
    : FeedModifyJob( parent )
{
}

void OpmlFeedModifyJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection OpmlFeedModifyJob::feed() const
{
    return m_feed;
}

QString OpmlFeedModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedModifyJob::CouldNotContactBackend:
            result = i18n( "Couldn't retrieve feeds.\n%1", errorText() );
            break;
        case FeedModifyJob::CouldNotModifyFeed:
            result = i18n( "Couldn't modify feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedModifyJob::doStart()
{
    OpmlFeedsRetrieveJob * const job = new OpmlFeedsRetrieveJob();
    job->setPath( m_path );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsRetrieved( KJob* ) ) );
    job->start();
}

void OpmlFeedModifyJob::slotFeedsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    OpmlFeedsRetrieveJob::m_feedsCache.insert( m_feed.remoteId(), m_feed );
    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, toParsedFeedList( OpmlFeedsRetrieveJob::m_feedsCache.values() ) );
    writer.writeEndDocument();

    kDebug() << "I'd write:" << data;
    KIO::TransferJob * const sjob = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    sjob->start();
}

void OpmlFeedModifyJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotModifyFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

OpmlFeedDeleteJob::OpmlFeedDeleteJob( QObject *parent )
    : FeedDeleteJob( parent )
{
}

void OpmlFeedDeleteJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection OpmlFeedDeleteJob::feed() const
{
    return m_feed;
}

QString OpmlFeedDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedDeleteJob::CouldNotContactBackend:
            result = i18n( "Couldn't retrieve feeds.\n%1", errorText() );
            break;
        case FeedDeleteJob::CouldNotDeleteFeed:
            result = i18n( "Couldn't delete feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedDeleteJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedDeleteJob::doStart()
{
    OpmlFeedsRetrieveJob * const job = new OpmlFeedsRetrieveJob();
    job->setPath( m_path );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsRetrieved( KJob* ) ) );
    job->start();
}

void OpmlFeedDeleteJob::slotFeedsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactBackend );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    OpmlFeedsRetrieveJob::m_feedsCache.remove( m_feed.remoteId() );
    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    OpmlWriter::writeOpml( writer, toParsedFeedList( OpmlFeedsRetrieveJob::m_feedsCache.values() ) );
    writer.writeEndDocument();

    kDebug() << "I'd write:" << data;
    KIO::TransferJob * const sjob = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    sjob->start();
}

void OpmlFeedDeleteJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotDeleteFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

OpmlFeedFetchJob::OpmlFeedFetchJob( QObject *parent )
    : FeedFetchJob( parent )
{
}

void OpmlFeedFetchJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection OpmlFeedFetchJob::feed() const
{
    return m_feed;
}

QList<Akonadi::Item> OpmlFeedFetchJob::items() const
{
    return m_items;
}

QString OpmlFeedFetchJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedFetchJob::CouldNotFetchFeed:
            result = i18n( "Couldn't fetch feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlFeedFetchJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlFeedFetchJob::doStart()
{
    Syndication::Loader * const loader = Syndication::Loader::create();
    connect( loader, SIGNAL( loadingComplete( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ),
             this, SLOT( slotFeedFetched( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ) );
    loader->loadFrom( m_feed.xmlUrl() );
}

void OpmlFeedFetchJob::slotFeedFetched( Syndication::Loader *loader, Syndication::FeedPtr feed,
                                        Syndication::ErrorCode status )
{
    Q_UNUSED( loader )
    kDebug() << "Status:" << status;

    if ( status == Syndication::Success ) {
        const QList<Syndication::ItemPtr> syndItems = feed->items();
        Q_FOREACH( const Syndication::ItemPtr& syndItem, syndItems ) {
            Akonadi::Item item;
            item.setRemoteId( syndItem->id() );
            item.setMimeType( KRss::Item::mimeType() );
            item.setPayload<KRss::RssItem>( KRssResource::fromSyndicationItem( syndItem ) );
            item.setFlag( KRss::RssItem::flagNew() );
            m_items.append( item );
        }
        emitResult();
        return;
    }

    // we failed to fetch feed
    // Error strings shamelessly taken from Syndication API docs
    setError( FeedFetchJob::CouldNotFetchFeed );
    switch ( status ) {
        case Syndication::Aborted:
                setErrorText( i18n( "File downloading/parsing was aborted." ) );
                break;
        case Syndication::Timeout:
                setErrorText( i18n( "File download timed out." ) );
                break;
        case Syndication::UnknownHost:
                setErrorText( i18n( "The hostname couldn't get resolved to an IP address." ) );
                break;
        case Syndication::FileNotFound:
                setErrorText( i18n( "The host was contacted successfully, but reported a 404 error." ) );
                break;
        case Syndication::OtherRetrieverError:
                setErrorText( i18n( "Retriever error not covered by the error codes above." ) );
                break;
        case Syndication::InvalidXml:
                setErrorText( i18n( "The XML is invalid." ) );
                break;
        case Syndication::XmlNotAccepted:
                setErrorText( i18n( "The source is valid XML, but no parser accepted it." ) );
                break;
        case Syndication::InvalidFormat:
                setErrorText( i18n( "The source was accepted by a parser, but the actual parsing failed." ) );
                break;
        default:
                setErrorText( i18n( "Unknown error occured when fetching the feed" ) );
    }

    kWarning() << errorText();
    emitResult();
}

OpmlItemModifyJob::OpmlItemModifyJob( QObject *parent )
    : ItemModifyJob( parent )
{
}

void OpmlItemModifyJob::setItem( const Akonadi::Item& item )
{
    m_item = item;
}

Akonadi::Item OpmlItemModifyJob::item() const
{
    return m_item;
}

QString OpmlItemModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void OpmlItemModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void OpmlItemModifyJob::doStart()
{
    emitResult();
}

#include "opmljobs.moc"
