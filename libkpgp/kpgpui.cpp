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

#include "kpgpui.h"
#include "kpgp.h"
#include "kpgpkey.h"
#include "kpgp_debug.h"

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
#include <QButtonGroup>
#include <QTextEdit>
#include <QGroupBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QScrollArea>
#include <QScrollBar>
#include <QAbstractTextDocumentLayout>

#include <kvbox.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kpassworddialog.h>
#include <kcharsets.h>
#include <kseparator.h>
#include <kiconloader.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <kprogressdialog.h>
#include <kwindowsystem.h>
#include <kpushbutton.h>
#include <kglobalsettings.h>
#include <klineedit.h>
#include <KGlobal>
#include <QIcon>

#include <assert.h>
#include <string.h> // for memcpy(3)
#include <KCharsets>
#include <KLocale>
#include <QDesktopWidget>

#ifndef QT_NO_TREEWIDGET
const int Kpgp::KeySelectionDialog::sCheckSelectionDelay = 250;
#endif

namespace Kpgp {

PassphraseDialog::PassphraseDialog( QWidget *parent,
                                    const QString &caption,
                                    const QString &keyID )
  :KPasswordDialog( parent )
{
  setWindowTitle( caption );
  //QT5
  //setButtons( Ok|Cancel );

  setPixmap( BarIcon(QLatin1String("dialog-password")) );

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
  connect( toolCombo, SIGNAL(activated(int)),
           this, SIGNAL(changed()) );
  // This is the place to add a KUrlRequester to be used for asking
  // the user for the path to the executable...
  topLayout->addWidget( group );

  mpOptionsGroupBox = new QGroupBox( i18n("Options"), this );
  lay = new QVBoxLayout( mpOptionsGroupBox );
  lay->setSpacing( KDialog::spacingHint() );
  storePass = new QCheckBox( i18n("&Keep passphrase in memory"),
                             mpOptionsGroupBox );
  lay->addWidget( storePass );
  connect( storePass, SIGNAL(toggled(bool)),
           this, SIGNAL(changed()) );
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
   connect( encToSelf, SIGNAL(toggled(bool)),
           this, SIGNAL(changed()) );

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
  connect( showCipherText, SIGNAL(toggled(bool)),
           this, SIGNAL(changed()) );

  msg = i18n( "<qt><p>When this option is enabled, the signed/encrypted text "
              "will be shown in a separate window, enabling you to know how "
              "it will look before it is sent. This is a good idea when "
              "you are verifying that your encryption system works.</p></qt>" );
  showCipherText->setWhatsThis( msg );
  if( encrypt ) {
    showKeyApprovalDlg = new QCheckBox( i18n("Always show the encryption "
                                             "keys &for approval"));
    lay->addWidget( showKeyApprovalDlg );
    connect( showKeyApprovalDlg, SIGNAL(toggled(bool)),
           this, SIGNAL(changed()) );
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

#ifndef QT_NO_TREEWIDGET


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
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Desktop ),
                                                        IconSize( KIconLoader::Desktop ) ),
                             qApp->windowIcon().pixmap( IconSize( KIconLoader::Small ),
                                                        IconSize( KIconLoader::Small ) ) );
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
  mKeyGoodPix    = new QPixmap( UserIcon(QLatin1String("key_ok")) );
  mKeyBadPix     = new QPixmap( UserIcon(QLatin1String("key_bad")) );
  mKeyUnknownPix = new QPixmap( UserIcon(QLatin1String("key_unknown")) );
  mKeyValidPix   = new QPixmap( UserIcon(QLatin1String("key")) );

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

  connect( le, SIGNAL(textChanged(QString)),
           this, SLOT(slotSearch(QString)) );
  connect( mStartSearchTimer, SIGNAL(timeout()), SLOT(slotFilter()) );

  mListView = new QTreeWidget( page );
  mListView->setHeaderLabels( QStringList()
    << i18n("Key ID")
    << i18n("User ID") );
  mListView->setAllColumnsShowFocus( true );
  mListView->header()->setStretchLastSection( true );
  mListView->setRootIsDecorated( true );
  mListView->setSortingEnabled( true );
  mListView->header()->setSortIndicatorShown( true );
  mListView->sortItems( 1, Qt::AscendingOrder ); // sort by User ID
