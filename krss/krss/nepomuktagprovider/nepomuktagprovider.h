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

#ifndef KRSS_NEPOMUKTAGPROVIDER_H
#define KRSS_NEPOMUKTAGPROVIDER_H

#include "krss/tagprovider.h"
#include <nepomuk/tag.h>

template<typename T> class QList;

namespace KRss {

/**
 * \a TagProvider implementation that stores tags in Nepomuk as
 * Nepomuk::Tag resources.
*/
class NepomukTagProvider : public TagProvider
{
    Q_OBJECT
public:
    explicit NepomukTagProvider( const QList<Nepomuk::Tag>& nepomukTags, QObject *parent = 0 );

    Tag tag( const TagId& id ) const;
    QHash<TagId, Tag> tags() const;

    TagCreateJob* tagCreateJob() const;
    TagModifyJob* tagModifyJob() const;
    TagDeleteJob* tagDeleteJob() const;

private:
    TagCreateReferencesJob* tagCreateReferencesJob() const;
    TagModifyReferencesJob* tagModifyReferencesJob() const;
    TagDeleteReferencesJob* tagDeleteReferencesJob() const;

private Q_SLOTS:
    void nepomukTagCreated( KJob *job );
    void nepomukTagModified( KJob *job );
    void nepomukTagDeleted( KJob *job );

private:
    QHash<TagId, Tag> m_tags;
    Q_DISABLE_COPY( NepomukTagProvider )
};

/** Creates and initializes \a NepomukTagProvider.
*/
class NepomukTagProviderLoadJob : public TagProviderLoadJob
{
    Q_OBJECT
public:

    explicit NepomukTagProviderLoadJob( QObject* parent = 0 );
    TagProvider* tagProvider() const;

    QString errorString() const;
    void start();

private:
    TagProvider *m_provider;
    Q_DISABLE_COPY( NepomukTagProviderLoadJob )
};

} //namespace KRss

#endif // KRSS_NEPOMUKTAGPROVIDER_H
