/*
    Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

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


namespace Akonadi {
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

public slots:
  void setSelectedAccount( int row );
  void setSelectedChildCollectionRow( int row );
  void setSelectedBreadcrumbCollectionRow( int row );
  int selectedCollectionRow();

  // FIXME: make non-virtual again once mark-as-read logic is in messageviewer
  virtual void setListSelectedRow( int row );

  /** Returns wheter or not the child collection at row @param row has children. */
  bool childCollectionHasChildren( int row );

  QString pathToItem( qint64 id );

  void launchAccountWizard();

  void saveFavorite();
  void loadFavorite( const QString &name );

  void configureCurrentAccount();

  void persistCurrentSelection(const QString &key);
  void clearPersistedSelection(const QString &key);
  void restorePersistedSelection(const QString &key);

signals:
  void numSelectedAccountsChanged();
  void selectedItemChanged( int row, qlonglong itemId );
  void isLoadingSelectedChanged();

protected:
  QItemSelectionModel* regularSelectionModel() const;
  QItemSelectionModel* favoriteSelectionModel() const;
  QAbstractItemModel *regularSelectedItems() const;
  QAbstractItemModel *favoriteSelectedItems() const;
  QItemSelectionModel* itemSelectionModel() const;
  QAbstractItemModel* selectedItemsModel() const;

  Akonadi::Item itemFromId( quint64 id ) const;

private:
  KDeclarativeMainViewPrivate * const d;
  Q_DISABLE_COPY( KDeclarativeMainView )
};

#endif // KDECLARATIVEMAINVIEW_H
