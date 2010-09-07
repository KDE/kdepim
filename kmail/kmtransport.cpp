/**
 * kmtransport.cpp
 *
 * Copyright (c) 2001-2002 Michael Haeckel <haeckel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2
 *  as published by the Free Software Foundation
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>
#include <assert.h>

#include "kmtransport.h"

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <klineedit.h>
#include <tqradiobutton.h>
#include <tqtabwidget.h>
#include <tqvalidator.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kdebug.h>
#include <kwallet.h>
using KWallet::Wallet;
#include <kprotocolinfo.h>

#include "kmkernel.h"
#include "kmservertest.h"
#include "kmaccount.h"
#include "protocols.h"
#include "transportmanager.h"
using namespace KMail;

KMTransportInfo::KMTransportInfo() : mPasswdDirty( false ),
  mStorePasswd( false ), mStorePasswdInConfig( false ), mId( 0 )
{
  name = i18n("Unnamed");
  port = "25";
  auth = false;
  specifyHostname = false;
}


KMTransportInfo::~KMTransportInfo()
{
}


void KMTransportInfo::readConfig(int id)
{
  KConfig *config = KMKernel::config();
  KConfigGroupSaver saver(config, "Transport " + TQString::number(id));
  mId = config->readUnsignedNumEntry( "id", 0 );
  type = config->readEntry("type", "smtp");
  name = config->readEntry("name", i18n("Unnamed"));
  host = config->readEntry("host", "localhost");
  port = config->readEntry("port", "25");
  user = config->readEntry("user");
  mPasswd = KMAccount::decryptStr(config->readEntry("pass"));
  precommand = config->readPathEntry("precommand");
  encryption = config->readEntry("encryption");
  authType = config->readEntry("authtype");
  auth = config->readBoolEntry("auth");
  mStorePasswd = config->readBoolEntry("storepass");
  specifyHostname = config->readBoolEntry("specifyHostname", false);
  localHostname = config->readEntry("localHostname");

  if ( !storePasswd() )
    return;

  if ( !mPasswd.isEmpty() ) {
    // migration to kwallet if available
    if ( Wallet::isEnabled() ) {
      config->deleteEntry( "pass" );
      mPasswdDirty = true;
      mStorePasswdInConfig = false;
      writeConfig( id );
    } else
      mStorePasswdInConfig = true;
  } else {
    // read password if wallet is open, defer otherwise
    if ( Wallet::isOpen( Wallet::NetworkWallet() ) )
      readPassword();
  }
}


void KMTransportInfo::writeConfig(int id)
{
  KConfig *config = KMKernel::config();
  KConfigGroupSaver saver(config, "Transport " + TQString::number(id));
  if (!mId)
    mId = TransportManager::createId();
  config->writeEntry("id", mId);
  config->writeEntry("type", type);
  config->writeEntry("name", name);
  config->writeEntry("host", host);
  config->writeEntry("port", port);
  config->writeEntry("user", user);
  config->writePathEntry("precommand", precommand);
  config->writeEntry("encryption", encryption);
  config->writeEntry("authtype", authType);
  config->writeEntry("auth", auth);
  config->writeEntry("storepass", storePasswd());
  config->writeEntry("specifyHostname", specifyHostname);
  config->writeEntry("localHostname", localHostname);

  if ( storePasswd() ) {
    // write password into the wallet if possible and necessary
    bool passwdStored = false;
    Wallet *wallet = kmkernel->wallet();
    if ( mPasswdDirty ) {
      if ( wallet && wallet->writePassword( "transport-" + TQString::number(mId), passwd() ) == 0 ) {
        passwdStored = true;
        mPasswdDirty = false;
        mStorePasswdInConfig = false;
      }
    } else {
      passwdStored = wallet ? !mStorePasswdInConfig /*already in the wallet*/ : config->hasKey("pass");
    }
    // wallet not available, ask the user if we should use the config file instead
    if ( !passwdStored && ( mStorePasswdInConfig ||  KMessageBox::warningYesNo( 0,
         i18n("KWallet is not available. It is strongly recommended to use "
              "KWallet for managing your passwords.\n"
              "However, KMail can store the password in its configuration "
              "file instead. The password is stored in an obfuscated format, "
              "but should not be considered secure from decryption efforts "
              "if access to the configuration file is obtained.\n"
              "Do you want to store the password for account '%1' in the "
              "configuration file?").arg( name ),
         i18n("KWallet Not Available"),
         KGuiItem( i18n("Store Password") ),
         KGuiItem( i18n("Do Not Store Password") ) )
         == KMessageBox::Yes ) ) {
      config->writeEntry( "pass", KMAccount::encryptStr( passwd() ) );
      mStorePasswdInConfig = true;
    }
  }

  // delete already stored password if password storage is disabled
  if ( !storePasswd() ) {
    if ( !Wallet::keyDoesNotExist(
          Wallet::NetworkWallet(), "kmail", "transport-" + TQString::number(mId) ) ) {
      Wallet *wallet = kmkernel->wallet();
      if ( wallet )
        wallet->removeEntry( "transport-" + TQString::number(mId) );
    }
    config->deleteEntry( "pass" );
  }
}


