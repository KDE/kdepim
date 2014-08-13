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

#ifndef __MESSAGELIST_CORE_ITEM_P_H__
#define __MESSAGELIST_CORE_ITEM_P_H__

#include "core/item.h"

#include "messagecore/utils/stringutil.h"

// See the MessageList::ItemPrivate::insertChildItem() function below for an explaination of this macro.
#if __GNUC__ >= 3  //krazy:exclude=cpp
  #define GCC_DONT_INLINE_THIS __attribute__((noinline))
#else
  #define GCC_DONT_INLINE_THIS
#endif

namespace MessageList
{

namespace Core
{

class ItemPrivate
{
public:
  explicit ItemPrivate( Item *owner )
    : q( owner ),
      mChildItems( 0 ),
      mParent( 0 ),
      mThisItemIndexGuess( 0 ),
      mInitialExpandStatus( Item::NoExpandNeeded ),
      mIsViewable( false ),
      mUseReceiver( false )
  {
  }
  virtual ~ItemPrivate() {}

  /**
   * Implements "in the middle" insertions of child items.
   * The template argument class must export a static inline bool firstGreaterOrEqual( Item *, Item * )
   * function which must return true when the first parameter item is considered to be greater
   * or equal to the second parameter item and false otherwise.
   *
   * The insertion function *IS* our very bottleneck on flat views
   * (when there are items with a lot of children). This is somewhat pathological...
   * beside the binary search based insertion sort we actually can only do "statement level" optimization.
   * I've found no better algorithms so far. If someone has a clever idea, please write to pragma
   * at kvirc dot net :)
   *
   * GCC_DONT_INLINE_THIS is a macro defined above to __attribute__((noinline))
   * if the current compiler is gcc. Without this attribute gcc attempts to inline THIS
   * function inside the callers. The problem is that while inlining this function
   * it doesn't inline the INNER comparison functions (which we _WANT_ to be inlined)
   * because they would make the caller function too big.
   *
   * This is what gcc reports with -Winline:
   *
   * /home/pragma/kmail-soc/kmail/messagelistview/item.h:352: warning: inlining failed in call to
   *   'static bool MessageList::ItemSubjectComparator::firstGreaterOrEqual(MessageList::Item*, MessageList::Item*)':
   *    --param large-function-growth limit reached while inlining the caller
   * /home/pragma/kmail-soc/kmail/messagelistview/model.cpp:239: warning: called from here
   *
   * The comparison functions then appear in the symbol table:
   *
   * etherea kmail # nm /usr/kde/4.0/lib/libkmailprivate.so | grep Comparator
   * 00000000005d2c10 W _ZN5KMail15MessageList18ItemDateComparator19firstGreaterOrEqualEPNS0_4ItemES3_
   * 00000000005d2cb0 W _ZN5KMail15MessageList20ItemSenderComparator19firstGreaterOrEqualEPNS0_4ItemES3_
   * 00000000005d2c50 W _ZN5KMail15MessageList21ItemSubjectComparator19firstGreaterOrEqualEPNS0_4ItemES3_
   * ...
   *
   * With this attribute, instead, gcc doesn't complain at all and the inner comparisons
   * *seem* to be inlined correctly (there is no sign of them in the symbol table).
   */
  template< class ItemComparator, bool bAscending > int GCC_DONT_INLINE_THIS insertChildItem( Model *model, Item *child )
  {
    if ( !mChildItems )
      return q->appendChildItem( model, child );

    int cnt = mChildItems->count();
    if ( cnt < 1 )
      return q->appendChildItem( model, child );

    int idx;
    Item * pivot;

    if ( bAscending )
    {
      pivot = mChildItems->at( cnt - 1 );

      if ( ItemComparator::firstGreaterOrEqual( child, pivot ) ) // gcc: <-- inline this instead, thnx
        return q->appendChildItem( model, child ); // this is very likely in date based comparisons (FIXME: not in other ones)

      // Binary search based insertion
      int l = 0;
      int h = cnt - 1;

      for(;;)
      {
        idx = (l + h) / 2;
        pivot = mChildItems->at( idx );
        if ( ItemComparator::firstGreaterOrEqual( pivot, child ) ) // gcc: <-- inline this instead, thnx
        {
          if ( l < h )
            h = idx - 1;
          else
            break;
        } else {
          if ( l < h )
            l = idx + 1;
          else {
            idx++;
            break;
          }
        }
      }
    } else {

      pivot = mChildItems->at( 0 );
      if ( ItemComparator::firstGreaterOrEqual( child, pivot ) ) // gcc: <-- inline this instead, thnx
        idx = 0;  // this is very likely in date based comparisons (FIXME: not in other ones)
      else {

        // Binary search based insertion
        int l = 0;
        int h = cnt - 1;

        for(;;)
        {
          idx = (l + h) / 2;
          pivot = mChildItems->at( idx );
          if ( ItemComparator::firstGreaterOrEqual( child, pivot ) ) // gcc: <-- inline this instead, thnx
          {
            if ( l < h )
              h = idx - 1;
            else
              break;
          } else {
            if ( l < h )
              l = idx + 1;
            else {
              idx++;
              break;
            }
          }
        }
      }
    }

    Q_ASSERT( idx >= 0 );
    Q_ASSERT( idx <= mChildItems->count() );

    if ( mIsViewable && model )
      model->beginInsertRows( model->index( q, 0 ), idx, idx ); // BLEAH :D

    mChildItems->insert( idx, child );
    child->setIndexGuess( idx );
    if ( mIsViewable )
    {
      if ( model )
        model->endInsertRows(); // BLEAH :D
      child->setViewable( model, true );
    }

    return idx;
  }

