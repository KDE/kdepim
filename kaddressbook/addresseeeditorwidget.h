#ifndef ADDRESSEEEDITORWIDGET_H
#define ADDRESSEEEDITORWIDGET_H

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
#include <qdatetime.h>

#include <kdialogbase.h>
#include <kabc/addressee.h>

class QCheckBox;
class QTabWidget;
class QTextEdit;
class QSpinBox;

class KLineEdit;
class KComboBox;
class KDateEdit;

class EmailWidget;
class AddressEditWidget;
class PhoneEditWidget;

namespace KPIM
{
  class CategorySelectDialog;
  class CategoryEditDialog;
}

namespace KABC { class AddressBook; }

class AddresseeEditorWidget : public QWidget
{
  Q_OBJECT
  
  public:
    AddresseeEditorWidget(QWidget *parent, const char *name);
    ~AddresseeEditorWidget();  

    void setAddressee(const KABC::Addressee &a);
    const KABC::Addressee &addressee();
  
    void load();
    void save();
    
    bool dirty();
    
  signals:
    void modified();
  
  protected slots:
    void textChanged(const QString &);
    
    /** Emits the modified signal and sets the dirty flag. Any slot
    * that modifies data should use this method instead of calling emit
    * modified() directly.
    */
    void emitModified();
    void dateChanged(QDate);
    void nameTextChanged(const QString &);
    void nameBoxChanged();
    void nameButtonClicked();
    void categoryButtonClicked();
    
    /** Called whenever the categories change in the categories dialog.
    */
    void categoriesSelected(const QStringList &);
    
    /** Edits which categories are available in the CategorySelectDialog.
    */
    void editCategories();
    
  private:
    void initGUI();
    void setupTab1();
    void setupTab2();
    void setupTab3();
    
    KABC::Addressee mAddressee;
    bool mDirty;
    
    // GUI
    QTabWidget *mTabWidget;
    KPIM::CategorySelectDialog *mCategoryDialog;
    KPIM::CategoryEditDialog *mCategoryEditDialog;
    
    // Tab1
    KLineEdit *mNameEdit;
    QCheckBox *mParseBox;
    KLineEdit *mRoleEdit;
    KLineEdit *mOrgEdit;
    KComboBox *mFormattedNameBox;
    EmailWidget *mEmailWidget;
    AddressEditWidget *mAddressEditWidget;
    PhoneEditWidget *mPhoneEditWidget;
    KLineEdit *mURLEdit;
    KLineEdit *mIMAddressEdit;
    KLineEdit *mCategoryEdit;
    
    // Tab2
    KLineEdit *mDepartmentEdit;
    KLineEdit *mOfficeEdit;
    KLineEdit *mProfessionEdit;
    KLineEdit *mManagerEdit;
    KLineEdit *mAssistantEdit;
    KLineEdit *mNicknameEdit;
    KLineEdit *mSpouseEdit;
    KDateEdit *mBirthdayPicker;
    KDateEdit *mAnniversaryPicker;
    QTextEdit *mNoteEdit;
    QSpinBox *mTimeZoneSpin;
    QSpinBox *mGeoLat;
    QSpinBox *mGeoLon;
};

#endif
