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

#include "addresseeeditorsupportwidgets.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qtextedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

#include <klineedit.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kdialog.h>

/////////////////////////////////////
// EmailWidget

EmailWidget::EmailWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
//  QVBoxLayout *topLayout = new QVBoxLayout(this);

  QGridLayout *topLayout = new QGridLayout(this, 4, 3);

  QLabel *label = new QLabel(i18n("Email address:"), this);
  topLayout->addWidget(label, 0, 0);

  mEmailEdit = new KLineEdit(this);
  topLayout->addWidget(mEmailEdit, 0, 1);
  connect(mEmailEdit, SIGNAL(returnPressed()), SLOT(add()));

  mAddButton = new QPushButton(i18n("Add"), this, "mAddButton");
  connect(mAddButton, SIGNAL(clicked()), SLOT(add()));
  topLayout->addWidget(mAddButton, 0, 2);

  mEmailListBox = new QListBox(this, "mEmailView");
  // Make sure there is room for the scrollbar
  mEmailListBox->setMinimumHeight(mEmailListBox->sizeHint().height() + 30);
  connect(mEmailListBox, SIGNAL(highlighted(int)), 
          SLOT(selectionChanged(int)));
  topLayout->addMultiCellWidget(mEmailListBox, 1, 3, 0, 1);
  
  mEditButton = new QPushButton(i18n("Change"), this, "mEditButton");
  connect(mEditButton, SIGNAL(clicked()), SLOT(edit()));
  topLayout->addWidget(mEditButton, 1, 2);

  mRemoveButton = new QPushButton(i18n("Remove..."), this, "mRemoveButton");
  connect(mRemoveButton, SIGNAL(clicked()), SLOT(remove()));
  topLayout->addWidget(mRemoveButton, 2, 2);

  mStandardButton = new QPushButton(i18n("Set Standard"), this, "mStandardButton");
  connect(mStandardButton, SIGNAL(clicked()), SLOT(standard()));
  topLayout->addWidget(mStandardButton, 3, 2);
  
  topLayout->activate();
  
  // set default state
  selectionChanged(-1);
}

EmailWidget::~EmailWidget()
{
}
    
void EmailWidget::setEmails(const QStringList &list)
{
  mEmailListBox->clear();
  mEmailListBox->insertStringList(list);
}

QStringList EmailWidget::emails() const
{
  QStringList emails;
  
  for (unsigned int i = 0; i < mEmailListBox->count(); ++i)
  {
    emails << mEmailListBox->text(i);
  }
  
  return emails;
}

void EmailWidget::add()
{
  if (!mEmailEdit->text().isEmpty())
  {
    mEmailListBox->insertItem(mEmailEdit->text());
    emit modified();
  }

  mEmailEdit->clear();
  mEmailEdit->setFocus();
}

void EmailWidget::edit()
{
  mEmailEdit->setText(mEmailListBox->currentText());
  mEmailEdit->setFocus();
}

void EmailWidget::remove()
{
  QString address = mEmailListBox->currentText();
  
  QString text = i18n("Are you sure that you want to remove the email address \"%1\"?").arg( address );
  
  QString caption = i18n("Confirm Remove");
  
  if (KMessageBox::questionYesNo(this, text, caption) == KMessageBox::Yes)
  {  
    mEmailListBox->removeItem(mEmailListBox->currentItem());
  
    emit modified();
  }
}

void EmailWidget::standard()
{
  QString text = mEmailListBox->currentText();
  mEmailListBox->removeItem(mEmailListBox->currentItem());
  mEmailListBox->insertItem(text, 0);
  mEmailListBox->setSelected(0, true);

  emit modified();
}

void EmailWidget::selectionChanged(int index)
{
  bool value = (index >= 0); // An item is selected

  mRemoveButton->setEnabled(value);
  mEditButton->setEnabled(value);
  mStandardButton->setEnabled(value);
}

void EmailWidget::keyPressEvent(QKeyEvent*)
{
}

/////////////////////////////////////
// NameEditDialog