int KMTransportInfo::findTransport(const TQString &name)
{
  KConfig *config = KMKernel::config();
  KConfigGroupSaver saver(config, "General");
  int numTransports = config->readNumEntry("transports", 0);
  for (int i = 1; i <= numTransports; i++)
  {
    KConfigGroupSaver saver(config, "Transport " + TQString::number(i));
    if (config->readEntry("name") == name) return i;
  }
  return 0;
}


TQStringList KMTransportInfo::availableTransports()
{
  TQStringList result;
  KConfig *config = KMKernel::config();
  KConfigGroupSaver saver(config, "General");
  int numTransports = config->readNumEntry("transports", 0);
  for (int i = 1; i <= numTransports; i++)
  {
    KConfigGroupSaver saver(config, "Transport " + TQString::number(i));
    result.append(config->readEntry("name"));
  }
  return result;
}


TQString KMTransportInfo::passwd() const
{
  if ( auth && storePasswd() && mPasswd.isEmpty() )
    readPassword();
  return mPasswd;
}


void KMTransportInfo::setPasswd( const TQString &passwd )
{
  if ( passwd != mPasswd ) {
    mPasswd = passwd;
    mPasswdDirty = true;
  }
}


void KMTransportInfo::setStorePasswd( bool store )
{
  if ( mStorePasswd != store && store )
    mPasswdDirty = true;
  mStorePasswd = store;
}


void KMTransportInfo::readPassword() const
{
  if ( !storePasswd() || !auth )
    return;

  // ### workaround for broken Wallet::keyDoesNotExist() which returns wrong
  // results for new entries without closing and reopening the wallet
  if ( Wallet::isOpen( Wallet::NetworkWallet() ) ) {
    Wallet* wallet = kmkernel->wallet();
    if ( !wallet || !wallet->hasEntry( "transport-" + TQString::number(mId) ) )
      return;
  } else {
    if ( Wallet::keyDoesNotExist( Wallet::NetworkWallet(), "kmail", "transport-" + TQString::number(mId) ) )
      return;
  }

  if ( kmkernel->wallet() )
    kmkernel->wallet()->readPassword( "transport-" + TQString::number(mId), mPasswd );
}


KMTransportSelDlg::KMTransportSelDlg( TQWidget *parent, const char *name,
  bool modal )
  : KDialogBase( parent, name, modal, i18n("Add Transport"), Ok|Cancel, Ok )
{
  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

  TQButtonGroup *group = new TQButtonGroup( i18n("Transport"), page );
  connect(group, TQT_SIGNAL(clicked(int)), TQT_SLOT(buttonClicked(int)) );

  topLayout->addWidget( group, 10 );
  TQVBoxLayout *vlay = new TQVBoxLayout( group, spacingHint()*2, spacingHint() );
  vlay->addSpacing( fontMetrics().lineSpacing() );

  TQRadioButton *radioButton1 = new TQRadioButton( i18n("SM&TP"), group );
  vlay->addWidget( radioButton1 );
  TQRadioButton *radioButton2 = new TQRadioButton( i18n("&Sendmail"), group );
  vlay->addWidget( radioButton2 );

  vlay->addStretch( 10 );

  radioButton1->setChecked(true); // Pop is most common ?
  buttonClicked(0);
}

