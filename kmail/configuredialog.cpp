/*   -*- mode: C++; c-file-style: "gnu" -*-
 *   kmail: KDE mail client
 *   This file: Copyright (C) 2000 Espen Sand, espen@kde.org
 *              Copyright (C) 2001-2003 Marc Mutz, mutz@kde.org
 *   Contains code segments and ideas from earlier kmail dialog code.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// This must be first
#include <config.h>

// my headers:
#include "configuredialog.h"
#include "configuredialog_p.h"

#include "globalsettings.h"
#include "replyphrases.h"
#include "templatesconfiguration_kfg.h"

// other KMail headers:
#include "kmkernel.h"
#include "simplestringlisteditor.h"
#include "accountdialog.h"
using KMail::AccountDialog;
#include "colorlistbox.h"
#include "kmacctseldlg.h"
#include "messagesender.h"
#include "kmtransport.h"
#include "kmfoldermgr.h"
#include <libkpimidentities/identitymanager.h>
#include "identitylistview.h"
using KMail::IdentityListView;
using KMail::IdentityListViewItem;
#include "kcursorsaver.h"
#include "accountmanager.h"
#include <composercryptoconfiguration.h>
#include <warningconfiguration.h>
#include <smimeconfiguration.h>
#include "templatesconfiguration.h"
#include "customtemplates.h"
#include "folderrequester.h"
using KMail::FolderRequester;
#include "accountcombobox.h"
#include "imapaccountbase.h"
using KMail::ImapAccountBase;
#include "folderstorage.h"
#include "kmfolder.h"
#include "kmmainwidget.h"
#include "recentaddresses.h"
using KRecentAddress::RecentAddresses;
#include "completionordereditor.h"
#include "ldapclient.h"
#include "index.h"

using KMail::IdentityListView;
using KMail::IdentityListViewItem;
#include "identitydialog.h"
using KMail::IdentityDialog;

// other kdenetwork headers:
#include <libkpimidentities/identity.h>
#include <kmime_util.h>
using KMime::DateFormatter;
#include <kleo/cryptoconfig.h>
#include <kleo/cryptobackendfactory.h>
#include <ui/backendconfigwidget.h>
#include <ui/keyrequester.h>
#include <ui/keyselectiondialog.h>

// other KDE headers:
#include <klocale.h>
#include <kapplication.h>
#include <kcharsets.h>
#include <kasciistringtools.h>
#include <kdebug.h>
#include <knuminput.h>
#include <kfontdialog.h>
#include <kmessagebox.h>
#include <kurlrequester.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kwin.h>
#include <knotifydialog.h>
#include <kconfig.h>
#include <kactivelabel.h>
#include <kcmultidialog.h>
#include <kcombobox.h>

// Qt headers:
#include <tqvalidator.h>
#include <tqwhatsthis.h>
#include <tqvgroupbox.h>
#include <tqvbox.h>
#include <tqvbuttongroup.h>
#include <tqhbuttongroup.h>
#include <tqtooltip.h>
#include <tqlabel.h>
#include <tqtextcodec.h>
#include <tqheader.h>
#include <tqpopupmenu.h>
#include <tqradiobutton.h>
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqwidgetstack.h>

// other headers:
#include <assert.h>
#include <stdlib.h>

#ifndef _PATH_SENDMAIL
#define _PATH_SENDMAIL  "/usr/sbin/sendmail"
#endif

#ifdef DIM
#undef DIM
#endif
#define DIM(x) sizeof(x) / sizeof(*x)

namespace {

  struct EnumConfigEntryItem {
    const char * key; // config key value, as appears in config file
    const char * desc; // description, to be i18n()ized
  };
  struct EnumConfigEntry {
    const char * group;
    const char * key;
    const char * desc;
    const EnumConfigEntryItem * items;
    int numItems;
    int defaultItem;
  };
  struct BoolConfigEntry {
    const char * group;
    const char * key;
    const char * desc;
    bool defaultValue;
  };

  static const char * lockedDownWarning =
    I18N_NOOP("<qt><p>This setting has been fixed by your administrator.</p>"
              "<p>If you think this is an error, please contact him.</p></qt>");

  void checkLockDown( TQWidget * w, const KConfigBase & c, const char * key ) {
    if ( c.entryIsImmutable( key ) ) {
      w->setEnabled( false );
      TQToolTip::add( w, i18n( lockedDownWarning ) );
    } else {
      TQToolTip::remove( w );
    }
  }

  void populateButtonGroup( TQButtonGroup * g, const EnumConfigEntry & e ) {
    g->setTitle( i18n( e.desc ) );
    g->layout()->setSpacing( KDialog::spacingHint() );
    for ( int i = 0 ; i < e.numItems ; ++i )
      g->insert( new TQRadioButton( i18n( e.items[i].desc ), g ), i );
  }

  void populateCheckBox( TQCheckBox * b, const BoolConfigEntry & e ) {
    b->setText( i18n( e.desc ) );
  }

  void loadWidget( TQCheckBox * b, const KConfigBase & c, const BoolConfigEntry & e ) {
    Q_ASSERT( c.group() == e.group );
    checkLockDown( b, c, e.key );
    b->setChecked( c.readBoolEntry( e.key, e.defaultValue ) );
  }

  void loadWidget( TQButtonGroup * g, const KConfigBase & c, const EnumConfigEntry & e ) {
    Q_ASSERT( c.group() == e.group );
    Q_ASSERT( g->count() == e.numItems );
    checkLockDown( g, c, e.key );
    const TQString s = c.readEntry( e.key, e.items[e.defaultItem].key );
    for ( int i = 0 ; i < e.numItems ; ++i )
      if ( s == e.items[i].key ) {
        g->setButton( i );
        return;
      }
    g->setButton( e.defaultItem );
  }

  void saveCheckBox( TQCheckBox * b, KConfigBase & c, const BoolConfigEntry & e ) {
    Q_ASSERT( c.group() == e.group );
    c.writeEntry( e.key, b->isChecked() );
  }

  void saveButtonGroup( TQButtonGroup * g, KConfigBase & c, const EnumConfigEntry & e ) {
    Q_ASSERT( c.group() == e.group );
    Q_ASSERT( g->count() == e.numItems );
    c.writeEntry( e.key, e.items[ g->id( g->selected() ) ].key );
  }

  template <typename T_Widget, typename T_Entry>
  inline void loadProfile( T_Widget * g, const KConfigBase & c, const T_Entry & e ) {
    if ( c.hasKey( e.key ) )
      loadWidget( g, c, e );
  }
}


ConfigureDialog::ConfigureDialog( TQWidget *parent, const char *name, bool modal )
  : KCMultiDialog( KDialogBase::IconList, KGuiItem( i18n( "&Load Profile..." ) ),
                   KGuiItem(), User2, i18n( "Configure" ), parent, name, modal )
  , mProfileDialog( 0 )
{
  KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
  showButton( User1, true );

  addModule ( "kmail_config_identity", false );
  addModule ( "kmail_config_accounts", false );
  addModule ( "kmail_config_appearance", false );
  addModule ( "kmail_config_composer", false );
  addModule ( "kmail_config_security", false );
  addModule ( "kmail_config_misc", false );

  // We store the size of the dialog on hide, because otherwise
  // the KCMultiDialog starts with the size of the first kcm, not
  // the largest one. This way at least after the first showing of
  // the largest kcm the size is kept.
  KConfigGroup geometry( KMKernel::config(), "Geometry" );
  int width = geometry.readNumEntry( "ConfigureDialogWidth" );
  int height = geometry.readNumEntry( "ConfigureDialogHeight" );
  if ( width != 0 && height != 0 ) {
     setMinimumSize( width, height );
  }

}

void ConfigureDialog::hideEvent( TQHideEvent *ev ) {
  KConfigGroup geometry( KMKernel::config(), "Geometry" );
  geometry.writeEntry( "ConfigureDialogWidth", width() );
  geometry.writeEntry( "ConfigureDialogHeight",height() );
  KDialogBase::hideEvent( ev );
}

ConfigureDialog::~ConfigureDialog() {
}

void ConfigureDialog::slotApply() {
  KCMultiDialog::slotApply();
  GlobalSettings::self()->writeConfig();
  emit configChanged();
}

void ConfigureDialog::slotOk() {
  KCMultiDialog::slotOk();
  GlobalSettings::self()->writeConfig();
  emit configChanged();
}

void ConfigureDialog::slotUser2() {
  if ( mProfileDialog ) {
    mProfileDialog->raise();
    return;
  }
  mProfileDialog = new ProfileDialog( this, "mProfileDialog" );
  connect( mProfileDialog, TQT_SIGNAL(profileSelected(KConfig*)),
                this, TQT_SIGNAL(installProfile(KConfig*)) );
  mProfileDialog->show();
}

// *************************************************************
// *                                                           *
// *                      IdentityPage                         *
// *                                                           *
// *************************************************************
TQString IdentityPage::helpAnchor() const {
  return TQString::fromLatin1("configure-identity");
}

IdentityPage::IdentityPage( TQWidget * parent, const char * name )
  : ConfigModule( parent, name ),
    mIdentityDialog( 0 )
{
  TQHBoxLayout * hlay = new TQHBoxLayout( this, 0, KDialog::spacingHint() );

  mIdentityList = new IdentityListView( this );
  connect( mIdentityList, TQT_SIGNAL(selectionChanged()),
           TQT_SLOT(slotIdentitySelectionChanged()) );
  connect( mIdentityList, TQT_SIGNAL(itemRenamed(TQListViewItem*,const TQString&,int)),
           TQT_SLOT(slotRenameIdentity(TQListViewItem*,const TQString&,int)) );
  connect( mIdentityList, TQT_SIGNAL(doubleClicked(TQListViewItem*,const TQPoint&,int)),
           TQT_SLOT(slotModifyIdentity()) );
  connect( mIdentityList, TQT_SIGNAL(contextMenu(KListView*,TQListViewItem*,const TQPoint&)),
           TQT_SLOT(slotContextMenu(KListView*,TQListViewItem*,const TQPoint&)) );
  // ### connect dragged(...), ...

  hlay->addWidget( mIdentityList, 1 );

  TQVBoxLayout * vlay = new TQVBoxLayout( hlay ); // inherits spacing

  TQPushButton * button = new TQPushButton( i18n("&Add..."), this );
  mModifyButton = new TQPushButton( i18n("&Modify..."), this );
  mRenameButton = new TQPushButton( i18n("&Rename"), this );
  mRemoveButton = new TQPushButton( i18n("Remo&ve"), this );
  mSetAsDefaultButton = new TQPushButton( i18n("Set as &Default"), this );
  button->setAutoDefault( false );
  mModifyButton->setAutoDefault( false );
  mModifyButton->setEnabled( false );
  mRenameButton->setAutoDefault( false );
  mRenameButton->setEnabled( false );
  mRemoveButton->setAutoDefault( false );
  mRemoveButton->setEnabled( false );
  mSetAsDefaultButton->setAutoDefault( false );
  mSetAsDefaultButton->setEnabled( false );
  connect( button, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotNewIdentity()) );
  connect( mModifyButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotModifyIdentity()) );
  connect( mRenameButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRenameIdentity()) );
  connect( mRemoveButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRemoveIdentity()) );
  connect( mSetAsDefaultButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotSetAsDefault()) );
  vlay->addWidget( button );
  vlay->addWidget( mModifyButton );
  vlay->addWidget( mRenameButton );
  vlay->addWidget( mRemoveButton );
  vlay->addWidget( mSetAsDefaultButton );
  vlay->addStretch( 1 );
  load();
}

void IdentityPage::load()
{
  KPIM::IdentityManager * im = kmkernel->identityManager();
  mOldNumberOfIdentities = im->shadowIdentities().count();
  // Fill the list:
  mIdentityList->clear();
  TQListViewItem * item = 0;
  for ( KPIM::IdentityManager::Iterator it = im->modifyBegin() ; it != im->modifyEnd() ; ++it )
    item = new IdentityListViewItem( mIdentityList, item, *it  );
  mIdentityList->setSelected( mIdentityList->currentItem(), true );
}

void IdentityPage::save() {
  assert( !mIdentityDialog );

  kmkernel->identityManager()->sort();
  kmkernel->identityManager()->commit();

  if( mOldNumberOfIdentities < 2 && mIdentityList->childCount() > 1 ) {
    // have more than one identity, so better show the combo in the
    // composer now:
    KConfigGroup composer( KMKernel::config(), "Composer" );
    int showHeaders = composer.readNumEntry( "headers", HDR_STANDARD );
    showHeaders |= HDR_IDENTITY;
    composer.writeEntry( "headers", showHeaders );
  }
  // and now the reverse
  if( mOldNumberOfIdentities > 1 && mIdentityList->childCount() < 2 ) {
    // have only one identity, so remove the combo in the composer:
    KConfigGroup composer( KMKernel::config(), "Composer" );
    int showHeaders = composer.readNumEntry( "headers", HDR_STANDARD );
    showHeaders &= ~HDR_IDENTITY;
    composer.writeEntry( "headers", showHeaders );
  }
}

void IdentityPage::slotNewIdentity()
{
  assert( !mIdentityDialog );

  KPIM::IdentityManager * im = kmkernel->identityManager();
  NewIdentityDialog dialog( im->shadowIdentities(), this, "new", true );

  if( dialog.exec() == TQDialog::Accepted ) {
    TQString identityName = dialog.identityName().stripWhiteSpace();
    assert( !identityName.isEmpty() );

    //
    // Construct a new Identity:
    //
    switch ( dialog.duplicateMode() ) {
    case NewIdentityDialog::ExistingEntry:
      {
        KPIM::Identity & dupThis = im->modifyIdentityForName( dialog.duplicateIdentity() );
        im->newFromExisting( dupThis, identityName );
        break;
      }
    case NewIdentityDialog::ControlCenter:
      im->newFromControlCenter( identityName );
      break;
    case NewIdentityDialog::Empty:
      im->newFromScratch( identityName );
    default: ;
    }

    //
    // Insert into listview:
    //
    KPIM::Identity & newIdent = im->modifyIdentityForName( identityName );
    TQListViewItem * item = mIdentityList->selectedItem();
    if ( item )
      item = item->itemAbove();
    mIdentityList->setSelected( new IdentityListViewItem( mIdentityList,
                                                          /*after*/ item,
                                                          newIdent ), true );
    slotModifyIdentity();
  }
}

void IdentityPage::slotModifyIdentity() {
  assert( !mIdentityDialog );

  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;

  mIdentityDialog = new IdentityDialog( this );
  mIdentityDialog->setIdentity( item->identity() );

  // Hmm, an unmodal dialog would be nicer, but a modal one is easier ;-)
  if ( mIdentityDialog->exec() == TQDialog::Accepted ) {
    mIdentityDialog->updateIdentity( item->identity() );
    item->redisplay();
    emit changed(true);
  }

  delete mIdentityDialog;
  mIdentityDialog = 0;
}

void IdentityPage::slotRemoveIdentity()
{
  assert( !mIdentityDialog );

  KPIM::IdentityManager * im = kmkernel->identityManager();
  kdFatal( im->shadowIdentities().count() < 2 )
    << "Attempted to remove the last identity!" << endl;

  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;

  TQString msg = i18n("<qt>Do you really want to remove the identity named "
                     "<b>%1</b>?</qt>").arg( item->identity().identityName() );
  if( KMessageBox::warningContinueCancel( this, msg, i18n("Remove Identity"),
   KGuiItem(i18n("&Remove"),"editdelete") ) == KMessageBox::Continue )
    if ( im->removeIdentity( item->identity().identityName() ) ) {
      delete item;
      mIdentityList->setSelected( mIdentityList->currentItem(), true );
      refreshList();
    }
}

void IdentityPage::slotRenameIdentity() {
  assert( !mIdentityDialog );

  TQListViewItem * item = mIdentityList->selectedItem();
  if ( !item ) return;

  mIdentityList->rename( item, 0 );
}

void IdentityPage::slotRenameIdentity( TQListViewItem * i,
                                       const TQString & s, int col ) {
  assert( col == 0 );
  Q_UNUSED( col );

  IdentityListViewItem * item = dynamic_cast<IdentityListViewItem*>( i );
  if ( !item ) return;

  TQString newName = s.stripWhiteSpace();
  if ( !newName.isEmpty() &&
       !kmkernel->identityManager()->shadowIdentities().contains( newName ) ) {
    KPIM::Identity & ident = item->identity();
    ident.setIdentityName( newName );
    emit changed(true);
  }
  item->redisplay();
}

void IdentityPage::slotContextMenu( KListView *, TQListViewItem * i,
                                    const TQPoint & pos ) {
  IdentityListViewItem * item = dynamic_cast<IdentityListViewItem*>( i );

  TQPopupMenu * menu = new TQPopupMenu( this );
  menu->insertItem( i18n("Add..."), this, TQT_SLOT(slotNewIdentity()) );
  if ( item ) {
    menu->insertItem( i18n("Modify..."), this, TQT_SLOT(slotModifyIdentity()) );
    if ( mIdentityList->childCount() > 1 )
      menu->insertItem( i18n("Remove"), this, TQT_SLOT(slotRemoveIdentity()) );
    if ( !item->identity().isDefault() )
      menu->insertItem( i18n("Set as Default"), this, TQT_SLOT(slotSetAsDefault()) );
  }
  menu->exec( pos );
  delete menu;
}


void IdentityPage::slotSetAsDefault() {
  assert( !mIdentityDialog );

  IdentityListViewItem * item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );
  if ( !item ) return;

  KPIM::IdentityManager * im = kmkernel->identityManager();
  im->setAsDefault( item->identity().identityName() );
  refreshList();
}

void IdentityPage::refreshList() {
  for ( TQListViewItemIterator it( mIdentityList ) ; it.current() ; ++it ) {
    IdentityListViewItem * item =
      dynamic_cast<IdentityListViewItem*>(it.current());
    if ( item )
      item->redisplay();
  }
  emit changed(true);
}

void IdentityPage::slotIdentitySelectionChanged()
{
  IdentityListViewItem *item =
    dynamic_cast<IdentityListViewItem*>( mIdentityList->selectedItem() );

  mRemoveButton->setEnabled( item && mIdentityList->childCount() > 1 );
  mModifyButton->setEnabled( item );
  mRenameButton->setEnabled( item );
  mSetAsDefaultButton->setEnabled( item && !item->identity().isDefault() );
}

void IdentityPage::slotUpdateTransportCombo( const TQStringList & sl )
{
  if ( mIdentityDialog ) mIdentityDialog->slotUpdateTransportCombo( sl );
}



// *************************************************************
// *                                                           *
// *                       AccountsPage                         *
// *                                                           *
// *************************************************************
TQString AccountsPage::helpAnchor() const {
  return TQString::fromLatin1("configure-accounts");
}

AccountsPage::AccountsPage( TQWidget * parent, const char * name )
  : ConfigModuleWithTabs( parent, name )
{
  //
  // "Receiving" tab:
  //
  mReceivingTab = new ReceivingTab();
  addTab( mReceivingTab, i18n( "&Receiving" ) );
  connect( mReceivingTab, TQT_SIGNAL(accountListChanged(const TQStringList &)),
           this, TQT_SIGNAL(accountListChanged(const TQStringList &)) );

  //
  // "Sending" tab:
  //
  mSendingTab = new SendingTab();
  addTab( mSendingTab, i18n( "&Sending" ) );
  connect( mSendingTab, TQT_SIGNAL(transportListChanged(const TQStringList&)),
           this, TQT_SIGNAL(transportListChanged(const TQStringList&)) );

  load();
}

TQString AccountsPage::SendingTab::helpAnchor() const {
  return TQString::fromLatin1("configure-accounts-sending");
}

