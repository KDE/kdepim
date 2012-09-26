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

#ifndef CONTACTSWITCHER_H
#define CONTACTSWITCHER_H

#include <QWidget>

class QAbstractItemView;
class QLabel;
class QPushButton;

/**
 * @short A widget to switch between the contacts of an contact view.
 *
 * This widgets provides a 'Next' and 'Previous' button to allow the
 * user to switch between the entries of a contact view.
 *
 * @author Tobias Koenig <tokoe@kde.org>
 */
class ContactSwitcher : public QWidget
{
  Q_OBJECT

  public:
    /**
     * Creates a new contact switcher.
     *
     * @param parent The parent widget.
     */
    ContactSwitcher( QWidget *parent = 0 );

    /**
     * Sets the @p view the contact switcher shall work on.
     */
    void setView( QAbstractItemView *view );

  private Q_SLOTS:
    /**
     * This slot is called when the 'Next' button is clicked.
     */
    void nextClicked();

    /**
     * This slot is called when the 'Previous' button is clicked.
     */
    void previousClicked();

    /**
     * This slot is called whenever the layout of the model has changed
     * or the user has clicked a button.
     */
    void updateStatus();

  private:
    QAbstractItemView *mView;
    QPushButton *mNextButton;
    QPushButton *mPreviousButton;
    QLabel *mStatusLabel;
};

#endif
