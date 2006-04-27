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

#include <qcheckbox.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <q3textedit.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include <kabc/resource.h>
#include <kabc/stdaddressbook.h>
#include <kacceleratormanager.h>
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
#include <kvbox.h>

#include "addresseditwidget.h"
#include "advancedcustomfields.h"
#include "emaileditwidget.h"
#include "imeditwidget.h"
#include "kabprefs.h"
#include "keywidget.h"
#include "nameeditdialog.h"
#include "phoneeditwidget.h"
#include "secrecywidget.h"

#include "addresseeeditorwidget.h"

AddresseeEditorWidget::AddresseeEditorWidget( QWidget *parent, const char *name )
  : AddresseeEditorBase( parent, name ),
    mBlockSignals( false ), mReadOnly( false )
{
  kDebug(5720) << "AddresseeEditorWidget()" << endl;

  initGUI();
  mCategorySelectDialog = 0;
  mCategoryEditDialog = 0;

  // Load the empty addressee as defaults
  load();

  mDirty = false;
}

AddresseeEditorWidget::~AddresseeEditorWidget()
{
  kDebug(5720) << "~AddresseeEditorWidget()" << endl;
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

  QGridLayout *layout = new QGridLayout( tab1 );
  layout->setMargin( KDialogBase::marginHint() );
  layout->setSpacing( KDialogBase::spacingHint() );

  QLabel *label;
  KSeparator* bar;
  QPushButton *button;

  //////////////////////////////////
  // Upper left group (person info)

  // Person icon
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", K3Icon::Desktop,
                                                      K3Icon::SizeMedium ) );
  layout->addWidget( label, 0, 0, 2, 1);

  // First name
  button = new QPushButton( i18n( "Edit Name..." ), tab1 );
  button->setToolTip( i18n( "Edit the contact's name" ) );
  mNameEdit = new KLineEdit( tab1 );
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
  label = new QLabel( i18nc( "<roleLabel>:", "%1:", KABC::Addressee::roleLabel() ), tab1 );
  mRoleEdit = new KLineEdit( tab1 );
  connect( mRoleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mRoleEdit );
  layout->addWidget( label, 1, 1 );
  layout->addWidget( mRoleEdit, 1, 2 );

  // Organization
  label = new QLabel( i18nc( "<organizationLabel>:", "%1:", KABC::Addressee::organizationLabel() ), tab1 );
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
  bar = new KSeparator( Qt::Horizontal, tab1 );
  layout->addWidget( bar, 4, 0, 1, 3 );

  //////////////////////////////////////
  // Phone numbers (upper right)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kaddressbook",
                    K3Icon::Desktop, K3Icon::SizeMedium ) );
  layout->addWidget( label, 0, 3, 2, 1 );

  mPhoneEditWidget = new PhoneEditWidget( tab1 );
  connect( mPhoneEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addWidget( mPhoneEditWidget, 0, 4, 4, 3 );

  bar = new KSeparator( Qt::Horizontal, tab1 );
  layout->addWidget( bar, 4, 3, 1, 4 );

  //////////////////////////////////////
  // Addresses (lower left)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kfm_home", K3Icon::Desktop,
                                                     K3Icon::SizeMedium ) );
  layout->addWidget( label, 5, 0, 2, 1);

  mAddressEditWidget = new AddressEditWidget( tab1 );
  connect( mAddressEditWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addWidget( mAddressEditWidget, 5, 1, 6, 2 );

  //////////////////////////////////////
  // Email / Web (lower right)
  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "email", K3Icon::Desktop,
                                                     K3Icon::SizeMedium ) );
  layout->addWidget( label, 5, 3, 2, 1);

  mEmailWidget = new EmailEditWidget( tab1 );
  connect( mEmailWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addWidget( mEmailWidget, 5, 4, 2, 3 );

  // add the separator
  bar = new KSeparator( Qt::Horizontal, tab1 );
  layout->addWidget( bar, 7, 3, 1, 4 );

  QHBoxLayout *homePageLayout = new QHBoxLayout();
  homePageLayout->setSpacing( 7 );
  homePageLayout->setMargin( 11 );

  label = new QLabel( tab1 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "homepage", K3Icon::Desktop,
                                                     K3Icon::SizeMedium ) );
  homePageLayout->addWidget( label );

  label = new QLabel( i18nc( "<urlLabel>:", "%1:", KABC::Addressee::urlLabel() ), tab1 );
  mURLEdit = new KLineEdit( tab1 );
  connect( mURLEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mURLEdit );
  homePageLayout->addWidget( label );
  homePageLayout->addWidget( mURLEdit );
  layout->addLayout( homePageLayout, 8, 3, 1, 4 );

  QHBoxLayout *blogLayout = new QHBoxLayout();
  blogLayout->setSpacing( 7 );
  blogLayout->setMargin( 11 );
  label = new QLabel( i18n( "Blog feed:" ), tab1 );
  blogLayout->addWidget( label );
  mBlogEdit = new KLineEdit( tab1 );
  blogLayout->addWidget( mBlogEdit );
  connect( mBlogEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( textChanged( const QString & ) ) );
  label->setBuddy( mBlogEdit );
  layout->addLayout( blogLayout, 9, 4, 1, 3 );

  mIMWidget = new IMEditWidget( tab1, mAddressee );
  connect( mIMWidget, SIGNAL( modified() ), SLOT( emitModified() ) );
  layout->addWidget( mIMWidget, 10, 4, 1, 3 );

  layout->addItem( new QSpacerItem( 50, 0 ), 0, 6 );

  bar = new KSeparator( Qt::Horizontal, tab1 );
  layout->addWidget( bar, 11, 0, 1, 7 );

  ///////////////////////////////////////
  KHBox *categoryBox = new KHBox( tab1 );
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

  layout->addWidget( categoryBox, 12, 0, 1, 7 );

  // Build the layout and add to the tab widget
  layout->activate(); // required

  mTabWidget->addTab( tab1, i18n( "&General" ) );
}

