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

#include <qcheckbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtextedit.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

#include <kaccelmanager.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>

#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>

#include <libkdepim/kdateedit.h>

#include "addresseditwidget.h"
#include "emaileditwidget.h"
#include "geowidget.h"
#include "imagewidget.h"
#include "kabcore.h"
#include "kabprefs.h"
#include "keywidget.h"
#include "nameeditdialog.h"
#include "phoneeditwidget.h"
#include "secrecywidget.h"
#include "soundwidget.h"

#include "addresseeeditorwidget.h"

AddresseeEditorWidget::AddresseeEditorWidget( KABCore *core, bool isExtension,
                                              QWidget *parent, const char *name )
  : ExtensionWidget( core, parent, name ), mIsExtension( isExtension ),
    mBlockSignals( false )
{
  kdDebug(5720) << "AddresseeEditorWidget()" << endl;

  initGUI();
  mCategoryDialog = 0;
  mCategoryEditDialog = 0;
  
  // Load the empty addressee as defaults
  load();
  
  mDirty = false;
}

AddresseeEditorWidget::~AddresseeEditorWidget()
{
  kdDebug(5720) << "~AddresseeEditorWidget()" << endl;
}  
  
void AddresseeEditorWidget::contactsSelectionChanged()
{
  KABC::Addressee::List list = selectedContacts();

  mAddressee = list[ 0 ];
  load();
}

void AddresseeEditorWidget::setAddressee( const KABC::Addressee &addr )
{
  mAddressee = addr;
  load();
}

const KABC::Addressee &AddresseeEditorWidget::addressee()
{
  return mAddressee;
}

void AddresseeEditorWidget::textChanged( const QString& )
{
  emitModified();
}
  
void AddresseeEditorWidget::initGUI()
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  mTabWidget = new QTabWidget( this );
  layout->addWidget( mTabWidget );

  setupTab1();
  setupTab2();
  setupTab3();

  mNameEdit->setFocus();

  connect( mTabWidget, SIGNAL( currentChanged(QWidget*) ),
           SLOT( pageChanged(QWidget*) ) );
}