AccountsPageSendingTab::AccountsPageSendingTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  mTransportInfoList.setAutoDelete( true );
  // temp. vars:
  TQVBoxLayout *vlay;
  TQVBoxLayout *btn_vlay;
  TQHBoxLayout *hlay;
  TQGridLayout *glay;
  TQPushButton *button;
  TQGroupBox   *group;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  // label: zero stretch ### FIXME more
  vlay->addWidget( new TQLabel( i18n("Outgoing accounts (add at least one):"), this ) );

  // hbox layout: stretch 10, spacing inherited from vlay
  hlay = new TQHBoxLayout();
  vlay->addLayout( hlay, 10 ); // high stretch b/c of the groupbox's sizeHint

  // transport list: left widget in hlay; stretch 1
  // ### FIXME: allow inline renaming of the account:
  mTransportList = new ListView( this, "transportList", 5 );
  mTransportList->addColumn( i18n("Name") );
  mTransportList->addColumn( i18n("Type") );
  mTransportList->setAllColumnsShowFocus( true );
  mTransportList->setSorting( -1 );
  connect( mTransportList, TQT_SIGNAL(selectionChanged()),
           this, TQT_SLOT(slotTransportSelected()) );
  connect( mTransportList, TQT_SIGNAL(doubleClicked( TQListViewItem *)),
           this, TQT_SLOT(slotModifySelectedTransport()) );
  hlay->addWidget( mTransportList, 1 );

  // a vbox layout for the buttons: zero stretch, spacing inherited from hlay
  btn_vlay = new TQVBoxLayout( hlay );

  // "add..." button: stretch 0
  button = new TQPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  connect( button, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotAddTransport()) );
  btn_vlay->addWidget( button );

  // "modify..." button: stretch 0
  mModifyTransportButton = new TQPushButton( i18n("&Modify..."), this );
  mModifyTransportButton->setAutoDefault( false );
  mModifyTransportButton->setEnabled( false ); // b/c no item is selected yet
  connect( mModifyTransportButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotModifySelectedTransport()) );
  btn_vlay->addWidget( mModifyTransportButton );

  // "remove" button: stretch 0
  mRemoveTransportButton = new TQPushButton( i18n("R&emove"), this );
  mRemoveTransportButton->setAutoDefault( false );
  mRemoveTransportButton->setEnabled( false ); // b/c no item is selected yet
  connect( mRemoveTransportButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRemoveSelectedTransport()) );
  btn_vlay->addWidget( mRemoveTransportButton );

  mSetDefaultTransportButton = new TQPushButton( i18n("Set Default"), this );
  mSetDefaultTransportButton->setAutoDefault( false );
  mSetDefaultTransportButton->setEnabled( false );
  connect ( mSetDefaultTransportButton, TQT_SIGNAL(clicked()),
            this, TQT_SLOT(slotSetDefaultTransport()) );
  btn_vlay->addWidget( mSetDefaultTransportButton );
  btn_vlay->addStretch( 1 ); // spacer

  // "Common options" groupbox:
  group = new TQGroupBox( 0, Qt::Vertical,
                         i18n("Common Options"), this );
  vlay->addWidget(group);

  // a grid layout for the contents of the "common options" group box
  glay = new TQGridLayout( group->layout(), 5, 3, KDialog::spacingHint() );
  glay->setColStretch( 2, 10 );

  // "confirm before send" check box:
  mConfirmSendCheck = new TQCheckBox( i18n("Confirm &before send"), group );
  glay->addMultiCellWidget( mConfirmSendCheck, 0, 0, 0, 1 );
  connect( mConfirmSendCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "send on check" combo:
  mSendOnCheckCombo = new TQComboBox( false, group );
  mSendOnCheckCombo->insertStringList( TQStringList()
                                      << i18n("Never Automatically")
                                      << i18n("On Manual Mail Checks")
                                      << i18n("On All Mail Checks") );
  glay->addWidget( mSendOnCheckCombo, 1, 1 );
  connect( mSendOnCheckCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "default send method" combo:
  mSendMethodCombo = new TQComboBox( false, group );
  mSendMethodCombo->insertStringList( TQStringList()
                                      << i18n("Send Now")
                                      << i18n("Send Later") );
  glay->addWidget( mSendMethodCombo, 2, 1 );
  connect( mSendMethodCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );


  // "message property" combo:
  // ### FIXME: remove completely?
  mMessagePropertyCombo = new TQComboBox( false, group );
  mMessagePropertyCombo->insertStringList( TQStringList()
                     << i18n("Allow 8-bit")
                     << i18n("MIME Compliant (Quoted Printable)") );
  glay->addWidget( mMessagePropertyCombo, 3, 1 );
  connect( mMessagePropertyCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "default domain" input field:
  mDefaultDomainEdit = new KLineEdit( group );
  glay->addMultiCellWidget( mDefaultDomainEdit, 4, 4, 1, 2 );
  connect( mDefaultDomainEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // labels:
  TQLabel *l =  new TQLabel( mSendOnCheckCombo, /*buddy*/
                            i18n("Send &messages in outbox folder:"), group );
  glay->addWidget( l, 1, 0 );

  TQString msg = i18n( GlobalSettings::self()->sendOnCheckItem()->whatsThis().utf8() );
  TQWhatsThis::add( l, msg );
  TQWhatsThis::add( mSendOnCheckCombo, msg );

  glay->addWidget( new TQLabel( mSendMethodCombo, /*buddy*/
                               i18n("Defa&ult send method:"), group ), 2, 0 );
  glay->addWidget( new TQLabel( mMessagePropertyCombo, /*buddy*/
                               i18n("Message &property:"), group ), 3, 0 );
  l = new TQLabel( mDefaultDomainEdit, /*buddy*/
                          i18n("Defaul&t domain:"), group );
  glay->addWidget( l, 4, 0 );

  // and now: add TQWhatsThis:
  msg = i18n( "<qt><p>The default domain is used to complete email "
              "addresses that only consist of the user's name."
              "</p></qt>" );
  TQWhatsThis::add( l, msg );
  TQWhatsThis::add( mDefaultDomainEdit, msg );
}


void AccountsPage::SendingTab::slotTransportSelected()
{
  TQListViewItem *cur = mTransportList->selectedItem();
  mModifyTransportButton->setEnabled( cur );
  mRemoveTransportButton->setEnabled( cur );
  mSetDefaultTransportButton->setEnabled( cur );
}

// adds a number to @p name to make the name unique
static inline TQString uniqueName( const TQStringList & list,
                                  const TQString & name )
{
  int suffix = 1;
  TQString result = name;
  while ( list.find( result ) != list.end() ) {
    result = i18n("%1: name; %2: number appended to it to make it unique "
                  "among a list of names", "%1 %2")
      .arg( name ).arg( suffix );
    suffix++;
  }
  return result;
}

void AccountsPage::SendingTab::slotSetDefaultTransport()
{
  TQListViewItem *item = mTransportList->selectedItem();
  if ( !item ) return;

  KMTransportInfo ti;

  TQListViewItemIterator it( mTransportList );
  for ( ; it.current(); ++it ) {
  ti.readConfig( KMTransportInfo::findTransport( it.current()->text(0) ));
  if ( ti.type != "sendmail" ) {
    it.current()->setText( 1, "smtp" );
  } else {
    it.current()->setText( 1, "sendmail" );
    }
  }

  if ( item->text(1) != "sendmail" ) {
    item->setText( 1, i18n( "smtp (Default)" ));
  } else {
    item->setText( 1, i18n( "sendmail (Default)" ));
  }
  GlobalSettings::self()->setDefaultTransport( item->text(0) );

}

void AccountsPage::SendingTab::slotAddTransport()
{
  int transportType;

  { // limit scope of selDialog
    KMTransportSelDlg selDialog( this );
    if ( selDialog.exec() != TQDialog::Accepted ) return;
    transportType = selDialog.selected();
  }

  KMTransportInfo *transportInfo = new KMTransportInfo();
  switch ( transportType ) {
  case 0: // smtp
    transportInfo->type = TQString::fromLatin1("smtp");
    break;
  case 1: // sendmail
    transportInfo->type = TQString::fromLatin1("sendmail");
    transportInfo->name = i18n("Sendmail");
    transportInfo->host = _PATH_SENDMAIL; // ### FIXME: use const, not #define
    break;
  default:
    assert( 0 );
  }

  KMTransportDialog dialog( i18n("Add Transport"), transportInfo, this );

  // create list of names:
  // ### move behind dialog.exec()?
  TQStringList transportNames;
  TQPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    transportNames << (*it)->name;

  if( dialog.exec() != TQDialog::Accepted ) {
    delete transportInfo;
    return;
  }

  // disambiguate the name by appending a number:
  // ### FIXME: don't allow this error to happen in the first place!
  transportInfo->name = uniqueName( transportNames, transportInfo->name );
  // append to names and transportinfo lists:
  transportNames << transportInfo->name;
  mTransportInfoList.append( transportInfo );

  // append to listview:
  // ### FIXME: insert before the selected item, append on empty selection
  TQListViewItem *lastItem = mTransportList->firstChild();
  TQString typeDisplayName;
  if ( lastItem ) {
    typeDisplayName = transportInfo->type;
  } else {
    typeDisplayName = i18n("%1: type of transport. Result used in "
                           "Configure->Accounts->Sending listview, \"type\" "
                           "column, first row, to indicate that this is the "
                           "default transport", "%1 (Default)")
      .arg( transportInfo->type );
    GlobalSettings::self()->setDefaultTransport( transportInfo->name );
  }
  (void) new TQListViewItem( mTransportList, lastItem, transportInfo->name,
                            typeDisplayName );

  // notify anyone who cares:
  emit transportListChanged( transportNames );
  emit changed( true );
}

void AccountsPage::SendingTab::slotModifySelectedTransport()
{
  TQListViewItem *item = mTransportList->selectedItem();
  if ( !item ) return;

  const TQString& originalTransport = item->text(0);

  TQPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    if ( (*it)->name == item->text(0) ) break;
  if ( !it.current() ) return;

  KMTransportDialog dialog( i18n("Modify Transport"), (*it), this );

  if ( dialog.exec() != TQDialog::Accepted ) return;

  // create the list of names of transports, but leave out the current
  // item:
  TQStringList transportNames;
  TQPtrListIterator<KMTransportInfo> jt( mTransportInfoList );
  int entryLocation = -1;
  for ( jt.toFirst() ; jt.current() ; ++jt )
    if ( jt != it )
      transportNames << (*jt)->name;
    else
      entryLocation = transportNames.count();
  assert( entryLocation >= 0 );

  // make the new name unique by appending a high enough number:
  (*it)->name = uniqueName( transportNames, (*it)->name );
  // change the list item to the new name
  item->setText( 0, (*it)->name );
  // and insert the new name at the position of the old in the list of
  // strings; then broadcast the new list:
  transportNames.insert( transportNames.at( entryLocation ), (*it)->name );
  const TQString& newTransportName = (*it)->name;

  TQStringList changedIdents;
  KPIM::IdentityManager * im = kmkernel->identityManager();
  for ( KPIM::IdentityManager::Iterator it = im->modifyBegin(); it != im->modifyEnd(); ++it ) {
    if ( originalTransport == (*it).transport() ) {
      (*it).setTransport( newTransportName );
      changedIdents += (*it).identityName();
    }
  }

  if ( !changedIdents.isEmpty() ) {
    TQString information = i18n( "This identity has been changed to use the modified transport:",
                          "These %n identities have been changed to use the modified transport:",
                          changedIdents.count() );
    KMessageBox::informationList( this, information, changedIdents );
  }

  emit transportListChanged( transportNames );
  emit changed( true );
}

void AccountsPage::SendingTab::slotRemoveSelectedTransport()
{
  TQListViewItem *item = mTransportList->selectedItem();
  if ( !item ) return;

  bool selectedTransportWasDefault = false;
  if ( item->text( 0 ) == GlobalSettings::self()->defaultTransport() ) {
      selectedTransportWasDefault = true;
  }
  TQStringList changedIdents;
  KPIM::IdentityManager * im = kmkernel->identityManager();
  for ( KPIM::IdentityManager::Iterator it = im->modifyBegin(); it != im->modifyEnd(); ++it ) {
    if ( item->text( 0 ) == (*it).transport() ) {
      (*it).setTransport( TQString::null );
      changedIdents += (*it).identityName();
    }
  }

  // if the deleted transport is the currently used transport reset it to default
  const TQString& currentTransport = GlobalSettings::self()->currentTransport();
  if ( item->text( 0 ) == currentTransport ) {
    GlobalSettings::self()->setCurrentTransport( TQString::null );
  }

  if ( !changedIdents.isEmpty() ) {
    TQString information = i18n( "This identity has been changed to use the default transport:",
                          "These %n identities have been changed to use the default transport:",
                          changedIdents.count() );
    KMessageBox::informationList( this, information, changedIdents );
  }

  TQPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( it.toFirst() ; it.current() ; ++it )
    if ( (*it)->name == item->text(0) ) break;
  if ( !it.current() ) return;

  KMTransportInfo ti;

  if( selectedTransportWasDefault )
  {
    TQListViewItem *newCurrent = item->itemBelow();
    if ( !newCurrent ) newCurrent = item->itemAbove();
    //mTransportList->removeItem( item );
    if ( newCurrent ) {
      mTransportList->setCurrentItem( newCurrent );
      mTransportList->setSelected( newCurrent, true );
      GlobalSettings::self()->setDefaultTransport( newCurrent->text(0) );
      ti.readConfig( KMTransportInfo::findTransport( newCurrent->text(0) ));
      if ( ti.type != "sendmail" ) {
        newCurrent->setText( 1, i18n("smtp (Default)") );
      } else {
        newCurrent->setText( 1, i18n("sendmail (Default)" ));
      }
    } else {
      GlobalSettings::self()->setDefaultTransport( TQString::null );
    }
  }
  delete item;
  mTransportInfoList.remove( it );

  TQStringList transportNames;
  for ( it.toFirst() ; it.current() ; ++it )
    transportNames << (*it)->name;
  emit transportListChanged( transportNames );
  emit changed( true );
}

void AccountsPage::SendingTab::doLoadFromGlobalSettings() {
  mSendOnCheckCombo->setCurrentItem( GlobalSettings::self()->sendOnCheck() );
}

void AccountsPage::SendingTab::doLoadOther() {
  KConfigGroup general( KMKernel::config(), "General");
  KConfigGroup composer( KMKernel::config(), "Composer");

  int numTransports = general.readNumEntry("transports", 0);

  TQListViewItem *top = 0;
  mTransportInfoList.clear();
  mTransportList->clear();
  TQStringList transportNames;
  for ( int i = 1 ; i <= numTransports ; i++ ) {
    KMTransportInfo *ti = new KMTransportInfo();
    ti->readConfig(i);
    mTransportInfoList.append( ti );
    transportNames << ti->name;
    top = new TQListViewItem( mTransportList, top, ti->name, ti->type );
  }
  emit transportListChanged( transportNames );

  const TQString &defaultTransport = GlobalSettings::self()->defaultTransport();

  TQListViewItemIterator it( mTransportList );
  for ( ; it.current(); ++it ) {
    if ( it.current()->text(0) == defaultTransport ) {
      if ( it.current()->text(1) != "sendmail" ) {
        it.current()->setText( 1, i18n( "smtp (Default)" ));
      } else {
        it.current()->setText( 1, i18n( "sendmail (Default)" ));
      }
    } else {
      if ( it.current()->text(1) != "sendmail" ) {
        it.current()->setText( 1, "smtp" );
      } else {
        it.current()->setText( 1, "sendmail" );
      }
    }
  }

  mSendMethodCombo->setCurrentItem(
                kmkernel->msgSender()->sendImmediate() ? 0 : 1 );
  mMessagePropertyCombo->setCurrentItem(
                kmkernel->msgSender()->sendQuotedPrintable() ? 1 : 0 );

  mConfirmSendCheck->setChecked( composer.readBoolEntry( "confirm-before-send",
                                                         false ) );
  TQString str = general.readEntry( "Default domain" );
  if( str.isEmpty() )
  {
    //### FIXME: Use the global convenience function instead of the homebrewed
    //           solution once we can rely on HEAD kdelibs.
    //str = KGlobal::hostname(); ???????
    char buffer[256];
    if ( !gethostname( buffer, 255 ) )
      // buffer need not be NUL-terminated if it has full length
      buffer[255] = 0;
    else
      buffer[0] = 0;
    str = TQString::fromLatin1( *buffer ? buffer : "localhost" );
  }
  mDefaultDomainEdit->setText( str );
}

void AccountsPage::SendingTab::save() {
  KConfigGroup general( KMKernel::config(), "General" );
  KConfigGroup composer( KMKernel::config(), "Composer" );

  // Save transports:
  general.writeEntry( "transports", mTransportInfoList.count() );
  TQPtrListIterator<KMTransportInfo> it( mTransportInfoList );
  for ( int i = 1 ; it.current() ; ++it, ++i )
    (*it)->writeConfig(i);

  // Save common options:
  GlobalSettings::self()->setSendOnCheck( mSendOnCheckCombo->currentItem() );
  kmkernel->msgSender()->setSendImmediate(
                             mSendMethodCombo->currentItem() == 0 );
  kmkernel->msgSender()->setSendQuotedPrintable(
                             mMessagePropertyCombo->currentItem() == 1 );
  kmkernel->msgSender()->writeConfig( false ); // don't sync
  composer.writeEntry("confirm-before-send", mConfirmSendCheck->isChecked() );
  general.writeEntry( "Default domain", mDefaultDomainEdit->text() );
}

TQString AccountsPage::ReceivingTab::helpAnchor() const {
  return TQString::fromLatin1("configure-accounts-receiving");
}

AccountsPageReceivingTab::AccountsPageReceivingTab( TQWidget * parent, const char * name )
  : ConfigModuleTab ( parent, name )
{
  // temp. vars:
  TQVBoxLayout *vlay;
  TQVBoxLayout *btn_vlay;
  TQHBoxLayout *hlay;
  TQPushButton *button;
  TQGroupBox   *group;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // label: zero stretch
  vlay->addWidget( new TQLabel( i18n("Incoming accounts (add at least one):"), this ) );

  // hbox layout: stretch 10, spacing inherited from vlay
  hlay = new TQHBoxLayout();
  vlay->addLayout( hlay, 10 ); // high stretch to suppress groupbox's growing

  // account list: left widget in hlay; stretch 1
  mAccountList = new ListView( this, "accountList", 5 );
  mAccountList->addColumn( i18n("Name") );
  mAccountList->addColumn( i18n("Type") );
  mAccountList->addColumn( i18n("Folder") );
  mAccountList->setAllColumnsShowFocus( true );
  mAccountList->setSorting( -1 );
  connect( mAccountList, TQT_SIGNAL(selectionChanged()),
           this, TQT_SLOT(slotAccountSelected()) );
  connect( mAccountList, TQT_SIGNAL(doubleClicked( TQListViewItem *)),
           this, TQT_SLOT(slotModifySelectedAccount()) );
  hlay->addWidget( mAccountList, 1 );

  // a vbox layout for the buttons: zero stretch, spacing inherited from hlay
  btn_vlay = new TQVBoxLayout( hlay );

  // "add..." button: stretch 0
  button = new TQPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  connect( button, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotAddAccount()) );
  btn_vlay->addWidget( button );

  // "modify..." button: stretch 0
  mModifyAccountButton = new TQPushButton( i18n("&Modify..."), this );
  mModifyAccountButton->setAutoDefault( false );
  mModifyAccountButton->setEnabled( false ); // b/c no item is selected yet
  connect( mModifyAccountButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotModifySelectedAccount()) );
  btn_vlay->addWidget( mModifyAccountButton );

  // "remove..." button: stretch 0
  mRemoveAccountButton = new TQPushButton( i18n("R&emove"), this );
  mRemoveAccountButton->setAutoDefault( false );
  mRemoveAccountButton->setEnabled( false ); // b/c no item is selected yet
  connect( mRemoveAccountButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRemoveSelectedAccount()) );
  btn_vlay->addWidget( mRemoveAccountButton );
  btn_vlay->addStretch( 1 ); // spacer

  mCheckmailStartupCheck = new TQCheckBox( i18n("Chec&k mail on startup"), this );
  vlay->addWidget( mCheckmailStartupCheck );
  connect( mCheckmailStartupCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "New Mail Notification" group box: stretch 0
  group = new TQVGroupBox( i18n("New Mail Notification"), this );
  vlay->addWidget( group );
  group->layout()->setSpacing( KDialog::spacingHint() );

  // "beep on new mail" check box:
  mBeepNewMailCheck = new TQCheckBox(i18n("&Beep"), group );
  mBeepNewMailCheck->setSizePolicy( TQSizePolicy( TQSizePolicy::MinimumExpanding,
                                                 TQSizePolicy::Fixed ) );
  connect( mBeepNewMailCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "Detailed new mail notification" check box
  mVerboseNotificationCheck =
    new TQCheckBox( i18n( "Deta&iled new mail notification" ), group );
  mVerboseNotificationCheck->setSizePolicy( TQSizePolicy( TQSizePolicy::MinimumExpanding,
                                                         TQSizePolicy::Fixed ) );
  TQToolTip::add( mVerboseNotificationCheck,
                 i18n( "Show for each folder the number of newly arrived "
                       "messages" ) );
  TQWhatsThis::add( mVerboseNotificationCheck,
    GlobalSettings::self()->verboseNewMailNotificationItem()->whatsThis() );
  connect( mVerboseNotificationCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "Other Actions" button:
  mOtherNewMailActionsButton = new TQPushButton( i18n("Other Actio&ns"), group );
  mOtherNewMailActionsButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed,
                                                          TQSizePolicy::Fixed ) );
  connect( mOtherNewMailActionsButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotEditNotifications()) );
}

AccountsPageReceivingTab::~AccountsPageReceivingTab()
{
  // When hitting Cancel or closing the dialog with the window-manager-button,
  // we have a number of things to clean up:

  // The newly created accounts
  TQValueList< TQGuardedPtr<KMAccount> >::Iterator it;
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it ) {
    delete (*it);
  }
  mNewAccounts.clear();

  // The modified accounts
  TQValueList<ModifiedAccountsType*>::Iterator j;
  for ( j = mModifiedAccounts.begin() ; j != mModifiedAccounts.end() ; ++j ) {
    delete (*j)->newAccount;
    delete (*j);
  }
  mModifiedAccounts.clear();


}

void AccountsPage::ReceivingTab::slotAccountSelected()
{
  TQListViewItem * item = mAccountList->selectedItem();
  mModifyAccountButton->setEnabled( item );
  mRemoveAccountButton->setEnabled( item );
}

TQStringList AccountsPage::ReceivingTab::occupiedNames()
{
  TQStringList accountNames = kmkernel->acctMgr()->getAccounts();

  TQValueList<ModifiedAccountsType*>::Iterator k;
  for (k = mModifiedAccounts.begin(); k != mModifiedAccounts.end(); ++k )
    if ((*k)->oldAccount)
      accountNames.remove( (*k)->oldAccount->name() );

  TQValueList< TQGuardedPtr<KMAccount> >::Iterator l;
  for (l = mAccountsToDelete.begin(); l != mAccountsToDelete.end(); ++l )
    if (*l)
      accountNames.remove( (*l)->name() );

  TQValueList< TQGuardedPtr<KMAccount> >::Iterator it;
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it )
    if (*it)
      accountNames += (*it)->name();

  TQValueList<ModifiedAccountsType*>::Iterator j;
  for (j = mModifiedAccounts.begin(); j != mModifiedAccounts.end(); ++j )
    accountNames += (*j)->newAccount->name();

  return accountNames;
}

void AccountsPage::ReceivingTab::slotAddAccount() {
  KMAcctSelDlg accountSelectorDialog( this );
  if( accountSelectorDialog.exec() != TQDialog::Accepted ) return;

  const char *accountType = 0;
  switch ( accountSelectorDialog.selected() ) {
    case 0: accountType = "local";      break;
    case 1: accountType = "pop";        break;
    case 2: accountType = "imap";       break;
    case 3: accountType = "cachedimap"; break;
    case 4: accountType = "maildir";    break;

    default:
      // ### FIXME: How should this happen???
      // replace with assert.
      KMessageBox::sorry( this, i18n("Unknown account type selected") );
      return;
  }

  KMAccount *account
    = kmkernel->acctMgr()->create( TQString::fromLatin1( accountType ) );
  if ( !account ) {
    // ### FIXME: Give the user more information. Is this error
    // recoverable?
    KMessageBox::sorry( this, i18n("Unable to create account") );
    return;
  }

  account->init(); // fill the account fields with good default values

  AccountDialog dialog( i18n("Add Account"), account, this );

  TQStringList accountNames = occupiedNames();

  if( dialog.exec() != TQDialog::Accepted ) {
    delete account;
    return;
  }

  account->deinstallTimer();
  account->setName( uniqueName( accountNames, account->name() ) );

  TQListViewItem *after = mAccountList->firstChild();
  while ( after && after->nextSibling() )
    after = after->nextSibling();

  TQListViewItem *listItem =
    new TQListViewItem( mAccountList, after, account->name(), account->type() );
  if( account->folder() )
    listItem->setText( 2, account->folder()->label() );

  mNewAccounts.append( account );
  emit changed( true );
}



void AccountsPage::ReceivingTab::slotModifySelectedAccount()
{
  TQListViewItem *listItem = mAccountList->selectedItem();
  if( !listItem ) return;

  KMAccount *account = 0;
  TQValueList<ModifiedAccountsType*>::Iterator j;
  for (j = mModifiedAccounts.begin(); j != mModifiedAccounts.end(); ++j )
    if ( (*j)->newAccount->name() == listItem->text(0) ) {
      account = (*j)->newAccount;
      break;
    }

  if ( !account ) {
    TQValueList< TQGuardedPtr<KMAccount> >::Iterator it;
    for ( it = mNewAccounts.begin() ; it != mNewAccounts.end() ; ++it )
      if ( (*it)->name() == listItem->text(0) ) {
        account = *it;
        break;
      }

    if ( !account ) {
      account = kmkernel->acctMgr()->findByName( listItem->text(0) );
      if( !account ) {
        // ### FIXME: How should this happen? See above.
        KMessageBox::sorry( this, i18n("Unable to locate account") );
        return;
      }
      if ( account->type() == "imap" || account->type() == "cachedimap" )
      {
        ImapAccountBase* ai = static_cast<ImapAccountBase*>( account );
        if ( ai->namespaces().isEmpty() || ai->namespaceToDelimiter().isEmpty() )
        {
          // connect to server - the namespaces are fetched automatically
          kdDebug(5006) << "slotModifySelectedAccount - connect" << endl;
          ai->makeConnection();
        }
      }

      ModifiedAccountsType *mod = new ModifiedAccountsType;
      mod->oldAccount = account;
      mod->newAccount = kmkernel->acctMgr()->create( account->type(),
                                                   account->name() );
      mod->newAccount->pseudoAssign( account );
      mModifiedAccounts.append( mod );
      account = mod->newAccount;
    }
  }

  TQStringList accountNames = occupiedNames();
  accountNames.remove( account->name() );

  AccountDialog dialog( i18n("Modify Account"), account, this );

  if( dialog.exec() != TQDialog::Accepted ) return;

  account->setName( uniqueName( accountNames, account->name() ) );

  listItem->setText( 0, account->name() );
  listItem->setText( 1, account->type() );
  if( account->folder() )
    listItem->setText( 2, account->folder()->label() );

  emit changed( true );
}



void AccountsPage::ReceivingTab::slotRemoveSelectedAccount() {
  TQListViewItem *listItem = mAccountList->selectedItem();
  if( !listItem ) return;

  KMAccount *acct = 0;
  TQValueList<ModifiedAccountsType*>::Iterator j;
  for ( j = mModifiedAccounts.begin() ; j != mModifiedAccounts.end() ; ++j )
    if ( (*j)->newAccount->name() == listItem->text(0) ) {
      acct = (*j)->oldAccount;
      mAccountsToDelete.append( acct );
      mModifiedAccounts.remove( j );
      break;
    }
  if ( !acct ) {
    TQValueList< TQGuardedPtr<KMAccount> >::Iterator it;
    for ( it = mNewAccounts.begin() ; it != mNewAccounts.end() ; ++it )
      if ( (*it)->name() == listItem->text(0) ) {
        acct = *it;
        mNewAccounts.remove( it );
        break;
      }
  }
  if ( !acct ) {
    acct = kmkernel->acctMgr()->findByName( listItem->text(0) );
    if ( acct )
      mAccountsToDelete.append( acct );
  }
  if ( !acct ) {
    // ### FIXME: see above
    KMessageBox::sorry( this, i18n("<qt>Unable to locate account <b>%1</b>.</qt>")
                        .arg(listItem->text(0)) );
    return;
  }

  TQListViewItem * item = listItem->itemBelow();
  if ( !item ) item = listItem->itemAbove();
  delete listItem;

  if ( item )
    mAccountList->setSelected( item, true );

  emit changed( true );
}

void AccountsPage::ReceivingTab::slotEditNotifications()
{
  if(kmkernel->xmlGuiInstance())
    KNotifyDialog::configure(this, 0, kmkernel->xmlGuiInstance()->aboutData());
  else
    KNotifyDialog::configure(this);
}

void AccountsPage::ReceivingTab::doLoadFromGlobalSettings() {
  mVerboseNotificationCheck->setChecked( GlobalSettings::self()->verboseNewMailNotification() );
}

void AccountsPage::ReceivingTab::doLoadOther() {
  KConfigGroup general( KMKernel::config(), "General" );

  mAccountList->clear();
  TQListViewItem *top = 0;

  for( KMAccount *a = kmkernel->acctMgr()->first(); a!=0;
       a = kmkernel->acctMgr()->next() ) {
    TQListViewItem *listItem =
      new TQListViewItem( mAccountList, top, a->name(), a->type() );
    if( a->folder() )
      listItem->setText( 2, a->folder()->label() );
    top = listItem;
  }
  TQListViewItem *listItem = mAccountList->firstChild();
  if ( listItem ) {
    mAccountList->setCurrentItem( listItem );
    mAccountList->setSelected( listItem, true );
  }

  mBeepNewMailCheck->setChecked( general.readBoolEntry("beep-on-mail", false ) );
  mCheckmailStartupCheck->setChecked( general.readBoolEntry("checkmail-startup", false) );
  TQTimer::singleShot( 0, this, TQT_SLOT( slotTweakAccountList() ) );
}

void AccountsPage::ReceivingTab::slotTweakAccountList()
{
  // Force the contentsWidth of mAccountList to be recalculated so that items can be
  // selected in the normal way. It would be best if this were not necessary.
  mAccountList->resizeContents( mAccountList->visibleWidth(), mAccountList->contentsHeight() );
}

void AccountsPage::ReceivingTab::save() {
  // Add accounts marked as new
  TQValueList< TQGuardedPtr<KMAccount> >::Iterator it;
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it ) {
    kmkernel->acctMgr()->add( *it ); // calls installTimer too
  }

  // Update accounts that have been modified
  TQValueList<ModifiedAccountsType*>::Iterator j;
  for ( j = mModifiedAccounts.begin() ; j != mModifiedAccounts.end() ; ++j ) {
    (*j)->oldAccount->pseudoAssign( (*j)->newAccount );
    delete (*j)->newAccount;
    delete (*j);
  }
  mModifiedAccounts.clear();

  // Delete accounts marked for deletion
  for ( it = mAccountsToDelete.begin() ;
        it != mAccountsToDelete.end() ; ++it ) {
    kmkernel->acctMgr()->writeConfig( true );
    if ( (*it) && !kmkernel->acctMgr()->remove(*it) )
      KMessageBox::sorry( this, i18n("<qt>Unable to locate account <b>%1</b>.</qt>")
                          .arg( (*it)->name() ) );
  }
  mAccountsToDelete.clear();

  // Incoming mail
  kmkernel->acctMgr()->writeConfig( false );
  kmkernel->cleanupImapFolders();

  // Save Mail notification settings
  KConfigGroup general( KMKernel::config(), "General" );
  general.writeEntry( "beep-on-mail", mBeepNewMailCheck->isChecked() );
  GlobalSettings::self()->setVerboseNewMailNotification( mVerboseNotificationCheck->isChecked() );

  general.writeEntry( "checkmail-startup", mCheckmailStartupCheck->isChecked() );

  // Sync new IMAP accounts ASAP:
  for (it = mNewAccounts.begin(); it != mNewAccounts.end(); ++it ) {
    KMAccount *macc = (*it);
    ImapAccountBase *acc = dynamic_cast<ImapAccountBase*> (macc);
    if ( acc ) {
      AccountUpdater *au = new AccountUpdater( acc );
      au->update();
    }
  }
  mNewAccounts.clear();

}

// *************************************************************
// *                                                           *
// *                     AppearancePage                        *
// *                                                           *
// *************************************************************
TQString AppearancePage::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance");
}

AppearancePage::AppearancePage( TQWidget * parent, const char * name )
  : ConfigModuleWithTabs( parent, name )
{
  //
  // "Fonts" tab:
  //
  mFontsTab = new FontsTab();
  addTab( mFontsTab, i18n("&Fonts") );

  //
  // "Colors" tab:
  //
  mColorsTab = new ColorsTab();
  addTab( mColorsTab, i18n("Color&s") );

  //
  // "Layout" tab:
  //
  mLayoutTab = new LayoutTab();
  addTab( mLayoutTab, i18n("La&yout") );

  //
  // "Headers" tab:
  //
  mHeadersTab = new HeadersTab();
  addTab( mHeadersTab, i18n("M&essage List") );

  //
  // "Reader window" tab:
  //
  mReaderTab = new ReaderTab();
  addTab( mReaderTab, i18n("Message W&indow") );

  //
  // "System Tray" tab:
  //
  mSystemTrayTab = new SystemTrayTab();
  addTab( mSystemTrayTab, i18n("System &Tray") );

  load();
}


TQString AppearancePage::FontsTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-fonts");
}

static const struct {
  const char * configName;
  const char * displayName;
  bool   enableFamilyAndSize;
  bool   onlyFixed;
} fontNames[] = {
  { "body-font", I18N_NOOP("Message Body"), true, false },
  { "list-font", I18N_NOOP("Message List"), true, false },
  { "list-new-font", I18N_NOOP("Message List - New Messages"), true, false },
  { "list-unread-font", I18N_NOOP("Message List - Unread Messages"), true, false },
  { "list-important-font", I18N_NOOP("Message List - Important Messages"), true, false },
  { "list-todo-font", I18N_NOOP("Message List - Todo Messages"), true, false },
  { "list-date-font", I18N_NOOP("Message List - Date Field"), true, false },
  { "folder-font", I18N_NOOP("Folder List"), true, false },
  { "quote1-font", I18N_NOOP("Quoted Text - First Level"), false, false },
  { "quote2-font", I18N_NOOP("Quoted Text - Second Level"), false, false },
  { "quote3-font", I18N_NOOP("Quoted Text - Third Level"), false, false },
  { "fixed-font", I18N_NOOP("Fixed Width Font"), true, true },
  { "composer-font", I18N_NOOP("Composer"), true, false },
  { "print-font",  I18N_NOOP("Printing Output"), true, false },
};
static const int numFontNames = sizeof fontNames / sizeof *fontNames;

