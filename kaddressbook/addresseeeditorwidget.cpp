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

#include <kabc/resource.h>
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
#include <kstandarddirs.h>

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/categoryeditdialog.h>
#include <libkdepim/categoryselectdialog.h>
#include <libkdepim/kdateedit.h>

#include "addresseditwidget.h"
#include "advancedcustomfields.h"
#include "core.h"
#include "emaileditwidget.h"
#include "imeditwidget.h"
#include "kabprefs.h"
#include "keywidget.h"
#include "nameeditdialog.h"
#include "phoneeditwidget.h"
#include "secrecywidget.h"

#include "addresseeeditorwidget.h"

AddresseeEditorWidget::AddresseeEditorWidget( KAB::Core *core, bool isExtension,
                                              QWidget *parent, const char *name )
  : AddresseeEditorBase( core, isExtension, parent, name ),
    mBlockSignals( false ), mReadOnly( false )
{
  kdDebug(5720) << "AddresseeEditorWidget()" << endl;

  initGUI();
  mCategorySelectDialog = 0;
  mCategoryEditDialog = 0;

  // Load the empty addressee as defaults
  load();

  mDirty = false;
}

AddresseeEditorWidget::~AddresseeEditorWidget()
{
  kdDebug(5720) << "~AddresseeEditorWidget()" << endl;
}

