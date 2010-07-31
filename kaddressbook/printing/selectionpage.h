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
#ifndef SELECTIONPAGE_H
#define SELECTIONPAGE_H

#include <tqwidget.h>

class QButtonGroup;
class QComboBox;
class QListView;
class QListViewItem;
class QRadioButton;

class SelectionPage : public QWidget
{
  Q_OBJECT

  public:
    SelectionPage( TQWidget* parent = 0, const char* name = 0 );
    ~SelectionPage();

    void setFilters( const TQStringList& );
    TQString filter() const;
    bool useFilters() const;

    void setCategories( const TQStringList& );
    TQStringList categories() const;
    bool useCategories();

    void setUseSelection( bool value );
    bool useSelection() const;

  private slots:
    void filterChanged( int );
    void categoryClicked( TQListViewItem * i );

  private:
    TQButtonGroup* mButtonGroup;
    TQRadioButton* mUseCategories;
    TQRadioButton* mUseFilters;
    TQRadioButton* mUseWholeBook;
    TQRadioButton* mUseSelection;
    TQComboBox* mFiltersCombo;
    TQListView* mCategoriesView;
};

#endif // SELECTIONPAGE_H