void KMTransportSelDlg::buttonClicked( int id )
{
  mSelectedButton = id;
}


int KMTransportSelDlg::selected( void ) const
{
  return mSelectedButton;
}


KMTransportDialog::KMTransportDialog( const TQString & caption,
				      KMTransportInfo *transportInfo,
				      TQWidget *parent, const char *name,
				      bool modal )
  : KDialogBase( parent, name, modal, caption, Ok|Cancel, Ok, true ),
    mServerTest( 0 ),
    mTransportInfo( transportInfo ),
    mAuthNone( AllAuth ), mAuthSSL( AllAuth ), mAuthTLS( AllAuth )
{
  assert(transportInfo != 0);

  if( transportInfo->type == TQString::fromLatin1("sendmail") )
  {
    makeSendmailPage();
  } else {
    makeSmtpPage();
  }

  setupSettings();
}


KMTransportDialog::~KMTransportDialog()
{
}


void KMTransportDialog::makeSendmailPage()
{
  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

  mSendmail.titleLabel = new TQLabel( page );
  mSendmail.titleLabel->setText( i18n("Transport: Sendmail") );
  TQFont titleFont( mSendmail.titleLabel->font() );
  titleFont.setBold( true );
  mSendmail.titleLabel->setFont( titleFont );
  topLayout->addWidget( mSendmail.titleLabel );
  KSeparator *hline = new KSeparator( KSeparator::HLine, page);
  topLayout->addWidget( hline );

  TQGridLayout *grid = new TQGridLayout( topLayout, 3, 3, spacingHint() );
  grid->addColSpacing( 1, fontMetrics().maxWidth()*15 );
  grid->setRowStretch( 2, 10 );
  grid->setColStretch( 1, 10 );

  TQLabel *label = new TQLabel( i18n("&Name:"), page );
  grid->addWidget( label, 0, 0 );
  mSendmail.nameEdit = new KLineEdit( page );
  label->setBuddy( mSendmail.nameEdit );
  grid->addWidget( mSendmail.nameEdit, 0, 1 );

  label = new TQLabel( i18n("&Location:"), page );
  grid->addWidget( label, 1, 0 );
  mSendmail.locationEdit = new KLineEdit( page );
  label->setBuddy(mSendmail.locationEdit);
  grid->addWidget( mSendmail.locationEdit, 1, 1 );
  mSendmail.chooseButton =
    new TQPushButton( i18n("Choos&e..."), page );
  connect( mSendmail.chooseButton, TQT_SIGNAL(clicked()),
           this, TQT_SLOT(slotSendmailChooser()) );

  connect( mSendmail.locationEdit, TQT_SIGNAL(textChanged ( const TQString & )),
           this, TQT_SLOT(slotSendmailEditPath(const TQString &)) );

  mSendmail.chooseButton->setAutoDefault( false );
  grid->addWidget( mSendmail.chooseButton, 1, 2 );
  slotSendmailEditPath(mSendmail.locationEdit->text());
}

void KMTransportDialog::slotSendmailEditPath(const TQString & _text)
{
  enableButtonOK( !_text.isEmpty() );
}

