/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#ifndef AKONADI_STANDARDCONTACTACTIONMANAGER_H
#define AKONADI_STANDARDCONTACTACTIONMANAGER_H

#include "kaddressbook_export.h"

#include <akonadi/item.h>
#include <akonadi/standardactionmanager.h>

#include <QtCore/QObject>

class KAction;
class KActionCollection;
class QItemSelectionModel;
class QWidget;

namespace Akonadi {

/**
 * @short Manages contact specific actions for collection and item views.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class KADDRESSBOOK_EXPORT StandardContactActionManager : public QObject
{
  Q_OBJECT

  public:
    /**
     * Describes the supported actions.
     */
    enum Type {
      CreateContact = StandardActionManager::LastType + 1, ///< Creates a new contact
      CreateContactGroup,                                  ///< Creates a new contact group
      EditItem,                                            ///< Edits the selected contact resp. contact group
      CreateAddressBook,                                   ///< Creates a new address book
      DeleteAddressBook,                                   ///< Deletes the selected address book.
      LastType                                             ///< Marks last action
    };

    /**
     * Creates a new standard contact action manager.
     *
     * @param actionCollection The action collection to operate on.
     * @param parent The parent widget.
     */
    explicit StandardContactActionManager( KActionCollection *actionCollection, QWidget *parent = 0 );

    /**
     * Destroys the standard contact action manager.
     */
    ~StandardContactActionManager();

    /**
     * Sets the collection selection model based on which the collection
     * related actions should operate. If none is set, all collection actions
     * will be disabled.
     */
    void setCollectionSelectionModel( QItemSelectionModel *selectionModel );

    /**
     * Sets the item selection model based on which the item related actions
     * should operate. If none is set, all item actions will be disabled.
     */
    void setItemSelectionModel( QItemSelectionModel* selectionModel );

    /**
     * Creates the action of the given type and adds it to the action collection
     * specified in the constructor if it does not exist yet. The action is
     * connected to its default implementation provided by this class.
     */
    KAction* createAction( Type type );

    /**
     * Convenience method to create all standard actions.
     * @see createAction()
     */
    void createAllActions();

    /**
     * Returns the action of the given type, 0 if it has not been created (yet).
     */
    KAction* action( Type type ) const;

  Q_SIGNALS:
    /**
     * This signal is emitted whenever the action state has been updated.
     * In case you have special needs for changing the state of some actions,
     * connect to this signal and adjust the action state.
     */
    void actionStateUpdated();

    /**
     * This signal is emitted whenever the user wants to edit a contact or contact group item.
     *
     * @param item The item that shall be edited.
     */
    void editItem( const Akonadi::Item &item );

  private:
    //@cond PRIVATE
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void updateActions() )
    Q_PRIVATE_SLOT( d, void editTriggered() )
    Q_PRIVATE_SLOT( d, void addAddressBookTriggered() )
    Q_PRIVATE_SLOT( d, void addAddressBookResult( KJob* ) )
    Q_PRIVATE_SLOT( d, void deleteAddressBookTriggered() )
    //@endcond
};

}

#endif