AppearancePageFontsTab::AppearancePageFontsTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name ), mActiveFontIndex( -1 )
{
  assert( numFontNames == sizeof mFont / sizeof *mFont );
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQHBoxLayout *hlay;
  TQLabel      *label;

  // "Use custom fonts" checkbox, followed by <hr>
  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  mCustomFontCheck = new TQCheckBox( i18n("&Use custom fonts"), this );
  vlay->addWidget( mCustomFontCheck );
  vlay->addWidget( new KSeparator( KSeparator::HLine, this ) );
  connect ( mCustomFontCheck, TQT_SIGNAL( stateChanged( int ) ),
            this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "font location" combo box and label:
  hlay = new TQHBoxLayout( vlay ); // inherites spacing
  mFontLocationCombo = new TQComboBox( false, this );
  mFontLocationCombo->setEnabled( false ); // !mCustomFontCheck->isChecked()

  TQStringList fontDescriptions;
  for ( int i = 0 ; i < numFontNames ; i++ )
    fontDescriptions << i18n( fontNames[i].displayName );
  mFontLocationCombo->insertStringList( fontDescriptions );

  label = new TQLabel( mFontLocationCombo, i18n("Apply &to:"), this );
  label->setEnabled( false ); // since !mCustomFontCheck->isChecked()
  hlay->addWidget( label );

  hlay->addWidget( mFontLocationCombo );
  hlay->addStretch( 10 );
  vlay->addSpacing( KDialog::spacingHint() );
  mFontChooser = new KFontChooser( this, "font", false, TQStringList(),
                                   false, 4 );
  mFontChooser->setEnabled( false ); // since !mCustomFontCheck->isChecked()
  vlay->addWidget( mFontChooser );
  connect ( mFontChooser, TQT_SIGNAL( fontSelected( const TQFont& ) ),
            this, TQT_SLOT( slotEmitChanged( void ) ) );


  // {en,dis}able widgets depending on the state of mCustomFontCheck:
  connect( mCustomFontCheck, TQT_SIGNAL(toggled(bool)),
           label, TQT_SLOT(setEnabled(bool)) );
  connect( mCustomFontCheck, TQT_SIGNAL(toggled(bool)),
           mFontLocationCombo, TQT_SLOT(setEnabled(bool)) );
  connect( mCustomFontCheck, TQT_SIGNAL(toggled(bool)),
           mFontChooser, TQT_SLOT(setEnabled(bool)) );
  // load the right font settings into mFontChooser:
  connect( mFontLocationCombo, TQT_SIGNAL(activated(int) ),
           this, TQT_SLOT(slotFontSelectorChanged(int)) );
}


void AppearancePage::FontsTab::slotFontSelectorChanged( int index )
{
  kdDebug(5006) << "slotFontSelectorChanged() called" << endl;
  if( index < 0 || index >= mFontLocationCombo->count() )
    return; // Should never happen, but it is better to check.

  // Save current fontselector setting before we install the new:
  if( mActiveFontIndex == 0 ) {
    mFont[0] = mFontChooser->font();
    // hardcode the family and size of "message body" dependant fonts:
    for ( int i = 0 ; i < numFontNames ; i++ )
      if ( !fontNames[i].enableFamilyAndSize ) {
        // ### shall we copy the font and set the save and re-set
        // {regular,italic,bold,bold italic} property or should we
        // copy only family and pointSize?
        mFont[i].setFamily( mFont[0].family() );
        mFont[i].setPointSize/*Float?*/( mFont[0].pointSize/*Float?*/() );
      }
  } else if ( mActiveFontIndex > 0 )
    mFont[ mActiveFontIndex ] = mFontChooser->font();
  mActiveFontIndex = index;

  // Disonnect so the "Apply" button is not activated by the change
  disconnect ( mFontChooser, TQT_SIGNAL( fontSelected( const TQFont& ) ),
            this, TQT_SLOT( slotEmitChanged( void ) ) );

  // Display the new setting:
  mFontChooser->setFont( mFont[index], fontNames[index].onlyFixed );

  connect ( mFontChooser, TQT_SIGNAL( fontSelected( const TQFont& ) ),
            this, TQT_SLOT( slotEmitChanged( void ) ) );

  // Disable Family and Size list if we have selected a quote font:
  mFontChooser->enableColumn( KFontChooser::FamilyList|KFontChooser::SizeList,
                              fontNames[ index ].enableFamilyAndSize );
}

void AppearancePage::FontsTab::doLoadOther() {
  KConfigGroup fonts( KMKernel::config(), "Fonts" );

  mFont[0] = KGlobalSettings::generalFont();
  TQFont fixedFont = KGlobalSettings::fixedFont();
  for ( int i = 0 ; i < numFontNames ; i++ )
    mFont[i] = fonts.readFontEntry( fontNames[i].configName,
      (fontNames[i].onlyFixed) ? &fixedFont : &mFont[0] );

  mCustomFontCheck->setChecked( !fonts.readBoolEntry( "defaultFonts", true ) );
  mFontLocationCombo->setCurrentItem( 0 );
  slotFontSelectorChanged( 0 );
}

void AppearancePage::FontsTab::installProfile( KConfig * profile ) {
  KConfigGroup fonts( profile, "Fonts" );

  // read fonts that are defined in the profile:
  bool needChange = false;
  for ( int i = 0 ; i < numFontNames ; i++ )
    if ( fonts.hasKey( fontNames[i].configName ) ) {
      needChange = true;
      mFont[i] = fonts.readFontEntry( fontNames[i].configName );
      kdDebug(5006) << "got font \"" << fontNames[i].configName
                << "\" thusly: \"" << mFont[i].toString() << "\"" << endl;
    }
  if ( needChange && mFontLocationCombo->currentItem() > 0 )
    mFontChooser->setFont( mFont[ mFontLocationCombo->currentItem() ],
      fontNames[ mFontLocationCombo->currentItem() ].onlyFixed );

  if ( fonts.hasKey( "defaultFonts" ) )
    mCustomFontCheck->setChecked( !fonts.readBoolEntry( "defaultFonts" ) );
}

void AppearancePage::FontsTab::save() {
  KConfigGroup fonts( KMKernel::config(), "Fonts" );

  // read the current font (might have been modified)
  if ( mActiveFontIndex >= 0 )
    mFont[ mActiveFontIndex ] = mFontChooser->font();

  bool customFonts = mCustomFontCheck->isChecked();
  fonts.writeEntry( "defaultFonts", !customFonts );
  for ( int i = 0 ; i < numFontNames ; i++ )
    if ( customFonts || fonts.hasKey( fontNames[i].configName ) )
      // Don't write font info when we use default fonts, but write
      // if it's already there:
      fonts.writeEntry( fontNames[i].configName, mFont[i] );
}

TQString AppearancePage::ColorsTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-colors");
}


static const struct {
  const char * configName;
  const char * displayName;
} colorNames[] = { // adjust setup() if you change this:
  { "BackgroundColor", I18N_NOOP("Composer Background") },
  { "AltBackgroundColor", I18N_NOOP("Alternative Background Color") },
  { "ForegroundColor", I18N_NOOP("Normal Text") },
  { "QuotedText1", I18N_NOOP("Quoted Text - First Level") },
  { "QuotedText2", I18N_NOOP("Quoted Text - Second Level") },
  { "QuotedText3", I18N_NOOP("Quoted Text - Third Level") },
  { "LinkColor", I18N_NOOP("Link") },
  { "FollowedColor", I18N_NOOP("Followed Link") },
  { "MisspelledColor", I18N_NOOP("Misspelled Words") },
  { "NewMessage", I18N_NOOP("New Message") },
  { "UnreadMessage", I18N_NOOP("Unread Message") },
  { "FlagMessage", I18N_NOOP("Important Message") },
  { "TodoMessage", I18N_NOOP("Todo Message") },
  { "PGPMessageEncr", I18N_NOOP("OpenPGP Message - Encrypted") },
  { "PGPMessageOkKeyOk", I18N_NOOP("OpenPGP Message - Valid Signature with Trusted Key") },
  { "PGPMessageOkKeyBad", I18N_NOOP("OpenPGP Message - Valid Signature with Untrusted Key") },
  { "PGPMessageWarn", I18N_NOOP("OpenPGP Message - Unchecked Signature") },
  { "PGPMessageErr", I18N_NOOP("OpenPGP Message - Bad Signature") },
  { "HTMLWarningColor", I18N_NOOP("Border Around Warning Prepending HTML Messages") },
  { "CloseToQuotaColor", I18N_NOOP("Folder Name and Size When Close to Quota") },
  { "ColorbarBackgroundPlain", I18N_NOOP("HTML Status Bar Background - No HTML Message") },
  { "ColorbarForegroundPlain", I18N_NOOP("HTML Status Bar Foreground - No HTML Message") },
  { "ColorbarBackgroundHTML",  I18N_NOOP("HTML Status Bar Background - HTML Message") },
  { "ColorbarForegroundHTML",  I18N_NOOP("HTML Status Bar Foreground - HTML Message") },
};
static const int numColorNames = sizeof colorNames / sizeof *colorNames;