void KMTransportDialog::makeSmtpPage()
{
  TQFrame *page = makeMainWidget();
  TQVBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );

  mSmtp.titleLabel = new TQLabel( page );
  mSmtp.titleLabel->setText( i18n("Transport: SMTP") );
  TQFont titleFont( mSmtp.titleLabel->font() );
  titleFont.setBold( true );
  mSmtp.titleLabel->setFont( titleFont );
  topLayout->addWidget( mSmtp.titleLabel );
  KSeparator *hline = new KSeparator( KSeparator::HLine, page);
  topLayout->addWidget( hline );

  TQTabWidget *tabWidget = new TQTabWidget(page);
  topLayout->addWidget( tabWidget );

  TQWidget *page1 = new TQWidget( tabWidget );
  tabWidget->addTab( page1, i18n("&General") );

  TQGridLayout *grid = new TQGridLayout( page1, 14, 2, spacingHint() );
  grid->addColSpacing( 1, fontMetrics().maxWidth()*15 );
  grid->setRowStretch( 13, 10 );
  grid->setColStretch( 1, 10 );

  TQLabel *label = new TQLabel( i18n("&Name:"), page1 );
  grid->addWidget( label, 0, 0 );
  mSmtp.nameEdit = new KLineEdit( page1 );
  TQWhatsThis::add(mSmtp.nameEdit,
                  i18n("The name that KMail will use when "
                       "referring to this server."));
  label->setBuddy( mSmtp.nameEdit );
  grid->addWidget( mSmtp.nameEdit, 0, 1 );

  label = new TQLabel( i18n("&Host:"), page1 );
  grid->addWidget( label, 3, 0 );
  mSmtp.hostEdit = new KLineEdit( page1 );
  TQWhatsThis::add(mSmtp.hostEdit,
                  i18n("The domain name or numerical address "
                       "of the SMTP server."));
  label->setBuddy( mSmtp.hostEdit );
  grid->addWidget( mSmtp.hostEdit, 3, 1 );

  label = new TQLabel( i18n("&Port:"), page1 );
  grid->addWidget( label, 4, 0 );
  mSmtp.portEdit = new KLineEdit( page1 );
  mSmtp.portEdit->setValidator( new TQIntValidator(this) );
  TQWhatsThis::add(mSmtp.portEdit,
                  i18n("The port number that the SMTP server "
                       "is listening on. The default port is 25."));
  label->setBuddy( mSmtp.portEdit );
  grid->addWidget( mSmtp.portEdit, 4, 1 );

  label = new TQLabel( i18n("Preco&mmand:"), page1 );
  grid->addWidget( label, 5, 0 );
  mSmtp.precommand = new KLineEdit( page1 );
  TQWhatsThis::add(mSmtp.precommand,
                  i18n("A command to run locally, prior "
                       "to sending email. This can be used "
                       "to set up ssh tunnels, for example. "
                       "Leave it empty if no command should be run."));
  label->setBuddy(mSmtp.precommand);
  grid->addWidget( mSmtp.precommand, 5, 1 );

  TQFrame* line = new TQFrame( page1 );
  line->setFrameStyle( TQFrame::HLine | TQFrame::Plain );
  grid->addMultiCellWidget( line, 6, 6, 0, 1 );

  mSmtp.authCheck =
    new TQCheckBox( i18n("Server &requires authentication"), page1 );
  TQWhatsThis::add(mSmtp.authCheck,
                  i18n("Check this option if your SMTP server "
                       "requires authentication before accepting "
                       "mail. This is known as "
                       "'Authenticated SMTP' or simply ASMTP."));
  connect(mSmtp.authCheck, TQT_SIGNAL(clicked()),
          TQT_SLOT(slotRequiresAuthClicked()));
  grid->addMultiCellWidget( mSmtp.authCheck, 7, 7, 0, 1 );

  mSmtp.loginLabel = new TQLabel( i18n("&Login:"), page1 );
  grid->addWidget( mSmtp.loginLabel, 8, 0 );
  mSmtp.loginEdit = new KLineEdit( page1 );
  mSmtp.loginLabel->setBuddy( mSmtp.loginEdit );
  TQWhatsThis::add(mSmtp.loginEdit,
                  i18n("The user name to send to the server "
                       "for authorization"));
  grid->addWidget( mSmtp.loginEdit, 8, 1 );

  mSmtp.passwordLabel = new TQLabel( i18n("P&assword:"), page1 );
  grid->addWidget( mSmtp.passwordLabel, 9, 0 );
  mSmtp.passwordEdit = new KLineEdit( page1 );
  mSmtp.passwordEdit->setEchoMode( TQLineEdit::Password );
  mSmtp.passwordLabel->setBuddy( mSmtp.passwordEdit );
  TQWhatsThis::add(mSmtp.passwordEdit,
                  i18n("The password to send to the server "
                       "for authorization"));
  grid->addWidget( mSmtp.passwordEdit, 9, 1 );

  mSmtp.storePasswordCheck =
    new TQCheckBox( i18n("&Store SMTP password"), page1 );
  TQWhatsThis::add(mSmtp.storePasswordCheck,
                  i18n("Check this option to have KMail store "
                  "the password.\nIf KWallet is available "
                  "the password will be stored there which is considered "
                  "safe.\nHowever, if KWallet is not available, "
                  "the password will be stored in KMail's configuration "
                  "file. The password is stored in an "
                  "obfuscated format, but should not be "
                  "considered secure from decryption efforts "
                  "if access to the configuration file is obtained."));
  grid->addMultiCellWidget( mSmtp.storePasswordCheck, 10, 10, 0, 1 );

  line = new TQFrame( page1 );
  line->setFrameStyle( TQFrame::HLine | TQFrame::Plain );
  grid->addMultiCellWidget( line, 11, 11, 0, 1 );

  mSmtp.specifyHostnameCheck =
    new TQCheckBox( i18n("Sen&d custom hostname to server"), page1 );
  grid->addMultiCellWidget( mSmtp.specifyHostnameCheck, 12, 12, 0, 1 );
  TQWhatsThis::add(mSmtp.specifyHostnameCheck,
                  i18n("Check this option to have KMail use "
                       "a custom hostname when identifying itself "
                       "to the mail server."
                       "<p>This is useful when your system's hostname "
                       "may not be set correctly or to mask your "
                       "system's true hostname."));

  mSmtp.localHostnameLabel = new TQLabel( i18n("Hos&tname:"), page1 );
  grid->addWidget( mSmtp.localHostnameLabel, 13, 0);
  mSmtp.localHostnameEdit = new KLineEdit( page1 );
  TQWhatsThis::add(mSmtp.localHostnameEdit,
                  i18n("Enter the hostname KMail should use when "
                       "identifying itself to the server."));
  mSmtp.localHostnameLabel->setBuddy( mSmtp.localHostnameEdit );
  grid->addWidget( mSmtp.localHostnameEdit, 13, 1 );
  connect( mSmtp.specifyHostnameCheck, TQT_SIGNAL(toggled(bool)),
           mSmtp.localHostnameEdit, TQT_SLOT(setEnabled(bool)));
  connect( mSmtp.specifyHostnameCheck, TQT_SIGNAL(toggled(bool)),
           mSmtp.localHostnameLabel, TQT_SLOT(setEnabled(bool)));

  TQWidget *page2 = new TQWidget( tabWidget );
  tabWidget->addTab( page2, i18n("S&ecurity") );
  TQVBoxLayout *vlay = new TQVBoxLayout( page2, spacingHint() );
  mSmtp.encryptionGroup = new TQButtonGroup( 1, Qt::Horizontal,
    i18n("Encryption"), page2 );
  mSmtp.encryptionNone =
    new TQRadioButton( i18n("&None"), mSmtp.encryptionGroup );
  mSmtp.encryptionSSL =
    new TQRadioButton( i18n("&SSL"), mSmtp.encryptionGroup );
  mSmtp.encryptionTLS =
    new TQRadioButton( i18n("&TLS"), mSmtp.encryptionGroup );
  connect(mSmtp.encryptionGroup, TQT_SIGNAL(clicked(int)),
    TQT_SLOT(slotSmtpEncryptionChanged(int)));
  vlay->addWidget( mSmtp.encryptionGroup );

  mSmtp.authGroup = new TQButtonGroup( 1, Qt::Horizontal,
    i18n("Authentication Method"), page2 );
  mSmtp.authLogin = new TQRadioButton( i18n("Please translate this "
    "authentication method only if you have a good reason", "&LOGIN"),
    mSmtp.authGroup );
  mSmtp.authPlain = new TQRadioButton( i18n("Please translate this "
    "authentication method only if you have a good reason", "&PLAIN"),
    mSmtp.authGroup  );
  mSmtp.authCramMd5 = new TQRadioButton( i18n("CRAM-MD&5"), mSmtp.authGroup );
  mSmtp.authDigestMd5 = new TQRadioButton( i18n("&DIGEST-MD5"), mSmtp.authGroup );
  mSmtp.authNTLM = new TQRadioButton( i18n("&NTLM"), mSmtp.authGroup );
  mSmtp.authGSSAPI = new TQRadioButton( i18n("&GSSAPI"), mSmtp.authGroup );
  if ( KProtocolInfo::capabilities("smtp").contains("SASL") == 0 ) {
    mSmtp.authNTLM->hide();
    mSmtp.authGSSAPI->hide();
  }
  vlay->addWidget( mSmtp.authGroup );

  vlay->addStretch();

  TQHBoxLayout *buttonLay = new TQHBoxLayout( vlay );
  mSmtp.checkCapabilities =
    new TQPushButton( i18n("Check &What the Server Supports"), page2 );
  connect(mSmtp.checkCapabilities, TQT_SIGNAL(clicked()),
    TQT_SLOT(slotCheckSmtpCapabilities()));
  buttonLay->addStretch();
  buttonLay->addWidget( mSmtp.checkCapabilities );
}


