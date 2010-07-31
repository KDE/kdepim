/*
    kpgpui.cpp

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

//#include <stdio.h>

#include <tqvgroupbox.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqapplication.h>
#include <tqtextcodec.h>
#include <tqdatetime.h>
#include <tqpixmap.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqpopupmenu.h>
#include <tqregexp.h>

#include <klocale.h>
#include <kpassdlg.h>
#include <kcharsets.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <kprogress.h>
#include <kapplication.h>
#include <kwin.h>
#if KDE_IS_VERSION( 3, 1, 90 )
#include <kglobalsettings.h>
#endif

#include "kpgp.h"
#include "kpgpui.h"
#include "kpgpkey.h"

#include <assert.h>
#include <string.h> // for memcpy(3)

const int Kpgp::KeySelectionDialog::sCheckSelectionDelay = 250;

namespace Kpgp {

PassphraseDialog::PassphraseDialog( TQWidget *parent,
                                    const TQString &caption, bool modal,
                                    const TQString &keyID )
  :KDialogBase( parent, 0, modal, caption, Ok|Cancel )
{
  TQHBox *hbox = makeHBoxMainWidget();
  hbox->setSpacing( spacingHint() );
  hbox->setMargin( marginHint() );

  TQLabel *label = new TQLabel(hbox);
  label->setPixmap( BarIcon("pgp-keys") );

  TQWidget *rightArea = new TQWidget( hbox );
  TQVBoxLayout *vlay = new TQVBoxLayout( rightArea, 0, spacingHint() );

  if (keyID.isNull())
    label = new TQLabel(i18n("Please enter your OpenPGP passphrase:"),rightArea);
  else
    label = new TQLabel(i18n("Please enter the OpenPGP passphrase for\n\"%1\":").arg(keyID),
                       rightArea);
  lineedit = new KPasswordEdit( rightArea );
  lineedit->setEchoMode(TQLineEdit::Password);
  lineedit->setMinimumWidth( fontMetrics().maxWidth()*20 );
  lineedit->setFocus();
  connect( lineedit, TQT_SIGNAL(returnPressed()), this, TQT_SLOT(slotOk()) );

  vlay->addWidget( label );
  vlay->addWidget( lineedit );

  disableResize();
}


PassphraseDialog::~PassphraseDialog()
{
}

const char * PassphraseDialog::passphrase()
{
  return lineedit->password();
}


// ------------------------------------------------------------------------
// Forbidden accels for KMail: AC GH OP
//                  for KNode: ACE H O
Config::Config( TQWidget *parent, const char *name, bool encrypt )
  : TQWidget( parent, name ), pgp( Module::getKpgp() )
{
  TQGroupBox * group;
  TQLabel    * label;
  TQString     msg;


  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  group = new TQVGroupBox( i18n("Warning"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );
  // (mmutz) work around Qt label bug in 3.0.0 (and possibly later):
  // 1. Don't use rich text: No <qt><b>...</b></qt>
  label = new TQLabel( i18n("Please check if encryption really "
  	"works before you start using it seriously. Also note that attachments "
	"are not encrypted by the PGP/GPG module."), group );
  // 2. instead, set the font to bold:
  TQFont labelFont = label->font();
  labelFont.setBold( true );
  label->setFont( labelFont );
  // 3. and activate wordwarp:
  label->setAlignment( AlignLeft|WordBreak );
  // end; to remove the workaround, add <qt><b>..</b></qt> around the
  // text and remove lines TQFont... -> label->setAlignment(...).
  topLayout->addWidget( group );

  group = new TQVGroupBox( i18n("Encryption Tool"), this );
  group->layout()->setSpacing( KDialog::spacingHint() );

  TQHBox * hbox = new TQHBox( group );
  label = new TQLabel( i18n("Select encryption tool to &use:"), hbox );
  toolCombo = new TQComboBox( false, hbox );
  toolCombo->insertStringList( TQStringList()
			       << i18n("Autodetect")
			       << i18n("GnuPG - Gnu Privacy Guard")
			       << i18n("PGP Version 2.x")
			       << i18n("PGP Version 5.x")
			       << i18n("PGP Version 6.x")
			       << i18n("Do not use any encryption tool") );
  label->setBuddy( toolCombo );
  hbox->setStretchFactor( toolCombo, 1 );
  connect( toolCombo, TQT_SIGNAL( activated( int ) ),
           this, TQT_SIGNAL( changed( void ) ) );
  // This is the place to add a KURLRequester to be used for asking
  // the user for the path to the executable...
  topLayout->addWidget( group );

  mpOptionsGroupBox = new TQVGroupBox( i18n("Options"), this );
  mpOptionsGroupBox->layout()->setSpacing( KDialog::spacingHint() );
  storePass = new TQCheckBox( i18n("&Keep passphrase in memory"),
                             mpOptionsGroupBox );
  connect( storePass, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SIGNAL( changed( void ) ) );
  msg = i18n( "<qt><p>When this option is enabled, the passphrase of your "
	      "private key will be remembered by the application as long "
	      "as the application is running. Thus you will only have to "
	      "enter the passphrase once.</p><p>Be aware that this could be a "
	      "security risk. If you leave your computer, others "
	      "can use it to send signed messages and/or read your encrypted "
	      "messages. If a core dump occurs, the contents of your RAM will "
	      "be saved onto disk, including your passphrase.</p>"
	      "<p>Note that when using KMail, this setting only applies "
	      "if you are not using gpg-agent. It is also ignored "
	      "if you are using crypto plugins.</p></qt>" );
  TQWhatsThis::add( storePass, msg );
  if( encrypt ) {
    encToSelf = new TQCheckBox( i18n("Always encr&ypt to self"),
                               mpOptionsGroupBox );
   connect( encToSelf, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SIGNAL( changed( void ) ) );

    msg = i18n( "<qt><p>When this option is enabled, the message/file "
		"will not only be encrypted with the receiver's public key, "
		"but also with your key. This will enable you to decrypt the "
		"message/file at a later time. This is generally a good idea."
		"</p></qt>" );
    TQWhatsThis::add( encToSelf, msg );
  }
  else
    encToSelf = 0;
  showCipherText = new TQCheckBox( i18n("&Show signed/encrypted text after "
                                       "composing"),
                                  mpOptionsGroupBox );
  connect( showCipherText, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SIGNAL( changed( void ) ) );

  msg = i18n( "<qt><p>When this option is enabled, the signed/encrypted text "
	      "will be shown in a separate window, enabling you to know how "
	      "it will look before it is sent. This is a good idea when "
	      "you are verifying that your encryption system works.</p></qt>" );
  TQWhatsThis::add( showCipherText, msg );
  if( encrypt ) {
    showKeyApprovalDlg = new TQCheckBox( i18n("Always show the encryption "
                                             "keys &for approval"),
                                        mpOptionsGroupBox );
    connect( showKeyApprovalDlg, TQT_SIGNAL( toggled( bool ) ),
           this, TQT_SIGNAL( changed( void ) ) );
    msg = i18n( "<qt><p>When this option is enabled, the application will "
		"always show you a list of public keys from which you can "
		"choose the one it will use for encryption. If it is off, "
		"the application will only show the dialog if it cannot find "
		"the right key or if there are several which could be used. "
		"</p></qt>" );
    TQWhatsThis::add( showKeyApprovalDlg, msg );
}
  else
    showKeyApprovalDlg = 0;

  topLayout->addWidget( mpOptionsGroupBox );

  topLayout->addStretch(1);

  setValues();  // is this needed by KNode, b/c for KMail, it's not.
}


Config::~Config()
{
}

void
Config::setValues()
{
  // set default values
  storePass->setChecked( pgp->storePassPhrase() );
  if( 0 != encToSelf )
    encToSelf->setChecked( pgp->encryptToSelf() );
  showCipherText->setChecked( pgp->showCipherText() );
  if( 0 != showKeyApprovalDlg )
    showKeyApprovalDlg->setChecked( pgp->showKeyApprovalDlg() );

  int type = 0;
  switch (pgp->pgpType) {
    // translate Kpgp::Module enum to combobox' entries:
  default:
  case Module::tAuto: type = 0; break;
  case Module::tGPG:  type = 1; break;
  case Module::tPGP2: type = 2; break;
  case Module::tPGP5: type = 3; break;
  case Module::tPGP6: type = 4; break;
  case Module::tOff:  type = 5; break;
  }
  toolCombo->setCurrentItem( type );
}

void
Config::applySettings()
{
  pgp->setStorePassPhrase(storePass->isChecked());
  if( 0 != encToSelf )
    pgp->setEncryptToSelf(encToSelf->isChecked());
  pgp->setShowCipherText(showCipherText->isChecked());
  if( 0 != showKeyApprovalDlg )
    pgp->setShowKeyApprovalDlg( showKeyApprovalDlg->isChecked() );

  Module::PGPType type;
  switch ( toolCombo->currentItem() ) {
    // convert combobox entry indices to Kpgp::Module constants:
  default:
  case 0: type = Module::tAuto; break;
  case 1: type = Module::tGPG;  break;
  case 2: type = Module::tPGP2; break;
  case 3: type = Module::tPGP5; break;
  case 4: type = Module::tPGP6; break;
  case 5: type = Module::tOff;  break;
  }
  pgp->pgpType = type;

  pgp->writeConfig(true);
}



// ------------------------------------------------------------------------
KeySelectionDialog::KeySelectionDialog( const KeyList& keyList,
                                        const TQString& title,
                                        const TQString& text,
                                        const KeyIDList& keyIds,
                                        const bool rememberChoice,
                                        const unsigned int allowedKeys,
                                        const bool extendedSelection,
                                        TQWidget *parent, const char *name,
                                        bool modal )
  : KDialogBase( parent, name, modal, title, Default|Ok|Cancel, Ok ),
    mRememberCB( 0 ),
    mAllowedKeys( allowedKeys ),
    mCurrentContextMenuItem( 0 )
{
  if ( kapp )
    KWin::setIcons( winId(), kapp->icon(), kapp->miniIcon() );
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();
  KConfig *config = pgp->getConfig();
  KConfigGroup dialogConfig( config, "Key Selection Dialog" );

  TQSize defaultSize( 580, 400 );
  TQSize dialogSize = dialogConfig.readSizeEntry( "Dialog size", &defaultSize );

  resize( dialogSize );

  mCheckSelectionTimer = new TQTimer( this, "mCheckSelectionTimer" );
  mStartSearchTimer = new TQTimer( this, "mStartSearchTimer" );

  // load the key status icons
  mKeyGoodPix    = new TQPixmap( UserIcon("key_ok") );
  mKeyBadPix     = new TQPixmap( UserIcon("key_bad") );
  mKeyUnknownPix = new TQPixmap( UserIcon("key_unknown") );
  mKeyValidPix   = new TQPixmap( UserIcon("key") );

  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

  if( !text.isEmpty() ) {
    TQLabel *label = new TQLabel( page );
    label->setText( text );
    topLayout->addWidget( label );
  }

  TQHBoxLayout * hlay = new TQHBoxLayout( topLayout ); // inherits spacing
  TQLineEdit * le = new TQLineEdit( page );
  hlay->addWidget( new TQLabel( le, i18n("&Search for:"), page ) );
  hlay->addWidget( le, 1 );
  le->setFocus();

  connect( le, TQT_SIGNAL(textChanged(const TQString&)),
	   this, TQT_SLOT(slotSearch(const TQString&)) );
  connect( mStartSearchTimer, TQT_SIGNAL(timeout()), TQT_SLOT(slotFilter()) );

  mListView = new KListView( page );
  mListView->addColumn( i18n("Key ID") );
  mListView->addColumn( i18n("User ID") );
  mListView->setAllColumnsShowFocus( true );
  mListView->setResizeMode( TQListView::LastColumn );
  mListView->setRootIsDecorated( true );
  mListView->setShowSortIndicator( true );
  mListView->setSorting( 1, true ); // sort by User ID
  mListView->setShowToolTips( true );
  if( extendedSelection ) {
    mListView->setSelectionMode( TQListView::Extended );
    //mListView->setSelectionMode( TQListView::Multi );
  }
  topLayout->addWidget( mListView, 10 );

  if (rememberChoice) {
    mRememberCB = new TQCheckBox( i18n("Remember choice"), page );
    topLayout->addWidget( mRememberCB );
    TQWhatsThis::add(mRememberCB,
                    i18n("<qt><p>If you check this box your choice will "
                         "be stored and you will not be asked again."
                         "</p></qt>"));
  }

  initKeylist( keyList, keyIds );

  TQListViewItem *lvi;
  if( extendedSelection ) {
    lvi = mListView->currentItem();
    slotCheckSelection();
  }
  else {
    lvi = mListView->selectedItem();
    slotCheckSelection( lvi );
  }
  // make sure that the selected item is visible
  // (ensureItemVisible(...) doesn't work correctly in Qt 3.0.0)
  if( lvi != 0 )
    mListView->center( mListView->contentsX(), mListView->itemPos( lvi ) );

  if( extendedSelection ) {
    connect( mCheckSelectionTimer, TQT_SIGNAL( timeout() ),
             this,                 TQT_SLOT( slotCheckSelection() ) );
    connect( mListView, TQT_SIGNAL( selectionChanged() ),
             this,      TQT_SLOT( slotSelectionChanged() ) );
  }
  else {
    connect( mListView, TQT_SIGNAL( selectionChanged( TQListViewItem* ) ),
             this,      TQT_SLOT( slotSelectionChanged( TQListViewItem* ) ) );
  }
  connect( mListView, TQT_SIGNAL( doubleClicked ( TQListViewItem *, const TQPoint &, int ) ), this, TQT_SLOT( accept() ) );

  connect( mListView, TQT_SIGNAL( contextMenuRequested( TQListViewItem*,
                                                    const TQPoint&, int ) ),
           this,      TQT_SLOT( slotRMB( TQListViewItem*, const TQPoint&, int ) ) );

  setButtonText( KDialogBase::Default, i18n("&Reread Keys") );
  connect( this, TQT_SIGNAL( defaultClicked() ),
           this, TQT_SLOT( slotRereadKeys() ) );
}


KeySelectionDialog::~KeySelectionDialog()
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();
  KConfig *config = pgp->getConfig();
  KConfigGroup dialogConfig( config, "Key Selection Dialog" );
  dialogConfig.writeEntry( "Dialog size", size() );
  config->sync();
  delete mKeyGoodPix;
  delete mKeyBadPix;
  delete mKeyUnknownPix;
  delete mKeyValidPix;
}


KeyID KeySelectionDialog::key() const
{
  if( mListView->isMultiSelection() || mKeyIds.isEmpty() )
    return KeyID();
  else
    return mKeyIds.first();
}


void KeySelectionDialog::initKeylist( const KeyList& keyList,
                                      const KeyIDList& keyIds )
{
  TQListViewItem* firstSelectedItem = 0;
  mKeyIds.clear();
  mListView->clear();

  // build a list of all public keys
  for( KeyListIterator it( keyList ); it.current(); ++it ) {
    KeyID curKeyId = (*it)->primaryKeyID();

    TQListViewItem* primaryUserID = new TQListViewItem( mListView, curKeyId,
                                                      (*it)->primaryUserID() );

    // select and open the given key
    if( keyIds.findIndex( curKeyId ) != -1 ) {
      if( 0 == firstSelectedItem ) {
        firstSelectedItem = primaryUserID;
      }
      mListView->setSelected( primaryUserID, true );
      mKeyIds.append( curKeyId );
    }
    primaryUserID->setOpen( false );

    // set icon for this key
    switch( keyValidity( *it ) ) {
      case 0: // the key's validity can't be determined
        primaryUserID->setPixmap( 0, *mKeyUnknownPix );
        break;
      case 1: // key is valid but not trusted
        primaryUserID->setPixmap( 0, *mKeyValidPix );
        break;
      case 2: // key is valid and trusted
        primaryUserID->setPixmap( 0, *mKeyGoodPix );
        break;
      case -1: // key is invalid
        primaryUserID->setPixmap( 0, *mKeyBadPix );
        break;
    }

    TQListViewItem* childItem;

    childItem = new TQListViewItem( primaryUserID, "",
                                   i18n( "Fingerprint: %1" )
                                   .arg( beautifyFingerprint( (*it)->primaryFingerprint() ) ) );
    if( primaryUserID->isSelected() && mListView->isMultiSelection() ) {
      mListView->setSelected( childItem, true );
    }

    childItem = new TQListViewItem( primaryUserID, "", keyInfo( *it ) );
    if( primaryUserID->isSelected() && mListView->isMultiSelection() ) {
      mListView->setSelected( childItem, true );
    }

    UserIDList userIDs = (*it)->userIDs();
    UserIDListIterator uidit( userIDs );
    if( *uidit ) {
      ++uidit; // skip the primary user ID
      for( ; *uidit; ++uidit ) {
        childItem = new TQListViewItem( primaryUserID, "", (*uidit)->text() );
        if( primaryUserID->isSelected() && mListView->isMultiSelection() ) {
          mListView->setSelected( childItem, true );
        }
      }
    }
  }

  if( 0 != firstSelectedItem ) {
    mListView->setCurrentItem( firstSelectedItem );
  }
}


TQString KeySelectionDialog::keyInfo( const Kpgp::Key *key ) const
{
  TQString status, remark;
  if( key->revoked() ) {
    status = i18n("Revoked");
  }
  else if( key->expired() ) {
    status = i18n("Expired");
  }
  else if( key->disabled() ) {
    status = i18n("Disabled");
  }
  else if( key->invalid() ) {
    status = i18n("Invalid");
  }
  else {
    Validity keyTrust = key->keyTrust();
    switch( keyTrust ) {
    case KPGP_VALIDITY_UNDEFINED:
      status = i18n("Undefined trust");
      break;
    case KPGP_VALIDITY_NEVER:
      status = i18n("Untrusted");
      break;
    case KPGP_VALIDITY_MARGINAL:
      status = i18n("Marginally trusted");
      break;
    case KPGP_VALIDITY_FULL:
      status = i18n("Fully trusted");
      break;
    case KPGP_VALIDITY_ULTIMATE:
      status = i18n("Ultimately trusted");
      break;
    case KPGP_VALIDITY_UNKNOWN:
    default:
      status = i18n("Unknown");
    }
    if( key->secret() ) {
      remark = i18n("Secret key available");
    }
    else if( !key->canEncrypt() ) {
      remark = i18n("Sign only key");
    }
    else if( !key->canSign() ) {
      remark = i18n("Encryption only key");
    }
  }

  TQDateTime dt;
  dt.setTime_t( key->creationDate() );
  if( remark.isEmpty() ) {
    return " " + i18n("creation date and status of an OpenPGP key",
                      "Creation date: %1, Status: %2")
                     .arg( KGlobal::locale()->formatDate( dt.date(), true ) )
                     .arg( status );
  }
  else {
    return " " + i18n("creation date, status and remark of an OpenPGP key",
                      "Creation date: %1, Status: %2 (%3)")
                     .arg( KGlobal::locale()->formatDate( dt.date(), true ) )
                     .arg( status )
                     .arg( remark );
  }
}

TQString KeySelectionDialog::beautifyFingerprint( const TQCString& fpr ) const
{
  TQCString result;

  if( 40 == fpr.length() ) {
    // convert to this format:
    // 0000 1111 2222 3333 4444  5555 6666 7777 8888 9999
    result.fill( ' ', 50 );
    memcpy( result.data()     , fpr.data()     , 4 );
    memcpy( result.data() +  5, fpr.data() +  4, 4 );
    memcpy( result.data() + 10, fpr.data() +  8, 4 );
    memcpy( result.data() + 15, fpr.data() + 12, 4 );
    memcpy( result.data() + 20, fpr.data() + 16, 4 );
    memcpy( result.data() + 26, fpr.data() + 20, 4 );
    memcpy( result.data() + 31, fpr.data() + 24, 4 );
    memcpy( result.data() + 36, fpr.data() + 28, 4 );
    memcpy( result.data() + 41, fpr.data() + 32, 4 );
    memcpy( result.data() + 46, fpr.data() + 36, 4 );
  }
  else if( 32 == fpr.length() ) {
    // convert to this format:
    // 00 11 22 33 44 55 66 77  88 99 AA BB CC DD EE FF
    result.fill( ' ', 48 );
    memcpy( result.data()     , fpr.data()     , 2 );
    memcpy( result.data() +  3, fpr.data() +  2, 2 );
    memcpy( result.data() +  6, fpr.data() +  4, 2 );
    memcpy( result.data() +  9, fpr.data() +  6, 2 );
    memcpy( result.data() + 12, fpr.data() +  8, 2 );
    memcpy( result.data() + 15, fpr.data() + 10, 2 );
    memcpy( result.data() + 18, fpr.data() + 12, 2 );
    memcpy( result.data() + 21, fpr.data() + 14, 2 );
    memcpy( result.data() + 25, fpr.data() + 16, 2 );
    memcpy( result.data() + 28, fpr.data() + 18, 2 );
    memcpy( result.data() + 31, fpr.data() + 20, 2 );
    memcpy( result.data() + 34, fpr.data() + 22, 2 );
    memcpy( result.data() + 37, fpr.data() + 24, 2 );
    memcpy( result.data() + 40, fpr.data() + 26, 2 );
    memcpy( result.data() + 43, fpr.data() + 28, 2 );
    memcpy( result.data() + 46, fpr.data() + 30, 2 );
  }
  else { // unknown length of fingerprint
    result = fpr;
  }

  return result;
}

int KeySelectionDialog::keyValidity( const Kpgp::Key *key ) const
{
  if( 0 == key ) {
    return -1;
  }

  if( ( mAllowedKeys & EncrSignKeys ) == EncryptionKeys ) {
    // only encryption keys are allowed
    if( ( mAllowedKeys & ValidKeys ) && !key->isValidEncryptionKey() ) {
      // only valid encryption keys are allowed
      return -1;
    }
    else if( !key->canEncrypt() ) {
      return -1;
    }
  }
  else if( ( mAllowedKeys & EncrSignKeys ) == SigningKeys ) {
    // only signing keys are allowed
    if( ( mAllowedKeys & ValidKeys ) && !key->isValidSigningKey() ) {
      // only valid signing keys are allowed
      return -1;
    }
    else if( !key->canSign() ) {
      return -1;
    }
  }
  else if( ( mAllowedKeys & ValidKeys ) && !key->isValid() ) {
    // only valid keys are allowed
    return -1;
  }

  // check the key's trust
  int val = 0;
  Validity keyTrust = key->keyTrust();
  switch( keyTrust ) {
  case KPGP_VALIDITY_NEVER:
    val = -1;
    break;
  case KPGP_VALIDITY_MARGINAL:
  case KPGP_VALIDITY_FULL:
  case KPGP_VALIDITY_ULTIMATE:
    val = 2;
    break;
  case KPGP_VALIDITY_UNDEFINED:
    if( mAllowedKeys & TrustedKeys ) {
      // only trusted keys are allowed
      val = -1;
    }
    else {
      val = 1;
    }
    break;
  case KPGP_VALIDITY_UNKNOWN:
  default:
    val = 0;
  }

  return val;
}


void KeySelectionDialog::updateKeyInfo( const Kpgp::Key* key,
                                        TQListViewItem* lvi ) const
{
  if( 0 == lvi ) {
    return;
  }

  if( lvi->parent() != 0 ) {
    lvi = lvi->parent();
  }

  if( 0 == key ) {
    // the key doesn't exist anymore -> delete it from the list view
    while( lvi->firstChild() ) {
      kdDebug(5100) << "Deleting '" << lvi->firstChild()->text( 1 ) << "'\n";
      delete lvi->firstChild();
    }
    kdDebug(5100) << "Deleting key 0x" << lvi->text( 0 ) << " ("
                  << lvi->text( 1 ) << ")\n";
    delete lvi;
    lvi = 0;
    return;
  }

  // update the icon for this key
  switch( keyValidity( key ) ) {
  case 0: // the key's validity can't be determined
    lvi->setPixmap( 0, *mKeyUnknownPix );
    break;
  case 1: // key is valid but not trusted
    lvi->setPixmap( 0, *mKeyValidPix );
    break;
  case 2: // key is valid and trusted
    lvi->setPixmap( 0, *mKeyGoodPix );
    break;
  case -1: // key is invalid
    lvi->setPixmap( 0, *mKeyBadPix );
    break;
  }

  // update the key info for this key
  // the key info is identified by a leading space; this shouldn't be
  // a problem because User Ids shouldn't start with a space
  for( lvi = lvi->firstChild(); lvi; lvi = lvi->nextSibling() ) {
    if( lvi->text( 1 ).at(0) == ' ' ) {
      lvi->setText( 1, keyInfo( key ) );
      break;
    }
  }
}


int
KeySelectionDialog::keyAdmissibility( TQListViewItem* lvi,
                                      TrustCheckMode trustCheckMode ) const
{
  // Return:
  //  -1 = key must not be chosen,
  //   0 = not enough information to decide whether the give key is allowed
  //       or not,
  //   1 = key can be chosen

  if( mAllowedKeys == AllKeys ) {
    return 1;
  }

  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( 0 == pgp ) {
    return 0;
  }

  KeyID keyId = getKeyId( lvi );
  Kpgp::Key* key = pgp->publicKey( keyId );

  if( 0 == key ) {
    return 0;
  }

  int val = 0;
  if( trustCheckMode == ForceTrustCheck ) {
    key = pgp->rereadKey( keyId, true );
    updateKeyInfo( key, lvi );
    val = keyValidity( key );
  }
  else {
    val = keyValidity( key );
    if( ( trustCheckMode == AllowExpensiveTrustCheck ) && ( 0 == val ) ) {
      key = pgp->rereadKey( keyId, true );
      updateKeyInfo( key, lvi );
      val = keyValidity( key );
    }
  }

  switch( val ) {
  case -1: // key is not usable
    return -1;
    break;
  case 0: // key status unknown
    return 0;
    break;
  case 1: // key is valid, but untrusted
    if( mAllowedKeys & TrustedKeys ) {
      // only trusted keys are allowed
      return -1;
    }
    return 1;
    break;
  case 2: // key is trusted
    return 1;
    break;
  default:
    kdDebug( 5100 ) << "Error: Invalid key status value.\n";
  }

  return 0;
}


KeyID
KeySelectionDialog::getKeyId( const TQListViewItem* lvi ) const
{
  KeyID keyId;

  if( 0 != lvi ) {
    if( 0 != lvi->parent() ) {
      keyId = lvi->parent()->text(0).local8Bit();
    }
    else {
      keyId = lvi->text(0).local8Bit();
    }
  }

  return keyId;
}


void KeySelectionDialog::slotRereadKeys()
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( 0 == pgp ) {
    return;
  }

  KeyList keys;

  if( PublicKeys & mAllowedKeys ) {
    pgp->readPublicKeys( true );
    keys = pgp->publicKeys();
  }
  else {
    pgp->readSecretKeys( true );
    keys = pgp->secretKeys();
  }

  // save the current position of the contents
  int offsetY = mListView->contentsY();

  if( mListView->isMultiSelection() ) {
    disconnect( mListView, TQT_SIGNAL( selectionChanged() ),
                this,      TQT_SLOT( slotSelectionChanged() ) );
  }
  else {
    disconnect( mListView, TQT_SIGNAL( selectionChanged( TQListViewItem * ) ),
                this,      TQT_SLOT( slotSelectionChanged( TQListViewItem * ) ) );
  }

  initKeylist( keys, KeyIDList( mKeyIds ) );
  slotFilter();

  if( mListView->isMultiSelection() ) {
    connect( mListView, TQT_SIGNAL( selectionChanged() ),
             this,      TQT_SLOT( slotSelectionChanged() ) );
    slotSelectionChanged();
  }
  else {
    connect( mListView, TQT_SIGNAL( selectionChanged( TQListViewItem * ) ),
             this,      TQT_SLOT( slotSelectionChanged( TQListViewItem * ) ) );
  }

  // restore the saved position of the contents
  mListView->setContentsPos( 0, offsetY );
}


void KeySelectionDialog::slotSelectionChanged( TQListViewItem * lvi )
{
  slotCheckSelection( lvi );
}


void KeySelectionDialog::slotSelectionChanged()
{
  kdDebug(5100) << "KeySelectionDialog::slotSelectionChanged()\n";

  // (re)start the check selection timer. Checking the selection is delayed
  // because else drag-selection doesn't work very good (checking key trust
  // is slow).
  mCheckSelectionTimer->start( sCheckSelectionDelay );
}


void KeySelectionDialog::slotCheckSelection( TQListViewItem* plvi /* = 0 */ )
{
  kdDebug(5100) << "KeySelectionDialog::slotCheckSelection()\n";

  if( !mListView->isMultiSelection() ) {
    mKeyIds.clear();
    KeyID keyId = getKeyId( plvi );
    if( !keyId.isEmpty() ) {
      mKeyIds.append( keyId );
      enableButtonOK( 1 == keyAdmissibility( plvi, AllowExpensiveTrustCheck ) );
    }
    else {
      enableButtonOK( false );
    }
  }
  else {
    mCheckSelectionTimer->stop();

    // As we might change the selection, we have to disconnect the slot
    // to prevent recursion
    disconnect( mListView, TQT_SIGNAL( selectionChanged() ),
                this,      TQT_SLOT( slotSelectionChanged() ) );

    KeyIDList newKeyIdList;
    TQValueList<TQListViewItem*> keysToBeChecked;

    bool keysAllowed = true;
    enum { UNKNOWN, SELECTED, DESELECTED } userAction = UNKNOWN;
    // Iterate over the tree to find selected keys.
    for( TQListViewItem *lvi = mListView->firstChild();
         0 != lvi;
         lvi = lvi->nextSibling() ) {
      // We make sure that either all items belonging to a key are selected
      // or unselected. As it's possible to select/deselect multiple keys at
      // once in extended selection mode we have to figure out whether the user
      // selected or deselected keys.

      // First count the selected items of this key
      int itemCount = 1 + lvi->childCount();
      int selectedCount = lvi->isSelected() ? 1 : 0;
      for( TQListViewItem *clvi = lvi->firstChild();
           0 != clvi;
           clvi = clvi->nextSibling() ) {
        if( clvi->isSelected() ) {
          ++selectedCount;
        }
      }

      if( userAction == UNKNOWN ) {
        // Figure out whether the user selected or deselected this key
        // Remark: A selected count of 0 doesn't mean anything since in
        //         extended selection mode a normal left click deselects
        //         the not clicked items.
        if( 0 < selectedCount ) {
          if( -1 == mKeyIds.findIndex( lvi->text(0).local8Bit() ) ) {
            // some items of this key are selected and the key wasn't selected
            // before => the user selected something
            kdDebug(5100) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User selected key "<<lvi->text(0)<<endl;
            userAction = SELECTED;
          }
          else if( ( itemCount > selectedCount ) &&
                   ( -1 != mKeyIds.findIndex( lvi->text(0).local8Bit() ) ) ) {
            // some items of this key are unselected and the key was selected
            // before => the user deselected something
            kdDebug(5100) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User deselected key "<<lvi->text(0)<<endl;
            userAction = DESELECTED;
          }
        }
      }
      if( itemCount == selectedCount ) {
        // add key to the list of selected keys
        KeyID keyId = lvi->text(0).local8Bit();
        newKeyIdList.append( keyId );
        int admissibility = keyAdmissibility( lvi, NoExpensiveTrustCheck );
        if( -1 == admissibility ) {
          keysAllowed = false;
        }
        else if ( 0 == admissibility ) {
          keysToBeChecked.append( lvi );
        }
      }
      else if( 0 < selectedCount ) {
        // not all items of this key are selected or unselected. change this
        // according to the user's action
        if( userAction == SELECTED ) {
          // select all items of this key
          mListView->setSelected( lvi, true );
          for( TQListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, true );
          }
          // add key to the list of selected keys
          KeyID keyId = lvi->text(0).local8Bit();
          newKeyIdList.append( keyId );
          int admissibility = keyAdmissibility( lvi, NoExpensiveTrustCheck );
          if( -1 == admissibility ) {
            keysAllowed = false;
          }
          else if ( 0 == admissibility ) {
            keysToBeChecked.append( lvi );
          }
        }
        else { // userAction == DESELECTED
          // deselect all items of this key
          mListView->setSelected( lvi, false );
          for( TQListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, false );
          }
        }
      }
    }
    kdDebug(5100) << "Selected keys: " << newKeyIdList.toStringList().join(", ") << endl;
    mKeyIds = newKeyIdList;
    if( !keysToBeChecked.isEmpty() ) {
      keysAllowed = keysAllowed && checkKeys( keysToBeChecked );
    }
    enableButtonOK( keysAllowed );

    connect( mListView, TQT_SIGNAL( selectionChanged() ),
             this,      TQT_SLOT( slotSelectionChanged() ) );
  }
}


