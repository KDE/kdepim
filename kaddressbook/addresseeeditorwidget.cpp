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

#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qtextedit.h>
#include <qtooltip.h>
#include <qlistbox.h>
#include <qhbox.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kconfig.h>
#include <klocale.h>
#include <kdialogbase.h>
#include <kseparator.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kdebug.h>

#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/categoryeditdialog.h>

#include <libkdepim/kdateedit.h>

#include "addresseeeditorsupportwidgets.h"
#include "kabprefs.h"

#include "addresseeeditorwidget.h"

AddresseeEditorWidget::AddresseeEditorWidget(QWidget *parent, 
                                            const char *name)
  : QWidget(parent, name)
{
  initGUI();
  mCategoryDialog = 0;
  mCategoryEditDialog = 0;
  
  // Load the empty addressee as defaults
  load();
  
  mDirty = false;
}

AddresseeEditorWidget::~AddresseeEditorWidget()
{
  kdDebug() << "~AddresseeEditorWidget()" << endl;
}  
  
void AddresseeEditorWidget::setAddressee(const KABC::Addressee &a)
{
  mAddressee = a;
  load();
}

const KABC::Addressee &AddresseeEditorWidget::addressee()
{
  return mAddressee;
}

void AddresseeEditorWidget::textChanged(const QString &)
{
  emitModified();
}
  
void AddresseeEditorWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  
  mTabWidget = new QTabWidget(this, "mTabWidget");
  layout->addWidget(mTabWidget);
  
  setupTab1();
  setupTab2();
  setupTab3();
}