void KMTransportDialog::setupSettings()
{
  if (mTransportInfo->type == "sendmail")
  {
    mSendmail.nameEdit->setText(mTransportInfo->name);
    mSendmail.locationEdit->setText(mTransportInfo->host);
  } else {
    mSmtp.nameEdit->setText(mTransportInfo->name);
    mSmtp.hostEdit->setText(mTransportInfo->host);
    mSmtp.portEdit->setText(mTransportInfo->port);
    mSmtp.authCheck->setChecked(mTransportInfo->auth);
    mSmtp.loginEdit->setText(mTransportInfo->user);
    mSmtp.passwordEdit->setText(mTransportInfo->passwd());
    mSmtp.storePasswordCheck->setChecked(mTransportInfo->storePasswd());
    mSmtp.precommand->setText(mTransportInfo->precommand);
    mSmtp.specifyHostnameCheck->setChecked(mTransportInfo->specifyHostname);
    mSmtp.localHostnameEdit->setText(mTransportInfo->localHostname);

    if (mTransportInfo->encryption == "TLS")
      mSmtp.encryptionTLS->setChecked(true);
    else if (mTransportInfo->encryption == "SSL")
      mSmtp.encryptionSSL->setChecked(true);
    else mSmtp.encryptionNone->setChecked(true);

    if (mTransportInfo->authType == "LOGIN")
      mSmtp.authLogin->setChecked(true);
    else if (mTransportInfo->authType == "CRAM-MD5")
      mSmtp.authCramMd5->setChecked(true);
    else if (mTransportInfo->authType == "DIGEST-MD5")
      mSmtp.authDigestMd5->setChecked(true);
    else if (mTransportInfo->authType == "NTLM")
      mSmtp.authNTLM->setChecked(true);
    else if (mTransportInfo->authType == "GSSAPI")
      mSmtp.authGSSAPI->setChecked(true);
    else mSmtp.authPlain->setChecked(true);

    slotRequiresAuthClicked();
    mSmtp.localHostnameEdit->setEnabled(mTransportInfo->specifyHostname);
    mSmtp.localHostnameLabel->setEnabled(mTransportInfo->specifyHostname);
  }
}