void AddresseeEditorWidget::setAddressee( const KABC::Addressee &addr )
{
  if ( mAddressee.uid() == addr.uid() )
	  return;

  mAddressee = addr;

  bool readOnly = ( !addr.resource() ? false : addr.resource()->readOnly() );
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

  mTabWidget = new QTabWidget( this );
  layout->addWidget( mTabWidget );

  setupTab1();
  setupTab2();
  setupAdditionalTabs();
  setupCustomFieldsTabs();

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
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", KIcon::Desktop,
                                                      KIcon::SizeMedium ) );
  layout->addMultiCellWidget( label, 0, 1, 0, 0 );

  // First name
  button = new QPushButton( i18n( "Edit Name..." ), tab1 );
  QToolTip::add( button, i18n( "Edit the contact's name" ) );
  mNameEdit = new KLineEdit( tab1, "mNameEdit" );
  connect( mNameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( nameTextChanged( const QString& ) ) );
  connect( button, SIGNAL( clicked() ), SLOT( nameButtonClicked() ) );
  mNameLabel = new KSqueezedTextLabel( tab1 );

  if ( KABPrefs::instance()->automaticNameParsing() ) {
    mNameLabel->hide();
    mNameEdit->show();
  } else {
    mNameEdit->hide();
    mNameLabel->show();
  }

  layout->addWidget( button, 0, 1 );
  layout->addWidget( mNameEdit, 0, 2 );
  layout->addWidget( mNameLabel, 0, 2 );
  label = new QLabel( i18n( "<roleLabel>:", "%1:" ).arg( KABC::Addressee::roleLabel() ), tab1 );
  mRoleEdit = new KLineEdit( tab1 );
  connect( mRoleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mRoleEdit );
  layout->addWidget( label, 1, 1 );
  layout->addWidget( mRoleEdit, 1, 2 );

  // Organization
  label = new QLabel( i18n( "<organizationLabel>:", "%1:" ).arg( KABC::Addressee::organizationLabel() ), tab1 );
  mOrgEdit = new KLineEdit( tab1 );
  label->setBuddy( mOrgEdit );
  connect( mOrgEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( organizationTextChanged( const QString& ) ) );
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
                    KIcon::Desktop, KIcon::SizeMedium ) );
  layout->addMultiCellWidget( label, 0, 1, 3, 3 );

  mPhoneEditWidget = new PhoneEditWidget( tab1 );
  connect( mPhoneEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mPhoneEditWidget, 0, 3, 4, 6 );

  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 4, 4, 3, 6 );

  //////////////////////////////////////
  // Addresses (lower left)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kfm_home", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
  layout->addMultiCellWidget( label, 5, 6, 0, 0 );

  mAddressEditWidget = new AddressEditWidget( tab1 );
  connect( mAddressEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mAddressEditWidget, 5, 10, 1, 2 );

  //////////////////////////////////////
  // Email / Web (lower right)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "email", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
  layout->addMultiCellWidget( label, 5, 6, 3, 3 );

  mEmailWidget = new EmailEditWidget( tab1 );
  connect( mEmailWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mEmailWidget, 5, 6, 4, 6 );

  // add the separator
  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 7, 7, 3, 6 );

  QHBoxLayout *homePageLayout = new QHBoxLayout( 0, 11, 7 );

  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "homepage", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
  homePageLayout->addWidget( label );

  label = new QLabel( i18n( "<urlLabel>:", "%1:" ).arg( KABC::Addressee::urlLabel() ), tab1 );
  mURLEdit = new KLineEdit( tab1 );
  connect( mURLEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mURLEdit );
  homePageLayout->addWidget( label );
  homePageLayout->addWidget( mURLEdit );
  layout->addMultiCellLayout( homePageLayout, 8, 8, 3, 6 );

  QHBoxLayout *blogLayout = new QHBoxLayout( 0, 11, 7 );
  label = new QLabel( i18n("Blog feed:"), tab1 );
  blogLayout->addWidget( label );
  mBlogEdit = new KLineEdit( tab1 );
  blogLayout->addWidget( mBlogEdit );
  connect( mBlogEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( textChanged( const QString & ) ) );
  label->setBuddy( mBlogEdit );
  layout->addMultiCellLayout( blogLayout, 9, 9, 4, 6 );

  mIMWidget = new IMEditWidget( tab1, mAddressee );
  connect( mIMWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addMultiCellWidget( mIMWidget, 10, 10, 4, 6 );

  layout->addColSpacing( 6, 50 );

  bar = new KSeparator( KSeparator::HLine, tab1 );
  layout->addMultiCellWidget( bar, 11, 11, 0, 6 );

  ///////////////////////////////////////
  QHBox *categoryBox = new QHBox( tab1 );
  categoryBox->setSpacing( KDialogBase::spacingHint() );

  // Categories
  mCategoryButton = new QPushButton( i18n( "Select Categories..." ), categoryBox );
  connect( mCategoryButton, SIGNAL( clicked() ), SLOT( selectCategories() ) );

  mCategoryEdit = new KLineEdit( categoryBox );
  mCategoryEdit->setReadOnly( true );
  connect( mCategoryEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );

  mSecrecyWidget = new SecrecyWidget( categoryBox );
  connect( mSecrecyWidget, SIGNAL( changed() ), SLOT( emitModified() ) );

  layout->addMultiCellWidget( categoryBox, 12, 12, 0, 6 );

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
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "folder", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
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
  mManagerEdit = new KPIM::AddresseeLineEdit( tab2 );
  connect( mManagerEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mManagerEdit );
  layout->addMultiCellWidget( mManagerEdit, 0, 0, 4, 5 );

  label = new QLabel( i18n( "Assistant's name:" ), tab2 );
  layout->addWidget( label, 1, 3 );
  mAssistantEdit = new KPIM::AddresseeLineEdit( tab2 );
  connect( mAssistantEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mAssistantEdit );
  layout->addMultiCellWidget( mAssistantEdit, 1, 1, 4, 5 );

  label = new QLabel( i18n( "<titleLabel>:", "%1:" ).arg( KABC::Addressee::titleLabel() ), tab2 );
  layout->addWidget( label, 2, 3 );
  mTitleEdit = new KLineEdit( tab2 );
  connect( mTitleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mTitleEdit );
  layout->addMultiCellWidget( mTitleEdit, 2, 2, 4, 5 );

  bar = new KSeparator( KSeparator::HLine, tab2 );
  layout->addMultiCellWidget( bar, 3, 3, 0, 5 );

  /////////////////////////////////////////////////
  // Personal info

  label = new QLabel( tab2 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", KIcon::Desktop,
                                                     KIcon::SizeMedium ) );
  layout->addMultiCellWidget( label, 4, 5, 0, 0 );

  label = new QLabel( i18n( "Nickname:" ), tab2 );
  layout->addWidget( label, 4, 1 );
  mNicknameEdit = new KLineEdit( tab2 );
  connect( mNicknameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mNicknameEdit );
  layout->addWidget( mNicknameEdit, 4, 2 );

  label = new QLabel( i18n( "Spouse's name:" ), tab2 );
  layout->addWidget( label, 5, 1 );
  mSpouseEdit = new KPIM::AddresseeLineEdit( tab2 );
  connect( mSpouseEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mSpouseEdit );
  layout->addWidget( mSpouseEdit, 5, 2 );

  label = new QLabel( i18n( "Birthdate:" ), tab2 );
  layout->addWidget( label, 4, 3 );
  mBirthdayPicker = new KDateEdit( tab2 );
  connect( mBirthdayPicker, SIGNAL( dateChanged( const QDate& ) ),
           SLOT( dateChanged( const QDate& ) ) );
  connect( mBirthdayPicker, SIGNAL( textChanged( const QString& ) ),
           SLOT( emitModified() ) );
  label->setBuddy( mBirthdayPicker );
  layout->addWidget( mBirthdayPicker, 4, 4 );

  label = new QLabel( i18n( "Anniversary:" ), tab2 );
  layout->addWidget( label, 5, 3 );
  mAnniversaryPicker = new KDateEdit( tab2 );
  connect( mAnniversaryPicker, SIGNAL( dateChanged( const QDate& ) ),
           SLOT( dateChanged( const QDate& ) ) );
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
              = manager->factory( i )->createWidget( core()->addressBook(),
                                                     page );
    if ( widget )
      page->addWidget( widget );
  }

  // query the layout update
  QDictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->updateLayout();
}

