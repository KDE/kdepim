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

#include "newsgatorjobs.h"
#include "soapmessages.h"

#include <krssresource/util.h>
#include <krss/rssitem.h>
#include <krss/item.h>

#include <KIO/Job>
#include <KLocale>
#include <KDebug>

using namespace KRss;
using namespace KRssResource;

const QString NewsgatorJob::m_apiToken = "E52FEFD1432342698B507EB2DDACF2DF";
static const char separator = '@';

void NewsgatorJob::setUserName( const QString& userName )
{
    m_userName = userName;
}

void NewsgatorJob::setPassword( const QString& password )
{
    m_password = password;
}

void NewsgatorJob::setLocation( const Location& location )
{
    m_location = location;
}



NewsgatorTransferJob::NewsgatorTransferJob( QObject *parent )
    : KJob( parent )
{
}

void NewsgatorTransferJob::setRequestData( const QByteArray& requestData )
{
    m_requestData = requestData;
}

void NewsgatorTransferJob::setEndpoint( const QString& endpoint )
{
    m_endpoint = endpoint;
}

void NewsgatorTransferJob::setSoapMessage( SoapMessage* soapMessage )
{
    m_soapMessage = soapMessage;
}

SoapMessage* NewsgatorTransferJob::soapMessage() const
{
    return m_soapMessage;
}

QString NewsgatorTransferJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorTransferJob::CouldNotContactNewsgator:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorTransferJob::CouldNotPassAuthentication:
            result = i18n( "Authentication failed.\n%1", errorText() );
            break;
        case NewsgatorTransferJob::CouldNotParseSoapMessage:
            result = i18n( "Error occured while paring the SOAP message.\n%1", errorText() );
            break;
        case NewsgatorTransferJob::SoapRequestFailed:
            result = i18n( "SOAP request failed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorTransferJob::start()
{
    Q_ASSERT( m_soapMessage );

    KUrl url( m_endpoint );
    url.setUser( m_userName );
    url.setPass( m_password );
    KIO::TransferJob* job = KIO::http_post( url, m_requestData, KIO::HideProgressInfo );

    job->addMetaData( "UserAgent", "Akonadi Newsgator RSS resource" );
    job->addMetaData( "content-type", "Content-Type: application/soap+xml; charset=utf-8" );

    connect( job, SIGNAL( data( KIO::Job*, const QByteArray& ) ),
             this, SLOT( slotData( KIO::Job*, const QByteArray& ) ) );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
}

void NewsgatorTransferJob::slotData( KIO::Job *job, const QByteArray& data )
{
    Q_UNUSED( job )
    kDebug() << "Got data:" << data;
    m_soapMessage->addData( data );
}

void NewsgatorTransferJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotContactNewsgator );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    m_soapMessage->parse();
    if ( m_soapMessage->hasError() ) {
        kWarning() << m_soapMessage->errorString();
        setError( CouldNotParseSoapMessage );
        setErrorText( m_soapMessage->errorString() );
        emitResult();
        return;
    }

    if ( m_soapMessage->messageType() == SoapMessage::Fault ) {
        kWarning() << m_soapMessage->soapErrorString();
        setError( SoapRequestFailed );
        setErrorText( m_soapMessage->soapErrorString() );
        emitResult();
        return;
    }

    emitResult();
}

NewsgatorDataRetriever::NewsgatorDataRetriever()
    : m_errorCode( 0 ), m_errorString( i18n( "No error.") )
{
}

