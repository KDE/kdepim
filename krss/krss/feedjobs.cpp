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

#include "feedjobs.h"
#include "feed.h"
#include "feed_p.h"
#include "tagprovider.h"
#include "tagjobs.h"

#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/collectiondeletejob.h>
#include <KLocale>
#include <KDebug>
#include <boost/shared_ptr.hpp>

using namespace KRss;
using Akonadi::CollectionFetchJob;
using Akonadi::CollectionModifyJob;
using Akonadi::CollectionDeleteJob;
using boost::shared_ptr;

namespace KRss {

class FeedModifyJobPrivate
{
public:
    FeedModifyJobPrivate( const shared_ptr<const Feed>& feed, FeedModifyJob *qq )
        : q( qq ), m_feed( feed )
    {
    }

    void doStart();
    void slotOriginalFeedLoaded( KJob * job );
    void slotFeedModified( KJob *job );
    void slotTagProviderRetrieved( KJob *job );
    void slotReferencesModified( KJob *job );

    FeedModifyJob * const q;
    const shared_ptr<const Feed> m_feed;
    FeedCollection m_originalFeed;
};

class FeedDeleteJobPrivate
{
public:
    FeedDeleteJobPrivate( const shared_ptr<const Feed>& feed, FeedDeleteJob *qq )
        : q( qq ), m_feed( feed )
    {
    }

    void doStart();
    void slotTagProviderRetrieved( KJob *job );
    void slotReferencesDeleted( KJob *job );
    void slotFeedDeleted( KJob *job );

    FeedDeleteJob * const q;
    const shared_ptr<const Feed> m_feed;
};

} // namespace KRss

void FeedModifyJobPrivate::doStart()
{
    CollectionFetchJob *job = new CollectionFetchJob( m_feed->d->m_feedCollection, CollectionFetchJob::Base );
    QObject::connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotOriginalFeedLoaded( KJob* ) ) );
    job->start();
}

void FeedModifyJobPrivate::slotOriginalFeedLoaded( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedModifyJob::CouldNotLoadOriginalFeed );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    const CollectionFetchJob* const fjob = qobject_cast<const CollectionFetchJob*>( job );
    Q_ASSERT( fjob );
    Q_ASSERT( fjob->collections().count() == 1 );
    m_originalFeed = fjob->collections().first();

    CollectionModifyJob *mjob = new CollectionModifyJob( m_feed->d->m_feedCollection );
    QObject::connect( mjob, SIGNAL( result( KJob* ) ), q, SLOT( slotFeedModified( KJob* ) ) );
    mjob->start();
}

void FeedModifyJobPrivate::slotFeedModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedModifyJob::CouldNotModifyFeed );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    TagProviderRetrieveJob *tjob = new TagProviderRetrieveJob();
    QObject::connect( tjob, SIGNAL( result( KJob* ) ), q, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void FeedModifyJobPrivate::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedModifyJob::CouldNotRetrieveTagProvider );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    const TagProviderRetrieveJob * const tjob = qobject_cast<const TagProviderRetrieveJob*>( job );
    Q_ASSERT( tjob );
    const shared_ptr<const TagProvider> tp = tjob->tagProvider();

    const QList<TagId> newTags = m_feed->tags();
    kDebug() << "New tags:" << newTags;
    const QList<TagId> oldTags = m_originalFeed.tags();
    kDebug() << "Old tags:" << oldTags;

    QSet<TagId> toRemove = QSet<TagId>( oldTags.toSet() ).subtract( newTags.toSet() );
    QSet<TagId> toAdd = QSet<TagId>( newTags.toSet() ).subtract( oldTags.toSet() );
    kDebug() << "Tags to remove:" << toRemove;
    kDebug() << "Tags to add:" << toAdd;

    TagModifyReferencesJob *mjob = tp->tagModifyReferencesJob();
    mjob->setReferrer( m_feed.get() );
    mjob->setAddedTags( toAdd.toList() );
    mjob->setRemovedTags( toRemove.toList() );
    QObject::connect( mjob, SIGNAL( result( KJob* ) ), q, SLOT( slotReferencesModified( KJob* ) ) );
    mjob->start();
}

void FeedModifyJobPrivate::slotReferencesModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedModifyJob::CouldNotModifyReferences );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    q->emitResult();
}

FeedModifyJob::FeedModifyJob( const shared_ptr<const Feed>& feed, QObject *parent ) :
    KJob( parent ), d( new FeedModifyJobPrivate( feed, this ) )
{
}

FeedModifyJob::~FeedModifyJob()
{
    delete d;
}

QString FeedModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedModifyJob::CouldNotModifyFeed:
            result = i18n( "Could not modify feed.\n%1", errorText() );
            break;
        case FeedModifyJob::CouldNotLoadOriginalFeed:
            result = i18n( "Could not load the original feed.\n%1", errorText() );
            break;
        case FeedModifyJob::CouldNotRetrieveTagProvider:
            result = i18n( "Could not retrieve the tag provider.\n%1", errorText() );
            break;
        case FeedModifyJob::CouldNotModifyReferences:
            result = i18n( "Could not modify the references inside the tag provider.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}


void FeedDeleteJobPrivate::doStart()
{
    TagProviderRetrieveJob *tjob = new TagProviderRetrieveJob();
    QObject::connect( tjob, SIGNAL( result( KJob* ) ), q, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void FeedDeleteJobPrivate::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedDeleteJob::CouldNotRetrieveTagProvider );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    const TagProviderRetrieveJob * const tjob = qobject_cast<const TagProviderRetrieveJob*>( job );
    Q_ASSERT( tjob );
    const shared_ptr<const TagProvider> tp = tjob->tagProvider();

    TagDeleteReferencesJob *djob = tp->tagDeleteReferencesJob();
    djob->setReferrer( m_feed.get() );
    QObject::connect( djob, SIGNAL( result( KJob* ) ), q, SLOT( slotReferencesDeleted( KJob* ) ) );
    djob->start();
}

void FeedDeleteJobPrivate::slotReferencesDeleted( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedDeleteJob::CouldNotDeleteReferences );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    CollectionDeleteJob *djob = new CollectionDeleteJob( m_feed->d->m_feedCollection );
    QObject::connect( djob, SIGNAL( result( KJob* ) ), q, SLOT( slotFeedDeleted( KJob* ) ) );
    djob->start();
}

void FeedDeleteJobPrivate::slotFeedDeleted( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedDeleteJob::CouldNotDeleteFeed );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    q->emitResult();
}


FeedDeleteJob::FeedDeleteJob( const shared_ptr<const Feed>& feed, QObject *parent ) :
    KJob( parent ), d( new FeedDeleteJobPrivate( feed, this ) )
{
}

FeedDeleteJob::~FeedDeleteJob()
{
    delete d;
}

void FeedDeleteJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

QString FeedDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedDeleteJob::CouldNotDeleteFeed:
            result = i18n( "Could not delete feed.\n%1", errorText() );
            break;
        case FeedDeleteJob::CouldNotRetrieveTagProvider:
            result = i18n( "Could not retrieve the tag provider.\n%1", errorText() );
            break;
        case FeedDeleteJob::CouldNotDeleteReferences:
            result = i18n( "Could not delete the references inside the tag provider.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

#include "feedjobs.moc"