NameEditDialog::NameEditDialog(const QString &familyName, 
                               const QString &givenName,
                               const QString &prefix, const QString &suffix,
                               const QString &additionalName, 
                               QWidget *parent, const char *name)
  : KDialogBase(KDialogBase::Plain, i18n("Edit Contact Name"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true)
{
  QWidget *page = plainPage();
  QGridLayout *layout = new QGridLayout(page);
  layout->setSpacing(spacingHint());
  layout->setMargin(marginHint());
  layout->addColSpacing(2, 50);
  QLabel *label;
  
  label = new QLabel(i18n("Honorific prefixes:"), page);
  layout->addWidget(label, 0, 0);
  mPrefixCombo = new KComboBox(page, "mPrefixCombo");
  mPrefixCombo->setDuplicatesEnabled(false);
  mPrefixCombo->setEditable(true);
  layout->addWidget(mPrefixCombo, 0, 1);
  
  label = new QLabel(i18n("Given name:"), page);
  layout->addWidget(label, 1, 0);
  mGivenNameEdit = new KLineEdit(page, "mGivenNameEdit");
  layout->addMultiCellWidget(mGivenNameEdit, 1, 1, 1, 2);

  label = new QLabel(i18n("Additional names:"), page);
  layout->addWidget(label, 2, 0);
  mAdditionalNameEdit = new KLineEdit(page, "mAdditionalNameEdit");
  layout->addMultiCellWidget(mAdditionalNameEdit, 2, 2, 1, 2);
  
  label = new QLabel(i18n("Family names:"), page);
  layout->addWidget(label, 3, 0);
  mFamilyNameEdit = new KLineEdit(page, "mFamilyNameEdit");
  layout->addMultiCellWidget(mFamilyNameEdit, 3, 3, 1, 2);
  
  label = new QLabel(i18n("Honorific suffixes:"), page);
  layout->addWidget(label, 4, 0);
  mSuffixCombo = new KComboBox(page, "mSuffixCombo");
  mSuffixCombo->setDuplicatesEnabled(false);
  mSuffixCombo->setEditable(true);
  layout->addWidget(mSuffixCombo, 4, 1);
  
  // Fill in the values
  mFamilyNameEdit->setText(familyName);
  mGivenNameEdit->setText(givenName);
  mAdditionalNameEdit->setText(additionalName);
  
  // Prefix and suffix combos
  QStringList sTitle;
  QStringList sSuffix;

  sTitle += i18n( "Dr." );
  sTitle += i18n( "Miss" );
  sTitle += i18n( "Mr." );
  sTitle += i18n( "Mrs." );
  sTitle += i18n( "Ms." );
  sTitle += i18n( "Prof." );
  sTitle.sort();

  sSuffix += i18n( "I" );
  sSuffix += i18n( "II" );
  sSuffix += i18n( "III" );
  sSuffix += i18n( "Jr." );
  sSuffix += i18n( "Sr." );
  sSuffix.sort();
  
  mPrefixCombo->insertStringList(sTitle);
  mSuffixCombo->insertStringList(sSuffix);
  
  mPrefixCombo->setCurrentText(prefix);
  mSuffixCombo->setCurrentText(suffix);
}
    
NameEditDialog::~NameEditDialog() 
{
}
    
QString NameEditDialog::familyName() const
{
  return mFamilyNameEdit->text();
}
    
QString NameEditDialog::givenName() const
{
  return mGivenNameEdit->text();
}
    
QString NameEditDialog::prefix() const
{
  return mPrefixCombo->currentText();
}
    
QString NameEditDialog::suffix() const
{
  return mSuffixCombo->currentText();
}
    
QString NameEditDialog::additionalName() const
{
  return mAdditionalNameEdit->text();
}

///////////////////////////
// AddressEditWidget

AddressEditWidget::AddressEditWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QHBoxLayout *layout = new QHBoxLayout(this);
  layout->setSpacing( KDialog::spacingHint() );
  
  QVBoxLayout *buttonLayout = new QVBoxLayout();
  buttonLayout->setSpacing( KDialog::spacingHint() );
  
  QPushButton *addressButton = new QPushButton(i18n("&Address..."), this);
  QToolTip::add(addressButton, i18n("Edit the current address"));
  connect(addressButton, SIGNAL(clicked()), SLOT(addressButtonClicked()));
  buttonLayout->addWidget(addressButton);
  
  mTypeCombo = new KComboBox(this);
  mTypeCombo->insertItem(i18n("Home"));
  mTypeCombo->insertItem(i18n("Organization"));
  mTypeMap[0] = KABC::Address::Home;
  mTypeMap[1] = KABC::Address::Work;
  connect(mTypeCombo, SIGNAL(highlighted(int)), SLOT(typeHighlighted(int)));
  buttonLayout->addWidget(mTypeCombo);
  
  // Add a spacer
  QWidget *spacer = new QWidget(this);
  buttonLayout->addWidget(spacer);
  
  layout->addLayout(buttonLayout);
  
  QVBoxLayout *editLayout = new QVBoxLayout();
  
  mAddressTextEdit = new QTextEdit(this, "mAddressTextEdit");
  // Set readonly for now since we can't parse an address from string
  mAddressTextEdit->setReadOnly(true);
  connect(mAddressTextEdit, SIGNAL(textChanged()), SLOT(textChanged()));
  editLayout->addWidget(mAddressTextEdit);
  
  mPreferredCheckBox = new QCheckBox(i18n("This is the preferred address"),
                                     this, "mPreferredCheckBox");
  connect(mPreferredCheckBox, SIGNAL(toggled(bool)), 
          SLOT(preferredToggled(bool)));
  editLayout->addWidget(mPreferredCheckBox);
  
  layout->addLayout(editLayout);
 
  mIndex = 0;
}

