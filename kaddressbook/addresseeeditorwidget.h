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

#ifndef ADDRESSEEEDITORWIDGET_H
#define ADDRESSEEEDITORWIDGET_H

#include <qdatetime.h>

#include <kabc/addressee.h>
#include <kdialogbase.h>
#include <kjanuswidget.h>

#include "addresseeeditorbase.h"
#include "contacteditorwidgetmanager.h"
#include "extensionwidget.h"

class QCheckBox;
class QSpinBox;
class QTabWidget;
class QTextEdit;

class KComboBox;
class KDateEdit;
class KLineEdit;
class KSqueezedTextLabel;

class AddressEditWidget;
class EmailEditWidget;
class IMEditWidget;
class KeyWidget;
class PhoneEditWidget;
class SecrecyWidget;

namespace KAB {
class Core;
}

namespace KPIM {
class AddresseeLineEdit;
class CategorySelectDialog;
class CategoryEditDialog;
}

namespace KABC { class AddressBook; }

class AddresseeEditorWidget : public AddresseeEditorBase
{
  Q_OBJECT

  public:
    AddresseeEditorWidget( KAB::Core *core, bool isExtension,
                           QWidget *parent, const char *name = 0 );
    ~AddresseeEditorWidget();

    void setAddressee( const KABC::Addressee& );
    const KABC::Addressee &addressee();

    void load();
    void save();

    bool dirty();

    QString title() const;
    QString identifier() const;

    void setInitialFocus();

    bool readyToClose();

  protected slots:
    void textChanged( const QString& );
    void pageChanged( QWidget *wdg );

    /**
      Emits the modified signal and sets the dirty flag. Any slot
      that modifies data should use this method instead of calling emit
      modified() directly.
     */
    void emitModified();

    void dateChanged( const QDate& );
    void invalidDate();
    void nameTextChanged( const QString& );
    void organizationTextChanged( const QString& );
    void nameBoxChanged();
    void nameButtonClicked();
    void selectCategories();

    /**
      Called whenever the categories change in the categories dialog.
     */
    void categoriesSelected( const QStringList& );

    /**
      Edits which categories are available in the CategorySelectDialog.
     */
    void editCategories();

  private:
    void initGUI();
    void setupTab1();
    void setupTab2();
    void setupAdditionalTabs();
    void setupCustomFieldsTabs();

    void setReadOnly( bool );

    KABC::Addressee mAddressee;
    int mFormattedNameType;
    bool mDirty;
    bool mBlockSignals;
    bool mReadOnly;

    // GUI
    KPIM::CategorySelectDialog *mCategorySelectDialog;
    KPIM::CategoryEditDialog *mCategoryEditDialog;
    QTabWidget *mTabWidget;

    // Tab1
    KLineEdit *mNameEdit;
    KLineEdit *mRoleEdit;
    KLineEdit *mOrgEdit;
    KSqueezedTextLabel *mFormattedNameLabel;
    AddressEditWidget *mAddressEditWidget;
    EmailEditWidget *mEmailWidget;
    IMEditWidget *mIMWidget;
    PhoneEditWidget *mPhoneEditWidget;
    KLineEdit *mURLEdit;
    KLineEdit *mBlogEdit;
//    KLineEdit *mIMAddressEdit;
    QPushButton *mCategoryButton;
    KLineEdit *mCategoryEdit;
    SecrecyWidget *mSecrecyWidget;
    KSqueezedTextLabel *mNameLabel;

    // Tab2
    KLineEdit *mDepartmentEdit;
    KLineEdit *mOfficeEdit;
    KLineEdit *mProfessionEdit;
    KLineEdit *mTitleEdit;
    KPIM::AddresseeLineEdit *mManagerEdit;
    KPIM::AddresseeLineEdit *mAssistantEdit;
    KLineEdit *mNicknameEdit;
    KPIM::AddresseeLineEdit *mSpouseEdit;
    KDateEdit *mBirthdayPicker;
    KDateEdit *mAnniversaryPicker;
    QTextEdit *mNoteEdit;

    QDict<ContactEditorTabPage> mTabPages;
};

#endif