  /**
   * Checks if the specified child item is actually in the wrong
   * position in the child list and returns true in that case.
   * Returns false if the item is already in the right position
   * and no re-sorting is needed.
   */
  template< class ItemComparator, bool bAscending > bool childItemNeedsReSorting( Item * child )
  {
    if ( !mChildItems )
      return false; // not my child! (ugh... should assert ?)

    const int idx = q->indexOfChildItem(child);

    if ( idx > 0 )
    {
      Item * prev = mChildItems->at( idx - 1 );
      if ( bAscending )
      {
        // child must be greater or equal to the previous item
        if ( !ItemComparator::firstGreaterOrEqual( child, prev ) )
          return true; // wrong order: needs re-sorting
      } else {
        // previous must be greater or equal to the child item
        if ( !ItemComparator::firstGreaterOrEqual( prev, child ) )
          return true; // wrong order: needs re-sorting
      }
    }

    if ( idx < ( mChildItems->count() - 1 ) )
    {
      Item * next = mChildItems->at( idx + 1 );
      if ( bAscending )
      {
        // next must be greater or equal to child
        if ( !ItemComparator::firstGreaterOrEqual( next, child ) )
          return true; // wrong order: needs re-sorting
      } else {
        // child must be greater or equal to next
        if ( !ItemComparator::firstGreaterOrEqual( child, next ) )
          return true; // wrong order: needs re-sorting
      }
    }

    return false;
  }

  /**
   * Internal handler for managing the children list.
   */
  void childItemDead( Item * child );

  Item * const q;