AddressEditWidget::~AddressEditWidget()
{
}
    
const KABC::Address::List &AddressEditWidget::addresses()
{
  return mAddressList;
}

void AddressEditWidget::setAddresses(const KABC::Address::List &list)
{
  mAddressList = list;
  
  mAddressTextEdit->setText("");
  
  // if there is no text in the edit, then the address of that type must
  // be empty. Try to find the first address that isn't empty
  for (unsigned int i = 0; 
      (i < mTypeMap.count()) && mAddressTextEdit->text().isEmpty(); ++i)
    mTypeCombo->setCurrentItem(i);
    
  if (mAddressTextEdit->text().isEmpty()) // still didn't find one, default to first
    mTypeCombo->setCurrentItem(0);
}

void AddressEditWidget::textChanged()
{
  // Parse the address from the string and store it back in the address
}

void AddressEditWidget::addressButtonClicked()
{
  // Find the current address
  bool found = false;
  int type = mTypeMap[mIndex];
  KABC::Address::List::Iterator iter;
  for (iter = mAddressList.begin(); iter != mAddressList.end() && !found; 
       ++iter)
  {
    found = (((*iter).type() & ~KABC::Address::Pref) == type);
  }
  
  if (found)
  {
    --iter;
    AddressEditDialog dialog(*iter, this);
    if (dialog.exec())
    {
      mAddressList.remove(iter);
      mAddressList.append(dialog.address());
      emit modified();
    }
  }
  else
  {
    // Create a default
    KABC::Address a;
    a.setType(type);
    AddressEditDialog dialog(a, this);
    if (dialog.exec())
    {
      mAddressList.append(dialog.address());
      emit modified();
    }
  }
  
  typeHighlighted(mIndex);
}

void AddressEditWidget::typeHighlighted(int index)
{
  // First try to find the type
  mIndex = index;
  int type = mTypeMap[mIndex];
  
  bool block = signalsBlocked();
  blockSignals(true);
  disconnect(mPreferredCheckBox, SIGNAL(toggled(bool)),
             this, SLOT(preferredToggled(bool)));
          
  bool found = false;
  KABC::Address a;
  KABC::Address::List::Iterator iter;
  for (iter = mAddressList.begin(); iter != mAddressList.end() && !found; 
       ++iter)
  {
    if (((*iter).type() & ~KABC::Address::Pref) == type)
    {
      found = true;
      a = *iter;
    }
  }
  
  if (found)
  {
    QString text;
    text += a.street() + "\n";
    if (!a.postOfficeBox().isEmpty())
      text += a.postOfficeBox() + "\n";
    text += a.locality() + QString(" ") + a.region() + QString(", ");
    text += a.postalCode() + "\n";
    text += a.country() + "\n";
    text += a.extended();
    mAddressTextEdit->setText(text);
    
    mPreferredCheckBox->setEnabled(true);
    mPreferredCheckBox->setChecked(a.type() & KABC::Address::Pref);
  }
  else
  {
    mAddressTextEdit->setText("");
    mPreferredCheckBox->setEnabled(false);
    mPreferredCheckBox->setChecked(false);
  }
  
  connect(mPreferredCheckBox, SIGNAL(toggled(bool)),
          SLOT(preferredToggled(bool)));
  blockSignals(block);
}

