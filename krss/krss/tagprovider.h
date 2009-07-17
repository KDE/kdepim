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

#ifndef KRSS_TAGPROVIDER_H
#define KRSS_TAGPROVIDER_H

#include "krss_export.h"
#include "tag.h"
#include <KJob>
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <boost/shared_ptr.hpp>

namespace KRss {

class TagCreateJob;
class TagModifyJob;
class TagDeleteJob;
class TagCreateReferencesJob;
class TagModifyReferencesJob;
class TagDeleteReferencesJob;
class TagProviderRetrieveJobPrivate;

/**
 * Central, authorative list of the tags.
 * Use \a TagProviderRetrieveJob to load and initialize \a TagProvider.
 * It ensures that only one instance of \a TagProvider is created and
 * used during runtime.
 */
class KRSS_EXPORT TagProvider : public QObject
{
    Q_OBJECT
public:
    explicit TagProvider( QObject *parent = 0 );
    virtual ~TagProvider();

    virtual Tag tag( const TagId& id ) const = 0;
    virtual QHash<TagId, Tag> tags() const = 0;

    virtual TagCreateJob* tagCreateJob() const = 0;
    virtual TagModifyJob* tagModifyJob() const = 0;
    virtual TagDeleteJob* tagDeleteJob() const = 0;

private:
    virtual TagCreateReferencesJob* tagCreateReferencesJob() const = 0;
    virtual TagModifyReferencesJob* tagModifyReferencesJob() const = 0;
    virtual TagDeleteReferencesJob* tagDeleteReferencesJob() const = 0;

Q_SIGNALS:
    void tagCreated( const KRss::Tag& tag );
    void tagModified( const KRss::Tag& tag );
    void tagDeleted( const KRss::TagId& id );

private:
    // these jobs use Tag*ReferencesJob to update
    // the tagprovider specific storage
    // when creating/modifying/deleting feeds/articles
    friend class FeedModifyJobPrivate;
    friend class FeedDeleteJobPrivate;
    friend class ItemModifyJobPrivate;
    friend class ItemDeleteJobPrivate;
    Q_DISABLE_COPY( TagProvider )
};

/**
 * Loads and initializes a \a TagProvider instance.
 */
class KRSS_EXPORT TagProviderRetrieveJob : public KJob
{
    Q_OBJECT
public:
    enum Error {
        CouldNotRetrieveTagProvider = KJob::UserDefinedError
    };

    explicit TagProviderRetrieveJob( QObject *parent = 0 );
    ~TagProviderRetrieveJob();

    boost::shared_ptr<TagProvider> tagProvider() const;
    void start();
    QString errorString() const;

private:
    friend class TagProviderRetrieveJobPrivate;
    TagProviderRetrieveJobPrivate* const d;
    Q_PRIVATE_SLOT( d, void tagProviderLoaded( KJob* ) )
};

/**
 * Does the dirty job of creating and loading storage-specific \a TagProvider.
 */
class TagProviderLoadJob : public KJob
{
public:
    enum Error {
        CouldNotLoadTagProvider = KJob::UserDefinedError
    };

    explicit TagProviderLoadJob( QObject* parent = 0 )
        : KJob( parent ) {}
    virtual TagProvider* tagProvider() const = 0;
};

} //namespace KRss

#endif // KRSS_TAGPROVIDER_H