void AddresseeEditorWidget::setupTab2()
{
  // This is the Details tab
  QWidget *tab2 = new QWidget( mTabWidget );

  QGridLayout *layout = new QGridLayout( tab2 );
  layout->setMargin( KDialogBase::marginHint() );
  layout->setSpacing( KDialogBase::spacingHint() );

  QLabel *label;
  KSeparator* bar;

  ///////////////////////
  // Office info

  // Department
  label = new QLabel( tab2 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "folder", K3Icon::Desktop,
                                                     K3Icon::SizeMedium ) );
  layout->addWidget( label, 0, 0, 2, 1 );

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
  layout->addWidget( mManagerEdit, 0, 4, 1, 2 );

  label = new QLabel( i18n( "Assistant's name:" ), tab2 );
  layout->addWidget( label, 1, 3 );
  mAssistantEdit = new KPIM::AddresseeLineEdit( tab2 );
  connect( mAssistantEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mAssistantEdit );
  layout->addWidget( mAssistantEdit, 1, 4, 1, 2 );

  label = new QLabel( i18nc( "<titleLabel>:", "%1:", KABC::Addressee::titleLabel() ), tab2 );
  layout->addWidget( label, 2, 3 );
  mTitleEdit = new KLineEdit( tab2 );
  connect( mTitleEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mTitleEdit );
  layout->addWidget( mTitleEdit, 2, 4, 1, 2 );

  bar = new KSeparator( Qt::Horizontal, tab2 );
  layout->addWidget( bar, 3, 0, 1, 6 );

  /////////////////////////////////////////////////
  // Personal info

  label = new QLabel( tab2 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "personal", K3Icon::Desktop,
                                                     K3Icon::SizeMedium ) );
  layout->addWidget( label, 4, 0, 2, 1);

  label = new QLabel( i18n( "Nickname:" ), tab2 );
  layout->addWidget( label, 4, 1 );
  mNicknameEdit = new KLineEdit( tab2 );
  connect( mNicknameEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  label->setBuddy( mNicknameEdit );
  layout->addWidget( mNicknameEdit, 4, 2 );

  label = new QLabel( i18n( "Partner's name:" ), tab2 );
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

  bar = new KSeparator( Qt::Horizontal, tab2 );
  layout->addWidget( bar, 6, 0, 1, 6 );

   //////////////////////////////////////
  // Notes
  label = new QLabel( i18n( "Note:" ), tab2 );
  label->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  layout->addWidget( label, 7, 0 );
  mNoteEdit = new Q3TextEdit( tab2 );
  mNoteEdit->setWordWrap( Q3TextEdit::WidgetWidth );
  mNoteEdit->setMinimumSize( mNoteEdit->sizeHint() );
  connect( mNoteEdit, SIGNAL( textChanged() ), SLOT( emitModified() ) );
  label->setBuddy( mNoteEdit );
  layout->addWidget( mNoteEdit, 7, 1, 1, 5 );

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
              = manager->factory( i )->createWidget( KABC::StdAddressBook::self( true ),
                                                     page );
    if ( widget )
      page->addWidget( widget );
  }

  // query the layout update
  Q3DictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->updateLayout();
}

void AddresseeEditorWidget::setupCustomFieldsTabs()
{
  QStringList activePages = KABPrefs::instance()->advancedCustomFields();

  const QStringList list = KGlobal::dirs()->findAllResources( "data", "kaddressbook/contacteditorpages/*.ui", true, true );
  for ( QStringList::ConstIterator it = list.begin(); it != list.end(); ++it ) {
    if ( activePages.find( (*it).mid( (*it).lastIndexOf('/') + 1 ) ) == activePages.end() )
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
  kDebug(5720) << "AddresseeEditorWidget::load()" << endl;

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

  Q3DictIterator<ContactEditorTabPage> it( mTabPages );
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
  mAddressee.setUrl( KUrl( mURLEdit->text().trimmed() ) );
  if ( !mBlogEdit->text().isEmpty() )
    mAddressee.insertCustom( "KADDRESSBOOK", "BlogFeed", mBlogEdit->text() );
  else
    mAddressee.removeCustom( "KADDRESSBOOK", "BlogFeed" );

  mAddressee.setNote( mNoteEdit->text() );
  if ( mBirthdayPicker->date().isValid() )
    mAddressee.setBirthday( QDateTime( mBirthdayPicker->date() ) );
  else
    mAddressee.setBirthday( QDateTime() );

  mAddressee.setNickName( mNicknameEdit->text() );
  mAddressee.setCategories( mCategoryEdit->text().split( ",", QString::SkipEmptyParts ) );

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

  Q3DictIterator<ContactEditorTabPage> it( mTabPages );
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

  mAddressEditWidget->updateAddressee( mAddressee );

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

  mAddressEditWidget->updateAddressee( mAddressee );
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

  mCategorySelectDialog->setSelected( mCategoryEdit->text().split( ",", QString::SkipEmptyParts) );
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

void AddresseeEditorWidget::pageChanged( QWidget *wdg )
{
  if ( wdg )
    KAcceleratorManager::manage( wdg );
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

  Q3DictIterator<ContactEditorTabPage> it( mTabPages );
  for ( ; it.current(); ++it )
    it.current()->setReadOnly( readOnly );
}

#include "addresseeeditorwidget.moc"
