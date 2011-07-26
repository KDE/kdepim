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

#ifndef KRSS_DEFAULTTAGJOBIMPLS_H
#define KRSS_DEFAULTTAGJOBIMPLS_H

#include "krss/tagjobs.h"
#include "krss/tag.h"
#include "krss/item.h"
#include <akonadi/collection.h>

namespace KRss {

class TagProvider;

class DefaultTagCreateJob : public TagCreateJob
{
    Q_OBJECT
public:
    explicit DefaultTagCreateJob( const Akonadi::Collection& searchCollection, const KRss::TagProvider *provider,
                                  QObject* parent = 0 );

    void setTag( const Tag& tag );
    Tag tag() const;
    void start();

    enum Error {
        CouldNotLoadTag = TagCreateJob::UserDefinedError,
        CouldNotFindTag,
        CouldNotModifyTag
    };
    QString errorString() const;

private Q_SLOTS:
    void doStart();
    void slotCollectionCreated( KJob *job );
    void slotCollectionsFetched( KJob *job );
    void slotCollectionModified( KJob *job );

private:
    const KRss::TagProvider * const m_tagProvider;
    const Akonadi::Collection m_akonadiSearchCollection;
    Tag m_tag;
};

class DefaultTagModifyJob : public TagModifyJob
{
    Q_OBJECT
public:
    explicit DefaultTagModifyJob( QObject* parent = 0 );

    void setTag( const Tag& tag );
    void start();
    QString errorString() const;

private Q_SLOTS:
    void slotCollectionModified( KJob *job );

private:
    Tag m_tag;
};

class DefaultTagDeleteJob : public TagDeleteJob
{
    Q_OBJECT
public:
    explicit DefaultTagDeleteJob( QObject* parent = 0 );

    void setTag( const Tag& tag );
    void start();

    enum Error {
        CouldNotLoadFeeds = TagDeleteJob::UserDefinedError,
        CouldNotModifyFeeds,
        CouldNotLoadItems,
        CouldNotModifyItems
    };
    QString errorString() const;

private Q_SLOTS:
    void slotCollectionsFetched( KJob *job );
    void slotCollectionModified( KJob *job );
    void slotItemsFetched( KJob *job );
    void slotItemModified( KJob *job );
    void slotTagDeleted( KJob *job );

private:
    Tag m_tag;
    int m_pendingCollectionFetchJobs;
    int m_pendingCollectionModifyJobs;
    int m_pendingItemModifyJobs;
};

class DefaultTagCreateReferencesJob : public TagCreateReferencesJob
{
    Q_OBJECT
public:
    explicit DefaultTagCreateReferencesJob( const TagProvider *provider, QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );

    void start();

    enum Error {
        ReferrerNotSet = TagCreateReferencesJob::UserDefinedError
    };
    QString errorString() const;

private Q_SLOTS:
    void slotItemLinked( KJob *job );

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    const TagProvider *m_tagProvider;
    int m_pendingLinkJobs;
    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
};

class DefaultTagModifyReferencesJob : public TagModifyReferencesJob
{
    Q_OBJECT
public:
    explicit DefaultTagModifyReferencesJob( const TagProvider *provider, QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );
    void setAddedTags( const QList<TagId>& tags );
    void setRemovedTags( const QList<TagId>& tags );

    void start();

    enum Error {
        ReferrerNotSet = TagModifyReferencesJob::UserDefinedError
    };
    QString errorString() const;

private Q_SLOTS:
    void slotItemLinked( KJob *job );
    void slotItemUnlinked( KJob *job );

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    const TagProvider *m_tagProvider;
    int m_pendingLinkJobs;
    int m_pendingUnlinkJobs;
    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
    QList<TagId> m_addedTags;
    QList<TagId> m_removedTags;
};

class DefaultTagDeleteReferencesJob : public TagDeleteReferencesJob
{
    Q_OBJECT
public:
    explicit DefaultTagDeleteReferencesJob( const TagProvider *provider, QObject *parent = 0 );

    void setReferrer( const Feed* feed );
    void setReferrer( const Item& item );

    void start();

    enum Error {
        ReferrerNotSet = TagDeleteReferencesJob::UserDefinedError
    };
    QString errorString() const;

private Q_SLOTS:
    void slotItemUnlinked( KJob *job );

private:
    enum ReferrerType {
        NoReferrer,
        FeedReferrer,
        ItemReferrer
    };

    const TagProvider *m_tagProvider;
    int m_pendingUnlinkJobs;
    Item m_item;
    const Feed *m_feed;
    ReferrerType m_referrerType;
};

} // namespace KRss

#endif // KRSS_DEFAULTTAGJOBIMPLS_H
