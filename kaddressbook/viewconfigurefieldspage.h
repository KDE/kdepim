/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef VIEWCONFIGUREFIELDSPAGE_H
#define VIEWCONFIGUREFIELDSPAGE_H

#include <qwidget.h>

#include <kabc/field.h>
#include <kabc/addressbook.h>

class KComboBox;
class QListBox;
class QListBoxItem;
class QPushButton;
class QToolButton;

class ViewConfigureFieldsPage : public QWidget
{
  Q_OBJECT

  public:
    ViewConfigureFieldsPage( KABC::AddressBook *ab, QWidget *parent = 0,
                             const char *name = 0 );

    void restoreSettings( KConfig* );
    void saveSettings( KConfig* );

  public slots:
    void slotSelect();
    void slotUnSelect();
    void slotMoveUp();
    void slotMoveDown();

    void slotShowFields( int );
    void slotButtonsEnabled();

  private:
    void initGUI();

    KComboBox *mCategoryCombo;
    QListBox *mSelectedBox;
    QListBox *mUnSelectedBox;
    QToolButton *mAddButton;
    QToolButton *mRemoveButton;
    QToolButton *mUpButton;
    QToolButton *mDownButton;

    KABC::AddressBook *mAddressBook;
};

#endif
