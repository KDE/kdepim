#ifndef ADDRESSEEEDITORSUPPORTWIDGETS_H
#define ADDRESSEEEDITORSUPPORTWIDGETS_H
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
#include <qmap.h>
#include <qptrvector.h>

#include <kdialogbase.h>
#include <kabc/address.h>
#include <kabc/addressee.h>
#include <kabc/phonenumber.h>

class QButtonGroup;
class QToolButton;
class QListView;
class QTextEdit;
class QCheckBox;

class KLineEdit;
class KListView;
class KComboBox;

/** This widget displays a list box of the email addresses as well as buttons
* to manipulate them (up, down, add, remove)
*/
class EmailWidget : public QWidget
{
  Q_OBJECT
  
  public:
    EmailWidget(QWidget *parent, const char *name);
    ~EmailWidget();
    
    void setEmails(const QStringList &list);
    QStringList emails() const;
    
  signals:
    void modified();
    
  protected slots:
    void add();
    void remove();
    void edit();
    void standard();
    void selectionChanged(int idx);

  protected:
    virtual void keyPressEvent(QKeyEvent*);
  
  private:
    KLineEdit *mEmailEdit;
    QListBox *mEmailListBox;
    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
    QPushButton *mStandardButton;
};

///////////////////////////////////
// NameEditDialog
class NameEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    NameEditDialog(const QString &familyName, const QString &givenName,
                   const QString &prefix, const QString &suffix,
                   const QString &additionalName, 
                   QWidget *parent, const char *name = 0);
    ~NameEditDialog();
   
    QString familyName() const;
    QString givenName() const;
    QString prefix() const;
    QString suffix() const;
    QString additionalName() const;
   
  protected slots:
    void parseBoxChanged(bool);

  private:
    KComboBox *mSuffixCombo;
    KComboBox *mPrefixCombo;
    KLineEdit *mFamilyNameEdit;
    KLineEdit *mGivenNameEdit;
    KLineEdit *mAdditionalNameEdit;
    QCheckBox *mParseBox;
};

////////////////////////////////////
// AddressEditDialog
class AddressEditDialog : public KDialogBase
{
  public:
    AddressEditDialog(const KABC::Address &a, QWidget *parent, 
                      const char *name = 0);
    ~AddressEditDialog();
    
    const KABC::Address &address();
    
  private:
    void fillCombo(KComboBox *combo);
    
    KABC::Address mAddress;
    KABC::Address::TypeList mTypeList;
    
    QButtonGroup *mGroup;
    QTextEdit *mStreetTextEdit;
    KLineEdit *mRegionEdit;
    KLineEdit *mLocalityEdit;
    KLineEdit *mPostalCodeEdit;
    KLineEdit *mPOBoxEdit;
    KComboBox *mCountryCombo;
};

////////////////////////////////////
// AddressEditWidget

class AddressEditWidget : public QWidget
{
  Q_OBJECT
  
  public:
    AddressEditWidget( QWidget *parent, const char *name = 0 );
    ~AddressEditWidget();
    
    void setAddresses( const KABC::Address::List &list );
    const KABC::Address::List &addresses();
    
  signals:
    void modified();

  protected slots:
    void slotAddAddress();
    void slotRemoveAddress();
    void slotEditAddress();
    void slotSelectionChanged();
    void slotTypeChanged( int pos );

  private:
    KABC::Address::List mAddressList;
    KABC::Address::TypeList mTypeList;
    KComboBox *mTypeBox;
    KListView *mListView;

    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
};

////////////////////////////////
// PhoneEditDialog

class PhoneEditDialog : public KDialogBase
{
  Q_OBJECT
public:
  PhoneEditDialog( const KABC::PhoneNumber &phoneNumber, QWidget *parent, const char *name = 0 );

  KABC::PhoneNumber phoneNumber();

private:
  KABC::PhoneNumber mPhoneNumber;
  KABC::PhoneNumber::TypeList mTypeList;

  KLineEdit *mNumber;
  QButtonGroup *mGroup;
};

////////////////////////////////
// PhoneEditWidget

class PhoneEditWidget : public QWidget
{
  Q_OBJECT
  
  public:
    PhoneEditWidget( QWidget *parent, const char *name = 0 );
    ~PhoneEditWidget();
    
    void setPhoneNumbers( const KABC::PhoneNumber::List &list );
    const KABC::PhoneNumber::List &phoneNumbers();
    
  signals:
    void modified();

  protected slots:
    void slotAddPhoneNumber();
    void slotRemovePhoneNumber();
    void slotEditPhoneNumber();
    void slotSelectionChanged();
    void slotTypeChanged( int pos );

  private:
    KABC::PhoneNumber::List mPhoneNumberList;
    KABC::PhoneNumber::TypeList mTypeList;
    KComboBox *mTypeBox;
    KListView *mListView;

    QPushButton *mAddButton;
    QPushButton *mRemoveButton;
    QPushButton *mEditButton;
};

#endif
