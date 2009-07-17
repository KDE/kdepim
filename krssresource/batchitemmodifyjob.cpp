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

#include "batchitemmodifyjob.h"

#include <Akonadi/Collection>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemModifyJob>
#include <KLocale>
#include <KDebug>

using namespace KRssResource;

class BatchItemModifyJob::Private
{
public:
    explicit Private( BatchItemModifyJob *qq )
        : q( qq ), m_pendingJobs( 0 ) {}

    void doStart();
    void slotItemsRetrieved( KJob *job );
    void slotItemModified( KJob *job );

    BatchItemModifyJob * const q;
    int m_pendingJobs;
    Akonadi::Collection m_feed;
    boost::function1<bool, Akonadi::Item&> m_modifier;
};

void BatchItemModifyJob::Private::doStart()
{
    Akonadi::ItemFetchJob * const job = new Akonadi::ItemFetchJob( m_feed );
    // we need only flags
    job->fetchScope().fetchFullPayload( false );
    job->fetchScope().fetchAllAttributes( false );
    connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotItemsRetrieved( KJob* ) ) );
    job->start();
}

void BatchItemModifyJob::Private::slotItemsRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( BatchItemModifyJob::CouldNotRetrieveItems );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const Akonadi::ItemFetchJob * const fjob = qobject_cast<const Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fjob );
    QList<Akonadi::Item> items = fjob->items();

    Q_ASSERT( m_modifier );
    Q_FOREACH( Akonadi::Item item, items ) {
        if ( m_modifier( item ) ) {
            m_pendingJobs++;
            Akonadi::ItemModifyJob * const mjob = new Akonadi::ItemModifyJob( item );
            connect( mjob, SIGNAL( result( KJob* ) ), q, SLOT( slotItemModified( KJob* ) ) );
            mjob->start();
        }
    }

    if ( m_pendingJobs == 0 )
        q->emitResult();
}

void BatchItemModifyJob::Private::slotItemModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( BatchItemModifyJob::CouldNotModifyItem );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
    }

    --m_pendingJobs;
    if ( m_pendingJobs == 0 )
        q->emitResult();
}

BatchItemModifyJob::BatchItemModifyJob( QObject *parent )
    : KJob( parent ), d( new Private( this ) )
{
}

BatchItemModifyJob::~BatchItemModifyJob()
{
    delete d;
}

void BatchItemModifyJob::setFeed( const Akonadi::Collection& feed )
{
    d->m_feed = feed;
}

void BatchItemModifyJob::setModifier( const boost::function1<bool, Akonadi::Item&>& modifier )
{
    d->m_modifier = modifier;
}

QString BatchItemModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case BatchItemModifyJob::CouldNotRetrieveItems:
            result = i18n( "Couldn't retrieve the items from Akonadi.\n%1", errorText() );
            break;
        case BatchItemModifyJob::CouldNotModifyItem:
            result = i18n( "Couldn't modify items.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void BatchItemModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

#include "batchitemmodifyjob.moc"
