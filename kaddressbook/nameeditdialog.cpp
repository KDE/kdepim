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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qlabel.h>
#include <q3listbox.h>
#include <q3listview.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qstring.h>

//Added by qt3to4:
#include <QGridLayout>

#include <kacceleratormanager.h>
#include <kapplication.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klineedit.h>
#include <k3listview.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktoolinvocation.h>

#include "nameeditdialog.h"

NameEditDialog::NameEditDialog( const KABC::Addressee &addr, int type,
                                bool readOnly, QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Edit Contact Name" ), Help | Ok | Cancel,
                 Ok, parent, name, true ), mAddressee( addr )
{
  QWidget *page = plainPage();
  QGridLayout *layout = new QGridLayout( page );
  layout->setSpacing( spacingHint() );
  layout->addItem( new QSpacerItem( 100, 0 ), 0, 2 );
  QLabel *label;

  label = new QLabel( i18n( "Honorific prefixes:" ), page );
  layout->addWidget( label, 0, 0 );
  mPrefixCombo = new KComboBox( page );
  mPrefixCombo->setDuplicatesEnabled( false );
  mPrefixCombo->setEditable( true );
  mPrefixCombo->setEnabled( !readOnly );
  label->setBuddy( mPrefixCombo );
  layout->addWidget( mPrefixCombo, 0, 1, 1, 2 );

  mPrefixCombo->setWhatsThis( i18n( "The predefined honorific prefixes can be extended in the settings dialog." ) );

  label = new QLabel( i18n( "Given name:" ), page );
  layout->addWidget( label, 1, 0 );
  mGivenNameEdit = new KLineEdit( page );
  mGivenNameEdit->setReadOnly( readOnly );
  label->setBuddy( mGivenNameEdit );
  layout->addWidget( mGivenNameEdit, 1, 1, 1, 2 );

  label = new QLabel( i18n( "Additional names:" ), page );
  layout->addWidget( label, 2, 0 );
  mAdditionalNameEdit = new KLineEdit( page );
  mAdditionalNameEdit->setReadOnly( readOnly );
  label->setBuddy( mAdditionalNameEdit );
  layout->addWidget( mAdditionalNameEdit, 2, 1, 1, 2 );

  label = new QLabel( i18n( "Family names:" ), page );
  layout->addWidget( label, 3, 0 );
  mFamilyNameEdit = new KLineEdit( page );
  mFamilyNameEdit->setReadOnly( readOnly );
  label->setBuddy( mFamilyNameEdit );
  layout->addWidget( mFamilyNameEdit, 3, 1, 1, 2 );

  label = new QLabel( i18n( "Honorific suffixes:" ), page );
  layout->addWidget( label, 4, 0 );
  mSuffixCombo = new KComboBox( page );
  mSuffixCombo->setDuplicatesEnabled( false );
  mSuffixCombo->setEditable( true );
  mSuffixCombo->setEnabled( !readOnly );
  label->setBuddy( mSuffixCombo );
  layout->addWidget( mSuffixCombo, 4, 1, 1, 2 );

  mSuffixCombo->setWhatsThis( i18n( "The predefined honorific suffixes can be extended in the settings dialog." ) );

  label = new QLabel( i18n( "Formatted name:" ), page );
  layout->addWidget( label, 5, 0 );

  mFormattedNameCombo = new KComboBox( page );
  mFormattedNameCombo->setEnabled( !readOnly );
  layout->addWidget( mFormattedNameCombo, 5, 1 );
  connect( mFormattedNameCombo, SIGNAL( activated( int ) ), SLOT( typeChanged( int ) ) );

  mFormattedNameEdit = new KLineEdit( page );
  mFormattedNameEdit->setEnabled( type == CustomName && !readOnly );
  layout->addWidget( mFormattedNameEdit, 5, 2 );

  mParseBox = new QCheckBox( i18n( "Parse name automatically" ), page );
  mParseBox->setEnabled( !readOnly );
  connect( mParseBox, SIGNAL( toggled(bool) ), SLOT( parseBoxChanged(bool) ) );
  connect( mParseBox, SIGNAL( toggled(bool) ), SLOT( modified() ) );
  layout->addWidget( mParseBox, 6, 0, 1, 2 );

  // Fill in the values
  mFamilyNameEdit->setText( addr.familyName() );
  mGivenNameEdit->setText( addr.givenName() );
  mAdditionalNameEdit->setText( addr.additionalName() );
  mFormattedNameEdit->setText( addr.formattedName() );

  // Prefix and suffix combos
  KConfig config( "kabcrc" );
  config.setGroup( "General" );

  QStringList sTitle;
  sTitle += "";
  sTitle += i18n( "Dr." );
  sTitle += i18n( "Miss" );
  sTitle += i18n( "Mr." );
  sTitle += i18n( "Mrs." );
  sTitle += i18n( "Ms." );
  sTitle += i18n( "Prof." );
  sTitle += config.readEntry( "Prefixes" , QStringList() );
  sTitle.sort();

  QStringList sSuffix;
  sSuffix += "";
  sSuffix += i18n( "I" );
  sSuffix += i18n( "II" );
  sSuffix += i18n( "III" );
  sSuffix += i18n( "Jr." );
  sSuffix += i18n( "Sr." );
  sSuffix += config.readEntry( "Suffixes" , QStringList() );
  sSuffix.sort();

  mPrefixCombo->insertStringList( sTitle );
  mSuffixCombo->insertStringList( sSuffix );

  mPrefixCombo->setCurrentText( addr.prefix() );
  mSuffixCombo->setCurrentText( addr.suffix() );

  mAddresseeConfig.setAddressee( addr );
  mParseBox->setChecked( mAddresseeConfig.automaticNameParsing() );

  KAcceleratorManager::manage( this );

  connect( mPrefixCombo, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mGivenNameEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mAdditionalNameEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mFamilyNameEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mSuffixCombo, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mFormattedNameCombo, SIGNAL( activated( int ) ),
           this, SLOT( modified() ) );
  connect( mFormattedNameCombo, SIGNAL( activated( int ) ),
           this, SLOT( formattedNameTypeChanged() ) );
  connect( mFormattedNameEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( modified() ) );
  connect( mFormattedNameEdit, SIGNAL( textChanged( const QString& ) ),
           this, SLOT( formattedNameChanged( const QString& ) ) );

  initTypeCombo();
  mFormattedNameCombo->setCurrentItem( type );
  mPrefixCombo->lineEdit()->setFocus();
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

QString NameEditDialog::customFormattedName() const
{
  return mFormattedNameEdit->text();
}

int NameEditDialog::formattedNameType() const
{
  return mFormattedNameCombo->currentItem();
}

bool NameEditDialog::changed() const
{
  return mChanged;
}

void NameEditDialog::formattedNameTypeChanged()
{
  QString name;

  if ( formattedNameType() == CustomName )
    name = mCustomFormattedName;
  else {
    KABC::Addressee addr;
    addr.setPrefix( prefix() );
    addr.setFamilyName( familyName() );
    addr.setAdditionalName( additionalName() );
    addr.setGivenName( givenName() );
    addr.setSuffix( suffix() );
    addr.setOrganization( mAddressee.organization() );

    name = formattedName( addr, formattedNameType() );
  }

  mFormattedNameEdit->setText( name );
}

QString NameEditDialog::formattedName( const KABC::Addressee &addr, int type )
{
  QString name;

  switch ( type ) {
    case SimpleName:
      name = addr.givenName() + " " + addr.familyName();
      break;
    case FullName:
      name = addr.assembledName();
      break;
    case ReverseNameWithComma:
      name = addr.familyName() + ", " + addr.givenName();
      break;
    case ReverseName:
      name = addr.familyName() + " " + addr.givenName();
      break;
    case Organization:
      name = addr.organization();
      break;
    default:
      name = "";
      break;
  }

  return name.simplified();
}

void NameEditDialog::parseBoxChanged( bool value )
{
  mAddresseeConfig.setAutomaticNameParsing( value );
}

void NameEditDialog::typeChanged( int pos )
{
  mFormattedNameEdit->setEnabled( pos == 0 );
}

void NameEditDialog::formattedNameChanged( const QString &name )
{
  if ( formattedNameType() == CustomName )
    mCustomFormattedName = name;
}

void NameEditDialog::modified()
{
  mChanged = true;
}

void NameEditDialog::initTypeCombo()
{
  int pos = mFormattedNameCombo->currentItem();

  mFormattedNameCombo->clear();
  mFormattedNameCombo->insertItem( i18n( "Custom" ) );
  mFormattedNameCombo->insertItem( i18n( "Simple Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Full Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Reverse Name with Comma" ) );
  mFormattedNameCombo->insertItem( i18n( "Reverse Name" ) );
  mFormattedNameCombo->insertItem( i18n( "Organization" ) );

  mFormattedNameCombo->setCurrentItem( pos );
}

void NameEditDialog::slotHelp()
{
  KToolInvocation::invokeHelp( "managing-contacts-automatic-nameparsing" );
}

#include "nameeditdialog.moc"