void AddresseeEditorWidget::setupTab1()
{
  // This is the General tab
  QWidget *tab1 = new QWidget(mTabWidget, "tab1");
  QGridLayout *layout = new QGridLayout(tab1, 11, 7);
  layout->setMargin(KDialogBase::marginHint());
  layout->setSpacing(KDialogBase::spacingHint());
  
  QLabel *label;
  KSeparator* bar;
  QPushButton *button;
  
  //////////////////////////////////
  // Upper left group (person info)
  
  // Person icon
  label = new QLabel(tab1);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("personal", KIcon::Desktop));
  layout->addMultiCellWidget(label, 0, 1, 0, 0);
  
  // First name
  button = new QPushButton( i18n("Name..."), tab1 );
  QToolTip::add(button, i18n("Edit the contact's name"));
  QHBoxLayout *nameLayout = new QHBoxLayout;
  mNameEdit = new KLineEdit( tab1, "mNameEdit" );
  connect( mNameEdit, SIGNAL( textChanged(const QString & )), 
           SLOT( nameTextChanged(const QString & )));
  connect( button, SIGNAL( clicked()), this, SLOT( nameButtonClicked()));
  mParseBox = new QCheckBox( i18n( "Parse name automatically" ), tab1 );
  nameLayout->addWidget( mNameEdit );
  nameLayout->addWidget( mParseBox );
  nameLayout->setStretchFactor( mNameEdit, 1 );
  layout->addWidget( button, 0, 1 );
  layout->addLayout( nameLayout, 0, 2 );
  
  label = new QLabel( i18n("Role:"), tab1 );
  mRoleEdit = new KLineEdit( tab1, "mRoleEdit" );
  connect(mRoleEdit, SIGNAL( textChanged(const QString &) ),
          SLOT( textChanged(const QString &) ));
  label->setBuddy( mRoleEdit );
  layout->addWidget( label, 1, 1 );
  layout->addWidget( mRoleEdit, 1, 2 );
  
  // Organization
  label = new QLabel( i18n("Organization:"), tab1 );
  mOrgEdit = new KLineEdit( tab1, "mOrgEdit" );
  label->setBuddy( mOrgEdit );
  connect( mOrgEdit, SIGNAL( textChanged(const QString &) ), 
           SLOT( textChanged(const QString &) ));
  layout->addWidget( label, 2, 1 );
  layout->addWidget( mOrgEdit, 2, 2 );
  
  // File as (formatted name)
  label = new QLabel( i18n("Formatted name:"), tab1 );
  mFormattedNameBox = new KComboBox(tab1, "mFormattedNameBox");
  mFormattedNameBox->setEditable(true);
  mFormattedNameBox->setDuplicatesEnabled(false);
  mFormattedNameBox->setAutoCompletion(true);
  label->setBuddy( mFormattedNameBox );
  connect(mFormattedNameBox, SIGNAL(activated(const QString &)),
          SLOT(textChanged(const QString &)));
  connect(mFormattedNameBox, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget( label, 3, 1 );
  layout->addWidget( mFormattedNameBox, 3, 2 );
  
  // Left hand separator. This separator doesn't go all the way
  // across so the dialog still flows from top to bottom
  bar = new KSeparator( KSeparator::HLine, tab1);
  layout->addMultiCellWidget( bar, 4, 4, 0, 2 );
  
  //////////////////////////////////////
  // Phone numbers (upper right)
  label = new QLabel(tab1);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("kaddressbook",
                                                   KIcon::Desktop));
  layout->addMultiCellWidget(label, 0, 1, 3, 3);
  
  mPhoneEditWidget = new PhoneEditWidget(tab1, "mPhoneEditWidget");
  connect(mPhoneEditWidget, SIGNAL(modified()), this,
          SLOT(emitModified()));
  layout->addMultiCellWidget(mPhoneEditWidget, 0, 3, 4, 6); 
  
  bar = new KSeparator( KSeparator::HLine, tab1);
  layout->addMultiCellWidget( bar, 4, 4, 3, 6 );
  
  //////////////////////////////////////
  // Addresses (lower left)
  label = new QLabel(tab1);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("gohome",
                                                   KIcon::Desktop));
  layout->addMultiCellWidget(label, 5, 6, 0, 0);
  
  mAddressEditWidget = new AddressEditWidget(tab1, "mAddressEditWidget");
  connect(mAddressEditWidget, SIGNAL(modified()), this, SLOT(emitModified()));
  layout->addMultiCellWidget(mAddressEditWidget, 5, 9, 1, 2);

  //////////////////////////////////////
  // Email / Web (lower right)
  label = new QLabel(tab1);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("email",
                                                   KIcon::Desktop));
  layout->addMultiCellWidget(label, 5, 6, 3, 3);

  mEmailWidget = new EmailWidget(tab1, "mEmailWidget");
  connect(mEmailWidget, SIGNAL(modified()), this, SLOT(emitModified()));
  layout->addMultiCellWidget( mEmailWidget, 5, 6, 4, 6 );

  // add the separator
  bar = new KSeparator( KSeparator::HLine, tab1);
  layout->addMultiCellWidget( bar, 7, 7, 3, 6 );

  label = new QLabel(tab1);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("homepage",
                                                   KIcon::Desktop));
  layout->addMultiCellWidget(label, 8, 9, 3, 3);

  label = new QLabel( i18n("URL:"), tab1 );
  mURLEdit = new KLineEdit( tab1, "mURLEdit" );
  label->setBuddy( mURLEdit );
  layout->addWidget( label, 8, 4 );
  layout->addMultiCellWidget( mURLEdit, 8, 8, 5, 6 );

  label = new QLabel( i18n( "&IM address:"), tab1 );
  mIMAddressEdit = new KLineEdit(tab1, "mIMAddressEdit");
  label->setBuddy( mIMAddressEdit );
  layout->addWidget( label, 9, 4 );
  layout->addMultiCellWidget( mIMAddressEdit, 9, 9, 5, 6 );
  
  layout->addColSpacing(6, 50); 
  
  bar = new KSeparator(KSeparator::HLine, tab1);
  layout->addMultiCellWidget(bar, 10, 10, 0, 6);
  
  ///////////////////////////////////////
  QHBox *categoryBox = new QHBox( tab1 );
  categoryBox->setSpacing( KDialogBase::spacingHint() );
  
  // Categories
  button = new QPushButton(i18n("Categories..."), categoryBox);
  connect(button, SIGNAL(clicked()), SLOT(categoryButtonClicked()));

  mCategoryEdit = new KLineEdit(categoryBox, "mCategoryEdit");
  mCategoryEdit->setReadOnly(true);
  connect(mCategoryEdit, SIGNAL(textChanged(const QString &)), 
          SLOT(textChanged(const QString &)));

  layout->addMultiCellWidget(categoryBox, 11, 11, 0, 6);
  
  // Build the layout and add to the tab widget
  layout->activate(); // required
  mTabWidget->addTab( tab1, i18n( "&General" ));
}