void NewsgatorDataRetriever::retrieveData( const KUrl& url )
{
    KUrl u = url;
    u.setUser( m_userName );
    u.setPass( m_password );
    u.addQueryItem( "unread", "False" );
    KIO::StoredTransferJob * const job = KIO::storedGet( u, KIO::NoReload, KIO::HideProgressInfo );
    job->addMetaData( "UserAgent", "Akonadi Newsgator RSS resource" );
    job->addMetaData( "customHTTPHeader", QString( "X-NGAPIToken: %1" ).arg( m_apiToken ) );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

int NewsgatorDataRetriever::errorCode() const
{
    return m_errorCode;
}

QString NewsgatorDataRetriever::errorString() const
{
    return m_errorString;
}

void NewsgatorDataRetriever::abort()
{
}

void NewsgatorDataRetriever::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        m_errorCode = job->error();
        m_errorString = job->errorString();
        emit dataRetrieved( QByteArray(), false );
        return;
    }

    const KIO::StoredTransferJob * const sjob = qobject_cast<const KIO::StoredTransferJob*>( job );
    Q_ASSERT( sjob );
    QByteArray data = sjob->data();
    emit dataRetrieved( data, true );
}

NewsgatorLocationsRetrieveJob::NewsgatorLocationsRetrieveJob( QObject *parent )
    : KJob( parent )
{
}

QList<Location> NewsgatorLocationsRetrieveJob::locations() const
{
    return m_locations;
}

QString NewsgatorLocationsRetrieveJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorLocationsRetrieveJob::CouldNotContactNewsgator:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorLocationsRetrieveJob::CouldNotRetrieveLocations:
            result = i18n( "Couldn't retrieve locations.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorLocationsRetrieveJob::start()
{
    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/svc/Location.asmx" );
    request.writeRequestStart();
    request.writeEmptyElement( "http://services.newsgator.com/svc/Location.asmx", "GetLocations" );
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/Location.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new LocationsResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorLocationsRetrieveJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotRetrieveLocations );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const NewsgatorTransferJob * const njob = qobject_cast<const NewsgatorTransferJob*>( job );
    Q_ASSERT( njob );
    const LocationsResponse * const response = dynamic_cast<const LocationsResponse*>( njob->soapMessage() );
    Q_ASSERT( response );

    m_locations = response->locations();
    emitResult();
}



NewsgatorFeedsRetrieveJob::NewsgatorFeedsRetrieveJob( QObject *parent )
    : FeedsRetrieveJob( parent )
{
}

QList<Akonadi::Collection> NewsgatorFeedsRetrieveJob::feeds() const
{
    return m_feeds;
}

QString NewsgatorFeedsRetrieveJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorFeedsRetrieveJob::CouldNotContactBackend:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorFeedsRetrieveJob::CouldNotRetrieveFeeds:
            result = i18n( "Couldn't retrieve feeds.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorFeedsRetrieveJob::start()
{
    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeRequestStart();
    request.writeStartElement( "GetSubscriptionList" );
    request.writeDefaultNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeTextElement( "location", m_location.name );
    request.writeEmptyElement( "syncToken" );
    request.writeTextElement( "getScores", "false" );
    request.writeEndElement(); // GetSubscriptionList
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/Subscription.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new SubscriptionsResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorFeedsRetrieveJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotRetrieveFeeds );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const NewsgatorTransferJob * const njob = qobject_cast<const NewsgatorTransferJob*>( job );
    Q_ASSERT( njob );
    const SubscriptionsResponse * const response = dynamic_cast<const SubscriptionsResponse*>( njob->soapMessage() );
    Q_ASSERT( response );

    // create a top-level collection representing the requested NewsGator location
    Akonadi::Collection loc;
    loc.setRemoteId( m_userName + QChar( '-' ) + m_location.name );
    loc.setName( loc.remoteId() );
    loc.setParent( Akonadi::Collection::root() );
    loc.setContentMimeTypes( QStringList( Akonadi::Collection::mimeType() ) );
    m_feeds.append( loc );

    QHashIterator<int, ParsedFeed> f_it( response->feeds() );
    while ( f_it.hasNext() ) {
        f_it.next();
        kDebug() << "Key:" << f_it.key() << ", feed:" << f_it.value().xmlUrl;

        const ParsedFeed parsed = f_it.value();
        FeedCollection feed;

        feed.setTitle( parsed.title );
        feed.setXmlUrl( parsed.attributes.value( "syncXmlUrl" ) );
        feed.setHtmlUrl( parsed.htmlUrl );
        feed.setDescription( parsed.description );
        feed.setFeedType( parsed.type );
        feed.setRemoteId( parsed.attributes.value( "id" ) );
        feed.setName( feed.title() + QChar( '-' ) + feed.remoteId() );
        feed.setParent( loc );
        feed.setContentMimeTypes( QStringList( "application/rss+xml" ) );
        m_feeds.append( feed );
    }

    QHashIterator<QString, QList<int> > t_it( response->tags() );
    while ( t_it.hasNext() ) {
        t_it.next();
        kDebug() << "Tag:" << t_it.key() << ", feeds:" << t_it.value();
    }

    emitResult();
}