bool KeySelectionDialog::checkKeys( const TQValueList<TQListViewItem*>& keys ) const
{
  KProgressDialog* pProgressDlg = 0;
  bool keysAllowed = true;
  kdDebug(5100) << "Checking keys...\n";

  pProgressDlg = new KProgressDialog( 0, 0, i18n("Checking Keys"),
                                      i18n("Checking key 0xMMMMMMMM..."),
                                      true );
  pProgressDlg->setAllowCancel( false );
  pProgressDlg->progressBar()->setTotalSteps( keys.count() );
  pProgressDlg->setMinimumDuration( 1000 );
  pProgressDlg->show();

  for( TQValueList<TQListViewItem*>::ConstIterator it = keys.begin();
       it != keys.end();
       ++it ) {
    kdDebug(5100) << "Checking key 0x" << getKeyId( *it ) << "...\n";
    pProgressDlg->setLabel( i18n("Checking key 0x%1...")
                            .arg( getKeyId( *it ) ) );
    kapp->processEvents();
    keysAllowed = keysAllowed && ( -1 != keyAdmissibility( *it, AllowExpensiveTrustCheck ) );
    pProgressDlg->progressBar()->advance( 1 );
    kapp->processEvents();
  }

  delete pProgressDlg;
  pProgressDlg = 0;

  return keysAllowed;
}


