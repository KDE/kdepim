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

#include "messagetag.h"
#include "ontologies/email.h"
#include "messagecore/annotationdialog.h"

#include <akonadi/item.h>

#include <Nepomuk/Resource>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>

#include <KIconLoader>

using namespace MessageList::Core;

class FakeItem::Private
{
  public:
    QList<Tag*> mFakeTags;
};

class MessageItem::Tag::Private
{
public:
  QPixmap mPixmap;
  QString mName;
  QString mId;             ///< The unique id of this tag
  QColor mTextColor;
  QColor mBackgroundColor;
  QFont  mFont;
  int    mPriority;
};

MessageItem::Tag::Tag( const QPixmap &pix, const QString &tagName, const QString &tagId )
  : d( new Private )
{
  d->mPixmap = pix;
  d->mName = tagName;
  d->mId = tagId;
}

MessageItem::Tag::~Tag()
{
  delete d;
}

QPixmap MessageItem::Tag::pixmap() const
{
  return d->mPixmap;
}

QString MessageItem::Tag::name() const
{
  return d->mName;
}

QString MessageItem::Tag::id() const
{
  return d->mId;
}

QColor MessageItem::Tag::textColor() const
{
  return d->mTextColor;
}

QColor MessageItem::Tag::backgroundColor() const
{
  return d->mBackgroundColor;
}

QFont MessageItem::Tag::font() const
{
  return d->mFont;
}

int MessageItem::Tag::priority() const
{
  return d->mPriority;
}

void MessageItem::Tag::setTextColor( const QColor &textColor )
{
  d->mTextColor = textColor;
}

void MessageItem::Tag::setBackgroundColor( const QColor &backgroundColor )
{
  d->mBackgroundColor = backgroundColor;
}

void MessageItem::Tag::setFont( const QFont &font )
{
  d->mFont = font;
}

void MessageItem::Tag::setPriority( int priority )
{
  d->mPriority = priority;
}


class MessageItem::Private
{
public:

  Private();
  ~Private();

  /**
   * Linear search in the list of tags. The lists of tags
   * associated to a message are supposed to be very short (c'mon.. you won't add more than a couple of tags to a single msg).
   * so a linear search is better than a hash lookup in most cases.
   */
  const Tag *findTagInternal( const QString &szTagId ) const;

  /// Returns the list of tags. This is calculated on demand and cached in mTagList
  QList<Tag*> getTagList() const;

  bool tagListInitialized() const;

  /// Returns the tag with the highest priority, or 0 if there are no tags
  const Tag* bestTag() const;

  /// Deletes the internal list of tags
  void invalidateTagCache();

  /// Deletes the cache of the annotation
  void invalidateAnnotationCache();

  ThreadingStatus mThreadingStatus;
  QByteArray mMessageIdMD5;            ///< always set
  QByteArray mInReplyToIdMD5;          ///< set only if we're doing threading
  QByteArray mReferencesIdMD5;         ///< set only if we're doing threading
  QByteArray mStrippedSubjectMD5;      ///< set only if we're doing threading
  EncryptionState mEncryptionState;
  SignatureState mSignatureState;
  Akonadi::Item mAkonadiItem;

  bool mAboutToBeRemoved : 1;       ///< Set to true when this item is going to be deleted and shouldn't be selectable
  bool mSubjectIsPrefixed : 1;      ///< set only if we're doing subject based threading
  bool mAnnotationStateChecked : 1; ///< The state of the annotation below has been checked
  bool mHasAnnotation : 1;          ///< Cached value for hasAnnotation()

  static QColor mColorNewMessage;
  static QColor mColorUnreadMessage;
  static QColor mColorImportantMessage;
  static QColor mColorToDoMessage;
  static QFont mFont;
  static QFont mFontNewMessage;
  static QFont mFontUnreadMessage;
  static QFont mFontImportantMessage;
  static QFont mFontToDoMessage;

private:

  // This creates mTagList and fills it with useful data
  void fillTagList() const;

  // List of all tags. If this is 0, it means we have not yet calculated this list. It is calculated
  // on demand when needed.
  mutable QList< Tag * > * mTagList;
};