NewsgatorFeedCreateJob::NewsgatorFeedCreateJob( QObject *parent )
    : FeedCreateJob( parent ), m_subscriptionId( 0 )
{
}

Akonadi::Collection NewsgatorFeedCreateJob::feed() const
{
    return m_feed;
}

void NewsgatorFeedCreateJob::setXmlUrl( const QString& xmlUrl )
{
    m_xmlUrl = xmlUrl;
}

QString NewsgatorFeedCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorFeedCreateJob::CouldNotContactBackend:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorFeedCreateJob::CouldNotCreateFeed:
            result = i18n( "Couldn't create feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorFeedCreateJob::start()
{
    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeRequestStart();
    request.writeStartElement( "AddSubscription" );
    request.writeDefaultNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeTextElement( "xmlUrl", m_xmlUrl );
    request.writeTextElement( "folderId", "0" );
    request.writeEmptyElement( "cred" );
    request.writeEmptyElement( "customTitle" );
    request.writeTextElement( "markRead", "false" );
    request.writeTextElement( "locationId", QString::number( m_location.id ) );
    request.writeEndElement(); // AddSubscription
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/Subscription.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new AddSubscriptionResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorFeedCreateJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotCreateFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const NewsgatorTransferJob * const njob = qobject_cast<const NewsgatorTransferJob*>( job );
    Q_ASSERT( njob );
    const AddSubscriptionResponse * const response = dynamic_cast<const AddSubscriptionResponse*>( njob->soapMessage() );
    Q_ASSERT( response );

    m_subscriptionId = response->subscriptionId();
    kDebug() << "Got subscription id:" << m_subscriptionId;

    NewsgatorFeedsRetrieveJob *rjob = new NewsgatorFeedsRetrieveJob();
    rjob->setUserName( m_userName );
    rjob->setPassword( m_password );
    rjob->setLocation( m_location );
    connect( rjob, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsRetrieved( KJob* ) ) );
    rjob->start();
}

void NewsgatorFeedCreateJob::slotFeedsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotCreateFeed );
        setErrorText( i18n( "Error occured while retrieving the subscription list: %1",
                      job->errorString() ) );
        emitResult();
        return;
    }

    const NewsgatorFeedsRetrieveJob * const rjob = qobject_cast<const NewsgatorFeedsRetrieveJob*>( job );
    Q_ASSERT( rjob );
    const QList<Akonadi::Collection> cols = rjob->feeds();
    Q_FOREACH( const Akonadi::Collection& col, cols ) {
        if ( col.remoteId() == QString::number( m_subscriptionId ) ) {
            kDebug() << "Found the created subscription:" << col.name();
            kDebug() << "Parent id:" << col.parent();
            kDebug() << "Parent remote id:" << col.parentRemoteId();
            m_feed = col;
            emitResult();
            return;
        }
    }

    setError( CouldNotCreateFeed );
    setErrorText( i18n( "Couldn't find the created feed with id %1 in the subscription list",
                  m_subscriptionId ) );
    kWarning() << errorString();
    emitResult();
}