void AddresseeEditorWidget::setupTab1()
{
  // This is the General tab
  QWidget *tab1 = new QWidget( mTabWidget );

  QGridLayout *layout = new QGridLayout( tab1, 11, 7 );
  layout->setMargin( KDialogBase::marginHint() );
  layout->setSpacing( KDialogBase::spacingHint() );
  
  QLabel *label;
  KSeparator* bar;
  QPushButton *button;
  
  //////////////////////////////////
  // Upper left group (person info)
  
  // Person icon
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", KIcon::Desktop ) );
  layout->addMultiCellWidget( label, 0, 1, 0, 0 );
  
  // First name
  button = new QPushButton( i18n( "Name..." ), tab1 );
  QToolTip::add( button, i18n( "Edit the contact's name" ) );
  mNameEdit = new KLineEdit( tab1, "mNameEdit" );
  connect( mNameEdit, SIGNAL( textChanged( const QString& ) ), 
           SLOT( nameTextChanged( const QString& ) ) );
  connect( button, SIGNAL( clicked() ), SLOT( nameButtonClicked() ) );
  mNameLabel = new KSqueezedTextLabel( tab1 );
  mNameLabel->hide();
  layout->addWidget( button, 0, 1 );
  layout->addWidget( mNameEdit, 0, 2 );
  layout->addWidget( mNameLabel, 0, 2 );
  label = new QLabel( i18n( "Role:" ), tab1 );
  mRoleEdit = new KLineEdit( tab1 );
  connect( mRoleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mRoleEdit );
  layout->addWidget( label, 1, 1 );
  layout->addWidget( mRoleEdit, 1, 2 );
  
  // Organization
  label = new QLabel( i18n( "Organization:" ), tab1 );
  mOrgEdit = new KLineEdit( tab1 );
  label->setBuddy( mOrgEdit );
  connect( mOrgEdit, SIGNAL( textChanged( const QString& ) ), 
           SLOT( textChanged( const QString& ) ) );
  layout->addWidget( label, 2, 1 );
  layout->addWidget( mOrgEdit, 2, 2 );
  
  // File as (formatted name)
  label = new QLabel( i18n( "Formatted name:" ), tab1 );
  mFormattedNameLabel = new KSqueezedTextLabel( tab1 );
  layout->addWidget( label, 3, 1 );
  layout->addWidget( mFormattedNameLabel, 3, 2 );
  
  // Left hand separator. This separator doesn't go all the way
  // across so the dialog still flows from top to bottom
  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 4, 4, 0, 2 );
  
  //////////////////////////////////////
  // Phone numbers (upper right)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kaddressbook",
                    KIcon::Desktop ) );
  layout->addMultiCellWidget( label, 0, 1, 3, 3 );
  
  mPhoneEditWidget = new PhoneEditWidget( tab1 );
  connect( mPhoneEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mPhoneEditWidget, 0, 3, 4, 6 ); 

  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 4, 4, 3, 6 );
  
  //////////////////////////////////////
  // Addresses (lower left)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "gohome", KIcon::Desktop ) );
  layout->addMultiCellWidget( label, 5, 6, 0, 0 );
  
  mAddressEditWidget = new AddressEditWidget( tab1 );
  connect( mAddressEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mAddressEditWidget, 5, 9, 1, 2 );

  //////////////////////////////////////
  // Email / Web (lower right)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "email", KIcon::Desktop) );
  layout->addMultiCellWidget( label, 5, 6, 3, 3 );

  mEmailWidget = new EmailEditWidget( tab1 );
  connect( mEmailWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mEmailWidget, 5, 6, 4, 6 );

  // add the separator
  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 7, 7, 3, 6 );

  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "homepage", KIcon::Desktop) );
  layout->addMultiCellWidget( label, 8, 9, 3, 3 );

  label = new QLabel( i18n( "URL:" ), tab1 );
  mURLEdit = new KLineEdit( tab1 );
  connect( mURLEdit, SIGNAL( textChanged( const QString& ) ), 
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mURLEdit );
  layout->addWidget( label, 8, 4 );
  layout->addMultiCellWidget( mURLEdit, 8, 8, 5, 6 );

  label = new QLabel( i18n( "&IM address:" ), tab1 );
  mIMAddressEdit = new KLineEdit( tab1 );
  connect( mIMAddressEdit, SIGNAL( textChanged( const QString& ) ), 
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mIMAddressEdit );
  layout->addWidget( label, 9, 4 );
  layout->addMultiCellWidget( mIMAddressEdit, 9, 9, 5, 6 );
  
  layout->addColSpacing( 6, 50 );
  
  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 10, 10, 0, 6 );
  
  ///////////////////////////////////////
  QHBox *categoryBox = new QHBox( tab1 );
  categoryBox->setSpacing( KDialogBase::spacingHint() );
  
  // Categories
  button = new QPushButton( i18n( "Categories" ), categoryBox );
  connect( button, SIGNAL( clicked() ), SLOT( categoryButtonClicked() ) );

  mCategoryEdit = new KLineEdit( categoryBox );
  mCategoryEdit->setReadOnly( true );
  connect( mCategoryEdit, SIGNAL( textChanged( const QString& ) ), 
           SLOT( textChanged( const QString& ) ) );

  mSecrecyWidget = new SecrecyWidget( categoryBox );
  connect( mSecrecyWidget, SIGNAL( changed() ), SLOT( emitModified() ) );

  layout->addMultiCellWidget( categoryBox, 11, 11, 0, 6 );
  
  // Build the layout and add to the tab widget
  layout->activate(); // required

  mTabWidget->addTab( tab1, i18n( "&General" ) );
}

