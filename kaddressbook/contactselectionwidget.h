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

#ifndef CONTACTSELECTIONWIDGET_H
#define CONTACTSELECTIONWIDGET_H

#include "contactlist.h"

#include <KABC/Addressee>
#include <Akonadi/Item>
#include <QWidget>

class QCheckBox;
class QItemSelectionModel;
class QLabel;
class QRadioButton;

namespace Akonadi {
class Collection;
class CollectionComboBox;
}

/**
 * @short A widget to select a group of contacts.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class ContactSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * Creates a new contact selection widget.
     *
     * @param selectionModel The model that contains the currently selected contacts.
     * @param parent The parent widget.
     */
    explicit ContactSelectionWidget( QItemSelectionModel *selectionModel, QWidget *parent = 0 );

    /**
     * Sets the @p message text.
     */
    void setMessageText( const QString &message );

    /**
     * Sets the default addressbook.
     */
    void setDefaultAddressBook( const Akonadi::Collection &addressBook );

    /**
     * Returns the list of selected contacts.
     */
    ContactList selectedContacts() const;

    void setAddGroupContact(bool addGroupContact);
private:
    void initGui();

    ContactList collectAllContacts() const;
    ContactList collectSelectedContacts() const;
    ContactList collectAddressBookContacts() const;

    QItemSelectionModel *mSelectionModel;

    QLabel *mMessageLabel;
    QRadioButton *mAllContactsButton;
    QRadioButton *mSelectedContactsButton;
    QRadioButton *mAddressBookContactsButton;
    Akonadi::CollectionComboBox *mAddressBookSelection;
    QCheckBox *mAddressBookSelectionRecursive;
    bool mAddContactGroup;
};

#endif