void KeySelectionDialog::slotRMB( TQListViewItem* lvi, const TQPoint& pos, int )
{
  if( !lvi ) {
    return;
  }

  mCurrentContextMenuItem = lvi;

  TQPopupMenu menu(this);
  menu.insertItem( i18n( "Recheck Key" ), this, TQT_SLOT( slotRecheckKey() ) );
  menu.exec( pos );
}


void KeySelectionDialog::slotRecheckKey()
{
  if( 0 != mCurrentContextMenuItem ) {
    // force rereading the key
    keyAdmissibility( mCurrentContextMenuItem, ForceTrustCheck );
    // recheck the selection
    slotCheckSelection( mCurrentContextMenuItem );
  }
}

void KeySelectionDialog::slotOk()
{
  if( mCheckSelectionTimer->isActive() ) {
    slotCheckSelection();
  }
  mStartSearchTimer->stop();
  accept();
}


void KeySelectionDialog::slotCancel()
{
  mCheckSelectionTimer->stop();
  mStartSearchTimer->stop();
  mKeyIds.clear();
  reject();
}

void KeySelectionDialog::slotSearch( const TQString & text )
{
  mSearchText = text.stripWhiteSpace().upper();
  mStartSearchTimer->start( sCheckSelectionDelay, true /*single-shot*/ );
}