void AddresseeEditorWidget::setupTab2()
{
  // This is the Details tab
  QWidget *tab2 = new QWidget( mTabWidget );

  QGridLayout *layout = new QGridLayout( tab2, 6, 6 );
  layout->setMargin( KDialogBase::marginHint() );
  layout->setSpacing( KDialogBase::spacingHint() );
  
  QLabel *label;
  KSeparator* bar;
  
  ///////////////////////
  // Office info
  
  // Department
  label = new QLabel( tab2 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "folder", KIcon::Desktop ) );
  layout->addMultiCellWidget( label, 0, 1, 0, 0 );
  
  label = new QLabel( i18n( "Department:" ), tab2 );
  layout->addWidget( label, 0, 1 );
  mDepartmentEdit = new KLineEdit( tab2 );
  connect( mDepartmentEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mDepartmentEdit );
  layout->addWidget( mDepartmentEdit, 0, 2 );
  
  label = new QLabel( i18n( "Office:" ), tab2 );
  layout->addWidget( label, 1, 1 );
  mOfficeEdit = new KLineEdit( tab2 );
  connect( mOfficeEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mOfficeEdit );
  layout->addWidget( mOfficeEdit, 1, 2 );
  
  label = new QLabel( i18n( "Profession:" ), tab2 );
  layout->addWidget( label, 2, 1 );
  mProfessionEdit = new KLineEdit( tab2 );
  connect( mProfessionEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mProfessionEdit );
  layout->addWidget( mProfessionEdit, 2, 2 );
  
  label = new QLabel( i18n( "Manager\'s name:" ), tab2 );
  layout->addWidget( label, 0, 3 );
  mManagerEdit = new KLineEdit( tab2 );
  connect( mManagerEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mManagerEdit );
  layout->addMultiCellWidget( mManagerEdit, 0, 0, 4, 5 );
  
  label = new QLabel( i18n( "Assistant's name:" ), tab2 );
  layout->addWidget( label, 1, 3 );
  mAssistantEdit = new KLineEdit( tab2 );
  connect( mAssistantEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mAssistantEdit );
  layout->addMultiCellWidget( mAssistantEdit, 1, 1, 4, 5 );
  
  bar = new KSeparator( KSeparator::HLine, tab2 );
  layout->addMultiCellWidget( bar, 3, 3, 0, 5 );
  
  /////////////////////////////////////////////////
  // Personal info
  
  label = new QLabel( tab2 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", KIcon::Desktop ) );
  layout->addMultiCellWidget( label, 4, 5, 0, 0 );
  
  label = new QLabel( i18n( "Nick name:" ), tab2 );
  layout->addWidget( label, 4, 1 );
  mNicknameEdit = new KLineEdit( tab2 );
  connect( mNicknameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mNicknameEdit );
  layout->addWidget( mNicknameEdit, 4, 2 );

  label = new QLabel( i18n( "Spouse's name:" ), tab2 );
  layout->addWidget( label, 5, 1 );
  mSpouseEdit = new KLineEdit( tab2 );
  connect( mSpouseEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mSpouseEdit );
  layout->addWidget( mSpouseEdit, 5, 2 );

  label = new QLabel( i18n( "Birthday:" ), tab2 );
  layout->addWidget( label, 4, 3 );
  mBirthdayPicker = new KDateEdit( tab2 );
  mBirthdayPicker->setHandleInvalid( true );
  connect( mBirthdayPicker, SIGNAL( dateChanged( QDate ) ),
           SLOT( dateChanged( QDate ) ) );
  connect( mBirthdayPicker, SIGNAL( invalidDateEntered() ),
           SLOT( invalidDate() ) );
  connect( mBirthdayPicker, SIGNAL( textChanged( const QString& ) ),
           SLOT( emitModified() ) );
  label->setBuddy( mBirthdayPicker );
  layout->addWidget( mBirthdayPicker, 4, 4 );
  
  label = new QLabel( i18n( "Anniversary:" ), tab2 );
  layout->addWidget( label, 5, 3 );
  mAnniversaryPicker = new KDateEdit( tab2 );
  mAnniversaryPicker->setHandleInvalid( true );
  connect( mAnniversaryPicker, SIGNAL( dateChanged( QDate ) ),
           SLOT( dateChanged( QDate ) ) );
  connect( mAnniversaryPicker, SIGNAL( invalidDateEntered() ),
           SLOT( invalidDate() ) );
  connect( mAnniversaryPicker, SIGNAL( textChanged( const QString& ) ),
           SLOT( emitModified() ) );
  label->setBuddy( mAnniversaryPicker );
  layout->addWidget( mAnniversaryPicker, 5, 4 );
  
  bar = new KSeparator( KSeparator::HLine, tab2 );
  layout->addMultiCellWidget( bar, 6, 6, 0, 5 );
  
   //////////////////////////////////////
  // Notes
  label = new QLabel( i18n( "Note:" ), tab2 );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  layout->addWidget( label, 7, 0 );
  mNoteEdit = new QTextEdit( tab2 );
  mNoteEdit->setWordWrap( QTextEdit::WidgetWidth );
  mNoteEdit->setMinimumSize( mNoteEdit->sizeHint() );
  connect( mNoteEdit, SIGNAL( textChanged() ), SLOT( emitModified() ) );
  label->setBuddy( mNoteEdit );
  layout->addMultiCellWidget( mNoteEdit, 7, 7, 1, 5 );

   // Build the layout and add to the tab widget
  layout->activate(); // required

  mTabWidget->addTab( tab2, i18n( "&Details" ) );
}

