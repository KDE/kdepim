
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

#include "messageitem.h"
#include "messageitem_p.h"

#include "PimCommon/AnnotationDialog"
#include "theme.h"

#include <item.h>
#include <entityannotationsattribute.h>
#include <tagattribute.h>
#include <tagfetchjob.h>
#include <tagfetchscope.h>
#include <KIconLoader>
#include <KLocalizedString>

#include "messagelist_debug.h"
using namespace MessageList::Core;

Q_GLOBAL_STATIC(TagCache, s_tagCache)

class MessageItem::Tag::Private
{
public:
    Private()
        : mPriority(0)  //Initialize it
    {

    }
    QPixmap mPixmap;
    QString mName;
    QString mId;             ///< The unique id of this tag
    QColor mTextColor;
    QColor mBackgroundColor;
    QFont  mFont;
    int mPriority;
};

MessageItem::Tag::Tag(const QPixmap &pix, const QString &tagName, const QString &tagId)
    : d(new Private)
{
    d->mPixmap = pix;
    d->mName = tagName;
    d->mId = tagId;
}

MessageItem::Tag::~Tag()
{
    delete d;
}

const QPixmap &MessageItem::Tag::pixmap() const
{
    return d->mPixmap;
}

const QString &MessageItem::Tag::name() const
{
    return d->mName;
}

const QString &MessageItem::Tag::id() const
{
    return d->mId;
}

const QColor &MessageItem::Tag::textColor() const
{
    return d->mTextColor;
}

const QColor &MessageItem::Tag::backgroundColor() const
{
    return d->mBackgroundColor;
}

const QFont &MessageItem::Tag::font() const
{
    return d->mFont;
}

int MessageItem::Tag::priority() const
{
    return d->mPriority;
}

void MessageItem::Tag::setTextColor(const QColor &textColor)
{
    d->mTextColor = textColor;
}

void MessageItem::Tag::setBackgroundColor(const QColor &backgroundColor)
{
    d->mBackgroundColor = backgroundColor;
}

void MessageItem::Tag::setFont(const QFont &font)
{
    d->mFont = font;
}

void MessageItem::Tag::setPriority(int priority)
{
    d->mPriority = priority;
}

class MessageItemPrivateSettings
{
public:
    QColor mColorUnreadMessage;
    QColor mColorImportantMessage;
    QColor mColorToDoMessage;
    QFont mFont;
    QFont mFontUnreadMessage;
    QFont mFontImportantMessage;
    QFont mFontToDoMessage;

    // Keep those two invalid. They are here purely so that MessageItem can return
    // const reference to them
    QColor mColor;
    QColor mBackgroundColor;

};

Q_GLOBAL_STATIC(MessageItemPrivateSettings, s_settings)

MessageItemPrivate::MessageItemPrivate(MessageItem *qq)
    : ItemPrivate(qq),
      mThreadingStatus(MessageItem::ParentMissing),
      mEncryptionState(MessageItem::NotEncrypted),
      mSignatureState(MessageItem::NotSigned),
      mAboutToBeRemoved(false),
      mSubjectIsPrefixed(false),
      mTagList(Q_NULLPTR)
{
}

MessageItemPrivate::~MessageItemPrivate()
{
    s_tagCache->cancelRequest(this);
    invalidateTagCache();
}

void MessageItemPrivate::invalidateTagCache()
{
    if (mTagList) {
        qDeleteAll(*mTagList);
        delete mTagList;
        mTagList = Q_NULLPTR;
    }
}

void MessageItemPrivate::invalidateAnnotationCache()
{
}

const MessageItem::Tag *MessageItemPrivate::bestTag() const
{
    const MessageItem::Tag *best = Q_NULLPTR;
    foreach (const MessageItem::Tag *tag, getTagList()) {
        if (!best || tag->priority() < best->priority()) {
            best = tag;
        }
    }
    return best;
}

