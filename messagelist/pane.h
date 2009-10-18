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

#ifndef __MESSAGELIST_PANE_H__
#define __MESSAGELIST_PANE_H__

#include <messagelist/core/enums.h>

#include <QtCore/QHash>
#include <QtGui/QTabWidget>

#include <kmime/kmime_message.h>

#include <messagelist/messagelist_export.h>

class KXMLGUIClient;
class QAbstractItemModel;
class QAbstractProxyModel;
class QItemSelectionModel;
class QItemSelection;
class QToolButton;

namespace KPIM
{
  class MessageStatus;
}

namespace Akonadi
{
  class Item;
}

namespace MessageList
{

class Widget;

/**
 * This is the main MessageList panel for Akonadi applications.
 * It contains multiple MessageList::Widget tabs
 * so it can actually display multiple folder sets at once.
 *
 * When a KXmlGuiWindow is passed to setXmlGuiClient, the XMLGUI
 * defined context menu @c akonadi_messagelist_contextmenu is
 * used if available.
 *
 */
class MESSAGELIST_EXPORT Pane : public QTabWidget
{
  Q_OBJECT

public:
  /**
   * Create a Pane wrapping the specified model and selection.
   */
  explicit Pane( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent = 0 );
  ~Pane();

  /**
   * Sets the XML GUI client which the pane is used in.
   *
   * This is needed if you want to use the built-in context menu.
   * Passing 0 is ok and will disable the builtin context menu.
   *
   * @param xmlGuiClient The KXMLGUIClient the view is used in.
   */
  void setXmlGuiClient( KXMLGUIClient *xmlGuiClient );

  /**
   * Returns the current message for the list as Akonadi::Item.
   * May return an invalid Item if there is no current message or no current folder.
   */
  Akonadi::Item currentItem() const;

  /**
   * Returns the current message for the list as KMime::Message::Ptr.
   * May return 0 if there is no current message or no current folder.
   */
  KMime::Message::Ptr currentMessage() const;


  /**
   * Returns the currently selected KMime::Message::Ptr (bound to current StorageModel).
   * The list may be empty if there are no selected messages or no StorageModel.
   *
   * If includeCollapsedChildren is true then the children of the selected but
   * collapsed items are also added to the list.
   *
   * The returned list is guaranteed to be valid only until you return control
   * to the main even loop. Don't store it for any longer. If you need to reference
   * this set of messages at a later stage then take a look at createPersistentSet().
   */
  QList<KMime::Message::Ptr > selectionAsMessageList( bool includeCollapsedChildren = true ) const;

  /**
   * Returns the currently selected Items (bound to current StorageModel).
   * The list may be empty if there are no selected messages or no StorageModel.
   *
   * If includeCollapsedChildren is true then the children of the selected but
   * collapsed items are also added to the list.
   *
   * The returned list is guaranteed to be valid only until you return control
   * to the main even loop. Don't store it for any longer. If you need to reference
   * this set of messages at a later stage then take a look at createPersistentSet().
   */
  QList<Akonadi::Item> selectionAsMessageItemList( bool includeCollapsedChildren = true ) const;

  /**
   * Returns the Akonadi::Item bound to the current StorageModel that
   * are part of the current thread. The current thread is the thread
   * that contains currentMessageItem().
   * The list may be empty if there is no currentMessageItem() or no StorageModel.
   *
   * The returned list is guaranteed to be valid only until you return control
   * to the main even loop. Don't store it for any longer. If you need to reference
   * this set of messages at a later stage then take a look at createPersistentSet().
   */
  QList<Akonadi::Item> currentThreadAsMessageList() const;


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

  /**
   * Sets the focus on the quick search line of the currently active tab.
   */
  void focusQuickSearch();

  /**
   * Returns the KPIM::MessageStatus in the current quicksearch field.
   */
  KPIM::MessageStatus currentFilterStatus() const;

  /**
   * Returns the search term in the current quicksearch field.
   */
  QString currentFilterSearchString() const;


  /**
   * Returns true if the current Aggregation is threaded, false otherwise
   * (or if there is no current Aggregation).
   */
  bool isThreaded() const;

  /**
   * Fast function that determines if the selection is empty
   */
  bool selectionEmpty() const;
  /**
   * Fills the lists of the selected message serial numbers and of the selected+visible ones.
   * Returns true if the returned stats are valid (there is a current folder after all)
   * and false otherwise. This is called by KMMainWidget in a single place so we optimize by
   * making it a single sweep on the selection.
   *
   * If includeCollapsedChildren is true then the children of the selected but
   * collapsed items are also included in the stats
   */
  bool getSelectionStats( QList< quint32 > &selectedSernums,
                          QList< quint32 > &selectedVisibleSernums,
                          bool * allSelectedBelongToSameThread,
                          bool includeCollapsedChildren = true) const;

public slots:
  /**
   * Selects all the items in the current folder.
   */
  void selectAll();

  /**
   * Add a new tab to the Pane and select it.
   */
  void createNewTab();

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

  /**
   * Emitted when a full search is requested.
   */
  void fullSearchRequest();

  /**
   * Notify the outside when updating the status bar with a message
   * could be useful
   */
  void statusMessage( const QString &message );


private:
  Q_PRIVATE_SLOT(d, void onSelectionChanged( const QItemSelection&, const QItemSelection& ))
  Q_PRIVATE_SLOT(d, void onNewTabClicked())
  Q_PRIVATE_SLOT(d, void onCloseTabClicked())
  Q_PRIVATE_SLOT(d, void onCurrentTabChanged())
  Q_PRIVATE_SLOT(d, void onTabContextMenuRequest( const QPoint& ))
  Q_PRIVATE_SLOT(d, void updateTabControls())

  class Private;
  Private * const d;
};

} // namespace MessageList

#endif //!__MESSAGELIST_PANE_H__
