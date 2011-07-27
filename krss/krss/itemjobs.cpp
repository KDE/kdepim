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

#include "itemjobs.h"
#include "item_p.h"
#include "tagidsattribute.h"
#include "tagprovider.h"
#include "tagjobs.h"

#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemDeleteJob>
#include <KLocale>
#include <KDebug>
#include <boost/shared_ptr.hpp>

using namespace KRss;
using boost::shared_ptr;

class KRss::ItemFetchJobPrivate
{
public:
    explicit ItemFetchJobPrivate( ItemFetchJob *qq )
        : q( qq ) {}

    void doStart();
    void slotItemFetched( KJob *job );

    ItemFetchJob * const q;
    Item m_item;
    Akonadi::ItemFetchScope m_fetchScope;
};

void ItemFetchJobPrivate::doStart()
{
    Akonadi::ItemFetchJob * const job = new Akonadi::ItemFetchJob( m_item.d->akonadiItem );
    q->connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotItemFetched( KJob* ) ) );
    job->setFetchScope( m_fetchScope );
    job->start();
}

void ItemFetchJobPrivate::slotItemFetched( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemFetchJob::CouldNotFetchItem );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const Akonadi::ItemFetchJob * const fjob = qobject_cast<const Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fjob );
    Q_ASSERT( fjob->items().count() == 1 );
    m_item.d->akonadiItem = fjob->items().first();
    q->emitResult();
}

ItemFetchJob::ItemFetchJob( QObject *parent )
    : KJob( parent ), d ( new ItemFetchJobPrivate( this ) )
{
}

ItemFetchJob::~ItemFetchJob()
{
    delete d;
}

void ItemFetchJob::setItem( const Item& item )
{
    d->m_item = item;
}

Item ItemFetchJob::item() const
{
    return d->m_item;
}

void ItemFetchJob::setFetchScope( const Akonadi::ItemFetchScope& fetchScope )
{
    d->m_fetchScope = fetchScope;
}

Akonadi::ItemFetchScope& ItemFetchJob::fetchScope() const
{
    return d->m_fetchScope;
}

QString ItemFetchJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case ItemFetchJob::CouldNotFetchItem:
            result = i18n( "Could not fetch item.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void ItemFetchJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

class KRss::ItemModifyJobPrivate
{
public:
    explicit ItemModifyJobPrivate( ItemModifyJob *qq )
        : q( qq ), m_ignorePayload( false ) {}

    void doStart();
    void slotOldItemFetched( KJob *job );
    void slotItemModified( KJob *job );
    void slotTagProviderRetrieved( KJob *job );
    void slotReferencesModified( KJob *job );

    ItemModifyJob * const q;
    bool m_ignorePayload;
    Item m_itemToModify;
    Item m_oldItem;
};

void ItemModifyJobPrivate::doStart()
{
    Akonadi::ItemFetchJob * const job = new Akonadi::ItemFetchJob( m_itemToModify.d->akonadiItem );
    job->fetchScope().fetchFullPayload( false );
    job->fetchScope().fetchAllAttributes( false );
    job->fetchScope().fetchAttribute<TagIdsAttribute>( true );
    q->connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotOldItemFetched( KJob* ) ) );
    job->start();
}

void ItemModifyJobPrivate::slotOldItemFetched( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemModifyJob::CouldNotFetchOldItem );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    // save the old item
    const Akonadi::ItemFetchJob * const fjob = qobject_cast<const Akonadi::ItemFetchJob*>( job );
    Q_ASSERT( fjob );
    Q_ASSERT( fjob->items().count() == 1 );
    m_oldItem.d->akonadiItem = fjob->items().first();

    Akonadi::ItemModifyJob * const mjob = new Akonadi::ItemModifyJob( m_itemToModify.d->akonadiItem );
    mjob->setIgnorePayload( m_ignorePayload );
    q->connect( mjob, SIGNAL( result( KJob* ) ), q, SLOT( slotItemModified( KJob* ) ) );
    job->start();
}

void ItemModifyJobPrivate::slotItemModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemModifyJob::CouldNotModifyItem );
        q->setErrorText( job->errorString() );
        kWarning() << job->errorString();
        q->emitResult();
        return;
    }

    const Akonadi::ItemModifyJob * const mjob = qobject_cast<const Akonadi::ItemModifyJob*>( job );
    Q_ASSERT( mjob );
    m_itemToModify.d->akonadiItem = mjob->item();

    TagProviderRetrieveJob * const tjob = new TagProviderRetrieveJob();
    q->connect( tjob, SIGNAL( result( KJob* ) ), q, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void ItemModifyJobPrivate::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemModifyJob::CouldNotRetrieveTagProvider );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const QList<TagId> newTags = m_itemToModify.tags();
    const QList<TagId> oldTags = m_oldItem.tags();

    QSet<TagId> toRemove = QSet<TagId>( oldTags.toSet() ).subtract( newTags.toSet() );
    QSet<TagId> toAdd = QSet<TagId>( newTags.toSet() ).subtract( oldTags.toSet() );

    const TagProviderRetrieveJob * const tjob = qobject_cast<const TagProviderRetrieveJob*>( job );
    Q_ASSERT( tjob );
    const shared_ptr<const TagProvider> tp = tjob->tagProvider();
    TagModifyReferencesJob * const cjob = tp->tagModifyReferencesJob();
    cjob->setReferrer( m_itemToModify );
    cjob->setAddedTags( toAdd.toList() );
    cjob->setRemovedTags( toRemove.toList() );
    q->connect( cjob, SIGNAL( result( KJob* ) ), q, SLOT( slotReferencesModified( KJob* ) ) );
    cjob->start();
}