void MessageItemPrivate::fillTagList(const Akonadi::Tag::List &taglist)
{
    Q_ASSERT(!mTagList);
    mTagList = new QList<MessageItem::Tag *>;

    // TODO: The tag pointers here could be shared between all items, there really is no point in
    //       creating them for each item that has tags

    //Priority sort this and make bestTag more efficient

    foreach (const Akonadi::Tag &tag, taglist) {
        QString symbol = QStringLiteral("mail-tagged");
        Akonadi::TagAttribute *attr = tag.attribute<Akonadi::TagAttribute>();
        if (attr) {
            if (!attr->iconName().isEmpty()) {
                symbol = attr->iconName();
            }
        }
        MessageItem::Tag *messageListTag =
            new MessageItem::Tag(SmallIcon(symbol), tag.name(), tag.url().url());

        if (attr) {
            messageListTag->setTextColor(attr->textColor());
            messageListTag->setBackgroundColor(attr->backgroundColor());
            if (!attr->font().isEmpty()) {
                QFont font;
                if (font.fromString(attr->font())) {
                    messageListTag->setFont(font);
                }
            }
            if (attr->priority() != -1) {
                messageListTag->setPriority(attr->priority());
            } else {
                messageListTag->setPriority(0xFFFF);
            }
        }

        mTagList->append(messageListTag);
    }
}

QList<MessageItem::Tag *> MessageItemPrivate::getTagList() const
{
    if (!mTagList) {
        s_tagCache->retrieveTags(mAkonadiItem.tags(), const_cast<MessageItemPrivate *>(this));
        return QList<MessageItem::Tag *>();
    }

    return *mTagList;
}

bool MessageItemPrivate::tagListInitialized() const
{
    return mTagList != Q_NULLPTR;
}

MessageItem::MessageItem()
    : Item(Message, new MessageItemPrivate(this)), ModelInvariantIndex()
{
}

MessageItem::MessageItem(MessageItemPrivate *dd)
    : Item(Message, dd), ModelInvariantIndex()
{
}

MessageItem::~MessageItem()
{
}

QList< MessageItem::Tag * > MessageItem::tagList() const
{
    Q_D(const MessageItem);
    return d->getTagList();
}

bool MessageItem::hasAnnotation() const
{
    Q_D(const MessageItem);
    //TODO check for note entry?
    return d->mAkonadiItem.hasAttribute<Akonadi::EntityAnnotationsAttribute>();
}

QString MessageItem::annotation() const
{
    Q_D(const MessageItem);
    if (d->mAkonadiItem.hasAttribute<Akonadi::EntityAnnotationsAttribute>()) {
        Akonadi::EntityAnnotationsAttribute *attr = d->mAkonadiItem.attribute<Akonadi::EntityAnnotationsAttribute>();
        const QMap<QByteArray, QByteArray> annotations = attr->annotations();
        QByteArray annot = annotations.value("/private/comment");
        if (!annot.isEmpty()) {
            return QString::fromLatin1(annot);
        }
        annot = annotations.value("/shared/comment");
        if (!annot.isEmpty()) {
            return QString::fromLatin1(annot);
        }
    }
    return QString();
}

void MessageItem::editAnnotation()
{
    Q_D(MessageItem);
    if (d->mAnnotationDialog.data()) {
        return;
    }
    d->mAnnotationDialog = new PimCommon::AnnotationEditDialog(d->mAkonadiItem);
    d->mAnnotationDialog.data()->setAttribute(Qt::WA_DeleteOnClose);
    //FIXME make async
    if (d->mAnnotationDialog.data()->exec()) {
        // invalidate the cached mHasAnnotation value
    }
}

const MessageItem::Tag *MessageItemPrivate::findTagInternal(const QString &szTagId) const
{
    foreach (const MessageItem::Tag *tag, getTagList()) {
        if (tag->id() == szTagId) {
            return tag;
        }
    }
    return Q_NULLPTR;
}

const MessageItem::Tag *MessageItem::findTag(const QString &szTagId) const
{
    Q_D(const MessageItem);
    return d->findTagInternal(szTagId);
}

QString MessageItem::tagListDescription() const
{
    QString ret;

    foreach (const Tag *tag, tagList()) {
        if (!ret.isEmpty()) {
            ret += QLatin1String(", ");
        }
        ret += tag->name();
    }

    return ret;
}