void KeySelectionDialog::slotFilter()
{
  if ( mSearchText.isEmpty() ) {
    showAllItems();
    return;
  }

  // OK, so we need to filter:
  TQRegExp keyIdRegExp( "(?:0x)?[A-F0-9]{1,8}", false /*case-insens.*/ );
  if ( keyIdRegExp.exactMatch( mSearchText ) ) {
    if ( mSearchText.startsWith( "0X" ) )
      // search for keyID only:
      filterByKeyID( mSearchText.mid( 2 ) );
    else
      // search for UID and keyID:
      filterByKeyIDOrUID( mSearchText );
  } else {
    // search in UID:
    filterByUID( mSearchText );
  }
}

void KeySelectionDialog::filterByKeyID( const TQString & keyID )
{
  assert( keyID.length() <= 8 );
  assert( !keyID.isEmpty() ); // regexp in slotFilter should prevent these
  if ( keyID.isEmpty() )
    showAllItems();
  else
    for ( TQListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
      item->setVisible( item->text( 0 ).upper().startsWith( keyID ) );
}

void KeySelectionDialog::filterByKeyIDOrUID( const TQString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  TQRegExp rx( "\\b" + TQRegExp::escape( str ), false );

  for ( TQListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( item->text( 0 ).upper().startsWith( str )
		      || rx.search( item->text( 1 ) ) >= 0
		      || anyChildMatches( item, rx ) );

}

void KeySelectionDialog::filterByUID( const TQString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  TQRegExp rx( "\\b" + TQRegExp::escape( str ), false );

  for ( TQListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( rx.search( item->text( 1 ) ) >= 0
		      || anyChildMatches( item, rx ) );
}


bool KeySelectionDialog::anyChildMatches( const TQListViewItem * item, TQRegExp & rx ) const
{
  if ( !item )
    return false;

  TQListViewItem * stop = item->nextSibling(); // It's OK if stop is NULL...

  for ( TQListViewItemIterator it( item->firstChild() ) ; it.current() && it.current() != stop ; ++it )
    if ( rx.search( it.current()->text( 1 ) ) >= 0 ) {
      //item->setOpen( true ); // do we want that?
      return true;
    }
  return false;
}

void KeySelectionDialog::showAllItems()
{
  for ( TQListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( true );
}

// ------------------------------------------------------------------------
KeyRequester::KeyRequester( TQWidget * parent, bool multipleKeys,
			    unsigned int allowedKeys, const char * name )
  : TQWidget( parent, name ),
    mDialogCaption( i18n("OpenPGP Key Selection") ),
    mDialogMessage( i18n("Please select an OpenPGP key to use.") ),
    mMulti( multipleKeys ),
    mAllowedKeys( allowedKeys ),
    d( 0 )
{
  TQHBoxLayout * hlay = new TQHBoxLayout( this, 0, KDialog::spacingHint() );

  // the label where the key id is to be displayed:
  mLabel = new TQLabel( this );
  mLabel->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );

  // the button to unset any key:
  mEraseButton = new TQPushButton( this );
  mEraseButton->setAutoDefault( false );
  mEraseButton->setSizePolicy( TQSizePolicy( TQSizePolicy::Minimum,
					    TQSizePolicy::Minimum ) );
  mEraseButton->setPixmap( SmallIcon( "clear_left" ) );
  TQToolTip::add( mEraseButton, i18n("Clear") );

  // the button to call the KeySelectionDialog:
  mDialogButton = new TQPushButton( i18n("Change..."), this );
  mDialogButton->setAutoDefault( false );

  hlay->addWidget( mLabel, 1 );
  hlay->addWidget( mEraseButton );
  hlay->addWidget( mDialogButton );

  connect( mEraseButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotEraseButtonClicked()) );
  connect( mDialogButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotDialogButtonClicked()) );

  setSizePolicy( TQSizePolicy( TQSizePolicy::MinimumExpanding,
			      TQSizePolicy::Fixed ) );
}

KeyRequester::~KeyRequester() {

}

KeyIDList KeyRequester::keyIDs() const {
  return mKeys;
}

void KeyRequester::setKeyIDs( const KeyIDList & keyIDs ) {
  mKeys = keyIDs;
  if ( mKeys.empty() ) {
    mLabel->clear();
    return;
  }
  if ( mKeys.size() > 1 )
    setMultipleKeysEnabled( true );

  TQString s = mKeys.toStringList().join(", ");

  mLabel->setText( s );
  TQToolTip::remove( mLabel );
  TQToolTip::add( mLabel, s );
}

void KeyRequester::slotDialogButtonClicked() {
  Module * pgp = Module::getKpgp();

  if ( !pgp ) {
    kdWarning() << "Kpgp::KeyRequester::slotDialogButtonClicked(): No pgp module found!" << endl;
    return;
  }

  setKeyIDs( keyRequestHook( pgp ) );
  emit changed();
}

void KeyRequester::slotEraseButtonClicked() {
  mKeys.clear();
  mLabel->clear();
  emit changed();
}

void KeyRequester::setDialogCaption( const TQString & caption ) {
  mDialogCaption = caption;
}

void KeyRequester::setDialogMessage( const TQString & msg ) {
  mDialogMessage = msg;
}

bool KeyRequester::isMultipleKeysEnabled() const {
  return mMulti;
}

void KeyRequester::setMultipleKeysEnabled( bool multi ) {
  if ( multi == mMulti ) return;

  if ( !multi && mKeys.size() > 1 )
    mKeys.erase( ++mKeys.begin(), mKeys.end() );

  mMulti = multi;
}

int KeyRequester::allowedKeys() const {
  return mAllowedKeys;
}

void KeyRequester::setAllowedKeys( int allowedKeys ) {
  mAllowedKeys = allowedKeys;
}


PublicKeyRequester::PublicKeyRequester( TQWidget * parent, bool multi,
					unsigned int allowed, const char * name )
  : KeyRequester( parent, multi, allowed & ~SecretKeys, name )
{

}

PublicKeyRequester::~PublicKeyRequester() {

}

KeyIDList PublicKeyRequester::keyRequestHook( Module * pgp ) const {
  assert( pgp );
  return pgp->selectPublicKeys( mDialogCaption, mDialogMessage, mKeys, TQString::null, mAllowedKeys );
}

SecretKeyRequester::SecretKeyRequester( TQWidget * parent, bool multi,
					unsigned int allowed, const char * name )
  : KeyRequester( parent, multi, allowed & ~PublicKeys, name )
{

}

SecretKeyRequester::~SecretKeyRequester() {

}

KeyIDList SecretKeyRequester::keyRequestHook( Module * pgp ) const {
  assert( pgp );

  KeyID keyID = mKeys.first();
  keyID = pgp->selectSecretKey( mDialogCaption, mDialogMessage, keyID );

  return KeyIDList() << keyID;
}



// ------------------------------------------------------------------------
KeyApprovalDialog::KeyApprovalDialog( const TQStringList& addresses,
                                      const TQValueVector<KeyIDList>& keyIDs,
                                      const int allowedKeys,
                                      TQWidget *parent, const char *name,
                                      bool modal )
  : KDialogBase( parent, name, modal, i18n("Encryption Key Approval"),
                 Ok|Cancel, Ok ),
    mKeys( keyIDs ),
    mAllowedKeys( allowedKeys ),
    mPrefsChanged( false )
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return;

  // ##### error handling
  // if( addresses.isEmpty() || keyList.isEmpty() ||
  //     addresses.count()+1 != keyList.count() )
  //   do something;

  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n("The following keys will be used for "
                                   "encryption:"),
                              page );
  topLayout->addWidget( label );

  TQScrollView* sv = new TQScrollView( page );
  sv->setResizePolicy( TQScrollView::AutoOneFit );
  topLayout->addWidget( sv );
  TQVBox* bigvbox = new TQVBox( sv->viewport() );
  bigvbox->setMargin( KDialog::marginHint() );
  bigvbox->setSpacing( KDialog::spacingHint() );
  sv->addChild( bigvbox );

  TQButtonGroup *mChangeButtonGroup = new TQButtonGroup( bigvbox );
  mChangeButtonGroup->hide();
  mAddressLabels.resize( addresses.count() );
  mKeyIdsLabels.resize( keyIDs.size() );
  //mKeyIdListBoxes.resize( keyIDs.size() );
  mEncrPrefCombos.resize( addresses.count() );

  // the sender's key
  if( pgp->encryptToSelf() ) {
    mEncryptToSelf = 1;
    TQHBox* hbox = new TQHBox( bigvbox );
    new TQLabel( i18n("Your keys:"), hbox );
    TQLabel* keyidsL = new TQLabel( hbox );
    if( keyIDs[0].isEmpty() ) {
      keyidsL->setText( i18n("<none> means 'no key'", "<none>") );
    }
    else {
      keyidsL->setText( "0x" + keyIDs[0].toStringList().join( "\n0x" ) );
    }
    keyidsL->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    /*
    TQListBox* keyidLB = new TQListBox( hbox );
    if( keyIDs[0].isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( keyIDs[0].toStringList() );
    }
    keyidLB->setSelectionMode( TQListBox::NoSelection );
    keyidLB->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    */
    TQPushButton *button = new TQPushButton( i18n("Change..."), hbox );
    mChangeButtonGroup->insert( button );
    button->setAutoDefault( false );
    hbox->setStretchFactor( keyidsL, 10 );
    mKeyIdsLabels.insert( 0, keyidsL );
    //hbox->setStretchFactor( keyidLB, 10 );
    //mKeyIdListBoxes.insert( 0, keyidLB );

    new KSeparator( Horizontal, bigvbox );
  }
  else {
    mEncryptToSelf = 0;
    // insert dummy KeyIdListBox
    mKeyIdsLabels.insert( 0, 0 );
    //mKeyIdListBoxes.insert( 0, 0 );
  }

  TQStringList::ConstIterator ait;
  TQValueVector<KeyIDList>::const_iterator kit;
  int i;
  for( ait = addresses.begin(), kit = keyIDs.begin(), i = 0;
       ( ait != addresses.end() ) && ( kit != keyIDs.end() );
       ++ait, ++kit, ++i ) {
    if( i == 0 ) {
      ++kit; // skip the sender's key id
    }
    else {
      new KSeparator( Horizontal, bigvbox );
    }

    TQHBox *hbox = new TQHBox( bigvbox );
    new TQLabel( i18n("Recipient:"), hbox );
    TQLabel *addressL = new TQLabel( *ait, hbox );
    hbox->setStretchFactor( addressL, 10 );
    mAddressLabels.insert( i, addressL  );

    hbox = new TQHBox( bigvbox );
    new TQLabel( i18n("Encryption keys:"), hbox );
    TQLabel* keyidsL = new TQLabel( hbox );
    if( (*kit).isEmpty() ) {
      keyidsL->setText( i18n("<none> means 'no key'", "<none>") );
    }
    else {
      keyidsL->setText( "0x" + (*kit).toStringList().join( "\n0x" ) );
    }
    keyidsL->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    /*
    TQListBox* keyidLB = new TQListBox( hbox );
    if( (*kit).isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( (*kit).toStringList() );
    }
    keyidLB->setSelectionMode( TQListBox::NoSelection );
    keyidLB->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    */
    TQPushButton *button = new TQPushButton( i18n("Change..."), hbox );
    mChangeButtonGroup->insert( button );
    button->setAutoDefault( false );
    hbox->setStretchFactor( keyidsL, 10 );
    mKeyIdsLabels.insert( i + 1, keyidsL );
    //hbox->setStretchFactor( keyidLB, 10 );
    //mKeyIdListBoxes.insert( i + 1, keyidLB );

    hbox = new TQHBox( bigvbox );
    new TQLabel( i18n("Encryption preference:"), hbox );
    TQComboBox *encrPrefCombo = new TQComboBox( hbox );
    encrPrefCombo->insertItem( i18n("<none>") );
    encrPrefCombo->insertItem( i18n("Never Encrypt with This Key") );
    encrPrefCombo->insertItem( i18n("Always Encrypt with This Key") );
    encrPrefCombo->insertItem( i18n("Encrypt Whenever Encryption is Possible") );
    encrPrefCombo->insertItem( i18n("Always Ask") );
    encrPrefCombo->insertItem( i18n("Ask Whenever Encryption is Possible") );

    EncryptPref encrPref = pgp->encryptionPreference( *ait );
    switch( encrPref ) {
      case NeverEncrypt:
        encrPrefCombo->setCurrentItem( 1 );
        break;
      case AlwaysEncrypt:
        encrPrefCombo->setCurrentItem( 2 );
        break;
      case AlwaysEncryptIfPossible:
        encrPrefCombo->setCurrentItem( 3 );
        break;
      case AlwaysAskForEncryption:
        encrPrefCombo->setCurrentItem( 4 );
        break;
      case AskWheneverPossible:
        encrPrefCombo->setCurrentItem( 5 );
        break;
      default:
        encrPrefCombo->setCurrentItem( 0 );
    }
    connect( encrPrefCombo, TQT_SIGNAL(activated(int)),
             this, TQT_SLOT(slotPrefsChanged(int)) );
    mEncrPrefCombos.insert( i, encrPrefCombo );
  }
  connect( mChangeButtonGroup, TQT_SIGNAL(clicked(int)),
           this, TQT_SLOT(slotChangeEncryptionKey(int)) );

  // calculate the optimal width for the dialog
  int dialogWidth = marginHint()
                  + sv->frameWidth()
                  + bigvbox->sizeHint().width()
                  + sv->verticalScrollBar()->sizeHint().width()
                  + sv->frameWidth()
                  + marginHint()
                  + 2;
  // calculate the optimal height for the dialog
  int dialogHeight = marginHint()
                   + label->sizeHint().height()
                   + topLayout->spacing()
                   + sv->frameWidth()
                   + bigvbox->sizeHint().height()
                   + sv->horizontalScrollBar()->sizeHint().height()
                   + sv->frameWidth()
                   + topLayout->spacing()
                   + actionButton( KDialogBase::Cancel )->sizeHint().height()
                   + marginHint()
                   + 2;
  // don't make the dialog too large
  TQRect desk = KGlobalSettings::desktopGeometry(this);
  int screenWidth = desk.width();
  if( dialogWidth > 3*screenWidth/4 )
    dialogWidth = 3*screenWidth/4;
  int screenHeight = desk.height();
  if( dialogHeight > 7*screenHeight/8 )
    dialogHeight = 7*screenHeight/8;

  setInitialSize( TQSize( dialogWidth, dialogHeight ) );
}

