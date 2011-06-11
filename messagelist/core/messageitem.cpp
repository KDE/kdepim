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

#include "messagetag.h"
#include "ontologies/email.h"
#include "messagecore/annotationdialog.h"

#include <akonadi/item.h>

#include <Nepomuk/Resource>
#include <Nepomuk/Tag>
#include <Nepomuk/Variant>

#include <KIconLoader>

using namespace MessageList::Core;

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


QColor MessageItemPrivate::mColorNewMessage;
QColor MessageItemPrivate::mColorUnreadMessage;
QColor MessageItemPrivate::mColorImportantMessage;
QColor MessageItemPrivate::mColorToDoMessage;
QFont MessageItemPrivate::mFont;
QFont MessageItemPrivate::mFontNewMessage;
QFont MessageItemPrivate::mFontUnreadMessage;
QFont MessageItemPrivate::mFontImportantMessage;
QFont MessageItemPrivate::mFontToDoMessage;

MessageItemPrivate::MessageItemPrivate( MessageItem* qq )
  : ItemPrivate( qq ),
    mThreadingStatus( MessageItem::ParentMissing ),
    mAboutToBeRemoved( false ),
    mAnnotationStateChecked( false ),
    mTagList( 0 )
{
}

MessageItemPrivate::~MessageItemPrivate()
{
  invalidateTagCache();
}

void MessageItemPrivate::invalidateTagCache()
{
  if ( mTagList ) {
    qDeleteAll( *mTagList );
    delete mTagList;
    mTagList = 0;
  }
}

void MessageItemPrivate::invalidateAnnotationCache()
{
  mAnnotationStateChecked = false;
}

const MessageItem::Tag* MessageItemPrivate::bestTag() const
{
  const MessageItem::Tag *best = 0;
  foreach( const MessageItem::Tag* tag, getTagList() ) {
    if ( !best || tag->priority() < best->priority() )
      best = tag;
  }
  return best;
}

