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

#ifndef __MESSAGELIST_CORE_MODEL_H__
#define __MESSAGELIST_CORE_MODEL_H__

#include <QAbstractItemModel>
#include <QList>
#include <QHash>
#include <QMultiHash>
#include <QDate>
#include <QTimer>

#include <messagelist/core/aggregation.h>
#include <messagelist/core/enums.h>
#include <messagelist/core/sortorder.h>

#include <time.h> // time_t

#include <messagelist/messagelist_export.h>

class QTime;

namespace MessageList
{

namespace Core
{

typedef long int MessageItemSetReference;

class ViewItemJob;
class Filter;
class GroupHeaderItem;
class Item;
class Manager;
class MessageItem;
class Theme;
class StorageModel;
class ModelInvariantRowMapper;
class MessageItemSetManager;
class View;
class ModelPrivate;

/**
 * This class manages the huge tree of displayable objects: GroupHeaderItems and MessageItems.
 * The tree is exposed via a 'hacked' QAbstractItemModel interface to a QTreeView
 * subclass (which is MessageList::View).
 *
 * The keypoint in this class is that it has to be non-blocking in manipulating the tree:
 * fill, cleanup and update operations are performed in timed chunks. Perfect non-blocking
 * behaviour is not possible since there are some small operations that basically can't be
 * split in chunks. However, these exceptions apply to a minority of tasks and in the
 * average case the user will not notice.
 *
 * The data for building the tree is obtained from a subclass of StorageModel. The
 * StorageModel must offer a consistent rappresentation of a "flat" folder containing
 * messages.
 */
class MESSAGELIST_EXPORT Model : public QAbstractItemModel
{
  friend class Item;
  friend class ItemPrivate;

  Q_OBJECT
public:

  /**
   * Creates the mighty Model attached to the specified View.
   */
  explicit Model( View *pParent );

  /**
   * Destroys the mighty model along with the tree of items it manages.
   */
  ~Model();

  /**
   * Returns the StorageModel currently set.
   */
  StorageModel *storageModel() const;

  /**
   * Sets the storage model from that the messages to be displayed should be fetched.
   * The model is then reset and a new fill operation is started. The fill operation may
   * or may not complete before setStorageModel() returns. This depends on the fill
   * strategy and the size of the folder. You can check if the fill operation has
   * completed by looking at the return value of isLoading().
   *
   * Pre-selection is the action of automatically selecting a message just after the folder
   * has finished loading. We may want to select the message that was selected the last
   * time this folder has been open, or we may want to select the first unread message.
   * We also may want to do no pre-selection at all (for example, when the user
   * starts navigating the view before the pre-selection could actually be made
   * and pre-selecting would confuse him). The pre-selection is applied once
   * loading is complete.
   */
  void setStorageModel( StorageModel *storageModel, PreSelectionMode preSelectionMode = PreSelectLastSelected );

  /**
   * Sets the pre-selection mode.
   *
   * Called with PreSelectNone to abort any pending message pre-selection. This may be done if the user
   * starts navigating the view and selecting items before we actually could
   * apply the pre-selection.
   */
  void setPreSelectionMode( PreSelectionMode preSelect );

  /**
   * Returns the hidden root item that all the messages are (or will be) attached to.
   * The returned value is never 0.
   */
  Item *rootItem() const;

  /**
   * Returns true if the view is currently loading, that is
   * it's in the first (possibly lenghty) job batch after attacching to a StorageModel.
   */
  bool isLoading() const;

  /**
   * Returns the message item that is at the _current_ storage row index
   * or zero if no such storage item is found. Please note that this may return 0
   * also if the specified storage row hasn't been actually read yet. This may happen
   * if isLoading() returns true. In this case the only thing you can do is to retry in a while.
   */
  MessageItem * messageItemByStorageRow( int row ) const;

