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

#include "nepomuktagprovider.h"
#include "nepomuktagjobimpls.h"
#include "krss/tag_p.h"

#include <KLocale>
#include <KDebug>

using namespace KRss;

NepomukTagProvider::NepomukTagProvider( const QList<Nepomuk::Tag>& nepomukTags, QObject *parent )
    : TagProvider( parent )
{
    Q_FOREACH( const Nepomuk::Tag& nepomukTag, nepomukTags ) {
        const Tag tag = fromNepomukTag( nepomukTag );
        m_tags[ tag.id() ] = tag;
        kDebug() << "Tag's uri:" << tag.id();
        kDebug() << "Tag's label:" << tag.label();
        kDebug() << "Tag's description:" << tag.description();
    }
}

Tag NepomukTagProvider::tag( const TagId& id ) const
{
    return m_tags.value( id );
}

QHash<TagId, Tag> NepomukTagProvider::tags() const
{
    return m_tags;
}

TagCreateJob* NepomukTagProvider::tagCreateJob() const
{
    // HACK: this is a temporary solution
    // remove as soon as Nepomuk gets support for resource monitoring
    NepomukTagCreateJob *job = new NepomukTagCreateJob();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( nepomukTagCreated( KJob* ) ) );
    return job;
}

TagModifyJob* NepomukTagProvider::tagModifyJob() const
{
    // HACK: this is a temporary solution
    // remove as soon as Nepomuk gets support for resource monitoring
    NepomukTagModifyJob *job = new NepomukTagModifyJob();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( nepomukTagModified( KJob* ) ) );
    return job;
}

TagDeleteJob* NepomukTagProvider::tagDeleteJob() const
{
    // HACK: this is a temporary solution
    // remove as soon as Nepomuk gets support for resource monitoring
    NepomukTagDeleteJob *job = new NepomukTagDeleteJob();
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( nepomukTagDeleted( KJob* ) ) );
    return job;
}

TagCreateReferencesJob* NepomukTagProvider::tagCreateReferencesJob() const
{
    return new NepomukTagCreateReferencesJob();
}

TagModifyReferencesJob* NepomukTagProvider::tagModifyReferencesJob() const
{
    return new NepomukTagModifyReferencesJob();
}

TagDeleteReferencesJob* NepomukTagProvider::tagDeleteReferencesJob() const
{
    return new NepomukTagDeleteReferencesJob();
}

void NepomukTagProvider::nepomukTagCreated( KJob *job )
{
    if ( job->error() == KJob::NoError ) {
        const NepomukTagCreateJob * const cjob = qobject_cast<const NepomukTagCreateJob*>( job );
        Q_ASSERT( cjob );
        const Tag createdTag = cjob->tag();
        Q_ASSERT( !m_tags.contains( createdTag.id() ) );
        m_tags[ createdTag.id() ] = createdTag;
        emit tagCreated( createdTag );
    }
}

void NepomukTagProvider::nepomukTagModified( KJob *job )
{
    if ( job->error() == KJob::NoError ) {
        const NepomukTagModifyJob * const mjob = qobject_cast<const NepomukTagModifyJob*>( job );
        Q_ASSERT( mjob );
        const Tag modifiedTag = mjob->tag();
        Q_ASSERT( m_tags.contains( modifiedTag.id() ) );
        m_tags[ modifiedTag.id() ] = modifiedTag;
        emit tagModified( modifiedTag );
    }
}

void NepomukTagProvider::nepomukTagDeleted( KJob *job )
{
    if ( job->error() == KJob::NoError ) {
        const NepomukTagDeleteJob * const djob = qobject_cast<const NepomukTagDeleteJob*>( job );
        Q_ASSERT( djob );
        const TagId id = djob->tag();
        Q_ASSERT( m_tags.contains( id ) );
        m_tags.remove( id );
        emit tagDeleted( id );
    }
}

NepomukTagProviderLoadJob::NepomukTagProviderLoadJob( QObject *parent )
    : TagProviderLoadJob( parent ), m_provider( 0 )
{
}

TagProvider* NepomukTagProviderLoadJob::tagProvider() const
{
    return m_provider;
}

QString NepomukTagProviderLoadJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case TagProviderLoadJob::CouldNotLoadTagProvider:
            result = i18n( "Couldn't load the tag provider." );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void NepomukTagProviderLoadJob::start()
{
    const QList<Nepomuk::Tag> nepomukTags = Nepomuk::Tag::allTags();
    kDebug() << "Loaded" << nepomukTags.count() << "tags from Nepomuk";
    m_provider = new NepomukTagProvider( nepomukTags );
    emitResult();
}

#include "nepomuktagprovider.moc"
