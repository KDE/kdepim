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

#ifndef KRSS_TAGJOBS_H
#define KRSS_TAGJOBS_H

#include "krss_export.h"
#include "tag.h"
#include <KJob>

template<typename T> class QList;

namespace KRss {

class Feed;
class Item;

/**
 * Creates a new tag.
 */
class KRSS_EXPORT TagCreateJob : public KJob {
    Q_OBJECT
public:
    explicit TagCreateJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setTag( const Tag& ) = 0;
    virtual Tag tag() const = 0;

    enum Error {
        CouldNotCreateTag = KJob::UserDefinedError,
        UserDefinedError
    };
};

/**
 * Modifies the properties of an existing tag.
 */
class KRSS_EXPORT TagModifyJob : public KJob {
    Q_OBJECT
public:
    explicit TagModifyJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setTag( const Tag& ) = 0;

    enum Error {
        CouldNotModifyTag = KJob::UserDefinedError,
        UserDefinedError
    };
};

/**
 * Deletes an existing tag.
 */
class KRSS_EXPORT TagDeleteJob : public KJob {
    Q_OBJECT
public:
    explicit TagDeleteJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setTag( const Tag& ) = 0;

    enum Error {
        CouldNotDeleteTag = KJob::UserDefinedError,
        UserDefinedError
    };
};

/**
 * What this job does depends on the tag provider but one can
 * assume that it links the specified feed/item (referrer) to its tags
 * inside the tag provider specific storage.
 * For internal use only, invoked by the jobs for creating items/feeds.
 */
class TagCreateReferencesJob : public KJob {
    Q_OBJECT
public:
    explicit TagCreateReferencesJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setReferrer( const Feed* feed ) = 0;
    virtual void setReferrer( const Item& item ) = 0;

    enum Error {
        CouldNotCreateReferences = KJob::UserDefinedError,
        UserDefinedError
    };
};

/**
 * What this job does depends on the tag provider but one can
 * assume that it (re)links the specified feed/item (referrer) to its tags
 * inside the tag provider specific storage.
 * For internal use only, invoked by the jobs for modifying items/feeds.
 */
class TagModifyReferencesJob : public KJob {
    Q_OBJECT
public:
    explicit TagModifyReferencesJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setReferrer( const Feed* feed ) = 0;
    virtual void setReferrer( const Item& item ) = 0;
    virtual void setAddedTags( const QList<TagId>& tags ) = 0;
    virtual void setRemovedTags( const QList<TagId>& tags ) = 0;

    enum Error {
        CouldNotModifyReferences = KJob::UserDefinedError,
        UserDefinedError
    };
};

/**
 * What this job does depends on the tag provider but one can
 * assume that it unlinks the specified feed/item (referrer) from its tags
 * inside the tag provider specific storage.
 * For internal use only, invoked by the jobs for deleting items/feeds.
 */
class TagDeleteReferencesJob : public KJob {
    Q_OBJECT
public:
    explicit TagDeleteReferencesJob( QObject* parent=0 ) : KJob( parent ) {}

    virtual void setReferrer( const Feed* feed ) = 0;
    virtual void setReferrer( const Item& item ) = 0;

    enum Error {
        CouldNotDeleteReferences = KJob::UserDefinedError,
        UserDefinedError
    };
};

} // namespace KRss

#endif // KRSS_TAGJOBS_H
