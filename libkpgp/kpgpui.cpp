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



#include <QLabel>

#include <QApplication>
#include <QTextCodec>
#include <QDateTime>
#include <QPixmap>
#include <QLayout>
#include <QTimer>
#include <QMenu>
#include <QRegExp>
#include <QFrame>
#include <QByteArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <qbuttongroup.h>
#include <q3multilineedit.h>
#include <QGroupBox>

#include <klocale.h>
#include <kpassworddialog.h>
#include <kcharsets.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <k3listview.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <kprogressdialog.h>
#include <QApplication>
#include <kwindowsystem.h>
#include <kpushbutton.h>
#include <kglobalsettings.h>
#include <klineedit.h>

#include "kpgp.h"
#include "kpgpui.h"
#include "kpgpkey.h"
#include <assert.h>
#include <string.h> // for memcpy(3)
#include <kvbox.h>
#include <kconfiggroup.h>

const int Kpgp::KeySelectionDialog::sCheckSelectionDelay = 250;

namespace Kpgp {

PassphraseDialog::PassphraseDialog( QWidget *parent,
                                    const QString &caption,
                                    const QString &keyID )
  :KPasswordDialog( parent )
{
  setCaption( caption );
  setButtons( Ok|Cancel );

  setPixmap( BarIcon("pgp-keys") );

  if (keyID.isNull())
    setPrompt(i18n("Please enter your OpenPGP passphrase:"));
  else
    setPrompt(i18n("Please enter the OpenPGP passphrase for\n\"%1\":", keyID) );
}


PassphraseDialog::~PassphraseDialog()
{
}

QString PassphraseDialog::passphrase()
{
  return password();
}


// ------------------------------------------------------------------------
// Forbidden accels for KMail: AC GH OP
//                  for KNode: ACE H O
Config::Config( QWidget *parent, bool encrypt )
  : QWidget( parent ), pgp( Module::getKpgp() )
{
  QGroupBox * group;
  QLabel    * label;
  QString     msg;


  QVBoxLayout *topLayout = new QVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  group = new QGroupBox( i18n("Warning"), this );
  QVBoxLayout *lay = new QVBoxLayout(group);
  lay->setSpacing( KDialog::spacingHint() );
  // (mmutz) work around Qt label bug in 3.0.0 (and possibly later):
  // 1. Don't use rich text: No <qt><b>...</b></qt>
  label = new QLabel( i18n("Please check if encryption really "
  	"works before you start using it seriously. Also note that attachments "
	"are not encrypted by the PGP/GPG module."), group );
  label->setWordWrap( true );
  lay->addWidget( label );
  // 2. instead, set the font to bold:
  QFont labelFont = label->font();
  labelFont.setBold( true );
  label->setFont( labelFont );
  // 3. and activate wordwarp:
  // end; to remove the workaround, add <qt><b>..</b></qt> around the
  // text and remove lines QFont... -> label->setAlignment(...).
  topLayout->addWidget( group );
  group = new QGroupBox( i18n("Encryption Tool"), this );
  lay = new QVBoxLayout(group);
  lay->setSpacing( KDialog::spacingHint() );

  KHBox * hbox = new KHBox( group );
  lay->addWidget( hbox );
  label = new QLabel( i18n("Select encryption tool to &use:"), hbox );
  toolCombo = new QComboBox( hbox );
  toolCombo->setEditable( false );
  toolCombo->addItems( QStringList()
			       << i18n("Autodetect")
			       << i18n("GnuPG - Gnu Privacy Guard")
			       << i18n("PGP Version 2.x")
			       << i18n("PGP Version 5.x")
			       << i18n("PGP Version 6.x")
			       << i18n("Do not use any encryption tool") );
  label->setBuddy( toolCombo );
  hbox->setStretchFactor( toolCombo, 1 );
  connect( toolCombo, SIGNAL( activated( int ) ),
           this, SIGNAL( changed( void ) ) );
  // This is the place to add a KUrlRequester to be used for asking
  // the user for the path to the executable...
  topLayout->addWidget( group );

  mpOptionsGroupBox = new QGroupBox( i18n("Options"), this );
  lay = new QVBoxLayout( mpOptionsGroupBox );
  lay->setSpacing( KDialog::spacingHint() );
  storePass = new QCheckBox( i18n("&Keep passphrase in memory"),
                             mpOptionsGroupBox );
  lay->addWidget( storePass );
  connect( storePass, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed( void ) ) );
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
  storePass->setWhatsThis( msg );
  if( encrypt ) {
    encToSelf = new QCheckBox( i18n("Always encr&ypt to self"),
                               mpOptionsGroupBox );
   connect( encToSelf, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed( void ) ) );

