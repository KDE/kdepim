/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#ifndef ADDRESSEDITWIDGET_H
#define ADDRESSEDITWIDGET_H

#include <qwidget.h>

#include <kdialogbase.h>
#include <kabc/address.h>
#include <kabc/addressee.h>

#include "addresseeconfig.h"

class QButtonGroup;
class QCheckBox;
class QListView;
class QTextEdit;
class QToolButton;

class KComboBox;
class KLineEdit;
class KListView;

/**
  Editor widget for addresses.
 */
class AddressEditWidget : public QWidget
{
  Q_OBJECT

  public:
    AddressEditWidget( QWidget *parent, const char *name = 0 );
    ~AddressEditWidget();

    KABC::Address::List addresses() const;
    void setAddresses( const KABC::Addressee &addr, const KABC::Address::List &list );

  signals:
    void modified();

  private slots:
    void updateTypeCombo();
    void updateView();

    void add();
    void edit();
    void remove();
    void setPreferred();

  private:
    KComboBox *mTypeCombo;

    QPushButton *mAddButton;
    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
    QPushButton *mPreferredButton;
    QTextEdit *mAddressView;

    KABC::Address::List mAddressList;
    KABC::Addressee mAddressee;
    QMap<int, int> mAddressMap;
};

/**
  Dialog for editing address details.
 */
class AddressEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddressEditDialog( const KABC::Address &addr, QWidget *parent,
                       const char *name = 0 );
    ~AddressEditDialog();

    KABC::Address address();

  private slots:
    void changeType();

  private:
    void fillCountryCombo();

    QTextEdit *mStreetTextEdit;
    KComboBox *mCountryCombo;
    KLineEdit *mRegionEdit;
    KLineEdit *mLocalityEdit;
    KLineEdit *mPostalCodeEdit;
    KLineEdit *mPOBoxEdit;
    QPushButton *mTypeButton;

    KABC::Address mAddress;
};

/**
  Dialog for selecting an address type.
 */
class AddressTypeDialog : public KDialogBase
{
  public:
    AddressTypeDialog( int type, QWidget *parent );
    ~AddressTypeDialog();

    int type() const;

  private:
    QButtonGroup *mGroup;

    KABC::Address::TypeList mTypeList;
};

#endif