void AddresseeEditorWidget::setupCustomFieldsTabs()
{
  QStringList activePages = KABPrefs::instance()->advancedCustomFields();

  const QStringList list = KGlobal::dirs()->findAllResources( "data", "kaddressbook/contacteditorpages/*.ui", true, true );
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    if ( activePages.find( (*it).mid( (*it).findRev('/') + 1 ) ) == activePages.end() )
      continue;

    ContactEditorTabPage *page = new ContactEditorTabPage( mTabWidget );
    AdvancedCustomFields *wdg = new AdvancedCustomFields( *it, core()->addressBook(), page );
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
  kdDebug(5720) << "AddresseeEditorWidget::load()" << endl;

  // Block signals in case anything tries to emit modified
  // CS: This doesn't seem to work.
  bool block = signalsBlocked();
  blockSignals( true );
  mBlockSignals = true; // used for internal signal blocking

  mNameEdit->blockSignals( true );
  mNameEdit->setText( mAddressee.assembledName() );
  mNameEdit->blockSignals( false );

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
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::ReverseNameWithComma ) )
      mFormattedNameType = NameEditDialog::ReverseNameWithComma;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::ReverseName ) )
      mFormattedNameType = NameEditDialog::ReverseName;
    else if ( mAddressee.formattedName() == NameEditDialog::formattedName( mAddressee, NameEditDialog::Organization ) )
      mFormattedNameType = NameEditDialog::Organization;
    else
      mFormattedNameType = NameEditDialog::CustomName;
  }

  mFormattedNameLabel->setText( mAddressee.formattedName() );

  mRoleEdit->setText( mAddressee.role() );
  mOrgEdit->setText( mAddressee.organization() );
  mURLEdit->setURL( mAddressee.url() );
  mURLEdit->home( false );
  mBlogEdit->setURL( mAddressee.custom( "KADDRESSBOOK", "BlogFeed" ) );
  mNoteEdit->setText( mAddressee.note() );
  mEmailWidget->setEmails( mAddressee.emails() );
  mPhoneEditWidget->setPhoneNumbers( mAddressee.phoneNumbers() );
  mAddressEditWidget->setAddresses( mAddressee, mAddressee.addresses() );
  mBirthdayPicker->setDate( mAddressee.birthday().date() );

  QString anniversaryStr = mAddressee.custom( "KADDRESSBOOK", "X-Anniversary" );
  QDate anniversary = (anniversaryStr.isEmpty() ? QDate() : QDate::fromString( anniversaryStr, Qt::ISODate ));
  mAnniversaryPicker->setDate( anniversary );
  mNicknameEdit->setText( mAddressee.nickName() );
  mCategoryEdit->setText( mAddressee.categories().join( "," ) );

  mSecrecyWidget->setSecrecy( mAddressee.secrecy() );

  // Load customs
  mIMWidget->setPreferredIM( mAddressee.custom( "KADDRESSBOOK", "X-IMAddress" ) );
  mSpouseEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-SpousesName" ) );
  mManagerEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-ManagersName" ) );
  mAssistantEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-AssistantsName" ) );
  mDepartmentEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Department" ) );
  mOfficeEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Office" ) );
  mProfessionEdit->setText( mAddressee.custom( "KADDRESSBOOK", "X-Profession" ) );
  mTitleEdit->setText( mAddressee.title() );

  QDictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->loadContact( &mAddressee );

  blockSignals( block );
  mBlockSignals = false;

  mDirty = false;
}