void AddresseeEditorWidget::setupTab3()
{
  // This is the Misc tab
  QWidget *tab3 = new QWidget( mTabWidget );

  QGridLayout *layout = new QGridLayout( tab3, 2, 3 );
  layout->setMargin( KDialogBase::marginHint() );
  layout->setSpacing( KDialogBase::spacingHint() );
  layout->setColStretch( 2, 1 );
  
  //////////////////////////////////////
  // Geo
  mGeoWidget = new GeoWidget( tab3 );
  mGeoWidget->setMinimumSize( mGeoWidget->sizeHint() );
  connect( mGeoWidget, SIGNAL( changed() ), SLOT( emitModified() ) );
  layout->addWidget( mGeoWidget, 0, 0, Qt::AlignTop );

  //////////////////////////////////////
  // Sound
  mSoundWidget = new SoundWidget( tab3 );
  mSoundWidget->setMinimumSize( mSoundWidget->sizeHint() );
  connect( mSoundWidget, SIGNAL( changed() ), SLOT( emitModified() ) );
  layout->addWidget( mSoundWidget, 0, 1, Qt::AlignTop );

  //////////////////////////////////////
  // Images
  mImageWidget = new ImageWidget( tab3 );
  mImageWidget->setMinimumSize( mImageWidget->sizeHint() );
  connect( mImageWidget, SIGNAL( changed() ), SLOT( emitModified() ) );
  layout->addWidget( mImageWidget, 1, 0, Qt::AlignTop );

  //////////////////////////////////////
  // Keys
  mKeyWidget = new KeyWidget( tab3 );
  mKeyWidget->setMinimumSize( mKeyWidget->sizeHint() );
  connect( mKeyWidget, SIGNAL( changed() ), SLOT( emitModified() ) );
  layout->addWidget( mKeyWidget, 1, 1, Qt::AlignTop );

  mTabWidget->addTab( tab3, i18n( "&Misc" ) );
}
    
void AddresseeEditorWidget::load()
{
  kdDebug(5720) << "AddresseeEditorWidget::load()" << endl;

  // Block signals in case anything tries to emit modified
  // CS: This doesn't seem to work.
  bool block = signalsBlocked();
  blockSignals( true ); 
  mBlockSignals = true; // used for internal signal blocking

  mNameEdit->setText( mAddressee.assembledName() );
  
  if ( mAddressee.formattedName().isEmpty() ) {
    KConfig config( "kaddressbookrc" );
    config.setGroup( "General" );
    mFormattedNameType = config.readNumEntry( "FormattedNameType", 1 );
    mAddressee.setFormattedName( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
  } else {
    if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::SimpleName ) )
      mFormattedNameType = NameEditDialog::SimpleName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::FullName ) )
      mFormattedNameType = NameEditDialog::FullName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::ReverseName ) )
      mFormattedNameType = NameEditDialog::ReverseName;
    else
      mFormattedNameType = NameEditDialog::CustomName;
  }

  mFormattedNameLabel->setText( mAddressee.formattedName() );

  mRoleEdit->setText( mAddressee.role() );
  mOrgEdit->setText( mAddressee.organization() );
  mURLEdit->setURL( mAddressee.url().url() );
  mURLEdit->home( false );
  mNoteEdit->setText( mAddressee.note() );
  mEmailWidget->setEmails( mAddressee.emails() );
  mPhoneEditWidget->setPhoneNumbers( mAddressee.phoneNumbers() );
  mAddressEditWidget->setAddresses( mAddressee, mAddressee.addresses() );
  mBirthdayPicker->setDate( mAddressee.birthday().date() );
  mAnniversaryPicker->setDate( QDate::fromString( mAddressee.custom(
                               "KADDRESSBOOK", "X-Anniversary" ), Qt::ISODate) );
  mNicknameEdit->setText( mAddressee.nickName() );
  mCategoryEdit->setText( mAddressee.categories().join( "," ) );

  mGeoWidget->setGeo( mAddressee.geo() );
  mImageWidget->setPhoto( mAddressee.photo() );
  mImageWidget->setLogo( mAddressee.logo() );
  mKeyWidget->setKeys( mAddressee.keys() );
  mSecrecyWidget->setSecrecy( mAddressee.secrecy() );
  mSoundWidget->setSound( mAddressee.sound() );

  // Load customs
  mIMAddressEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-IMAddress" ) );
  mSpouseEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-SpousesName" ) );
  mManagerEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-ManagersName" ) );
  mAssistantEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  mDepartmentEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Department" ) );
  mOfficeEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Office" ) );
  mProfessionEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Profession" ) );
  
  blockSignals( block );
  mBlockSignals = false;

  mDirty = false;
}