void
KeyApprovalDialog::slotChangeEncryptionKey( int nr )
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  kdDebug(5100)<<"Key approval dialog size is "
               <<width()<<"x"<<height()<<endl;

  if( pgp == 0 )
    return;

  if( !mEncryptToSelf )
    nr++;
  KeyIDList keyIds = mKeys[nr];
  if( nr == 0 ) {
    keyIds = pgp->selectPublicKeys( i18n("Encryption Key Selection"),
                                    i18n("if in your language something like "
                                         "'key(s)' isn't possible please "
                                         "use the plural in the translation",
                                         "Select the key(s) which should "
                                         "be used to encrypt the message "
                                         "to yourself."),
                                    keyIds,
                                    "",
                                    mAllowedKeys );
  }
  else {
    keyIds = pgp->selectPublicKeys( i18n("Encryption Key Selection"),
                                    i18n("if in your language something like "
                                         "'key(s)' isn't possible please "
                                         "use the plural in the translation",
                                         "Select the key(s) which should "
                                         "be used to encrypt the message "
                                         "for\n%1")
                                    .arg( mAddressLabels[nr-1]->text() ),
                                    keyIds,
                                    mAddressLabels[nr-1]->text(),
                                    mAllowedKeys );
  }
  if( !keyIds.isEmpty() ) {
    mKeys[nr] = keyIds;
    TQLabel* keyidsL = mKeyIdsLabels[nr];
    keyidsL->setText( "0x" + keyIds.toStringList().join( "\n0x" ) );
    /*
    TQListBox* qlb = mKeyIdListBoxes[nr];
    qlb->clear();
    qlb->insertStringList( keyIds.toStringList() );
    */
  }
}