QColor MessageItem::Private::mColorNewMessage;
QColor MessageItem::Private::mColorUnreadMessage;
QColor MessageItem::Private::mColorImportantMessage;
QColor MessageItem::Private::mColorToDoMessage;
QFont MessageItem::Private::mFont;
QFont MessageItem::Private::mFontNewMessage;
QFont MessageItem::Private::mFontUnreadMessage;
QFont MessageItem::Private::mFontImportantMessage;
QFont MessageItem::Private::mFontToDoMessage;

MessageItem::Private::Private()
  : mThreadingStatus( MessageItem::ParentMissing ),
    mAboutToBeRemoved( false ),
    mAnnotationStateChecked( false ),
    mTagList( 0 )
{
}

MessageItem::Private::~Private()
{
  invalidateTagCache();
}

void MessageItem::Private::invalidateTagCache()
{
  if ( mTagList ) {
    qDeleteAll( *mTagList );
    delete mTagList;
    mTagList = 0;
  }
}

void MessageItem::Private::invalidateAnnotationCache()
{
  mAnnotationStateChecked = false;
}

const MessageItem::Tag* MessageItem::Private::bestTag() const
{
  const Tag *best = 0;
  foreach( const Tag* tag, getTagList() ) {
    if ( !best || tag->priority() < best->priority() )
      best = tag;
  }
  return best;
}

void MessageItem::Private::fillTagList() const
{
  Q_ASSERT( !mTagList );
  mTagList = new QList<Tag*>;

  // TODO: The tag pointers here could be shared between all items, there really is no point in
  //       creating them for each item that has tags

  const Nepomuk::Resource resource( mAkonadiItem.url() );
  const QList< Nepomuk::Tag > nepomukTagList = resource.tags();
  if ( !nepomukTagList.isEmpty() ) {
    foreach( const Nepomuk::Tag &nepomukTag, nepomukTagList ) {
      QString symbol = QLatin1String( "mail-tagged" );
      if ( !nepomukTag.symbols().isEmpty() ) {
        symbol = nepomukTag.symbols().first();
      }
      Tag *messageListTag =
          new Tag( SmallIcon( symbol ),
                   nepomukTag.label(), nepomukTag.resourceUri().toString() );
      if ( nepomukTag.hasProperty( Vocabulary::MessageTag::textColor() ) ) {
        const QString name = nepomukTag.property( Vocabulary::MessageTag::textColor() ).toString();
        messageListTag->setTextColor( QColor( name ) );
      }
      if ( nepomukTag.hasProperty( Vocabulary::MessageTag::backgroundColor() ) ) {
        const QString name = nepomukTag.property( Vocabulary::MessageTag::backgroundColor() ).toString();
        messageListTag->setBackgroundColor( QColor( name ) );
      }
      if ( nepomukTag.hasProperty( Vocabulary::MessageTag::priority() ) ) {
        messageListTag->setPriority( nepomukTag.property( Vocabulary::MessageTag::priority() ).toInt() );
      }
      else
        messageListTag->setPriority( 0xFFFF );

      if ( nepomukTag.hasProperty( Vocabulary::MessageTag::font() ) ) {
        const QString fontName = nepomukTag.property( Vocabulary::MessageTag::font() ).toString();
        QFont font;
        font.fromString( fontName );
        messageListTag->setFont( font );
      }

      mTagList->append( messageListTag );
    }
  }
}

QList<MessageItem::Tag*> MessageItem::Private::getTagList() const
{
  if ( !mTagList )
    fillTagList();

  return *mTagList;
}

bool MessageItem::Private::tagListInitialized() const
{
  return mTagList != 0;
}

MessageItem::MessageItem()
  : Item( Message ), ModelInvariantIndex(), d( new Private )
{
}

MessageItem::~MessageItem()
{
  delete d;
}

QList< MessageItem::Tag * > MessageItem::tagList() const
{
  return d->getTagList();
}

bool MessageItem::hasAnnotation() const
{
  if ( d->mAnnotationStateChecked )
    return d->mHasAnnotation;

  Nepomuk::Resource resource( d->mAkonadiItem.url() );
  if ( resource.hasProperty( QUrl( Nepomuk::Resource::descriptionUri() ) ) ) {
    d->mHasAnnotation = !resource.description().isEmpty();
  } else {
    d->mHasAnnotation = false;
  }

  d->mAnnotationStateChecked = true;
  return d->mHasAnnotation;
}