    msg = i18n( "<qt><p>When this option is enabled, the message/file "
		"will not only be encrypted with the receiver's public key, "
		"but also with your key. This will enable you to decrypt the "
		"message/file at a later time. This is generally a good idea."
		"</p></qt>" );
    encToSelf->setWhatsThis( msg );
  }
  else
    encToSelf = 0;
  showCipherText = new QCheckBox( i18n("&Show signed/encrypted text after "
                                       "composing"));
  lay->addWidget( showCipherText );
  connect( showCipherText, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed( void ) ) );

  msg = i18n( "<qt><p>When this option is enabled, the signed/encrypted text "
	      "will be shown in a separate window, enabling you to know how "
	      "it will look before it is sent. This is a good idea when "
	      "you are verifying that your encryption system works.</p></qt>" );
  showCipherText->setWhatsThis( msg );
  if( encrypt ) {
    showKeyApprovalDlg = new QCheckBox( i18n("Always show the encryption "
                                             "keys &for approval"));
    lay->addWidget( showKeyApprovalDlg );
    connect( showKeyApprovalDlg, SIGNAL( toggled( bool ) ),
           this, SIGNAL( changed( void ) ) );
    msg = i18n( "<qt><p>When this option is enabled, the application will "
		"always show you a list of public keys from which you can "
		"choose the one it will use for encryption. If it is off, "
		"the application will only show the dialog if it cannot find "
		"the right key or if there are several which could be used. "
		"</p></qt>" );
    showKeyApprovalDlg->setWhatsThis( msg );
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
  toolCombo->setCurrentIndex( type );
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
  switch ( toolCombo->currentIndex() ) {
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
                                        const QString& title,
                                        const QString& text,
                                        const KeyIDList& keyIds,
                                        const bool rememberChoice,
                                        const unsigned int allowedKeys,
                                        const bool extendedSelection,
                                        QWidget *parent )
  : KDialog( parent ),
    mRememberCB( 0 ),
    mAllowedKeys( allowedKeys ),
    mCurrentContextMenuItem( 0 )
{
  setCaption( title );
  setButtons( Default|Ok|Cancel );
  if ( qApp ) {
    KWindowSystem::setIcons( winId(),
                             qApp->windowIcon().pixmap( IconSize( K3Icon::Desktop ),
                                                        IconSize( K3Icon::Desktop ) ),
                             qApp->windowIcon().pixmap( IconSize( K3Icon::Small ),
                                                        IconSize( K3Icon::Small ) ) );
  }
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();
  KConfig *config = pgp->getConfig();
  KConfigGroup dialogConfig( config, "Key Selection Dialog" );

  QSize defaultSize( 580, 400 );
  QSize dialogSize = dialogConfig.readEntry( "Dialog size", defaultSize );

  resize( dialogSize );

  mCheckSelectionTimer = new QTimer( this );
  mStartSearchTimer = new QTimer( this );
  mStartSearchTimer->setSingleShot( true );

  // load the key status icons
  mKeyGoodPix    = new QPixmap( UserIcon("key_ok") );
  mKeyBadPix     = new QPixmap( UserIcon("key_bad") );
  mKeyUnknownPix = new QPixmap( UserIcon("key_unknown") );
  mKeyValidPix   = new QPixmap( UserIcon("key") );

  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  if( !text.isEmpty() ) {
    QLabel *label = new QLabel( page );
    label->setText( text );
    topLayout->addWidget( label );
  }

  QHBoxLayout * hlay = new QHBoxLayout(); // inherits spacing
  topLayout->addLayout( hlay );
  QLineEdit * le = new QLineEdit( page );
  QLabel *label = new QLabel( i18n("&Search for:"), page );
  label->setBuddy( le );
  hlay->addWidget( label );
  hlay->addWidget( le, 1 );
  le->setFocus();

  connect( le, SIGNAL(textChanged(const QString&)),
	   this, SLOT(slotSearch(const QString&)) );
  connect( mStartSearchTimer, SIGNAL(timeout()), SLOT(slotFilter()) );

  mListView = new K3ListView( page );
  mListView->addColumn( i18n("Key ID") );
  mListView->addColumn( i18n("User ID") );
  mListView->setAllColumnsShowFocus( true );
  mListView->setResizeMode( Q3ListView::LastColumn );
  mListView->setRootIsDecorated( true );
  mListView->setShowSortIndicator( true );
  mListView->setSorting( 1, true ); // sort by User ID
  mListView->setShowToolTips( true );
  if( extendedSelection ) {
    mListView->setSelectionMode( Q3ListView::Extended );
    //mListView->setSelectionMode( QListView::Multi );
  }
  topLayout->addWidget( mListView, 10 );

  if (rememberChoice) {
    mRememberCB = new QCheckBox( i18n("Remember choice"), page );
    topLayout->addWidget( mRememberCB );
    mRememberCB->setWhatsThis(
                    i18n("<qt><p>If you check this box your choice will "
                         "be stored and you will not be asked again."
                         "</p></qt>"));
  }

  initKeylist( keyList, keyIds );

  Q3ListViewItem *lvi;
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
    connect( mCheckSelectionTimer, SIGNAL( timeout() ),
             this,                 SLOT( slotCheckSelection() ) );
    connect( mListView, SIGNAL( selectionChanged() ),
             this,      SLOT( slotSelectionChanged() ) );
  }
  else {
    connect( mListView, SIGNAL( selectionChanged( Q3ListViewItem* ) ),
             this,      SLOT( slotSelectionChanged( Q3ListViewItem* ) ) );
  }
  connect( mListView, SIGNAL( doubleClicked ( Q3ListViewItem *, const QPoint &, int ) ), this, SLOT( accept() ) );

  connect( mListView, SIGNAL( contextMenuRequested( Q3ListViewItem*,
                                                    const QPoint&, int ) ),
           this,      SLOT( slotRMB( Q3ListViewItem*, const QPoint&, int ) ) );

  setButtonGuiItem( KDialog::Default, KGuiItem(i18n("&Reread Keys")) );
  connect( this, SIGNAL( defaultClicked() ),
           this, SLOT( slotRereadKeys() ) );
  connect(this, SIGNAL( okClicked()),SLOT(slotOk()));
  connect(this,SIGNAL( cancelClicked()),SLOT(slotCancel()));
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
  Q3ListViewItem* firstSelectedItem = 0;
  mKeyIds.clear();
  mListView->clear();

  // build a list of all public keys
  for( KeyListIterator it( keyList ); it.current(); ++it ) {
    KeyID curKeyId = (*it)->primaryKeyID();

    Q3ListViewItem* primaryUserID = new Q3ListViewItem( mListView, curKeyId,
                                                      (*it)->primaryUserID() );

    // select and open the given key
    if( keyIds.indexOf( curKeyId ) != -1 ) {
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

    Q3ListViewItem* childItem;

    childItem = new Q3ListViewItem( primaryUserID, "",
                                   i18n( "Fingerprint: %1" ,
                                     beautifyFingerprint( (*it)->primaryFingerprint() ) ) );
    if( primaryUserID->isSelected() && mListView->isMultiSelection() ) {
      mListView->setSelected( childItem, true );
    }

    childItem = new Q3ListViewItem( primaryUserID, "", keyInfo( *it ) );
    if( primaryUserID->isSelected() && mListView->isMultiSelection() ) {
      mListView->setSelected( childItem, true );
    }

    UserIDList userIDs = (*it)->userIDs();
    UserIDListIterator uidit( userIDs );
    if( *uidit ) {
      ++uidit; // skip the primary user ID
      for( ; *uidit; ++uidit ) {
        childItem = new Q3ListViewItem( primaryUserID, "", (*uidit)->text() );
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


QString KeySelectionDialog::keyInfo( const Kpgp::Key *key ) const
{
  QString status, remark;
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

  QDateTime dt;
  dt.setTime_t( key->creationDate() );
  if( remark.isEmpty() ) {
    return " " + i18nc("creation date and status of an OpenPGP key",
                      "Creation date: %1, Status: %2",
                       KGlobal::locale()->formatDate( dt.date(), KLocale::ShortDate ) ,
                       status );
  }
  else {
    return " " + i18nc("creation date, status and remark of an OpenPGP key",
                      "Creation date: %1, Status: %2 (%3)",
                       KGlobal::locale()->formatDate( dt.date(), KLocale::ShortDate ) ,
                       status ,
                       remark );
  }
}

QString KeySelectionDialog::beautifyFingerprint( const QByteArray& fpr ) const
{
  QByteArray result;

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
                                        Q3ListViewItem* lvi ) const
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
      kDebug( 5326 ) << "Deleting '" << lvi->firstChild()->text( 1 ) << "'\n";
      delete lvi->firstChild();
    }
    kDebug( 5326 ) << "Deleting key 0x" << lvi->text( 0 ) << " ("
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
KeySelectionDialog::keyAdmissibility( Q3ListViewItem* lvi,
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
    kDebug( 5326 ) << "Error: Invalid key status value.\n";
  }

  return 0;
}


KeyID
KeySelectionDialog::getKeyId( const Q3ListViewItem* lvi ) const
{
  KeyID keyId;

  if( 0 != lvi ) {
    if( 0 != lvi->parent() ) {
      keyId = lvi->parent()->text(0).toLocal8Bit();
    }
    else {
      keyId = lvi->text(0).toLocal8Bit();
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
    disconnect( mListView, SIGNAL( selectionChanged() ),
                this,      SLOT( slotSelectionChanged() ) );
  }
  else {
    disconnect( mListView, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
                this,      SLOT( slotSelectionChanged( Q3ListViewItem * ) ) );
  }

  initKeylist( keys, KeyIDList( mKeyIds ) );
  slotFilter();

  if( mListView->isMultiSelection() ) {
    connect( mListView, SIGNAL( selectionChanged() ),
             this,      SLOT( slotSelectionChanged() ) );
    slotSelectionChanged();
  }
  else {
    connect( mListView, SIGNAL( selectionChanged( Q3ListViewItem * ) ),
             this,      SLOT( slotSelectionChanged( Q3ListViewItem * ) ) );
  }

  // restore the saved position of the contents
  mListView->setContentsPos( 0, offsetY );
}


void KeySelectionDialog::slotSelectionChanged( Q3ListViewItem * lvi )
{
  slotCheckSelection( lvi );
}


void KeySelectionDialog::slotSelectionChanged()
{
  kDebug( 5326 ) << "KeySelectionDialog::slotSelectionChanged()\n";

  // (re)start the check selection timer. Checking the selection is delayed
  // because else drag-selection doesn't work very good (checking key trust
  // is slow).
  mCheckSelectionTimer->start( sCheckSelectionDelay );
}


void KeySelectionDialog::slotCheckSelection( Q3ListViewItem* plvi /* = 0 */ )
{
  kDebug( 5326 ) << "KeySelectionDialog::slotCheckSelection()\n";

  if( !mListView->isMultiSelection() ) {
    mKeyIds.clear();
    KeyID keyId = getKeyId( plvi );
    if( !keyId.isEmpty() ) {
      mKeyIds.append( keyId );
      enableButton( Ok, 1 == keyAdmissibility( plvi, AllowExpensiveTrustCheck ) );
    }
    else {
      enableButton( Ok, false );
    }
  }
  else {
    mCheckSelectionTimer->stop();

    // As we might change the selection, we have to disconnect the slot
    // to prevent recursion
    disconnect( mListView, SIGNAL( selectionChanged() ),
                this,      SLOT( slotSelectionChanged() ) );

    KeyIDList newKeyIdList;
    QList<Q3ListViewItem*> keysToBeChecked;

    bool keysAllowed = true;
    enum { UNKNOWN, SELECTED, DESELECTED } userAction = UNKNOWN;
    // Iterate over the tree to find selected keys.
    for( Q3ListViewItem *lvi = mListView->firstChild();
         0 != lvi;
         lvi = lvi->nextSibling() ) {
      // We make sure that either all items belonging to a key are selected
      // or unselected. As it's possible to select/deselect multiple keys at
      // once in extended selection mode we have to figure out whether the user
      // selected or deselected keys.

      // First count the selected items of this key
      int itemCount = 1 + lvi->childCount();
      int selectedCount = lvi->isSelected() ? 1 : 0;
      for( Q3ListViewItem *clvi = lvi->firstChild();
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
          if( -1 == mKeyIds.indexOf( lvi->text(0).toLocal8Bit() ) ) {
            // some items of this key are selected and the key wasn't selected
            // before => the user selected something
            kDebug( 5326 ) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User selected key "<<lvi->text(0)<<endl;
            userAction = SELECTED;
          }
          else if( ( itemCount > selectedCount ) &&
                   ( -1 != mKeyIds.indexOf( lvi->text(0).toLocal8Bit() ) ) ) {
            // some items of this key are unselected and the key was selected
            // before => the user deselected something
            kDebug( 5326 ) << "selectedCount: "<<selectedCount<<"/"<<itemCount
                          <<" --- User deselected key "<<lvi->text(0)<<endl;
            userAction = DESELECTED;
          }
        }
      }
      if( itemCount == selectedCount ) {
        // add key to the list of selected keys
        KeyID keyId = lvi->text(0).toLocal8Bit();
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
          for( Q3ListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, true );
          }
          // add key to the list of selected keys
          KeyID keyId = lvi->text(0).toLocal8Bit();
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
          for( Q3ListViewItem *clvi = lvi->firstChild();
               0 != clvi;
               clvi = clvi->nextSibling() ) {
            mListView->setSelected( clvi, false );
          }
        }
      }
    }
    kDebug( 5326 ) << "Selected keys: " << newKeyIdList.toStringList().join(", ") << endl;
    mKeyIds = newKeyIdList;
    if( !keysToBeChecked.isEmpty() ) {
      keysAllowed = keysAllowed && checkKeys( keysToBeChecked );
    }
    enableButton( Ok, keysAllowed );

    connect( mListView, SIGNAL( selectionChanged() ),
             this,      SLOT( slotSelectionChanged() ) );
  }
}


bool KeySelectionDialog::checkKeys( const QList<Q3ListViewItem*>& keys ) const
{
  KProgressDialog* pProgressDlg = 0;
  bool keysAllowed = true;
  kDebug( 5326 ) << "Checking keys...\n";

  pProgressDlg = new KProgressDialog( 0, i18n("Checking Keys"),
                                      i18n("Checking key 0xMMMMMMMM..."));
  pProgressDlg->setModal(true );
  pProgressDlg->setAllowCancel( false );
  pProgressDlg->progressBar()->setMaximum( keys.count() );
  pProgressDlg->setMinimumDuration( 1000 );
  pProgressDlg->show();

  for( QList<Q3ListViewItem*>::ConstIterator it = keys.begin();
       it != keys.end();
       ++it ) {
    kDebug( 5326 ) << "Checking key 0x" << getKeyId( *it ) << "...\n";
    pProgressDlg->setLabelText( i18n("Checking key 0x%1...",
                              QString::fromAscii( getKeyId( *it ) ) ) );
    qApp->processEvents();
    keysAllowed = keysAllowed && ( -1 != keyAdmissibility( *it, AllowExpensiveTrustCheck ) );
    pProgressDlg->progressBar()->setValue( pProgressDlg->progressBar()->value() + 1 );
    qApp->processEvents();
  }

  delete pProgressDlg;
  pProgressDlg = 0;

  return keysAllowed;
}


void KeySelectionDialog::slotRMB( Q3ListViewItem* lvi, const QPoint& pos, int )
{
  if( !lvi ) {
    return;
  }

  mCurrentContextMenuItem = lvi;

  QMenu menu(this);
  menu.addAction( i18n( "Recheck Key" ), this, SLOT( slotRecheckKey() ) );
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

void KeySelectionDialog::slotSearch( const QString & text )
{
  mSearchText = text.trimmed().toUpper();
  mStartSearchTimer->start( sCheckSelectionDelay );
}

void KeySelectionDialog::slotFilter()
{
  if ( mSearchText.isEmpty() ) {
    showAllItems();
    return;
  }

  // OK, so we need to filter:
  QRegExp keyIdRegExp( "(?:0x)?[A-F0-9]{1,8}", Qt::CaseInsensitive );
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

void KeySelectionDialog::filterByKeyID( const QString & keyID )
{
  assert( keyID.length() <= 8 );
  assert( !keyID.isEmpty() ); // regexp in slotFilter should prevent these
  if ( keyID.isEmpty() )
    showAllItems();
  else
    for ( Q3ListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
      item->setVisible( item->text( 0 ).toUpper().startsWith( keyID ) );
}

void KeySelectionDialog::filterByKeyIDOrUID( const QString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( "\\b" + QRegExp::escape( str ), Qt::CaseInsensitive );

  for ( Q3ListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( item->text( 0 ).toUpper().startsWith( str )
                      || rx.indexIn( item->text( 1 ) ) >= 0
                      || anyChildMatches( item, rx ) );

}

void KeySelectionDialog::filterByUID( const QString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( "\\b" + QRegExp::escape( str ), Qt::CaseInsensitive );

  for ( Q3ListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( rx.indexIn( item->text( 1 ) ) >= 0
                      || anyChildMatches( item, rx ) );
}


bool KeySelectionDialog::anyChildMatches( const Q3ListViewItem * item, QRegExp & rx ) const
{
  if ( !item )
    return false;

  Q3ListViewItem * stop = item->nextSibling(); // It's OK if stop is NULL...

  for ( Q3ListViewItemIterator it( item->firstChild() ) ; it.current() && it.current() != stop ; ++it )
    if ( rx.indexIn( it.current()->text( 1 ) ) >= 0 ) {
      //item->setOpen( true ); // do we want that?
      return true;
    }
  return false;
}

void KeySelectionDialog::showAllItems()
{
  for ( Q3ListViewItem * item = mListView->firstChild() ; item ; item = item->nextSibling() )
    item->setVisible( true );
}

// ------------------------------------------------------------------------
KeyRequester::KeyRequester( QWidget * parent, bool multipleKeys,
                            unsigned int allowedKeys, const char * name )
  : QWidget( parent ),
    mDialogCaption( i18n("OpenPGP Key Selection") ),
    mDialogMessage( i18n("Please select an OpenPGP key to use.") ),
    mMulti( multipleKeys ),
    mAllowedKeys( allowedKeys ),
    d( 0 )
{
  setObjectName( name );
  QHBoxLayout * hlay = new QHBoxLayout( this );
  hlay->setSpacing( KDialog::spacingHint() );
  hlay->setMargin( 0 );

  // the label where the key id is to be displayed:
  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );

  // the button to unset any key:
  mEraseButton = new QPushButton( this );
  mEraseButton->setAutoDefault( false );
  mEraseButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                            QSizePolicy::Minimum ) );
  mEraseButton->setIcon( KIcon( "clear-left" ) );
  mEraseButton->setToolTip( i18n("Clear") );

  // the button to call the KeySelectionDialog:
  mDialogButton = new QPushButton( i18n("Change..."), this );
  mDialogButton->setAutoDefault( false );

  hlay->addWidget( mLabel, 1 );
  hlay->addWidget( mEraseButton );
  hlay->addWidget( mDialogButton );

  connect( mEraseButton, SIGNAL(clicked()), SLOT(slotEraseButtonClicked()) );
  connect( mDialogButton, SIGNAL(clicked()), SLOT(slotDialogButtonClicked()) );

  setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed ) );
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

  QString s = mKeys.toStringList().join(", ");

  mLabel->setText( s );
  mLabel->setToolTip( s );
}

void KeyRequester::slotDialogButtonClicked() {
  Module * pgp = Module::getKpgp();

  if ( !pgp ) {
    kWarning(5326) << "Kpgp::KeyRequester::slotDialogButtonClicked(): No pgp module found!" << endl;
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

void KeyRequester::setDialogCaption( const QString & caption ) {
  mDialogCaption = caption;
}

void KeyRequester::setDialogMessage( const QString & msg ) {
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


PublicKeyRequester::PublicKeyRequester( QWidget * parent, bool multi,
					unsigned int allowed, const char * name )
  : KeyRequester( parent, multi, allowed & ~SecretKeys, name )
{

}

PublicKeyRequester::~PublicKeyRequester() {

}

KeyIDList PublicKeyRequester::keyRequestHook( Module * pgp ) const {
  assert( pgp );
  return pgp->selectPublicKeys( mDialogCaption, mDialogMessage, mKeys, QString(), mAllowedKeys );
}

SecretKeyRequester::SecretKeyRequester( QWidget * parent, bool multi,
					unsigned int allowed, const char * name )
  : KeyRequester( parent, multi, allowed & ~PublicKeys, name )
{

}

SecretKeyRequester::~SecretKeyRequester() {

}

KeyIDList SecretKeyRequester::keyRequestHook( Module * pgp ) const {
  assert( pgp );

  if(mKeys.isEmpty())
     return KeyIDList();

  KeyID keyID = mKeys.first();
  keyID = pgp->selectSecretKey( mDialogCaption, mDialogMessage, keyID );

  return KeyIDList() << keyID;
}



// ------------------------------------------------------------------------
KeyApprovalDialog::KeyApprovalDialog( const QStringList& addresses,
                                      const QVector<KeyIDList>& keyIDs,
                                      const int allowedKeys,
                                      QWidget *parent )
  : KDialog( parent ),
    mKeys( keyIDs ),
    mAllowedKeys( allowedKeys ),
    mPrefsChanged( false )
{
  setCaption( i18n("Encryption Key Approval") );
  setButtons( Ok|Cancel );

  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  if( pgp == 0 )
    return;

  // ##### error handling
  // if( addresses.isEmpty() || keyList.isEmpty() ||
  //     addresses.count()+1 != keyList.count() )
  //   do something;

  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QLabel *label = new QLabel( i18n("The following keys will be used for "
                                   "encryption:"),
                              page );
  topLayout->addWidget( label );

  Q3ScrollView* sv = new Q3ScrollView( page );
  sv->setResizePolicy( Q3ScrollView::AutoOneFit );
  topLayout->addWidget( sv );
  KVBox* bigvbox = new KVBox( sv->viewport() );
  //bigvbox->setMargin( KDialog::marginHint() );
  bigvbox->setSpacing( KDialog::spacingHint() );
  sv->addChild( bigvbox );

  QButtonGroup *mChangeButtonGroup = new QButtonGroup;
  mAddressLabels.resize( addresses.count() );
  mKeyIdsLabels.resize( keyIDs.size() );
  //mKeyIdListBoxes.resize( keyIDs.size() );
  mEncrPrefCombos.resize( addresses.count() );

  // the sender's key
  if( pgp->encryptToSelf() ) {
    mEncryptToSelf = 1;
    KHBox* hbox = new KHBox( bigvbox );
    new QLabel( i18n("Your keys:"), hbox );
    QLabel* keyidsL = new QLabel( hbox );
    if( keyIDs[0].isEmpty() ) {
      keyidsL->setText( i18nc("<none> means 'no key'", "<none>") );
    }
    else {
      keyidsL->setText( "0x" + keyIDs[0].toStringList().join( "\n0x" ) );
    }
    keyidsL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    /*
    QListBox* keyidLB = new QListBox( hbox );
    if( keyIDs[0].isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( keyIDs[0].toStringList() );
    }
    keyidLB->setSelectionMode( QListBox::NoSelection );
    keyidLB->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    */
    QPushButton *button = new QPushButton( i18n("Change..."), hbox );
    mChangeButtonGroup->addButton( button );
    button->setAutoDefault( false );
    hbox->setStretchFactor( keyidsL, 10 );
    mKeyIdsLabels.insert( 0, keyidsL );
    //hbox->setStretchFactor( keyidLB, 10 );
    //mKeyIdListBoxes.insert( 0, keyidLB );

    new KSeparator( Qt::Horizontal, bigvbox );
  }
  else {
    mEncryptToSelf = 0;
    // insert dummy KeyIdListBox
    mKeyIdsLabels.insert( 0, 0 );
    //mKeyIdListBoxes.insert( 0, 0 );
  }

  QStringList::ConstIterator ait;
  QVector<KeyIDList>::const_iterator kit;
  int i;
  for( ait = addresses.begin(), kit = keyIDs.begin(), i = 0;
       ( ait != addresses.end() ) && ( kit != keyIDs.end() );
       ++ait, ++kit, ++i ) {
    if( i == 0 ) {
      ++kit; // skip the sender's key id
    }
    else {
      new KSeparator( Qt::Horizontal, bigvbox );
    }

    KHBox *hbox = new KHBox( bigvbox );
    new QLabel( i18n("Recipient:"), hbox );
    QLabel *addressL = new QLabel( *ait, hbox );
    hbox->setStretchFactor( addressL, 10 );
    mAddressLabels.insert( i, addressL  );

    hbox = new KHBox( bigvbox );
    new QLabel( i18n("Encryption keys:"), hbox );
    QLabel* keyidsL = new QLabel( hbox );
    if( (*kit).isEmpty() ) {
      keyidsL->setText( i18nc("<none> means 'no key'", "<none>") );
    }
    else {
      keyidsL->setText( "0x" + (*kit).toStringList().join( "\n0x" ) );
    }
    keyidsL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    /*
    QListBox* keyidLB = new QListBox( hbox );
    if( (*kit).isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( (*kit).toStringList() );
    }
    keyidLB->setSelectionMode( QListBox::NoSelection );
    keyidLB->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    */
    QPushButton *button = new QPushButton( i18n("Change..."), hbox );
    mChangeButtonGroup->addButton( button );
    button->setAutoDefault( false );
    hbox->setStretchFactor( keyidsL, 10 );
    mKeyIdsLabels.insert( i + 1, keyidsL );
    //hbox->setStretchFactor( keyidLB, 10 );
    //mKeyIdListBoxes.insert( i + 1, keyidLB );

    hbox = new KHBox( bigvbox );
    new QLabel( i18n("Encryption preference:"), hbox );
    QComboBox *encrPrefCombo = new QComboBox( hbox );
    encrPrefCombo->addItem( i18n("<none>") );
    encrPrefCombo->addItem( i18n("Never Encrypt with This Key") );
    encrPrefCombo->addItem( i18n("Always Encrypt with This Key") );
    encrPrefCombo->addItem( i18n("Encrypt Whenever Encryption is Possible") );
    encrPrefCombo->addItem( i18n("Always Ask") );
    encrPrefCombo->addItem( i18n("Ask Whenever Encryption is Possible") );

    EncryptPref encrPref = pgp->encryptionPreference( *ait );
    switch( encrPref ) {
      case NeverEncrypt:
        encrPrefCombo->setCurrentIndex( 1 );
        break;
      case AlwaysEncrypt:
        encrPrefCombo->setCurrentIndex( 2 );
        break;
      case AlwaysEncryptIfPossible:
        encrPrefCombo->setCurrentIndex( 3 );
        break;
      case AlwaysAskForEncryption:
        encrPrefCombo->setCurrentIndex( 4 );
        break;
      case AskWheneverPossible:
        encrPrefCombo->setCurrentIndex( 5 );
        break;
      default:
        encrPrefCombo->setCurrentIndex( 0 );
    }
    connect( encrPrefCombo, SIGNAL(activated(int)),
             this, SLOT(slotPrefsChanged(int)) );
    mEncrPrefCombos.insert( i, encrPrefCombo );
  }
  connect( mChangeButtonGroup, SIGNAL(buttonClicked(int)),
           this, SLOT(slotChangeEncryptionKey(int)) );

  QSize size = sizeHint();

  // don't make the dialog too large
  QRect desk = KGlobalSettings::desktopGeometry(this);
  int screenWidth = desk.width();
  if( size.width() > 3*screenWidth/4 )
    size.setWidth( 3*screenWidth/4 );
  int screenHeight = desk.height();
  if( size.height() > 7*screenHeight/8 )
    size.setHeight( 7*screenHeight/8 );
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
  connect(this,SIGNAL(cancelClicked()),SLOT(slotCancel()));
  setInitialSize( size );
}

void
KeyApprovalDialog::slotChangeEncryptionKey( int nr )
{
  Kpgp::Module *pgp = Kpgp::Module::getKpgp();

  kDebug( 5326 )<<"Key approval dialog size is "
               <<width()<<"x"<<height()<<endl;

  if( pgp == 0 )
    return;

  if( !mEncryptToSelf )
    nr++;
  KeyIDList keyIds = mKeys[nr];
  if( nr == 0 ) {
    keyIds = pgp->selectPublicKeys( i18n("Encryption Key Selection"),
                                    i18nc("if in your language something like "
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
                                    i18nc("if in your language something like "
                                         "'key(s)' isn't possible please "
                                         "use the plural in the translation",
                                         "Select the key(s) which should "
                                         "be used to encrypt the message "
                                         "for\n%1",
                                      mAddressLabels[nr-1]->text() ),
                                    keyIds,
                                    mAddressLabels[nr-1]->text(),
                                    mAllowedKeys );
  }
  if( !keyIds.isEmpty() ) {
    mKeys[nr] = keyIds;
    QLabel* keyidsL = mKeyIdsLabels[nr];
    keyidsL->setText( "0x" + keyIds.toStringList().join( "\n0x" ) );
    /*
    QListBox* qlb = mKeyIdListBoxes[nr];
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
      switch( mEncrPrefCombos[i]->currentIndex() ) {
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
CipherTextDialog::CipherTextDialog( const QByteArray & text,
                                    const QByteArray & charset, QWidget *parent )
  :KDialog( parent )
{
  setCaption( i18n("OpenPGP Information") );
  setButtons( Ok|Cancel );

  // FIXME (post KDE2.2): show some more info, e.g. the output of GnuPG/PGP
  QFrame *page = new QFrame( this );
  setMainWidget( page );
  QVBoxLayout *topLayout = new QVBoxLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );

  QLabel *label = new QLabel( page );
  label->setText(i18n("Result of the last encryption/sign operation:"));
  topLayout->addWidget( label );

  mEditBox = new Q3MultiLineEdit( page );
  mEditBox->setReadOnly(true);
  topLayout->addWidget( mEditBox, 10 );

  QString unicodeText;
  if (charset.isEmpty())
    unicodeText = QString::fromLocal8Bit(text.data());
  else {
    bool ok=true;
    QTextCodec *codec = KGlobal::charsets()->codecForName(charset, ok);
    if(!ok)
      unicodeText = QString::fromLocal8Bit(text.data());
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


  int maxWidth = KGlobalSettings::desktopGeometry(parentWidget()).width()-100;

  mEditBox->setMinimumWidth( qMin( textWidth, maxWidth ) );
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
