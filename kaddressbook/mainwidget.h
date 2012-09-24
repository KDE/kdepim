/*
  This file is part of KAddressBook.

  Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "kaddressbook_export.h"

#include <QWidget>

namespace Akonadi {
  class Collection;
  class ContactGroupViewer;
  class ContactViewer;
  class ContactsFilterProxyModel;
  class EntityMimeTypeFilterModel;
  class EntityTreeView;
  class Item;
  class ItemView;
  class StandardContactActionManager;
}

class ContactSwitcher;
class KActionCollection;
class KXMLGUIClient;
class ModelColumnManager;
class QAbstractItemModel;
class QItemSelectionModel;
class QModelIndex;
class QSplitter;
class QStackedWidget;
class QuickSearchWidget;
class XXPortManager;
class QActionGroup;

class KADDRESSBOOK_EXPORT MainWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit MainWidget( KXMLGUIClient *guiClient, QWidget *parent = 0 );
    ~MainWidget();

  public Q_SLOTS:
    void newContact();
    void newGroup();
    void print();
    void configure();

  private Q_SLOTS:
    void delayedInit();

    void itemSelected( const Akonadi::Item &item );
    void itemSelectionChanged( const QModelIndex &, const QModelIndex & );
    void selectFirstItem();

    void setViewMode( QAction *action );
    void setQRCodeShow( bool on );

    void restoreState();
    void saveState();

  private:
    void setupGui();
    void setupActions( KActionCollection * );
    bool showQRCodes();
    void setViewMode( int mode );
    void saveSplitterStates() const;
    void restoreSplitterStates();

    QAbstractItemModel *allContactsModel();

    /**
     * Returns the address book collection that is currently
     * selected by the user or an invalid collection if no
     * address book is selected.
     */
    Akonadi::Collection currentAddressBook() const;

    Akonadi::EntityMimeTypeFilterModel *mCollectionTree;
    Akonadi::EntityMimeTypeFilterModel *mItemTree;
    Akonadi::EntityMimeTypeFilterModel *mAllContactsModel;
    Akonadi::ContactsFilterProxyModel *mContactsFilterModel;

    QuickSearchWidget *mQuickSearchWidget;
    Akonadi::EntityTreeView *mCollectionView;
    Akonadi::EntityTreeView *mItemView;
    QWidget *mDetailsPane;
    QStackedWidget *mDetailsViewStack;
    ContactSwitcher *mContactSwitcher;

    QSplitter *mMainWidgetSplitter1;
    QSplitter *mMainWidgetSplitter2;
    Akonadi::ContactViewer *mContactDetails;
    Akonadi::ContactGroupViewer *mContactGroupDetails;
    QWidget *mEmptyDetails;
    Akonadi::StandardContactActionManager *mActionManager;
    QItemSelectionModel *mCollectionSelectionModel;

    QActionGroup *mViewModeGroup;

    XXPortManager *mXXPortManager;
    ModelColumnManager *mModelColumnManager;
    KXMLGUIClient *mXmlGuiClient;
};

#endif
