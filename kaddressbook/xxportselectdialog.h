/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>
                       Tobias Koenig <tokoe@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef XXPORTSELECTDIALOG_H
#define XXPORTSELECTDIALOG_H

#include <kabc/addresseelist.h>
#include <kabc/field.h>
#include <kdialogbase.h>

#include "filter.h"

class Q3ButtonGroup;
class QComboBox;
class Q3ListView;
class Q3ListViewItem;
class QRadioButton;

namespace KAB {
class Core;
}

class KComboBox;

class XXPortSelectDialog : public KDialogBase
{
  Q_OBJECT

  public:
    XXPortSelectDialog( KAB::Core *core, bool sort, QWidget* parent,
                        const char* name = 0 );

    KABC::AddresseeList contacts();

  private slots:
    void filterChanged( int );
    void categoryClicked( Q3ListViewItem * i );

  protected slots:
    void slotHelp();

  private:
    void initGUI();
    QStringList categories() const;

    Q3ButtonGroup* mButtonGroup;
    QRadioButton* mUseCategories;
    QRadioButton* mUseFilters;
    QRadioButton* mUseWholeBook;
    QRadioButton* mUseSelection;
    QComboBox* mFiltersCombo;
    Q3ListView* mCategoriesView;

    KComboBox *mFieldCombo;
    KComboBox *mSortTypeCombo;

    KAB::Core *mCore;
    KABC::AddresseeList mAddresseeList;
    Filter::List mFilters;
    KABC::Field::List mFields;
    bool mUseSorting;
};

#endif
