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

#ifndef __MESSAGELIST_CORE_MESSAGEITEM_H__
#define __MESSAGELIST_CORE_MESSAGEITEM_H__

#include <messagelist/core/item.h>
#include <messagelist/core/modelinvariantindex.h>

#include <QPixmap>
#include <QString>
#include <QColor>

#include <messagelist/messagelist_export.h>

namespace MessageList
{

namespace Core
{

class MESSAGELIST_EXPORT MessageItem : public Item, public ModelInvariantIndex
{
public:
  class MESSAGELIST_EXPORT Tag
  {
  public:
    Tag( const QPixmap &pix, const QString &tagName, const QString &tagId );
    ~Tag();
    QPixmap pixmap() const;
    QString name() const;
    QString id() const;
    QColor textColor() const;
    QColor backgroundColor() const;
    QFont font() const;
    int priority() const;

    void setTextColor( const QColor &textColor );
    void setBackgroundColor( const QColor &backgroundColor );
    void setFont( const QFont &font );
    void setPriority( int priority );

  private:
    class Private;
    Private * const d;
  };

  enum ThreadingStatus
  {
    PerfectParentFound,     ///< this message found a perfect parent to attach to
    ImperfectParentFound,   ///< this message found an imperfect parent to attach to (might be fixed later)
    ParentMissing,          ///< this message might belong to a thread but its parent is actually missing
    NonThreadable           ///< this message does not look as being threadable
  };

  enum EncryptionState
  {
    NotEncrypted,
    PartiallyEncrypted,
    FullyEncrypted,
    EncryptionStateUnknown
  };

  enum SignatureState
  {
    NotSigned,
    PartiallySigned,
    FullySigned,
    SignatureStateUnknown
  };

  MessageItem();
  virtual ~MessageItem();

public:

  /// Returns the list of tags for this item.
  /// The tags are fetched from Nepomuk, you need to call setNepomukResourceURI() before so that
  /// the correct Nepomuk::Resource is used for the tags.
  virtual QList< Tag * > tagList() const;

  /**
   * Returns Tag associated to this message that has the specified id or 0
   * if no such tag exists. mTagList will be 0 in 99% of the cases.
   */
  const Tag * findTag( const QString &szTagId ) const;

  QString tagListDescription() const;

  /// Deletes all cached tags. The next time someone asks this item for the tags, they are
  /// fetched from Nepomuk again
  void invalidateTagCache();

  QColor textColor() const;

  QColor backgroundColor() const;

  QFont font() const;

  SignatureState signatureState() const;

  void setSignatureState( SignatureState state );

  EncryptionState encryptionState() const;

  void setEncryptionState( EncryptionState state );

  QString messageIdMD5() const;

  void setMessageIdMD5( const QString &md5 );

  QString inReplyToIdMD5() const;

  void setInReplyToIdMD5( const QString &md5 );

  QString referencesIdMD5() const;

  void setReferencesIdMD5( const QString &md5 );

  void setSubjectIsPrefixed( bool subjectIsPrefixed );

  bool subjectIsPrefixed() const;

  QString strippedSubjectMD5() const;

  void setStrippedSubjectMD5( const QString &md5 );

  bool aboutToBeRemoved() const;

  void setAboutToBeRemoved( bool aboutToBeRemoved );

  ThreadingStatus threadingStatus() const;

  void setThreadingStatus( ThreadingStatus threadingStatus );

  unsigned long uniqueId() const;

  void setUniqueId( unsigned long uniqueId );

  void setNepomukResourceURI( const QUrl &nepomukUri );

  MessageItem * topmostMessage();

  /**
   * Appends the whole subtree originating at this item
   * to the specified list. This item is included!
   */
  void subTreeToList( QList< MessageItem * > &list );

  //
  // Colors and fonts shared by all message items.
  // textColor() and font() will take the message status into account and return
  // one of these.
  // Call these setters only once when reading the colors from the config file.
  //
  static void setNewMessageColor( const QColor &color );
  static void setUnreadMessageColor( const QColor &color );
  static void setImportantMessageColor( const QColor &color );
  static void setToDoMessageColor( const QColor &color );
  static void setGeneralFont( const QFont &font );
  static void setNewMessageFont( const QFont &font );
  static void setUnreadMessageFont( const QFont &font );
  static void setImportantMessageFont( const QFont &font );
  static void setToDoMessageFont( const QFont &font );

private:
  class Private;
  Private * const d;
};

/// A message item that can have a fake tag list
class MESSAGELIST_EXPORT FakeItem : public MessageItem
{
  public:

    FakeItem();
    ~FakeItem();

    /// Reimplemented to return the fake tag list
    virtual QList< Tag * > tagList() const;

    /// Sets a list of fake tags for this item
    void setFakeTags( const QList< Tag* > &tagList );

  private:

    class Private;
    Private * const d;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_MESSAGEITEM_H__
