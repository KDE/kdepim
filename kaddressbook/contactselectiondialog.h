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

#ifndef CONTACTSELECTIONDIALOG_H
#define CONTACTSELECTIONDIALOG_H

#include <kabc/addressee.h>
#include <kdialog.h>

class ContactSelectionWidget;

class QAbstractItemModel;
class QItemSelectionModel;

/**
 * @short A dialog to select a group of contacts.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class ContactSelectionDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * Creates a new contact selection dialog.
     *
     * @param model The model that contains all available contacts.
     * @param selectionModel The model that contains the currently selected contacts.
     * @param parent The parent widget.
     */
    ContactSelectionDialog( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent = 0 );

    /**
     * Sets the @p message text.
     */
    void setMessageText( const QString &message );

    /**
     * Returns the list of selected contacts.
     */
    KABC::Addressee::List selectedContacts() const;

  protected Q_SLOTS:
    void slotButtonClicked( int button );

  private Q_SLOTS:
    void slotSelectedContacts( const KABC::Addressee::List& );

  private:
    ContactSelectionWidget *mSelectionWidget;
    KABC::Addressee::List mContacts;
};

#endif
