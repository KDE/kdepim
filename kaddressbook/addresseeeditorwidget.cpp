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

#include "addresseeeditorwidget.h"

#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>

#include <kabc/resource.h>
#include <kabc/resourceabc.h>
#include <kabc/stdaddressbook.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <kvbox.h>

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/kdateedit.h>

#include "addresseditwidget.h"
#include "advancedcustomfields.h"
#include "emaileditwidget.h"
#include "imeditwidget.h"
#include "kabprefs.h"
#include "keywidget.h"
#include "nameeditdialog.h"
#include "phoneeditwidget.h"
#include "secrecywidget.h"

AddresseeEditorWidget::AddresseeEditorWidget( QWidget *parent )
  : AddresseeEditorBase( parent ),
    mBlockSignals( false ), mReadOnly( false )
{
  kDebug(5720) <<"AddresseeEditorWidget()";

  initGUI();
  mCategorySelectDialog = 0;
  mCategoryEditDialog = 0;

  // Load the empty addressee as defaults
  load();

  mDirty = false;
}

AddresseeEditorWidget::~AddresseeEditorWidget()
{
  kDebug(5720) <<"~AddresseeEditorWidget()";
}

void AddresseeEditorWidget::setAddressee( const KABC::Addressee &addr )
{
  if ( mAddressee.uid() == addr.uid() )
    return;
  mAddressee = addr;

  bool readOnly = false;
  if ( KABC::Resource *res = addr.resource() ) {
    if ( res->readOnly() ) {
      readOnly = true;

    //HACK: some resources have finer access control than "generic" resources
    } else {
      KABC::ResourceABC *resAbc = dynamic_cast<KABC::ResourceABC *>( res );
      if ( resAbc ) {
        QString subresource = resAbc->uidToResourceMap()[ addr.uid() ];
        if ( !subresource.isEmpty() )
          readOnly |= !resAbc->subresourceWritable( subresource );
      }
    }
  }
  setReadOnly( readOnly );

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
  layout->setMargin( 0 );
  layout->setSpacing( KDialog::spacingHint() );

  mTabWidget = new KTabWidget( this );
  layout->addWidget( mTabWidget );

  setupTab1();
  setupTab2();
  setupAdditionalTabs();
  setupCustomFieldsTabs();
}