void MessageItem::invalidateTagCache()
{
    Q_D(MessageItem);
    d->invalidateTagCache();
}

void MessageItem::invalidateAnnotationCache()
{
    Q_D(MessageItem);
    d->invalidateAnnotationCache();
}

const QColor &MessageItem::textColor() const
{
    Q_D(const MessageItem);
    const Tag *bestTag = d->bestTag();
    if (bestTag != Q_NULLPTR && bestTag->textColor().isValid()) {
        return bestTag->textColor();
    }

    Akonadi::MessageStatus messageStatus = status();
    if (!messageStatus.isRead()) {
        return s_settings->mColorUnreadMessage;
    } else if (messageStatus.isImportant()) {
        return s_settings->mColorImportantMessage;
    } else if (messageStatus.isToAct()) {
        return s_settings->mColorToDoMessage;
    } else {
        return s_settings->mColor;
    }
}

const QColor &MessageItem::backgroundColor() const
{
    Q_D(const MessageItem);
    const Tag *bestTag = d->bestTag();
    if (bestTag) {
        return bestTag->backgroundColor();
    } else {
        return s_settings->mBackgroundColor;
    }
}

const QFont &MessageItem::font() const
{
    Q_D(const MessageItem);
    // for performance reasons we don't want font retrieval to trigger
    // full tags loading, as the font is used for geometry calculation
    // and thus this method called for each item
    if (d->tagListInitialized()) {
        const Tag *bestTag = d->bestTag();
        if (bestTag && bestTag->font() != QFont()) {
            return bestTag->font();
        }
    }

    // from KDE3: "important" overrides "new" overrides "unread" overrides "todo"
    Akonadi::MessageStatus messageStatus = status();
    if (messageStatus.isImportant()) {
        return s_settings->mFontImportantMessage;
    } else if (!messageStatus.isRead()) {
        return s_settings->mFontUnreadMessage;
    } else if (messageStatus.isToAct()) {
        return s_settings->mFontToDoMessage;
    } else {
        return s_settings->mFont;
    }
}

MessageItem::SignatureState MessageItem::signatureState() const
{
    Q_D(const MessageItem);
    return d->mSignatureState;
}

void MessageItem::setSignatureState(SignatureState state)
{
    Q_D(MessageItem);
    d->mSignatureState = state;
}

MessageItem::EncryptionState MessageItem::encryptionState() const
{
    Q_D(const MessageItem);
    return d->mEncryptionState;
}

void MessageItem::setEncryptionState(EncryptionState state)
{
    Q_D(MessageItem);
    d->mEncryptionState = state;
}

QByteArray MessageItem::messageIdMD5() const
{
    Q_D(const MessageItem);
    return d->mMessageIdMD5;
}

void MessageItem::setMessageIdMD5(const QByteArray &md5)
{
    Q_D(MessageItem);
    d->mMessageIdMD5 = md5;
}

QByteArray MessageItem::inReplyToIdMD5() const
{
    Q_D(const MessageItem);
    return d->mInReplyToIdMD5;
}

void MessageItem::setInReplyToIdMD5(const QByteArray &md5)
{
    Q_D(MessageItem);
    d->mInReplyToIdMD5 = md5;
}

QByteArray MessageItem::referencesIdMD5() const
{
    Q_D(const MessageItem);
    return d->mReferencesIdMD5;
}

void MessageItem::setReferencesIdMD5(const QByteArray &md5)
{
    Q_D(MessageItem);
    d->mReferencesIdMD5 = md5;
}

void MessageItem::setSubjectIsPrefixed(bool subjectIsPrefixed)
{
    Q_D(MessageItem);
    d->mSubjectIsPrefixed = subjectIsPrefixed;
}

bool MessageItem::subjectIsPrefixed() const
{
    Q_D(const MessageItem);
    return d->mSubjectIsPrefixed;
}

QByteArray MessageItem::strippedSubjectMD5() const
{
    Q_D(const MessageItem);
    return d->mStrippedSubjectMD5;
}

void MessageItem::setStrippedSubjectMD5(const QByteArray &md5)
{
    Q_D(MessageItem);
    d->mStrippedSubjectMD5 = md5;
}