//  mListView->setShowToolTips( true );
  if( extendedSelection ) {
    mListView->setSelectionMode( QTreeWidget::ExtendedSelection );
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

  QTreeWidgetItem *lvi = 0;
  if( extendedSelection ) {
    lvi = mListView->currentItem();
    slotCheckSelection();
  }
  else {
    if ( mListView->selectedItems().size() > 0 )
      lvi = mListView->selectedItems().first();
    slotCheckSelection( lvi );
  }
  if( lvi != 0 )
    mListView->scrollToItem( lvi );

  if( extendedSelection ) {
    connect( mCheckSelectionTimer, SIGNAL(timeout()),
             this,                 SLOT(slotCheckSelection()) );
    connect( mListView, SIGNAL(itemSelectionChanged()),
             this,      SLOT(slotSelectionChanged()) );
  }
  else {
    connect( mListView, SIGNAL(itemSelectionChanged()),
             this,      SLOT(slotSelectionChanged()) );
  }
  connect( mListView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(accept()) );

  mListView->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( mListView, SIGNAL(customContextMenuRequested(QPoint)),
           this,      SLOT(slotRMB(QPoint)) );

  setButtonGuiItem( KDialog::Default, KGuiItem(i18n("&Reread Keys")) );
  connect( this, SIGNAL(defaultClicked()),
           this, SLOT(slotRereadKeys()) );
  connect(this, SIGNAL(okClicked()),SLOT(slotOk()));
  connect(this,SIGNAL(cancelClicked()),SLOT(slotCancel()));
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
  if( mListView->selectionMode() == QTreeWidget::ExtendedSelection || mKeyIds.isEmpty() )
    return KeyID();
  else
    return mKeyIds.first();
}