void MessageItemPrivate::fillTagList() const
{
  Q_ASSERT( !mTagList );
  mTagList = new QList<MessageItem::Tag*>;

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
      MessageItem::Tag *messageListTag =
          new MessageItem::Tag( SmallIcon( symbol ),
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

QList<MessageItem::Tag*> MessageItemPrivate::getTagList() const
{
  if ( !mTagList )
    fillTagList();

  return *mTagList;
}

bool MessageItemPrivate::tagListInitialized() const
{
  return mTagList != 0;
}

MessageItem::MessageItem()
  : Item( Message, new MessageItemPrivate( this ) ), ModelInvariantIndex()
{
}

MessageItem::~MessageItem()
{
}

QList< MessageItem::Tag * > MessageItem::tagList() const
{
  Q_D( const MessageItem );
  return d->getTagList();
}

bool MessageItem::hasAnnotation() const
{
  Q_D( const MessageItem );
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
  Q_D( const MessageItem );
  if ( hasAnnotation() ) {
    Nepomuk::Resource resource( d->mAkonadiItem.url() );
    return resource.description();
  }
  else return QString();
}

void MessageItem::editAnnotation()
{
  Q_D( MessageItem );
  MessageCore::AnnotationEditDialog *dialog = new MessageCore::AnnotationEditDialog( d->mAkonadiItem.url() );
  dialog->setAttribute( Qt::WA_DeleteOnClose );
  dialog->show();
  // invalidate the cached mHasAnnotation value
  d->mAnnotationStateChecked = false;
}

QString MessageItem::contentSummary() const
{
  Q_D( const MessageItem );
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

const MessageItem::Tag * MessageItemPrivate::findTagInternal( const QString &szTagId ) const
{
  foreach( const MessageItem::Tag *tag, getTagList() ) {
    if ( tag->id() == szTagId )
      return tag;
  }
  return 0;
}

const MessageItem::Tag *MessageItem::findTag( const QString &szTagId ) const
{
  Q_D( const MessageItem );
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
  Q_D( MessageItem );
  d->invalidateTagCache();
}

void MessageItem::invalidateAnnotationCache()
{
  Q_D( MessageItem );
  d->invalidateAnnotationCache();
}

QColor MessageItem::textColor() const
{
  Q_D( const MessageItem );
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
  Q_D( const MessageItem );
  const Tag *bestTag = d->bestTag();
  if ( bestTag != 0 ) {
    return bestTag->backgroundColor();
  } else {
    return QColor();
  }
}

QFont MessageItem::font() const
{
  Q_D( const MessageItem );
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
  Q_D( const MessageItem );
  return d->mSignatureState;
}

void MessageItem::setSignatureState( SignatureState state )
{
  Q_D( MessageItem );
  d->mSignatureState = state;
}

MessageItem::EncryptionState MessageItem::encryptionState() const
{
  Q_D( const MessageItem );
  return d->mEncryptionState;
}

void MessageItem::setEncryptionState( EncryptionState state )
{
  Q_D( MessageItem );
  d->mEncryptionState = state;
}

QByteArray MessageItem::messageIdMD5() const
{
  Q_D( const MessageItem );
  return d->mMessageIdMD5;
}

void MessageItem::setMessageIdMD5( const QByteArray &md5 )
{
  Q_D( MessageItem );
  d->mMessageIdMD5 = md5;
}

QByteArray MessageItem::inReplyToIdMD5() const
{
  Q_D( const MessageItem );
  return d->mInReplyToIdMD5;
}

void MessageItem::setInReplyToIdMD5( const QByteArray& md5 )
{
  Q_D( MessageItem );
  d->mInReplyToIdMD5 = md5;
}

QByteArray MessageItem::referencesIdMD5() const
{
  Q_D( const MessageItem );
  return d->mReferencesIdMD5;
}

void MessageItem::setReferencesIdMD5( const QByteArray& md5 )
{
  Q_D( MessageItem );
  d->mReferencesIdMD5 = md5;
}

void MessageItem::setSubjectIsPrefixed( bool subjectIsPrefixed )
{
  Q_D( MessageItem );
  d->mSubjectIsPrefixed = subjectIsPrefixed;
}

bool MessageItem::subjectIsPrefixed() const
{
  Q_D( const MessageItem );
  return d->mSubjectIsPrefixed;
}

QByteArray MessageItem::strippedSubjectMD5() const
{
  Q_D( const MessageItem );
  return d->mStrippedSubjectMD5;
}

void MessageItem::setStrippedSubjectMD5( const QByteArray& md5 )
{
  Q_D( MessageItem );
  d->mStrippedSubjectMD5 = md5;
}

bool MessageItem::aboutToBeRemoved() const
{
  Q_D( const MessageItem );
  return d->mAboutToBeRemoved;
}

void MessageItem::setAboutToBeRemoved( bool aboutToBeRemoved )
{
  Q_D( MessageItem );
  d->mAboutToBeRemoved = aboutToBeRemoved;
}

MessageItem::ThreadingStatus MessageItem::threadingStatus() const
{
  Q_D( const MessageItem );
  return d->mThreadingStatus;
}

void MessageItem::setThreadingStatus( ThreadingStatus threadingStatus )
{
  Q_D( MessageItem );
  d->mThreadingStatus = threadingStatus;
}

unsigned long MessageItem::uniqueId() const
{
  Q_D( const MessageItem );
  return d->mAkonadiItem.id();
}

Akonadi::Item MessageList::Core::MessageItem::akonadiItem() const
{
  Q_D( const MessageItem );
  return d->mAkonadiItem;
}

void MessageList::Core::MessageItem::setAkonadiItem(const Akonadi::Item& item)
{
  Q_D( MessageItem );
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
  MessageItemPrivate::mColorNewMessage = color;
}


void MessageItem::setUnreadMessageColor( const QColor &color )
{
  MessageItemPrivate::mColorUnreadMessage = color;
}


void MessageItem::setImportantMessageColor( const QColor &color )
{
  MessageItemPrivate::mColorImportantMessage = color;
}


void MessageItem::setToDoMessageColor( const QColor &color )
{
  MessageItemPrivate::mColorToDoMessage = color;
}


void MessageItem::setGeneralFont( const QFont &font )
{
  MessageItemPrivate::mFont = font;
}

void MessageItem::setNewMessageFont( const QFont &font )
{
  MessageItemPrivate::mFontNewMessage = font;
}

void MessageItem::setUnreadMessageFont( const QFont &font )
{
  MessageItemPrivate::mFontUnreadMessage = font;
}

void MessageItem::setImportantMessageFont( const QFont &font )
{
  MessageItemPrivate::mFontImportantMessage = font;
}

void MessageItem::setToDoMessageFont( const QFont &font )
{
  MessageItemPrivate::mFontToDoMessage = font;
}

FakeItem::FakeItem()
  : d( new FakeItemPrivate() )
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


