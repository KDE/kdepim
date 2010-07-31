/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <tqcheckbox.h>
#include <tqframe.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqtooltip.h>
#include <tqcombobox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktrader.h>

#include "addresseewidget.h"
#include "kabprefs.h"

#include "kabconfigwidget.h"

KABConfigWidget::KABConfigWidget( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0,
                                            KDialog::spacingHint() );

  TQTabWidget *tabWidget = new TQTabWidget( this );
  topLayout->addWidget( tabWidget );

  // General page
  TQWidget *generalPage = new TQWidget( this );
  TQVBoxLayout *layout = new TQVBoxLayout( generalPage, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQGroupBox *groupBox = new TQGroupBox( 0, Qt::Vertical, i18n( "General" ), generalPage );
  TQBoxLayout *boxLayout = new TQVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new TQCheckBox( i18n( "Honor KDE single click" ), groupBox, "msingle" );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new TQCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox, "mparse" );
  boxLayout->addWidget( mNameParsing );

  mTradeAsFamilyName = new TQCheckBox( i18n( "Trade single name component as family name" ), groupBox, "mtrade" );
  boxLayout->addWidget( mTradeAsFamilyName );
/**
  TODO: show the checkbox when we can compile agains kdelibs from HEAD, atm it
        doesn't work and would just confuse the users ;)
*/
  mTradeAsFamilyName->hide();

  mLimitContactDisplay = new TQCheckBox( i18n( "Limit unfiltered display to 100 contacts" ), groupBox, "mlimit" );
  boxLayout->addWidget( mLimitContactDisplay );

  TQBoxLayout *editorLayout = new TQHBoxLayout( boxLayout, KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Addressee editor type:" ), groupBox );
  editorLayout->addWidget( label );

  mEditorCombo = new TQComboBox( groupBox );
  mEditorCombo->insertItem( i18n( "Full Editor" ) );
  mEditorCombo->insertItem( i18n( "Simple Editor" ) );
  label->setBuddy( mEditorCombo );
  editorLayout->addWidget( mEditorCombo );

  editorLayout->addStretch( 1 );


  layout->addWidget( groupBox );

  groupBox = new TQGroupBox( 0, Qt::Vertical, i18n( "Script-Hooks" ), generalPage );
  TQGridLayout *grid = new TQGridLayout( groupBox->layout(), 3, 2,
                                       KDialog::spacingHint() );
  label = new TQLabel( i18n( "Phone:" ), groupBox );
  grid->addWidget( label, 0, 0 );

  mPhoneHook = new TQLineEdit( groupBox );
  TQToolTip::add( mPhoneHook, i18n( "<ul><li>%N: Phone Number</li></ul>" ) );
  grid->addWidget( mPhoneHook, 0, 1 );

  label = new TQLabel( i18n( "Fax:" ), groupBox );
  grid->addWidget( label, 1, 0 );

  mFaxHook = new TQLineEdit( groupBox );
  TQToolTip::add( mFaxHook, i18n( "<ul><li>%N: Fax Number</li></ul>" ) );
  grid->addWidget( mFaxHook, 1, 1 );


  label = new TQLabel( i18n( "SMS Text:" ), groupBox );
  grid->addWidget( label, 2, 0 );

  mSMSHook = new TQLineEdit( groupBox );
  TQToolTip::add( mSMSHook, i18n( "<ul><li>%N: Phone Number</li><li>%F: File containing the text message(s)</li></ul>" ) );
  grid->addWidget( mSMSHook, 2, 1 );


  grid->setColStretch( 1, 1 );

  layout->addWidget( groupBox );

  groupBox = new TQGroupBox( 0, Qt::Vertical, i18n( "Location Map" ), generalPage );
  boxLayout = new TQVBoxLayout( groupBox->layout(), KDialog::spacingHint() );
  boxLayout->setAlignment( Qt::AlignTop );

  mLocationMapURL = new TQComboBox( true, groupBox );
  mLocationMapURL->setSizePolicy( TQSizePolicy( TQSizePolicy::Preferred, TQSizePolicy::Fixed ) );
  TQToolTip::add( mLocationMapURL, i18n( "<ul> <li>%s: Street</li>"
                                 "<li>%r: Region</li>"
                                 "<li>%l: Location</li>"
                                 "<li>%z: Zip Code</li>"
                                 "<li>%c: Country ISO Code</li> </ul>" ) );
  mLocationMapURL->insertStringList( KABPrefs::instance()->locationMapURLs() );
  boxLayout->addWidget( mLocationMapURL );
  layout->addWidget( groupBox );

  connect( mNameParsing, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( modified() ) );
  connect( mViewsSingleClickBox, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( modified() ) );
  connect( mTradeAsFamilyName, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( modified() ) );
  connect( mLimitContactDisplay, TQT_SIGNAL( toggled( bool ) ), TQT_SLOT( modified() ) );
  connect( mPhoneHook, TQT_SIGNAL( textChanged( const TQString& ) ), TQT_SLOT( modified() ) );
  connect( mSMSHook, TQT_SIGNAL( textChanged( const TQString& ) ), TQT_SLOT( modified() ) );
  connect( mFaxHook, TQT_SIGNAL( textChanged( const TQString& ) ), TQT_SLOT( modified() ) );
  connect( mLocationMapURL, TQT_SIGNAL( textChanged( const TQString& ) ), TQT_SLOT( modified() ) );
  connect( mEditorCombo, TQT_SIGNAL( activated( int ) ), TQT_SLOT( modified() ) );

  tabWidget->addTab( generalPage, i18n( "General" ) );

  // Addressee page
  mAddresseeWidget = new AddresseeWidget( this );
  tabWidget->addTab( mAddresseeWidget, i18n( "Contact" ) );
  connect( mAddresseeWidget, TQT_SIGNAL( modified() ), TQT_SLOT( modified() ) );
}

void KABConfigWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->automaticNameParsing() );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->honorSingleClick() );
  mPhoneHook->setText( KABPrefs::instance()->phoneHookApplication() );
  mSMSHook->setText( KABPrefs::instance()->sMSHookApplication() );
  mFaxHook->setText( KABPrefs::instance()->faxHookApplication() );
  mAddresseeWidget->restoreSettings();
  mEditorCombo->setCurrentItem( KABPrefs::instance()->editorType() );
  mLocationMapURL->setCurrentText( KABPrefs::instance()->locationMapURL().arg( KGlobal::locale()->country() ) );
  mLocationMapURL->lineEdit()->setCursorPosition( 0 );

  KConfig config( "kabcrc", false, false );
  config.setGroup( "General" );
  mTradeAsFamilyName->setChecked( config.readBoolEntry( "TradeAsFamilyName", true ) );
  mLimitContactDisplay->setChecked( config.readBoolEntry( "LimitContactDisplay", true ) );

  blockSignals( blocked );

  emit changed( false );
}

void KABConfigWidget::saveSettings()
{
  KABPrefs::instance()->setAutomaticNameParsing( mNameParsing->isChecked() );
  KABPrefs::instance()->setHonorSingleClick( mViewsSingleClickBox->isChecked() );
  KABPrefs::instance()->setPhoneHookApplication( mPhoneHook->text() );
  KABPrefs::instance()->setSMSHookApplication( mSMSHook->text() );
  KABPrefs::instance()->setFaxHookApplication( mFaxHook->text() );
  KABPrefs::instance()->setEditorType( mEditorCombo->currentItem() );
  KABPrefs::instance()->setLocationMapURL( mLocationMapURL->currentText() );
  mAddresseeWidget->saveSettings();

  KABPrefs::instance()->writeConfig();

  KConfig config( "kabcrc", false, false );
  config.setGroup( "General" );
  config.writeEntry( "TradeAsFamilyName", mTradeAsFamilyName->isChecked() );
  config.writeEntry( "LimitContactDisplay", mLimitContactDisplay->isChecked() );

  emit changed( false );
}

void KABConfigWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );
  mEditorCombo->setCurrentItem( 0 );
  mLimitContactDisplay->setChecked( true );

  emit changed( true );
}

void KABConfigWidget::modified()
{
  emit changed( true );
}

#include "kabconfigwidget.moc"
