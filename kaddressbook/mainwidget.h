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

#include <QtGui/QWidget>

namespace Akonadi {
class Collection;
class ContactGroupViewer;
class ContactViewer;
class EntityFilterProxyModel;
class EntityTreeView;
class Item;
class ItemView;
class StandardContactActionManager;
}

class ContactSwitcher;
class KActionCollection;
class KDescendantsProxyModel;
class KXMLGUIClient;
class ModelColumnManager;
class QAbstractItemModel;
class QModelIndex;
class QStackedWidget;
class QTextBrowser;
class QuickSearchWidget;
class XXPortManager;

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

  private Q_SLOTS:
    void editItem( const Akonadi::Item &item );
    void itemSelected( const Akonadi::Item &item );
    void itemSelectionChanged( const QModelIndex&, const QModelIndex& );
    void selectFirstItem();

    void setDetailsViewVisible( bool visible );
    void setItemViewVisible( bool visible );
    void setCollectionViewVisible( bool visible );

    void setSimpleGuiMode( bool on );

  private:
    void setupGui();
    void setupActions( KActionCollection* );

    void editContact( const Akonadi::Item& );
    void editGroup( const Akonadi::Item& );

    QAbstractItemModel* allContactsModel();

    /**
     * Returns the address book collection that is currently
     * selected by the user or an invalid collection if no
     * address book is selected.
     */
    Akonadi::Collection currentAddressBook() const;

    Akonadi::EntityFilterProxyModel *mCollectionTree;
    Akonadi::EntityFilterProxyModel *mItemTree;
    Akonadi::EntityFilterProxyModel *mAllContactsModel;
    KDescendantsProxyModel *mDescendantTree;

    QuickSearchWidget *mQuickSearchWidget;
    Akonadi::EntityTreeView *mCollectionView;
    Akonadi::EntityTreeView *mItemView;
    QWidget *mDetailsPane;
    QStackedWidget *mDetailsViewStack;
    ContactSwitcher *mContactSwitcher;

    Akonadi::ContactViewer *mContactDetails;
    Akonadi::ContactGroupViewer *mContactGroupDetails;
    QTextBrowser *mEmptyDetails;
    Akonadi::StandardContactActionManager *mActionManager;

    XXPortManager *mXXPortManager;
    ModelColumnManager *mModelColumnManager;
};

#endif
