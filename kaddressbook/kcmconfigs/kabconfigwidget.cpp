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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qtooltip.h>
#include <qcombobox.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktrader.h>

#include "addresseewidget.h"
#include "extensionconfigdialog.h"
#include "extensionwidget.h"
#include "kabprefs.h"

#include "kabconfigwidget.h"

class ExtensionItem : public QCheckListItem
{
  public:
    ExtensionItem( QListView *parent, const QString &text );

    void setService( const KService::Ptr &ptr );
    bool configWidgetAvailable() const;
    KAB::ExtensionFactory *factory() const;

    virtual QString text( int column ) const;

  private:
    KService::Ptr mPtr;
};

KABConfigWidget::KABConfigWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, 0,
                                            KDialog::spacingHint() );

  QTabWidget *tabWidget = new QTabWidget( this );
  topLayout->addWidget( tabWidget );

  // General page
  QWidget *generalPage = new QWidget( this );
  QVBoxLayout *layout = new QVBoxLayout( generalPage, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QGroupBox *groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "General" ), generalPage );
  QBoxLayout *boxLayout = new QVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), groupBox, "msingle" );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox, "mparse" );
  boxLayout->addWidget( mNameParsing );

  mTradeAsFamilyName = new QCheckBox( i18n( "Trade single name component as family name" ), groupBox, "mtrade" );
  boxLayout->addWidget( mTradeAsFamilyName );
/**
  TODO: show the checkbox when we can compile agains kdelibs from HEAD, atm it 
        doesn't work and would just confuse the users ;)
*/
  mTradeAsFamilyName->hide();

  QBoxLayout *editorLayout = new QHBoxLayout( boxLayout, KDialog::spacingHint() );
  
  QLabel *label = new QLabel( i18n( "Addressee Editor Type:" ), groupBox );
  editorLayout->addWidget( label );
  
  mEditorCombo = new QComboBox( groupBox );
  mEditorCombo->insertItem( i18n( "Full Editor" ) );
  mEditorCombo->insertItem( i18n( "Simple Editor" ) );
  label->setBuddy( mEditorCombo );
  editorLayout->addWidget( mEditorCombo );

  editorLayout->addStretch( 1 );


  layout->addWidget( groupBox );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Script-Hooks" ), generalPage );
  QGridLayout *grid = new QGridLayout( groupBox->layout(), 2, 2,
                                       KDialog::spacingHint() );
  label = new QLabel( i18n( "Phone:" ), groupBox );
  grid->addWidget( label, 0, 0 );

  mPhoneHook = new QLineEdit( groupBox );
  QToolTip::add( mPhoneHook, i18n( "<ul><li>%N: Phone Number</li></ul>" ) );
  grid->addWidget( mPhoneHook, 0, 1 );

  label = new QLabel( i18n( "Fax:" ), groupBox );
  grid->addWidget( label, 1, 0 );

  mFaxHook = new QLineEdit( groupBox );
  QToolTip::add( mFaxHook, i18n( "<ul><li>%N: Fax Number</li></ul>" ) );
  grid->addWidget( mFaxHook, 1, 1 );
  grid->setColStretch( 1, 1 );

  layout->addWidget( groupBox );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Extensions" ), generalPage );
  boxLayout = new QVBoxLayout( groupBox->layout(), KDialog::spacingHint() );
  boxLayout->setAlignment( Qt::AlignTop );

  mExtensionView = new KListView( groupBox );
  mExtensionView->setAllColumnsShowFocus( true );
  mExtensionView->setFullWidth( true );
  mExtensionView->addColumn( i18n( "Name" ) );
  mExtensionView->addColumn( i18n( "Description" ) );
  boxLayout->addWidget( mExtensionView );
  connect( mExtensionView, SIGNAL( doubleClicked ( QListViewItem* ) ),
           this, SLOT( configureExtension( QListViewItem* ) ) );
  QHBoxLayout *hboxLayout = new QHBoxLayout( boxLayout, KDialog::spacingHint() );
  hboxLayout->addStretch( 1 );
  mConfigureButton = new QPushButton( i18n( "Configure..." ), groupBox );
  mConfigureButton->setEnabled( false );
  hboxLayout->addWidget( mConfigureButton );

  layout->addWidget( groupBox );

  connect( mNameParsing, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mViewsSingleClickBox, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mTradeAsFamilyName, SIGNAL( toggled( bool ) ), SLOT( modified() ) );
  connect( mPhoneHook, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mFaxHook, SIGNAL( textChanged( const QString& ) ), SLOT( modified() ) );
  connect( mExtensionView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( selectionChanged( QListViewItem* ) ) );
  connect( mExtensionView, SIGNAL( clicked( QListViewItem* ) ),
           SLOT( itemClicked( QListViewItem* ) ) );
  connect( mConfigureButton, SIGNAL( clicked() ),
           SLOT( configureExtension() ) );
  connect( mEditorCombo, SIGNAL( activated( int ) ), SLOT( modified() ) );

  tabWidget->addTab( generalPage, i18n( "General" ) );

  // Addressee page
  mAddresseeWidget = new AddresseeWidget( this );
  tabWidget->addTab( mAddresseeWidget, i18n( "Contact" ) );
  connect( mAddresseeWidget, SIGNAL( modified() ), SLOT( modified() ) );
}