void KMTransportDialog::saveSettings()
{
  if (mTransportInfo->type == "sendmail")
  {
    mTransportInfo->name = mSendmail.nameEdit->text().stripWhiteSpace();
    mTransportInfo->host = mSendmail.locationEdit->text().stripWhiteSpace();
  } else {
    mTransportInfo->name = mSmtp.nameEdit->text();
    mTransportInfo->host = mSmtp.hostEdit->text().stripWhiteSpace();
    mTransportInfo->port = mSmtp.portEdit->text().stripWhiteSpace();
    mTransportInfo->auth = mSmtp.authCheck->isChecked();
    mTransportInfo->user = mSmtp.loginEdit->text().stripWhiteSpace();
    mTransportInfo->setPasswd( mSmtp.passwordEdit->text() );
    mTransportInfo->setStorePasswd( mSmtp.storePasswordCheck->isChecked() );
    mTransportInfo->precommand = mSmtp.precommand->text().stripWhiteSpace();
    mTransportInfo->specifyHostname = mSmtp.specifyHostnameCheck->isChecked();
    mTransportInfo->localHostname = mSmtp.localHostnameEdit->text().stripWhiteSpace();

    mTransportInfo->encryption = (mSmtp.encryptionTLS->isChecked()) ? "TLS" :
    (mSmtp.encryptionSSL->isChecked()) ? "SSL" : "NONE";

    mTransportInfo->authType = (mSmtp.authLogin->isChecked()) ? "LOGIN" :
    (mSmtp.authCramMd5->isChecked()) ? "CRAM-MD5" :
    (mSmtp.authDigestMd5->isChecked()) ? "DIGEST-MD5" :
    (mSmtp.authNTLM->isChecked()) ? "NTLM" :
    (mSmtp.authGSSAPI->isChecked()) ? "GSSAPI" : "PLAIN";
  }
}


