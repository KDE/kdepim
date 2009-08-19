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
 * The KMail specific implementation of the Core::Widget.
 *
 * Provides an interface over a KMFolder. In the future
 * it's expected to wrap Akonadi::MessageModel.
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
  Akonadi::Item::List selectionAsItems() const;
  Akonadi::Item itemForRow( int row ) const;

  int mLastSelectedMessage;
};

} // namespace MessageList

#endif //!__MESSAGELIST_WIDGET_H__