  /**
   * Sets the Aggregation mode.
   * Does not reload the model in any way: you need to call setStorageModel( storageModel() ) for this to happen.
   * The pointer ownership remains of the caller which must ensure its validity until the next
   * call to setAggretation() or until this Model dies. The caller, in fact, is Widget which
   * takes care of meeting the above conditions. The aggregation pointer must not be null.
   */
  void setAggregation( const Aggregation * aggregation );

  /**
   * Sets the Theme.
   * Does not reload the model in any way: you need to call setStorageModel( storageModel() ) for this to happen.
   * The pointer ownership remains of the caller which must ensure its validity until the next
   * call to setTheme() or until this Model dies. The caller, in fact, is Widget which
   * takes care of meeting the above conditions. The theme pointer must not be null.
   */
  void setTheme( const Theme * theme );

  /**
   * Sets the sort order. As with setTheme() and setAggregation(), this does not reload the
   * model in any way.
   */
  void setSortOrder( const SortOrder * sortOrder );

  /**
   * Returns the sort order
   */
  const SortOrder * sortOrder() const;

  /**
   * Sets the Filter to be applied on messages. filter may be null (no filter is applied).
   * The pointer ownership remains of the caller which must ensure its validity until the next
   * call to setFilter() or until this Model dies. The caller, in fact, is Widget which
   * takes care of meeting the above conditions. The Filter pointer may be null.
   */
  void setFilter( const Filter *filter );

  /**
   * Creates a persistent set for the specified MessageItems and
   * returns its reference. Later you can use this reference
   * to retrieve the list of MessageItems that are still valid.
   * See persistentSetActualMessageList() for that.
   *
   * Persistent sets consume resources (both memory and CPU time
   * while manipulating the view) so be sure to call deletePersistentSet()
   * when you no longer need it.
   */
  MessageItemSetReference createPersistentSet( const QList< MessageItem * > &items );

  /**
   * Returns the list of MessageItems that are still existing in the
   * set pointed by the specified reference. This list will contain
   * at most the messages that you have passed to createPersistentSet()
   * but may contain less (even 0) if these MessageItem object were removed
   * from the view for some reason.
   */
  QList< MessageItem * > persistentSetCurrentMessageItemList( MessageItemSetReference ref );

  /**
   * Deletes the persistent set pointed by the specified reference.
   * If the set does not exist anymore, nothing happens.
   */
  void deletePersistentSet( MessageItemSetReference ref );

  // Mandatory QAbstractItemModel interface.

  virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;
  virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;
  QModelIndex index( Item *item, int column ) const;
  virtual QModelIndex parent( const QModelIndex &index ) const;
  virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
  virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

  /// Called when user initiates a drag from the messagelist
  virtual QMimeData* mimeData( const QModelIndexList& indexes ) const;

Q_SIGNALS:
  /**
   * Notify the outside when updating the status bar with a message
   * could be useful
   */
  void statusMessage( const QString &message );

private:
  Q_PRIVATE_SLOT(d, void checkIfDateChanged())
  Q_PRIVATE_SLOT(d, void viewItemJobStep())
  Q_PRIVATE_SLOT(d, void slotStorageModelRowsInserted( const QModelIndex &, int, int ))
  Q_PRIVATE_SLOT(d, void slotStorageModelRowsRemoved( const QModelIndex &, int, int ))
  Q_PRIVATE_SLOT(d, void slotStorageModelDataChanged( const QModelIndex &, const QModelIndex & ))
  Q_PRIVATE_SLOT(d, void slotStorageModelHeaderDataChanged( Qt::Orientation, int, int ))
  Q_PRIVATE_SLOT(d, void slotStorageModelLayoutChanged())
  Q_PRIVATE_SLOT(d, void slotApplyFilter())

  friend class ModelPrivate;
  ModelPrivate * const d;
};

} // namespace Core

} // namespace MessageList

#endif //!__MESSAGELIST_CORE_MODEL_H__
