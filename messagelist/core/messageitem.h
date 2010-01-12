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
  QList< Tag * > * tagList() const;

  void setTagList( QList< Tag * > * list );

  /**
   * Returns Tag associated to this message that has the specified id or 0
   * if no such tag exists. mTagList will be 0 in 99% of the cases.
   */
  Tag * findTag( const QString &szTagId ) const;

  QString tagListDescription() const;

  QColor textColor() const;

  QColor backgroundColor() const;

  QFont font() const;

  void setTextColor( const QColor &clr );

  void setBackgroundColor( const QColor &clr );

  void setFont( const QFont &f );

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

  void setUniqueId(unsigned long uniqueId);

  MessageItem * topmostMessage();

  /**
   * Appends the whole subtree originating at this item
   * to the specified list. This item is included!
   */
  void subTreeToList( QList< MessageItem * > &list );

private:
  class Private;
  Private * const d;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_MESSAGEITEM_H__