void KMTransportDialog::slotSendmailChooser()
{
  KFileDialog dialog("/", TQString::null, this, 0, true );
  dialog.setCaption(i18n("Choose sendmail Location") );

  if( dialog.exec() == TQDialog::Accepted )
  {
    KURL url = dialog.selectedURL();
    if( url.isEmpty() == true )
    {
      return;
    }

    if( url.isLocalFile() == false )
    {
      KMessageBox::sorry( 0, i18n( "Only local files allowed." ) );
      return;
    }

    mSendmail.locationEdit->setText( url.path() );
  }
}


void KMTransportDialog::slotRequiresAuthClicked()
{
  bool b = mSmtp.authCheck->isChecked();
  mSmtp.loginLabel->setEnabled(b);
  mSmtp.loginEdit->setEnabled(b);
  mSmtp.passwordLabel->setEnabled(b);
  mSmtp.passwordEdit->setEnabled(b);
  mSmtp.storePasswordCheck->setEnabled(b);
  mSmtp.authGroup->setEnabled(b);
}


void KMTransportDialog::slotSmtpEncryptionChanged(int id)
{
  kdDebug(5006) << "KMTransportDialog::slotSmtpEncryptionChanged( " << id << " )" << endl;
  // adjust SSL port:
  if (id == SSL || mSmtp.portEdit->text() == "465")
    mSmtp.portEdit->setText((id == SSL) ? "465" : "25");

  // switch supported auth methods:
  TQButton * old = mSmtp.authGroup->selected();
  int authMethods = id == TLS ? mAuthTLS : id == SSL ? mAuthSSL : mAuthNone ;
  enableAuthMethods( authMethods );
  if ( !old->isEnabled() )
    checkHighest( mSmtp.authGroup );
}

void KMTransportDialog::enableAuthMethods( unsigned int auth ) {
  kdDebug(5006) << "KMTransportDialog::enableAuthMethods( " << auth << " )" << endl;
  mSmtp.authPlain->setEnabled( auth & PLAIN );
  // LOGIN doesn't offer anything over PLAIN, requires more server
  // roundtrips and is not an official SASL mechanism, but a MS-ism,
  // so only enable it if PLAIN isn't available:
  mSmtp.authLogin->setEnabled( auth & LOGIN /*&& !(auth & PLAIN)*/);
  mSmtp.authCramMd5->setEnabled( auth & CRAM_MD5 );
  mSmtp.authDigestMd5->setEnabled( auth & DIGEST_MD5 );
  mSmtp.authNTLM->setEnabled( auth & NTLM );
  mSmtp.authGSSAPI->setEnabled( auth & GSSAPI );
}

unsigned int KMTransportDialog::authMethodsFromString( const TQString & s ) {
  unsigned int result = 0;
  TQStringList sl = TQStringList::split( '\n', s.upper() );
  for ( TQStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it )
    if (  *it == "SASL/LOGIN" )
      result |= LOGIN;
    else if ( *it == "SASL/PLAIN" )
      result |= PLAIN;
    else if ( *it == "SASL/CRAM-MD5" )
      result |= CRAM_MD5;
    else if ( *it == "SASL/DIGEST-MD5" )
      result |= DIGEST_MD5;
    else if ( *it == "SASL/NTLM" )
      result |= NTLM;
    else if ( *it == "SASL/GSSAPI" )
      result |= GSSAPI;
  return result;
}