QString MessageItem::annotation() const
{
  if ( hasAnnotation() ) {
    Nepomuk::Resource resource( d->mAkonadiItem.url() );
    return resource.description();
  }
  else return QString();
}

void MessageItem::editAnnotation()
{
  MessageCore::AnnotationEditDialog *dialog = new MessageCore::AnnotationEditDialog( d->mAkonadiItem.url() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();
  // invalidate the cached mHasAnnotation value
  d->mAnnotationStateChecked = false;
}

QString MessageItem::contentSummary() const
{
  Nepomuk::Resource mail( d->mAkonadiItem.url() );
  const QString content =
      mail.property( NepomukFast::Message::plainTextMessageContentUri() ).toString();

  // Extract the first 5 non-empty, non-quoted lines from the content and return it
  int numLines = 0;
  const int maxLines = 5;
  const QStringList lines = content.split( QLatin1Char( '\n' ) );
  QString ret;
  foreach( const QString &line, lines ) {
    const bool isQuoted = line.trimmed().startsWith( QLatin1Char( '>' ) ) || line.trimmed().startsWith( QLatin1Char( '|' ) );
    if ( !line.trimmed().isEmpty() && !isQuoted ) {
      ret += line + QLatin1Char( '\n' );
      numLines++;
      if ( numLines >= maxLines )
        break;
    }
  }
  return ret;
}

const MessageItem::Tag * MessageItem::Private::findTagInternal( const QString &szTagId ) const
{
  foreach( const Tag *tag, getTagList() ) {
    if ( tag->id() == szTagId )
      return tag;
  }
  return 0;
}

const MessageItem::Tag *MessageItem::findTag( const QString &szTagId ) const
{
  return d->findTagInternal( szTagId );
}

QString MessageItem::tagListDescription() const
{
  QString ret;

  foreach( const Tag *tag, tagList() ) {
    if ( !ret.isEmpty() )
      ret += QLatin1String( ", " );
    ret += tag->name();
  }

  return ret;
}

void MessageItem::invalidateTagCache()
{
  d->invalidateTagCache();
}

void MessageItem::invalidateAnnotationCache()
{
  d->invalidateAnnotationCache();
}

QColor MessageItem::textColor() const
{
  QColor clr;
  Akonadi::MessageStatus messageStatus = status();
  if ( !messageStatus.isRead() ) {
    clr = d->mColorUnreadMessage;
  } else if ( messageStatus.isImportant() ) {
    clr = d->mColorImportantMessage;
  } else if ( messageStatus.isToAct() ) {
    clr = d->mColorToDoMessage;
  }

  const Tag *bestTag = d->bestTag();
  if ( bestTag != 0 ) {
    clr = bestTag->textColor();
  }

  return clr;
}

QColor MessageItem::backgroundColor() const
{
  const Tag *bestTag = d->bestTag();
  if ( bestTag != 0 ) {
    return bestTag->backgroundColor();
  } else {
    return QColor();
  }
}

QFont MessageItem::font() const
{
  // for performance reasons we don't want font retrieval to trigger
  // full tags loading, as the font is used for geometry calculation
  // and thus this method called for each item
  if ( d->tagListInitialized() ) {
    const Tag *bestTag = d->bestTag();
    if ( bestTag != 0 && bestTag->font() != QFont() ) {
      return bestTag->font();
    }
  }

  QFont font;

  // from KDE3: "important" overrides "new" overrides "unread" overrides "todo"
  Akonadi::MessageStatus messageStatus = status();
  if ( messageStatus.isImportant() ) {
    font = d->mFontImportantMessage;
  } else if ( !messageStatus.isRead() ) {
    font = d->mFontUnreadMessage;
  } else if ( messageStatus.isToAct() ) {
    font = d->mFontToDoMessage;
  } else {
    font = d->mFont;
  }

  return font;
}

MessageItem::SignatureState MessageItem::signatureState() const
{
  return d->mSignatureState;
}

void MessageItem::setSignatureState( SignatureState state )
{
  d->mSignatureState = state;
}

MessageItem::EncryptionState MessageItem::encryptionState() const
{
  return d->mEncryptionState;
}

void MessageItem::setEncryptionState( EncryptionState state )
{
  d->mEncryptionState = state;
}

QByteArray MessageItem::messageIdMD5() const
{
  return d->mMessageIdMD5;
}

void MessageItem::setMessageIdMD5( const QByteArray &md5 )
{
  d->mMessageIdMD5 = md5;
}

QByteArray MessageItem::inReplyToIdMD5() const
{
  return d->mInReplyToIdMD5;
}

void MessageItem::setInReplyToIdMD5( const QByteArray& md5 )
{
  d->mInReplyToIdMD5 = md5;
}

QByteArray MessageItem::referencesIdMD5() const
{
  return d->mReferencesIdMD5;
}

void MessageItem::setReferencesIdMD5( const QByteArray& md5 )
{
  d->mReferencesIdMD5 = md5;
}

void MessageItem::setSubjectIsPrefixed( bool subjectIsPrefixed )
{
  d->mSubjectIsPrefixed = subjectIsPrefixed;
}

bool MessageItem::subjectIsPrefixed() const
{
  return d->mSubjectIsPrefixed;
}

QByteArray MessageItem::strippedSubjectMD5() const
{
  return d->mStrippedSubjectMD5;
}

void MessageItem::setStrippedSubjectMD5( const QByteArray& md5 )
{
  d->mStrippedSubjectMD5 = md5;
}

bool MessageItem::aboutToBeRemoved() const
{
  return d->mAboutToBeRemoved;
}

void MessageItem::setAboutToBeRemoved( bool aboutToBeRemoved )
{
  d->mAboutToBeRemoved = aboutToBeRemoved;
}

MessageItem::ThreadingStatus MessageItem::threadingStatus() const
{
  return d->mThreadingStatus;
}

void MessageItem::setThreadingStatus( ThreadingStatus threadingStatus )
{
  d->mThreadingStatus = threadingStatus;
}

unsigned long MessageItem::uniqueId() const
{
  return d->mAkonadiItem.id();
}

Akonadi::Item MessageList::Core::MessageItem::akonadiItem() const
{
  return d->mAkonadiItem;
}

void MessageList::Core::MessageItem::setAkonadiItem(const Akonadi::Item& item)
{
  d->mAkonadiItem = item;
}

MessageItem * MessageItem::topmostMessage()
{
  if ( !parent() )
    return this;
  if ( parent()->type() == Item::Message )
    return static_cast< MessageItem * >( parent() )->topmostMessage();
  return this;
}

void MessageItem::subTreeToList( QList< MessageItem * > &list )
{
  list.append( this );
  QList< Item * > * childList = childItems();
  if ( !childList )
    return;
  for ( QList< Item * >::Iterator it = childList->begin(); it != childList->end(); ++it )
  {
    Q_ASSERT( ( *it )->type() == Item::Message );
    static_cast< MessageItem * >( *it )->subTreeToList( list );
  }
}

void MessageItem::setNewMessageColor( const QColor &color )
{
  MessageItem::Private::mColorNewMessage = color;
}


void MessageItem::setUnreadMessageColor( const QColor &color )
{
  MessageItem::Private::mColorUnreadMessage = color;
}


void MessageItem::setImportantMessageColor( const QColor &color )
{
  MessageItem::Private::mColorImportantMessage = color;
}


void MessageItem::setToDoMessageColor( const QColor &color )
{
  MessageItem::Private::mColorToDoMessage = color;
}


void MessageItem::setGeneralFont( const QFont &font )
{
  MessageItem::Private::mFont = font;
}

void MessageItem::setNewMessageFont( const QFont &font )
{
  MessageItem::Private::mFontNewMessage = font;
}

void MessageItem::setUnreadMessageFont( const QFont &font )
{
  MessageItem::Private::mFontUnreadMessage = font;
}

void MessageItem::setImportantMessageFont( const QFont &font )
{
  MessageItem::Private::mFontImportantMessage = font;
}

void MessageItem::setToDoMessageFont( const QFont &font )
{
  MessageItem::Private::mFontToDoMessage = font;
}

FakeItem::FakeItem()
  : d( new Private() )
{
}

FakeItem::~FakeItem()
{
  delete d;
}

QList< MessageItem::Tag * > FakeItem::tagList() const
{
  return d->mFakeTags;
}

void FakeItem::setFakeTags( const QList< MessageItem::Tag* > &tagList )
{
  d->mFakeTags = tagList;
}

bool FakeItem::hasAnnotation() const
{
  return true;
}