void AddresseeEditorWidget::save()
{
  if ( !mDirty ) return;

  mAddressee.setRole( mRoleEdit->text() );
  mAddressee.setOrganization( mOrgEdit->text() );
  mAddressee.setUrl( KURL( mURLEdit->text() ) );
  mAddressee.setNote( mNoteEdit->text() );
  if ( mBirthdayPicker->inputIsValid() )
    mAddressee.setBirthday( QDateTime( mBirthdayPicker->date() ) );
  else
    mAddressee.setBirthday( QDateTime() );
  
  mAddressee.setNickName( mNicknameEdit->text() );
  mAddressee.setCategories( QStringList::split( ",", mCategoryEdit->text() ) );

  mAddressee.setGeo( mGeoWidget->geo() );
  mAddressee.setPhoto( mImageWidget->photo() );
  mAddressee.setLogo( mImageWidget->logo() );
  mAddressee.setKeys( mKeyWidget->keys() );
  mAddressee.setSound( mSoundWidget->sound() );
  mAddressee.setSecrecy( mSecrecyWidget->secrecy() );

  // save custom fields
  mAddressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", mIMAddressEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", mSpouseEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", mManagerEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", mAssistantEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-Department", mDepartmentEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-Office", mOfficeEdit->text() );
  mAddressee.insertCustom( "KADDRESSBOOK", "X-Profession", mProfessionEdit->text() );
  if ( mAnniversaryPicker->inputIsValid() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Anniversary",
                             mAnniversaryPicker->date().toString( Qt::ISODate ) );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Anniversary" );
                          
  // Save the email addresses
  QStringList emails = mAddressee.emails();
  QStringList::Iterator iter;
  for ( iter = emails.begin(); iter != emails.end(); ++iter )
    mAddressee.removeEmail( *iter );
  
  emails = mEmailWidget->emails();
  bool first = true;
  for ( iter = emails.begin(); iter != emails.end(); ++iter ) {
    mAddressee.insertEmail( *iter, first );
    first = false;
  }
  
  // Save the phone numbers
  KABC::PhoneNumber::List phoneNumbers;
  KABC::PhoneNumber::List::Iterator phoneIter;
  phoneNumbers = mAddressee.phoneNumbers();
  for ( phoneIter = phoneNumbers.begin(); phoneIter != phoneNumbers.end();
        ++phoneIter )
    mAddressee.removePhoneNumber( *phoneIter );
    
  phoneNumbers = mPhoneEditWidget->phoneNumbers();
  for ( phoneIter = phoneNumbers.begin(); phoneIter != phoneNumbers.end();
        ++phoneIter )
    mAddressee.insertPhoneNumber( *phoneIter );

  // Save the addresses
  KABC::Address::List addresses;
  KABC::Address::List::Iterator addressIter;
  addresses = mAddressee.addresses();
  for ( addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter )
    mAddressee.removeAddress( *addressIter );
    
  addresses = mAddressEditWidget->addresses();
  for ( addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter )
    mAddressee.insertAddress( *addressIter );

  mDirty = false;
}

bool AddresseeEditorWidget::dirty()
{
  return mDirty;
}

