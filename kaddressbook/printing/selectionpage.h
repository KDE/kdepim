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

#include <qwidget.h>

class Q3ButtonGroup;
class QComboBox;
class Q3ListView;
class Q3ListViewItem;
class QRadioButton;

class SelectionPage : public QWidget
{
  Q_OBJECT

  public:
    SelectionPage( QWidget* parent = 0, const char* name = 0 );
    ~SelectionPage();

    void setFilters( const QStringList& );
    QString filter() const;
    bool useFilters() const;

    void setCategories( const QStringList& );
    QStringList categories() const;
    bool useCategories();

    void setUseSelection( bool value );
    bool useSelection() const;

  private slots:
    void filterChanged( int );
    void categoryClicked( Q3ListViewItem * i );

  private:
    Q3ButtonGroup* mButtonGroup;
    QRadioButton* mUseCategories;
    QRadioButton* mUseFilters;
    QRadioButton* mUseWholeBook;
    QRadioButton* mUseSelection;
    QComboBox* mFiltersCombo;
    Q3ListView* mCategoriesView;
};

#endif // SELECTIONPAGE_H
