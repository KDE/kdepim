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

#include "defaulttagprovider.h"
#include "defaulttagjobimpls.h"
#include "defaulttag_p.h"
#include "tagpropertiesattribute.h"

#include <akonadi/collectionfetchjob.h>
#include <akonadi/monitor.h>
#include <KLocale>
#include <KDebug>

using namespace KRss;
using Akonadi::Collection;
using Akonadi::Monitor;
using Akonadi::CollectionFetchJob;

const Collection DefaultTagProvider::AkonadiSearchCollection = Collection( 1 );

DefaultTagProvider::DefaultTagProvider( const QList<Akonadi::Collection>& tagCollections, QObject *parent )
    : TagProvider( parent )
{
    // convert akonadi collections to tags
    Q_FOREACH( const Collection& tagCollection, tagCollections ) {
        if ( tagCollection.hasAttribute<TagPropertiesAttribute>() ) {
            Tag tag;
            tag.d->setAkonadiCollection( tagCollection );
            m_tags[ tag.id() ] = tag;
            kDebug() << "Converted tag:" << tag.label();
        }
    }
    // setup the monitor
    Monitor* const monitor = new Monitor( this );
    monitor->setCollectionMonitored( AkonadiSearchCollection );
    monitor->fetchCollection( true );
    connect( monitor, SIGNAL( collectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ) );
    connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionChanged( const Akonadi::Collection& ) ) );
    connect( monitor, SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
             this, SLOT( slotCollectionRemoved( const Akonadi::Collection& ) ) );
}

Tag DefaultTagProvider::tag( const TagId& id ) const
{
    return m_tags.value( id );
}

QHash<TagId, Tag> DefaultTagProvider::tags() const
{
    return m_tags;
}

TagCreateJob* DefaultTagProvider::tagCreateJob() const
{
    return new DefaultTagCreateJob( AkonadiSearchCollection, this );
}

TagModifyJob* DefaultTagProvider::tagModifyJob() const
{
    return new DefaultTagModifyJob;
}

TagDeleteJob* DefaultTagProvider::tagDeleteJob() const
{
    return new DefaultTagDeleteJob;
}

TagCreateReferencesJob* DefaultTagProvider::tagCreateReferencesJob() const
{
    return new DefaultTagCreateReferencesJob( this );
}

TagModifyReferencesJob* DefaultTagProvider::tagModifyReferencesJob() const
{
    return new DefaultTagModifyReferencesJob( this );
}

TagDeleteReferencesJob* DefaultTagProvider::tagDeleteReferencesJob() const
{
    return new DefaultTagDeleteReferencesJob( this );
}

void DefaultTagProvider::slotCollectionAdded( const Akonadi::Collection& col, const Akonadi::Collection& parent )
{
    if ( parent != AkonadiSearchCollection || !col.hasAttribute<TagPropertiesAttribute>() )
        return;

    kDebug() << "Collection created: " << col.id();
    Tag tag;
    tag.d->setAkonadiCollection( col );
    m_tags[ tag.id() ] = tag;
    emit tagCreated( tag );
}

void DefaultTagProvider::slotCollectionChanged( const Akonadi::Collection& col )
{
    if ( col.parent() != AkonadiSearchCollection.id() || !col.hasAttribute<TagPropertiesAttribute>() )
        return;

    kDebug() << "Collection modified: " << col.id();
    Tag tag;
    tag.d->setAkonadiCollection( col );

    // When creating a new tag, first a virtual search collection is created and then
    // its attributes are modified using a separate job. Therefore slotCollectionAdded()
    // blocks newly created tags (they have no TagPropertiesAttribute) but we'll see these tags
    // then their properties get modified. So we emit tagCreated() here.
    if ( m_tags.contains( tag.id() ) ) {
        m_tags[ tag.id() ] = tag;
        emit tagModified( tag );
    } else {
        m_tags[ tag.id() ] = tag;
        emit tagCreated( tag );
    }
}

void DefaultTagProvider::slotCollectionRemoved( const Akonadi::Collection& col )
{
    // since the col has no attributes set (they are all deleted) we can't tell for sure
    // whether it is a Tag collection or just a normal virtual Akonadi collection.
    // so we only check whether col is in the internal list and if so, remove it
    if ( col.parent() != AkonadiSearchCollection.id() )
        return;

    kDebug() << "Collection deleted: " << col.id();
    Tag tag;
    tag.d->setAkonadiCollection( col );

    // see the explanation above
    if ( !m_tags.contains( tag.id() ) )
        return;

    m_tags.remove( tag.id() );
    emit tagDeleted( tag.id() );
}


DefaultTagProviderLoadJob::DefaultTagProviderLoadJob( QObject *parent )
    : TagProviderLoadJob( parent ), m_provider( 0 )
{
}

TagProvider* DefaultTagProviderLoadJob::tagProvider() const
{
    return m_provider;
}

QString DefaultTagProviderLoadJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case TagProviderLoadJob::CouldNotLoadTagProvider:
            result = i18n( "Couldn't load the tag provider.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void DefaultTagProviderLoadJob::start()
{
    CollectionFetchJob *job = new CollectionFetchJob( DefaultTagProvider::AkonadiSearchCollection,
                                                      CollectionFetchJob::FirstLevel );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsFetched( KJob* ) ) );
    job->start();
}

void DefaultTagProviderLoadJob::slotCollectionsFetched( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotLoadTagProvider );
        setErrorText( job->errorString() );
        kWarning() << job->errorString();
        emitResult();
        return;
    }

    const QList<Collection> cols = static_cast<CollectionFetchJob*>( job )->collections();
    kDebug() << "Loaded" << cols.count() << "collections";
    m_provider = new DefaultTagProvider( cols );
    emitResult();
}

#include "defaulttagprovider.moc"
