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

#include <kactioncollection.h>
#include <QtGui/QWidget>
#include "kcontactmanager_export.h"

namespace Akonadi {
class Collection;
class CollectionFilterProxyModel;
class CollectionModel;
class CollectionView;
class ContactGroupBrowser;
class Item;
class ItemView;
class KABCItemBrowser;
class KABCModel;
class StandardActionManager;
}

class KXmlGuiWindow;
class QStackedWidget;
class QuickSearchWidget;
class XXPortManager;

class KCONTACTMANAGER_EXPORT MainWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit MainWidget(KActionCollection *action, KXmlGuiWindow *guiWindow, QWidget *parent = 0 );
    ~MainWidget();

  public Q_SLOTS:
    void newContact();
    void newGroup();

 private Q_SLOTS:
    void editItem( const Akonadi::Item &item );
    void itemSelected( const Akonadi::Item &item );

    void collectionSelected( const Akonadi::Collection &collection );

  private:
    void setupGui();
    void setupActions(KActionCollection *);

    void editContact( const Akonadi::Item &contact );
    void editGroup( const Akonadi::Item &group );

    Akonadi::CollectionModel *mCollectionModel;
    Akonadi::CollectionFilterProxyModel *mCollectionFilterModel;
    Akonadi::KABCModel *mContactModel;

    QuickSearchWidget *mQuickSearchWidget;
    Akonadi::CollectionView *mCollectionView;
    Akonadi::ItemView *mItemView;
    QStackedWidget *mDetailsViewStack;

    Akonadi::KABCItemBrowser *mContactDetails;
    Akonadi::ContactGroupBrowser *mContactGroupDetails;
    Akonadi::StandardActionManager *mActionManager;

    XXPortManager *mXXPortManager;
};

#endif
