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

#ifndef KRSS_NEPOMUKTAGJOBIMPLS_H
#define KRSS_NEPOMUKTAGJOBIMPLS_H

#include "krss/tagjobs.h"
#include "krss/tag.h"
#include "krss/item.h"

namespace Nepomuk{
class Tag;
}

namespace KRss {

Tag fromNepomukTag( const Nepomuk::Tag& ntag );

class NepomukTagCreateJob : public TagCreateJob
{
    Q_OBJECT
public:
    explicit NepomukTagCreateJob( QObject* parent = 0 );

    void setTag( const Tag& tag );
    Tag tag() const;
    void start();

    QString errorString() const;

private Q_SLOTS:
    void doStart();

private:
    Tag m_tag;
};

class NepomukTagModifyJob : public TagModifyJob
{
    Q_OBJECT
public:
    explicit NepomukTagModifyJob( QObject* parent = 0 );

    void setTag( const Tag& tag );
    Tag tag() const;
    void start();
    QString errorString() const;

private Q_SLOTS:
    void doStart();

private:
    Tag m_tag;
};

class NepomukTagDeleteJob : public TagDeleteJob
{
    Q_OBJECT
public:
    explicit NepomukTagDeleteJob( QObject* parent = 0 );

    void setTag( const Tag& tag );
    TagId tag() const;
    void start();

    enum Error {
        CouldNotLoadFeeds = TagDeleteJob::UserDefinedError,
        CouldNotModifyFeeds,
        CouldNotLoadItems,
        CouldNotModifyItems,
        CouldNotDeleteNepomukTag
    };
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotCollectionsFetched( KJob *job );
    void slotCollectionModified( KJob *job );
    void slotItemFetched( KJob *job );
    void slotItemModified( KJob *job );
    void deleteNepomukTag();

private:
    Tag m_tag;
    int m_pendingCollectionModifyJobs;
    int m_pendingItemFetchJobs;
    int m_pendingItemModifyJobs;
};

class NepomukTagCreateReferencesJob : public TagCreateReferencesJob
{
    Q_OBJECT
public:
    explicit NepomukTagCreateReferencesJob( QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );

    void start();

    enum Error {
        ReferrerNotSet = TagCreateReferencesJob::UserDefinedError,
        ResourceAlreadyExists,
        TagNotFound,
        CouldNotAllocateNepomukResource
    };
    QString errorString() const;

private Q_SLOTS:
    void doStart();

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
};

class NepomukTagModifyReferencesJob : public TagModifyReferencesJob
{
    Q_OBJECT
public:
    explicit NepomukTagModifyReferencesJob( QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );
    void setAddedTags( const QList<TagId>& tags );
    void setRemovedTags( const QList<TagId>& tags );

    void start();

    enum Error {
        ReferrerNotSet = TagModifyReferencesJob::UserDefinedError,
        ResourceNotFound,
        TagNotFound,
        CouldNotAllocateNepomukResource
    };
    QString errorString() const;

private Q_SLOTS:
    void doStart();

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
    QList<TagId> m_addedTags;
    QList<TagId> m_removedTags;
};

class NepomukTagDeleteReferencesJob : public TagDeleteReferencesJob
{
    Q_OBJECT
public:
    explicit NepomukTagDeleteReferencesJob( QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );

    void start();

    enum Error {
        ReferrerNotSet = TagDeleteReferencesJob::UserDefinedError,
        ResourceNotFound,
        CouldNotDeleteNepomukResource
    };
    QString errorString() const;

private Q_SLOTS:
    void doStart();

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
};

} // namespace KRss

#endif // KRSS_NEPOMUKTAGJOBIMPLS_H