void KeySelectionDialog::initKeylist( const KeyList& keyList,
                                      const KeyIDList& keyIds )
{
  QTreeWidgetItem* firstSelectedItem = 0;
  mKeyIds.clear();
  mListView->clear();

  // build a list of all public keys
  foreach ( Key* key, keyList ) {
    KeyID curKeyId = key->primaryKeyID();

    QTreeWidgetItem* primaryUserID = new QTreeWidgetItem( mListView );
    primaryUserID->setText( 0, QLatin1String(curKeyId) );
    primaryUserID->setText( 1, key->primaryUserID() );

    // select and open the given key
    if( keyIds.indexOf( curKeyId ) != -1 ) {
      if( 0 == firstSelectedItem ) {
        firstSelectedItem = primaryUserID;
      }
      primaryUserID->setSelected( true );
      mKeyIds.append( curKeyId );
    }
    primaryUserID->setExpanded( false );

    // set icon for this key
    switch( keyValidity( key ) ) {
      case 0: // the key's validity can't be determined
        primaryUserID->setData( 0, Qt::DecorationRole, *mKeyUnknownPix );
        break;
      case 1: // key is valid but not trusted
        primaryUserID->setData( 0, Qt::DecorationRole, *mKeyValidPix );
        break;
      case 2: // key is valid and trusted
        primaryUserID->setData( 0, Qt::DecorationRole, *mKeyGoodPix );
        break;
      case -1: // key is invalid
        primaryUserID->setData( 0, Qt::DecorationRole, *mKeyBadPix );
        break;
    }

    QTreeWidgetItem* childItem;

    childItem = new QTreeWidgetItem( primaryUserID );
    childItem->setText( 1, i18n( "Fingerprint: %1" , beautifyFingerprint( key->primaryFingerprint() ) ) );
    if( primaryUserID->isSelected() && mListView->selectionMode() == QTreeWidget::ExtendedSelection ) {
      childItem->setSelected( true );
    }

    childItem = new QTreeWidgetItem( primaryUserID );
    childItem->setText( 1, keyInfo( key ) );
    if( primaryUserID->isSelected() && mListView->selectionMode() == QTreeWidget::ExtendedSelection ) {
      childItem->setSelected( true );
    }

    UserIDList userIDs = key->userIDs();
    UserIDList::Iterator uidit( userIDs.begin() );
    if( uidit != userIDs.end() ) {
      ++uidit; // skip the primary user ID
      for( ; uidit != userIDs.end(); ++uidit ) {
        childItem = new QTreeWidgetItem( primaryUserID );
        childItem->setText( 1, (*uidit)->text() );
        if( primaryUserID->isSelected() && mListView->selectionMode() == QTreeWidget::ExtendedSelection ) {
          childItem->setSelected( true );
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
    return QLatin1Char(' ') + i18nc("creation date and status of an OpenPGP key",
                      "Creation date: %1, Status: %2",
                       KLocale::global()->formatDate( dt.date(), KLocale::ShortDate ) ,
                       status );
  }
  else {
    return QLatin1Char(' ') + i18nc("creation date, status and remark of an OpenPGP key",
                      "Creation date: %1, Status: %2 (%3)",
                       KLocale::global()->formatDate( dt.date(), KLocale::ShortDate ) ,
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

  return QLatin1String(result);
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
                                        QTreeWidgetItem* lvi ) const
{
  if( 0 == lvi ) {
    return;
  }

  if( lvi->parent() != 0 ) {
    lvi = lvi->parent();
  }

  if( 0 == key ) {
    // the key doesn't exist anymore -> delete it from the list view
    while( lvi->childCount() ) {
      qCDebug(KPGP_LOG) <<"Deleting '" << lvi->child( 0 )->text( 1 ) <<"'";
      delete lvi->takeChild( 0 );
    }
    qCDebug(KPGP_LOG) <<"Deleting key 0x" << lvi->text( 0 ) <<" ("
                  << lvi->text( 1 ) << ")\n";
    delete lvi;
    lvi = 0;
    return;
  }

  // update the icon for this key
  switch( keyValidity( key ) ) {
  case 0: // the key's validity can't be determined
    lvi->setData( 0, Qt::DecorationRole, *mKeyUnknownPix );
    break;
  case 1: // key is valid but not trusted
    lvi->setData( 0, Qt::DecorationRole, *mKeyValidPix );
    break;
  case 2: // key is valid and trusted
    lvi->setData( 0, Qt::DecorationRole, *mKeyGoodPix );
    break;
  case -1: // key is invalid
    lvi->setData( 0, Qt::DecorationRole, *mKeyBadPix );
    break;
  }

  // update the key info for this key
  // the key info is identified by a leading space; this shouldn't be
  // a problem because User Ids shouldn't start with a space
  QTreeWidgetItemIterator it( lvi );
  while ( *it ) {
    if( lvi->text( 1 ).at(0) == QLatin1Char(' ') ) {
      lvi->setText( 1, keyInfo( key ) );
      break;
    }
    ++it;
  }
}


int
KeySelectionDialog::keyAdmissibility( QTreeWidgetItem* lvi,
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
    qCDebug(KPGP_LOG) <<"Error: Invalid key status value.";
  }

  return 0;
}


KeyID
KeySelectionDialog::getKeyId( const QTreeWidgetItem* lvi ) const
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
  int offsetY = mListView->verticalScrollBar()->value();

  disconnect( mListView, SIGNAL(itemSelectionChanged()),
              this,      SLOT(slotSelectionChanged()) );

  initKeylist( keys, KeyIDList( mKeyIds ) );
  slotFilter();

  connect( mListView, SIGNAL(itemSelectionChanged()),
           this,      SLOT(slotSelectionChanged()) );
  slotSelectionChanged();

  // restore the saved position of the contents
  mListView->verticalScrollBar()->setValue( offsetY );
}


void KeySelectionDialog::slotSelectionChanged()
{
  qCDebug(KPGP_LOG) <<"KeySelectionDialog::slotSelectionChanged()";

  if ( mListView->selectionMode() == QTreeWidget::ExtendedSelection ) {
    // (re)start the check selection timer. Checking the selection is delayed
    // because else drag-selection doesn't work very good (checking key trust
    // is slow).
    mCheckSelectionTimer->start( sCheckSelectionDelay );
  } else {
    if ( mListView->selectedItems().size() > 0 )
      slotCheckSelection( mListView->selectedItems().first() );
  }
}


void KeySelectionDialog::slotCheckSelection( QTreeWidgetItem* plvi /* = 0 */ )
{
  qCDebug(KPGP_LOG) <<"KeySelectionDialog::slotCheckSelection()";

  if( mListView->selectionMode() != QTreeWidget::ExtendedSelection ) {
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
    disconnect( mListView, SIGNAL(itemSelectionChanged()),
                this,      SLOT(slotSelectionChanged()) );

    KeyIDList newKeyIdList;
    QList<QTreeWidgetItem*> keysToBeChecked;

    bool keysAllowed = true;
    enum { UNKNOWN, SELECTED, DESELECTED } userAction = UNKNOWN;
    // Iterate over the tree to find selected keys.
    for( int lviIndex = 0; lviIndex < mListView->topLevelItemCount(); ++lviIndex ) {
      QTreeWidgetItem *lvi = mListView->topLevelItem( lviIndex );
      // We make sure that either all items belonging to a key are selected
      // or unselected. As it's possible to select/deselect multiple keys at
      // once in extended selection mode we have to figure out whether the user
      // selected or deselected keys.

      // First count the selected items of this key
      int itemCount = 1 + lvi->childCount();
      int selectedCount = lvi->isSelected() ? 1 : 0;
      for( int clviIndex = 0; clviIndex < lvi->childCount(); ++clviIndex ) {
        QTreeWidgetItem *clvi = lvi->child( clviIndex );
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
            qCDebug(KPGP_LOG) <<"selectedCount:"<<selectedCount<<"/"<<itemCount
                          <<"--- User selected key"<<lvi->text(0);
            userAction = SELECTED;
          }
          else if( ( itemCount > selectedCount ) &&
                   ( -1 != mKeyIds.indexOf( lvi->text(0).toLocal8Bit() ) ) ) {
            // some items of this key are unselected and the key was selected
            // before => the user deselected something
            qCDebug(KPGP_LOG) <<"selectedCount:"<<selectedCount<<"/"<<itemCount
                          <<"--- User deselected key"<<lvi->text(0);
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
          lvi->setSelected( true );
          for( int clviIndex = 0; clviIndex < lvi->childCount(); ++clviIndex ) {
            QTreeWidgetItem *clvi = lvi->child( clviIndex );
            clvi->setSelected( true );
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
          lvi->setSelected( false );
          for ( int clviIndex = 0; clviIndex < lvi->childCount(); ++clviIndex ) {
            QTreeWidgetItem *clvi = lvi->child( clviIndex );
            clvi->setSelected( false );
          }
        }
      }
    }
    qCDebug(KPGP_LOG) <<"Selected keys:" << newKeyIdList.toStringList().join(QLatin1String(","));
    mKeyIds = newKeyIdList;
    if( !keysToBeChecked.isEmpty() ) {
      keysAllowed = keysAllowed && checkKeys( keysToBeChecked );
    }
    enableButton( Ok, keysAllowed );

    connect( mListView, SIGNAL(selectionChanged()),
             this,      SLOT(slotSelectionChanged()) );
  }
}


bool KeySelectionDialog::checkKeys( const QList<QTreeWidgetItem*>& keys ) const
{
  KProgressDialog* pProgressDlg = 0;
  bool keysAllowed = true;
  qCDebug(KPGP_LOG) <<"Checking keys...";

  pProgressDlg = new KProgressDialog( 0, i18n("Checking Keys"),
                                      i18n("Checking key 0xMMMMMMMM..."));
  pProgressDlg->setModal(true );
  pProgressDlg->setAllowCancel( false );
  pProgressDlg->progressBar()->setMaximum( keys.count() );
  pProgressDlg->setMinimumDuration( 1000 );
  pProgressDlg->show();

  for( QList<QTreeWidgetItem*>::ConstIterator it = keys.begin();
       it != keys.end();
       ++it ) {
    qCDebug(KPGP_LOG) <<"Checking key 0x" << getKeyId( *it ) <<"...";
    pProgressDlg->setLabelText( i18n("Checking key 0x%1...",
                              QString::fromLatin1( getKeyId( *it ) ) ) );
    qApp->processEvents();
    keysAllowed = keysAllowed && ( -1 != keyAdmissibility( *it, AllowExpensiveTrustCheck ) );
    pProgressDlg->progressBar()->setValue( pProgressDlg->progressBar()->value() + 1 );
    qApp->processEvents();
  }

  delete pProgressDlg;
  pProgressDlg = 0;

  return keysAllowed;
}


void KeySelectionDialog::slotRMB( const QPoint& pos )
{
  QTreeWidgetItem *lvi = mListView->itemAt( pos );
  if( !lvi ) {
    return;
  }

  mCurrentContextMenuItem = lvi;

  QMenu menu(this);
  menu.addAction( i18n( "Recheck Key" ), this, SLOT(slotRecheckKey()) );
  menu.exec( mListView->viewport()->mapToGlobal( pos ) );
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
  QRegExp keyIdRegExp( QLatin1String("(?:0x)?[A-F0-9]{1,8}"), Qt::CaseInsensitive );
  if ( keyIdRegExp.exactMatch( mSearchText ) ) {
    if ( mSearchText.startsWith( QLatin1String("0X") ) )
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
  else {
    for ( int i = 0; i < mListView->topLevelItemCount(); ++i ) {
      QTreeWidgetItem * item = mListView->topLevelItem( i );
      item->setHidden( !item->text( 0 ).toUpper().startsWith( keyID ) );
    }
  }
}

void KeySelectionDialog::filterByKeyIDOrUID( const QString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( QLatin1String("\\b") + QRegExp::escape( str ), Qt::CaseInsensitive );

  for ( int i = 0; i < mListView->topLevelItemCount(); ++i ) {
    QTreeWidgetItem * item = mListView->topLevelItem( i );
    item->setHidden( !item->text( 0 ).toUpper().startsWith( str )
                     && rx.indexIn( item->text( 1 ) ) < 0
                     && !anyChildMatches( item, rx ) );
  }
}

void KeySelectionDialog::filterByUID( const QString & str )
{
  assert( !str.isEmpty() );

  // match beginnings of words:
  QRegExp rx( QLatin1String("\\b") + QRegExp::escape( str ), Qt::CaseInsensitive );

  for ( int i = 0; i < mListView->topLevelItemCount(); ++i ) {
    QTreeWidgetItem * item = mListView->topLevelItem( i );
    item->setHidden( rx.indexIn( item->text( 1 ) ) < 0
                     && !anyChildMatches( item, rx ) );
  }
}


bool KeySelectionDialog::anyChildMatches( const QTreeWidgetItem * item, QRegExp & rx ) const
{
  if ( !item )
    return false;

  for ( int i = 0; i < item->childCount(); ++i ) {
    QTreeWidgetItem* it = item->child( i );
    if ( rx.indexIn( it->text( 1 ) ) >= 0 ) {
      //item->setOpen( true ); // do we want that?
      return true;
    }
  }

  return false;
}

void KeySelectionDialog::showAllItems()
{
  for ( QTreeWidgetItemIterator it( mListView ); *it; ++it )
    (*it)->setHidden( false );
}

#endif // QT_NO_TREEWIDGET

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
  setObjectName( QLatin1String(name) );
  QHBoxLayout * hlay = new QHBoxLayout( this );
  hlay->setSpacing( KDialog::spacingHint() );
  hlay->setMargin( 0 );

  // the label where the key id is to be displayed:
  mLabel = new QLabel( this );
  mLabel->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );

  // the button to unset any key:
  mEraseButton = new QPushButton( this );
  mEraseButton->setAutoDefault( false );
  mEraseButton->setSizePolicy( QSizePolicy( QSizePolicy::Minimum,
                                            QSizePolicy::Minimum ) );
  mEraseButton->setIcon( QIcon::fromTheme( QLatin1String("edit-clear-locationbar-rtl") ) );
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

  QString s = mKeys.toStringList().join(QLatin1String(", "));

  mLabel->setText( s );
  mLabel->setToolTip( s );
}

void KeyRequester::slotDialogButtonClicked() {
  Module * pgp = Module::getKpgp();

  if ( !pgp ) {
    qCWarning(KPGP_LOG) <<"Kpgp::KeyRequester::slotDialogButtonClicked(): No pgp module found!";
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

  QScrollArea* sv = new QScrollArea( page );
  sv->setWidgetResizable( true );
  topLayout->addWidget( sv );
  KVBox* bigvbox = new KVBox;
  sv->setWidget( bigvbox );
  //bigvbox->setMargin( KDialog::marginHint() );
  bigvbox->setSpacing( KDialog::spacingHint() );

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
      keyidsL->setText( i18nc( "@info", "<placeholder>none</placeholder> means 'no key'" ) );
    }
    else {
      keyidsL->setText( QLatin1String("0x") + keyIDs[0].toStringList().join( QLatin1String("\n0x") ) );
    }
    keyidsL->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    /*
    QListBox* keyidLB = new QListBox( hbox );
    if( keyIDs[0].isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( keyIDs[0].toStringList() );
    }
    keyidLB->setSelectionMode( QListBox::NoSelection );
    keyidLB->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
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
      keyidsL->setText( i18nc( "@info", "<placeholder>none</placeholder> means 'no key'" ) );
    }
    else {
      keyidsL->setText( QLatin1String("0x") + (*kit).toStringList().join( QLatin1String("\n0x") ) );
    }
    keyidsL->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    /*
    QListBox* keyidLB = new QListBox( hbox );
    if( (*kit).isEmpty() ) {
      keyidLB->insertItem( i18n("<none>") );
    }
    else {
      keyidLB->insertStringList( (*kit).toStringList() );
    }
    keyidLB->setSelectionMode( QListBox::NoSelection );
    keyidLB->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
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
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "<placeholder>none</placeholder>") );
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "Never Encrypt with This Key") );
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "Always Encrypt with This Key") );
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "Encrypt Whenever Encryption is Possible") );
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "Always Ask") );
    encrPrefCombo->addItem( i18nc( "@item:inlistbox", "Ask Whenever Encryption is Possible") );

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
  QRect desk = QApplication::desktop()->screenGeometry(this);
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

  qCDebug(KPGP_LOG)<<"Key approval dialog size is"
               <<width()<<"x"<<height();

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
                                    QLatin1String(""),
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
    keyidsL->setText( QLatin1String("0x") + keyIds.toStringList().join( QLatin1String("\n0x") ) );
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
    for( int i = 0; i < mAddressLabels.size(); ++i ) {
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

  mEditBox = new QTextEdit( page );
  mEditBox->setReadOnly(true);
  topLayout->addWidget( mEditBox, 10 );

  QString unicodeText;
  if (charset.isEmpty())
    unicodeText = QString::fromLocal8Bit(text.data());
  else {
    bool ok=true;
    QTextCodec *codec = KCharsets::charsets()->codecForName(QLatin1String(charset), ok);
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
  (void) mEditBox->document()->documentLayout()->documentSize();

  mEditBox->setMinimumHeight( mEditBox->fontMetrics().lineSpacing() * 25 );

  int textWidth = mEditBox->viewport()->width() + 30;


  int maxWidth = QApplication::desktop()->screenGeometry(parentWidget()).width()-100;

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



