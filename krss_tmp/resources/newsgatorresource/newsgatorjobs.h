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

#ifndef KRSS_NEWSGATORRESOURCE_NEWSGATORJOBS_H
#define KRSS_NEWSGATORRESOURCE_NEWSGATORJOBS_H

#include "location.h"

#include <krssresource/rssbackendjobs.h>
#include <krss/feedcollection.h>

#include <Syndication/Syndication>
#include <Syndication/DataRetriever>
#include <Akonadi/Collection>
#include <KJob>
#include <QtCore/QString>

namespace KIO {
class Job;
}

class KJob;
class QByteArray;

class SoapMessage;

class NewsgatorJob
{
public:
    void setUserName( const QString& userName );
    void setPassword( const QString& password );
    void setLocation( const Location& location );

protected:
    static const QString m_apiToken;
    QString m_userName;
    QString m_password;
    Location m_location;
};

class NewsgatorTransferJob : public KJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorTransferJob( QObject *parent = 0 );

    void setRequestData( const QByteArray& requestData );
    void setEndpoint( const QString& endpoint );
    void setSoapMessage( SoapMessage* soapMessage );
    SoapMessage* soapMessage() const;

    void start();

    QString errorString() const;
    enum Error {
        CouldNotContactNewsgator = KJob::UserDefinedError,
        CouldNotPassAuthentication,
        CouldNotParseSoapMessage,
        SoapRequestFailed,
        UserDefinedError
    };

private Q_SLOTS:
    void slotData( KIO::Job *job, const QByteArray& data );
    void slotResult( KJob *job );

protected:
    QByteArray m_requestData;
    QString m_endpoint;
    SoapMessage *m_soapMessage;
    Q_DISABLE_COPY( NewsgatorTransferJob )
};

class NewsgatorDataRetriever : public Syndication::DataRetriever, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorDataRetriever();

    void retrieveData( const KUrl& url );
    int errorCode() const;
    QString errorString() const;
    void abort();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    int m_errorCode;
    QString m_errorString;
    Q_DISABLE_COPY( NewsgatorDataRetriever )
};

class NewsgatorLocationsRetrieveJob : public KJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorLocationsRetrieveJob( QObject *parent = 0 );

    QList<Location> locations() const;

    enum Error {
        CouldNotContactNewsgator = KJob::UserDefinedError,
        CouldNotRetrieveLocations,
        UserDefinedError
    };

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    QList<Location> m_locations;
    Q_DISABLE_COPY( NewsgatorLocationsRetrieveJob )
};

class NewsgatorFeedsRetrieveJob : public KRssResource::FeedsRetrieveJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorFeedsRetrieveJob( QObject *parent = 0 );

    QList<Akonadi::Collection> feeds() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    QList<Akonadi::Collection> m_feeds;
    Q_DISABLE_COPY( NewsgatorFeedsRetrieveJob )
};

class NewsgatorFeedCreateJob : public KRssResource::FeedCreateJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorFeedCreateJob( QObject *parent = 0 );

    Akonadi::Collection feed() const;
    void setXmlUrl( const QString& xmlUrl );

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );
    void slotFeedsRetrieved( KJob *job );

private:
    QString m_xmlUrl;
    int m_subscriptionId;
    KRss::FeedCollection m_feed;
    Q_DISABLE_COPY( NewsgatorFeedCreateJob )
};

class NewsgatorFeedModifyJob : public KRssResource::FeedModifyJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorFeedModifyJob( QObject *parent = 0 );

    void setFeed( const Akonadi::Collection& feed );
    Akonadi::Collection feed() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    KRss::FeedCollection m_feed;
    Q_DISABLE_COPY( NewsgatorFeedModifyJob )
};

class NewsgatorFeedDeleteJob : public KRssResource::FeedDeleteJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorFeedDeleteJob( QObject *parent = 0 );

    void setFeed( const Akonadi::Collection& feed );
    Akonadi::Collection feed() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    KRss::FeedCollection m_feed;
    Q_DISABLE_COPY( NewsgatorFeedDeleteJob )
};

class NewsgatorFeedFetchJob : public KRssResource::FeedFetchJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorFeedFetchJob( QObject *parent = 0 );

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
    Q_DISABLE_COPY( NewsgatorFeedFetchJob )
};

class NewsgatorItemModifyJob : public KRssResource::ItemModifyJob, public NewsgatorJob
{
    Q_OBJECT
public:
    explicit NewsgatorItemModifyJob( QObject *parent = 0 );

    void setItem( const Akonadi::Item& item );
    Akonadi::Item item() const;

    QString errorString() const;
    void start();

private Q_SLOTS:
    void slotResult( KJob *job );

private:
    Akonadi::Item m_item;
    Q_DISABLE_COPY( NewsgatorItemModifyJob )
};

#endif // KRSS_NEWSGATORRESOURCE_NEWSGATORJOBS_H