void AddressEditWidget::preferredToggled(bool state)
{
  int type = mTypeMap[mIndex];
  
  KABC::Address::List::Iterator iter;
  if (state)
  {
    for (iter = mAddressList.begin(); iter != mAddressList.end(); ++iter)
    {
      if ((*iter).type() != type)
        (*iter).setType((*iter).type() & ~KABC::Address::Pref);
      else
        (*iter).setType((*iter).type() | KABC::Address::Pref);
    }
  }
  else
  {
    for (iter = mAddressList.begin(); iter != mAddressList.end(); ++iter)
    {
      if ((*iter).type() == (type | KABC::Address::Pref))
        (*iter).setType((*iter).type() & ~KABC::Address::Pref);
    }
  }
  
  emit modified();
}

///////////////////////////////////////////
// PhoneEditWidget

PhoneEditWidget::PhoneEditWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
  QGridLayout *layout = new QGridLayout(this, 4, 2);
  layout->setSpacing(KDialogBase::spacingHint());
  layout->setAutoAdd(true);
  
  KComboBox *combo;
  KLineEdit *edit;
  
  int numPhones = 4;
  mComboVector.resize(numPhones);
  mEditVector.resize(numPhones);
  for (int i = 0; i < numPhones; i++)
  {
    combo = new KComboBox(this);
    connect(combo, SIGNAL(highlighted(int)), SLOT(typeChanged(int)));
  
    edit = new KLineEdit(this);
    connect(edit, SIGNAL(textChanged(const QString &)),
            SLOT(numberChanged(const QString &)));
    
    fillCombo(combo);        
    mComboVector.insert(i, combo);
    mEditVector.insert(i, edit);
  }
}

PhoneEditWidget::~PhoneEditWidget()
{
}

void PhoneEditWidget::fillCombo(KComboBox *combo)
{
  if (mTypeList.count() == 0)
  {
    mTypeList << i18n("Home")
              << i18n("Organization")
              << i18n("Home Fax")
              << i18n("Organization Fax")
              << i18n("Pager")
              << i18n("Car")
              << i18n("Cell")
              << i18n("Video")
              << i18n("ISDN");
              
    mTypeMap[0] = KABC::PhoneNumber::Home;
    mTypeMap[1] = KABC::PhoneNumber::Work;
    mTypeMap[2] = KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax;
    mTypeMap[3] = KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax;
    mTypeMap[4] = KABC::PhoneNumber::Pager;
    mTypeMap[5] = KABC::PhoneNumber::Car;
    mTypeMap[6] = KABC::PhoneNumber::Cell;
    mTypeMap[7] = KABC::PhoneNumber::Video;
    mTypeMap[8] = KABC::PhoneNumber::Isdn;
  }
  
  combo->insertStringList(mTypeList);
}

void PhoneEditWidget::setPhoneNumbers(const KABC::PhoneNumber::List &list)
{
  mPhoneNumberList = list;
 
  KABC::PhoneNumber::List::Iterator iter;
  bool found = false;
  QString number;
  
  for (unsigned int i = 0; i < mComboVector.count(); ++i)
  {
    mComboVector[i]->setCurrentItem(i);
    
    found = false;
    number = "";
    for (iter = mPhoneNumberList.begin();
         iter != mPhoneNumberList.end() && !found; ++iter)
    {
      if ((*iter).type() == mTypeMap[i])
      {
        number = (*iter).number();
        found = true;
      }
    }
    
    mEditVector[i]->setText(number);
  }
}

const KABC::PhoneNumber::List &PhoneEditWidget::phoneNumbers()
{
  return mPhoneNumberList;
}
 
void PhoneEditWidget::numberChanged(const QString &number)
{
  int index =  mEditVector.findRef(dynamic_cast<const KLineEdit*>(sender()));
  KLineEdit *sendingEdit = mEditVector[index];
  KComboBox *sendingCombo = mComboVector[index];
  KLineEdit *edit;
  
  // Update any other edits that may be displaying this number
  for (unsigned int i = 0; i < mEditVector.count(); i++)
  {
    edit = mEditVector[i];
    if (edit != sendingEdit)
    {
      if (mComboVector[i]->currentItem() == sendingCombo->currentItem())
      {
        // Avoid recursive signals
        disconnect(edit, SIGNAL(textChanged(const QString &)),
                   this, SLOT(numberChanged(const QString &)));
        edit->setText(number);
        connect(edit, SIGNAL(textChanged(const QString &)),
                SLOT(numberChanged(const QString &)));
      }
    }
  }
  
  // Now update the number in the list
  updatePhoneNumber(mTypeMap[sendingCombo->currentItem()], number);
}