void KABConfigWidget::configureExtension(QListViewItem *i)
{
    ExtensionItem *item = static_cast<ExtensionItem*>( i );
    if ( !item )
        return;
    if ( item->configWidgetAvailable() )
        configureExtension();
}

void KABConfigWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->mAutomaticNameParsing );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );
  mPhoneHook->setText( KABPrefs::instance()->mPhoneHookApplication );
  mFaxHook->setText( KABPrefs::instance()->mFaxHookApplication );
  mAddresseeWidget->restoreSettings();
  mEditorCombo->setCurrentItem( KABPrefs::instance()->mEditorType );

  restoreExtensionSettings();

  KConfig config( "kabcrc", false, false );
  config.setGroup( "General" );
  mTradeAsFamilyName->setChecked( config.readBoolEntry( "TradeAsFamilyName", true ) );

  blockSignals( blocked );

  emit changed( false );
}

void KABConfigWidget::saveSettings()
{
  KABPrefs::instance()->mAutomaticNameParsing = mNameParsing->isChecked();
  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();
  KABPrefs::instance()->mPhoneHookApplication = mPhoneHook->text();
  KABPrefs::instance()->mFaxHookApplication = mFaxHook->text();
  KABPrefs::instance()->mEditorType = mEditorCombo->currentItem();
  mAddresseeWidget->saveSettings();

  saveExtensionSettings();
  KABPrefs::instance()->writeConfig();

  KConfig config( "kabcrc", false, false );
  config.setGroup( "General" );
  config.writeEntry( "TradeAsFamilyName", mTradeAsFamilyName->isChecked() );

  emit changed( false );
}

void KABConfigWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );
  mEditorCombo->setCurrentItem( 0 );

  emit changed( true );
}

void KABConfigWidget::modified()
{
  emit changed( true );
}

void KABConfigWidget::restoreExtensionSettings()
{
  QStringList activeExtensions = KABPrefs::instance()->mActiveExtensions;

  mExtensionView->clear();

  KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/Extension" );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( !(*it)->hasServiceType( "KAddressBook/Extension" ) )
      continue;

    ExtensionItem *item = new ExtensionItem( mExtensionView, (*it)->name() );
    item->setService( *it );
    if ( activeExtensions.contains( item->factory()->identifier() ) )
      item->setOn( true );
  }
}

void KABConfigWidget::saveExtensionSettings()
{
  QStringList activeExtensions;

  QPtrList<QListViewItem> list;
  QListViewItemIterator it( mExtensionView );
  while ( it.current() ) {
    ExtensionItem *item = static_cast<ExtensionItem*>( it.current() );
    if ( item ) {
      if ( item->isOn() )
        activeExtensions.append( item->factory()->identifier() );
    }
    ++it;
  }

  KABPrefs::instance()->mActiveExtensions = activeExtensions;
}

void KABConfigWidget::configureExtension()
{
  ExtensionItem *item = static_cast<ExtensionItem*>( mExtensionView->currentItem() );
  if ( !item )
    return;

  KConfig config( "kaddressbookrc" );
  config.setGroup( QString( "Extensions_%1" ).arg( item->factory()->identifier() ) );

  ExtensionConfigDialog dlg( item->factory(), &config, this );
  dlg.exec();

  config.sync();
}

void KABConfigWidget::selectionChanged( QListViewItem *i )
{
  ExtensionItem *item = static_cast<ExtensionItem*>( i );
  if ( !item )
    return;

  mConfigureButton->setEnabled( item->configWidgetAvailable() );
}

void KABConfigWidget::itemClicked( QListViewItem *item )
{
  if ( item != 0 )
    modified();
}



ExtensionItem::ExtensionItem( QListView *parent, const QString &text )
  : QCheckListItem( parent, text, CheckBox )
{
}

void ExtensionItem::setService( const KService::Ptr &ptr )
{
  mPtr = ptr;
}

bool ExtensionItem::configWidgetAvailable() const
{
  KLibFactory *factory = KLibLoader::self()->factory( mPtr->library().latin1() );
  if ( !factory )
    return false;

  KAB::ExtensionFactory *extensionFactory = static_cast<KAB::ExtensionFactory*>( factory );
  if ( !extensionFactory )
    return false;

  return extensionFactory->configureWidgetAvailable();
}

KAB::ExtensionFactory *ExtensionItem::factory() const
{
  KLibFactory *factory = KLibLoader::self()->factory( mPtr->library().latin1() );
  if ( !factory )
    return 0;

  return static_cast<KAB::ExtensionFactory*>( factory );
}

QString ExtensionItem::text( int column ) const
{
  if ( column == 0 )
    return mPtr->name();
  else if ( column == 1 )
    return mPtr->comment();
  else
    return QString::null;
}

#include "kabconfigwidget.moc"
