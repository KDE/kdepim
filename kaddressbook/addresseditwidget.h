#ifndef ADDRESSEDITWIDGET_H
#define ADDRESSEDITWIDGET_H
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qwidget.h>

#include <kdialogbase.h>
#include <kabc/address.h>
#include <kabc/addressee.h>

#include "addresseeconfig.h"
#include "typecombo.h"

class QButtonGroup;
class QToolButton;
class QListView;
class QTextEdit;
class QCheckBox;

class KLineEdit;
class KListView;
class KComboBox;

typedef TypeCombo<KABC::Address> AddressTypeCombo;

/**
  Editor widget for addresses.
*/
class AddressEditWidget : public QWidget
{
  Q_OBJECT
  
  public:
    AddressEditWidget(QWidget *parent, const char *name = 0);
    ~AddressEditWidget();
    
    const KABC::Address::List &addresses();
    void setAddresses(const KABC::Address::List &list);
    
    void updateTypeCombo( const KABC::Address::List&, KComboBox* );
    KABC::Address currentAddress( KComboBox*, int );

  signals:
    void modified();
    
  protected slots:
    void updateAddressEdit();

    void edit();

  private:
    AddressTypeCombo *mTypeCombo;

    QPushButton *mEditButton;
    QPushButton *mRemoveButton;
    QTextEdit *mAddressTextEdit;

    KABC::Address::List mAddressList;
    int mIndex;
};

/**
  Dialog for editing address details.
*/
class AddressEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    AddressEditDialog( const KABC::Address::List &list, int selected,
                       QWidget *parent, const char *name = 0 );
    ~AddressEditDialog();
    
    KABC::Address::List addresses();

  protected slots:
    void addAddress();
    void removeAddress();

    void updateAddressEdits();

  private:
    void fillCountryCombo(KComboBox *combo);

    void saveAddress();

    KABC::Address::List mAddressList;
    
    AddressTypeCombo *mTypeCombo;
    
    QCheckBox *mPreferredCheckBox;
    QTextEdit *mStreetTextEdit;
    KComboBox *mCountryCombo;
    KLineEdit *mRegionEdit;
    KLineEdit *mLocalityEdit;
    KLineEdit *mPostalCodeEdit;
    KLineEdit *mPOBoxEdit;
};

/**
  Dialog for selecting an address type.
*/
class AddressTypeDialog : public KDialogBase
{
  public:
    AddressTypeDialog( int type, QWidget *parent );
    ~AddressTypeDialog();
    
    int type();

  private:
    QButtonGroup *mGroup;
    
    KABC::Address::TypeList mTypeList;
};

#endif
