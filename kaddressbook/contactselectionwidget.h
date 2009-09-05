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

#include <QtGui/QWidget>

#include <kabc/addressee.h>

class QAbstractItemModel;
class QItemSelectionModel;
class QLabel;
class QRadioButton;

namespace Akonadi
{
class AddressBookComboBox;
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
     * @param model The model that contains all available contacts.
     * @param selectionModel The model that contains the currently selected contacts.
     * @param parent The parent widget.
     */
    ContactSelectionWidget( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent = 0 );

    /**
     * Sets the @p message text.
     */
    void setMessageText( const QString &message );

    /**
     * Requests the list of selected contacts.
     * The list is made available via the selectedContacts() signal.
     */
    void requestSelectedContacts();

  Q_SIGNALS:
    /**
     * This signal is emitted when all contacts, the user has choosen,
     * are available.
     */
    void selectedContacts( const KABC::Addressee::List &contacts );

  private:
    void initGui();

    void collectAllContacts();
    void collectSelectedContacts();
    void collectAddressBookContacts();

    QAbstractItemModel *mModel;
    QItemSelectionModel *mSelectionModel;

    QLabel *mMessageLabel;
    QRadioButton *mAllContactsButton;
    QRadioButton *mSelectedContactsButton;
    QRadioButton *mAddressBookContactsButton;
    Akonadi::AddressBookComboBox *mAddressBookSelection;
};

#endif
