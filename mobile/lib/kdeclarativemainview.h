/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#ifndef KDECLARATIVEMAINVIEW_H
#define KDECLARATIVEMAINVIEW_H

#include "kdeclarativefullscreenview.h"
#include <QItemSelectionModel>

class QAbstractItemModel;
class QAbstractProxyModel;
class KLineEdit;

namespace Akonadi {
class ChangeRecorder;
class EntityTreeModel;
class Item;
class ItemFetchScope;
}

class ListProxy;
class KDeclarativeMainViewPrivate;

/**
 * Main view for mobile applications. This class is just to share code and therefore
 * should not be instantiated by itself.
 */
class MOBILEUI_EXPORT KDeclarativeMainView : public KDeclarativeFullScreenView
{
  Q_OBJECT
  Q_PROPERTY(int numSelectedAccounts READ numSelectedAccounts NOTIFY numSelectedAccountsChanged)
  Q_PROPERTY(bool isLoadingSelected READ isLoadingSelected NOTIFY isLoadingSelectedChanged)
  Q_PROPERTY(QString version READ version CONSTANT)

protected:
  /**
   * Creates a new main view for a mobile application.
   *
   * @param appName is used to find the QML file in ${DATA_DIR}/mobile/appname.qml
   * @param listProxy proxy for the list view of the application. KDeclarativeMainView
                      takes ownwership over the pointer.
   */
  KDeclarativeMainView( const QString &appName, ListProxy *listProxy, QWidget *parent = 0 );

  /** Returns the central ETM. */
  Akonadi::EntityTreeModel* entityTreeModel() const;
  /** Returns the filtered and QML-adapted item model. */
  QAbstractItemModel* itemModel() const;

  /**
   * Returns whether the currently selected item is being loaded.
   * Note that results appear asynchronously in chunks while loading the contents
   * of a collection. That means that the number of items can be greater then zero
   * while isLoadingSelected returns true.
   */
  bool isLoadingSelected();

  /**
   * Initializes the standard action manager that will be used by the application.
   * This is a point of extension to use a custom action manager.
   *
   * @param collectionSelectionModel The selection model for the collections.
   * @param itemSelectionModel The selection model for the items.
   */
  virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel );

  /**
   * Initializes the agent action manager that will be used by the application.
   * This is a point of extension to use a custom action manager.
   *
   * @param selectionModel The selection model for the agent instances.
   */
  virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

  /**
   * Returns the filter proxy model that will be used to filter the item list.
   * If @c 0 is returned, no filtering is done.
   *
   * @note The model has to provide a public slot with the following signature:
   *       void setFilterString( const QString& )
   */
  virtual QAbstractProxyModel* itemFilterModel() const;

protected slots:
  void delayedInit();

public:
  virtual ~KDeclarativeMainView();

  /**
    * Item fetch scope to specify how much data should be loaded for the list view.
    * By default nothing is loaded.
    */
  Akonadi::ItemFetchScope& itemFetchScope();

  /** Mime type of the items handled by this application. */
  void addMimeType( const QString &mimeType );

  QStringList mimeTypes() const;

  int numSelectedAccounts();

  QString version() const;

  Akonadi::ChangeRecorder* monitor() const;

  void setFilterLineEdit( KLineEdit *lineEdit );

public slots:
  void setSelectedAccount( int row );
  int selectedCollectionRow();

  // FIXME: make non-virtual again once mark-as-read logic is in messageviewer
  virtual void setListSelectedRow( int row );
  bool blockHook();
  void unblockHook( bool block);

  void setAgentInstanceListSelectedRow( int row );

  QString pathToItem( qint64 id );

  void launchAccountWizard();
  void synchronizeAllItems();

  void saveFavorite();
  void loadFavorite( const QString &name );
  void multipleSelectionFinished();

  void persistCurrentSelection(const QString &key);
  void clearPersistedSelection(const QString &key);
  void restorePersistedSelection(const QString &key);

signals:
  void numSelectedAccountsChanged();
  void selectedItemChanged( int row, qlonglong itemId );
  void isLoadingSelectedChanged();

protected:
  QItemSelectionModel* regularSelectionModel() const;
  QAbstractItemModel *regularSelectedItems() const;
  QItemSelectionModel* itemSelectionModel() const;
  QItemSelectionModel* itemActionModel() const;
  QAbstractItemModel* selectedItemsModel() const;

  Akonadi::Item itemFromId( quint64 id ) const;

  virtual void keyPressEvent( QKeyEvent *event );

private:
  KDeclarativeMainViewPrivate * const d;
  Q_DISABLE_COPY( KDeclarativeMainView )

  Q_PRIVATE_SLOT( d, void filterLineEditChanged( const QString& ) )
};

#endif // KDECLARATIVEMAINVIEW_H
