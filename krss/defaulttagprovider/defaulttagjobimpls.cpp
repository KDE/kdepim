/*
    Copyright (C) 2008    Frank Osterfeld <osterfeld@kde.org>
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

#include "defaulttagjobimpls.h"
#include "krss/tag_p.h"
#include "defaulttagprovider.h"
#include "krss/feedcollection.h"
#include "krss/tagidsattribute.h"
#include "krss/tagprovider.h"
#include "krss/resourcemanager.h"
#include "krss/rssitem.h"

#include <akonadi/collection.h>
#include <akonadi/searchcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemdeletejob.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionmodifyjob.h>
#include <akonadi/collectiondeletejob.h>
#include <akonadi/linkjob.h>
#include <akonadi/unlinkjob.h>
#include <KRandom>
#include <KLocale>
#include <KDebug>

using namespace KRss;
using Akonadi::Collection;
using Akonadi::SearchCreateJob;
using Akonadi::CollectionFetchJob;
using Akonadi::CollectionModifyJob;
using Akonadi::CollectionDeleteJob;
using Akonadi::LinkJob;
using Akonadi::UnlinkJob;

DefaultTagCreateJob::DefaultTagCreateJob( const Collection& searchCollection, const TagProvider *provider,
                                          QObject* parent )
    : TagCreateJob( parent ), m_tagProvider( provider ), m_akonadiSearchCollection( searchCollection )
{
}

void DefaultTagCreateJob::setTag( const Tag& tag )
{
    m_tag = tag;
}

Tag DefaultTagCreateJob::tag() const
{
    return m_tag;
}

QString DefaultTagCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagCreateJob::CouldNotCreateTag:
            result = i18n( "Couldn't create tag.\n%1", errorText() );
            break;
        case DefaultTagCreateJob::CouldNotLoadTag:
            result = i18n( "Couldn't load tag.\n%1", errorText() );
            break;
        case DefaultTagCreateJob::CouldNotFindTag:
            result = i18n( "Couldn't find tag." );
            break;
        case DefaultTagCreateJob::CouldNotModifyTag:
            result = i18n( "Couldn't set tag properties.\n1%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagCreateJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void DefaultTagCreateJob::doStart()
{
    const QList<Tag> tags = m_tagProvider->tags().values();
    Q_FOREACH( const KRss::Tag& tag, tags ) {
        if ( tag.label() == m_tag.label() ) {
            m_tag = tag;
            emitResult();
            return;
        }
    }

    // generate a pseudo unique URI
    const QString remoteId = "rss-tag-" + KRandom::randomString( 10 );
    TagPrivate::tag_d( m_tag )->m_collection.setRemoteId( remoteId );
    const Collection col = TagPrivate::tag_d( m_tag )->m_collection;
    SearchCreateJob *sjob = new SearchCreateJob( col.name(), col.remoteId() );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionCreated( KJob* ) ) );
    sjob->start();
}

void DefaultTagCreateJob::slotCollectionCreated( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotCreateTag );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    // now load all the items from m_tagsCollection to find the just created item
    CollectionFetchJob *fjob = new CollectionFetchJob( m_akonadiSearchCollection,
                                                       CollectionFetchJob::FirstLevel );
    connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsFetched( KJob* ) ) );
    fjob->start();
}

void DefaultTagCreateJob::slotCollectionsFetched( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotLoadTag );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    const CollectionFetchJob *fjob = qobject_cast<const CollectionFetchJob*>( job );
    Q_ASSERT( fjob );
    const QList<Collection> cols = fjob->collections();
    Q_FOREACH( const Collection& col, cols ) {
        if ( col.remoteId() == Tag::idToString( m_tag.id() ) ) {
            kDebug() << "Found the just created tag at id:" << col.id();
            TagPrivate::tag_d( m_tag )->m_collection.setId( col.id() );
            CollectionModifyJob *mjob = new CollectionModifyJob( TagPrivate::tag_d( m_tag )->m_collection );
            connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionModified( KJob* ) ) );
            mjob->start();
            return;
        }
    }

    kWarning() << "Could not find the just created tag in Akonadi";
    setError( CouldNotFindTag );
    emitResult();
}

void DefaultTagCreateJob::slotCollectionModified( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotModifyTag );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    emitResult();
}

DefaultTagModifyJob::DefaultTagModifyJob( QObject* parent ) : TagModifyJob( parent ) {}

void DefaultTagModifyJob::setTag( const Tag& tag )
{
    m_tag = tag;
}

QString DefaultTagModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagModifyJob::CouldNotModifyTag:
            result = i18n( "Couldn't modify tag.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagModifyJob::start()
{
    CollectionModifyJob *mjob = new CollectionModifyJob( TagPrivate::tag_d( m_tag )->m_collection );
    connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionModified( KJob* ) ) );
    mjob->start();
}

void DefaultTagModifyJob::slotCollectionModified( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotModifyTag );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
    }

    emitResult();
}

DefaultTagDeleteJob::DefaultTagDeleteJob( QObject* parent )
    : TagDeleteJob( parent ), m_pendingCollectionFetchJobs( 0 ),
      m_pendingCollectionModifyJobs( 0 ), m_pendingItemModifyJobs( 0 )
{
}

void DefaultTagDeleteJob::setTag( const Tag& tag )
{
    m_tag = tag;
}

QString DefaultTagDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagDeleteJob::CouldNotDeleteTag:
            result = i18n( "Couldn't delete tag.\n%1", errorText() );
            break;
        case DefaultTagDeleteJob::CouldNotLoadFeeds:
            result = i18n( "Couldn't load feeds.\n%1", errorText() );
            break;
        case DefaultTagDeleteJob::CouldNotModifyFeeds:
            result = i18n( "Couldn't remove the tag from feeds.\n%1", errorText() );
            break;
        case DefaultTagDeleteJob::CouldNotLoadItems:
            result = i18n( "Couldn't load items.\n%1", errorText() );
            break;
        case DefaultTagDeleteJob::CouldNotModifyItems:
            result = i18n( "Couldn't remove the tag from items.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagDeleteJob::start()
{
    const QStringList resources = ResourceManager::self()->identifiers();
    Q_FOREACH( const QString& resource, resources ) {
        CollectionFetchJob *fjob = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive );
        fjob->includeUnsubscribed( true );
        fjob->setResource( resource );
        connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsFetched( KJob* ) ) );
        ++m_pendingCollectionFetchJobs;
        fjob->start();
    }
}

void DefaultTagDeleteJob::slotCollectionsFetched( KJob *job )
{
    --m_pendingCollectionFetchJobs;
    if ( job->error() ) {
        setError( CouldNotLoadFeeds );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    const CollectionFetchJob *fjob = qobject_cast<const CollectionFetchJob*>( job );
    Q_ASSERT( fjob );

    const QList<Collection> cols = fjob->collections();
    Q_FOREACH( const FeedCollection &col, cols ) {
        // skip the root collections and collections without this tag
        if ( col.parent() == Collection::root().id() || !col.tags().contains( m_tag.id() ) )
            continue;

        kDebug() << "Found collection with this tag:" << col.id() << "," << col.name();
        FeedCollection copy( col );
        copy.removeTag( m_tag.id() );
        ++m_pendingCollectionModifyJobs;
        CollectionModifyJob *mjob = new CollectionModifyJob( copy );
        connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionModified( KJob* ) ) );
        mjob->start();
    }

    if ( m_pendingCollectionFetchJobs == 0 && m_pendingCollectionModifyJobs == 0 ) {
        kDebug() << "Done with collections, loading items";
        Akonadi::ItemFetchJob *fjob = new Akonadi::ItemFetchJob( TagPrivate::tag_d( m_tag )->m_collection );
        fjob->fetchScope().fetchFullPayload( false );
        fjob->fetchScope().fetchAllAttributes( false );
        fjob->fetchScope().fetchAttribute<TagIdsAttribute>( true );
        connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemsFetched( KJob* ) ) );
        fjob->start();
    }
}

void DefaultTagDeleteJob::slotCollectionModified( KJob *job )
{
    --m_pendingCollectionModifyJobs;
    if ( job->error() ) {
        setError( CouldNotModifyFeeds );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    if ( m_pendingCollectionFetchJobs == 0 && m_pendingCollectionModifyJobs == 0 ) {
        kDebug() << "Done with collections, loading items";
        Akonadi::ItemFetchJob *fjob = new Akonadi::ItemFetchJob( TagPrivate::tag_d( m_tag )->m_collection );
        fjob->fetchScope().fetchFullPayload( false );
        fjob->fetchScope().fetchAllAttributes( false );
        fjob->fetchScope().fetchAttribute<TagIdsAttribute>( true );
        connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemsFetched( KJob* ) ) );
        fjob->start();
    }
}

void DefaultTagDeleteJob::slotItemsFetched( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotLoadItems );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    const Akonadi::ItemFetchJob *fjob = qobject_cast<const Akonadi::ItemFetchJob*>( job );
    const QList<Akonadi::Item> items = fjob->items();
    Q_FOREACH( const Akonadi::Item& item, items ) {
        TagIdsAttribute *attr = item.attribute<TagIdsAttribute>();
        if ( !attr || !attr->tagIds().contains( m_tag.id() ) ) {
            kWarning() << "Item" << item.id() << "doesn't have this tag";
            continue;
        }

        kDebug() << "Found items with this tag:" << item.id();
        Akonadi::Item copy( item );
        copy.attribute<TagIdsAttribute>()->removeTagId( m_tag.id() );
        ++m_pendingItemModifyJobs;
        Akonadi::ItemModifyJob *mjob = new Akonadi::ItemModifyJob( copy );
        connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemModified( KJob* ) ) );
        mjob->start();
    }

    if ( items.isEmpty() ) {
        // there were no items, we are done
        kDebug() << "Deleting the tag";
        CollectionDeleteJob *djob = new CollectionDeleteJob( TagPrivate::tag_d( m_tag )->m_collection );
        connect( djob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagDeleted( KJob* ) ) );
        djob->start();
    }
}

void DefaultTagDeleteJob::slotItemModified( KJob *job )
{
    --m_pendingItemModifyJobs;
    if ( job->error() ) {
        setError( CouldNotModifyItems );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    if ( m_pendingItemModifyJobs == 0 ) {
        kDebug() << "Deleting the tag";
        CollectionDeleteJob *djob = new CollectionDeleteJob( TagPrivate::tag_d( m_tag )->m_collection );
        connect( djob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagDeleted( KJob* ) ) );
        djob->start();
    }
}

void DefaultTagDeleteJob::slotTagDeleted( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotDeleteTag );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    Q_ASSERT( m_pendingCollectionFetchJobs == 0 );
    Q_ASSERT( m_pendingCollectionModifyJobs == 0 );
    Q_ASSERT( m_pendingItemModifyJobs == 0 );

    emitResult();
}

DefaultTagCreateReferencesJob::DefaultTagCreateReferencesJob( const TagProvider *provider, QObject *parent )
    : TagCreateReferencesJob( parent ), m_tagProvider( provider ), m_pendingLinkJobs( 0 ),
      m_referrerType( NoReferrer )
{
}

void DefaultTagCreateReferencesJob::setReferrer( const Feed *feed )
{
    m_feed = feed;
    m_referrerType = FeedReferrer;
}

void DefaultTagCreateReferencesJob::setReferrer( const Item& item )
{
    m_item = item;
    m_referrerType = ItemReferrer;
}

QString DefaultTagCreateReferencesJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagCreateReferencesJob::CouldNotCreateReferences:
            result = i18n( "Couldn't create references.\n%1", errorText() );
            break;
        case DefaultTagCreateReferencesJob::ReferrerNotSet:
            result = i18n( "Referrer not set." );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagCreateReferencesJob::start()
{
    if ( m_referrerType == NoReferrer) {
        setError( ReferrerNotSet );
        emitResult();
        return;
    }

    // feeds have no hardlinks in virtual collecions,
    // so do nothing
    if ( m_referrerType == FeedReferrer ) {
        emitResult();
        return;
    }

    // create hardlinks
    const QList<TagId> tags = m_item.tags();
    if ( tags.isEmpty() ) {
        emitResult();
        return;
    }

    Q_FOREACH( const TagId& id, tags ) {
        const Tag tag = m_tagProvider->tag( id );
        const Collection col = TagPrivate::tag_d( tag )->m_collection;
        ++m_pendingLinkJobs;
        LinkJob *ljob = new LinkJob( col, QList<Akonadi::Item>() <<
                                     Akonadi::Item( RssItem::itemIdToAkonadi( m_item.id() ) )
                                   );
        connect( ljob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemLinked( KJob* ) ) );
        ljob->start();
    }
}

void DefaultTagCreateReferencesJob::slotItemLinked( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotCreateReferences );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    --m_pendingLinkJobs;
    if ( m_pendingLinkJobs == 0 ) {
        kDebug() << "Done!";
        emitResult();
    }
}

DefaultTagModifyReferencesJob::DefaultTagModifyReferencesJob( const TagProvider *provider, QObject *parent )
    : TagModifyReferencesJob( parent ), m_tagProvider( provider ), m_pendingLinkJobs( 0 ),
      m_pendingUnlinkJobs( 0 ), m_referrerType( NoReferrer )
{
}

void DefaultTagModifyReferencesJob::setReferrer( const Feed *feed )
{
    m_feed = feed;
    m_referrerType = FeedReferrer;
}

void DefaultTagModifyReferencesJob::setReferrer( const Item& item )
{
    m_item = item;
    m_referrerType = ItemReferrer;
}

void DefaultTagModifyReferencesJob::setAddedTags( const QList<TagId>& tags )
{
    m_addedTags = tags;
}

void DefaultTagModifyReferencesJob::setRemovedTags( const QList<TagId>& tags )
{
    m_removedTags = tags;
}

QString DefaultTagModifyReferencesJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagModifyReferencesJob::CouldNotModifyReferences:
            result = i18n( "Couldn't modify references.\n%1", errorText() );
            break;
        case DefaultTagModifyReferencesJob::ReferrerNotSet:
            result = i18n( "Referrer not set." );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagModifyReferencesJob::start()
{
    if ( m_referrerType == NoReferrer) {
        setError( ReferrerNotSet );
        emitResult();
        return;
    }

    // feeds have no hardlinks in virtual collecions,
    // so do nothing
    if ( m_referrerType == FeedReferrer ) {
        emitResult();
        return;
    }

    if ( m_removedTags.isEmpty() && m_addedTags.isEmpty() ) {
        emitResult();
        return;
    }

    // delete hardlinks
    Q_FOREACH( const TagId& id, m_removedTags ) {
        const Tag tag = m_tagProvider->tag( id );
        const Collection col = TagPrivate::tag_d( tag )->m_collection;
        ++m_pendingUnlinkJobs;
        UnlinkJob *ujob = new UnlinkJob( col, QList<Akonadi::Item>() <<
                                         Akonadi::Item( RssItem::itemIdToAkonadi( m_item.id() ) )
                                       );
        connect( ujob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemUnlinked( KJob* ) ) );
        ujob->start();
    }

    // create hardlinks
    Q_FOREACH( const TagId& id, m_addedTags ) {
        const Tag tag = m_tagProvider->tag( id );
        const Collection col = TagPrivate::tag_d( tag )->m_collection;
        ++m_pendingLinkJobs;
        LinkJob *ljob = new LinkJob( col, QList<Akonadi::Item>() <<
                                     Akonadi::Item( RssItem::itemIdToAkonadi( m_item.id() ) )
                                   );
        connect( ljob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemLinked( KJob* ) ) );
        ljob->start();
    }
}

void DefaultTagModifyReferencesJob::slotItemUnlinked( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotModifyReferences );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    --m_pendingUnlinkJobs;
    if ( m_pendingUnlinkJobs == 0 && m_pendingLinkJobs == 0 ) {
        kDebug() << "Done!";
        emitResult();
    }
}

void DefaultTagModifyReferencesJob::slotItemLinked( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotModifyReferences );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    --m_pendingLinkJobs;
    if ( m_pendingUnlinkJobs == 0 && m_pendingLinkJobs == 0 ) {
        kDebug() << "Done!";
        emitResult();
    }
}

DefaultTagDeleteReferencesJob::DefaultTagDeleteReferencesJob( const TagProvider *provider, QObject *parent )
    : TagDeleteReferencesJob( parent ), m_tagProvider( provider ), m_pendingUnlinkJobs( 0 ),
      m_referrerType( NoReferrer )
{
}

void DefaultTagDeleteReferencesJob::setReferrer( const Feed *feed )
{
    m_feed = feed;
    m_referrerType = FeedReferrer;
}

void DefaultTagDeleteReferencesJob::setReferrer( const Item& item )
{
    m_item = item;
    m_referrerType = ItemReferrer;
}

QString DefaultTagDeleteReferencesJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case DefaultTagDeleteReferencesJob::CouldNotDeleteReferences:
            result = i18n( "Couldn't delete references.\n%1", errorText() );
            break;
        case DefaultTagDeleteReferencesJob::ReferrerNotSet:
            result = i18n( "Referrer not set." );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagDeleteReferencesJob::start()
{
    if ( m_referrerType == NoReferrer) {
        setError( ReferrerNotSet );
        emitResult();
        return;
    }

    // feeds have no hardlinks in virtual collecions,
    // so do nothing
    if ( m_referrerType == FeedReferrer ) {
        emitResult();
        return;
    }

    const QList<TagId> tags = m_item.tags();
    if ( tags.isEmpty() ) {
        emitResult();
        return;
    }

    Q_FOREACH( const TagId& id, tags ) {
        const Tag tag = m_tagProvider->tag( id );
        const Collection col = TagPrivate::tag_d( tag )->m_collection;
        kDebug() << "Unlinking from:" << col.id();
        ++m_pendingUnlinkJobs;
        UnlinkJob *ujob = new UnlinkJob( col, QList<Akonadi::Item>() <<
                                         Akonadi::Item( RssItem::itemIdToAkonadi( m_item.id() ) )
                                       );
        connect( ujob, SIGNAL( result( KJob* ) ), this, SLOT( slotItemUnlinked( KJob* ) ) );
        ujob->start();
    }
}

void DefaultTagDeleteReferencesJob::slotItemUnlinked( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotDeleteReferences );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    --m_pendingUnlinkJobs;
    if ( m_pendingUnlinkJobs == 0 ) {
        kDebug() << "Done!";
        emitResult();
    }
}

#include "defaulttagjobimpls.moc"