bool MessageItem::aboutToBeRemoved() const
{
    Q_D(const MessageItem);
    return d->mAboutToBeRemoved;
}

void MessageItem::setAboutToBeRemoved(bool aboutToBeRemoved)
{
    Q_D(MessageItem);
    d->mAboutToBeRemoved = aboutToBeRemoved;
}

MessageItem::ThreadingStatus MessageItem::threadingStatus() const
{
    Q_D(const MessageItem);
    return d->mThreadingStatus;
}

void MessageItem::setThreadingStatus(ThreadingStatus threadingStatus)
{
    Q_D(MessageItem);
    d->mThreadingStatus = threadingStatus;
}

unsigned long MessageItem::uniqueId() const
{
    Q_D(const MessageItem);
    return d->mAkonadiItem.id();
}

Akonadi::Item MessageList::Core::MessageItem::akonadiItem() const
{
    Q_D(const MessageItem);
    return d->mAkonadiItem;
}

void MessageList::Core::MessageItem::setAkonadiItem(const Akonadi::Item &item)
{
    Q_D(MessageItem);
    d->mAkonadiItem = item;
}

MessageItem *MessageItem::topmostMessage()
{
    if (!parent()) {
        return this;
    }
    if (parent()->type() == Item::Message) {
        return static_cast< MessageItem * >(parent())->topmostMessage();
    }
    return this;
}

QString MessageItem::accessibleTextForField(Theme::ContentItem::Type field)
{
    switch (field) {
    case Theme::ContentItem::Subject:
        return d_ptr->mSubject;
    case Theme::ContentItem::Sender:
        return d_ptr->mSender;
    case Theme::ContentItem::Receiver:
        return d_ptr->mReceiver;
    case Theme::ContentItem::SenderOrReceiver:
        return senderOrReceiver();
    case Theme::ContentItem::Date:
        return formattedDate();
    case Theme::ContentItem::Size:
        return formattedSize();
    case Theme::ContentItem::RepliedStateIcon:
        return status().isReplied() ? i18nc("Status of an item", "Replied") : QString();
    case Theme::ContentItem::ReadStateIcon:
        return status().isRead() ? i18nc("Status of an item", "Read") : i18nc("Status of an item", "Unread");
    case Theme::ContentItem::CombinedReadRepliedStateIcon:
        return accessibleTextForField(Theme::ContentItem::ReadStateIcon) + accessibleTextForField(Theme::ContentItem::RepliedStateIcon);
    default:
        return QString();
    }
}

QString MessageItem::accessibleText(const Theme *theme, int columnIndex)
{
    QStringList rowsTexts;
    const QList<Theme::Row *> rows = theme->column(columnIndex)->messageRows();
    rowsTexts.reserve(rows.count());

    Q_FOREACH (Theme::Row *row, rows) {
        QStringList leftStrings;
        QStringList rightStrings;
        const QList<Theme::ContentItem *> leftItems = row->leftItems();
        leftStrings.reserve(leftItems.count());
        Q_FOREACH (Theme::ContentItem *contentItem, leftItems) {
            leftStrings.append(accessibleTextForField(contentItem->type()));
        }

        Q_FOREACH (Theme::ContentItem *contentItem, row->rightItems()) {
            rightStrings.insert(rightStrings.begin(), accessibleTextForField(contentItem->type()));
        }

        rowsTexts.append((leftStrings + rightStrings).join(QStringLiteral(" ")));
    }

    return rowsTexts.join(QStringLiteral(" "));
}

void MessageItem::subTreeToList(QList< MessageItem * > &list)
{
    list.append(this);
    QList< Item * > *childList = childItems();
    if (!childList) {
        return;
    }
    QList< Item * >::ConstIterator end(childList->constEnd());
    for (QList< Item * >::ConstIterator it = childList->constBegin(); it != end; ++it) {
        Q_ASSERT((*it)->type() == Item::Message);
        static_cast< MessageItem * >(*it)->subTreeToList(list);
    }
}

void MessageItem::setUnreadMessageColor(const QColor &color)
{
    s_settings->mColorUnreadMessage = color;
}

void MessageItem::setImportantMessageColor(const QColor &color)
{
    s_settings->mColorImportantMessage = color;
}