void AddresseeEditorWidget::nameTextChanged( const QString &text )
{
  // use the addressee class to parse the name for us
  AddresseeConfig config( mAddressee );
  if ( config.automaticNameParsing() ) {
    if ( !mAddressee.formattedName().isEmpty() ) {
      QString fn = mAddressee.formattedName();
      mAddressee.setNameFromString( text );
      mAddressee.setFormattedName( fn );
    } else {
      // use extra addressee to avoid a formatted name assignment
      Addressee addr;
      addr.setNameFromString( text );
      mAddressee.setPrefix( addr.prefix() );
      mAddressee.setGivenName( addr.givenName() );
      mAddressee.setAdditionalName( addr.additionalName() );
      mAddressee.setFamilyName( addr.familyName() );
      mAddressee.setSuffix( addr.suffix() );
    }
  }

  nameBoxChanged();

  emitModified();
}

void AddresseeEditorWidget::nameBoxChanged()
{
  KABC::Addressee addr;
  AddresseeConfig config( mAddressee );
  if ( config.automaticNameParsing() ) {
    addr.setNameFromString( mNameEdit->text() );
    mNameLabel->hide();
    mNameEdit->show();
  } else {
    addr = mAddressee;
    mNameEdit->hide();
    mNameLabel->setText( mNameEdit->text() );
    mNameLabel->show();
  }

  if ( mFormattedNameType != NameEditDialog::CustomName ) {
    mFormattedNameLabel->setText( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
    mAddressee.setFormattedName( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
  }
}

void AddresseeEditorWidget::nameButtonClicked()
{
  // show the name dialog.
  NameEditDialog dialog( mAddressee, mFormattedNameType, this );
  
  if ( dialog.exec() ) {
    if ( dialog.changed() ) {
      mAddressee.setFamilyName( dialog.familyName() );
      mAddressee.setGivenName( dialog.givenName() );
      mAddressee.setPrefix( dialog.prefix() );
      mAddressee.setSuffix( dialog.suffix() );
      mAddressee.setAdditionalName( dialog.additionalName() );
      mFormattedNameType = dialog.formattedNameType();
      if ( mFormattedNameType == NameEditDialog::CustomName ) {
        mFormattedNameLabel->setText( dialog.customFormattedName() );
        mAddressee.setFormattedName( dialog.customFormattedName() );
      }
      // Update the name edit.
      bool block = mNameEdit->signalsBlocked();
      mNameEdit->blockSignals( true );
      mNameEdit->setText( mAddressee.assembledName() );
      mNameEdit->blockSignals( block );

      // Update the combo box.
      nameBoxChanged();
    
      emitModified();
    }
  }
}

void AddresseeEditorWidget::categoryButtonClicked()
{
  // Show the category dialog
  if ( mCategoryDialog == 0 ) {
    mCategoryDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), this );
    connect( mCategoryDialog, SIGNAL( categoriesSelected( const QStringList& ) ),
             SLOT(categoriesSelected( const QStringList& ) ) );
    connect( mCategoryDialog, SIGNAL( editCategories() ), SLOT( editCategories() ) );
  }

  mCategoryDialog->setCategories();
  mCategoryDialog->setSelected( QStringList::split( ",", mCategoryEdit->text() ) );
  mCategoryDialog->show();
  mCategoryDialog->raise();
}

void AddresseeEditorWidget::categoriesSelected( const QStringList &list )
{
  mCategoryEdit->setText( list.join( "," ) );
}

void AddresseeEditorWidget::editCategories()
{
  if ( mCategoryEditDialog == 0 ) {
    mCategoryEditDialog = new KPIM::CategoryEditDialog( KABPrefs::instance(), this );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             SLOT( categoryButtonClicked() ) );
  }
  
  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void AddresseeEditorWidget::emitModified()
{
  mDirty = true;

  KABC::Addressee::List list;

  if ( mIsExtension && !mBlockSignals ) {
    save();
    list.append( mAddressee );
  }

  emit modified( list );
}

void AddresseeEditorWidget::dateChanged( QDate )
{
  emitModified();
}

void AddresseeEditorWidget::invalidDate()
{
  KMessageBox::sorry( this, i18n( "You must specify a valid date" ) );
}

void AddresseeEditorWidget::pageChanged( QWidget *wdg )
{
  if ( wdg )
    KAcceleratorManager::manage( wdg );
}

QString AddresseeEditorWidget::title() const
{
  return i18n( "Contact Editor" );
}

QString AddresseeEditorWidget::identifier() const
{
  return i18n( "contact_editor" );
}

#include "addresseeeditorwidget.moc"