void PhoneEditWidget::typeChanged(int index)
{ 
  int vIndex = mComboVector.findRef(dynamic_cast<const KComboBox*>(sender()));
  KLineEdit *sendingEdit = mEditVector[vIndex];
  int type = mTypeMap[index];
  bool found = false;
  QString number = "";
  
  KABC::PhoneNumber::List::Iterator iter;
  for (iter = mPhoneNumberList.begin(); 
       iter != mPhoneNumberList.end() && !found; ++iter)
  {
    if ((*iter).type() == type)
    {
      found = true;
      number = (*iter).number();
    }
  }
  
  disconnect(sendingEdit, SIGNAL(textChanged(const QString &)),
             this, SLOT(numberChanged(const QString &)));
  sendingEdit->setText(number);
  connect(sendingEdit, SIGNAL(textChanged(const QString &)),
          SLOT(numberChanged(const QString &)));
}

void PhoneEditWidget::updatePhoneNumber(int type, const QString &number)
{
  bool found = false;
  KABC::PhoneNumber p;
  KABC::PhoneNumber::List::Iterator iter;
  for (iter = mPhoneNumberList.begin(); 
       iter != mPhoneNumberList.end() && !found; ++iter)
  {
    p = *iter;
    if (p.type() == type)
      found = true;
  }
  
  if (found)
  {
    p.setNumber(number);
    mPhoneNumberList.remove(--iter);
    mPhoneNumberList.append(p);
  }
  else
  {
    KABC::PhoneNumber newP(number, type);
    mPhoneNumberList.append(newP);
  }
  
  emit modified();
}