void MessageItem::setToDoMessageColor(const QColor &color)
{
    s_settings->mColorToDoMessage = color;
}

void MessageItem::setGeneralFont(const QFont &font)
{
    s_settings->mFont = font;
}

void MessageItem::setUnreadMessageFont(const QFont &font)
{
    s_settings->mFontUnreadMessage = font;
}

void MessageItem::setImportantMessageFont(const QFont &font)
{
    s_settings->mFontImportantMessage = font;
}

void MessageItem::setToDoMessageFont(const QFont &font)
{
    s_settings->mFontToDoMessage = font;
}

FakeItemPrivate::FakeItemPrivate(FakeItem *qq) : MessageItemPrivate(qq)
{
}

FakeItem::FakeItem()
    : MessageItem(new FakeItemPrivate(this))
{
}

FakeItem::~FakeItem()
{
}

QList< MessageItem::Tag * > FakeItem::tagList() const
{
    Q_D(const FakeItem);
    return d->mFakeTags;
}

void FakeItem::setFakeTags(const QList< MessageItem::Tag * > &tagList)
{
    Q_D(FakeItem);
    d->mFakeTags = tagList;
}

bool FakeItem::hasAnnotation() const
{
    return true;
}

TagCache::TagCache()
    : QObject(),
      mMonitor(new Akonadi::Monitor(this))
{
    mCache.setMaxCost(100);
    mMonitor->setTypeMonitored(Akonadi::Monitor::Tags);
    mMonitor->tagFetchScope().fetchAttribute<Akonadi::TagAttribute>();
    connect(mMonitor, &Akonadi::Monitor::tagAdded, this, &TagCache::onTagAdded);
    connect(mMonitor, &Akonadi::Monitor::tagRemoved, this, &TagCache::onTagRemoved);
    connect(mMonitor, &Akonadi::Monitor::tagChanged, this, &TagCache::onTagChanged);
}

void TagCache::onTagAdded(const Akonadi::Tag &tag)
{
    mCache.insert(tag.id(), new Akonadi::Tag(tag));
}

void TagCache::onTagChanged(const Akonadi::Tag &tag)
{
    mCache.remove(tag.id());
}

void TagCache::onTagRemoved(const Akonadi::Tag &tag)
{
    mCache.remove(tag.id());
}

void TagCache::retrieveTags(const Akonadi::Tag::List &tags, MessageItemPrivate *m)
{
    //Retrieval is in progress
    if (mRequests.key(m)) {
        return;
    }
    Akonadi::Tag::List toFetch;
    Akonadi::Tag::List available;
    Q_FOREACH (const Akonadi::Tag &tag, tags) {
        if (mCache.contains(tag.id())) {
            available << *mCache.object(tag.id());
        } else {
            toFetch << tag;
        }
    }
    //Because fillTagList expects to be called once we either fetch all or none
    if (!toFetch.isEmpty()) {
        Akonadi::TagFetchJob *tagFetchJob = new Akonadi::TagFetchJob(tags, this);
        tagFetchJob->fetchScope().fetchAttribute<Akonadi::TagAttribute>();
        connect(tagFetchJob, &Akonadi::TagFetchJob::result, this, &TagCache::onTagsFetched);
        mRequests.insert(tagFetchJob, m);
    } else {
        m->fillTagList(available);
    }
}

void TagCache::cancelRequest(MessageItemPrivate *m)
{
    const QList<KJob *> keys = mRequests.keys(m);
    Q_FOREACH (KJob *job, keys) {
        mRequests.remove(job);
    }
}

void TagCache::onTagsFetched(KJob *job)
{
    if (job->error()) {
        qCWarning(MESSAGELIST_LOG) << "Failed to fetch tags: " << job->errorString();
        return;
    }
    Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob *>(job);
    Q_FOREACH (const Akonadi::Tag &tag, fetchJob->tags()) {
        mCache.insert(tag.id(), new Akonadi::Tag(tag));
    }
    MessageItemPrivate *m = mRequests.take(fetchJob);
    if (m) {
        m->fillTagList(fetchJob->tags());
    }
}
