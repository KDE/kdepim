/******************************************************************************
 *
 *  Copyright 2008 Szymon Tomasz Stefanek <pragma@kvirc.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *******************************************************************************/

#ifndef MESSAGELIST_MESSAGEITEM_P_H
#define MESSAGELIST_MESSAGEITEM_P_H

#include "messageitem.h"
#include "item_p.h"
#include "messagecore/widgets/annotationdialog.h"
#include <Item>
#include <Monitor>
#include <Tag>
#include <KJob>
#include <QWeakPointer>
#include <QCache>

namespace MessageList
{
namespace Core
{

class MessageItemPrivate : public ItemPrivate
{
public:

    explicit MessageItemPrivate(MessageItem *qq);
    ~MessageItemPrivate();

    /**
     * Linear search in the list of tags. The lists of tags
     * associated to a message are supposed to be very short (c'mon.. you won't add more than a couple of tags to a single msg).
     * so a linear search is better than a hash lookup in most cases.
     */
    const MessageItem::Tag *findTagInternal(const QString &szTagId) const;

    /// Returns the list of tags. This is calculated on demand and cached in mTagList
    QList<MessageItem::Tag *> getTagList() const;

    bool tagListInitialized() const;

    /// Returns the tag with the highest priority, or 0 if there are no tags
    const MessageItem::Tag *bestTag() const;

    /// Deletes the internal list of tags
    void invalidateTagCache();

    /// Deletes the cache of the annotation
    void invalidateAnnotationCache();

    // This creates mTagList and fills it with useful data
    void fillTagList(const Akonadi::Tag::List &taglist);

    QByteArray mMessageIdMD5;            ///< always set
    QByteArray mInReplyToIdMD5;          ///< set only if we're doing threading
    QByteArray mReferencesIdMD5;         ///< set only if we're doing threading
    QByteArray mStrippedSubjectMD5;      ///< set only if we're doing threading
    Akonadi::Item mAkonadiItem;
    QWeakPointer<MessageCore::AnnotationEditDialog> mAnnotationDialog;
    MessageItem::ThreadingStatus mThreadingStatus : 4;
    MessageItem::EncryptionState mEncryptionState : 4;
    MessageItem::SignatureState mSignatureState : 4;

    bool mAboutToBeRemoved : 1;       ///< Set to true when this item is going to be deleted and shouldn't be selectable
    bool mSubjectIsPrefixed : 1;      ///< set only if we're doing subject based threading

private:

    // List of all tags. If this is 0, it means we have not yet calculated this list. It is calculated
    // on demand when needed.
    mutable QList< MessageItem::Tag * > *mTagList;
};

class FakeItemPrivate : public MessageItemPrivate
{
public:
    explicit FakeItemPrivate(FakeItem *qq);
    QList<MessageItem::Tag *> mFakeTags;
};

/**
 * A tag cache
 */
class TagCache : public QObject
{
    Q_OBJECT
public:
    TagCache();
    void retrieveTags(const Akonadi::Tag::List &tags, MessageItemPrivate *m);
    void cancelRequest(MessageItemPrivate *m);

private Q_SLOTS:
    void onTagAdded(const Akonadi::Tag &);
    void onTagChanged(const Akonadi::Tag &);
    void onTagRemoved(const Akonadi::Tag &);
    void onTagsFetched(KJob *);

private:
    QHash<KJob *, MessageItemPrivate *> mRequests;
    QCache<Akonadi::Tag::Id, Akonadi::Tag> mCache;
    Akonadi::Monitor *mMonitor;
};

}

}

#endif
