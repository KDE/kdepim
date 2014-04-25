/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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

#ifndef MAINVIEW_H
#define MAINVIEW_H

#include "kdeclarativemainview.h"

#include <AkonadiCore/Entity>

namespace KLDAP {
class LdapSearchDialog;
}

namespace Akonadi
{
  class Item;
  class StandardContactActionManager;
}

class ContactListProxy;
class KLineEdit;

class MainView : public KDeclarativeMainView
{
  Q_OBJECT
  public:
    explicit MainView( QWidget *parent = 0 );

  public Q_SLOTS:
    void newContact();
    void newContactGroup();

    void editItem();
    void editContact( const Akonadi::Item &item );
    void editContactGroup( const Akonadi::Item &item );

  private Q_SLOTS:
    void finishEdit( QObject *editor );
    void itemSelectionChanged( const QItemSelection &selected, const QItemSelection& );
    void bulkActionSelectionChanged();
    void sendMailTo();
    void searchLdap();
    void importFromLdap();
    void updateActionTexts();
    void configureCategories();

  protected:
    virtual void doDelayedInit();
    virtual void setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                             QItemSelectionModel *itemSelectionModel );

    virtual void setupAgentActionManager( QItemSelectionModel *selectionModel );

    virtual QAbstractProxyModel* createItemFilterModel() const;
    virtual ImportHandlerBase* importHandler() const;
    virtual ExportHandlerBase* exportHandler() const;
    virtual GuiStateManager* createGuiStateManager() const;

  private:
    Akonadi::StandardContactActionManager *mActionManager;
    ContactListProxy *mContactListProxy;
    QHash<QObject*, Akonadi::Entity::Id> mOpenItemEditors;
    KLDAP::LdapSearchDialog* mLdapSearchDialog;
};

#endif // MAINVIEW_H