AppearancePageColorsTab::AppearancePageColorsTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout *vlay;

  // "use custom colors" check box
  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );
  mCustomColorCheck = new TQCheckBox( i18n("&Use custom colors"), this );
  vlay->addWidget( mCustomColorCheck );
  connect( mCustomColorCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // color list box:
  mColorList = new ColorListBox( this );
  mColorList->setEnabled( false ); // since !mCustomColorCheck->isChecked()
  TQStringList modeList;
  for ( int i = 0 ; i < numColorNames ; i++ )
    mColorList->insertItem( new ColorListItem( i18n( colorNames[i].displayName ) ) );
  vlay->addWidget( mColorList, 1 );

  // "recycle colors" check box:
  mRecycleColorCheck =
    new TQCheckBox( i18n("Recycle colors on deep &quoting"), this );
  mRecycleColorCheck->setEnabled( false );
  vlay->addWidget( mRecycleColorCheck );
  connect( mRecycleColorCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // close to quota threshold
  TQHBoxLayout *hbox = new TQHBoxLayout(vlay);
  TQLabel *l = new TQLabel( i18n("Close to quota threshold"), this );
  hbox->addWidget( l );
  l->setEnabled( false );
  mCloseToQuotaThreshold = new TQSpinBox( 0, 100, 1, this );
  connect( mCloseToQuotaThreshold, TQT_SIGNAL( valueChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  mCloseToQuotaThreshold->setEnabled( false );
  mCloseToQuotaThreshold->setSuffix( i18n("%"));
  hbox->addWidget( mCloseToQuotaThreshold );
  hbox->addWidget( new TQWidget(this), 2 );

  // {en,dir}able widgets depending on the state of mCustomColorCheck:
  connect( mCustomColorCheck, TQT_SIGNAL(toggled(bool)),
           mColorList, TQT_SLOT(setEnabled(bool)) );
  connect( mCustomColorCheck, TQT_SIGNAL(toggled(bool)),
           mRecycleColorCheck, TQT_SLOT(setEnabled(bool)) );
  connect( mCustomColorCheck, TQT_SIGNAL(toggled(bool)),
           l, TQT_SLOT(setEnabled(bool)) );
  connect( mCustomColorCheck, TQT_SIGNAL(toggled(bool)),
	   mCloseToQuotaThreshold, TQT_SLOT(setEnabled(bool)) );

  connect( mCustomColorCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
}

void AppearancePage::ColorsTab::doLoadOther() {
  KConfigGroup reader( KMKernel::config(), "Reader" );

  mCustomColorCheck->setChecked( !reader.readBoolEntry( "defaultColors", true ) );
  mRecycleColorCheck->setChecked( reader.readBoolEntry( "RecycleQuoteColors", false ) );
  mCloseToQuotaThreshold->setValue( GlobalSettings::closeToQuotaThreshold() );

  static const TQColor defaultColor[ numColorNames ] = {
    kapp->palette().active().base(), // bg
    KGlobalSettings::alternateBackgroundColor(), // alt bg
    kapp->palette().active().text(), // fg
    TQColor( 0x00, 0x80, 0x00 ), // quoted l1
    TQColor( 0x00, 0x70, 0x00 ), // quoted l2
    TQColor( 0x00, 0x60, 0x00 ), // quoted l3
    KGlobalSettings::linkColor(), // link
    KGlobalSettings::visitedLinkColor(), // visited link
    Qt::red, // misspelled words
    Qt::red, // new msg
    Qt::blue, // unread mgs
    TQColor( 0x00, 0x7F, 0x00 ), // important msg
    Qt::blue, // todo mgs
    TQColor( 0x00, 0x80, 0xFF ), // light blue // pgp encrypted
    TQColor( 0x40, 0xFF, 0x40 ), // light green // pgp ok, trusted key
    TQColor( 0xFF, 0xFF, 0x40 ), // light yellow // pgp ok, untrusted key
    TQColor( 0xFF, 0xFF, 0x40 ), // light yellow // pgp unchk
    Qt::red, // pgp bad
    TQColor( 0xFF, 0x40, 0x40 ), // warning text color: light red
    Qt::red, // close to quota
    Qt::lightGray, // colorbar plain bg
    Qt::black,     // colorbar plain fg
    Qt::black,     // colorbar html  bg
    Qt::white,     // colorbar html  fg
  };

  for ( int i = 0 ; i < numColorNames ; i++ ) {
    mColorList->setColor( i,
      reader.readColorEntry( colorNames[i].configName, &defaultColor[i] ) );
  }
  connect( mColorList, TQT_SIGNAL( changed( ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
}

void AppearancePage::ColorsTab::installProfile( KConfig * profile ) {
  KConfigGroup reader( profile, "Reader" );

  if ( reader.hasKey( "defaultColors" ) )
    mCustomColorCheck->setChecked( !reader.readBoolEntry( "defaultColors" ) );
  if ( reader.hasKey( "RecycleQuoteColors" ) )
    mRecycleColorCheck->setChecked( reader.readBoolEntry( "RecycleQuoteColors" ) );

  for ( int i = 0 ; i < numColorNames ; i++ )
    if ( reader.hasKey( colorNames[i].configName ) )
      mColorList->setColor( i, reader.readColorEntry( colorNames[i].configName ) );
}

void AppearancePage::ColorsTab::save() {
  KConfigGroup reader( KMKernel::config(), "Reader" );

  bool customColors = mCustomColorCheck->isChecked();
  reader.writeEntry( "defaultColors", !customColors );

  for ( int i = 0 ; i < numColorNames ; i++ )
    // Don't write color info when we use default colors, but write
    // if it's already there:
    if ( customColors || reader.hasKey( colorNames[i].configName ) )
      reader.writeEntry( colorNames[i].configName, mColorList->color(i) );

  reader.writeEntry( "RecycleQuoteColors", mRecycleColorCheck->isChecked() );
  GlobalSettings::setCloseToQuotaThreshold( mCloseToQuotaThreshold->value() );
}

TQString AppearancePage::LayoutTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-layout");
}

static const EnumConfigEntryItem folderListModes[] = {
  { "long", I18N_NOOP("Lon&g folder list") },
  { "short", I18N_NOOP("Shor&t folder list" ) }
};
static const EnumConfigEntry folderListMode = {
  "Geometry", "FolderList", I18N_NOOP("Folder List"),
  folderListModes, DIM(folderListModes), 0
};


static const EnumConfigEntryItem mimeTreeLocations[] = {
  { "top", I18N_NOOP("Abo&ve the message pane") },
  { "bottom", I18N_NOOP("&Below the message pane") }
};
static const EnumConfigEntry mimeTreeLocation = {
  "Reader", "MimeTreeLocation", I18N_NOOP("Message Structure Viewer Placement"),
  mimeTreeLocations, DIM(mimeTreeLocations), 1
};

static const EnumConfigEntryItem mimeTreeModes[] = {
  { "never", I18N_NOOP("Show &never") },
  { "smart", I18N_NOOP("Show only for non-plaintext &messages") },
  { "always", I18N_NOOP("Show alway&s") }
};
static const EnumConfigEntry mimeTreeMode = {
  "Reader", "MimeTreeMode", I18N_NOOP("Message Structure Viewer"),
  mimeTreeModes, DIM(mimeTreeModes), 1
};


static const EnumConfigEntryItem readerWindowModes[] = {
  { "hide", I18N_NOOP("&Do not show a message preview pane") },
  { "below", I18N_NOOP("Show the message preview pane belo&w the message list") },
  { "right", I18N_NOOP("Show the message preview pane ne&xt to the message list") }
};
static const EnumConfigEntry readerWindowMode = {
  "Geometry", "readerWindowMode", I18N_NOOP("Message Preview Pane"),
  readerWindowModes, DIM(readerWindowModes), 1
};

AppearancePageLayoutTab::AppearancePageLayoutTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout * vlay;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "folder list" radio buttons:
  populateButtonGroup( mFolderListGroup = new TQHButtonGroup( this ), folderListMode );
  vlay->addWidget( mFolderListGroup );
  connect( mFolderListGroup, TQT_SIGNAL ( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  mFavoriteFolderViewCB = new TQCheckBox( i18n("Show favorite folder view"), this );
  connect( mFavoriteFolderViewCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  vlay->addWidget( mFavoriteFolderViewCB );

  // "show reader window" radio buttons:
  populateButtonGroup( mReaderWindowModeGroup = new TQVButtonGroup( this ), readerWindowMode );
  vlay->addWidget( mReaderWindowModeGroup );
  connect( mReaderWindowModeGroup, TQT_SIGNAL ( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "Show MIME Tree" radio buttons:
  populateButtonGroup( mMIMETreeModeGroup = new TQVButtonGroup( this ), mimeTreeMode );
  vlay->addWidget( mMIMETreeModeGroup );
  connect( mMIMETreeModeGroup, TQT_SIGNAL ( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "MIME Tree Location" radio buttons:
  populateButtonGroup( mMIMETreeLocationGroup = new TQHButtonGroup( this ), mimeTreeLocation );
  vlay->addWidget( mMIMETreeLocationGroup );
  connect( mMIMETreeLocationGroup, TQT_SIGNAL ( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  vlay->addStretch( 10 ); // spacer
}

void AppearancePage::LayoutTab::doLoadOther() {
  const KConfigGroup reader( KMKernel::config(), "Reader" );
  const KConfigGroup geometry( KMKernel::config(), "Geometry" );

  loadWidget( mFolderListGroup, geometry, folderListMode );
  loadWidget( mMIMETreeLocationGroup, reader, mimeTreeLocation );
  loadWidget( mMIMETreeModeGroup, reader, mimeTreeMode );
  loadWidget( mReaderWindowModeGroup, geometry, readerWindowMode );
  mFavoriteFolderViewCB->setChecked( GlobalSettings::self()->enableFavoriteFolderView() );
}

void AppearancePage::LayoutTab::installProfile( KConfig * profile ) {
  const KConfigGroup reader( profile, "Reader" );
  const KConfigGroup geometry( profile, "Geometry" );

  loadProfile( mFolderListGroup, geometry, folderListMode );
  loadProfile( mMIMETreeLocationGroup, reader, mimeTreeLocation );
  loadProfile( mMIMETreeModeGroup, reader, mimeTreeMode );
  loadProfile( mReaderWindowModeGroup, geometry, readerWindowMode );
}

void AppearancePage::LayoutTab::save() {
  KConfigGroup reader( KMKernel::config(), "Reader" );
  KConfigGroup geometry( KMKernel::config(), "Geometry" );

  saveButtonGroup( mFolderListGroup, geometry, folderListMode );
  saveButtonGroup( mMIMETreeLocationGroup, reader, mimeTreeLocation );
  saveButtonGroup( mMIMETreeModeGroup, reader, mimeTreeMode );
  saveButtonGroup( mReaderWindowModeGroup, geometry, readerWindowMode );
  GlobalSettings::self()->setEnableFavoriteFolderView( mFavoriteFolderViewCB->isChecked() );
}

//
// Appearance Message List
//

TQString AppearancePage::HeadersTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-headers");
}

static const struct {
  const char * displayName;
  DateFormatter::FormatType dateDisplay;
} dateDisplayConfig[] = {
  { I18N_NOOP("Sta&ndard format (%1)"), KMime::DateFormatter::CTime },
  { I18N_NOOP("Locali&zed format (%1)"), KMime::DateFormatter::Localized },
  { I18N_NOOP("Fancy for&mat (%1)"), KMime::DateFormatter::Fancy },
  { I18N_NOOP("C&ustom format (Shift+F1 for help):"),
    KMime::DateFormatter::Custom }
};
static const int numDateDisplayConfig =
  sizeof dateDisplayConfig / sizeof *dateDisplayConfig;

AppearancePageHeadersTab::AppearancePageHeadersTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name ),
    mCustomDateFormatEdit( 0 )
{
  // tmp. vars:
  TQButtonGroup * group;
  TQRadioButton * radio;

  TQVBoxLayout * vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "General Options" group:
  group = new TQVButtonGroup( i18n( "General Options" ), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mMessageSizeCheck = new TQCheckBox( i18n("Display messa&ge sizes"), group );

  mCryptoIconsCheck = new TQCheckBox( i18n( "Show crypto &icons" ), group );

  mAttachmentCheck = new TQCheckBox( i18n("Show attachment icon"), group );

  mNestedMessagesCheck =
    new TQCheckBox( i18n("&Threaded message list"), group );

  connect( mMessageSizeCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mAttachmentCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mCryptoIconsCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mNestedMessagesCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );


  vlay->addWidget( group );

  // "Message Header Threading Options" group:
  mNestingPolicy =
    new TQVButtonGroup( i18n("Threaded Message List Options"), this );
  mNestingPolicy->layout()->setSpacing( KDialog::spacingHint() );

  mNestingPolicy->insert(
    new TQRadioButton( i18n("Always &keep threads open"),
                      mNestingPolicy ), 0 );
  mNestingPolicy->insert(
    new TQRadioButton( i18n("Threads default to o&pen"),
                      mNestingPolicy ), 1 );
  mNestingPolicy->insert(
    new TQRadioButton( i18n("Threads default to closed"),
                      mNestingPolicy ), 2 );
  mNestingPolicy->insert(
    new TQRadioButton( i18n("Open threads that contain ne&w, unread "
                           "or important messages and open watched threads."),
                      mNestingPolicy ), 3 );

  vlay->addWidget( mNestingPolicy );

  connect( mNestingPolicy, TQT_SIGNAL( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "Date Display" group:
  mDateDisplay = new TQVButtonGroup( i18n("Date Display"), this );
  mDateDisplay->layout()->setSpacing( KDialog::spacingHint() );

  for ( int i = 0 ; i < numDateDisplayConfig ; i++ ) {
    TQString buttonLabel = i18n(dateDisplayConfig[i].displayName);
    if ( buttonLabel.contains("%1") )
      buttonLabel = buttonLabel.arg( DateFormatter::formatCurrentDate( dateDisplayConfig[i].dateDisplay ) );
    radio = new TQRadioButton( buttonLabel, mDateDisplay );
    mDateDisplay->insert( radio, i );
    if ( dateDisplayConfig[i].dateDisplay == DateFormatter::Custom ) {
      mCustomDateFormatEdit = new KLineEdit( mDateDisplay );
      mCustomDateFormatEdit->setEnabled( false );
      connect( radio, TQT_SIGNAL(toggled(bool)),
               mCustomDateFormatEdit, TQT_SLOT(setEnabled(bool)) );
      connect( mCustomDateFormatEdit, TQT_SIGNAL(textChanged(const TQString&)),
               this, TQT_SLOT(slotEmitChanged(void)) );
      TQString customDateWhatsThis =
        i18n("<qt><p><strong>These expressions may be used for the date:"
             "</strong></p>"
             "<ul>"
             "<li>d - the day as a number without a leading zero (1-31)</li>"
             "<li>dd - the day as a number with a leading zero (01-31)</li>"
             "<li>ddd - the abbreviated day name (Mon - Sun)</li>"
             "<li>dddd - the long day name (Monday - Sunday)</li>"
             "<li>M - the month as a number without a leading zero (1-12)</li>"
             "<li>MM - the month as a number with a leading zero (01-12)</li>"
             "<li>MMM - the abbreviated month name (Jan - Dec)</li>"
             "<li>MMMM - the long month name (January - December)</li>"
             "<li>yy - the year as a two digit number (00-99)</li>"
             "<li>yyyy - the year as a four digit number (0000-9999)</li>"
             "</ul>"
             "<p><strong>These expressions may be used for the time:"
             "</string></p> "
             "<ul>"
             "<li>h - the hour without a leading zero (0-23 or 1-12 if AM/PM display)</li>"
             "<li>hh - the hour with a leading zero (00-23 or 01-12 if AM/PM display)</li>"
             "<li>m - the minutes without a leading zero (0-59)</li>"
             "<li>mm - the minutes with a leading zero (00-59)</li>"
             "<li>s - the seconds without a leading zero (0-59)</li>"
             "<li>ss - the seconds with a leading zero (00-59)</li>"
             "<li>z - the milliseconds without leading zeroes (0-999)</li>"
             "<li>zzz - the milliseconds with leading zeroes (000-999)</li>"
             "<li>AP - switch to AM/PM display. AP will be replaced by either \"AM\" or \"PM\".</li>"
             "<li>ap - switch to AM/PM display. ap will be replaced by either \"am\" or \"pm\".</li>"
             "<li>Z - time zone in numeric form (-0500)</li>"
             "</ul>"
             "<p><strong>All other input characters will be ignored."
             "</strong></p></qt>");
      TQWhatsThis::add( mCustomDateFormatEdit, customDateWhatsThis );
      TQWhatsThis::add( radio, customDateWhatsThis );
    }
  } // end for loop populating mDateDisplay

  vlay->addWidget( mDateDisplay );
  connect( mDateDisplay, TQT_SIGNAL( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );


  vlay->addStretch( 10 ); // spacer
}

void AppearancePage::HeadersTab::doLoadOther() {
  KConfigGroup general( KMKernel::config(), "General" );
  KConfigGroup geometry( KMKernel::config(), "Geometry" );

  // "General Options":
  mNestedMessagesCheck->setChecked( geometry.readBoolEntry( "nestedMessages", false ) );
  mMessageSizeCheck->setChecked( general.readBoolEntry( "showMessageSize", false ) );
  mCryptoIconsCheck->setChecked( general.readBoolEntry( "showCryptoIcons", false ) );
  mAttachmentCheck->setChecked( general.readBoolEntry( "showAttachmentIcon", true ) );

  // "Message Header Threading Options":
  int num = geometry.readNumEntry( "nestingPolicy", 3 );
  if ( num < 0 || num > 3 ) num = 3;
  mNestingPolicy->setButton( num );

  // "Date Display":
  setDateDisplay( general.readNumEntry( "dateFormat", DateFormatter::Fancy ),
                  general.readEntry( "customDateFormat" ) );
}

void AppearancePage::HeadersTab::setDateDisplay( int num, const TQString & format ) {
  DateFormatter::FormatType dateDisplay =
    static_cast<DateFormatter::FormatType>( num );

  // special case: needs text for the line edit:
  if ( dateDisplay == DateFormatter::Custom )
    mCustomDateFormatEdit->setText( format );

  for ( int i = 0 ; i < numDateDisplayConfig ; i++ )
    if ( dateDisplay == dateDisplayConfig[i].dateDisplay ) {
      mDateDisplay->setButton( i );
      return;
    }
  // fell through since none found:
  mDateDisplay->setButton( numDateDisplayConfig - 2 ); // default
}

void AppearancePage::HeadersTab::installProfile( KConfig * profile ) {
  KConfigGroup general( profile, "General" );
  KConfigGroup geometry( profile, "Geometry" );

  if ( geometry.hasKey( "nestedMessages" ) )
    mNestedMessagesCheck->setChecked( geometry.readBoolEntry( "nestedMessages" ) );
  if ( general.hasKey( "showMessageSize" ) )
    mMessageSizeCheck->setChecked( general.readBoolEntry( "showMessageSize" ) );

  if( general.hasKey( "showCryptoIcons" ) )
    mCryptoIconsCheck->setChecked( general.readBoolEntry( "showCryptoIcons" ) );
  if ( general.hasKey( "showAttachmentIcon" ) )
    mAttachmentCheck->setChecked( general.readBoolEntry( "showAttachmentIcon" ) );

  if ( geometry.hasKey( "nestingPolicy" ) ) {
    int num = geometry.readNumEntry( "nestingPolicy" );
    if ( num < 0 || num > 3 ) num = 3;
    mNestingPolicy->setButton( num );
  }

  if ( general.hasKey( "dateFormat" ) )
    setDateDisplay( general.readNumEntry( "dateFormat" ),
                   general.readEntry( "customDateFormat" ) );
}

void AppearancePage::HeadersTab::save() {
  KConfigGroup general( KMKernel::config(), "General" );
  KConfigGroup geometry( KMKernel::config(), "Geometry" );

  if ( geometry.readBoolEntry( "nestedMessages", false )
       != mNestedMessagesCheck->isChecked() ) {
    int result = KMessageBox::warningContinueCancel( this,
                   i18n("Changing the global threading setting will override "
                        "all folder specific values."),
                   TQString::null, KStdGuiItem::cont(), "threadOverride" );
    if ( result == KMessageBox::Continue ) {
      geometry.writeEntry( "nestedMessages", mNestedMessagesCheck->isChecked() );
      // remove all threadMessagesOverride keys from all [Folder-*] groups:
      TQStringList groups = KMKernel::config()->groupList().grep( TQRegExp("^Folder-") );
      kdDebug(5006) << "groups.count() == " << groups.count() << endl;
      for ( TQStringList::const_iterator it = groups.begin() ; it != groups.end() ; ++it ) {
        KConfigGroup group( KMKernel::config(), *it );
        group.deleteEntry( "threadMessagesOverride" );
      }
    }
  }

  geometry.writeEntry( "nestingPolicy",
                       mNestingPolicy->id( mNestingPolicy->selected() ) );
  general.writeEntry( "showMessageSize", mMessageSizeCheck->isChecked() );
  general.writeEntry( "showCryptoIcons", mCryptoIconsCheck->isChecked() );
  general.writeEntry( "showAttachmentIcon", mAttachmentCheck->isChecked() );

  int dateDisplayID = mDateDisplay->id( mDateDisplay->selected() );
  // check bounds:
  assert( dateDisplayID >= 0 ); assert( dateDisplayID < numDateDisplayConfig );
  general.writeEntry( "dateFormat",
                      dateDisplayConfig[ dateDisplayID ].dateDisplay );
  general.writeEntry( "customDateFormat", mCustomDateFormatEdit->text() );
}


//
// Message Window
//


static const BoolConfigEntry closeAfterReplyOrForward = {
  "Reader", "CloseAfterReplyOrForward", I18N_NOOP("Close message window after replying or forwarding"), false
};

static const BoolConfigEntry showColorbarMode = {
  "Reader", "showColorbar", I18N_NOOP("Show HTML stat&us bar"), false
};

static const BoolConfigEntry showSpamStatusMode = {
  "Reader", "showSpamStatus", I18N_NOOP("Show s&pam status in fancy headers"), true
};

static const BoolConfigEntry showEmoticons = {
  "Reader", "ShowEmoticons", I18N_NOOP("Replace smileys by emoticons"), true
};

static const BoolConfigEntry shrinkQuotes = {
  "Reader", "ShrinkQuotes", I18N_NOOP("Use smaller font for quoted text"), false
};

static const BoolConfigEntry showExpandQuotesMark= {
  "Reader", "ShowExpandQuotesMark", I18N_NOOP("Show expand/collapse quote marks"), false
};

static const BoolConfigEntry showCurrentTime = {
  "Reader", "ShowCurrentTime", I18N_NOOP("Show current sender time"), true
};

TQString AppearancePage::ReaderTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-reader");
}

AppearancePageReaderTab::AppearancePageReaderTab( TQWidget * parent,
                                                  const char * name )
  : ConfigModuleTab( parent, name )
{
  TQVBoxLayout *vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "close message window after replying or forwarding" checkbox
  populateCheckBox( mCloseAfterReplyOrForwardCheck = new TQCheckBox( this ),
                    closeAfterReplyOrForward );
  TQToolTip::add( mCloseAfterReplyOrForwardCheck,
                 i18n( "Close the standalone message window after replying or forwarding the message" ) );
  vlay->addWidget( mCloseAfterReplyOrForwardCheck );
  connect( mCloseAfterReplyOrForwardCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "show colorbar" check box:
  populateCheckBox( mShowColorbarCheck = new TQCheckBox( this ), showColorbarMode );
  vlay->addWidget( mShowColorbarCheck );
  connect( mShowColorbarCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "show spam status" check box;
  populateCheckBox( mShowSpamStatusCheck = new TQCheckBox( this ), showSpamStatusMode );
  vlay->addWidget( mShowSpamStatusCheck );
  connect( mShowSpamStatusCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "replace smileys by emoticons" check box;
  populateCheckBox( mShowEmoticonsCheck = new TQCheckBox( this ), showEmoticons );
  vlay->addWidget( mShowEmoticonsCheck );
  connect( mShowEmoticonsCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "Use smaller font for quoted text" check box
  mShrinkQuotesCheck = new TQCheckBox( i18n( shrinkQuotes.desc ), this,
                                      "kcfg_ShrinkQuotes" );
  vlay->addWidget( mShrinkQuotesCheck );
  connect( mShrinkQuotesCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // "Show expand/collaps quote marks" check box;
  TQHBoxLayout *hlay= new TQHBoxLayout( vlay ); // inherits spacing
  populateCheckBox( mShowExpandQuotesMark= new TQCheckBox( this ), showExpandQuotesMark);
  hlay->addWidget( mShowExpandQuotesMark);
  connect( mShowExpandQuotesMark, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  hlay->addStretch( 1 );
  mCollapseQuoteLevelSpin = new KIntSpinBox( 0/*min*/,10/*max*/,1/*step*/,
      3/*init*/,10/*base*/,this );

  TQLabel *label = new TQLabel( mCollapseQuoteLevelSpin,
           GlobalSettings::self()->collapseQuoteLevelSpinItem()->label(), this );

  hlay->addWidget( label );

  mCollapseQuoteLevelSpin->setEnabled( false ); //since !mShowExpandQuotesMark->isCheckec()
  connect(  mCollapseQuoteLevelSpin, TQT_SIGNAL( valueChanged( int ) ),
      this, TQT_SLOT( slotEmitChanged( void ) ) );
  hlay->addWidget( mCollapseQuoteLevelSpin);

  connect( mShowExpandQuotesMark, TQT_SIGNAL( toggled( bool ) ),
      mCollapseQuoteLevelSpin, TQT_SLOT( setEnabled( bool ) ) );

  // Fallback Character Encoding
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mCharsetCombo = new TQComboBox( this );
  mCharsetCombo->insertStringList( KMMsgBase::supportedEncodings( false ) );

  connect( mCharsetCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  TQString fallbackCharsetWhatsThis =
    i18n( GlobalSettings::self()->fallbackCharacterEncodingItem()->whatsThis().utf8() );
  TQWhatsThis::add( mCharsetCombo, fallbackCharsetWhatsThis );

  label = new TQLabel( i18n("Fallback ch&aracter encoding:"), this );
  label->setBuddy( mCharsetCombo );

  hlay->addWidget( label );
  hlay->addWidget( mCharsetCombo );

  // Override Character Encoding
  TQHBoxLayout *hlay2 = new TQHBoxLayout( vlay ); // inherits spacing
  mOverrideCharsetCombo = new TQComboBox( this );
  TQStringList encodings = KMMsgBase::supportedEncodings( false );
  encodings.prepend( i18n( "Auto" ) );
  mOverrideCharsetCombo->insertStringList( encodings );
  mOverrideCharsetCombo->setCurrentItem(0);

  connect( mOverrideCharsetCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  TQString overrideCharsetWhatsThis =
    i18n( GlobalSettings::self()->overrideCharacterEncodingItem()->whatsThis().utf8() );
  TQWhatsThis::add( mOverrideCharsetCombo, overrideCharsetWhatsThis );

  label = new TQLabel( i18n("&Override character encoding:"), this );
  label->setBuddy( mOverrideCharsetCombo );

  hlay2->addWidget( label );
  hlay2->addWidget( mOverrideCharsetCombo );

  populateCheckBox( mShowCurrentTimeCheck = new TQCheckBox( this ), showCurrentTime );
  vlay->addWidget( mShowCurrentTimeCheck );
  connect( mShowCurrentTimeCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  vlay->addStretch( 100 ); // spacer
}


void AppearancePage::ReaderTab::readCurrentFallbackCodec()
{
  TQStringList encodings = KMMsgBase::supportedEncodings( false );
  TQStringList::ConstIterator it( encodings.begin() );
  TQStringList::ConstIterator end( encodings.end() );
  TQString currentEncoding = GlobalSettings::self()->fallbackCharacterEncoding();
  currentEncoding = currentEncoding.replace( "iso ", "iso-", false );
  ///kdDebug(5006) << "Looking for encoding: " << currentEncoding << endl;
  int i = 0;
  int indexOfLatin9 = 0;
  bool found = false;
  for( ; it != end; ++it)
  {
    const TQString encoding = KGlobal::charsets()->encodingForName(*it);
    if ( encoding == "iso-8859-15" )
        indexOfLatin9 = i;
    if( encoding == currentEncoding )
    {
      mCharsetCombo->setCurrentItem( i );
      found = true;
      break;
    }
    i++;
  }
  if ( !found ) // nothing matched, use latin9
    mCharsetCombo->setCurrentItem( indexOfLatin9 );
}

void AppearancePage::ReaderTab::readCurrentOverrideCodec()
{
  const TQString &currentOverrideEncoding = GlobalSettings::self()->overrideCharacterEncoding();
  if ( currentOverrideEncoding.isEmpty() ) {
    mOverrideCharsetCombo->setCurrentItem( 0 );
    return;
  }
  TQStringList encodings = KMMsgBase::supportedEncodings( false );
  encodings.prepend( i18n( "Auto" ) );
  TQStringList::Iterator it( encodings.begin() );
  TQStringList::Iterator end( encodings.end() );
  uint i = 0;
  for( ; it != end; ++it)
  {
    if( KGlobal::charsets()->encodingForName(*it) == currentOverrideEncoding )
    {
      mOverrideCharsetCombo->setCurrentItem( i );
      break;
    }
    i++;
  }
  if ( i == encodings.size() ) {
    // the current value of overrideCharacterEncoding is an unknown encoding => reset to Auto
    kdWarning(5006) << "Unknown override character encoding \"" << currentOverrideEncoding
                    << "\". Resetting to Auto." << endl;
    mOverrideCharsetCombo->setCurrentItem( 0 );
    GlobalSettings::self()->setOverrideCharacterEncoding( TQString::null );
  }
}

void AppearancePage::ReaderTab::doLoadFromGlobalSettings()
{
  mCloseAfterReplyOrForwardCheck->setChecked( GlobalSettings::self()->closeAfterReplyOrForward() );
  mShowEmoticonsCheck->setChecked( GlobalSettings::self()->showEmoticons() );
  mShrinkQuotesCheck->setChecked( GlobalSettings::self()->shrinkQuotes() );
  mShowExpandQuotesMark->setChecked( GlobalSettings::self()->showExpandQuotesMark() );
  mCollapseQuoteLevelSpin->setValue( GlobalSettings::self()->collapseQuoteLevelSpin() );
  readCurrentFallbackCodec();
  readCurrentOverrideCodec();
  mShowCurrentTimeCheck->setChecked( GlobalSettings::self()->showCurrentTime() );
}

void AppearancePage::ReaderTab::doLoadOther()
{
  const KConfigGroup reader( KMKernel::config(), "Reader" );
  loadWidget( mShowColorbarCheck, reader, showColorbarMode );
  loadWidget( mShowSpamStatusCheck, reader, showSpamStatusMode );
}


void AppearancePage::ReaderTab::save() {
  KConfigGroup reader( KMKernel::config(), "Reader" );
  saveCheckBox( mShowColorbarCheck, reader, showColorbarMode );
  saveCheckBox( mShowSpamStatusCheck, reader, showSpamStatusMode );
  GlobalSettings::self()->setCloseAfterReplyOrForward( mCloseAfterReplyOrForwardCheck->isChecked() );
  GlobalSettings::self()->setShowEmoticons( mShowEmoticonsCheck->isChecked() );
  GlobalSettings::self()->setShrinkQuotes( mShrinkQuotesCheck->isChecked() );
  GlobalSettings::self()->setShowExpandQuotesMark( mShowExpandQuotesMark->isChecked() );

  GlobalSettings::self()->setCollapseQuoteLevelSpin( mCollapseQuoteLevelSpin->value() );
  GlobalSettings::self()->setFallbackCharacterEncoding(
      KGlobal::charsets()->encodingForName( mCharsetCombo->currentText() ) );
  GlobalSettings::self()->setOverrideCharacterEncoding(
      mOverrideCharsetCombo->currentItem() == 0 ?
        TQString() :
        KGlobal::charsets()->encodingForName( mOverrideCharsetCombo->currentText() ) );
  GlobalSettings::self()->setShowCurrentTime( mShowCurrentTimeCheck->isChecked() );
}


void AppearancePage::ReaderTab::installProfile( KConfig * /* profile */ ) {
  const KConfigGroup reader( KMKernel::config(), "Reader" );
  loadProfile( mCloseAfterReplyOrForwardCheck, reader, closeAfterReplyOrForward );
  loadProfile( mShowColorbarCheck, reader, showColorbarMode );
  loadProfile( mShowSpamStatusCheck, reader, showSpamStatusMode );
  loadProfile( mShowEmoticonsCheck, reader, showEmoticons );
  loadProfile( mShrinkQuotesCheck, reader, shrinkQuotes );
  loadProfile( mShowExpandQuotesMark, reader, showExpandQuotesMark);
  loadProfile( mShowCurrentTimeCheck, reader, showCurrentTime );
}


TQString AppearancePage::SystemTrayTab::helpAnchor() const {
  return TQString::fromLatin1("configure-appearance-systemtray");
}

AppearancePageSystemTrayTab::AppearancePageSystemTrayTab( TQWidget * parent,
                                                          const char * name )
  : ConfigModuleTab( parent, name )
{
  TQVBoxLayout * vlay = new TQVBoxLayout( this, KDialog::marginHint(),
                                        KDialog::spacingHint() );

  // "Enable system tray applet" check box
  mSystemTrayCheck = new TQCheckBox( i18n("Enable system tray icon"), this );
  vlay->addWidget( mSystemTrayCheck );
  connect( mSystemTrayCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // System tray modes
  mSystemTrayGroup = new TQVButtonGroup( i18n("System Tray Mode"), this );
  mSystemTrayGroup->layout()->setSpacing( KDialog::spacingHint() );
  vlay->addWidget( mSystemTrayGroup );
  connect( mSystemTrayGroup, TQT_SIGNAL( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mSystemTrayCheck, TQT_SIGNAL( toggled( bool ) ),
           mSystemTrayGroup, TQT_SLOT( setEnabled( bool ) ) );

  mSystemTrayGroup->insert( new TQRadioButton( i18n("Always show KMail in system tray"), mSystemTrayGroup ),
                            GlobalSettings::EnumSystemTrayPolicy::ShowAlways );

  mSystemTrayGroup->insert( new TQRadioButton( i18n("Only show KMail in system tray if there are unread messages"), mSystemTrayGroup ),
                            GlobalSettings::EnumSystemTrayPolicy::ShowOnUnread );

  vlay->addStretch( 10 ); // spacer
}

void AppearancePage::SystemTrayTab::doLoadFromGlobalSettings() {
  mSystemTrayCheck->setChecked( GlobalSettings::self()->systemTrayEnabled() );
  mSystemTrayGroup->setButton( GlobalSettings::self()->systemTrayPolicy() );
  mSystemTrayGroup->setEnabled( mSystemTrayCheck->isChecked() );
}

void AppearancePage::SystemTrayTab::installProfile( KConfig * profile ) {
  KConfigGroup general( profile, "General" );

  if ( general.hasKey( "SystemTrayEnabled" ) ) {
    mSystemTrayCheck->setChecked( general.readBoolEntry( "SystemTrayEnabled" ) );
  }
  if ( general.hasKey( "SystemTrayPolicy" ) ) {
    mSystemTrayGroup->setButton( general.readNumEntry( "SystemTrayPolicy" ) );
  }
  mSystemTrayGroup->setEnabled( mSystemTrayCheck->isChecked() );
}

void AppearancePage::SystemTrayTab::save() {
  GlobalSettings::self()->setSystemTrayEnabled( mSystemTrayCheck->isChecked() );
  GlobalSettings::self()->setSystemTrayPolicy( mSystemTrayGroup->id( mSystemTrayGroup->selected() ) );
}


// *************************************************************
// *                                                           *
// *                      ComposerPage                         *
// *                                                           *
// *************************************************************

TQString ComposerPage::helpAnchor() const {
  return TQString::fromLatin1("configure-composer");
}

ComposerPage::ComposerPage( TQWidget * parent, const char * name )
  : ConfigModuleWithTabs( parent, name )
{
  //
  // "General" tab:
  //
  mGeneralTab = new GeneralTab();
  addTab( mGeneralTab, i18n("&General") );
  addConfig( GlobalSettings::self(), mGeneralTab );

  //
  // "Phrases" tab:
  //
  // mPhrasesTab = new PhrasesTab();
  // addTab( mPhrasesTab, i18n("&Phrases") );

  //
  // "Templates" tab:
  //
  mTemplatesTab = new TemplatesTab();
  addTab( mTemplatesTab, i18n("&Templates") );

  //
  // "Custom Templates" tab:
  //
  mCustomTemplatesTab = new CustomTemplatesTab();
  addTab( mCustomTemplatesTab, i18n("&Custom Templates") );

  //
  // "Subject" tab:
  //
  mSubjectTab = new SubjectTab();
  addTab( mSubjectTab, i18n("&Subject") );
  addConfig( GlobalSettings::self(), mSubjectTab );

  //
  // "Charset" tab:
  //
  mCharsetTab = new CharsetTab();
  addTab( mCharsetTab, i18n("Cha&rset") );

  //
  // "Headers" tab:
  //
  mHeadersTab = new HeadersTab();
  addTab( mHeadersTab, i18n("H&eaders") );

  //
  // "Attachments" tab:
  //
  mAttachmentsTab = new AttachmentsTab();
  addTab( mAttachmentsTab, i18n("Config->Composer->Attachments", "A&ttachments") );
  load();
}

TQString ComposerPage::GeneralTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-general");
}

ComposerPageGeneralTab::ComposerPageGeneralTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQHBoxLayout *hlay;
  TQGroupBox   *group;
  TQLabel      *label;
  TQHBox       *hbox;
  TQString      msg;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // some check buttons...
  mAutoAppSignFileCheck = new TQCheckBox(
           GlobalSettings::self()->autoTextSignatureItem()->label(),
           this );
  vlay->addWidget( mAutoAppSignFileCheck );
  connect( mAutoAppSignFileCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mTopQuoteCheck =
    new TQCheckBox( GlobalSettings::self()->prependSignatureItem()->label(), this );
  vlay->addWidget( mTopQuoteCheck);
  connect( mTopQuoteCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mSmartQuoteCheck = new TQCheckBox(
           GlobalSettings::self()->smartQuoteItem()->label(),
           this, "kcfg_SmartQuote" );
  TQToolTip::add( mSmartQuoteCheck,
                 i18n( "When replying, add quote signs in front of all lines of the quoted text,\n"
                       "even when the line was created by adding an additional linebreak while\n"
                       "word-wrapping the text." ) );
  vlay->addWidget( mSmartQuoteCheck );
  connect( mSmartQuoteCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mQuoteSelectionOnlyCheck = new TQCheckBox( GlobalSettings::self()->quoteSelectionOnlyItem()->label(),
                                            this, "kcfg_QuoteSelectionOnly" );
  TQToolTip::add( mQuoteSelectionOnlyCheck,
                 i18n( "When replying, only quote the selected text instead of the complete message "
                       "when there is text selected in the message window." ) );
  vlay->addWidget( mQuoteSelectionOnlyCheck );
  connect( mQuoteSelectionOnlyCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged(void) ) );

  mStripSignatureCheck = new TQCheckBox( GlobalSettings::self()->stripSignatureItem()->label(),
                                        this, "kcfg_StripSignature" );
  vlay->addWidget( mStripSignatureCheck );
  connect( mStripSignatureCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mAutoRequestMDNCheck = new TQCheckBox(
           GlobalSettings::self()->requestMDNItem()->label(),
           this, "kcfg_RequestMDN" );
  vlay->addWidget( mAutoRequestMDNCheck );
  connect( mAutoRequestMDNCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mShowRecentAddressesInComposer = new TQCheckBox(
           GlobalSettings::self()->showRecentAddressesInComposerItem()->label(),
           this, "kcfg_ShowRecentAddressesInComposer" );
  vlay->addWidget( mShowRecentAddressesInComposer );
  connect( mShowRecentAddressesInComposer, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // a checkbox for "word wrap" and a spinbox for the column in
  // which to wrap:
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mWordWrapCheck = new TQCheckBox(
           GlobalSettings::self()->wordWrapItem()->label(),
           this, "kcfg_WordWrap" );
  hlay->addWidget( mWordWrapCheck );
  connect( mWordWrapCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mWrapColumnSpin = new KIntSpinBox( 30/*min*/, 78/*max*/, 1/*step*/,
           78/*init*/, 10 /*base*/, this, "kcfg_LineWrapWidth" );
  mWrapColumnSpin->setEnabled( false ); // since !mWordWrapCheck->isChecked()
  connect( mWrapColumnSpin, TQT_SIGNAL( valueChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hlay->addWidget( mWrapColumnSpin );
  hlay->addStretch( 1 );
  // only enable the spinbox if the checkbox is checked:
  connect( mWordWrapCheck, TQT_SIGNAL(toggled(bool)),
           mWrapColumnSpin, TQT_SLOT(setEnabled(bool)) );

  // a checkbox for "too many recipient warning" and a spinbox for the recipient threshold
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mRecipientCheck = new TQCheckBox(
           GlobalSettings::self()->tooManyRecipientsItem()->label(),
           this, "kcfg_TooManyRecipients" );
  hlay->addWidget( mRecipientCheck );
  connect( mRecipientCheck, TQT_SIGNAL( stateChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  TQString recipientCheckWhatsthis =
    i18n( GlobalSettings::self()->tooManyRecipientsItem()->whatsThis().utf8() );
  TQWhatsThis::add( mRecipientCheck, recipientCheckWhatsthis );
  TQToolTip::add( mRecipientCheck,
                 i18n( "Warn if too many recipients are specified" ) );

  mRecipientSpin = new KIntSpinBox( 1/*min*/, 100/*max*/, 1/*step*/,
           5/*init*/, 10 /*base*/, this, "kcfg_RecipientThreshold" );
  mRecipientSpin->setEnabled( false );
  connect( mRecipientSpin, TQT_SIGNAL( valueChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  TQString recipientWhatsthis =
    i18n( GlobalSettings::self()->recipientThresholdItem()->whatsThis().utf8() );
  TQWhatsThis::add( mRecipientSpin, recipientWhatsthis );
  TQToolTip::add( mRecipientSpin,
                 i18n( "Warn if more than this many recipients are specified" ) );


  hlay->addWidget( mRecipientSpin );
  hlay->addStretch( 1 );
  // only enable the spinbox if the checkbox is checked:
  connect( mRecipientCheck, TQT_SIGNAL(toggled(bool)),
           mRecipientSpin, TQT_SLOT(setEnabled(bool)) );


  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mAutoSave = new KIntSpinBox( 0, 60, 1, 1, 10, this, "kcfg_AutosaveInterval" );
  label = new TQLabel( mAutoSave,
           GlobalSettings::self()->autosaveIntervalItem()->label(), this );
  hlay->addWidget( label );
  hlay->addWidget( mAutoSave );
  mAutoSave->setSpecialValueText( i18n("No autosave") );
  mAutoSave->setSuffix( i18n(" min") );
  hlay->addStretch( 1 );
  connect( mAutoSave, TQT_SIGNAL( valueChanged(int) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mForwardTypeCombo = new KComboBox( false, this );
  label = new TQLabel( mForwardTypeCombo,
                      i18n( "Default Forwarding Type:" ),
                      this );
  mForwardTypeCombo->insertStringList( TQStringList()
                                       << i18n( "Inline" )
                                       << i18n( "As Attachment" ) );
  hlay->addWidget( label );
  hlay->addWidget( mForwardTypeCombo );
  hlay->addStretch( 1 );
  connect( mForwardTypeCombo, TQT_SIGNAL(activated(int)),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  TQPushButton *completionOrderBtn = new TQPushButton( i18n( "Configure Completion Order" ), this );
  connect( completionOrderBtn, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( slotConfigureCompletionOrder() ) );
  hlay->addWidget( completionOrderBtn );
  hlay->addItem( new TQSpacerItem(0, 0) );

  // recent addresses
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  TQPushButton *recentAddressesBtn = new TQPushButton( i18n( "Edit Recent Addresses..." ), this );
  connect( recentAddressesBtn, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( slotConfigureRecentAddresses() ) );
  hlay->addWidget( recentAddressesBtn );
  hlay->addItem( new TQSpacerItem(0, 0) );

  // The "external editor" group:
  group = new TQVGroupBox( i18n("External Editor"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mExternalEditorCheck = new TQCheckBox(
           GlobalSettings::self()->useExternalEditorItem()->label(),
           group, "kcfg_UseExternalEditor" );
  connect( mExternalEditorCheck, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hbox = new TQHBox( group );
  label = new TQLabel( GlobalSettings::self()->externalEditorItem()->label(),
                   hbox );
  mEditorRequester = new KURLRequester( hbox, "kcfg_ExternalEditor" );
  connect( mEditorRequester, TQT_SIGNAL( urlSelected(const TQString&) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mEditorRequester, TQT_SIGNAL( textChanged(const TQString&) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hbox->setStretchFactor( mEditorRequester, 1 );
  label->setBuddy( mEditorRequester );
  label->setEnabled( false ); // since !mExternalEditorCheck->isChecked()
  // ### FIXME: allow only executables (x-bit when available..)
  mEditorRequester->setFilter( "application/x-executable "
                               "application/x-shellscript "
                               "application/x-desktop" );
  mEditorRequester->setEnabled( false ); // !mExternalEditorCheck->isChecked()
  connect( mExternalEditorCheck, TQT_SIGNAL(toggled(bool)),
           label, TQT_SLOT(setEnabled(bool)) );
  connect( mExternalEditorCheck, TQT_SIGNAL(toggled(bool)),
           mEditorRequester, TQT_SLOT(setEnabled(bool)) );

  label = new TQLabel( i18n("<b>%f</b> will be replaced with the "
                           "filename to edit."), group );
  label->setEnabled( false ); // see above
  connect( mExternalEditorCheck, TQT_SIGNAL(toggled(bool)),
           label, TQT_SLOT(setEnabled(bool)) );

  vlay->addWidget( group );
  vlay->addStretch( 100 );
}

void ComposerPage::GeneralTab::doLoadFromGlobalSettings() {
  // various check boxes:

  mAutoAppSignFileCheck->setChecked(
           GlobalSettings::self()->autoTextSignature()=="auto" );
  mTopQuoteCheck->setChecked( GlobalSettings::self()->prependSignature() );
  mSmartQuoteCheck->setChecked( GlobalSettings::self()->smartQuote() );
  mQuoteSelectionOnlyCheck->setChecked( GlobalSettings::self()->quoteSelectionOnly() );
  mStripSignatureCheck->setChecked( GlobalSettings::self()->stripSignature() );
  mAutoRequestMDNCheck->setChecked( GlobalSettings::self()->requestMDN() );
  mWordWrapCheck->setChecked( GlobalSettings::self()->wordWrap() );

  mWrapColumnSpin->setValue( GlobalSettings::self()->lineWrapWidth() );
  mRecipientCheck->setChecked( GlobalSettings::self()->tooManyRecipients() );
  mRecipientSpin->setValue( GlobalSettings::self()->recipientThreshold() );
  mAutoSave->setValue( GlobalSettings::self()->autosaveInterval() );
  if ( GlobalSettings::self()->forwardingInlineByDefault() )
    mForwardTypeCombo->setCurrentItem( 0 );
  else
    mForwardTypeCombo->setCurrentItem( 1 );

  // editor group:
  mExternalEditorCheck->setChecked( GlobalSettings::self()->useExternalEditor() );
  mEditorRequester->setURL( GlobalSettings::self()->externalEditor() );
}

void ComposerPage::GeneralTab::installProfile( KConfig * profile ) {
  KConfigGroup composer( profile, "Composer" );
  KConfigGroup general( profile, "General" );

  if ( composer.hasKey( "signature" ) ) {
    bool state = composer.readBoolEntry("signature");
    mAutoAppSignFileCheck->setChecked( state );
  }
  if ( composer.hasKey( "prepend-signature" ) )
    mTopQuoteCheck->setChecked( composer.readBoolEntry( "prepend-signature" ) );
  if ( composer.hasKey( "smart-quote" ) )
    mSmartQuoteCheck->setChecked( composer.readBoolEntry( "smart-quote" ) );
  if ( composer.hasKey( "StripSignature" ) )
    mStripSignatureCheck->setChecked( composer.readBoolEntry( "StripSignature" ) );
  if ( composer.hasKey( "QuoteSelectionOnly" ) )
    mQuoteSelectionOnlyCheck->setChecked( composer.readBoolEntry( "QuoteSelectionOnly" ) );
  if ( composer.hasKey( "request-mdn" ) )
    mAutoRequestMDNCheck->setChecked( composer.readBoolEntry( "request-mdn" ) );
  if ( composer.hasKey( "word-wrap" ) )
    mWordWrapCheck->setChecked( composer.readBoolEntry( "word-wrap" ) );
  if ( composer.hasKey( "break-at" ) )
    mWrapColumnSpin->setValue( composer.readNumEntry( "break-at" ) );
  if ( composer.hasKey( "too-many-recipients" ) )
    mRecipientCheck->setChecked( composer.readBoolEntry( "too-many-recipients" ) );
  if ( composer.hasKey( "recipient-threshold" ) )
    mRecipientSpin->setValue( composer.readNumEntry( "recipient-threshold" ) );
  if ( composer.hasKey( "autosave" ) )
    mAutoSave->setValue( composer.readNumEntry( "autosave" ) );

  if ( general.hasKey( "use-external-editor" )
       && general.hasKey( "external-editor" ) ) {
    mExternalEditorCheck->setChecked( general.readBoolEntry( "use-external-editor" ) );
    mEditorRequester->setURL( general.readPathEntry( "external-editor" ) );
  }
}

void ComposerPage::GeneralTab::save() {
  GlobalSettings::self()->setAutoTextSignature(
         mAutoAppSignFileCheck->isChecked() ? "auto" : "manual" );
  GlobalSettings::self()->setPrependSignature( mTopQuoteCheck->isChecked());
  GlobalSettings::self()->setSmartQuote( mSmartQuoteCheck->isChecked() );
  GlobalSettings::self()->setQuoteSelectionOnly( mQuoteSelectionOnlyCheck->isChecked() );
  GlobalSettings::self()->setStripSignature( mStripSignatureCheck->isChecked() );
  GlobalSettings::self()->setRequestMDN( mAutoRequestMDNCheck->isChecked() );
  GlobalSettings::self()->setWordWrap( mWordWrapCheck->isChecked() );

  GlobalSettings::self()->setLineWrapWidth( mWrapColumnSpin->value() );
  GlobalSettings::self()->setTooManyRecipients( mRecipientCheck->isChecked() );
  GlobalSettings::self()->setRecipientThreshold( mRecipientSpin->value() );
  GlobalSettings::self()->setAutosaveInterval( mAutoSave->value() );
  GlobalSettings::self()->setForwardingInlineByDefault( mForwardTypeCombo->currentItem() == 0 );

  // editor group:
  GlobalSettings::self()->setUseExternalEditor( mExternalEditorCheck->isChecked() );
  GlobalSettings::self()->setExternalEditor( mEditorRequester->url() );
}

void ComposerPage::GeneralTab::slotConfigureRecentAddresses( )
{
  KRecentAddress::RecentAddressDialog dlg( this );
  dlg.setAddresses( RecentAddresses::self( KMKernel::config() )->addresses() );
  if ( dlg.exec() ) {
    RecentAddresses::self( KMKernel::config() )->clear();
    const TQStringList &addrList = dlg.addresses();
    TQStringList::ConstIterator it;
    for ( it = addrList.constBegin(); it != addrList.constEnd(); ++it )
      RecentAddresses::self( KMKernel::config() )->add( *it );
  }
}

void ComposerPage::GeneralTab::slotConfigureCompletionOrder( )
{
  KPIM::LdapSearch search;
  KPIM::CompletionOrderEditor editor( &search, this );
  editor.exec();
}

TQString ComposerPage::PhrasesTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-phrases");
}

ComposerPagePhrasesTab::ComposerPagePhrasesTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQGridLayout *glay;
  TQPushButton *button;

  glay = new TQGridLayout( this, 7, 3, KDialog::spacingHint() );
  glay->setMargin( KDialog::marginHint() );
  glay->setColStretch( 1, 1 );
  glay->setColStretch( 2, 1 );
  glay->setRowStretch( 7, 1 );

  // row 0: help text
  glay->addMultiCellWidget( new TQLabel( i18n("<qt>The following placeholders are "
                                             "supported in the reply phrases:<br>"
                                             "<b>%D</b>: date, <b>%S</b>: subject,<br>"
                                             "<b>%e</b>: sender's address, <b>%F</b>: sender's name, <b>%f</b>: sender's initials,<br>"
                                             "<b>%T</b>: recipient's name, <b>%t</b>: recipient's name and address,<br>"
                                             "<b>%C</b>: carbon copy names, <b>%c</b>: carbon copy names and addresses,<br>"
                                             "<b>%%</b>: percent sign, <b>%_</b>: space, "
                                             "<b>%L</b>: linebreak</qt>"), this ),
                            0, 0, 0, 2 ); // row 0; cols 0..2

  // row 1: label and language combo box:
  mPhraseLanguageCombo = new LanguageComboBox( false, this );
  glay->addWidget( new TQLabel( mPhraseLanguageCombo,
                               i18n("Lang&uage:"), this ), 1, 0 );
  glay->addMultiCellWidget( mPhraseLanguageCombo, 1, 1, 1, 2 );
  connect( mPhraseLanguageCombo, TQT_SIGNAL(activated(const TQString&)),
           this, TQT_SLOT(slotLanguageChanged(const TQString&)) );

  // row 2: "add..." and "remove" push buttons:
  button = new TQPushButton( i18n("A&dd..."), this );
  button->setAutoDefault( false );
  glay->addWidget( button, 2, 1 );
  mRemoveButton = new TQPushButton( i18n("Re&move"), this );
  mRemoveButton->setAutoDefault( false );
  mRemoveButton->setEnabled( false ); // combo doesn't contain anything...
  glay->addWidget( mRemoveButton, 2, 2 );
  connect( button, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotNewLanguage()) );
  connect( mRemoveButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRemoveLanguage()) );

  // row 3: "reply to sender" line edit and label:
  mPhraseReplyEdit = new KLineEdit( this );
  connect( mPhraseReplyEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  glay->addWidget( new TQLabel( mPhraseReplyEdit,
                               i18n("Reply to se&nder:"), this ), 3, 0 );
  glay->addMultiCellWidget( mPhraseReplyEdit, 3, 3, 1, 2 ); // cols 1..2

  // row 4: "reply to all" line edit and label:
  mPhraseReplyAllEdit = new KLineEdit( this );
  connect( mPhraseReplyAllEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  glay->addWidget( new TQLabel( mPhraseReplyAllEdit,
                               i18n("Repl&y to all:"), this ), 4, 0 );
  glay->addMultiCellWidget( mPhraseReplyAllEdit, 4, 4, 1, 2 ); // cols 1..2

  // row 5: "forward" line edit and label:
  mPhraseForwardEdit = new KLineEdit( this );
  connect( mPhraseForwardEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  glay->addWidget( new TQLabel( mPhraseForwardEdit,
                               i18n("&Forward:"), this ), 5, 0 );
  glay->addMultiCellWidget( mPhraseForwardEdit, 5, 5, 1, 2 ); // cols 1..2

  // row 6: "quote indicator" line edit and label:
  mPhraseIndentPrefixEdit = new KLineEdit( this );
  connect( mPhraseIndentPrefixEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  glay->addWidget( new TQLabel( mPhraseIndentPrefixEdit,
                               i18n("&Quote indicator:"), this ), 6, 0 );
  glay->addMultiCellWidget( mPhraseIndentPrefixEdit, 6, 6, 1, 2 );

  // row 7: spacer
}


void ComposerPage::PhrasesTab::setLanguageItemInformation( int index ) {
  assert( 0 <= index && index < (int)mLanguageList.count() );

  LanguageItem &l = *mLanguageList.at( index );

  mPhraseReplyEdit->setText( l.mReply );
  mPhraseReplyAllEdit->setText( l.mReplyAll );
  mPhraseForwardEdit->setText( l.mForward );
  mPhraseIndentPrefixEdit->setText( l.mIndentPrefix );
}

void ComposerPage::PhrasesTab::saveActiveLanguageItem() {
  int index = mActiveLanguageItem;
  if (index == -1) return;
  assert( 0 <= index && index < (int)mLanguageList.count() );

  LanguageItem &l = *mLanguageList.at( index );

  l.mReply = mPhraseReplyEdit->text();
  l.mReplyAll = mPhraseReplyAllEdit->text();
  l.mForward = mPhraseForwardEdit->text();
  l.mIndentPrefix = mPhraseIndentPrefixEdit->text();
}

void ComposerPage::PhrasesTab::slotNewLanguage()
{
  NewLanguageDialog dialog( mLanguageList, parentWidget(), "New", true );
  if ( dialog.exec() == TQDialog::Accepted ) slotAddNewLanguage( dialog.language() );
}

void ComposerPage::PhrasesTab::slotAddNewLanguage( const TQString& lang )
{
  mPhraseLanguageCombo->setCurrentItem(
    mPhraseLanguageCombo->insertLanguage( lang ) );
  KLocale locale("kmail");
  locale.setLanguage( lang );
  mLanguageList.append(
     LanguageItem( lang,
                   locale.translate("On %D, you wrote:"),
                   locale.translate("On %D, %F wrote:"),
                   locale.translate("Forwarded Message"),
                   locale.translate(">%_") ) );
  mRemoveButton->setEnabled( true );
  slotLanguageChanged( TQString::null );
}

void ComposerPage::PhrasesTab::slotRemoveLanguage()
{
  assert( mPhraseLanguageCombo->count() > 1 );
  int index = mPhraseLanguageCombo->currentItem();
  assert( 0 <= index && index < (int)mLanguageList.count() );

  // remove current item from internal list and combobox:
  mLanguageList.remove( mLanguageList.at( index ) );
  mPhraseLanguageCombo->removeItem( index );

  if ( index >= (int)mLanguageList.count() ) index--;

  mActiveLanguageItem = index;
  setLanguageItemInformation( index );
  mRemoveButton->setEnabled( mLanguageList.count() > 1 );
  emit changed( true );
}

void ComposerPage::PhrasesTab::slotLanguageChanged( const TQString& )
{
  int index = mPhraseLanguageCombo->currentItem();
  assert( index < (int)mLanguageList.count() );
  saveActiveLanguageItem();
  mActiveLanguageItem = index;
  setLanguageItemInformation( index );
  emit changed( true );
}


void ComposerPage::PhrasesTab::doLoadFromGlobalSettings() {
  mLanguageList.clear();
  mPhraseLanguageCombo->clear();
  mActiveLanguageItem = -1;

  int numLang = GlobalSettings::self()->replyLanguagesCount();
  int currentNr = GlobalSettings::self()->replyCurrentLanguage();

  // build mLanguageList and mPhraseLanguageCombo:
  for ( int i = 0 ; i < numLang ; i++ ) {
    ReplyPhrases replyPhrases( TQString::number(i) );
    replyPhrases.readConfig();
    TQString lang = replyPhrases.language();
    mLanguageList.append(
         LanguageItem( lang,
                       replyPhrases.phraseReplySender(),
                       replyPhrases.phraseReplyAll(),
                       replyPhrases.phraseForward(),
                       replyPhrases.indentPrefix() ) );
    mPhraseLanguageCombo->insertLanguage( lang );
  }

  if ( currentNr >= numLang || currentNr < 0 )
    currentNr = 0;

  if ( numLang == 0 ) {
    slotAddNewLanguage( KGlobal::locale()->language() );
  }

  mPhraseLanguageCombo->setCurrentItem( currentNr );
  mActiveLanguageItem = currentNr;
  setLanguageItemInformation( currentNr );
  mRemoveButton->setEnabled( mLanguageList.count() > 1 );
}

void ComposerPage::PhrasesTab::save() {
  GlobalSettings::self()->setReplyLanguagesCount( mLanguageList.count() );
  GlobalSettings::self()->setReplyCurrentLanguage( mPhraseLanguageCombo->currentItem() );

  saveActiveLanguageItem();
  LanguageItemList::Iterator it = mLanguageList.begin();
  for ( int i = 0 ; it != mLanguageList.end() ; ++it, ++i ) {
    ReplyPhrases replyPhrases( TQString::number(i) );
    replyPhrases.setLanguage( (*it).mLanguage );
    replyPhrases.setPhraseReplySender( (*it).mReply );
    replyPhrases.setPhraseReplyAll( (*it).mReplyAll );
    replyPhrases.setPhraseForward( (*it).mForward );
    replyPhrases.setIndentPrefix( (*it).mIndentPrefix );
    replyPhrases.writeConfig();
  }
}

TQString ComposerPage::TemplatesTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-templates");
}

ComposerPageTemplatesTab::ComposerPageTemplatesTab( TQWidget * parent, const char * name )
  : ConfigModuleTab ( parent, name )
{
  TQVBoxLayout* vlay = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  mWidget = new TemplatesConfiguration( this );
  vlay->addWidget( mWidget );

  connect( mWidget, TQT_SIGNAL( changed() ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
}

void ComposerPage::TemplatesTab::doLoadFromGlobalSettings() {
    mWidget->loadFromGlobal();
}

void ComposerPage::TemplatesTab::save() {
    mWidget->saveToGlobal();
}

TQString ComposerPage::CustomTemplatesTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-custom-templates");
}

ComposerPageCustomTemplatesTab::ComposerPageCustomTemplatesTab( TQWidget * parent, const char * name )
  : ConfigModuleTab ( parent, name )
{
  TQVBoxLayout* vlay = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  mWidget = new CustomTemplates( this );
  vlay->addWidget( mWidget );

  connect( mWidget, TQT_SIGNAL( changed() ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
}

void ComposerPage::CustomTemplatesTab::doLoadFromGlobalSettings() {
    mWidget->load();
}

void ComposerPage::CustomTemplatesTab::save() {
    mWidget->save();
}

TQString ComposerPage::SubjectTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-subject");
}

ComposerPageSubjectTab::ComposerPageSubjectTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQGroupBox   *group;
  TQLabel      *label;


  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  group = new TQVGroupBox( i18n("Repl&y Subject Prefixes"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  // row 0: help text:
  label = new TQLabel( i18n("Recognize any sequence of the following prefixes\n"
                           "(entries are case-insensitive regular expressions):"), group );
  label->setAlignment( AlignLeft|WordBreak );

  // row 1, string list editor:
  SimpleStringListEditor::ButtonCode buttonCode =
    static_cast<SimpleStringListEditor::ButtonCode>( SimpleStringListEditor::Add | SimpleStringListEditor::Remove | SimpleStringListEditor::Modify );
  mReplyListEditor =
    new SimpleStringListEditor( group, 0, buttonCode,
                                i18n("A&dd..."), i18n("Re&move"),
                                i18n("Mod&ify..."),
                                i18n("Enter new reply prefix:") );
  connect( mReplyListEditor, TQT_SIGNAL( changed( void ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // row 2: "replace [...]" check box:
  mReplaceReplyPrefixCheck = new TQCheckBox(
     GlobalSettings::self()->replaceReplyPrefixItem()->label(),
     group, "kcfg_ReplaceReplyPrefix" );
  connect( mReplaceReplyPrefixCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  vlay->addWidget( group );


  group = new TQVGroupBox( i18n("For&ward Subject Prefixes"), this );
  group->layout()->setSpacing( KDialog::marginHint() );

  // row 0: help text:
  label= new TQLabel( i18n("Recognize any sequence of the following prefixes\n"
                          "(entries are case-insensitive regular expressions):"), group );
  label->setAlignment( AlignLeft|WordBreak );

  // row 1: string list editor
  mForwardListEditor =
    new SimpleStringListEditor( group, 0, buttonCode,
                                i18n("Add..."),
                                i18n("Remo&ve"),
                                i18n("Modify..."),
                                i18n("Enter new forward prefix:") );
  connect( mForwardListEditor, TQT_SIGNAL( changed( void ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // row 3: "replace [...]" check box:
  mReplaceForwardPrefixCheck = new TQCheckBox(
       GlobalSettings::self()->replaceForwardPrefixItem()->label(),
       group, "kcfg_ReplaceForwardPrefix" );
  connect( mReplaceForwardPrefixCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  vlay->addWidget( group );
}

void ComposerPage::SubjectTab::doLoadFromGlobalSettings() {
  mReplyListEditor->setStringList( GlobalSettings::self()->replyPrefixes() );
  mReplaceReplyPrefixCheck->setChecked( GlobalSettings::self()->replaceReplyPrefix() );
  mForwardListEditor->setStringList( GlobalSettings::self()->forwardPrefixes() );
  mReplaceForwardPrefixCheck->setChecked( GlobalSettings::self()->replaceForwardPrefix() );
}

void ComposerPage::SubjectTab::save() {
  GlobalSettings::self()->setReplyPrefixes( mReplyListEditor->stringList() );
  GlobalSettings::self()->setForwardPrefixes( mForwardListEditor->stringList() );
}

TQString ComposerPage::CharsetTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-charset");
}

ComposerPageCharsetTab::ComposerPageCharsetTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQLabel      *label;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  label = new TQLabel( i18n("This list is checked for every outgoing message "
                           "from the top to the bottom for a charset that "
                           "contains all required characters."), this );
  label->setAlignment( WordBreak);
  vlay->addWidget( label );

  mCharsetListEditor =
    new SimpleStringListEditor( this, 0, SimpleStringListEditor::All,
                                i18n("A&dd..."), i18n("Remo&ve"),
                                i18n("&Modify..."), i18n("Enter charset:") );
  connect( mCharsetListEditor, TQT_SIGNAL( changed( void ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  vlay->addWidget( mCharsetListEditor, 1 );

  mKeepReplyCharsetCheck = new TQCheckBox( i18n("&Keep original charset when "
                                                "replying or forwarding (if "
                                                "possible)"), this );
  connect( mKeepReplyCharsetCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  vlay->addWidget( mKeepReplyCharsetCheck );

  connect( mCharsetListEditor, TQT_SIGNAL(aboutToAdd(TQString&)),
           this, TQT_SLOT(slotVerifyCharset(TQString&)) );
}

void ComposerPage::CharsetTab::slotVerifyCharset( TQString & charset ) {
  if ( charset.isEmpty() ) return;

  // KCharsets::codecForName("us-ascii") returns "iso-8859-1" (cf. Bug #49812)
  // therefore we have to treat this case specially
  if ( charset.lower() == TQString::fromLatin1("us-ascii") ) {
    charset = TQString::fromLatin1("us-ascii");
    return;
  }

  if ( charset.lower() == TQString::fromLatin1("locale") ) {
    charset =  TQString::fromLatin1("%1 (locale)")
      .arg( TQCString( kmkernel->networkCodec()->mimeName() ).lower() );
    return;
  }

  bool ok = false;
  TQTextCodec *codec = KGlobal::charsets()->codecForName( charset, ok );
  if ( ok && codec ) {
    charset = TQString::fromLatin1( codec->mimeName() ).lower();
    return;
  }

  KMessageBox::sorry( this, i18n("This charset is not supported.") );
  charset = TQString::null;
}

void ComposerPage::CharsetTab::doLoadOther() {
  KConfigGroup composer( KMKernel::config(), "Composer" );

  TQStringList charsets = composer.readListEntry( "pref-charsets" );
  for ( TQStringList::Iterator it = charsets.begin() ;
        it != charsets.end() ; ++it )
    if ( (*it) == TQString::fromLatin1("locale") ) {
      TQCString cset = kmkernel->networkCodec()->mimeName();
      KPIM::kAsciiToLower( cset.data() );
      (*it) = TQString("%1 (locale)").arg( cset );
    }

  mCharsetListEditor->setStringList( charsets );
  mKeepReplyCharsetCheck->setChecked( !composer.readBoolEntry( "force-reply-charset", false ) );
}

void ComposerPage::CharsetTab::save() {
  KConfigGroup composer( KMKernel::config(), "Composer" );

  TQStringList charsetList = mCharsetListEditor->stringList();
  TQStringList::Iterator it = charsetList.begin();
  for ( ; it != charsetList.end() ; ++it )
    if ( (*it).endsWith("(locale)") )
      (*it) = "locale";
  composer.writeEntry( "pref-charsets", charsetList );
  composer.writeEntry( "force-reply-charset",
                       !mKeepReplyCharsetCheck->isChecked() );
}

TQString ComposerPage::HeadersTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-headers");
}

ComposerPageHeadersTab::ComposerPageHeadersTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQHBoxLayout *hlay;
  TQGridLayout *glay;
  TQLabel      *label;
  TQPushButton *button;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "Use custom Message-Id suffix" checkbox:
  mCreateOwnMessageIdCheck =
    new TQCheckBox( i18n("&Use custom message-id suffix"), this );
  connect( mCreateOwnMessageIdCheck, TQT_SIGNAL ( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  vlay->addWidget( mCreateOwnMessageIdCheck );

  // "Message-Id suffix" line edit and label:
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mMessageIdSuffixEdit = new KLineEdit( this );
  // only ASCII letters, digits, plus, minus and dots are allowed
  mMessageIdSuffixValidator =
    new TQRegExpValidator( TQRegExp( "[a-zA-Z0-9+-]+(?:\\.[a-zA-Z0-9+-]+)*" ), this );
  mMessageIdSuffixEdit->setValidator( mMessageIdSuffixValidator );
  label = new TQLabel( mMessageIdSuffixEdit,
                      i18n("Custom message-&id suffix:"), this );
  label->setEnabled( false ); // since !mCreateOwnMessageIdCheck->isChecked()
  mMessageIdSuffixEdit->setEnabled( false );
  hlay->addWidget( label );
  hlay->addWidget( mMessageIdSuffixEdit, 1 );
  connect( mCreateOwnMessageIdCheck, TQT_SIGNAL(toggled(bool) ),
           label, TQT_SLOT(setEnabled(bool)) );
  connect( mCreateOwnMessageIdCheck, TQT_SIGNAL(toggled(bool) ),
           mMessageIdSuffixEdit, TQT_SLOT(setEnabled(bool)) );
  connect( mMessageIdSuffixEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // horizontal rule and "custom header fields" label:
  vlay->addWidget( new KSeparator( KSeparator::HLine, this ) );
  vlay->addWidget( new TQLabel( i18n("Define custom mime header fields:"), this) );

  // "custom header fields" listbox:
  glay = new TQGridLayout( vlay, 5, 3 ); // inherits spacing
  glay->setRowStretch( 2, 1 );
  glay->setColStretch( 1, 1 );
  mTagList = new ListView( this, "tagList" );
  mTagList->addColumn( i18n("Name") );
  mTagList->addColumn( i18n("Value") );
  mTagList->setAllColumnsShowFocus( true );
  mTagList->setSorting( -1 );
  connect( mTagList, TQT_SIGNAL(selectionChanged()),
           this, TQT_SLOT(slotMimeHeaderSelectionChanged()) );
  glay->addMultiCellWidget( mTagList, 0, 2, 0, 1 );

  // "new" and "remove" buttons:
  button = new TQPushButton( i18n("Ne&w"), this );
  connect( button, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotNewMimeHeader()) );
  button->setAutoDefault( false );
  glay->addWidget( button, 0, 2 );
  mRemoveHeaderButton = new TQPushButton( i18n("Re&move"), this );
  connect( mRemoveHeaderButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotRemoveMimeHeader()) );
  button->setAutoDefault( false );
  glay->addWidget( mRemoveHeaderButton, 1, 2 );

  // "name" and "value" line edits and labels:
  mTagNameEdit = new KLineEdit( this );
  mTagNameEdit->setEnabled( false );
  mTagNameLabel = new TQLabel( mTagNameEdit, i18n("&Name:"), this );
  mTagNameLabel->setEnabled( false );
  glay->addWidget( mTagNameLabel, 3, 0 );
  glay->addWidget( mTagNameEdit, 3, 1 );
  connect( mTagNameEdit, TQT_SIGNAL(textChanged(const TQString&)),
           this, TQT_SLOT(slotMimeHeaderNameChanged(const TQString&)) );

  mTagValueEdit = new KLineEdit( this );
  mTagValueEdit->setEnabled( false );
  mTagValueLabel = new TQLabel( mTagValueEdit, i18n("&Value:"), this );
  mTagValueLabel->setEnabled( false );
  glay->addWidget( mTagValueLabel, 4, 0 );
  glay->addWidget( mTagValueEdit, 4, 1 );
  connect( mTagValueEdit, TQT_SIGNAL(textChanged(const TQString&)),
           this, TQT_SLOT(slotMimeHeaderValueChanged(const TQString&)) );
}

void ComposerPage::HeadersTab::slotMimeHeaderSelectionChanged()
{
  TQListViewItem * item = mTagList->selectedItem();

  if ( item ) {
    mTagNameEdit->setText( item->text( 0 ) );
    mTagValueEdit->setText( item->text( 1 ) );
  } else {
    mTagNameEdit->clear();
    mTagValueEdit->clear();
  }
  mRemoveHeaderButton->setEnabled( item );
  mTagNameEdit->setEnabled( item );
  mTagValueEdit->setEnabled( item );
  mTagNameLabel->setEnabled( item );
  mTagValueLabel->setEnabled( item );
}


void ComposerPage::HeadersTab::slotMimeHeaderNameChanged( const TQString & text ) {
  // is called on ::setup(), when clearing the line edits. So be
  // prepared to not find a selection:
  TQListViewItem * item = mTagList->selectedItem();
  if ( item )
    item->setText( 0, text );
  emit changed( true );
}


void ComposerPage::HeadersTab::slotMimeHeaderValueChanged( const TQString & text ) {
  // is called on ::setup(), when clearing the line edits. So be
  // prepared to not find a selection:
  TQListViewItem * item = mTagList->selectedItem();
  if ( item )
    item->setText( 1, text );
  emit changed( true );
}


void ComposerPage::HeadersTab::slotNewMimeHeader()
{
  TQListViewItem *listItem = new TQListViewItem( mTagList );
  mTagList->setCurrentItem( listItem );
  mTagList->setSelected( listItem, true );
  emit changed( true );
}


void ComposerPage::HeadersTab::slotRemoveMimeHeader()
{
  // calling this w/o selection is a programming error:
  TQListViewItem * item = mTagList->selectedItem();
  if ( !item ) {
    kdDebug(5006) << "==================================================\n"
                  << "Error: Remove button was pressed although no custom header was selected\n"
                  << "==================================================\n";
    return;
  }

  TQListViewItem * below = item->nextSibling();
  delete item;

  if ( below )
    mTagList->setSelected( below, true );
  else if ( mTagList->lastItem() )
    mTagList->setSelected( mTagList->lastItem(), true );
  emit changed( true );
}

void ComposerPage::HeadersTab::doLoadOther() {
  KConfigGroup general( KMKernel::config(), "General" );

  TQString suffix = general.readEntry( "myMessageIdSuffix" );
  mMessageIdSuffixEdit->setText( suffix );
  bool state = ( !suffix.isEmpty() &&
            general.readBoolEntry( "useCustomMessageIdSuffix", false ) );
  mCreateOwnMessageIdCheck->setChecked( state );

  mTagList->clear();
  mTagNameEdit->clear();
  mTagValueEdit->clear();

  TQListViewItem * item = 0;

  int count = general.readNumEntry( "mime-header-count", 0 );
  for( int i = 0 ; i < count ; i++ ) {
    KConfigGroup config( KMKernel::config(),
                         TQCString("Mime #") + TQCString().setNum(i) );
    TQString name  = config.readEntry( "name" );
    TQString value = config.readEntry( "value" );
    if( !name.isEmpty() )
      item = new TQListViewItem( mTagList, item, name, value );
  }
  if ( mTagList->childCount() ) {
    mTagList->setCurrentItem( mTagList->firstChild() );
    mTagList->setSelected( mTagList->firstChild(), true );
  }
  else {
    // disable the "Remove" button
    mRemoveHeaderButton->setEnabled( false );
  }
}

void ComposerPage::HeadersTab::save() {
  KConfigGroup general( KMKernel::config(), "General" );

  general.writeEntry( "useCustomMessageIdSuffix",
                      mCreateOwnMessageIdCheck->isChecked() );
  general.writeEntry( "myMessageIdSuffix",
                      mMessageIdSuffixEdit->text() );

  int numValidEntries = 0;
  TQListViewItem * item = mTagList->firstChild();
  for ( ; item ; item = item->itemBelow() )
    if( !item->text(0).isEmpty() ) {
      KConfigGroup config( KMKernel::config(), TQCString("Mime #")
                             + TQCString().setNum( numValidEntries ) );
      config.writeEntry( "name",  item->text( 0 ) );
      config.writeEntry( "value", item->text( 1 ) );
      numValidEntries++;
    }
  general.writeEntry( "mime-header-count", numValidEntries );
}

TQString ComposerPage::AttachmentsTab::helpAnchor() const {
  return TQString::fromLatin1("configure-composer-attachments");
}

ComposerPageAttachmentsTab::ComposerPageAttachmentsTab( TQWidget * parent,
                                                        const char * name )
  : ConfigModuleTab( parent, name ) {
  // tmp. vars:
  TQVBoxLayout *vlay;
  TQLabel      *label;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "Outlook compatible attachment naming" check box
  mOutlookCompatibleCheck =
    new TQCheckBox( i18n( "Outlook-compatible attachment naming" ), this );
  mOutlookCompatibleCheck->setChecked( false );
  TQToolTip::add( mOutlookCompatibleCheck, i18n(
    "Turn this option on to make Outlook(tm) understand attachment names "
    "containing non-English characters" ) );
  connect( mOutlookCompatibleCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mOutlookCompatibleCheck, TQT_SIGNAL( clicked() ),
           this, TQT_SLOT( slotOutlookCompatibleClicked() ) );
  vlay->addWidget( mOutlookCompatibleCheck );
  vlay->addSpacing( 5 );

  // "Enable detection of missing attachments" check box
  mMissingAttachmentDetectionCheck =
    new TQCheckBox( i18n("E&nable detection of missing attachments"), this );
  mMissingAttachmentDetectionCheck->setChecked( true );
  connect( mMissingAttachmentDetectionCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  vlay->addWidget( mMissingAttachmentDetectionCheck );

  // "Attachment key words" label and string list editor
  label = new TQLabel( i18n("Recognize any of the following key words as "
                           "intention to attach a file:"), this );
  label->setAlignment( AlignLeft|WordBreak );
  vlay->addWidget( label );

  SimpleStringListEditor::ButtonCode buttonCode =
    static_cast<SimpleStringListEditor::ButtonCode>( SimpleStringListEditor::Add | SimpleStringListEditor::Remove | SimpleStringListEditor::Modify );
  mAttachWordsListEditor =
    new SimpleStringListEditor( this, 0, buttonCode,
                                i18n("A&dd..."), i18n("Re&move"),
                                i18n("Mod&ify..."),
                                i18n("Enter new key word:") );
  connect( mAttachWordsListEditor, TQT_SIGNAL( changed( void ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  vlay->addWidget( mAttachWordsListEditor );

  connect( mMissingAttachmentDetectionCheck, TQT_SIGNAL(toggled(bool) ),
           label, TQT_SLOT(setEnabled(bool)) );
  connect( mMissingAttachmentDetectionCheck, TQT_SIGNAL(toggled(bool) ),
           mAttachWordsListEditor, TQT_SLOT(setEnabled(bool)) );
}

void ComposerPage::AttachmentsTab::doLoadFromGlobalSettings() {
  mOutlookCompatibleCheck->setChecked(
    GlobalSettings::self()->outlookCompatibleAttachments() );
  mMissingAttachmentDetectionCheck->setChecked(
    GlobalSettings::self()->showForgottenAttachmentWarning() );
  TQStringList attachWordsList = GlobalSettings::self()->attachmentKeywords();
  if ( attachWordsList.isEmpty() ) {
    // default value
    attachWordsList << TQString::fromLatin1("attachment")
                    << TQString::fromLatin1("attached");
    if ( TQString::fromLatin1("attachment") != i18n("attachment") )
      attachWordsList << i18n("attachment");
    if ( TQString::fromLatin1("attached") != i18n("attached") )
      attachWordsList << i18n("attached");
  }

  mAttachWordsListEditor->setStringList( attachWordsList );
}

void ComposerPage::AttachmentsTab::save() {
  GlobalSettings::self()->setOutlookCompatibleAttachments(
    mOutlookCompatibleCheck->isChecked() );
  GlobalSettings::self()->setShowForgottenAttachmentWarning(
    mMissingAttachmentDetectionCheck->isChecked() );
  GlobalSettings::self()->setAttachmentKeywords(
    mAttachWordsListEditor->stringList() );
}

void ComposerPageAttachmentsTab::slotOutlookCompatibleClicked()
{
  if (mOutlookCompatibleCheck->isChecked()) {
    KMessageBox::information(0,i18n("You have chosen to "
    "encode attachment names containing non-English characters in a way that "
    "is understood by Outlook(tm) and other mail clients that do not "
    "support standard-compliant encoded attachment names.\n"
    "Note that KMail may create non-standard compliant messages, "
    "and consequently it is possible that your messages will not be "
    "understood by standard-compliant mail clients; so, unless you have no "
    "other choice, you should not enable this option." ) );
  }
}

// *************************************************************
// *                                                           *
// *                      SecurityPage                         *
// *                                                           *
// *************************************************************
TQString SecurityPage::helpAnchor() const {
  return TQString::fromLatin1("configure-security");
}

SecurityPage::SecurityPage( TQWidget * parent, const char * name )
  : ConfigModuleWithTabs( parent, name )
{
  //
  // "Reading" tab:
  //
  mGeneralTab = new GeneralTab(); //  @TODO: rename
  addTab( mGeneralTab, i18n("&Reading") );

  //
  // "Composing" tab:
  //
  mComposerCryptoTab = new ComposerCryptoTab();
  addTab( mComposerCryptoTab, i18n("Composing") );

  //
  // "Warnings" tab:
  //
  mWarningTab = new WarningTab();
  addTab( mWarningTab, i18n("Warnings") );

  //
  // "S/MIME Validation" tab:
  //
  mSMimeTab = new SMimeTab();
  addTab( mSMimeTab, i18n("S/MIME &Validation") );

  //
  // "Crypto Backends" tab:
  //
  mCryptPlugTab = new CryptPlugTab();
  addTab( mCryptPlugTab, i18n("Crypto Backe&nds") );
  load();
}


void SecurityPage::installProfile( KConfig * profile ) {
  mGeneralTab->installProfile( profile );
  mComposerCryptoTab->installProfile( profile );
  mWarningTab->installProfile( profile );
  mSMimeTab->installProfile( profile );
}

TQString SecurityPage::GeneralTab::helpAnchor() const {
  return TQString::fromLatin1("configure-security-reading");
}

SecurityPageGeneralTab::SecurityPageGeneralTab( TQWidget * parent, const char * name )
  : ConfigModuleTab ( parent, name )
{
  // tmp. vars:
  TQVBoxLayout  *vlay;
  TQHBox        *hbox;
  TQGroupBox    *group;
  TQRadioButton *radio;
  KActiveLabel *label;
  TQWidget      *w;
  TQString       msg;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // QWhat'sThis texts
  TQString htmlWhatsThis = i18n( "<qt><p>Messages sometimes come in both formats. "
              "This option controls whether you want the HTML part or the plain "
              "text part to be displayed.</p>"
              "<p>Displaying the HTML part makes the message look better, "
              "but at the same time increases the risk of security holes "
              "being exploited.</p>"
              "<p>Displaying the plain text part loses much of the message's "
              "formatting, but makes it almost <em>impossible</em> "
              "to exploit security holes in the HTML renderer (Konqueror).</p>"
              "<p>The option below guards against one common misuse of HTML "
              "messages, but it cannot guard against security issues that were "
              "not known at the time this version of KMail was written.</p>"
              "<p>It is therefore advisable to <em>not</em> prefer HTML to "
              "plain text.</p>"
              "<p><b>Note:</b> You can set this option on a per-folder basis "
              "from the <i>Folder</i> menu of KMail's main window.</p></qt>" );

  TQString externalWhatsThis = i18n( "<qt><p>Some mail advertisements are in HTML "
              "and contain references to, for example, images that the advertisers"
              " employ to find out that you have read their message "
              "(&quot;web bugs&quot;).</p>"
              "<p>There is no valid reason to load images off the Internet like "
              "this, since the sender can always attach the required images "
              "directly to the message.</p>"
              "<p>To guard from such a misuse of the HTML displaying feature "
              "of KMail, this option is <em>disabled</em> by default.</p>"
              "<p>However, if you wish to, for example, view images in HTML "
              "messages that were not attached to it, you can enable this "
              "option, but you should be aware of the possible problem.</p></qt>" );

  TQString receiptWhatsThis = i18n( "<qt><h3>Message Disposition "
              "Notification Policy</h3>"
              "<p>MDNs are a generalization of what is commonly called <b>read "
              "receipt</b>. The message author requests a disposition "
              "notification to be sent and the receiver's mail program "
              "generates a reply from which the author can learn what "
              "happened to his message. Common disposition types include "
              "<b>displayed</b> (i.e. read), <b>deleted</b> and <b>dispatched</b> "
              "(e.g. forwarded).</p>"
              "<p>The following options are available to control KMail's "
              "sending of MDNs:</p>"
              "<ul>"
              "<li><em>Ignore</em>: Ignores any request for disposition "
              "notifications. No MDN will ever be sent automatically "
              "(recommended).</li>"
              "<li><em>Ask</em>: Answers requests only after asking the user "
              "for permission. This way, you can send MDNs for selected "
              "messages while denying or ignoring them for others.</li>"
              "<li><em>Deny</em>: Always sends a <b>denied</b> notification. This "
              "is only <em>slightly</em> better than always sending MDNs. "
              "The author will still know that the messages has been acted "
              "upon, he just cannot tell whether it was deleted or read etc.</li>"
              "<li><em>Always send</em>: Always sends the requested "
              "disposition notification. That means that the author of the "
              "message gets to know when the message was acted upon and, "
              "in addition, what happened to it (displayed, deleted, "
              "etc.). This option is strongly discouraged, but since it "
              "makes much sense e.g. for customer relationship management, "
              "it has been made available.</li>"
              "</ul></qt>" );


  // "HTML Messages" group box:
  group = new TQVGroupBox( i18n( "HTML Messages" ), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mHtmlMailCheck = new TQCheckBox( i18n("Prefer H&TML to plain text"), group );
  TQWhatsThis::add( mHtmlMailCheck, htmlWhatsThis );
  connect( mHtmlMailCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  mExternalReferences = new TQCheckBox( i18n("Allow messages to load e&xternal "
                                            "references from the Internet" ), group );
  TQWhatsThis::add( mExternalReferences, externalWhatsThis );
  connect( mExternalReferences, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  label = new KActiveLabel( i18n("<b>WARNING:</b> Allowing HTML in email may "
                           "increase the risk that your system will be "
                           "compromised by present and anticipated security "
                           "exploits. <a href=\"whatsthis:%1\">More about "
                           "HTML mails...</a> <a href=\"whatsthis:%2\">More "
                           "about external references...</a>")
                           .arg(htmlWhatsThis).arg(externalWhatsThis),
                           group );

  vlay->addWidget( group );

  // encrypted messages group
  group = new TQVGroupBox( i18n("Encrypted Messages"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );
  mAlwaysDecrypt = new TQCheckBox( i18n( "Attempt decryption of encrypted messages when viewing" ), group );
  connect( mAlwaysDecrypt, TQT_SIGNAL(stateChanged(int)), this, TQT_SLOT(slotEmitChanged()) );
  vlay->addWidget( group );

  // "Message Disposition Notification" groupbox:
  group = new TQVGroupBox( i18n("Message Disposition Notifications"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );


  // "ignore", "ask", "deny", "always send" radiobutton line:
  mMDNGroup = new TQButtonGroup( group );
  mMDNGroup->hide();
  connect( mMDNGroup, TQT_SIGNAL( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  hbox = new TQHBox( group );
  hbox->setSpacing( KDialog::spacingHint() );

  (void)new TQLabel( i18n("Send policy:"), hbox );

  radio = new TQRadioButton( i18n("&Ignore"), hbox );
  mMDNGroup->insert( radio );

  radio = new TQRadioButton( i18n("As&k"), hbox );
  mMDNGroup->insert( radio );

  radio = new TQRadioButton( i18n("&Deny"), hbox );
  mMDNGroup->insert( radio );

  radio = new TQRadioButton( i18n("Al&ways send"), hbox );
  mMDNGroup->insert( radio );

  for ( int i = 0 ; i < mMDNGroup->count() ; ++i )
      TQWhatsThis::add( mMDNGroup->find( i ), receiptWhatsThis );

  w = new TQWidget( hbox ); // spacer
  hbox->setStretchFactor( w, 1 );

  // "Original Message quote" radiobutton line:
  mOrigQuoteGroup = new TQButtonGroup( group );
  mOrigQuoteGroup->hide();
  connect( mOrigQuoteGroup, TQT_SIGNAL( clicked( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hbox = new TQHBox( group );
  hbox->setSpacing( KDialog::spacingHint() );

  (void)new TQLabel( i18n("Quote original message:"), hbox );

  radio = new TQRadioButton( i18n("Nothin&g"), hbox );
  mOrigQuoteGroup->insert( radio );

  radio = new TQRadioButton( i18n("&Full message"), hbox );
  mOrigQuoteGroup->insert( radio );

  radio = new TQRadioButton( i18n("Onl&y headers"), hbox );
  mOrigQuoteGroup->insert( radio );

  w = new TQWidget( hbox );
  hbox->setStretchFactor( w, 1 );

  mNoMDNsWhenEncryptedCheck = new TQCheckBox( i18n("Do not send MDNs in response to encrypted messages"), group );
  connect( mNoMDNsWhenEncryptedCheck, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );

  // Warning label:
  label = new KActiveLabel( i18n("<b>WARNING:</b> Unconditionally returning "
                           "confirmations undermines your privacy. "
                           "<a href=\"whatsthis:%1\">More...</a>")
                             .arg(receiptWhatsThis),
                           group );

  vlay->addWidget( group );

  // "Attached keys" group box:
  group = new TQVGroupBox( i18n( "Certificate && Key Bundle Attachments" ), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  mAutomaticallyImportAttachedKeysCheck = new TQCheckBox( i18n("Automatically import keys and certificates"), group );
  connect( mAutomaticallyImportAttachedKeysCheck, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );

  vlay->addWidget( group );



  vlay->addStretch( 10 ); // spacer
}

void SecurityPage::GeneralTab::doLoadOther() {
  const KConfigGroup reader( KMKernel::config(), "Reader" );

  mHtmlMailCheck->setChecked( reader.readBoolEntry( "htmlMail", false ) );
  mExternalReferences->setChecked( reader.readBoolEntry( "htmlLoadExternal", false ) );
  mAutomaticallyImportAttachedKeysCheck->setChecked( reader.readBoolEntry( "AutoImportKeys", false ) );

  mAlwaysDecrypt->setChecked( GlobalSettings::self()->alwaysDecrypt() );

  const KConfigGroup mdn( KMKernel::config(), "MDN" );

  int num = mdn.readNumEntry( "default-policy", 0 );
  if ( num < 0 || num >= mMDNGroup->count() ) num = 0;
  mMDNGroup->setButton( num );
  num = mdn.readNumEntry( "quote-message", 0 );
  if ( num < 0 || num >= mOrigQuoteGroup->count() ) num = 0;
  mOrigQuoteGroup->setButton( num );
  mNoMDNsWhenEncryptedCheck->setChecked(mdn.readBoolEntry( "not-send-when-encrypted", true ));
}

void SecurityPage::GeneralTab::installProfile( KConfig * profile ) {
  const KConfigGroup reader( profile, "Reader" );
  const KConfigGroup mdn( profile, "MDN" );

  if ( reader.hasKey( "htmlMail" ) )
    mHtmlMailCheck->setChecked( reader.readBoolEntry( "htmlMail" ) );
  if ( reader.hasKey( "htmlLoadExternal" ) )
    mExternalReferences->setChecked( reader.readBoolEntry( "htmlLoadExternal" ) );
  if ( reader.hasKey( "AutoImportKeys" ) )
    mAutomaticallyImportAttachedKeysCheck->setChecked( reader.readBoolEntry( "AutoImportKeys" ) );

  if ( mdn.hasKey( "default-policy" ) ) {
      int num = mdn.readNumEntry( "default-policy" );
      if ( num < 0 || num >= mMDNGroup->count() ) num = 0;
      mMDNGroup->setButton( num );
  }
  if ( mdn.hasKey( "quote-message" ) ) {
      int num = mdn.readNumEntry( "quote-message" );
      if ( num < 0 || num >= mOrigQuoteGroup->count() ) num = 0;
      mOrigQuoteGroup->setButton( num );
  }
  if ( mdn.hasKey( "not-send-when-encrypted" ) )
      mNoMDNsWhenEncryptedCheck->setChecked(mdn.readBoolEntry( "not-send-when-encrypted" ));
}

void SecurityPage::GeneralTab::save() {
  KConfigGroup reader( KMKernel::config(), "Reader" );
  KConfigGroup mdn( KMKernel::config(), "MDN" );

  if (reader.readBoolEntry( "htmlMail", false ) != mHtmlMailCheck->isChecked())
  {
    if (KMessageBox::warningContinueCancel(this, i18n("Changing the global "
      "HTML setting will override all folder specific values."), TQString::null,
      KStdGuiItem::cont(), "htmlMailOverride") == KMessageBox::Continue)
    {
      reader.writeEntry( "htmlMail", mHtmlMailCheck->isChecked() );
      TQStringList names;
      TQValueList<TQGuardedPtr<KMFolder> > folders;
      kmkernel->folderMgr()->createFolderList(&names, &folders);
      kmkernel->imapFolderMgr()->createFolderList(&names, &folders);
      kmkernel->dimapFolderMgr()->createFolderList(&names, &folders);
      kmkernel->searchFolderMgr()->createFolderList(&names, &folders);
      for (TQValueList<TQGuardedPtr<KMFolder> >::iterator it = folders.begin();
        it != folders.end(); ++it)
      {
        if (*it)
        {
          KConfigGroupSaver saver(KMKernel::config(),
            "Folder-" + (*it)->idString());
          KMKernel::config()->writeEntry("htmlMailOverride", false);
        }
      }
    }
  }
  reader.writeEntry( "htmlLoadExternal", mExternalReferences->isChecked() );
  reader.writeEntry( "AutoImportKeys", mAutomaticallyImportAttachedKeysCheck->isChecked() );
  mdn.writeEntry( "default-policy", mMDNGroup->id( mMDNGroup->selected() ) );
  mdn.writeEntry( "quote-message", mOrigQuoteGroup->id( mOrigQuoteGroup->selected() ) );
  mdn.writeEntry( "not-send-when-encrypted", mNoMDNsWhenEncryptedCheck->isChecked() );
  GlobalSettings::self()->setAlwaysDecrypt( mAlwaysDecrypt->isChecked() );
}


TQString SecurityPage::ComposerCryptoTab::helpAnchor() const {
  return TQString::fromLatin1("configure-security-composing");
}

SecurityPageComposerCryptoTab::SecurityPageComposerCryptoTab( TQWidget * parent, const char * name )
  : ConfigModuleTab ( parent, name )
{
  // the margins are inside mWidget itself
  TQVBoxLayout* vlay = new TQVBoxLayout( this, 0, 0 );

  mWidget = new ComposerCryptoConfiguration( this );
  connect( mWidget->mAutoSignature, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mEncToSelf, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mShowEncryptionResult, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mShowKeyApprovalDlg, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mAutoEncrypt, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mNeverEncryptWhenSavingInDrafts, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mStoreEncrypted, TQT_SIGNAL( toggled(bool) ), this, TQT_SLOT( slotEmitChanged() ) );
  vlay->addWidget( mWidget );
}

void SecurityPage::ComposerCryptoTab::doLoadOther() {
  const KConfigGroup composer( KMKernel::config(), "Composer" );

  // If you change default values, sync messagecomposer.cpp too

  mWidget->mAutoSignature->setChecked( composer.readBoolEntry( "pgp-auto-sign", false ) );

  mWidget->mEncToSelf->setChecked( composer.readBoolEntry( "crypto-encrypt-to-self", true ) );
  mWidget->mShowEncryptionResult->setChecked( false ); //composer.readBoolEntry( "crypto-show-encryption-result", true ) );
  mWidget->mShowEncryptionResult->hide();
  mWidget->mShowKeyApprovalDlg->setChecked( composer.readBoolEntry( "crypto-show-keys-for-approval", true ) );

  mWidget->mAutoEncrypt->setChecked( composer.readBoolEntry( "pgp-auto-encrypt", false ) );
  mWidget->mNeverEncryptWhenSavingInDrafts->setChecked( composer.readBoolEntry( "never-encrypt-drafts", true ) );

  mWidget->mStoreEncrypted->setChecked( composer.readBoolEntry( "crypto-store-encrypted", true ) );
}

void SecurityPage::ComposerCryptoTab::installProfile( KConfig * profile ) {
  const KConfigGroup composer( profile, "Composer" );

  if ( composer.hasKey( "pgp-auto-sign" ) )
    mWidget->mAutoSignature->setChecked( composer.readBoolEntry( "pgp-auto-sign" ) );

  if ( composer.hasKey( "crypto-encrypt-to-self" ) )
    mWidget->mEncToSelf->setChecked( composer.readBoolEntry( "crypto-encrypt-to-self" ) );
  if ( composer.hasKey( "crypto-show-encryption-result" ) )
    mWidget->mShowEncryptionResult->setChecked( composer.readBoolEntry( "crypto-show-encryption-result" ) );
  if ( composer.hasKey( "crypto-show-keys-for-approval" ) )
    mWidget->mShowKeyApprovalDlg->setChecked( composer.readBoolEntry( "crypto-show-keys-for-approval" ) );
  if ( composer.hasKey( "pgp-auto-encrypt" ) )
    mWidget->mAutoEncrypt->setChecked( composer.readBoolEntry( "pgp-auto-encrypt" ) );
  if ( composer.hasKey( "never-encrypt-drafts" ) )
    mWidget->mNeverEncryptWhenSavingInDrafts->setChecked( composer.readBoolEntry( "never-encrypt-drafts" ) );

  if ( composer.hasKey( "crypto-store-encrypted" ) )
    mWidget->mStoreEncrypted->setChecked( composer.readBoolEntry( "crypto-store-encrypted" ) );
}

void SecurityPage::ComposerCryptoTab::save() {
  KConfigGroup composer( KMKernel::config(), "Composer" );

  composer.writeEntry( "pgp-auto-sign", mWidget->mAutoSignature->isChecked() );

  composer.writeEntry( "crypto-encrypt-to-self", mWidget->mEncToSelf->isChecked() );
  composer.writeEntry( "crypto-show-encryption-result", mWidget->mShowEncryptionResult->isChecked() );
  composer.writeEntry( "crypto-show-keys-for-approval", mWidget->mShowKeyApprovalDlg->isChecked() );

  composer.writeEntry( "pgp-auto-encrypt", mWidget->mAutoEncrypt->isChecked() );
  composer.writeEntry( "never-encrypt-drafts", mWidget->mNeverEncryptWhenSavingInDrafts->isChecked() );

  composer.writeEntry( "crypto-store-encrypted", mWidget->mStoreEncrypted->isChecked() );
}

TQString SecurityPage::WarningTab::helpAnchor() const {
  return TQString::fromLatin1("configure-security-warnings");
}

SecurityPageWarningTab::SecurityPageWarningTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // the margins are inside mWidget itself
  TQVBoxLayout* vlay = new TQVBoxLayout( this, 0, 0 );

  mWidget = new WarningConfiguration( this );
  vlay->addWidget( mWidget );

  connect( mWidget->warnGroupBox, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  connect( mWidget->mWarnUnsigned, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  connect( mWidget->warnUnencryptedCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  connect( mWidget->warnReceiverNotInCertificateCB, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  connect( mWidget->mWarnSignKeyExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mWarnSignChainCertExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mWarnSignRootCertExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );

  connect( mWidget->mWarnEncrKeyExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mWarnEncrChainCertExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->mWarnEncrRootCertExpiresSB, TQT_SIGNAL( valueChanged( int ) ), TQT_SLOT( slotEmitChanged() ) );

  connect( mWidget->enableAllWarningsPB, TQT_SIGNAL(clicked()),
           TQT_SLOT(slotReenableAllWarningsClicked()) );
}

void SecurityPage::WarningTab::doLoadOther() {
  const KConfigGroup composer( KMKernel::config(), "Composer" );

  mWidget->warnUnencryptedCB->setChecked( composer.readBoolEntry( "crypto-warning-unencrypted", false ) );
  mWidget->mWarnUnsigned->setChecked( composer.readBoolEntry( "crypto-warning-unsigned", false ) );
  mWidget->warnReceiverNotInCertificateCB->setChecked( composer.readBoolEntry( "crypto-warn-recv-not-in-cert", true ) );

  // The "-int" part of the key name is because there used to be a separate boolean
  // config entry for enabling/disabling. This is done with the single bool value now.
  mWidget->warnGroupBox->setChecked( composer.readBoolEntry( "crypto-warn-when-near-expire", true ) );

  mWidget->mWarnSignKeyExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-key-near-expire-int", 14 ) );
  mWidget->mWarnSignChainCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-chaincert-near-expire-int", 14 ) );
  mWidget->mWarnSignRootCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-root-near-expire-int", 14 ) );

  mWidget->mWarnEncrKeyExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-key-near-expire-int", 14 ) );
  mWidget->mWarnEncrChainCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-chaincert-near-expire-int", 14 ) );
  mWidget->mWarnEncrRootCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-root-near-expire-int", 14 ) );

  mWidget->enableAllWarningsPB->setEnabled( true );
}

void SecurityPage::WarningTab::installProfile( KConfig * profile ) {
  const KConfigGroup composer( profile, "Composer" );

  if ( composer.hasKey( "crypto-warning-unencrypted" ) )
    mWidget->warnUnencryptedCB->setChecked( composer.readBoolEntry( "crypto-warning-unencrypted" ) );
  if ( composer.hasKey( "crypto-warning-unsigned" ) )
    mWidget->mWarnUnsigned->setChecked( composer.readBoolEntry( "crypto-warning-unsigned" ) );
  if ( composer.hasKey( "crypto-warn-recv-not-in-cert" ) )
    mWidget->warnReceiverNotInCertificateCB->setChecked( composer.readBoolEntry( "crypto-warn-recv-not-in-cert" ) );

  if ( composer.hasKey( "crypto-warn-when-near-expire" ) )
    mWidget->warnGroupBox->setChecked( composer.readBoolEntry( "crypto-warn-when-near-expire" ) );

  if ( composer.hasKey( "crypto-warn-sign-key-near-expire-int" ) )
    mWidget->mWarnSignKeyExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-key-near-expire-int" ) );
  if ( composer.hasKey( "crypto-warn-sign-chaincert-near-expire-int" ) )
    mWidget->mWarnSignChainCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-chaincert-near-expire-int" ) );
  if ( composer.hasKey( "crypto-warn-sign-root-near-expire-int" ) )
    mWidget->mWarnSignRootCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-sign-root-near-expire-int" ) );

  if ( composer.hasKey( "crypto-warn-encr-key-near-expire-int" ) )
    mWidget->mWarnEncrKeyExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-key-near-expire-int" ) );
  if ( composer.hasKey( "crypto-warn-encr-chaincert-near-expire-int" ) )
    mWidget->mWarnEncrChainCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-chaincert-near-expire-int" ) );
  if ( composer.hasKey( "crypto-warn-encr-root-near-expire-int" ) )
    mWidget->mWarnEncrRootCertExpiresSB->setValue( composer.readNumEntry( "crypto-warn-encr-root-near-expire-int" ) );
}

void SecurityPage::WarningTab::save() {
  KConfigGroup composer( KMKernel::config(), "Composer" );

  composer.writeEntry( "crypto-warn-recv-not-in-cert", mWidget->warnReceiverNotInCertificateCB->isChecked() );
  composer.writeEntry( "crypto-warning-unencrypted", mWidget->warnUnencryptedCB->isChecked() );
  composer.writeEntry( "crypto-warning-unsigned", mWidget->mWarnUnsigned->isChecked() );

  composer.writeEntry( "crypto-warn-when-near-expire", mWidget->warnGroupBox->isChecked() );
  composer.writeEntry( "crypto-warn-sign-key-near-expire-int",
                       mWidget->mWarnSignKeyExpiresSB->value() );
  composer.writeEntry( "crypto-warn-sign-chaincert-near-expire-int",
                       mWidget->mWarnSignChainCertExpiresSB->value() );
  composer.writeEntry( "crypto-warn-sign-root-near-expire-int",
                       mWidget->mWarnSignRootCertExpiresSB->value() );

  composer.writeEntry( "crypto-warn-encr-key-near-expire-int",
                       mWidget->mWarnEncrKeyExpiresSB->value() );
  composer.writeEntry( "crypto-warn-encr-chaincert-near-expire-int",
                       mWidget->mWarnEncrChainCertExpiresSB->value() );
  composer.writeEntry( "crypto-warn-encr-root-near-expire-int",
                       mWidget->mWarnEncrRootCertExpiresSB->value() );
}

void SecurityPage::WarningTab::slotReenableAllWarningsClicked() {
  KMessageBox::enableAllMessages();
  mWidget->enableAllWarningsPB->setEnabled( false );
}

////

TQString SecurityPage::SMimeTab::helpAnchor() const {
  return TQString::fromLatin1("configure-security-smime-validation");
}

SecurityPageSMimeTab::SecurityPageSMimeTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // the margins are inside mWidget itself
  TQVBoxLayout* vlay = new TQVBoxLayout( this, 0, 0 );

  mWidget = new SMimeConfiguration( this );
  vlay->addWidget( mWidget );

  // Button-group for exclusive radiobuttons
  TQButtonGroup* bg = new TQButtonGroup( mWidget );
  bg->hide();
  bg->insert( mWidget->CRLRB );
  bg->insert( mWidget->OCSPRB );

  // Settings for the keyrequester custom widget
  mWidget->OCSPResponderSignature->setAllowedKeys(
     Kleo::KeySelectionDialog::SMIMEKeys
     | Kleo::KeySelectionDialog::TrustedKeys
     | Kleo::KeySelectionDialog::ValidKeys
     | Kleo::KeySelectionDialog::SigningKeys
     | Kleo::KeySelectionDialog::PublicKeys );
  mWidget->OCSPResponderSignature->setMultipleKeysEnabled( false );

  mConfig = Kleo::CryptoBackendFactory::instance()->config();

  connect( mWidget->CRLRB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->OCSPRB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->OCSPResponderURL, TQT_SIGNAL( textChanged( const TQString& ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->OCSPResponderSignature, TQT_SIGNAL( changed() ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->doNotCheckCertPolicyCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->neverConsultCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->fetchMissingCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );

  connect( mWidget->ignoreServiceURLCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->ignoreHTTPDPCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->disableHTTPCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->honorHTTPProxyRB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->useCustomHTTPProxyRB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->customHTTPProxy, TQT_SIGNAL( textChanged( const TQString& ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->ignoreLDAPDPCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->disableLDAPCB, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( slotEmitChanged() ) );
  connect( mWidget->customLDAPProxy, TQT_SIGNAL( textChanged( const TQString& ) ), this, TQT_SLOT( slotEmitChanged() ) );

  connect( mWidget->disableHTTPCB, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotUpdateHTTPActions() ) );
  connect( mWidget->ignoreHTTPDPCB, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotUpdateHTTPActions() ) );

  // Button-group for exclusive radiobuttons
  TQButtonGroup* bgHTTPProxy = new TQButtonGroup( mWidget );
  bgHTTPProxy->hide();
  bgHTTPProxy->insert( mWidget->honorHTTPProxyRB );
  bgHTTPProxy->insert( mWidget->useCustomHTTPProxyRB );

  if ( !connectDCOPSignal( 0, "KPIM::CryptoConfig", "changed()",
                           "load()", false ) )
    kdError(5650) << "SecurityPageSMimeTab: connection to CryptoConfig's changed() failed" << endl;

}

SecurityPageSMimeTab::~SecurityPageSMimeTab()
{
}

static void disableDirmngrWidget( TQWidget* w ) {
  w->setEnabled( false );
  TQWhatsThis::remove( w );
  TQWhatsThis::add( w, i18n( "This option requires dirmngr >= 0.9.0" ) );
}

static void initializeDirmngrCheckbox( TQCheckBox* cb, Kleo::CryptoConfigEntry* entry ) {
  if ( entry )
    cb->setChecked( entry->boolValue() );
  else
    disableDirmngrWidget( cb );
}

struct SMIMECryptoConfigEntries {
  SMIMECryptoConfigEntries( Kleo::CryptoConfig* config )
    : mConfig( config ) {

    // Checkboxes
    mCheckUsingOCSPConfigEntry = configEntry( "gpgsm", "Security", "enable-ocsp", Kleo::CryptoConfigEntry::ArgType_None, false );
    mEnableOCSPsendingConfigEntry = configEntry( "dirmngr", "OCSP", "allow-ocsp", Kleo::CryptoConfigEntry::ArgType_None, false );
    mDoNotCheckCertPolicyConfigEntry = configEntry( "gpgsm", "Security", "disable-policy-checks", Kleo::CryptoConfigEntry::ArgType_None, false );
    mNeverConsultConfigEntry = configEntry( "gpgsm", "Security", "disable-crl-checks", Kleo::CryptoConfigEntry::ArgType_None, false );
    mFetchMissingConfigEntry = configEntry( "gpgsm", "Security", "auto-issuer-key-retrieve", Kleo::CryptoConfigEntry::ArgType_None, false );
    // dirmngr-0.9.0 options
    mIgnoreServiceURLEntry = configEntry( "dirmngr", "OCSP", "ignore-ocsp-service-url", Kleo::CryptoConfigEntry::ArgType_None, false );
    mIgnoreHTTPDPEntry = configEntry( "dirmngr", "HTTP", "ignore-http-dp", Kleo::CryptoConfigEntry::ArgType_None, false );
    mDisableHTTPEntry = configEntry( "dirmngr", "HTTP", "disable-http", Kleo::CryptoConfigEntry::ArgType_None, false );
    mHonorHTTPProxy = configEntry( "dirmngr", "HTTP", "honor-http-proxy", Kleo::CryptoConfigEntry::ArgType_None, false );

    mIgnoreLDAPDPEntry = configEntry( "dirmngr", "LDAP", "ignore-ldap-dp", Kleo::CryptoConfigEntry::ArgType_None, false );
    mDisableLDAPEntry = configEntry( "dirmngr", "LDAP", "disable-ldap", Kleo::CryptoConfigEntry::ArgType_None, false );
    // Other widgets
    mOCSPResponderURLConfigEntry = configEntry( "dirmngr", "OCSP", "ocsp-responder", Kleo::CryptoConfigEntry::ArgType_String, false );
    mOCSPResponderSignature = configEntry( "dirmngr", "OCSP", "ocsp-signer", Kleo::CryptoConfigEntry::ArgType_String, false );
    mCustomHTTPProxy = configEntry( "dirmngr", "HTTP", "http-proxy", Kleo::CryptoConfigEntry::ArgType_String, false );
    mCustomLDAPProxy = configEntry( "dirmngr", "LDAP", "ldap-proxy", Kleo::CryptoConfigEntry::ArgType_String, false );
  }

  Kleo::CryptoConfigEntry* configEntry( const char* componentName,
                                        const char* groupName,
                                        const char* entryName,
                                        int argType,
                                        bool isList );

  // Checkboxes
  Kleo::CryptoConfigEntry* mCheckUsingOCSPConfigEntry;
  Kleo::CryptoConfigEntry* mEnableOCSPsendingConfigEntry;
  Kleo::CryptoConfigEntry* mDoNotCheckCertPolicyConfigEntry;
  Kleo::CryptoConfigEntry* mNeverConsultConfigEntry;
  Kleo::CryptoConfigEntry* mFetchMissingConfigEntry;
  Kleo::CryptoConfigEntry* mIgnoreServiceURLEntry;
  Kleo::CryptoConfigEntry* mIgnoreHTTPDPEntry;
  Kleo::CryptoConfigEntry* mDisableHTTPEntry;
  Kleo::CryptoConfigEntry* mHonorHTTPProxy;
  Kleo::CryptoConfigEntry* mIgnoreLDAPDPEntry;
  Kleo::CryptoConfigEntry* mDisableLDAPEntry;
  // Other widgets
  Kleo::CryptoConfigEntry* mOCSPResponderURLConfigEntry;
  Kleo::CryptoConfigEntry* mOCSPResponderSignature;
  Kleo::CryptoConfigEntry* mCustomHTTPProxy;
  Kleo::CryptoConfigEntry* mCustomLDAPProxy;

  Kleo::CryptoConfig* mConfig;
};

void SecurityPage::SMimeTab::doLoadOther() {
  if ( !mConfig ) {
    setEnabled( false );
    return;
  }

  // Force re-parsing gpgconf data, in case e.g. kleopatra or "configure backend" was used
  // (which ends up calling us via dcop)
  mConfig->clear();

  // Create config entries
  // Don't keep them around, they'll get deleted by clear(), which could be
  // done by the "configure backend" button even before we save().
  SMIMECryptoConfigEntries e( mConfig );

  // Initialize GUI items from the config entries

  if ( e.mCheckUsingOCSPConfigEntry ) {
    bool b = e.mCheckUsingOCSPConfigEntry->boolValue();
    mWidget->OCSPRB->setChecked( b );
    mWidget->CRLRB->setChecked( !b );
    mWidget->OCSPGroupBox->setEnabled( b );
  } else {
    mWidget->OCSPGroupBox->setEnabled( false );
  }
  if ( e.mDoNotCheckCertPolicyConfigEntry )
    mWidget->doNotCheckCertPolicyCB->setChecked( e.mDoNotCheckCertPolicyConfigEntry->boolValue() );
  if ( e.mNeverConsultConfigEntry )
    mWidget->neverConsultCB->setChecked( e.mNeverConsultConfigEntry->boolValue() );
  if ( e.mFetchMissingConfigEntry )
    mWidget->fetchMissingCB->setChecked( e.mFetchMissingConfigEntry->boolValue() );

  if ( e.mOCSPResponderURLConfigEntry )
    mWidget->OCSPResponderURL->setText( e.mOCSPResponderURLConfigEntry->stringValue() );
  if ( e.mOCSPResponderSignature ) {
    mWidget->OCSPResponderSignature->setFingerprint( e.mOCSPResponderSignature->stringValue() );
  }

  // dirmngr-0.9.0 options
  initializeDirmngrCheckbox( mWidget->ignoreServiceURLCB, e.mIgnoreServiceURLEntry );
  initializeDirmngrCheckbox( mWidget->ignoreHTTPDPCB, e.mIgnoreHTTPDPEntry );
  initializeDirmngrCheckbox( mWidget->disableHTTPCB, e.mDisableHTTPEntry );
  initializeDirmngrCheckbox( mWidget->ignoreLDAPDPCB, e.mIgnoreLDAPDPEntry );
  initializeDirmngrCheckbox( mWidget->disableLDAPCB, e.mDisableLDAPEntry );
  if ( e.mCustomHTTPProxy ) {
    TQString systemProxy = TQString::fromLocal8Bit( getenv( "http_proxy" ) );
    if ( systemProxy.isEmpty() )
      systemProxy = i18n( "no proxy" );
    mWidget->systemHTTPProxy->setText( i18n( "(Current system setting: %1)" ).arg( systemProxy ) );
    bool honor = e.mHonorHTTPProxy && e.mHonorHTTPProxy->boolValue();
    mWidget->honorHTTPProxyRB->setChecked( honor );
    mWidget->useCustomHTTPProxyRB->setChecked( !honor );
    mWidget->customHTTPProxy->setText( e.mCustomHTTPProxy->stringValue() );
  } else {
    disableDirmngrWidget( mWidget->honorHTTPProxyRB );
    disableDirmngrWidget( mWidget->useCustomHTTPProxyRB );
    disableDirmngrWidget( mWidget->systemHTTPProxy );
    disableDirmngrWidget( mWidget->customHTTPProxy );
  }
  if ( e.mCustomLDAPProxy )
    mWidget->customLDAPProxy->setText( e.mCustomLDAPProxy->stringValue() );
  else {
    disableDirmngrWidget( mWidget->customLDAPProxy );
    disableDirmngrWidget( mWidget->customLDAPLabel );
  }
  slotUpdateHTTPActions();
}

void SecurityPage::SMimeTab::slotUpdateHTTPActions() {
  mWidget->ignoreHTTPDPCB->setEnabled( !mWidget->disableHTTPCB->isChecked() );

  // The proxy settings only make sense when "Ignore HTTP CRL DPs of certificate" is checked.
  bool enableProxySettings = !mWidget->disableHTTPCB->isChecked()
                          && mWidget->ignoreHTTPDPCB->isChecked();
  mWidget->systemHTTPProxy->setEnabled( enableProxySettings );
  mWidget->useCustomHTTPProxyRB->setEnabled( enableProxySettings );
  mWidget->honorHTTPProxyRB->setEnabled( enableProxySettings );
  mWidget->customHTTPProxy->setEnabled( enableProxySettings );
}

void SecurityPage::SMimeTab::installProfile( KConfig * ) {
}

static void saveCheckBoxToKleoEntry( TQCheckBox* cb, Kleo::CryptoConfigEntry* entry ) {
  const bool b = cb->isChecked();
  if ( entry && entry->boolValue() != b )
    entry->setBoolValue( b );
}

void SecurityPage::SMimeTab::save() {
  if ( !mConfig ) {
    return;
  }
  // Create config entries
  // Don't keep them around, they'll get deleted by clear(), which could be done by the
  // "configure backend" button.
  SMIMECryptoConfigEntries e( mConfig );

  bool b = mWidget->OCSPRB->isChecked();
  if ( e.mCheckUsingOCSPConfigEntry && e.mCheckUsingOCSPConfigEntry->boolValue() != b )
    e.mCheckUsingOCSPConfigEntry->setBoolValue( b );
  // Set allow-ocsp together with enable-ocsp
  if ( e.mEnableOCSPsendingConfigEntry && e.mEnableOCSPsendingConfigEntry->boolValue() != b )
    e.mEnableOCSPsendingConfigEntry->setBoolValue( b );

  saveCheckBoxToKleoEntry( mWidget->doNotCheckCertPolicyCB, e.mDoNotCheckCertPolicyConfigEntry );
  saveCheckBoxToKleoEntry( mWidget->neverConsultCB, e.mNeverConsultConfigEntry );
  saveCheckBoxToKleoEntry( mWidget->fetchMissingCB, e.mFetchMissingConfigEntry );

  TQString txt = mWidget->OCSPResponderURL->text();
  if ( e.mOCSPResponderURLConfigEntry && e.mOCSPResponderURLConfigEntry->stringValue() != txt )
    e.mOCSPResponderURLConfigEntry->setStringValue( txt );

  txt = mWidget->OCSPResponderSignature->fingerprint();
  if ( e.mOCSPResponderSignature && e.mOCSPResponderSignature->stringValue() != txt ) {
    e.mOCSPResponderSignature->setStringValue( txt );
  }

  //dirmngr-0.9.0 options
  saveCheckBoxToKleoEntry( mWidget->ignoreServiceURLCB, e.mIgnoreServiceURLEntry );
  saveCheckBoxToKleoEntry( mWidget->ignoreHTTPDPCB, e.mIgnoreHTTPDPEntry );
  saveCheckBoxToKleoEntry( mWidget->disableHTTPCB, e.mDisableHTTPEntry );
  saveCheckBoxToKleoEntry( mWidget->ignoreLDAPDPCB, e.mIgnoreLDAPDPEntry );
  saveCheckBoxToKleoEntry( mWidget->disableLDAPCB, e.mDisableLDAPEntry );
  if ( e.mCustomHTTPProxy ) {
    const bool honor = mWidget->honorHTTPProxyRB->isChecked();
    if ( e.mHonorHTTPProxy && e.mHonorHTTPProxy->boolValue() != honor )
        e.mHonorHTTPProxy->setBoolValue( honor );

    TQString chosenProxy = mWidget->customHTTPProxy->text();
    if ( chosenProxy != e.mCustomHTTPProxy->stringValue() )
      e.mCustomHTTPProxy->setStringValue( chosenProxy );
  }
  txt = mWidget->customLDAPProxy->text();
  if ( e.mCustomLDAPProxy && e.mCustomLDAPProxy->stringValue() != txt )
    e.mCustomLDAPProxy->setStringValue( mWidget->customLDAPProxy->text() );

  mConfig->sync( true );
}

bool SecurityPageSMimeTab::process(const TQCString &fun, const TQByteArray &data, TQCString& replyType, TQByteArray &replyData)
{
    if ( fun == "load()" ) {
        replyType = "void";
        load();
    } else {
        return DCOPObject::process( fun, data, replyType, replyData );
    }
    return true;
}

QCStringList SecurityPageSMimeTab::interfaces()
{
  QCStringList ifaces = DCOPObject::interfaces();
  ifaces += "SecurityPageSMimeTab";
  return ifaces;
}

QCStringList SecurityPageSMimeTab::functions()
{
  // Hide our slot, just because it's simpler to do so.
  return DCOPObject::functions();
}

Kleo::CryptoConfigEntry* SMIMECryptoConfigEntries::configEntry( const char* componentName,
                                                                const char* groupName,
                                                                const char* entryName,
                                                                int /*Kleo::CryptoConfigEntry::ArgType*/ argType,
                                                                bool isList )
{
    Kleo::CryptoConfigEntry* entry = mConfig->entry( componentName, groupName, entryName );
    if ( !entry ) {
        kdWarning(5006) << TQString( "Backend error: gpgconf doesn't seem to know the entry for %1/%2/%3" ).arg( componentName, groupName, entryName ) << endl;
        return 0;
    }
    if( entry->argType() != argType || entry->isList() != isList ) {
        kdWarning(5006) << TQString( "Backend error: gpgconf has wrong type for %1/%2/%3: %4 %5" ).arg( componentName, groupName, entryName ).arg( entry->argType() ).arg( entry->isList() ) << endl;
        return 0;
    }
    return entry;
}

////

TQString SecurityPage::CryptPlugTab::helpAnchor() const {
  return TQString::fromLatin1("configure-security-crypto-backends");
}

SecurityPageCryptPlugTab::SecurityPageCryptPlugTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  TQVBoxLayout * vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  mBackendConfig = Kleo::CryptoBackendFactory::instance()->configWidget( this, "mBackendConfig" );
  connect( mBackendConfig, TQT_SIGNAL( changed( bool ) ), this, TQT_SIGNAL( changed( bool ) ) );

  vlay->addWidget( mBackendConfig );
}

SecurityPageCryptPlugTab::~SecurityPageCryptPlugTab()
{

}

void SecurityPage::CryptPlugTab::doLoadOther() {
  mBackendConfig->load();
}

void SecurityPage::CryptPlugTab::save() {
  mBackendConfig->save();
}

// *************************************************************
// *                                                           *
// *                        MiscPage                           *
// *                                                           *
// *************************************************************
TQString MiscPage::helpAnchor() const {
  return TQString::fromLatin1("configure-misc");
}

MiscPage::MiscPage( TQWidget * parent, const char * name )
  : ConfigModuleWithTabs( parent, name )
{
  mFolderTab = new FolderTab();
  addTab( mFolderTab, i18n("&Folders") );

  mGroupwareTab = new GroupwareTab();
  addTab( mGroupwareTab, i18n("&Groupware") );
  load();
}

TQString MiscPage::FolderTab::helpAnchor() const {
  return TQString::fromLatin1("configure-misc-folders");
}

MiscPageFolderTab::MiscPageFolderTab( TQWidget * parent, const char * name )
  : ConfigModuleTab( parent, name )
{
  // temp. vars:
  TQVBoxLayout *vlay;
  TQHBoxLayout *hlay;
  TQLabel      *label;

  vlay = new TQVBoxLayout( this, KDialog::marginHint(), KDialog::spacingHint() );

  // "confirm before emptying folder" check box: stretch 0
  mEmptyFolderConfirmCheck =
    new TQCheckBox( i18n("Corresponds to Folder->Move All Messages to Trash",
                        "Ask for co&nfirmation before moving all messages to "
                        "trash"),
                   this );
  vlay->addWidget( mEmptyFolderConfirmCheck );
  connect( mEmptyFolderConfirmCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  mExcludeImportantFromExpiry =
    new TQCheckBox( i18n("E&xclude important messages from expiry"), this );
  vlay->addWidget( mExcludeImportantFromExpiry );
  connect( mExcludeImportantFromExpiry, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "when trying to find unread messages" combo + label: stretch 0
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mLoopOnGotoUnread = new TQComboBox( false, this );
  label = new TQLabel( mLoopOnGotoUnread,
           i18n("to be continued with \"do not loop\", \"loop in current folder\", "
                "and \"loop in all folders\".",
                "When trying to find unread messages:"), this );
  mLoopOnGotoUnread->insertStringList( TQStringList()
      << i18n("continuation of \"When trying to find unread messages:\"",
              "Do not Loop")
      << i18n("continuation of \"When trying to find unread messages:\"",
              "Loop in Current Folder")
      << i18n("continuation of \"When trying to find unread messages:\"",
              "Loop in All Folders"));
  hlay->addWidget( label );
  hlay->addWidget( mLoopOnGotoUnread, 1 );
  connect( mLoopOnGotoUnread, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // when entering a folder
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mActionEnterFolder = new TQComboBox( false, this );
  label = new TQLabel( mActionEnterFolder,
           i18n("to be continued with \"jump to first new message\", "
                "\"jump to first unread or new message\","
                "and \"jump to last selected message\".",
                "When entering a folder:"), this );
  mActionEnterFolder->insertStringList( TQStringList()
      << i18n("continuation of \"When entering a folder:\"",
              "Jump to First New Message")
      << i18n("continuation of \"When entering a folder:\"",
              "Jump to First Unread or New Message")
      << i18n("continuation of \"When entering a folder:\"",
              "Jump to Last Selected Message")
      << i18n("continuation of \"When entering a folder:\"",
              "Jump to Newest Message")
      << i18n("continuation of \"When entering a folder:\"",
              "Jump to Oldest Message") );
  hlay->addWidget( label );
  hlay->addWidget( mActionEnterFolder, 1 );
  connect( mActionEnterFolder, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mDelayedMarkAsRead = new TQCheckBox( i18n("Mar&k selected message as read after"), this );
  hlay->addWidget( mDelayedMarkAsRead );
  mDelayedMarkTime = new KIntSpinBox( 0 /*min*/, 60 /*max*/, 1/*step*/,
                                      0 /*init*/, 10 /*base*/, this);
  mDelayedMarkTime->setSuffix( i18n(" sec") );
  mDelayedMarkTime->setEnabled( false ); // since mDelayedMarkAsREad is off
  hlay->addWidget( mDelayedMarkTime );
  hlay->addStretch( 1 );
  connect( mDelayedMarkTime, TQT_SIGNAL( valueChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  connect( mDelayedMarkAsRead, TQT_SIGNAL(toggled(bool)),
           mDelayedMarkTime, TQT_SLOT(setEnabled(bool)));
  connect( mDelayedMarkAsRead, TQT_SIGNAL(toggled(bool)),
           this , TQT_SLOT(slotEmitChanged( void )));

  // "show popup after Drag'n'Drop" checkbox: stretch 0
  mShowPopupAfterDnD =
    new TQCheckBox( i18n("Ask for action after &dragging messages to another folder"), this );
  vlay->addWidget( mShowPopupAfterDnD );
  connect( mShowPopupAfterDnD, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "default mailbox format" combo + label: stretch 0
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mMailboxPrefCombo = new TQComboBox( false, this );
  label = new TQLabel( mMailboxPrefCombo,
                      i18n("to be continued with \"flat files\" and "
                           "\"directories\", resp.",
                           "By default, &message folders on disk are:"), this );
  mMailboxPrefCombo->insertStringList( TQStringList()
          << i18n("continuation of \"By default, &message folders on disk are\"",
                  "Flat Files (\"mbox\" format)")
          << i18n("continuation of \"By default, &message folders on disk are\"",
                  "Directories (\"maildir\" format)") );
  // and now: add TQWhatsThis:
  TQString msg = i18n( "what's this help",
                      "<qt><p>This selects which mailbox format will be "
                      "the default for local folders:</p>"
                      "<p><b>mbox:</b> KMail's mail "
                      "folders are represented by a single file each. "
                      "Individual messages are separated from each other by a "
                      "line starting with \"From \". This saves space on "
                      "disk, but may be less robust, e.g. when moving messages "
                      "between folders.</p>"
                      "<p><b>maildir:</b> KMail's mail folders are "
                      "represented by real folders on disk. Individual messages "
                      "are separate files. This may waste a bit of space on "
                      "disk, but should be more robust, e.g. when moving "
                      "messages between folders.</p></qt>");
  TQWhatsThis::add( mMailboxPrefCombo, msg );
  TQWhatsThis::add( label, msg );
  hlay->addWidget( label );
  hlay->addWidget( mMailboxPrefCombo, 1 );
  connect( mMailboxPrefCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "On startup..." option:
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mOnStartupOpenFolder = new FolderRequester( this,
      kmkernel->getKMMainWidget()->folderTree() );
  label = new TQLabel( mOnStartupOpenFolder,
                      i18n("Open this folder on startup:"), this );
  hlay->addWidget( label );
  hlay->addWidget( mOnStartupOpenFolder, 1 );
  connect( mOnStartupOpenFolder, TQT_SIGNAL( folderChanged( KMFolder* ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // "Empty &trash on program exit" option:
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mEmptyTrashCheck = new TQCheckBox( i18n("Empty local &trash folder on program exit"),
                                    this );
  hlay->addWidget( mEmptyTrashCheck );
  connect( mEmptyTrashCheck, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

#ifdef HAVE_INDEXLIB
  // indexing enabled option:
  mIndexingEnabled = new TQCheckBox( i18n("Enable full text &indexing"), this );
  vlay->addWidget( mIndexingEnabled );
  connect( mIndexingEnabled, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
#endif

  // "Quota Units"
  hlay = new TQHBoxLayout( vlay ); // inherits spacing
  mQuotaCmbBox = new TQComboBox( false, this );
  label = new TQLabel( mQuotaCmbBox,
                      i18n("Quota units: "), this );
  mQuotaCmbBox->insertStringList( TQStringList()
                   << i18n("KB")
                   << i18n("MB")
                   << i18n("GB") );
  hlay->addWidget( label );
  hlay->addWidget( mQuotaCmbBox, 1 );
  connect( mQuotaCmbBox, TQT_SIGNAL( activated( int )  ), this, TQT_SLOT( slotEmitChanged( void ) ) );

  vlay->addStretch( 1 );

  // @TODO: Till, move into .kcgc file
  msg = i18n( "what's this help",
            "<qt><p>When jumping to the next unread message, it may occur "
            "that no more unread messages are below the current message.</p>"
            "<p><b>Do not loop:</b> The search will stop at the last message in "
            "the current folder.</p>"
            "<p><b>Loop in current folder:</b> The search will continue at the "
            "top of the message list, but not go to another folder.</p>"
            "<p><b>Loop in all folders:</b> The search will continue at the top of "
            "the message list. If no unread messages are found it will then continue "
            "to the next folder.</p>"
            "<p>Similarly, when searching for the previous unread message, "
            "the search will start from the bottom of the message list and continue to "
            "the previous folder depending on which option is selected.</p></qt>" );
  TQWhatsThis::add( mLoopOnGotoUnread, msg );

#ifdef HAVE_INDEXLIB
 // this is probably overly pessimistic
  msg = i18n( "what's this help",
		  "<qt><p>Full text indexing allows very fast searches on the content "
		  "of your messages. When enabled, the search dialog will work very fast. "
		  "Also, the search tool bar will select messages based on content.</p>"
		  "<p>It takes up a certain amount of disk space "
		  "(about half the disk space for the messages).</p>"
		  "<p>After enabling, the index will need to be built, but you can continue to use KMail "
		  "while this operation is running.</p>"
		  "</qt>"
	    );

  TQWhatsThis::add( mIndexingEnabled, msg );
#endif
}

void MiscPage::FolderTab::doLoadFromGlobalSettings() {
  mExcludeImportantFromExpiry->setChecked( GlobalSettings::self()->excludeImportantMailFromExpiry() );
  // default = "Loop in current folder"
  mLoopOnGotoUnread->setCurrentItem( GlobalSettings::self()->loopOnGotoUnread() );
  mActionEnterFolder->setCurrentItem( GlobalSettings::self()->actionEnterFolder() );
  mDelayedMarkAsRead->setChecked( GlobalSettings::self()->delayedMarkAsRead() );
  mDelayedMarkTime->setValue( GlobalSettings::self()->delayedMarkTime() );
  mShowPopupAfterDnD->setChecked( GlobalSettings::self()->showPopupAfterDnD() );
  mQuotaCmbBox->setCurrentItem( GlobalSettings::self()->quotaUnit() );
}

void MiscPage::FolderTab::doLoadOther() {
  KConfigGroup general( KMKernel::config(), "General" );

  mEmptyTrashCheck->setChecked( general.readBoolEntry( "empty-trash-on-exit", true ) );
  mOnStartupOpenFolder->setFolder( general.readEntry( "startupFolder",
                                                  kmkernel->inboxFolder()->idString() ) );
  mEmptyFolderConfirmCheck->setChecked( general.readBoolEntry( "confirm-before-empty", true ) );

  int num = general.readNumEntry("default-mailbox-format", 1 );
  if ( num < 0 || num > 1 ) num = 1;
  mMailboxPrefCombo->setCurrentItem( num );

#ifdef HAVE_INDEXLIB
  mIndexingEnabled->setChecked( kmkernel->msgIndex() && kmkernel->msgIndex()->isEnabled() );
#endif
}

void MiscPage::FolderTab::save() {
  KConfigGroup general( KMKernel::config(), "General" );

  general.writeEntry( "empty-trash-on-exit", mEmptyTrashCheck->isChecked() );
  general.writeEntry( "confirm-before-empty", mEmptyFolderConfirmCheck->isChecked() );
  general.writeEntry( "default-mailbox-format", mMailboxPrefCombo->currentItem() );
  general.writeEntry( "startupFolder", mOnStartupOpenFolder->folder() ?
                                  mOnStartupOpenFolder->folder()->idString() : TQString::null );

  GlobalSettings::self()->setDelayedMarkAsRead( mDelayedMarkAsRead->isChecked() );
  GlobalSettings::self()->setDelayedMarkTime( mDelayedMarkTime->value() );
  GlobalSettings::self()->setActionEnterFolder( mActionEnterFolder->currentItem() );
  GlobalSettings::self()->setLoopOnGotoUnread( mLoopOnGotoUnread->currentItem() );
  GlobalSettings::self()->setShowPopupAfterDnD( mShowPopupAfterDnD->isChecked() );
  GlobalSettings::self()->setExcludeImportantMailFromExpiry(
        mExcludeImportantFromExpiry->isChecked() );
  GlobalSettings::self()->setQuotaUnit( mQuotaCmbBox->currentItem() );
#ifdef HAVE_INDEXLIB
  if ( kmkernel->msgIndex() ) kmkernel->msgIndex()->setEnabled( mIndexingEnabled->isChecked() );
#endif
}

TQString MiscPage::GroupwareTab::helpAnchor() const {
  return TQString::fromLatin1("configure-misc-groupware");
}

MiscPageGroupwareTab::MiscPageGroupwareTab( TQWidget* parent, const char* name )
  : ConfigModuleTab( parent, name )
{
  TQBoxLayout* vlay = new TQVBoxLayout( this, KDialog::marginHint(),
                                      KDialog::spacingHint() );
  vlay->setAutoAdd( true );

  // IMAP resource setup
  TQVGroupBox* b1 = new TQVGroupBox( i18n("&IMAP Resource Folder Options"),
                                   this );

  mEnableImapResCB =
    new TQCheckBox( i18n("&Enable IMAP resource functionality"), b1 );
  TQToolTip::add( mEnableImapResCB,  i18n( "This enables the IMAP storage for "
                                          "the Kontact applications" ) );
  TQWhatsThis::add( mEnableImapResCB,
        i18n( GlobalSettings::self()->theIMAPResourceEnabledItem()->whatsThis().utf8() ) );
  connect( mEnableImapResCB, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mBox = new TQWidget( b1 );
  TQGridLayout* grid = new TQGridLayout( mBox, 5, 2, 0, KDialog::spacingHint() );
  grid->setColStretch( 1, 1 );
  connect( mEnableImapResCB, TQT_SIGNAL( toggled(bool) ),
           mBox, TQT_SLOT( setEnabled(bool) ) );

  TQLabel* storageFormatLA = new TQLabel( i18n("&Format used for the groupware folders:"),
                                        mBox );
  TQString toolTip = i18n( "Choose the format to use to store the contents of the groupware folders." );
  TQString whatsThis = i18n( GlobalSettings::self()
        ->theIMAPResourceStorageFormatItem()->whatsThis().utf8() );
  grid->addWidget( storageFormatLA, 0, 0 );
  TQToolTip::add( storageFormatLA, toolTip );
  TQWhatsThis::add( storageFormatLA, whatsThis );
  mStorageFormatCombo = new TQComboBox( false, mBox );
  storageFormatLA->setBuddy( mStorageFormatCombo );
  TQStringList formatLst;
  formatLst << i18n("Deprecated Kolab1 (iCal/vCard)") << i18n("Kolab2 (XML)");
  mStorageFormatCombo->insertStringList( formatLst );
  grid->addWidget( mStorageFormatCombo, 0, 1 );
  TQToolTip::add( mStorageFormatCombo, toolTip );
  TQWhatsThis::add( mStorageFormatCombo, whatsThis );
  connect( mStorageFormatCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotStorageFormatChanged( int ) ) );

  TQLabel* languageLA = new TQLabel( i18n("&Language of the groupware folders:"),
                                   mBox );

  toolTip = i18n( "Set the language of the folder names" );
  whatsThis = i18n( GlobalSettings::self()
        ->theIMAPResourceFolderLanguageItem()->whatsThis().utf8() );
  grid->addWidget( languageLA, 1, 0 );
  TQToolTip::add( languageLA, toolTip );
  TQWhatsThis::add( languageLA, whatsThis );
  mLanguageCombo = new TQComboBox( false, mBox );
  languageLA->setBuddy( mLanguageCombo );
  TQStringList lst;
  lst << i18n("English") << i18n("German") << i18n("French") << i18n("Dutch");
  mLanguageCombo->insertStringList( lst );
  grid->addWidget( mLanguageCombo, 1, 1 );
  TQToolTip::add( mLanguageCombo, toolTip );
  TQWhatsThis::add( mLanguageCombo, whatsThis );
  connect( mLanguageCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mFolderComboLabel = new TQLabel( mBox ); // text depends on storage format
  toolTip = i18n( "Set the parent of the resource folders" );
  whatsThis = i18n( GlobalSettings::self()->theIMAPResourceFolderParentItem()->whatsThis().utf8() );
  TQToolTip::add( mFolderComboLabel, toolTip );
  TQWhatsThis::add( mFolderComboLabel, whatsThis );
  grid->addWidget( mFolderComboLabel, 2, 0 );

  mFolderComboStack = new TQWidgetStack( mBox );
  grid->addWidget( mFolderComboStack, 2, 1 );

  // First possibility in the widgetstack: a combo showing the list of all folders
  // This is used with the ical/vcard storage
  mFolderCombo = new FolderRequester( mBox,
      kmkernel->getKMMainWidget()->folderTree() );
  mFolderComboStack->addWidget( mFolderCombo, 0 );
  TQToolTip::add( mFolderCombo, toolTip );
  TQWhatsThis::add( mFolderCombo, whatsThis );
  connect( mFolderCombo, TQT_SIGNAL( folderChanged( KMFolder* ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  // Second possibility in the widgetstack: a combo showing the list of accounts
  // This is used with the kolab xml storage since the groupware folders
  // are always under the inbox.
  mAccountCombo = new KMail::AccountComboBox( mBox );
  mFolderComboStack->addWidget( mAccountCombo, 1 );
  TQToolTip::add( mAccountCombo, toolTip );
  TQWhatsThis::add( mAccountCombo, whatsThis );
  connect( mAccountCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  mHideGroupwareFolders = new TQCheckBox( i18n( "&Hide groupware folders" ),
                                         mBox, "HideGroupwareFoldersBox" );
  grid->addMultiCellWidget( mHideGroupwareFolders, 3, 3, 0, 0 );
  TQToolTip::add( mHideGroupwareFolders,
                 i18n( "When this is checked, you will not see the IMAP "
                       "resource folders in the folder tree." ) );
  TQWhatsThis::add( mHideGroupwareFolders, i18n( GlobalSettings::self()
           ->hideGroupwareFoldersItem()->whatsThis().utf8() ) );
  connect( mHideGroupwareFolders, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  mOnlyShowGroupwareFolders = new TQCheckBox( i18n( "&Only show groupware folders for this account" ),
                                         mBox, "OnlyGroupwareFoldersBox" );
  grid->addMultiCellWidget( mOnlyShowGroupwareFolders, 3, 3, 1, 1 );
  TQToolTip::add( mOnlyShowGroupwareFolders,
                 i18n( "When this is checked, you will not see normal  "
                       "mail folders in the folder tree for the account "
                       "configured for groupware." ) );
  TQWhatsThis::add( mOnlyShowGroupwareFolders, i18n( GlobalSettings::self()
           ->showOnlyGroupwareFoldersForGroupwareAccountItem()->whatsThis().utf8() ) );
  connect( mOnlyShowGroupwareFolders, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotEmitChanged() ) );

  mSyncImmediately = new TQCheckBox( i18n( "Synchronize groupware changes immediately" ), mBox );
  TQToolTip::add( mSyncImmediately,
                 i18n( "Synchronize groupware changes in disconnected IMAP folders immediately when being online." ) );
  connect( mSyncImmediately, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
  grid->addMultiCellWidget( mSyncImmediately, 4, 4, 0, 1 );

  mDeleteInvitations = new TQCheckBox(
             i18n( GlobalSettings::self()->deleteInvitationEmailsAfterSendingReplyItem()->label().utf8() ), mBox );
  TQWhatsThis::add( mDeleteInvitations, i18n( GlobalSettings::self()
             ->deleteInvitationEmailsAfterSendingReplyItem()->whatsThis().utf8() ) );
    connect( mDeleteInvitations, TQT_SIGNAL(toggled(bool)), TQT_SLOT(slotEmitChanged()) );
    grid->addMultiCellWidget( mDeleteInvitations, 5, 5, 0, 1 );

  // Groupware functionality compatibility setup
  b1 = new TQVGroupBox( i18n("Groupware Compatibility && Legacy Options"), this );

  gBox = new TQVBox( b1 );
#if 0
  // Currently believed to be disused.
  mEnableGwCB = new TQCheckBox( i18n("&Enable groupware functionality"), b1 );
  gBox->setSpacing( KDialog::spacingHint() );
  connect( mEnableGwCB, TQT_SIGNAL( toggled(bool) ),
           gBox, TQT_SLOT( setEnabled(bool) ) );
  connect( mEnableGwCB, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
#endif
  mEnableGwCB = 0;
  mLegacyMangleFromTo = new TQCheckBox( i18n( "Mangle From:/To: headers in replies to invitations" ), gBox );
  TQToolTip::add( mLegacyMangleFromTo, i18n( "Turn this option on in order to make Outlook(tm) understand your answers to invitation replies" ) );
  TQWhatsThis::add( mLegacyMangleFromTo, i18n( GlobalSettings::self()->
           legacyMangleFromToHeadersItem()->whatsThis().utf8() ) );
  connect( mLegacyMangleFromTo, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );
  mLegacyBodyInvites = new TQCheckBox( i18n( "Send invitations in the mail body" ), gBox );
  TQToolTip::add( mLegacyBodyInvites, i18n( "Turn this option on in order to make Outlook(tm) understand your answers to invitations" ) );
  TQWhatsThis::add( mLegacyMangleFromTo, i18n( GlobalSettings::self()->
           legacyBodyInvitesItem()->whatsThis().utf8() ) );
  connect( mLegacyBodyInvites, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SLOT( slotLegacyBodyInvitesToggled( bool ) ) );
  connect( mLegacyBodyInvites, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mExchangeCompatibleInvitations = new TQCheckBox( i18n( "Exchange compatible invitation naming" ), gBox );
  TQToolTip::add( mExchangeCompatibleInvitations, i18n( "Outlook(tm), when used in combination with a Microsoft Exchange server,\nhas a problem understanding standards-compliant groupware e-mail.\nTurn this option on to send groupware invitations and replies in an Exchange compatible way." ) );
  TQWhatsThis::add( mExchangeCompatibleInvitations, i18n( GlobalSettings::self()->
           exchangeCompatibleInvitationsItem()->whatsThis().utf8() ) );
  connect( mExchangeCompatibleInvitations, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mOutlookCompatibleInvitationComments = new TQCheckBox( i18n( "Outlook compatible invitation reply comments" ), gBox );
  TQToolTip::add( mOutlookCompatibleInvitationComments, i18n( "Send invitation reply comments in a way that Microsoft Outlook(tm) understands." ) );
  TQWhatsThis::add( mOutlookCompatibleInvitationComments, i18n( GlobalSettings::self()->
           outlookCompatibleInvitationReplyCommentsItem()->whatsThis().utf8() ) );
  connect( mOutlookCompatibleInvitationComments, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  mAutomaticSending = new TQCheckBox( i18n( "Automatic invitation sending" ), gBox );
  TQToolTip::add( mAutomaticSending, i18n( "When this is on, the user will not see the mail composer window. Invitation mails are sent automatically" ) );
  TQWhatsThis::add( mAutomaticSending, i18n( GlobalSettings::self()->
           automaticSendingItem()->whatsThis().utf8() ) );
  connect( mAutomaticSending, TQT_SIGNAL( stateChanged( int ) ),
           this, TQT_SLOT( slotEmitChanged( void ) ) );

  // Open space padding at the end
  new TQLabel( this );
}

void MiscPageGroupwareTab::slotLegacyBodyInvitesToggled( bool on )
{
  if ( on ) {
    TQString txt = i18n( "<qt>Invitations are normally sent as attachments to "
                        "a mail. This switch changes the invitation mails to "
                        "be sent in the text of the mail instead; this is "
                        "necessary to send invitations and replies to "
                        "Microsoft Outlook.<br>But, when you do this, you no "
                        "longer get descriptive text that mail programs "
                        "can read; so, to people who have email programs "
                        "that do not understand the invitations, the "
                        "resulting messages look very odd.<br>People that have email "
                        "programs that do understand invitations will still "
                        "be able to work with this.</qt>" );
    KMessageBox::information( this, txt, TQString::null,
                              "LegacyBodyInvitesWarning" );
  }
  // Invitations in the body are autosent in any case (no point in editing raw ICAL)
  // So the autosend option is only available if invitations are sent as attachment.
  mAutomaticSending->setEnabled( !mLegacyBodyInvites->isChecked() );
}

void MiscPage::GroupwareTab::doLoadFromGlobalSettings() {
  if ( mEnableGwCB ) {
    mEnableGwCB->setChecked( GlobalSettings::self()->groupwareEnabled() );
    gBox->setEnabled( mEnableGwCB->isChecked() );
  }

  mLegacyMangleFromTo->setChecked( GlobalSettings::self()->legacyMangleFromToHeaders() );
  mLegacyBodyInvites->blockSignals( true );

  mLegacyBodyInvites->setChecked( GlobalSettings::self()->legacyBodyInvites() );
  mLegacyBodyInvites->blockSignals( false );

  mExchangeCompatibleInvitations->setChecked( GlobalSettings::self()->exchangeCompatibleInvitations() );

  mOutlookCompatibleInvitationComments->setChecked( GlobalSettings::self()->outlookCompatibleInvitationReplyComments() );

  mAutomaticSending->setChecked( GlobalSettings::self()->automaticSending() );
  mAutomaticSending->setEnabled( !mLegacyBodyInvites->isChecked() );

  // Read the IMAP resource config
  mEnableImapResCB->setChecked( GlobalSettings::self()->theIMAPResourceEnabled() );
  mBox->setEnabled( mEnableImapResCB->isChecked() );

  mHideGroupwareFolders->setChecked( GlobalSettings::self()->hideGroupwareFolders() );
  int i = GlobalSettings::self()->theIMAPResourceFolderLanguage();
  mLanguageCombo->setCurrentItem(i);
  i = GlobalSettings::self()->theIMAPResourceStorageFormat();
  mStorageFormatCombo->setCurrentItem(i);
  slotStorageFormatChanged( i );
  mOnlyShowGroupwareFolders->setChecked( GlobalSettings::self()->showOnlyGroupwareFoldersForGroupwareAccount() );
  mSyncImmediately->setChecked( GlobalSettings::self()->immediatlySyncDIMAPOnGroupwareChanges() );
  mDeleteInvitations->setChecked( GlobalSettings::self()->deleteInvitationEmailsAfterSendingReply() );

  TQString folderId( GlobalSettings::self()->theIMAPResourceFolderParent() );
  if( !folderId.isNull() && kmkernel->findFolderById( folderId ) ) {
    mFolderCombo->setFolder( folderId );
  } else {
    // Folder was deleted, we have to choose a new one
    mFolderCombo->setFolder( i18n( "<Choose a Folder>" ) );
  }

  KMAccount* selectedAccount = 0;
  int accountId = GlobalSettings::self()->theIMAPResourceAccount();
  if ( accountId )
    selectedAccount = kmkernel->acctMgr()->find( accountId );
  else {
    // Fallback: iterate over accounts to select folderId if found (as an inbox folder)
      for( KMAccount *a = kmkernel->acctMgr()->first(); a!=0;
         a = kmkernel->acctMgr()->next() ) {
      if( a->folder() && a->folder()->child() ) {
        // Look inside that folder for an INBOX
        KMFolderNode *node;
        for (node = a->folder()->child()->first(); node; node = a->folder()->child()->next())
          if (!node->isDir() && node->name() == "INBOX") break;

        if ( node && static_cast<KMFolder*>(node)->idString() == folderId ) {
          selectedAccount = a;
          break;
        }
      }
    }
  }
  if ( selectedAccount )
    mAccountCombo->setCurrentAccount( selectedAccount );
  else if ( GlobalSettings::self()->theIMAPResourceStorageFormat() == 1 )
    kdDebug(5006) << "Folder " << folderId << " not found as an account's inbox" << endl;
}

void MiscPage::GroupwareTab::save() {
  KConfigGroup groupware( KMKernel::config(), "Groupware" );

  // Write the groupware config
  if ( mEnableGwCB ) {
    groupware.writeEntry( "GroupwareEnabled", mEnableGwCB->isChecked() );
  }
  groupware.writeEntry( "LegacyMangleFromToHeaders", mLegacyMangleFromTo->isChecked() );
  groupware.writeEntry( "LegacyBodyInvites", mLegacyBodyInvites->isChecked() );
  groupware.writeEntry( "ExchangeCompatibleInvitations", mExchangeCompatibleInvitations->isChecked() );
  groupware.writeEntry( "OutlookCompatibleInvitationReplyComments", mOutlookCompatibleInvitationComments->isChecked() );
  groupware.writeEntry( "AutomaticSending", mAutomaticSending->isChecked() );

  if ( mEnableGwCB ) {
    GlobalSettings::self()->setGroupwareEnabled( mEnableGwCB->isChecked() );
  }
  GlobalSettings::self()->setLegacyMangleFromToHeaders( mLegacyMangleFromTo->isChecked() );
  GlobalSettings::self()->setLegacyBodyInvites( mLegacyBodyInvites->isChecked() );
  GlobalSettings::self()->setExchangeCompatibleInvitations( mExchangeCompatibleInvitations->isChecked() );
  GlobalSettings::self()->setOutlookCompatibleInvitationReplyComments( mOutlookCompatibleInvitationComments->isChecked() );
  GlobalSettings::self()->setAutomaticSending( mAutomaticSending->isChecked() );

  int format = mStorageFormatCombo->currentItem();
  GlobalSettings::self()->setTheIMAPResourceStorageFormat( format );

  // Write the IMAP resource config
  GlobalSettings::self()->setHideGroupwareFolders( mHideGroupwareFolders->isChecked() );
  GlobalSettings::self()->setShowOnlyGroupwareFoldersForGroupwareAccount( mOnlyShowGroupwareFolders->isChecked() );
  GlobalSettings::self()->setImmediatlySyncDIMAPOnGroupwareChanges( mSyncImmediately->isChecked() );
  GlobalSettings::self()->setDeleteInvitationEmailsAfterSendingReply( mDeleteInvitations->isChecked() );

  // If there is a leftover folder in the foldercombo, getFolder can
  // return 0. In that case we really don't have it enabled
  TQString folderId;
  if (  format == 0 ) {
    KMFolder* folder = mFolderCombo->folder();
    if (  folder )
      folderId = folder->idString();
    KMAccount* account = 0;
    // Didn't find an easy way to find the account for a given folder...
    // Fallback: iterate over accounts to select folderId if found (as an inbox folder)
    for( KMAccount *a = kmkernel->acctMgr()->first();
        a && !account; // stop when found
        a = kmkernel->acctMgr()->next() ) {
      if( a->folder() && a->folder()->child() ) {
        KMFolderNode *node;
        for ( node = a->folder()->child()->first(); node; node = a->folder()->child()->next() )
        {
          if ( static_cast<KMFolder*>(node) == folder ) {
            account = a;
            break;
          }
        }
      }
    }
    GlobalSettings::self()->setTheIMAPResourceAccount( account ? account->id() : 0 );
  } else {
    // Inbox folder of the selected account
    KMAccount* acct = mAccountCombo->currentAccount();
    if (  acct ) {
      folderId = TQString( ".%1.directory/INBOX" ).arg( acct->id() );
      GlobalSettings::self()->setTheIMAPResourceAccount( acct->id() );
    }
  }

  bool enabled = mEnableImapResCB->isChecked() && !folderId.isEmpty();
  GlobalSettings::self()->setTheIMAPResourceEnabled( enabled );
  GlobalSettings::self()->setTheIMAPResourceFolderLanguage( mLanguageCombo->currentItem() );
  GlobalSettings::self()->setTheIMAPResourceFolderParent( folderId );
}

void MiscPage::GroupwareTab::slotStorageFormatChanged( int format )
{
  mLanguageCombo->setEnabled( format == 0 ); // only ical/vcard needs the language hack
  mFolderComboStack->raiseWidget( format );
  if ( format == 0 ) {
    mFolderComboLabel->setText( i18n("&Resource folders are subfolders of:") );
    mFolderComboLabel->setBuddy( mFolderCombo );
  } else {
    mFolderComboLabel->setText( i18n("&Resource folders are in account:") );
    mFolderComboLabel->setBuddy( mAccountCombo );
  }
  slotEmitChanged();
}


// *************************************************************
// *                                                           *
// *                     AccountUpdater                        *
// *                                                           *
// *************************************************************
AccountUpdater::AccountUpdater(ImapAccountBase *account)
    : TQObject()
{
  mAccount = account;
}

void AccountUpdater::update()
{
  connect( mAccount, TQT_SIGNAL( connectionResult(int, const TQString&) ),
          this, TQT_SLOT( namespacesFetched() ) );
  mAccount->makeConnection();
}

void AccountUpdater::namespacesFetched()
{
  mAccount->setCheckingMail( true );
  mAccount->processNewMail( false );
  deleteLater();
}

#undef DIM

//----------------------------
#include "configuredialog.moc"
