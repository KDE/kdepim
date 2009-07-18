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

#include "netfeedcreatejob.h"
#include "krssinterface.h"
#include "feedcollection.h"
#include "feedlist.h"
#include "feedlist_p.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>
#include <KLocale>
#include <KDebug>

#include <QtCore/QTimer>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

using namespace KRss;
using boost::weak_ptr;
using boost::shared_ptr;
using Akonadi::CollectionFetchJob;
using Akonadi::Collection;

namespace KRss {

class NetFeedCreateJobPrivate
{
    NetFeedCreateJob* const q;
public:
    explicit NetFeedCreateJobPrivate( const QString &xmlUrl,
                      const QString &subscriptionLabel,
                      const QString &resourceIdentifier,
                      NetFeedCreateJob* qq )
        : q( qq ), m_xmlUrl( xmlUrl ), m_subscriptionLabel( subscriptionLabel ),
          m_resourceIdentifier( resourceIdentifier ), id(0)
    {
    }

    void doStart();
    void slotCallFinished( const QVariantMap& );
    void slotCollectionLoaded( KJob* job );

    const QString m_xmlUrl;
    const QString m_subscriptionLabel;
    const QString m_resourceIdentifier;
    Feed::Id id;
    weak_ptr<FeedList> m_feedList;
    shared_ptr<FeedList> m_sharedFeedList;
};

} // namespace KRss

NetFeedCreateJob::NetFeedCreateJob( const QString &xmlUrl, const QString &subscriptionLabel,
                                    const QString &resourceIdentifier, QObject *parent ) :
    KJob( parent ), d( new NetFeedCreateJobPrivate( xmlUrl, subscriptionLabel, resourceIdentifier, this ) )
{
}

NetFeedCreateJob::~NetFeedCreateJob()
{
    delete d;
}

void NetFeedCreateJob::setFeedList( const weak_ptr<FeedList>& feedList )
{
    d->m_feedList = feedList;
}

Feed::Id NetFeedCreateJob::feedId() const
{
    return d->id;
}

void NetFeedCreateJob::start()
{
    QTimer::singleShot( 0, this, SLOT( doStart() ) );
}

QString NetFeedCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case NetFeedCreateJob::CouldNotCreateFeed:
            result = i18n( "Could not create feed.\n%1", errorText() );
            break;
        case NetFeedCreateJob::CouldNotLoadFeed:
            result = i18n( "Could not load feed.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NetFeedCreateJobPrivate::doStart()
{
    org::kde::krss *interface = new org::kde::krss( "org.freedesktop.Akonadi.Agent." + m_resourceIdentifier, "/KRss",
                                                    QDBusConnection::sessionBus(), q );

    // don't block, set callbacks instead
    QList<QVariant> argumentList;
    argumentList << qVariantFromValue( m_xmlUrl ) << qVariantFromValue( m_subscriptionLabel );
    if ( !interface->callWithCallback( QLatin1String( "addFeed" ), argumentList,
                            q, SLOT( slotCallFinished(QVariantMap) ) ) ) {
        q->setError( NetFeedCreateJob::CouldNotCreateFeed );
        q->setErrorText( "Failed to place a D-Bus call");
        q->emitResult();
    }
}

void NetFeedCreateJobPrivate::slotCallFinished( const QVariantMap& res )
{
    const int error = res.value( "error" ).toInt();
    if ( error ) {
        const QString errorString = res.value( "errorString" ).toString();
        q->setError( NetFeedCreateJob::CouldNotCreateFeed );
        q->setErrorText( errorString );
        q->emitResult();
        return;
    }
    id = res.value( "feedId" ).toLongLong();

    if ( m_feedList.expired() ) {
        q->emitResult();
    }
    else {
        m_sharedFeedList = m_feedList.lock();
        CollectionFetchJob *job = new CollectionFetchJob( Collection( FeedCollection::feedIdToAkonadi( id ) ),
                                                          CollectionFetchJob::Base, q );
        QObject::connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotCollectionLoaded( KJob* ) ) );
        job->start();
    }
}

void NetFeedCreateJobPrivate::slotCollectionLoaded( KJob* job )
{
    if ( job->error() ) {
        q->setError( NetFeedCreateJob::CouldNotLoadFeed );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const CollectionFetchJob* const cj = qobject_cast<const CollectionFetchJob*>( job );
    Q_ASSERT( cj && cj->collections().count() == 1 );
    if ( !m_sharedFeedList->d->m_feeds.contains( id ) ) {
        m_sharedFeedList->d->appendFeed( cj->collections().first() );
    }

    q->emitResult();
}

#include "netfeedcreatejob.moc"