///////////////////////////////////////////
// AddressEditDialog
AddressEditDialog::AddressEditDialog(const KABC::Address &a, QWidget *parent,
                                     const char *name)
  : KDialogBase(KDialogBase::Plain, i18n("Edit Address"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name)
{
  mAddress = a;
  
  QWidget *page = plainPage();
  
  QGridLayout *topLayout = new QGridLayout(page, 6, 2);
  topLayout->setSpacing(spacingHint());
  topLayout->setAutoAdd(true);
  
  QLabel *label = new QLabel(i18n("Street:"), page);
  label->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  //topLayout->addWidget(label);
  mStreetTextEdit = new QTextEdit(page, "mStreetTextEdit");
  //topLayout->addWidget(mStreetTextEdit);

  label = new QLabel(i18n("Post office box:"), page);
  //topLayout->addWidget(label);
  mPOBoxEdit = new KLineEdit(page, "mPOBoxEdit");
  //topLayout->addWidget(mPOBoxEdit);

  label = new QLabel(i18n("Locality:"), page);
  //topLayout->addWidget(label);
  mLocalityEdit = new KLineEdit(page, "mLocalityEdit");
  //topLayout->addWidget(mLocalityEdit);

  label = new QLabel(i18n("Region:"), page);
  //topLayout->addWidget(label);
  mRegionEdit = new KLineEdit(page, "mRegionEdit");
  //topLayout->addWidget(mRegionEdit);

  label = new QLabel(i18n("Postal code:"), page);
  //topLayout->addWidget(label);
  mPostalCodeEdit = new KLineEdit(page, "mPostalCodeEdit");
  //topLayout->addWidget(mPostalCodeEdit);

  label = new QLabel(i18n("Country:"), page);
  //topLayout->addWidget(label);
  mCountryCombo = new KComboBox(page, "mCountryCombo");
  mCountryCombo->setEditable(true);
  mCountryCombo->setDuplicatesEnabled(false);
  mCountryCombo->setAutoCompletion(true);
  //topLayout->addWidget(mCountryCombo);
  
  fillCombo(mCountryCombo);
  
  // Fill in the values
  mStreetTextEdit->setText(mAddress.street());
  mStreetTextEdit->setFocus();
  mRegionEdit->setText(mAddress.region());
  mLocalityEdit->setText(mAddress.locality());
  mPostalCodeEdit->setText(mAddress.postalCode());
  mPOBoxEdit->setText(mAddress.postOfficeBox());
  mCountryCombo->setCurrentText(mAddress.country());
  
  // initialize the layout
  topLayout->activate();
}

AddressEditDialog::~AddressEditDialog()
{
}
    
const KABC::Address &AddressEditDialog::address()
{
  mAddress.setLocality(mLocalityEdit->text());
  mAddress.setRegion(mRegionEdit->text());
  mAddress.setPostalCode(mPostalCodeEdit->text());
  mAddress.setCountry(mCountryCombo->currentText());
  mAddress.setPostOfficeBox(mPOBoxEdit->text());
  mAddress.setStreet(mStreetTextEdit->text());
  
  return mAddress;
}

void AddressEditDialog::fillCombo(KComboBox *combo)
{
  QString sCountry[] = {
    i18n( "Afghanistan" ), i18n( "Albania" ), i18n( "Algeria" ),
    i18n( "American Samoa" ), i18n( "Andorra" ), i18n( "Angola" ),
    i18n( "Anguilla" ), i18n( "Antarctica" ), i18n( "Antigua and Barbuda" ),
    i18n( "Argentina" ), i18n( "Armenia" ), i18n( "Aruba" ),
    i18n( "Ashmore and Cartier Islands" ), i18n( "Australia" ),
    i18n( "Austria" ), i18n( "Azerbaijan" ), i18n( "Bahamas" ),
    i18n( "Bahrain" ), i18n( "Bangladesh" ), i18n( "Barbados" ),
    i18n( "Belarus" ), i18n( "Belgium" ), i18n( "Belize" ),
    i18n( "Benin" ), i18n( "Bermuda" ), i18n( "Bhutan" ),
    i18n( "Bolivia" ), i18n( "Bosnia and Herzegovina" ), i18n( "Botswana" ),
    i18n( "Brazil" ), i18n( "Brunei" ), i18n( "Bulgaria" ),
    i18n( "Burkina Faso" ), i18n( "Burundi" ), i18n( "Cambodia" ),
    i18n( "Cameroon" ), i18n( "Canada" ), i18n( "Cape Verde" ),
    i18n( "Cayman Islands" ), i18n( "Central African Republic" ),
    i18n( "Chad" ), i18n( "Chile" ), i18n( "China" ), i18n( "Colombia" ),
    i18n( "Comoros" ), i18n( "Congo" ), i18n( "Congo, Dem. Rep." ),
    i18n( "Costa Rica" ), i18n( "Croatia" ),
    i18n( "Cuba" ), i18n( "Cyprus" ), i18n( "Czech Republic" ),
    i18n( "Denmark" ), i18n( "Djibouti" ),
    i18n( "Dominica" ), i18n( "Dominican Republic" ), i18n( "Ecuador" ),
    i18n( "Egypt" ), i18n( "El Salvador" ), i18n( "Equatorial Guinea" ),
    i18n( "Eritrea" ), i18n( "Estonia" ), i18n( "England" ),
    i18n( "Ethiopia" ), i18n( "European Union" ), i18n( "Faroe Islands" ),
    i18n( "Fiji" ), i18n( "Finland" ), i18n( "France" ),
    i18n( "French Polynesia" ), i18n( "Gabon" ), i18n( "Gambia" ),
    i18n( "Georgia" ), i18n( "Germany" ), i18n( "Ghana" ),
    i18n( "Greece" ), i18n( "Greenland" ), i18n( "Grenada" ),
    i18n( "Guam" ), i18n( "Guatemala" ), i18n( "Guinea" ),
    i18n( "Guinea-Bissau" ), i18n( "Guyana" ), i18n( "Haiti" ),
    i18n( "Honduras" ), i18n( "Hong Kong" ), i18n( "Hungary" ), 
    i18n( "Iceland" ), i18n( "India" ), i18n( "Indonesia" ),
    i18n( "Iran" ), i18n( "Iraq" ), i18n( "Ireland" ),
    i18n( "Israel" ), i18n( "Italy" ), i18n( "Ivory Coast" ),
    i18n( "Jamaica" ), i18n( "Japan" ), i18n( "Jordan" ),
    i18n( "Kazakhstan" ), i18n( "Kenya" ), i18n( "Kiribati" ),
    i18n( "Korea, North" ), i18n( "Korea, South" ),
    i18n( "Kuwait" ), i18n( "Kyrgyzstan" ), i18n( "Laos" ),
    i18n( "Latvia" ), i18n( "Lebanon" ), i18n( "Lesotho" ),
    i18n( "Liberia" ), i18n( "Libya" ), i18n( "Liechtenstein" ),
    i18n( "Lithuania" ), i18n( "Luxembourg" ), i18n( "Macau" ),
    i18n( "Madagascar" ), i18n( "Malawi" ), i18n( "Malaysia" ),
    i18n( "Maldives" ), i18n( "Mali" ), i18n( "Malta" ),
    i18n( "Marshall Islands" ), i18n( "Martinique" ), i18n( "Mauritania" ),
    i18n( "Mauritius" ), i18n( "Mexico" ),
    i18n( "Micronesia, Federated States Of" ), i18n( "Moldova" ),
    i18n( "Monaco" ), i18n( "Mongolia" ), i18n( "Montserrat" ),
    i18n( "Morocco" ), i18n( "Mozambique" ), i18n( "Myanmar" ),
    i18n( "Namibia" ),
    i18n( "Nauru" ), i18n( "Nepal" ), i18n( "Netherlands" ),
    i18n( "Netherlands Antilles" ), i18n( "New Caledonia" ),
    i18n( "New Zealand" ), i18n( "Nicaragua" ), i18n( "Niger" ),
    i18n( "Nigeria" ), i18n( "Niue" ), i18n( "North Korea" ),
    i18n( "Northern Ireland" ), i18n( "Northern Mariana Islands" ),
    i18n( "Norway" ), i18n( "Oman" ), i18n( "Pakistan" ), i18n( "Palau" ),
    i18n( "Palestinian" ), i18n( "Panama" ), i18n( "Papua New Guinea" ),
    i18n( "Paraguay" ), i18n( "Peru" ), i18n( "Philippines" ),
    i18n( "Poland" ), i18n( "Portugal" ), i18n( "Puerto Rico" ),
    i18n( "Qatar" ), i18n( "Romania" ), i18n( "Russia" ), i18n( "Rwanda" ),
    i18n( "St. Kitts and Nevis" ), i18n( "St. Lucia" ),
    i18n( "St. Vincent and the Grenadines" ), i18n( "San Marino" ),
    i18n( "Sao Tome and Principe" ), i18n( "Saudi Arabia" ),
    i18n( "Senegal" ), i18n( "Serbia & Montenegro" ), i18n( "Seychelles" ),
    i18n( "Sierra Leone" ), i18n( "Singapore" ), i18n( "Slovakia" ),
    i18n( "Slovenia" ), i18n( "Solomon Islands" ), i18n( "Somalia" ),
    i18n( "South Africa" ), i18n( "South Korea" ), i18n( "Spain" ),
    i18n( "Sri Lanka" ), i18n( "St. Kitts and Nevis" ), i18n( "Sudan" ),
    i18n( "Suriname" ), i18n( "Swaziland" ), i18n( "Sweden" ),
    i18n( "Switzerland" ), i18n( "Syria" ), i18n( "Taiwan" ),
    i18n( "Tajikistan" ), i18n( "Tanzania" ), i18n( "Thailand" ),
    i18n( "Tibet" ), i18n( "Togo" ), i18n( "Tonga" ),
    i18n( "Trinidad and Tobago" ), i18n( "Tunisia" ), i18n( "Turkey" ),
    i18n( "Turkmenistan" ), i18n( "Turks and Caicos Islands" ),
    i18n( "Tuvalu" ), i18n( "Uganda " ), i18n( "Ukraine" ),
    i18n( "United Arab Emirates" ), i18n( "United Kingdom" ),
    i18n( "United States" ), i18n( "Uruguay" ), i18n( "Uzbekistan" ),
    i18n( "Vanuatu" ), i18n( "Vatican City" ), i18n( "Venezuela" ),
    i18n( "Vietnam" ), i18n( "Western Samoa" ), i18n( "Yemen" ),
    i18n( "Yugoslavia" ), i18n( "Zaire" ), i18n( "Zambia" ),
    i18n( "Zimbabwe" ),
    ""
  };
  
  for (int i =0; sCountry[i] != ""; ++i )
    combo->insertItem(sCountry[i]);
}
    
#include "addresseeeditorsupportwidgets.moc"
