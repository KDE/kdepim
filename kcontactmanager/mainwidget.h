/*
    This file is part of KContactManager.

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

#include "kcontactmanager_export.h"

#include <QtGui/QWidget>

namespace Akonadi {
class Collection;
class CollectionFilterProxyModel;
class DescendantEntitiesProxyModel;
class EntityFilterProxyModel;
class EntityTreeView;
class ContactGroupBrowser;
class Item;
class ItemView;
class KABCItemBrowser;
class StandardActionManager;
}

class KActionCollection;
class KXMLGUIClient;
class QItemSelection;
class QStackedWidget;
class QuickSearchWidget;
class XXPortManager;

class KCONTACTMANAGER_EXPORT MainWidget : public QWidget
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

  private:
    void setupGui();
    void setupActions( KActionCollection* );

    void editContact( const Akonadi::Item& );
    void editGroup( const Akonadi::Item& );

    Akonadi::EntityFilterProxyModel *mCollectionTree;
    Akonadi::EntityFilterProxyModel *mItemTree;
    Akonadi::DescendantEntitiesProxyModel *mDescendantTree;

    QuickSearchWidget *mQuickSearchWidget;
    Akonadi::EntityTreeView *mCollectionView;
    Akonadi::EntityTreeView *mItemView;
    QStackedWidget *mDetailsViewStack;

    Akonadi::KABCItemBrowser *mContactDetails;
    Akonadi::ContactGroupBrowser *mContactGroupDetails;
    Akonadi::StandardActionManager *mActionManager;

    XXPortManager *mXXPortManager;
};

#endif