NewsgatorFeedModifyJob::NewsgatorFeedModifyJob( QObject *parent )
    : FeedModifyJob( parent )
{
}

void NewsgatorFeedModifyJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection NewsgatorFeedModifyJob::feed() const
{
    return m_feed;
}

QString NewsgatorFeedModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorFeedModifyJob::CouldNotContactBackend:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorFeedModifyJob::CouldNotModifyFeed:
            result = i18n( "Couldn't modify feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorFeedModifyJob::start()
{
    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeRequestStart();
    request.writeStartElement( "RenameSubscription" );
    request.writeDefaultNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeTextElement( "subscriptionId", m_feed.remoteId() );
    request.writeTextElement( "newName", m_feed.title() );
    request.writeTextElement( "locationId", QString::number( m_location.id ) );
    request.writeEndElement(); // RenameSubscription
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/Subscription.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new RenameSubscriptionResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorFeedModifyJob::slotResult( KJob *job )
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



NewsgatorFeedDeleteJob::NewsgatorFeedDeleteJob( QObject *parent )
    : FeedDeleteJob( parent )
{
}

void NewsgatorFeedDeleteJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection NewsgatorFeedDeleteJob::feed() const
{
    return m_feed;
}

QString NewsgatorFeedDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorFeedDeleteJob::CouldNotContactBackend:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorFeedDeleteJob::CouldNotDeleteFeed:
            result = i18n( "Couldn't delete feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorFeedDeleteJob::start()
{
    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeRequestStart();
    request.writeStartElement( "DeleteSubscriptions" );
    request.writeDefaultNamespace( "http://services.newsgator.com/svc/Subscription.asmx" );
    request.writeStartElement( "subscriptionList" );
    request.writeTextElement( "int", m_feed.remoteId() );
    request.writeEndElement(); // subscriptionList
    request.writeTextElement( "locationId", QString::number( m_location.id ) );
    request.writeEndElement(); // RenameSubscription
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/Subscription.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new DeleteSubscriptionResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorFeedDeleteJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotDeleteFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const NewsgatorTransferJob * const njob = qobject_cast<const NewsgatorTransferJob*>( job );
    Q_ASSERT( njob );
    const DeleteSubscriptionResponse * const response = dynamic_cast<const DeleteSubscriptionResponse*>(
                                                                           njob->soapMessage() );
    Q_ASSERT( response );

    if ( response->statusCode() != 0 ) {
        setError( CouldNotDeleteFeed );
        setErrorText( i18n( "Response error code: %1").arg( response->statusCode() ) );
        kWarning() << errorText();
        emitResult();
        return;
    }

    emitResult();
}


NewsgatorFeedFetchJob::NewsgatorFeedFetchJob( QObject *parent )
    : FeedFetchJob( parent )
{
}

void NewsgatorFeedFetchJob::setFeed( const Akonadi::Collection& feed )
{
    m_feed = feed;
}

Akonadi::Collection NewsgatorFeedFetchJob::feed() const
{
    return m_feed;
}

QList<Akonadi::Item> NewsgatorFeedFetchJob::items() const
{
    return m_items;
}

QString NewsgatorFeedFetchJob::errorString() const
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

void NewsgatorFeedFetchJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void NewsgatorFeedFetchJob::doStart()
{
    Syndication::Loader * const loader = Syndication::Loader::create();
    connect( loader, SIGNAL( loadingComplete( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ),
             this, SLOT( slotFeedFetched( Syndication::Loader*, Syndication::FeedPtr, Syndication::ErrorCode ) ) );

    NewsgatorDataRetriever * const dr = new NewsgatorDataRetriever();
    dr->setUserName( m_userName );
    dr->setPassword( m_password );
    loader->loadFrom( m_feed.xmlUrl(), dr );
}

void NewsgatorFeedFetchJob::slotFeedFetched( Syndication::Loader *loader, Syndication::FeedPtr feed,
                                        Syndication::ErrorCode status )
{
    Q_UNUSED( loader )
    kDebug() << "Status:" << status;

    if ( status == Syndication::Success ) {
        const QList<Syndication::ItemPtr> syndItems = feed->items();
        Q_FOREACH( const Syndication::ItemPtr& syndItem, syndItems ) {
            Akonadi::Item item;
            KRss::RssItem rssItem = KRssResource::fromSyndicationItem( syndItem );
            item.setPayload<KRss::RssItem>( rssItem );
            item.setRemoteId( rssItem.customProperty( "http://newsgator.com/schema/extensionspostId" ) +
                              QChar( separator ) + m_feed.remoteId() );
            item.setMimeType( KRss::Item::mimeType() );

            // handle the item's flags
            if ( rssItem.customProperty( "http://newsgator.com/schema/extensionsread" ) == "False" ) {
                item.setFlag( KRss::RssItem::flagNew() );
            } else {
                item.setFlag( KRss::RssItem::flagRead() );
            }

            if ( rssItem.customProperties().contains( "http://newsgator.com/schema/extensionsflagState" ) ) {
                item.setFlag( KRss::RssItem::flagImportant() );
            }

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


NewsgatorItemModifyJob::NewsgatorItemModifyJob( QObject *parent )
    : ItemModifyJob( parent )
{
}

void NewsgatorItemModifyJob::setItem( const Akonadi::Item& item )
{
    m_item = item;
}

Akonadi::Item NewsgatorItemModifyJob::item() const
{
    return m_item;
}

QString NewsgatorItemModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NewsgatorItemModifyJob::CouldNotContactBackend:
            result = i18n( "Couldn't contact the Newsgator service.\n%1", errorText() );
            break;
        case NewsgatorItemModifyJob::CouldNotModifyItem:
            result = i18n( "Couldn't modify item\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NewsgatorItemModifyJob::start()
{
    const QStringList ids = m_item.remoteId().split( QChar( separator ) );

    QByteArray requestData;
    SoapRequest request( &requestData );
    request.setApiToken( m_apiToken );
    request.setHeaderNamespace( "http://services.newsgator.com/ngws/svc/PostItem.asmx" );
    request.writeRequestStart();
    request.writeStartElement( "UpdatePostMetadatav3" );
    request.writeDefaultNamespace( "http://services.newsgator.com/ngws/svc/PostItem.asmx" );
    request.writeTextElement( "locationName", m_location.name );
    request.writeTextElement( "updateRelevanceScores", "false" );
    request.writeStartElement( "newStates" );
    request.writeStartElement( "FeedMetadata" );
    request.writeTextElement( "FeedID", ids.at( 1 ) );
    request.writeStartElement( "PostMetadata" );
    request.writeStartElement( "PostMetadata" );
    request.writeTextElement( "PostID", ids.at( 0 ) );
    request.writeTextElement( "State", m_item.hasFlag( KRss::RssItem::flagRead() ) ? "1" : "0" );
    request.writeTextElement( "FlagState", m_item.hasFlag( KRss::RssItem::flagImportant() ) ? "1" : "0" );
    request.writeEndElement();  // PostMetadata
    request.writeEndElement();  // PostMetadata
    request.writeEndElement();  // FeedMetadata
    request.writeEndElement();  // newStates
    request.writeEndElement();  // UpdatePostMetadatav3
    request.writeRequestEnd();

    kDebug() << "Request data:" << requestData;

    NewsgatorTransferJob *job = new NewsgatorTransferJob();
    job->setUserName( m_userName );
    job->setPassword( m_password );
    job->setEndpoint( "http://services.newsgator.com/ngws/svc/PostItem.asmx" );
    job->setRequestData( requestData );
    job->setSoapMessage( new UpdatePostResponse() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotResult( KJob* ) ) );
    job->start();
}

void NewsgatorItemModifyJob::slotResult( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotModifyItem );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}

#include "newsgatorjobs.moc"
