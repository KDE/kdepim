/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef ADDRESSEEWIDGET_H
#define ADDRESSEEWIDGET_H

#include <tqwidget.h>

class KComboBox;
class KLineEdit;

class TQListBox;
class TQListBoxItem;
class TQPushButton;

class NamePartWidget : public QWidget
{
  Q_OBJECT

  public:
    NamePartWidget( const TQString &title, const TQString &label, TQWidget *parent,
                    const char *name = 0 );
    ~NamePartWidget();

    void setNameParts( const TQStringList &list );
    TQStringList nameParts() const;

  signals:
    void modified();

  private slots:
    void add();
    void edit();
    void remove();

    void selectionChanged( TQListBoxItem* );

  private:
    TQListBox *mBox;
    TQPushButton *mAddButton;
    TQPushButton *mEditButton;
    TQPushButton *mRemoveButton;

    TQString mTitle;
    TQString mLabel;
};

class AddresseeWidget : public QWidget
{
  Q_OBJECT

  public:
    AddresseeWidget( TQWidget *parent, const char *name = 0 );
    ~AddresseeWidget();

    void restoreSettings();
    void saveSettings();

  signals:
    void modified();

  private:
    KComboBox *mFormattedNameCombo;
    NamePartWidget *mPrefix;
    NamePartWidget *mInclusion;
    NamePartWidget *mSuffix;
};

#endif