void AddresseeEditorWidget::setupTab2()
{
  // This is the Details tab
  QWidget *tab2 = new QWidget(mTabWidget, "tab2");
  QGridLayout *layout = new QGridLayout(tab2, 6, 6);
  layout->setMargin(KDialogBase::marginHint());
  layout->setSpacing(KDialogBase::spacingHint());
  
  QLabel *label;
  KSeparator* bar;
  
  ///////////////////////
  // Office info
  
  // Department
  label = new QLabel(tab2);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("folder", KIcon::Desktop));
  layout->addMultiCellWidget(label, 0, 1, 0, 0);
  
  label = new QLabel(i18n("Department:"), tab2);
  layout->addWidget(label, 0, 1);
  mDepartmentEdit = new KLineEdit(tab2, "mDepartmentEdit");
  connect(mDepartmentEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget(mDepartmentEdit, 0, 2);
  
  label = new QLabel(i18n("Office:"), tab2);
  layout->addWidget(label, 1, 1);
  mOfficeEdit = new KLineEdit(tab2, "mOfficeEdit");
  connect(mOfficeEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget(mOfficeEdit, 1, 2);
  
  label = new QLabel(i18n("Profession:"), tab2);
  layout->addWidget(label, 2, 1);
  mProfessionEdit = new KLineEdit(tab2, "mProfessionEdit");
  connect(mProfessionEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget(mProfessionEdit, 2, 2);
  
  label = new QLabel(i18n("Manager\'s name:"), tab2);
  layout->addWidget(label, 0, 3);
  mManagerEdit = new KLineEdit(tab2, "mManagerEdit");
  connect(mManagerEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addMultiCellWidget(mManagerEdit, 0, 0, 4, 5);
  
  label = new QLabel(i18n("Assistant's name:"), tab2);
  layout->addWidget(label, 1, 3);
  mAssistantEdit = new KLineEdit(tab2, "mAssistantEdit");
  connect(mAssistantEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addMultiCellWidget(mAssistantEdit, 1, 1, 4, 5);
  
  bar = new KSeparator(KSeparator::HLine, tab2);
  layout->addMultiCellWidget(bar, 3, 3, 0, 5);
  
  /////////////////////////////////////////////////
  // Personal info
  
  label = new QLabel(tab2);
  label->setPixmap(KGlobal::iconLoader()->loadIcon("personal", KIcon::Desktop));
  layout->addMultiCellWidget(label, 4, 5, 0, 0);
  
  label = new QLabel(i18n("Nick name:"), tab2);
  layout->addWidget(label, 4, 1);
  mNicknameEdit = new KLineEdit(tab2, "mNicknameEdit");
  connect(mNicknameEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget(mNicknameEdit, 4, 2);

  label = new QLabel(i18n("Spouse's name:"), tab2);
  layout->addWidget(label, 5, 1);
  mSpouseEdit = new KLineEdit(tab2, "mSpouseEdit");
  connect(mSpouseEdit, SIGNAL(textChanged(const QString &)),
          SLOT(textChanged(const QString &)));
  layout->addWidget(mSpouseEdit, 5, 2);

  label = new QLabel(i18n("Birthday:"), tab2);
  layout->addWidget(label, 4, 3);
  mBirthdayPicker = new KDateEdit(tab2, "mBirthdayPicker");
  mBirthdayPicker->setHandleInvalid(true);
  connect(mBirthdayPicker, SIGNAL(dateChanged(QDate)),
          SLOT(dateChanged(QDate)));
  layout->addWidget(mBirthdayPicker, 4, 4);
  
  label = new QLabel(i18n("Anniversary:"), tab2);
  layout->addWidget(label, 5, 3);
  mAnniversaryPicker = new KDateEdit(tab2, "mAnniversaryPicker");
  mAnniversaryPicker->setHandleInvalid(true);
  connect(mAnniversaryPicker, SIGNAL(dateChanged(QDate)),
          SLOT(dateChanged(QDate)));
  layout->addWidget(mAnniversaryPicker, 5, 4);
  
  bar = new KSeparator(KSeparator::HLine, tab2);
  layout->addMultiCellWidget(bar, 6, 6, 0, 5);
  
   //////////////////////////////////////
  // Notes
  label = new QLabel(i18n("Note:"), tab2);
  label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  layout->addWidget(label, 7, 0);
  mNoteEdit = new QTextEdit( tab2, "mNoteEdit" );
  mNoteEdit->setWordWrap( QTextEdit::WidgetWidth );
  mNoteEdit->setMinimumSize( mNoteEdit->sizeHint() );
  connect(mNoteEdit, SIGNAL(textChanged()),
          SLOT(emitModified()));
  layout->addMultiCellWidget( mNoteEdit, 7, 7, 1, 5 );
  
   // Build the layout and add to the tab widget
  layout->activate(); // required
  mTabWidget->addTab( tab2, i18n( "&Details" ));
}

void AddresseeEditorWidget::setupTab3()
{
}
    
void AddresseeEditorWidget::load()
{ 
  // Block signals in case anything tries to emit modified
  // CS: This doesn't seem to work.
  bool block = signalsBlocked();
  blockSignals(true); 

  mNameEdit->setText(mAddressee.realName());
  mRoleEdit->setText(mAddressee.role());
  mOrgEdit->setText(mAddressee.organization());
  mURLEdit->setURL(mAddressee.url());
  mURLEdit->home(false);
  mNoteEdit->setText(mAddressee.note());
  mEmailWidget->setEmails(mAddressee.emails());
  mPhoneEditWidget->setPhoneNumbers(mAddressee.phoneNumbers());
  mAddressEditWidget->setAddresses(mAddressee.addresses());
  mBirthdayPicker->setDate(mAddressee.birthday().date());
  mAnniversaryPicker->setDate(QDate::fromString(mAddressee.custom("KADDRESSBOOK", "X-Anniversary"), Qt::ISODate));
  mNicknameEdit->setText(mAddressee.nickName());
  mCategoryEdit->setText(mAddressee.categories().join(","));
  
  // Load customs
  mIMAddressEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-IMAddress"));
  mSpouseEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-SpousesName"));
  mManagerEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-ManagersName"));
  mAssistantEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-AssistantsName"));
  mDepartmentEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-Department"));
  mOfficeEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-Office"));
  mProfessionEdit->setText(mAddressee.custom("KADDRESSBOOK", "X-Profession"));
  
  blockSignals(block);

  KConfig *config = kapp->config();
  config->setGroup( "General" );
  mParseBox->setChecked( config->readBoolEntry( "AutomaticNameParsing", true ) );
  
  mDirty = false;
}

void AddresseeEditorWidget::save()
{
  if ( !mDirty ) return;

  mAddressee.setNameFromString(mNameEdit->text());
  mAddressee.setFormattedName(mFormattedNameBox->currentText());
  mAddressee.setRole(mRoleEdit->text());
  mAddressee.setOrganization(mOrgEdit->text());
  mAddressee.setUrl(KURL(mURLEdit->text()));
  mAddressee.setNote(mNoteEdit->text());
  if ( mBirthdayPicker->inputIsValid() )
    mAddressee.setBirthday(QDateTime(mBirthdayPicker->date()));
  else
    mAddressee.setBirthday(QDateTime());
  
  mAddressee.setNickName(mNicknameEdit->text());
  mAddressee.setCategories(QStringList::split(",", mCategoryEdit->text()));
  
  // save custom fields
  mAddressee.insertCustom("KADDRESSBOOK", "X-IMAddress",
                          mIMAddressEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-SpousesName",
                          mSpouseEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-ManagersName",
                          mManagerEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-AssistantsName",
                          mAssistantEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-Department",
                          mDepartmentEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-Office",
                          mOfficeEdit->text());
  mAddressee.insertCustom("KADDRESSBOOK", "X-Profession",
                          mProfessionEdit->text());
  if ( mAnniversaryPicker->inputIsValid() )
    mAddressee.insertCustom("KADDRESSBOOK", "X-Anniversary",
                          mAnniversaryPicker->date().toString(Qt::ISODate));
  else
    mAddressee.removeCustom("KADDRESSBOOK", "X-Anniversary");
                          
  // Save the email addresses
  QStringList emails = mAddressee.emails();
  QStringList::Iterator iter;
  for (iter = emails.begin(); iter != emails.end(); ++iter)
    mAddressee.removeEmail(*iter);
  
  emails = mEmailWidget->emails();
  bool first = true;
  for (iter = emails.begin(); iter != emails.end(); ++iter)
  {
    mAddressee.insertEmail(*iter, first);
    first = false;
  }
  
  // Save the phone numbers
  KABC::PhoneNumber::List phoneNumbers;
  KABC::PhoneNumber::List::Iterator phoneIter;
  phoneNumbers = mAddressee.phoneNumbers();
  for (phoneIter = phoneNumbers.begin(); phoneIter != phoneNumbers.end();
        ++phoneIter)
    mAddressee.removePhoneNumber(*phoneIter);
    
  phoneNumbers = mPhoneEditWidget->phoneNumbers();
  for (phoneIter = phoneNumbers.begin(); phoneIter != phoneNumbers.end();
        ++phoneIter)
    mAddressee.insertPhoneNumber(*phoneIter);
    
  // Save the addresses
  KABC::Address::List addresses;
  KABC::Address::List::Iterator addressIter;
  addresses = mAddressee.addresses();
  for (addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter)
    mAddressee.removeAddress(*addressIter);
    
  addresses = mAddressEditWidget->addresses();
  for (addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter)
    mAddressee.insertAddress(*addressIter);

  KConfig *config = kapp->config();
  config->setGroup( "General" );
  config->writeEntry( "AutomaticNameParsing", mParseBox->isChecked() );

  mDirty = false;
}

bool AddresseeEditorWidget::dirty()
{
  return mDirty;
}

void AddresseeEditorWidget::nameTextChanged(const QString &text)
{
  // use the addressee class to parse the name for us
  if ( mParseBox->isChecked() ) {
    if ( !mAddressee.formattedName().isEmpty() ) {
      QString fn = mAddressee.formattedName();
      mAddressee.setNameFromString(text);
      mAddressee.setFormattedName( fn );
    } else
      mAddressee.setNameFromString(text);
  }

  nameBoxChanged();  

  emitModified();
}

void AddresseeEditorWidget::nameBoxChanged()
{
  /* 
   * Dummy addressee for parsing the name even if automatic parsing is
   * disabled
   */
  KABC::Addressee addr;
  addr.setNameFromString( mNameEdit->text() );

  int pos = mFormattedNameBox->currentItem();
  bool isEmpty = ( mFormattedNameBox->count() == 0 );
  mFormattedNameBox->clear();
  QStringList options;
  options << addr.givenName() + QString(" ") + addr.familyName()
          << mAddressee.formattedName()
          << addr.familyName() + QString(", ") + addr.givenName();
  mFormattedNameBox->insertStringList(options);
  if ( isEmpty )
    mFormattedNameBox->setCurrentText( mAddressee.formattedName() );
  else
    mFormattedNameBox->setCurrentItem( pos );
}

void AddresseeEditorWidget::nameButtonClicked()
{
  // show the name dialog.
  NameEditDialog dialog(mAddressee.familyName(), mAddressee.givenName(),
                        mAddressee.prefix(), mAddressee.suffix(),
                        mAddressee.additionalName(), this, "NameDialog");
  
  if (dialog.exec())
  {
    mAddressee.setFamilyName(dialog.familyName());
    mAddressee.setGivenName(dialog.givenName());
    mAddressee.setPrefix(dialog.prefix());
    mAddressee.setSuffix(dialog.suffix());
    mAddressee.setAdditionalName(dialog.additionalName());

    // Update the name edit.
    bool block = mNameEdit->signalsBlocked();
    mNameEdit->blockSignals( true );
    mNameEdit->setText( mAddressee.realName() );
    mNameEdit->blockSignals( block );

    // Update the combo box.
    nameBoxChanged();
    
    emitModified();
  }
}

void AddresseeEditorWidget::categoryButtonClicked()
{
  // Show the category dialog
  if (mCategoryDialog == 0)
  {
    mCategoryDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), this );
    connect(mCategoryDialog, SIGNAL(categoriesSelected(const QStringList &)),
            SLOT(categoriesSelected(const QStringList &)));
    connect(mCategoryDialog, SIGNAL(editCategories()),
            SLOT(editCategories()));
  }

  mCategoryDialog->setCategories();
  mCategoryDialog->setSelected(QStringList::split(",", mCategoryEdit->text()));
  mCategoryDialog->show();
  mCategoryDialog->raise();
}

void AddresseeEditorWidget::categoriesSelected(const QStringList &list)
{
  mCategoryEdit->setText(list.join(","));
}

void AddresseeEditorWidget::editCategories()
{
  if (mCategoryEditDialog == 0)
  {
    mCategoryEditDialog = new KPIM::CategoryEditDialog( KABPrefs::instance(), this );
    connect(mCategoryEditDialog, SIGNAL(categoryConfigChanged()),
            SLOT(categoryButtonClicked()));
  }
  
  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void AddresseeEditorWidget::emitModified()
{
//  kdDebug() << "AddresseeEditorWidget::emitModified()" << endl;

  mDirty = true;
  emit modified();
}


void AddresseeEditorWidget::dateChanged(QDate)
{
  emitModified();
}

#include "addresseeeditorwidget.moc"
