#ifndef PHONEEDITWIDGET_H
#define PHONEEDITWIDGET_H
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

#include <kdialogbase.h>

#include "addresseeconfig.h"

class QButtonGroup;
class QCheckBox;

class KLineEdit;
class KComboBox;

/**
  Widget for editing phone numbers.
*/
class PhoneEditWidget : public QWidget
{
  Q_OBJECT

  public:
    PhoneEditWidget( QWidget *parent, const char *name = 0 );
    ~PhoneEditWidget();
    
    void setPhoneNumbers( const KABC::PhoneNumber::List &list );
    const KABC::PhoneNumber::List &phoneNumbers();

    void updateTypeCombo( const KABC::PhoneNumber::List&, KComboBox* );
    KABC::PhoneNumber currentPhoneNumber( KComboBox*, int );

    void updateAllTypeCombos();

  signals:
    void modified();

  private slots:
    void numberChanged( const QString& );
    void comboChanged( int );
    void comboChanged( KComboBox*, int );
    void edit();

  private:
    KComboBox *mPrefCombo;
    KComboBox *mSecondCombo;
    KComboBox *mThirdCombo;
    KComboBox *mFourthCombo;
    KLineEdit *mPrefEdit;
    KLineEdit *mSecondEdit;
    KLineEdit *mThirdEdit;
    KLineEdit *mFourthEdit;

    QMap<KLineEdit*, KComboBox*> mEditMap;
    KABC::PhoneNumber::List mPhoneList;
};
  
/**
  Dialog for editing lists of phonenumbers.
*/
class PhoneEditDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    PhoneEditDialog( const KABC::PhoneNumber::List &list, QWidget *parent, const char *name = 0 );
    ~PhoneEditDialog();
    
    const KABC::PhoneNumber::List &phoneNumbers();
    
  protected slots:
    void slotAddPhoneNumber();
    void slotRemovePhoneNumber();
    void slotEditPhoneNumber();
    void slotSelectionChanged();

  private:
    KABC::PhoneNumber::List mPhoneNumberList;
    KABC::PhoneNumber::TypeList mTypeList;
    KComboBox *mTypeBox;
    KListView *mListView;

    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
};

/**
  Dialog for editing phone number types.
*/
class PhoneTypeDialog : public KDialogBase
{
  Q_OBJECT
public:
  PhoneTypeDialog( const KABC::PhoneNumber &phoneNumber, QWidget *parent, const char *name = 0 );

  KABC::PhoneNumber phoneNumber();

private:
  KABC::PhoneNumber mPhoneNumber;
  KABC::PhoneNumber::TypeList mTypeList;

  QButtonGroup *mGroup;
  QCheckBox *mPreferredBox;
  KLineEdit *mNumber;
};

#endif