void
KeyApprovalDialog::slotOk()
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 ) {
    accept();
    return;
  }

  if( mPrefsChanged ) {
    // store the changed preferences
    for( unsigned int i = 0; i < mAddressLabels.size(); i++ ) {
      // traverse all Address and Encryption Preference widgets
      EncryptPref encrPref;
      switch( mEncrPrefCombos[i]->currentItem() ) {
        case 1:
          encrPref = NeverEncrypt;
          break;
        case 2:
          encrPref = AlwaysEncrypt;
          break;
        case 3:
          encrPref = AlwaysEncryptIfPossible;
          break;
        case 4:
          encrPref = AlwaysAskForEncryption;
          break;
        case 5:
          encrPref = AskWheneverPossible;
          break;
        default:
        case 0:
          encrPref = UnknownEncryptPref;
      }
      pgp->setEncryptionPreference( mAddressLabels[i]->text(), encrPref );
    }
  }

  accept();
}


void
KeyApprovalDialog::slotCancel()
{
  reject();
}



// ------------------------------------------------------------------------
CipherTextDialog::CipherTextDialog( const TQCString & text,
                                    const TQCString & charset, TQWidget *parent,
                                    const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("OpenPGP Information"), Ok|Cancel, Ok)
{
  // FIXME (post KDE2.2): show some more info, e.g. the output of GnuPG/PGP
  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

  TQLabel *label = new TQLabel( page );
  label->setText(i18n("Result of the last encryption/sign operation:"));
  topLayout->addWidget( label );

  mEditBox = new TQMultiLineEdit( page );
  mEditBox->setReadOnly(true);
  topLayout->addWidget( mEditBox, 10 );

  TQString unicodeText;
  if (charset.isEmpty())
    unicodeText = TQString::fromLocal8Bit(text.data());
  else {
    bool ok=true;
    TQTextCodec *codec = KGlobal::charsets()->codecForName(charset, ok);
    if(!ok)
      unicodeText = TQString::fromLocal8Bit(text.data());
    else
      unicodeText = codec->toUnicode(text.data(), text.length());
  }

  mEditBox->setText(unicodeText);

  setMinimumSize();
}