  QList< Item * > *mChildItems;               ///< List of children, may be 0
  Item * mParent;                             ///< The parent view item
  time_t mMaxDate;                            ///< The maximum date in the subtree
  time_t mDate;                               ///< The date of the message (or group date)
  size_t mSize;                               ///< The size of the message in bytes
  QString mSender;                            ///< The sender of the message (or group sender)
  QString mReceiver;                          ///< The receiver of the message (or group receiver)
  int mThisItemIndexGuess;                    ///< The guess for the index in the parent's child list
  QString mSubject;                           ///< The subject of the message (or group subject)
  Akonadi::MessageStatus mStatus;             ///< The status of the message (may be extended to groups in the future)
  qint64 mItemId;                             ///< The Akonadi item id
  qint64 mParentCollectionId;                 ///< The Akonadi ID of collection that this particular item comes from (can be virtual collection)
  Item::Type mType : 4;                       ///< The type of this item
  Item::InitialExpandStatus mInitialExpandStatus : 4; ///< The expand status we have to honor when we attach to the viewable root
  bool mIsViewable : 1;                       ///< Is this item attacched to the viewable root ?
  bool mUseReceiver : 1;                      ///< senderOrReceiver() returns receiver rather than sender
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemSizeComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( first->size() < second->size() )
      return false;
    // When the sizes are equal compare by date too
    if ( first->size() == second->size() )
      return first->date() >= second->date();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemDateComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    // When the dates are equal compare by subject too
    // This is useful, for example, in kernel mailing list where people
    // send out multiple messages with patch parts at exactly the same time.
    if ( first->date() == second->date() )
      return first->subject() >= second->subject();
    if ( first->date() == static_cast<uint>( -1 ) ) // invalid is always smaller
      return false;
    if ( second->date() == static_cast<uint>( -1 ) )
      return true;
    if ( first->date() < second->date() )
      return false;
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemMaxDateComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( first->maxDate() < second->maxDate() )
      return false;
    if ( first->maxDate() == second->maxDate() )
      return first->subject() >= second->subject();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemSubjectComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    const int ret = MessageCore::StringUtil::stripOffPrefixes( first->subject() ).
                compare( MessageCore::StringUtil::stripOffPrefixes( second->subject() ), Qt::CaseInsensitive );
    if ( ret < 0 )
      return false;
    // compare by date when subjects are equal
    if ( ret == 0 )
      return first->date() >= second->date();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemSenderComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    const int ret = MessageCore::StringUtil::stripEmailAddr( first->sender() ).compare(
      MessageCore::StringUtil::stripEmailAddr( second->sender() ), Qt::CaseInsensitive );
    if ( ret < 0 )
      return false;
    // compare by date when senders are equal
    if ( ret == 0 )
      return first->date() >= second->date();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemReceiverComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    const int ret = MessageCore::StringUtil::stripEmailAddr( first->receiver() ).compare(
      MessageCore::StringUtil::stripEmailAddr( second->receiver() ), Qt::CaseInsensitive );
    if ( ret < 0 )
      return false;
    // compare by date when receivers are equal
    if ( ret == 0 )
      return first->date() >= second->date();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemSenderOrReceiverComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    const int ret = MessageCore::StringUtil::stripEmailAddr( first->senderOrReceiver() ).compare(
      MessageCore::StringUtil::stripEmailAddr( second->senderOrReceiver() ), Qt::CaseInsensitive );
    if ( ret < 0 )
      return false;
    // compare by date when sender/receiver are equal
    if ( ret == 0 )
      return first->date() >= second->date();
    return true;
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemActionItemStatusComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( first->status().isToAct() )
    {
      if ( second->status().isToAct() )
        return first->date() >= second->date();
      return true;
    }
    if ( second->status().isToAct() )
      return false;
    return first->date() >= second->date();
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemUnreadStatusComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( !first->status().isRead() )
    {
      // fist is unread
      if ( !second->status().isRead() )
        return first->date() >= second->date(); // both are unread
      // unread comes always first with respect to non-unread
      return true;
    }
    if ( !second->status().isRead() )
      return false;
    // both are read
    return first->date() >= second->date();
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemImportantStatusComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( !first->status().isImportant() )
    {
      // fist is unread
      if ( !second->status().isImportant() )
        return first->date() >= second->date(); // both are unread
      // unread comes always first with respect to non-unread
      return true;
    }
    if ( !second->status().isImportant() )
      return false;
    // both are read
    return first->date() >= second->date();
  }
};

/**
 * A helper class used with MessageList::Item::childItemNeedsReSorting() and
 * MessageList::Item::insertChildItem().
 */
class ItemAttachmentStatusComparator
{
public:
  static inline bool firstGreaterOrEqual( Item * first, Item * second )
  {
    if ( !first->status().hasAttachment() )
    {
      // fist is unread
      if ( !second->status().hasAttachment() )
        return first->date() >= second->date(); // both are unread
      // unread comes always first with respect to non-unread
      return true;
    }
    if ( !second->status().hasAttachment() )
      return false;
    // both are read
    return first->date() >= second->date();
  }
};


} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_ITEM_P_H__