void AddresseeEditorWidget::setupTab1()
{
  // This is the General tab
  QWidget *page = new QWidget( mTabWidget );
  tab1.setupUi( page );
  //tab1.setMainWidget( page );

  //////////////////////////////////
  // Upper left group (person info)

  // Person icon
  tab1.mUserLabel->setPixmap( KIconLoader::global()->loadIcon( "user-identity", KIconLoader::Desktop,
                                                      KIconLoader::SizeMedium ) );

  // First name
  connect( tab1.mNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( nameTextChanged( const QString& ) ) );
  connect( tab1.mNameButton, SIGNAL( clicked() ), SLOT( nameButtonClicked() ) );
  mNameLabel = new KSqueezedTextLabel( page );

  if ( KABPrefs::instance()->automaticNameParsing() ) {
    mNameLabel->hide();
    tab1.mNameEdit->show();
  } else {
    tab1.mNameEdit->hide();
    mNameLabel->show();
  }

  tab1.layout->addWidget( mNameLabel, 0, 2 );
  tab1.mRoleLabel->setText( i18nc( "<roleLabel>:", "%1:", KABC::Addressee::roleLabel() ) );
  connect( tab1.mRoleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );

  // Organization
  tab1.mOrgLabel->setText( i18nc( "<organizationLabel>:", "%1:", KABC::Addressee::organizationLabel() ) );
  connect( tab1.mOrgEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( organizationTextChanged( const QString& ) ) );

  //////////////////////////////////////
  // Phone numbers (upper right)
  tab1.mPhoneLabel->setPixmap( KIconLoader::global()->loadIcon( "x-office-contact",
                    KIconLoader::Desktop, KIconLoader::SizeMedium ) );

  connect( tab1.mPhoneEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );

  //////////////////////////////////////
  // Addresses (lower left)
  tab1.mAddressLabel->setPixmap( KIconLoader::global()->loadIcon( "go-home", KIconLoader::Desktop,
                                                     KIconLoader::SizeMedium ) );

  connect( tab1.mAddressEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );

  //////////////////////////////////////
  // Email / Web (lower right)
  tab1.mEmailLabel->setPixmap( KIconLoader::global()->loadIcon( "mail-message", KIconLoader::Desktop,
                                                     KIconLoader::SizeMedium ) );

  tab1.mEmailWidget = new EmailEditWidget( page );
  connect( tab1.mEmailWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  tab1.layout->addWidget( tab1.mEmailWidget, 5, 4, 1, 2 );

  // add the separator

  tab1.mInternetLabel->setPixmap( KIconLoader::global()->loadIcon( "internet-web-browser", KIconLoader::Desktop,
                                                     KIconLoader::SizeMedium ) );

  tab1.mURLLabel->setText( i18nc( "<urlLabel>:", "%1:", KABC::Addressee::urlLabel() ) );

  connect( tab1.mURLEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );

  connect( tab1.mBlogEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( textChanged( const QString & ) ) );

  mIMWidget = new IMEditWidget( page, mAddressee, tab1.mInetLayout );
  connect( mIMWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  tab1.layout->addWidget( mIMWidget, 7, 4, 3, 2 );

  // Categories
  connect( tab1.mCategoryButton, SIGNAL( clicked() ), SLOT( selectCategories() ) );

  connect( tab1.mCategoryEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );

  connect( tab1.mSecrecyWidget, SIGNAL( changed() ), SLOT( emitModified() ) );

  // Build the layout and add to the tab widget
  //layout->activate(); // required

  mTabWidget->addTab( page, i18n( "General" ) );
}

void AddresseeEditorWidget::setupTab2()
{
  // This is the Details tab
  QWidget *page = new QWidget( mTabWidget );
  tab2.setupUi( page );

  ///////////////////////
  // Office info

  // Department
  tab2.mDepartmentIcon->setPixmap( KIconLoader::global()->loadIcon( "folder", KIconLoader::Desktop,
                                                     KIconLoader::SizeMedium ) );
  connect( tab2.mDepartmentEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( tab2.mOfficeEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( tab2.mProfessionEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( tab2.mManagerEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( tab2.mAssistantEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  tab2.mTitleLabel->setText( i18nc( "<titleLabel>:", "%1:", KABC::Addressee::titleLabel() ) );
  connect( tab2.mTitleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );

  /////////////////////////////////////////////////
  // Personal info

  tab2.mPersonalIcon->setPixmap( KIconLoader::global()->loadIcon( "user-identity", KIconLoader::Desktop,
                                                     KIconLoader::SizeMedium ) );
  connect( tab2.mNicknameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  tab2.mSpouseLabel->setText( i18nc( "Wife/Husband/...", "Partner's name:" ) );
  connect( tab2.mSpouseEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( tab2.mBirthdayPicker, SIGNAL( dateChanged( const QDate& ) ),
           SLOT( dateChanged( const QDate& ) ) );
  connect( tab2.mBirthdayPicker, SIGNAL( textChanged( const QString& ) ),
           SLOT( emitModified() ) );
  connect( tab2.mAnniversaryPicker, SIGNAL( dateChanged( const QDate& ) ),
           SLOT( dateChanged( const QDate& ) ) );
  connect( tab2.mAnniversaryPicker, SIGNAL( textChanged( const QString& ) ),
           SLOT( emitModified() ) );
   //////////////////////////////////////
  // Notes
  tab2.mNoteEdit->setWordWrapMode ( QTextOption::WrapAnywhere );
  tab2.mNoteEdit->setMinimumSize( tab2.mNoteEdit->sizeHint() );
  connect( tab2.mNoteEdit, SIGNAL( textChanged() ), SLOT( emitModified() ) );

  mTabWidget->addTab( page, i18n( "Details" ) );
}

void AddresseeEditorWidget::setupAdditionalTabs()
{
  ContactEditorWidgetManager *manager = ContactEditorWidgetManager::self();

  // create all tab pages and add the widgets
  for ( int i = 0; i < manager->count(); ++i ) {
    QString pageIdentifier = manager->factory( i )->pageIdentifier();
    QString pageTitle = manager->factory( i )->pageTitle();

    if ( pageIdentifier == "misc" )
      pageTitle = i18n( "Misc" );

    ContactEditorTabPage *page = mTabPages[ pageIdentifier ];
    if ( page == 0 ) { // tab not yet available, create one
      page = new ContactEditorTabPage( mTabWidget );
      mTabPages.insert( pageIdentifier, page );

      mTabWidget->addTab( page, pageTitle );

      connect( page, SIGNAL( changed() ), SLOT( emitModified() ) );
    }

    KAB::ContactEditorWidget *widget
              = manager->factory( i )->createWidget( KABC::StdAddressBook::self( true ),
                                                     page );
    if ( widget )
      page->addWidget( widget );
  }

  // query the layout update
  QHashIterator<QString, ContactEditorTabPage*> it( mTabPages );
  while ( it.hasNext() ) {
    it.next();
    it.value()->updateLayout();
  }
}

void AddresseeEditorWidget::setupCustomFieldsTabs()
{
  QStringList activePages = KABPrefs::instance()->advancedCustomFields();

  const QStringList list = KGlobal::dirs()->findAllResources( "data", "kaddressbook/contacteditorpages/*.ui",
                                                              KStandardDirs::Recursive |
                                                              KStandardDirs::NoDuplicates );
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    if ( !activePages.contains( (*it).mid( (*it).lastIndexOf('/') + 1 ) )  )
      continue;

    ContactEditorTabPage *page = new ContactEditorTabPage( mTabWidget );
    AdvancedCustomFields *wdg = new AdvancedCustomFields( *it, KABC::StdAddressBook::self( true ), page );
    if ( wdg ) {
      mTabPages.insert( wdg->pageIdentifier(), page );
      mTabWidget->addTab( page, wdg->pageTitle() );

      page->addWidget( wdg );
      page->updateLayout();

      connect( page, SIGNAL( changed() ), SLOT( emitModified() ) );
    } else
      delete page;
  }
}

void AddresseeEditorWidget::load()
{
  kDebug(5720) <<"AddresseeEditorWidget::load()";

  // Block signals in case anything tries to emit modified
  // CS: This doesn't seem to work.
  bool block = signalsBlocked();
  blockSignals( true );
  mBlockSignals = true; // used for internal signal blocking

  tab1.mNameEdit->blockSignals( true );
  tab1.mNameEdit->setText( mAddressee.assembledName() );
  tab1.mNameEdit->blockSignals( false );

  if ( mAddressee.formattedName().isEmpty() ) {
    KConfig _config( "kaddressbookrc" );
    KConfigGroup config(&_config, "General" );
    mFormattedNameType = config.readEntry( "FormattedNameType", 1 );
    mAddressee.setFormattedName( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
  } else {
    if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::SimpleName ) )
      mFormattedNameType = NameEditDialog::SimpleName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::FullName ) )
      mFormattedNameType = NameEditDialog::FullName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::ReverseNameWithComma ) )
      mFormattedNameType = NameEditDialog::ReverseNameWithComma;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::ReverseName ) )
      mFormattedNameType = NameEditDialog::ReverseName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::Organization ) )
      mFormattedNameType = NameEditDialog::Organization;
    else
      mFormattedNameType = NameEditDialog::CustomName;
  }

  tab1.mFormattedNameLabel->setText( mAddressee.formattedName() );

  tab1.mRoleEdit->setText( mAddressee.role() );
  tab1.mOrgEdit->setText( mAddressee.organization() );
  tab2.mDepartmentEdit->setText( mAddressee.department() );
  // compatibility with older versions
  if ( mAddressee.department().isEmpty() )
    tab2.mDepartmentEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Department" ) );
  tab1.mURLEdit->setUrl( mAddressee.url() );
  tab1.mURLEdit->home( false );
  tab1.mBlogEdit->setUrl( mAddressee.custom( "KADDRESSBOOK", "BlogFeed" ) );
  tab2.mNoteEdit->setText( mAddressee.note() );
  tab1.mEmailWidget->setEmails( mAddressee.emails() );
  tab1.mPhoneEditWidget->setPhoneNumbers( mAddressee.phoneNumbers() );
  tab1.mAddressEditWidget->setAddresses( mAddressee, mAddressee.addresses() );
  tab2.mBirthdayPicker->setDate( mAddressee.birthday().date() );

  QString anniversaryStr = mAddressee.custom( "KADDRESSBOOK", "X-Anniversary" );
  QDate anniversary = (anniversaryStr.isEmpty() ? QDate() : QDate::fromString( anniversaryStr, Qt::ISODate ));
  tab2.mAnniversaryPicker->setDate( anniversary );
  tab2.mNicknameEdit->setText( mAddressee.nickName() );
  tab1.mCategoryEdit->setText( mAddressee.categories().join( "," ) );

  tab1.mSecrecyWidget->setSecrecy( mAddressee.secrecy() );

  // Load customs
  mIMWidget->setPreferredIM( mAddressee.custom( "KADDRESSBOOK", "X-IMAddress" ) );
  tab2.mSpouseEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-SpousesName" ) );
  tab2.mManagerEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-ManagersName" ) );
  tab2.mAssistantEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  tab2.mOfficeEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Office" ) );
  tab2.mProfessionEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Profession" ) );
  tab2.mTitleEdit->setText( mAddressee.title() );

  QHashIterator<QString, ContactEditorTabPage*> it( mTabPages );
  while ( it.hasNext() ) {
    it.next();
    it.value()->loadContact( &mAddressee );
  }

  blockSignals( block );
  mBlockSignals = false;

  mDirty = false;
}