void ItemModifyJobPrivate::slotReferencesModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemModifyJob::CouldNotModifyTagReferences );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    q->emitResult();
}


ItemModifyJob::ItemModifyJob( QObject *parent ) :
    KJob( parent ), d( new ItemModifyJobPrivate( this ) )
{
}

ItemModifyJob::~ItemModifyJob()
{
    delete d;
}

void ItemModifyJob::setIgnorePayload( bool ignorePayload )
{
    d->m_ignorePayload = ignorePayload;
}

bool ItemModifyJob::ignorePayload() const
{
    return d->m_ignorePayload;
}

void ItemModifyJob::setItem( const Item& item )
{
    d->m_itemToModify = item;
}

Item ItemModifyJob::item() const
{
    return d->m_itemToModify;
}

QString ItemModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case ItemModifyJob::CouldNotFetchOldItem:
            result = i18n( "Could not fetch old item to get the old tags.\n%1", errorText() );
            break;
        case ItemModifyJob::CouldNotModifyItem:
            result = i18n( "Could not modify item.\n%1", errorText() );
            break;
        case ItemModifyJob::CouldNotRetrieveTagProvider:
            result = i18n( "Could not retrieve tag provider in order to process the item's tags.\n%1",
                           errorText() );
            break;
        case ItemModifyJob::CouldNotModifyTagReferences:
            result = i18n( "Could not modify references to this item inside the tag provider.\n%1",
                           errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void ItemModifyJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}


class KRss::ItemDeleteJobPrivate
{
public:
    explicit ItemDeleteJobPrivate( ItemDeleteJob *qq )
        : q( qq ) {}

    void doStart();
    void slotTagProviderRetrieved( KJob *job );
    void slotReferencesDeleted( KJob *job );
    void slotItemDeleted( KJob *job );

    ItemDeleteJob * const q;
    Item m_item;
};

void ItemDeleteJobPrivate::doStart()
{
    TagProviderRetrieveJob * const tjob = new TagProviderRetrieveJob();
    q->connect( tjob, SIGNAL( result( KJob* ) ), q, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void ItemDeleteJobPrivate::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemDeleteJob::CouldNotRetrieveTagProvider );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const TagProviderRetrieveJob * const tjob = qobject_cast<const TagProviderRetrieveJob*>( job );
    Q_ASSERT( tjob );
    const shared_ptr<const TagProvider> tp = tjob->tagProvider();
    TagDeleteReferencesJob * const cjob = tp->tagDeleteReferencesJob();
    cjob->setReferrer( m_item );
    q->connect( cjob, SIGNAL( result( KJob* ) ), q, SLOT( slotReferencesDeleted( KJob* ) ) );
    cjob->start();
}

void ItemDeleteJobPrivate::slotReferencesDeleted( KJob* job )
{
    if ( job->error() ) {
        q->setError( ItemDeleteJob::CouldNotDeleteTagReferences );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    Akonadi::ItemDeleteJob * const djob = new Akonadi::ItemDeleteJob( m_item.d->akonadiItem );
    q->connect( djob, SIGNAL( result( KJob* ) ), q, SLOT( slotItemDeleted( KJob* ) ) );
    djob->start();
}

void ItemDeleteJobPrivate::slotItemDeleted( KJob *job )
{
    if ( job->error() ) {
        q->setError( ItemDeleteJob::CouldNotDeleteItem );
        q->setErrorText( job->errorString() );
    }

    q->emitResult();
}


ItemDeleteJob::ItemDeleteJob( QObject *parent )
    : KJob( parent ), d( new ItemDeleteJobPrivate( this ) )
{
}

ItemDeleteJob::~ItemDeleteJob()
{
    delete d;
}

void ItemDeleteJob::setItem( const Item& item )
{
    d->m_item = item;
}

ItemId ItemDeleteJob::item() const
{
    return d->m_item.id();
}

QString ItemDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case ItemDeleteJob::CouldNotRetrieveTagProvider:
            result = i18n( "Could not retrieve tag provider in order to process the item's tags.\n%1",
                           errorText() );
            break;
        case ItemDeleteJob::CouldNotDeleteTagReferences:
            result = i18n( "Could not delete references to this item inside the tag provider.\n%1",
                           errorText() );
            break;
        case ItemDeleteJob::CouldNotDeleteItem:
            result = i18n( "Could not delete item.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void ItemDeleteJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

#include "itemjobs.moc"
