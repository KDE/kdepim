/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __MESSAGELIST_WIDGET_H__
#define __MESSAGELIST_WIDGET_H__

#include <messagelist/core/widgetbase.h>

#include <akonadi/item.h>

#include <messagelist/messagelist_export.h>

class QWidget;

namespace MessageList
{

/**
 * The Akonadi specific implementation of the Core::Widget.
 */
class MESSAGELIST_EXPORT Widget : public MessageList::Core::Widget
{
  Q_OBJECT

public:
  /**
   * Create a Widget wrapping the specified folder.
   */
  explicit Widget( QWidget *parent );
  ~Widget();

  /**
   * Returns true if this drag can be accepted by the underlying view
   */
  bool canAcceptDrag( const QDragMoveEvent *e );

  /**
   * Selects the next message item in the view.
   *
   * messageTypeFilter can be used to restrict the selection to only certain message types.
   *
   * existingSelectionBehaviour specifies how the existing selection
   * is manipulated. It may be cleared, expanded or grown/shrinked.
   *
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "next" algorithm will restart from the beginning
   * of the list if the end is reached, otherwise it will just stop returning false.
   */
  bool selectNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                              MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                              bool centerItem,
                              bool loop );

  /**
   * Selects the previous message item in the view.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   *
   * messageTypeFilter can be used to restrict the selection to only certain message types.
   *
   * existingSelectionBehaviour specifies how the existing selection
   * is manipulated. It may be cleared, expanded or grown/shrinked.
   *
   * If loop is true then the "previous" algorithm will restart from the end
   * of the list if the beginning is reached, otherwise it will just stop returning false.
   */
  bool selectPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter,
                                  MessageList::Core::ExistingSelectionBehaviour existingSelectionBehaviour,
                                  bool centerItem,
                                  bool loop );

  /**
   * Focuses the next message item in the view without actually selecting it.
   *
   * messageTypeFilter can be used to restrict the selection to only certain message types.
   *
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "next" algorithm will restart from the beginning
   * of the list if the end is reached, otherwise it will just stop returning false.
   */
  bool focusNextMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop );

  /**
   * Focuses the previous message item in the view without actually selecting it.
   *
   * messageTypeFilter can be used to restrict the selection to only certain message types.
   *
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   * If loop is true then the "previous" algorithm will restart from the end
   * of the list if the beginning is reached, otherwise it will just stop returning false.
   */
  bool focusPreviousMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem, bool loop );

  /**
   * Selects the currently focused message item. May do nothing if the
   * focused message item is already selected (which is very likely).
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   */
  void selectFocusedMessageItem( bool centerItem );

  /**
   * Selects the first message item in the view that matches the specified Core::MessageTypeFilter.
   * If centerItem is true then the specified item will be positioned
   * at the center of the view, if possible.
   *
   * If the current view is already loaded then the request will
   * be satisfied immediately (well... if an unread message exists at all).
   * If the current view is still loading then the selection of the first
   * message will be scheduled to be executed when loading terminates.
   *
   * So this function doesn't actually guarantee that an unread or new message
   * was selected when the call returns. Take care :)
   *
   * The function returns true if a message was selected and false otherwise.
   */
  bool selectFirstMessageItem( MessageList::Core::MessageTypeFilter messageTypeFilter, bool centerItem );

  /**
   * Selects all the items in the current folder.
   */
  void selectAll();
  /**
   * If expand is true then it expands the current thread, otherwise
   * collapses it.
   */
  void setCurrentThreadExpanded( bool expand );

  /**
   * If expand is true then it expands all the threads, otherwise
   * collapses them.
   */
  void setAllThreadsExpanded( bool expand );

  /**
   * If expand is true then it expands all the groups (only the toplevel
   * group item: inner threads are NOT expanded). If expand is false
   * then it collapses all the groups. If no grouping is in effect
   * then this function does nothing.
   */
  void setAllGroupsExpanded( bool expand );

protected:
  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewMessageSelected( MessageList::Core::MessageItem *msg );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewMessageActivated( MessageList::Core::MessageItem *msg );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewSelectionChanged();

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewMessageListContextPopupRequest( const QList< MessageList::Core::MessageItem * > &selectedItems, const QPoint &globalPos );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewGroupHeaderContextPopupRequest( MessageList::Core::GroupHeaderItem *group, const QPoint &globalPos );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewDragEnterEvent( QDragEnterEvent * e );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewDragMoveEvent( QDragMoveEvent * e );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewDropEvent( QDropEvent * e );

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewStartDragRequest();

  /**
   * Reimplemented from MessageList::Core::Widget
   */
  virtual void viewMessageStatusChangeRequest( MessageList::Core::MessageItem *msg, const KPIM::MessageStatus &set, const KPIM::MessageStatus &clear );

signals:
  /**
   * Emitted when a message is selected (that is, single clicked and thus made current in the view)
   * Note that this message CAN be 0 (when the current item is cleared, for example).
   *
   * This signal is emitted when a SINGLE message is selected in the view, probably
   * by clicking on it or by simple keyboard navigation. When multiple items
   * are selected at once (by shift+clicking, for example) then you will get
   * this signal only for the last clicked message (or at all, if the last shift+clicked
   * thing is a group header...). You should handle selection changed in this case.
   */
  void messageSelected( const Akonadi::Item &item );

  /**
   * Emitted when a message is doubleclicked or activated by other input means
   */
  void messageActivated( const Akonadi::Item &item );

  /**
   * Emitted when the selection in the view changes.
   */
  void selectionChanged();

  /**
   * Emitted when a message wants its status to be changed
   */
  void messageStatusChangeRequest( const Akonadi::Item &item, const KPIM::MessageStatus &set, const KPIM::MessageStatus &clear );

private:
  class Private;
  Private * const d;
};

} // namespace MessageList

#endif //!__MESSAGELIST_WIDGET_H__
