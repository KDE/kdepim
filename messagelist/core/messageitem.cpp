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
#include "core/messageitem.h"

using namespace MessageList::Core;

class MessageItem::Tag::Private
{
public:
  QPixmap mPixmap;      ///< The pixmap associated to this tag
  QString mName;        ///< The name of this tag
  QString mId;          ///< The unique id of this tag
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


class MessageItem::Private
{
public:
  /**
   * Linear search in the list of tags. The lists of tags
   * associated to a message are supposed to be very short (c'mon.. you won't add more than a couple of tags to a single msg).
   * so a linear search is better than a hash lookup in most cases.
   */
  Tag *findTagInternal( const QString &szTagId ) const;

  ThreadingStatus mThreadingStatus;
  QString mMessageIdMD5;            ///< always set
  QString mInReplyToIdMD5;          ///< set only if we're doing threading
  QString mReferencesIdMD5;         ///< set only if we're doing threading
  QString mStrippedSubjectMD5;      ///< set only if we're doing threading
  bool mSubjectIsPrefixed;          ///< set only if we're doing subject based threading
  EncryptionState mEncryptionState;
  SignatureState mSignatureState;
  QList< Tag * > mTagList;          ///< Usually empty
  QColor mTextColor;                ///< If invalid, use default text color
  QColor mBackgroundColor;          ///< If invalid, use default background color
  QFont  mFont;
  unsigned long mUniqueId;          ///< The unique id of this message (serial number of KMMsgBase at the moment of writing)

  bool mAboutToBeRemoved;           ///< Set to true when this item is going to be deleted and shouldn't be selectable
};

MessageItem::MessageItem()
  : Item( Message ), ModelInvariantIndex(), d( new Private )
{
  d->mThreadingStatus = MessageItem::ParentMissing;
  d->mAboutToBeRemoved = false;
  d->mUniqueId = 0;
}

MessageItem::~MessageItem()
{
  qDeleteAll( d->mTagList );
  delete d;
}

const QList< MessageItem::Tag * > MessageItem::tagList() const
{
  return d->mTagList;
}

void MessageItem::setTagList( const QList< Tag * > &list )
{
  qDeleteAll( d->mTagList );
  d->mTagList = list;
}

MessageItem::Tag * MessageItem::Private::findTagInternal( const QString &szTagId ) const
{
  foreach( Tag *tag, mTagList ) {
    if ( tag->id() == szTagId )
      return tag;
  }
  return 0;
}

MessageItem::Tag *MessageItem::findTag( const QString &szTagId ) const
{
  return d->findTagInternal( szTagId );
}

QString MessageItem::tagListDescription() const
{
  QString ret;

  foreach( const Tag *tag, d->mTagList ) {
    if ( !ret.isEmpty() )
      ret += ", ";
    ret += tag->name();
  }

  return ret;
}

QColor MessageItem::textColor() const
{
  return d->mTextColor;
}

QColor MessageItem::backgroundColor() const
{
  return d->mBackgroundColor;
}

QFont MessageItem::font() const
{
  return d->mFont;
}

void MessageItem::setTextColor( const QColor &clr )
{
  d->mTextColor = clr;
}

void MessageItem::setBackgroundColor( const QColor &clr )
{
  d->mBackgroundColor = clr;
}

void MessageItem::setFont( const QFont &f )
{
  d->mFont = f;
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

QString MessageItem::messageIdMD5() const
{
  return d->mMessageIdMD5;
}

void MessageItem::setMessageIdMD5( const QString &md5 )
{
  d->mMessageIdMD5 = md5;
}

QString MessageItem::inReplyToIdMD5() const
{
  return d->mInReplyToIdMD5;
}

void MessageItem::setInReplyToIdMD5( const QString &md5 )
{
  d->mInReplyToIdMD5 = md5;
}

QString MessageItem::referencesIdMD5() const
{
  return d->mReferencesIdMD5;
}

void MessageItem::setReferencesIdMD5( const QString &md5 )
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

QString MessageItem::strippedSubjectMD5() const
{
  return d->mStrippedSubjectMD5;
}

void MessageItem::setStrippedSubjectMD5( const QString &md5 )
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
  return d->mUniqueId;
}

void MessageItem::setUniqueId( unsigned long uniqueId )
{
  d->mUniqueId = uniqueId;
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
