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

#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>

#include <kaccelmanager.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klistview.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "nameeditdialog.h"

NameEditDialog::NameEditDialog( const KABC::Addressee &addr, QWidget *parent, const char *name )
  : KDialogBase(KDialogBase::Plain, i18n("Edit Contact Name"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, true)
{
  QWidget *page = plainPage();
  QGridLayout *layout = new QGridLayout(page);
  layout->setSpacing(spacingHint());
  layout->addColSpacing(2, 50);
  QLabel *label;
  
  label = new QLabel(i18n("Honorific prefixes:"), page);
  layout->addWidget(label, 0, 0);
  mPrefixCombo = new KComboBox(page, "mPrefixCombo");
  mPrefixCombo->setDuplicatesEnabled(false);
  mPrefixCombo->setEditable(true);
  label->setBuddy( mPrefixCombo );
  layout->addWidget(mPrefixCombo, 0, 1);
  
  label = new QLabel(i18n("Given name:"), page);
  layout->addWidget(label, 1, 0);
  mGivenNameEdit = new KLineEdit(page, "mGivenNameEdit");
  label->setBuddy( mGivenNameEdit );
  layout->addMultiCellWidget(mGivenNameEdit, 1, 1, 1, 2);

  label = new QLabel(i18n("Additional names:"), page);
  layout->addWidget(label, 2, 0);
  mAdditionalNameEdit = new KLineEdit(page, "mAdditionalNameEdit");
  label->setBuddy( mAdditionalNameEdit );
  layout->addMultiCellWidget(mAdditionalNameEdit, 2, 2, 1, 2);
  
  label = new QLabel(i18n("Family names:"), page);
  layout->addWidget(label, 3, 0);
  mFamilyNameEdit = new KLineEdit(page, "mFamilyNameEdit");
  label->setBuddy( mFamilyNameEdit );
  layout->addMultiCellWidget(mFamilyNameEdit, 3, 3, 1, 2);
  
  label = new QLabel(i18n("Honorific suffixes:"), page);
  layout->addWidget(label, 4, 0);
  mSuffixCombo = new KComboBox(page, "mSuffixCombo");
  mSuffixCombo->setDuplicatesEnabled(false);
  mSuffixCombo->setEditable(true);
  label->setBuddy( mSuffixCombo );
  layout->addWidget(mSuffixCombo, 4, 1);

  mParseBox = new QCheckBox( i18n( "Parse name automatically" ), page );
  connect( mParseBox, SIGNAL( toggled(bool) ), SLOT( parseBoxChanged(bool) ) );
  layout->addMultiCellWidget( mParseBox, 5, 5, 0, 1 );
  
  // Fill in the values
  mFamilyNameEdit->setText(addr.familyName());
  mGivenNameEdit->setText(addr.givenName());
  mAdditionalNameEdit->setText(addr.additionalName());
  
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
  
  mPrefixCombo->setCurrentText(addr.prefix());
  mSuffixCombo->setCurrentText(addr.suffix());

  mAddresseeConfig.setAddressee( addr );
  mParseBox->setChecked( mAddresseeConfig.automaticNameParsing() );

  KAcceleratorManager::manage( this );

  connect( mPrefixCombo, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mGivenNameEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mAdditionalNameEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mFamilyNameEdit, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mSuffixCombo, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );

  mChanged = false;
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

bool NameEditDialog::changed() const
{
  return mChanged;
}

void NameEditDialog::parseBoxChanged( bool value )
{
  mAddresseeConfig.setAutomaticNameParsing( value );
}

void NameEditDialog::modified()
{
  mChanged = true;
}

#include "nameeditdialog.moc"