unsigned int KMTransportDialog::authMethodsFromStringList( const TQStringList & sl ) {
  unsigned int result = 0;
  for ( TQStringList::const_iterator it = sl.begin() ; it != sl.end() ; ++it )
    if ( *it == "LOGIN" )
      result |= LOGIN;
    else if ( *it == "PLAIN" )
      result |= PLAIN;
    else if ( *it == "CRAM-MD5" )
      result |= CRAM_MD5;
    else if ( *it == "DIGEST-MD5" )
      result |= DIGEST_MD5;
    else if ( *it == "NTLM" )
      result |= NTLM;
    else if ( *it == "GSSAPI" )
      result |= GSSAPI;
  return result;
}

void KMTransportDialog::slotCheckSmtpCapabilities()
{
  delete mServerTest;
  mServerTest = new KMServerTest(SMTP_PROTOCOL, mSmtp.hostEdit->text(),
    mSmtp.portEdit->text().toInt());
  connect( mServerTest,
           TQT_SIGNAL( capabilities( const TQStringList &, const TQStringList &,
                                 const TQString &, const TQString &,
                                 const TQString & )),
           this,
           TQT_SLOT( slotSmtpCapabilities( const TQStringList &,
                                       const TQStringList &, const TQString &,
                                       const TQString &, const TQString & ) ) );
  mSmtp.checkCapabilities->setEnabled(false);
}


void KMTransportDialog::checkHighest(TQButtonGroup *btnGroup)
{
  for ( int i = btnGroup->count() - 1; i >= 0 ; --i )
  {
    TQButton * btn = btnGroup->find(i);
    if (btn && btn->isEnabled())
    {
      btn->animateClick();
      return;
    }
  }
}


void KMTransportDialog::slotSmtpCapabilities( const TQStringList & capaNormal,
                                              const TQStringList & capaSSL,
                                              const TQString & authNone,
                                              const TQString & authSSL,
                                              const TQString & authTLS )
{
  mSmtp.checkCapabilities->setEnabled( true );
  kdDebug(5006) << "KMTransportDialog::slotSmtpCapabilities( ..., "
	    << authNone << ", " << authSSL << ", " << authTLS << " )" << endl;
  mSmtp.encryptionNone->setEnabled( !capaNormal.isEmpty() );
  mSmtp.encryptionSSL->setEnabled( !capaSSL.isEmpty() );
  mSmtp.encryptionTLS->setEnabled( capaNormal.findIndex("STARTTLS") != -1 );
  if ( authNone.isEmpty() && authSSL.isEmpty() && authTLS.isEmpty() ) {
    // slave doesn't seem to support "* AUTH METHODS" metadata (or server can't do AUTH)
    mAuthNone = authMethodsFromStringList( capaNormal );
    if ( mSmtp.encryptionTLS->isEnabled() )
      mAuthTLS = mAuthNone;
    else
      mAuthTLS = 0;
    mAuthSSL = authMethodsFromStringList( capaSSL );
  }
  else {
    mAuthNone = authMethodsFromString( authNone );
    mAuthSSL = authMethodsFromString( authSSL );
    mAuthTLS = authMethodsFromString( authTLS );
  }
  kdDebug(5006) << "mAuthNone = " << mAuthNone
                << "; mAuthSSL = " << mAuthSSL
                << "; mAuthTLS = " << mAuthTLS << endl;
  checkHighest( mSmtp.encryptionGroup );
  delete mServerTest;
  mServerTest = 0;
}
bool KMTransportDialog::sanityCheckSmtpInput()
{
  // FIXME: add additional checks for all fields that needs it
  // this is only the beginning
  if ( mSmtp.hostEdit->text().isEmpty() ) {
    TQString errorMsg = i18n("The Host field cannot be empty. Please  "
                            "enter the name or the IP address of the SMTP server.");
    KMessageBox::sorry( this, errorMsg, i18n("Invalid Hostname or Address") );
    return false;
  }
  return true;
}

void KMTransportDialog::slotOk()
{
  if (mTransportInfo->type != "sendmail") {
    if( !sanityCheckSmtpInput() ) {
      return;
    }
  }

  saveSettings();
  accept();
}


#include "kmtransport.moc"
