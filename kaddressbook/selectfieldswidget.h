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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SELECTFIELDSWIDGET_H 
#define SELECTFIELDSWIDGET_H 

#include <qwidget.h>

#include <kabc/field.h>
#include <kabc/addressbook.h>

class KComboBox;
class QListBox;
class QListBoxItem;
class QPushButton;
class QToolButton;

class SelectFieldsWidget : public QWidget
{
  Q_OBJECT

  public:
    SelectFieldsWidget( KABC::AddressBook *doc,
                        const KABC::Field::List &selectedFields,
                        QWidget *parent = 0, const char *name = 0 );
                       
    SelectFieldsWidget( KABC::AddressBook *doc, QWidget *parent = 0,
                        const char *name = 0);
    
    virtual void setSelectedFields( const KABC::Field::List & );
    virtual KABC::Field::List selectedFields();

  public slots:
    void slotSelect();
    void slotUnSelect();
    void slotMoveUp();
    void slotMoveDown();

    void slotShowFields( int );
    void slotButtonsEnabled();
  
  private:
    void initGUI( KABC::AddressBook * );
    
    KComboBox *mCategoryCombo;
    QListBox *mSelectedBox;
    QListBox *mUnSelectedBox;
    QToolButton *mAddButton;
    QToolButton *mRemoveButton;
    QToolButton *mUpButton;
    QToolButton *mDownButton;
    
    KABC::AddressBook *mDoc;
};

#endif // SELECTFIELDSWIDGET_H 