void CipherTextDialog::setMinimumSize()
{
  // this seems to force a layout of the entire document, so we get a
  // a proper contentsWidth(). Is there a better way?
  for ( int i = 0; i < mEditBox->paragraphs(); i++ )
      (void) mEditBox->paragraphRect( i );

  mEditBox->setMinimumHeight( mEditBox->fontMetrics().lineSpacing() * 25 );

  int textWidth = mEditBox->contentsWidth() + 30;


#if KDE_IS_VERSION( 3, 1, 90 )
  int maxWidth = KGlobalSettings::desktopGeometry(parentWidget()).width()-100;
#else
  KConfig gc("kdeglobals", false, false);
  gc.setGroup("Windows");
  int maxWidth;
  if (TQApplication::desktop()->isVirtualDesktop() &&
      gc.readBoolEntry("XineramaEnabled", true) &&
      gc.readBoolEntry("XineramaPlacementEnabled", true)) {
    maxWidth = TQApplication::desktop()->screenGeometry(TQApplication::desktop()->screenNumber(parentWidget())).width()-100;
  } else {
    maxWidth = TQApplication::desktop()->geometry().width()-100;
  }
#endif

  mEditBox->setMinimumWidth( QMIN( textWidth, maxWidth ) );
}

void KeyRequester::virtual_hook( int, void* ) {}

void PublicKeyRequester::virtual_hook( int id, void* data ) {
  base::virtual_hook( id, data );
}

void SecretKeyRequester::virtual_hook( int id, void* data ) {
  base::virtual_hook( id, data );
}

} // namespace Kpgp



#include "kpgpui.moc"