void AddresseeEditorWidget::save()
{
  if ( !mDirty ) return;

  mAddressee.setRole( mRoleEdit->text() );
  mAddressee.setOrganization( mOrgEdit->text() );
  mAddressee.setUrl( KURL( mURLEdit->text().stripWhiteSpace() ) );
  mAddressee.insertCustom( "KADDRESSBOOK", "BlogFeed", mBlogEdit->text() );
  mAddressee.setNote( mNoteEdit->text() );
  if ( mBirthdayPicker->date().isValid() )
    mAddressee.setBirthday( QDateTime( mBirthdayPicker->date() ) );
  else
    mAddressee.setBirthday( QDateTime() );

  mAddressee.setNickName( mNicknameEdit->text() );
  mAddressee.setCategories( QStringList::split( ",", mCategoryEdit->text() ) );

  mAddressee.setSecrecy( mSecrecyWidget->secrecy() );

  // save custom fields
  if ( !mIMWidget->preferredIM().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-IMAddress", mIMWidget->preferredIM() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-IMAddress" );
  if ( !mSpouseEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-SpousesName", mSpouseEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-SpousesName" );
  if ( !mManagerEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-ManagersName", mManagerEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-ManagersName" );
  if ( !mAssistantEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-AssistantsName", mAssistantEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-AssistantsName" );

  if ( !mDepartmentEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Department", mDepartmentEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Department" );
  if ( !mOfficeEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Office", mOfficeEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Office" );
  if ( !mProfessionEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Profession", mProfessionEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Profession" );

  if ( mAnniversaryPicker->date().isValid() )
    mAddressee.insertCustom( "KADDRESSBOOK", "X-Anniversary",
                             mAnniversaryPicker->date().toString( Qt::ISODate ) );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "X-Anniversary" );

  mAddressee.setTitle( mTitleEdit->text() );

  // Save the email addresses
  mAddressee.setEmails( mEmailWidget->emails() );

  // Save the phone numbers
  KABC::PhoneNumber::List phoneNumbers;
  KABC::PhoneNumber::List::ConstIterator phoneIter;
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
  KABC::Address::List::ConstIterator addressIter;
  addresses = mAddressee.addresses();
  for ( addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter )
    mAddressee.removeAddress( *addressIter );

  addresses = mAddressEditWidget->addresses();
  for ( addressIter = addresses.begin(); addressIter != addresses.end();
        ++addressIter )
    mAddressee.insertAddress( *addressIter );

  QDictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->storeContact( &mAddressee );

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

  mCategorySelectDialog->setSelected( QStringList::split( ",", mCategoryEdit->text() ) );
  mCategorySelectDialog->show();
  mCategorySelectDialog->raise();
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
             mCategorySelectDialog, SLOT( updateCategoryConfig() ) );
  }

  mCategoryEditDialog->show();
  mCategoryEditDialog->raise();
}

void AddresseeEditorWidget::emitModified()
{
  mDirty = true;

  if ( !mBlockSignals ) {
    KABC::Addressee::List list;

    save();
    list.append( mAddressee );

    // I hate these hacks...
    if ( !isExtension() )
      mDirty = true;

    emit modified( list );
  }
}

void AddresseeEditorWidget::dateChanged( const QDate& )
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
  return "contact_editor";
}

void AddresseeEditorWidget::setInitialFocus()
{
  mNameEdit->setFocus();
}

bool AddresseeEditorWidget::readyToClose()
{
  bool ok = true;

  QDate date = mBirthdayPicker->date();
  if ( !date.isValid() && !mBirthdayPicker->currentText().isEmpty() ) {
    KMessageBox::error( this, i18n( "You have to enter a valid birthdate." ) );
    ok = false;
  }

  date = mAnniversaryPicker->date();
  if ( !date.isValid() && !mAnniversaryPicker->currentText().isEmpty() ) {
    KMessageBox::error( this, i18n( "You have to enter a valid anniversary." ) );
    ok = false;
  }

  return ok;
}

void AddresseeEditorWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  mNameEdit->setReadOnly( readOnly );
  mRoleEdit->setReadOnly( readOnly );
  mOrgEdit->setReadOnly( readOnly );
  mPhoneEditWidget->setReadOnly( readOnly );
  mAddressEditWidget->setReadOnly( readOnly );
  mEmailWidget->setReadOnly( readOnly );
  mURLEdit->setReadOnly( readOnly );
  mBlogEdit->setReadOnly( readOnly );
  mIMWidget->setReadOnly( readOnly );
  mCategoryButton->setEnabled( !readOnly );
  mSecrecyWidget->setReadOnly( readOnly );
  mDepartmentEdit->setReadOnly( readOnly );
  mOfficeEdit->setReadOnly( readOnly );
  mProfessionEdit->setReadOnly( readOnly );
  mManagerEdit->setReadOnly( readOnly );
  mAssistantEdit->setReadOnly( readOnly );
  mTitleEdit->setReadOnly( readOnly );
  mNicknameEdit->setReadOnly( readOnly );
  mSpouseEdit->setReadOnly( readOnly );
  mBirthdayPicker->setEnabled( !readOnly );
  mAnniversaryPicker->setEnabled( !readOnly );
  mNoteEdit->setReadOnly( mReadOnly );

  QDictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->setReadOnly( readOnly );
}

#include "addresseeeditorwidget.moc"
