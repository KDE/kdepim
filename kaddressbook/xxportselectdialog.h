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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

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

class QButtonGroup;
class QComboBox;
class QListView;
class QListViewItem;
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
    void categoryClicked( QListViewItem * i );

  protected slots:
    void slotHelp();

  private:
    void initGUI();
    QStringList categories() const;

    QButtonGroup* mButtonGroup;
    QRadioButton* mUseCategories;
    QRadioButton* mUseFilters;
    QRadioButton* mUseWholeBook;
    QRadioButton* mUseSelection;
    QComboBox* mFiltersCombo;
    QListView* mCategoriesView;

    KComboBox *mFieldCombo;
    KComboBox *mSortTypeCombo;

    KAB::Core *mCore;
    KABC::AddresseeList mAddresseeList;
    Filter::List mFilters;
    KABC::Field::List mFields;
    bool mUseSorting;
};

#endif