void AddresseeEditorWidget::save()
{
  if ( !mDirty ) return;

  mAddressee.setRole( tab1.mRoleEdit->text() );
  mAddressee.setOrganization( tab1.mOrgEdit->text() );
  mAddressee.setDepartment( tab2.mDepartmentEdit->text() );
  mAddressee.setUrl( KUrl( tab1.mURLEdit->text().trimmed() ) );
  if ( !tab1.mBlogEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "BlogFeed", tab1.mBlogEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "BlogFeed" );

  mAddressee.setNote( tab2.mNoteEdit->toPlainText() );
  if ( tab2.mBirthdayPicker->date().isValid() )
    mAddressee.setBirthday( QDateTime( tab2.mBirthdayPicker->date() ) );
  else
    mAddressee.setBirthday( QDateTime() );

  mAddressee.setNickName( tab2.mNicknameEdit->text() );
  mAddressee.setCategories( tab1.mCategoryEdit->text().split( ',', QString::SkipEmptyParts ) );

  mAddressee.setSecrecy( tab1.mSecrecyWidget->secrecy() );

  // save custom fields
  if ( !mIMWidget->preferredIM().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", mIMWidget->preferredIM() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-IMAddress" );
  if ( !tab2.mSpouseEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", tab2.mSpouseEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-SpousesName" );
  if ( !tab2.mManagerEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", tab2.mManagerEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-ManagersName" );
  if ( !tab2.mAssistantEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", tab2.mAssistantEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-AssistantsName" );

  if ( !tab2.mOfficeEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Office", tab2.mOfficeEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Office" );
  if ( !tab2.mProfessionEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Profession", tab2.mProfessionEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Profession" );

  if ( tab2.mAnniversaryPicker->date().isValid() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Anniversary",
                             tab2.mAnniversaryPicker->date().toString( Qt::ISODate ) );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Anniversary" );

  mAddressee.setTitle( tab2.mTitleEdit->text() );

  // Save the email addresses
  mAddressee.setEmails( tab1.mEmailWidget->emails() );

  // Save the phone numbers
  KABC::PhoneNumber::List phoneNumbers;
  KABC::PhoneNumber::List::ConstIterator phoneIter;
  phoneNumbers = mAddressee.phoneNumbers();
  for ( phoneIter = phoneNumbers.constBegin(); phoneIter != phoneNumbers.constEnd();
        ++phoneIter )
    mAddressee.removePhoneNumber( *phoneIter );

  phoneNumbers = tab1.mPhoneEditWidget->phoneNumbers();
  for ( phoneIter = phoneNumbers.constBegin(); phoneIter != phoneNumbers.constEnd();
        ++phoneIter )
    mAddressee.insertPhoneNumber( *phoneIter );

  // Save the addresses
  KABC::Address::List addresses;
  KABC::Address::List::ConstIterator addressIter;
  addresses = mAddressee.addresses();
  for ( addressIter = addresses.constBegin(); addressIter != addresses.constEnd();
        ++addressIter )
    mAddressee.removeAddress( *addressIter );

  addresses = tab1.mAddressEditWidget->addresses();
  for ( addressIter = addresses.constBegin(); addressIter != addresses.constEnd();
        ++addressIter )
    mAddressee.insertAddress( *addressIter );

  QHashIterator<QString, ContactEditorTabPage*> it( mTabPages );
  while ( it.hasNext() ) {
    it.next();
    it.value()->storeContact( &mAddressee );
  }

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

void AddresseeEditorWidget::organizationTextChanged( const QString &text )
{

  AddresseeConfig config( mAddressee );
  if ( config.automaticNameParsing() )
    mAddressee.setOrganization( text );

  nameBoxChanged();

  tab1.mAddressEditWidget->updateAddressee( mAddressee );

  emitModified();
}

void AddresseeEditorWidget::nameBoxChanged()
{
  KABC::Addressee addr;
  AddresseeConfig config( mAddressee );
  if ( config.automaticNameParsing() ) {
    addr.setNameFromString( tab1.mNameEdit->text() );
    mNameLabel->hide();
    tab1.mNameEdit->show();
  } else {
    addr = mAddressee;
    tab1.mNameEdit->hide();
    mNameLabel->setText( tab1.mNameEdit->text() );
    mNameLabel->show();
  }

  if ( mFormattedNameType != NameEditDialog::CustomName ) {
    tab1.mFormattedNameLabel->setText( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
    mAddressee.setFormattedName( NameEditDialog::formattedName( mAddressee, mFormattedNameType ) );
  }

  tab1.mAddressEditWidget->updateAddressee( mAddressee );
}

void AddresseeEditorWidget::nameButtonClicked()
{
  // show the name dialog.
  NameEditDialog dialog( mAddressee, mFormattedNameType, mReadOnly, this );

  if ( dialog.exec() ) {
    if ( dialog.changed() ) {
      mAddressee.setFamilyName( dialog.familyName() );
      mAddressee.setGivenName( dialog.givenName() );
      mAddressee.setPrefix( dialog.prefix() );
      mAddressee.setSuffix( dialog.suffix() );
      mAddressee.setAdditionalName( dialog.additionalName() );
      mFormattedNameType = dialog.formattedNameType();
      if ( mFormattedNameType == NameEditDialog::CustomName ) {
        tab1.mFormattedNameLabel->setText( dialog.customFormattedName() );
        mAddressee.setFormattedName( dialog.customFormattedName() );
      }
      // Update the name edit.
      bool block = tab1.mNameEdit->signalsBlocked();
      tab1.mNameEdit->blockSignals( true );
      tab1.mNameEdit->setText( mAddressee.assembledName() );
      tab1.mNameEdit->blockSignals( block );

      // Update the combo box.
      nameBoxChanged();

      emitModified();
    }
  }
}

void AddresseeEditorWidget::selectCategories()
{
  // Show the category dialog
  if ( mCategorySelectDialog == 0 ) {
    mCategorySelectDialog = new KPIM::CategorySelectDialog( KABPrefs::instance(), this );
    connect( mCategorySelectDialog, SIGNAL( categoriesSelected( const QStringList& ) ),
             this, SLOT( categoriesSelected( const QStringList& ) ) );
    connect( mCategorySelectDialog, SIGNAL( editCategories() ),
             this, SLOT( editCategories() ) );
  }

  mCategorySelectDialog->setSelected( tab1.mCategoryEdit->text().split( ',', QString::SkipEmptyParts) );
  mCategorySelectDialog->show();
  mCategorySelectDialog->raise();
}

void AddresseeEditorWidget::categoriesSelected( const QStringList &list )
{
  tab1.mCategoryEdit->setText( list.join( "," ) );
}

void AddresseeEditorWidget::editCategories()
{
  if ( mCategoryEditDialog == 0 ) {
    mCategoryEditDialog = new KPIM::CategoryEditDialog( KABPrefs::instance(), this );
    connect( mCategoryEditDialog, SIGNAL( categoryConfigChanged() ),
             mCategorySelectDialog, SLOT( updateCategoryConfig() ) );
  }

  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void AddresseeEditorWidget::emitModified()
{
  if ( mBlockSignals )
    return;

  mDirty = true;

  emit modified();
}

void AddresseeEditorWidget::dateChanged( const QDate& )
{
  emitModified();
}

void AddresseeEditorWidget::invalidDate()
{
  KMessageBox::sorry( this, i18n( "You must specify a valid date" ) );
}

void AddresseeEditorWidget::setInitialFocus()
{
  tab1.mNameEdit->setFocus();
}

bool AddresseeEditorWidget::readyToClose()
{
  bool ok = true;

  QDate date = tab2.mBirthdayPicker->date();
  if ( !date.isValid() && !tab2.mBirthdayPicker->currentText().isEmpty() ) {
    KMessageBox::error( this, i18n( "You have to enter a valid birthdate." ) );
    ok = false;
  }

  date = tab2.mAnniversaryPicker->date();
  if ( !date.isValid() && !tab2.mAnniversaryPicker->currentText().isEmpty() ) {
    KMessageBox::error( this, i18n( "You have to enter a valid anniversary." ) );
    ok = false;
  }

  return ok;
}

void AddresseeEditorWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  tab1.mNameEdit->setReadOnly( readOnly );
  tab1.mRoleEdit->setReadOnly( readOnly );
  tab1.mOrgEdit->setReadOnly( readOnly );
  tab1.mPhoneEditWidget->setReadOnly( readOnly );
  tab1.mAddressEditWidget->setReadOnly( readOnly );
  tab1.mEmailWidget->setReadOnly( readOnly );
  tab1.mURLEdit->setReadOnly( readOnly );
  tab1.mBlogEdit->setReadOnly( readOnly );
  mIMWidget->setReadOnly( readOnly );
  tab1.mCategoryButton->setEnabled( !readOnly );
  tab1.mSecrecyWidget->setReadOnly( readOnly );
  tab2.mDepartmentEdit->setReadOnly( readOnly );
  tab2.mOfficeEdit->setReadOnly( readOnly );
  tab2.mProfessionEdit->setReadOnly( readOnly );
  tab2.mManagerEdit->setReadOnly( readOnly );
  tab2.mAssistantEdit->setReadOnly( readOnly );
  tab2.mTitleEdit->setReadOnly( readOnly );
  tab2.mNicknameEdit->setReadOnly( readOnly );
  tab2.mSpouseEdit->setReadOnly( readOnly );
  tab2.mBirthdayPicker->setEnabled( !readOnly );
  tab2.mAnniversaryPicker->setEnabled( !readOnly );
  tab2.mNoteEdit->setReadOnly( mReadOnly );

  QHashIterator<QString, ContactEditorTabPage*> it( mTabPages );
  while ( it.hasNext() ) {
    it.next();
    it.value()->setReadOnly( readOnly );
  }
}

#include "addresseeeditorwidget.moc"
