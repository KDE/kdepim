/*  -*- mode: C++; c-file-style: "gnu" -*-
    kpgp.cpp

    Copyright (C) 2001,2002 the KPGP authors
    See file AUTHORS.kpgp for details

    This file is part of KPGP, the KDE PGP/GnuPG support library.

    KPGP is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include <qlabel.h>
#include <qcursor.h>
#include <qapplication.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfigbase.h>
#include <kconfig.h>
#include <kstaticdeleter.h>

#include "kpgpbase.h"
#include "kpgpui.h"
#include "kpgp.h"

namespace Kpgp {

Module *Module::kpgpObject = 0L;
static KStaticDeleter<Module> kpgpod;

Module::Module()
  : mPublicKeys(),
    mPublicKeysCached(false),
    mSecretKeys(),
    mSecretKeysCached(false),
    passphrase(0), passphrase_buffer_len(0), havePassPhrase(false)
{
  if (!kpgpObject) {
    kdDebug(5100) << "creating new pgp object" << endl;
  }
  kpgpObject=kpgpod.setObject(Module::kpgpObject, this);
  pgp = 0;

  config = new KConfig("kpgprc");

  init();
}

Module::~Module()
{
  writeAddressData();

  if (kpgpObject == this) kpgpObject = kpgpod.setObject( Module::kpgpObject, 0, false );
  clear(TRUE);
  delete config;
  delete pgp;
}

// ----------------- public methods -------------------------

void
Module::init()
{
  wipePassPhrase();

  // read kpgp config file entries
  readConfig();

  // read the email address -> { encryption keys, encryption preference }
  // associations
  readAddressData();

  // do we have a pgp executable
  checkForPGP();

  // create the Base object later when it is
  // needed to avoid the costly check done for
  // the autodetection of PGP 2/6
  //assignPGPBase();
  delete pgp;
  pgp=0;
}


void
Module::readConfig()
{
  storePass = config->readBoolEntry("storePass", false);
  showEncryptionResult = config->readBoolEntry("showEncryptionResult", true);
  mShowKeyApprovalDlg = config->readBoolEntry( "showKeysForApproval", true );
  // We have no config GUI for this key anymore, and the KPGP backend isn't ported,
  // so let's just use Auto all the time.  See #92619.
  ///pgpType = (Module::PGPType) config->readNumEntry("pgpType", tAuto);
  pgpType = tAuto;
  flagEncryptToSelf = config->readBoolEntry("encryptToSelf", true);
}

void
Module::writeConfig(bool sync)
{
  config->writeEntry("storePass", storePass);
  config->writeEntry("showEncryptionResult", showEncryptionResult);
  config->writeEntry( "showKeysForApproval", mShowKeyApprovalDlg );
  //config->writeEntry("pgpType", (int) pgpType);
  config->writeEntry("encryptToSelf", flagEncryptToSelf);

  if(sync)
    config->sync();

  /// ### Why is the pgp object deleted? This is only necessary if the
  ///     PGP type was changed in the config dialog.
  delete pgp;
  pgp = 0;
}


void
Module::setUser(const KeyID& keyID)
{
  if (pgpUser != keyID) {
    pgpUser = keyID;
    wipePassPhrase();
  }
}

const KeyID
Module::user(void) const
{
  return pgpUser;
}


void
Module::setEncryptToSelf(bool flag)
{
  flagEncryptToSelf = flag;
}

bool
Module::encryptToSelf(void) const
{
  return flagEncryptToSelf;
}


void
Module::setStorePassPhrase(bool flag)
{
  storePass = flag;
}

bool
Module::storePassPhrase(void) const
{
  return storePass;
}

int
Module::prepare( bool needPassPhrase, Block* block )
{
  if (0 == pgp) assignPGPBase();

  if(!havePgp)
  {
    errMsg = i18n("Could not find PGP executable.\n"
		       "Please check your PATH is set correctly.");
    return 0;
  }

  if( block && ( block->status() & NO_SEC_KEY ) )
    return 0;

  if(needPassPhrase && !havePassPhrase) {
    if( ( tGPG == pgpType ) && ( 0 != getenv("GPG_AGENT_INFO") ) ) {
      // the user uses gpg-agent which asks itself for the passphrase
      kdDebug(5100) << "user uses gpg-agent -> don't ask for passphrase\n";
      // set dummy passphrase (because else signing doesn't work -> FIXME)
      setPassPhrase( "dummy" );
    }
    else {
      QString ID;
      if( block )
        ID = block->requiredUserId();
      PassphraseDialog passdlg(0, i18n("OpenPGP Security Check"), true, ID);
      QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
      int passdlgResult = passdlg.exec();
      QApplication::restoreOverrideCursor();
      if (passdlgResult == QDialog::Accepted) {
        if (!setPassPhrase(passdlg.passphrase())) {
          if (strlen(passdlg.passphrase()) >= 1024)
             errMsg = i18n("Passphrase is too long, it must contain fewer than 1024 characters.");
          else
             errMsg = i18n("Out of memory.");
  	return 0;
        }
      } else {
        wipePassPhrase();
        return -1;
      }
    }
  }
  return 1;
}

void
Module::wipePassPhrase(bool freeMem)
{
  if ( passphrase ) {
    if ( passphrase_buffer_len )
      memset( passphrase, 0x00, passphrase_buffer_len );
    else {
      kdDebug(5100) << "wipePassPhrase: passphrase && !passphrase_buffer_len ???" << endl;
      passphrase = 0;
    }
  }
  if ( freeMem && passphrase ) {
    free( passphrase );
    passphrase = 0;
    passphrase_buffer_len = 0;
  }
  havePassPhrase = false;
}

bool
Module::verify( Block& block )
{
  int retval;

  if (0 == pgp) assignPGPBase();

  // everything ready
  if( !prepare( false, &block ) )
    return false;
  // ok now try to verify the message.
  retval = pgp->verify( block );

  if(retval & ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}

bool
Module::decrypt( Block& block )
{
  int retval;

  if (0 == pgp) assignPGPBase();

  do {
    // everything ready
    if( prepare( true, &block ) != 1 )
      return FALSE;
    // ok now try to decrypt the message.
    retval = pgp->decrypt( block, passphrase );
    // loop on bad passphrase
    if( retval & BADPHRASE ) {
      wipePassPhrase();
      QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
      int ret = KMessageBox::warningContinueCancel(0,
	     i18n("You just entered an invalid passphrase.\n"
		  "Do you want to try again, or "
		  "cancel and view the message undecrypted?"),
	     i18n("PGP Warning"), i18n("&Retry"));
      QApplication::restoreOverrideCursor();
      if ( ret == KMessageBox::Cancel ) break;
    } else
      break;
  } while ( true );

  // erase the passphrase if we do not want to keep it
  cleanupPass();

  if(retval & ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}

Kpgp::Result
Module::clearsign( Block& block,
                   const KeyID& keyId, const QCString& charset )
{
  return encrypt( block, QStringList(), keyId, true, charset );
}

Kpgp::Result
Module::encrypt( Block& block,
                 const QStringList& receivers, const KeyID& keyId,
                 bool sign, const QCString& charset )
{
  KeyIDList encryptionKeyIds; // list of keys which are used for encryption
  int status = 0;
  errMsg = "";

  if( 0 == pgp ) assignPGPBase();

  setUser( keyId );

  if( !receivers.empty() ) {
    Kpgp::Result result = getEncryptionKeys( encryptionKeyIds, receivers,
                                             keyId );
    if( Kpgp::Ok != result ) {
      return result;
    }
  }

  status = doEncSign( block, encryptionKeyIds, sign );

  if( status & CANCEL )
    return Kpgp::Canceled;

  // check for bad passphrase
  while( status & BADPHRASE ) {
    wipePassPhrase();
    QString str = i18n("You entered an invalid passphrase.\n"
                       "Do you want to try again, continue and leave the "
                       "message unsigned, or cancel sending the message?");
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               i18n("&Retry"),
                                               i18n("Send &Unsigned") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    if( ret == KMessageBox::No ) {
      // the user selected "Send unsigned"
      if( encryptionKeyIds.isEmpty() ) {
        block.reset();
        return Kpgp::Ok;
      }
      else {
        sign = false;
      }
    }
    // ok let's try once again...
    status = doEncSign( block, encryptionKeyIds, sign );
  }

  // did signing fail?
  if( status & ERR_SIGNING ) {
    QString str = i18n("%1 = 'signing failed' error message",
                       "%1\nDo you want to send the message unsigned, "
                       "or cancel sending the message?")
                  .arg( pgp->lastErrorMessage() );
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  i18n("Send &Unsigned") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    sign = false;
    status = doEncSign( block, encryptionKeyIds, sign );
  }

  // check for bad keys
  if( status & BADKEYS ) {
    QString str = i18n("%1 = 'bad keys' error message",
                       "%1\nDo you want to encrypt anyway, leave the "
                       "message as-is, or cancel sending the message?")
                  .arg( pgp->lastErrorMessage() );

    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               i18n("Send &Encrypted"),
                                               i18n("Send &Unencrypted") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    if( ret == KMessageBox::No ) {
      // the user selected "Send unencrypted"
      if( sign ) {
        doEncSign( block, KeyIDList(), sign );
      }
      else {
        block.reset();
      }
      return Kpgp::Ok;
    }
  }

  if( status & MISSINGKEY ) {
    QString str = i18n("%1 = 'missing keys' error message",
                       "%1\nDo you want to leave the message as-is, "
                       "or cancel sending the message?")
                  .arg( pgp->lastErrorMessage() );
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  i18n("&Send As-Is") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    block.reset();
    return Kpgp::Ok;
  }

  if( status & ERROR ) {
    // show error dialog
    errMsg = i18n( "The following error occurred:\n%1" )
             .arg( pgp->lastErrorMessage() );
    QString details = i18n( "This is the error message of %1:\n%2" )
                      .arg( ( pgpType == tGPG ) ? "GnuPG" : "PGP" )
                      .arg( block.error().data() );
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    KMessageBox::detailedSorry( 0, errMsg, details );
    QApplication::restoreOverrideCursor();
    return Kpgp::Failure;
  }

  if( showCipherText() ) {
    // show cipher text dialog
    CipherTextDialog *cipherTextDlg = new CipherTextDialog( block.text(), charset );
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    bool result = ( cipherTextDlg->exec() == QDialog::Accepted );
    QApplication::restoreOverrideCursor();
    delete cipherTextDlg;
    return result == QDialog::Accepted ? Kpgp::Ok : Kpgp::Canceled;
  }
  return Kpgp::Ok;
}

int
Module::doEncSign( Block& block,
                   const KeyIDList& recipientKeyIds, bool sign )
{
  int retval = 0;

  if( 0 == pgp ) assignPGPBase();

  // to avoid error messages in case pgp is not installed
  if( !havePgp ) return OK;

  if( sign ) {
    int result = prepare( true, &block );
    switch( result ) {
      case -1:
        return CANCEL;
      case 0:
        return ERROR;
    }
    retval = pgp->encsign( block, recipientKeyIds, passphrase );
  }
  else {
    if( !prepare( false, &block ) ) return ERROR;
    retval = pgp->encrypt( block, recipientKeyIds );
  }
  // erase the passphrase if we do not want to keep it
  cleanupPass();

  return retval;
}

Kpgp::Result
Module::getEncryptionKeys( KeyIDList& encryptionKeyIds,
                           const QStringList& recipients,
                           const KeyID& keyId )
{
  if( recipients.empty() ) {
    encryptionKeyIds.clear();
    return Kpgp::Ok;
  }

  // list of lists of encryption keys (one list per recipient + one list
  // for the sender)
  QValueVector<KeyIDList> recipientKeyIds( recipients.count() + 1 );
  // add the sender's encryption key(s) to the list of recipient key IDs
  if( encryptToSelf() ) {
    recipientKeyIds[0] = KeyIDList( keyId );
  }
  else {
    recipientKeyIds[0] = KeyIDList();
  }
  bool showKeysForApproval = false;
  int i = 1;
  for( QStringList::ConstIterator it = recipients.begin();
       it != recipients.end(); ++it, ++i ) {
    EncryptPref encrPref = encryptionPreference( *it );
    if( ( encrPref == UnknownEncryptPref ) || ( encrPref == NeverEncrypt ) )
      showKeysForApproval = true;

    KeyIDList keyIds = getEncryptionKeys( *it );
    if( keyIds.isEmpty() ) {
      showKeysForApproval = true;
    }
    recipientKeyIds[i] = keyIds;
  }

  kdDebug(5100) << "recipientKeyIds = (\n";
  QValueVector<KeyIDList>::const_iterator kit;
  for( kit = recipientKeyIds.begin(); kit != recipientKeyIds.end(); ++kit ) {
    kdDebug(5100) << "( 0x" << (*kit).toStringList().join( ", 0x" )
                  << " ),\n";
  }
  kdDebug(5100) << ")\n";

  if( showKeysForApproval || mShowKeyApprovalDlg ) {
    // #### FIXME: Until we support encryption with untrusted keys only
    // ####        trusted keys are allowed
    unsigned int allowedKeys = PublicKeys | EncryptionKeys | ValidKeys | TrustedKeys;
#if 0
    // ### reenable this code when we support encryption with untrusted keys
    if( pgpType != tGPG ) {
      // usage of untrusted keys is only possible with GnuPG
      allowedKeys |= TrustedKeys;
    }
#endif
    // show the recipients <-> key relation
    KeyApprovalDialog dlg( recipients, recipientKeyIds, allowedKeys );

    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = dlg.exec();

    if( ret == QDialog::Rejected ) {
        QApplication::restoreOverrideCursor();
        return Kpgp::Canceled;
    }

    recipientKeyIds = dlg.keys();
    QApplication::restoreOverrideCursor();
  }

  // flatten the list of lists of key IDs and count empty key ID lists
  unsigned int emptyListCount = 0;
  for( QValueVector<KeyIDList>::const_iterator it = recipientKeyIds.begin();
       it != recipientKeyIds.end(); ++it ) {
    if( (*it).isEmpty() ) {
      // only count empty key ID lists for the recipients
      if( it != recipientKeyIds.begin() ) {
        emptyListCount++;
      }
    }
    else {
      for( KeyIDList::ConstIterator kit = (*it).begin();
           kit != (*it).end(); kit++ ) {
        encryptionKeyIds.append( *kit );
      }
    }
  }

  // FIXME-AFTER-KDE-3.1: Show warning if message won't be encrypted to self

  // show a warning if the user didn't select an encryption key for
  // some of the recipients
  if( recipientKeyIds.size() == emptyListCount + 1 ) { // (+1 because of the sender's key)
    QString str = ( recipients.count() == 1 )
                  ? i18n("You did not select an encryption key for the "
                         "recipient of this message; therefore, the message "
                         "will not be encrypted.")
                  : i18n("You did not select an encryption key for any of the "
                         "recipients of this message; therefore, the message "
                         "will not be encrypted.");
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  i18n("Send &Unencrypted") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    else
      encryptionKeyIds.clear();
  }
  else if( emptyListCount > 0 ) {
    QString str = ( emptyListCount == 1 )
                  ? i18n("You did not select an encryption key for one of "
                         "the recipients; this person will not be able to "
                         "decrypt the message if you encrypt it.")
                  : i18n("You did not select encryption keys for some of "
                         "the recipients; these persons will not be able to "
                         "decrypt the message if you encrypt it." );
    QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               i18n("Send &Encrypted"),
                                               i18n("Send &Unencrypted") );
    QApplication::restoreOverrideCursor();
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    else if( ret == KMessageBox::No ) {
      // the user selected "Send unencrypted"
      encryptionKeyIds.clear();
    }
  }

  return Kpgp::Ok;
}

int
Module::encryptionPossible( const QStringList& recipients )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() )
    return 0;

  if( recipients.empty() )
    return 0;

  int noKey = 0, never = 0, unknown = 0, always = 0, aip = 0, ask = 0,
      askwp = 0;
  for( QStringList::ConstIterator it = recipients.begin();
       it != recipients.end(); ++it) {
    if( haveTrustedEncryptionKey( *it ) ) {
      EncryptPref encrPref = encryptionPreference( *it );
      switch( encrPref ) {
        case NeverEncrypt:
          never++;
          break;
        case UnknownEncryptPref:
          unknown++;
          break;
        case AlwaysEncrypt:
          always++;
          break;
        case AlwaysEncryptIfPossible:
          aip++;
          break;
        case AlwaysAskForEncryption:
          ask++;
          break;
        case AskWheneverPossible:
          askwp++;
          break;
      }
    }
    else {
      noKey++;
    }
  }

  if( ( always+aip > 0 ) && ( never+unknown+ask+askwp+noKey == 0 ) ) {
    return 1; // encryption possible and desired
  }

  if( ( unknown+ask+askwp > 0 ) && ( never+noKey == 0 ) ) {
    return 2; // encryption possible, but user has to be asked
  }

  if( ( never+noKey > 0 ) && ( always+ask == 0 ) ) {
    return 0; // encryption isn't possible or desired
  }

  return -1; // we can't decide it automatically
}

bool
Module::signKey(const KeyID& keyId)
{
  if (0 == pgp) assignPGPBase();

  if( prepare( true ) != 1 )
    return FALSE;
  if(pgp->signKey(keyId, passphrase) & ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}


const KeyList
Module::publicKeys()
{
  if (0 == pgp) assignPGPBase();

  if (!prepare()) return KeyList();

  if( !mPublicKeysCached ) {
    readPublicKeys();
  }

  return mPublicKeys;
}


const KeyList
Module::secretKeys()
{
  if (0 == pgp) assignPGPBase();

  if (!prepare()) return KeyList();

  if( !mSecretKeysCached ) {
    readSecretKeys();
  }

  return mSecretKeys;
}


Key*
Module::publicKey(const KeyID& keyID)
{
  readPublicKeys();

  for( KeyListIterator it( mPublicKeys ); (*it); ++it )
    if( keyID == (*it)->primaryKeyID() ||
	keyID == (*it)->primaryFingerprint() )
      return (*it);

  return 0;
}

Key*
Module::publicKey( const QString& userID )
{
  readPublicKeys();

  for( KeyListIterator it( mPublicKeys ); (*it); ++it )
    if( (*it)->matchesUserID( userID ) )
      return (*it);

  return 0;
}

Key*
Module::secretKey(const KeyID& keyID)
{
  readSecretKeys();

  for( KeyListIterator it( mSecretKeys ); (*it); ++it )
    if( keyID == (*it)->primaryKeyID() ||
	keyID == (*it)->primaryFingerprint() )
      return (*it);

  return 0;
}

Validity
Module::keyTrust( const KeyID& keyID )
{
  Key *key = publicKey( keyID );

  if( ( 0 == key ) || ( key->keyTrust() == KPGP_VALIDITY_UNKNOWN ) )
  { // (re)check the key if it's unknown or if its trust is unknown
    key = rereadKey( keyID, true );
    if( key == 0 )
      return KPGP_VALIDITY_UNKNOWN;
  }

  return key->keyTrust();
}

Validity
Module::keyTrust( const QString& userID )
{
  Key *key = publicKey( userID );

  if( key == 0 )
    return KPGP_VALIDITY_UNKNOWN;

  if( key->keyTrust() == KPGP_VALIDITY_UNKNOWN )
  {
    key = rereadKey( key->primaryKeyID(), true );
    if( key == 0 )
      return KPGP_VALIDITY_UNKNOWN;
  }

  return key->keyTrust();
}

bool
Module::isTrusted( const KeyID& keyID )
{
  return ( keyTrust( keyID ) >= KPGP_VALIDITY_MARGINAL );
}

Key*
Module::rereadKey( const KeyID& keyID, const bool readTrust /* = true */ )
{
  if( 0 == pgp ) assignPGPBase();

  // search the old key data in the key list
  Key* oldKey = publicKey( keyID );

  Key* newKey = pgp->readPublicKey( keyID, readTrust, oldKey );

  if( ( 0 == oldKey ) && ( 0 != newKey ) )
  {
    mPublicKeys.inSort( newKey );
    kdDebug(5100) << "New public key 0x" << newKey->primaryKeyID() << " ("
                  << newKey->primaryUserID() << ").\n";
  }
  else if( ( 0 != oldKey ) && ( 0 == newKey ) )
  { // the key has been deleted in the meantime
    kdDebug(5100) << "Public key 0x" << oldKey->primaryKeyID() << " ("
                  << oldKey->primaryUserID() << ") will be removed.\n";
    mPublicKeys.removeRef( oldKey );
  }

  return newKey;
}

QCString
Module::getAsciiPublicKey(const KeyID& keyID)
{
  if (0 == pgp) assignPGPBase();

  return pgp->getAsciiPublicKey(keyID);
}


bool Module::setPassPhrase(const char * aPass)
{
  // null out old buffer before we touch the new string.  So in case
  // aPass isn't properly null-terminated, we don't leak secret data.
  wipePassPhrase();

  if (aPass)
  {
    size_t newlen = strlen( aPass );
    if ( newlen >= 1024 ) {
      // rediculously long passphrase.
      // Maybe someone wants to trick us in malloc()'ing
      // huge buffers...
      return false;
    }
    if ( passphrase_buffer_len < newlen + 1 ) {
      // too little space in current buffer:
      // allocate a larger one.
      if ( passphrase )
	free( passphrase );
      passphrase_buffer_len = (newlen + 1 + 15) & ~0xF; // make it a multiple of 16.
      passphrase = (char*)malloc( passphrase_buffer_len );
      if (!passphrase) {
	passphrase_buffer_len = 0;
	return false;
      }
    }
    memcpy( passphrase, aPass, newlen + 1 );
    havePassPhrase = true;
  }
  return true;
}

bool
Module::changePassPhrase()
{
  //FIXME...
  KMessageBox::information(0,i18n("This feature is\nstill missing"));
  return FALSE;
}

void
Module::clear(const bool erasePassPhrase)
{
  if(erasePassPhrase)
    wipePassPhrase(true);
}

const QString
Module::lastErrorMsg(void) const
{
  return errMsg;
}

bool
Module::havePGP(void) const
{
  return havePgp;
}

void
Module::setShowCipherText(const bool flag)
{
  showEncryptionResult = flag;
}

bool
Module::showCipherText(void) const
{
  return showEncryptionResult;
}

KeyID
Module::selectSecretKey( const QString& title,
                         const QString& text,
                         const KeyID& keyId )
{
  if( 0 == pgp ) {
    assignPGPBase();
  }

  if( usePGP() ) {
    return selectKey( secretKeys(), title, text, keyId, SecretKeys );
  }
  else {
    KMessageBox::sorry( 0, i18n("You either do not have GnuPG/PGP installed "
                                "or you chose not to use GnuPG/PGP.") );
    return KeyID();
  }
}

KeyID
Module::selectPublicKey( const QString& title,
                         const QString& text /* = QString::null */,
                         const KeyID& oldKeyId /* = KeyID() */,
                         const QString& address /* = QString::null */,
                         const unsigned int allowedKeys /* = AllKeys */ )
{
  if( 0 == pgp ) {
    assignPGPBase();
  }

  if( usePGP() ) {
    KeyID keyId;

    if( address.isEmpty() ) {
      keyId = selectKey( publicKeys(), title, text, oldKeyId, allowedKeys );
    }
    else {
      bool rememberChoice;
      keyId = selectKey( rememberChoice, publicKeys(), title, text, oldKeyId,
                         allowedKeys );
      if( !keyId.isEmpty() && rememberChoice ) {
        setKeysForAddress( address, KeyIDList( keyId ) );
      }
    }

    return keyId;
  }
  else {
    KMessageBox::sorry( 0, i18n("You either do not have GnuPG/PGP installed "
                                "or you chose not to use GnuPG/PGP.") );
    return KeyID();
  }
}


KeyIDList
Module::selectPublicKeys( const QString& title,
                          const QString& text /* = QString::null */,
                          const KeyIDList& oldKeyIds /* = KeyIDList() */,
                          const QString& address /* = QString::null */,
                          const unsigned int allowedKeys /* = AllKeys */ )
{
  if( 0 == pgp ) {
    assignPGPBase();
  }

  if( usePGP() ) {
    KeyIDList keyIds;

    if( address.isEmpty() ) {
      keyIds = selectKeys( publicKeys(), title, text, oldKeyIds, allowedKeys );
    }
    else {
      bool rememberChoice;
      keyIds = selectKeys( rememberChoice, publicKeys(), title, text,
                           oldKeyIds, allowedKeys );
      if( !keyIds.isEmpty() && rememberChoice ) {
        setKeysForAddress( address, keyIds );
      }
    }

    return keyIds;
  }
  else {
    KMessageBox::sorry( 0, i18n("You either do not have GnuPG/PGP installed "
                                "or you chose not to use GnuPG/PGP.") );
    return KeyIDList();
  }
}


// -- static member functions ----------------------------------------------

Module *
Module::getKpgp()
{
  if (!kpgpObject)
  {
      kdError(5100) << "there is no instance of kpgp available" << endl;
  }
  return kpgpObject;
}


KConfig *
Module::getConfig()
{
  return getKpgp()->config;
}


bool
Module::prepareMessageForDecryption( const QCString& msg,
                                     QPtrList<Block>& pgpBlocks,
                                     QStrList& nonPgpBlocks )
{
  BlockType pgpBlock = NoPgpBlock;
  int start = -1;   // start of the current PGP block
  int lastEnd = -1; // end of the last PGP block

  pgpBlocks.setAutoDelete( true );
  pgpBlocks.clear();
  nonPgpBlocks.setAutoDelete( true );
  nonPgpBlocks.clear();

  if( msg.isEmpty() )
  {
    nonPgpBlocks.append( "" );
    return false;
  }

  if( !strncmp( msg.data(), "-----BEGIN PGP ", 15 ) )
    start = 0;
  else
  {
    start = msg.find( "\n-----BEGIN PGP" ) + 1;
    if( start == 0 )
    {
      nonPgpBlocks.append( msg );
      return false; // message doesn't contain an OpenPGP block
    }
  }

  while( start != -1 )
  {
    int nextEnd, nextStart;

    // is the PGP block a clearsigned block?
    if( !strncmp( msg.data() + start + 15, "SIGNED", 6 ) )
      pgpBlock = ClearsignedBlock;
    else
      pgpBlock = UnknownBlock;

    nextEnd = msg.find( "\n-----END PGP", start + 15 );
    if( nextEnd == -1 )
    {
      nonPgpBlocks.append( msg.mid( lastEnd+1 ) );
      break;
    }
    nextStart = msg.find( "\n-----BEGIN PGP", start + 15 );

    if( ( nextStart == -1 ) || ( nextEnd < nextStart ) ||
        ( pgpBlock == ClearsignedBlock ) )
    { // most likely we found a PGP block (but we don't check if it's valid)
      // store the preceding non-PGP block
      nonPgpBlocks.append( msg.mid( lastEnd+1, start-lastEnd-1 ) );
      lastEnd = msg.find( "\n", nextEnd + 14 );
      if( lastEnd == -1 )
      {
        pgpBlocks.append( new Block( msg.mid( start ) ) );
        nonPgpBlocks.append( "" );
        break;
      }
      else
      {
        pgpBlocks.append( new Block( msg.mid( start, lastEnd+1-start ) ) );
        if( ( nextStart != -1 ) && ( nextEnd > nextStart ) )
          nextStart = msg.find( "\n-----BEGIN PGP", lastEnd+1 );
      }
    }

    start = nextStart;
    if( start == -1 )
      nonPgpBlocks.append( msg.mid( lastEnd+1 ) );
    else
      start++; // move start behind the '\n'
  }

  return ( !pgpBlocks.isEmpty() );
}


// --------------------- private functions -------------------

bool
Module::haveTrustedEncryptionKey( const QString& person )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() ) return false;

  readPublicKeys();

  QString address = canonicalAddress( person ).lower();

  // First look for this person's address in the address data dictionary
  KeyIDList keyIds = keysForAddress( address );
  if( !keyIds.isEmpty() ) {
    // Check if at least one of the keys is a trusted and valid encryption key
    for( KeyIDList::ConstIterator it = keyIds.begin();
         it != keyIds.end(); ++it ) {
      keyTrust( *it ); // this is called to make sure that the trust info
                       // for this key is read
      Key *key = publicKey( *it );
      if( key && ( key->isValidEncryptionKey() ) &&
          ( key->keyTrust() >= KPGP_VALIDITY_MARGINAL ) )
        return true;
    }
  }

  // Now search the public keys for matching keys
  KeyListIterator it( mPublicKeys );

  // search a key which matches the complete address
  for( it.toFirst(); (*it); ++it ) {
    // search case insensitively in the list of userIDs of this key
    if( (*it)->matchesUserID( person, false ) ) {
      keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                         // the trust info for this key is read
      if( ( (*it)->isValidEncryptionKey() ) &&
          ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
        return true;
      }
    }
  }

  // if no key matches the complete address look for a key which matches
  // the canonical mail address
  for( it.toFirst(); (*it); ++it ) {
    // search case insensitively in the list of userIDs of this key
    if( (*it)->matchesUserID( address, false ) ) {
      keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                         // the trust info for this key is read
      if( ( (*it)->isValidEncryptionKey() ) &&
          ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
        return true;
      }
    }
  }

  // no trusted encryption key was found for the given person
  return false;
}

KeyIDList
Module::getEncryptionKeys( const QString& person )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() ) return KeyIDList();

  readPublicKeys();

  QString address = canonicalAddress( person ).lower();

  // #### FIXME: Until we support encryption with untrusted keys only
  // ####        trusted keys are allowed
  unsigned int allowedKeys = PublicKeys | EncryptionKeys | ValidKeys | TrustedKeys;
#if 0
  // ### reenable this code when we support encryption with untrusted keys
  if( pgpType != tGPG ) {
    // usage of untrusted keys is only possible with GnuPG
    allowedKeys |= TrustedKeys;
  }
#endif

  // First look for this person's address in the address->key dictionary
  KeyIDList keyIds = keysForAddress( address );
  if( !keyIds.isEmpty() ) {
    kdDebug(5100) << "Using encryption keys 0x"
                  << keyIds.toStringList().join( ", 0x" )
                  << " for " << person << endl;
    // Check if all of the keys are a trusted and valid encryption keys
    bool keysOk = true;
    for( KeyIDList::ConstIterator it = keyIds.begin();
         it != keyIds.end(); ++it ) {
      keyTrust( *it ); // this is called to make sure that the trust info
                       // for this key is read
      Key *key = publicKey( *it );
      if( !( key && ( key->isValidEncryptionKey() ) &&
             ( key->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) )
        keysOk = false;
    }
    if( keysOk ) {
      return keyIds;
    }
    else {
      bool rememberChoice;
      keyIds = selectKeys( rememberChoice, mPublicKeys,
                           i18n("Encryption Key Selection"),
                           i18n("if in your language something like "
                                "'key(s)' isn't possible please "
                                "use the plural in the translation",
                                "There is a problem with the "
                                "encryption key(s) for \"%1\".\n\n"
                                "Please re-select the key(s) which should "
                                "be used for this recipient."
                                ).arg(person),
                           keyIds,
                           allowedKeys );
      if( !keyIds.isEmpty() ) {
        if( rememberChoice ) {
          setKeysForAddress( person, keyIds );
        }
        return keyIds;
      }
    }
  }

  // Now search all public keys for matching keys
  KeyListIterator it( mPublicKeys );
  KeyList matchingKeys;

  // search all keys which match the complete address
  kdDebug(5100) << "Looking for keys matching " << person << " ...\n";
  for( it.toFirst(); (*it); ++it ) {
    // search case insensitively in the list of userIDs of this key
    if( (*it)->matchesUserID( person, false ) ) {
      keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                         // the trust info for this key is read
      if( ( (*it)->isValidEncryptionKey() ) &&
          ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
        kdDebug(5100) << "Matching trusted key found: "
                      << (*it)->primaryKeyID() << endl;
        matchingKeys.append( *it );
      }
    }
  }

  // if no keys match the complete address look for keys which match
  // the canonical mail address
  kdDebug(5100) << "Looking for keys matching " << address << " ...\n";
  if( matchingKeys.isEmpty() ) {
    for ( it.toFirst(); (*it); ++it ) {
      // search case insensitively in the list of userIDs of this key
      if( (*it)->matchesUserID( address, false ) ) {
        keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                           // the trust info for this key is read
        if( ( (*it)->isValidEncryptionKey() ) &&
            ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
          kdDebug(5100) << "Matching trusted key found: "
                        << (*it)->primaryKeyID() << endl;
          matchingKeys.append( *it );
        }
      }
    }
  }

  // no match until now, let the user choose the key
  if( matchingKeys.isEmpty() ) {
    // FIXME: let user get the key from keyserver
    bool rememberChoice;
    KeyIDList keyIds = selectKeys( rememberChoice, mPublicKeys,
                                   i18n("Encryption Key Selection"),
                                   i18n("if in your language something like "
                                        "'key(s)' isn't possible please "
                                        "use the plural in the translation",
                                        "No valid and trusted OpenPGP key was "
                                        "found for \"%1\".\n\n"
                                        "Select the key(s) which should "
                                        "be used for this recipient."
                                        ).arg(person),
                                   KeyIDList(),
                                   allowedKeys );
    if( !keyIds.isEmpty() ) {
      if( rememberChoice ) {
        setKeysForAddress( person, keyIds );
      }
      return keyIds;
    }
  }
  // only one key matches
  else if( matchingKeys.count() == 1 ) {
    return KeyIDList( matchingKeys.getFirst()->primaryKeyID() );
  }
  // more than one key matches; let the user choose the key(s)
  else {
    bool rememberChoice;
    KeyIDList keyIds = selectKeys( rememberChoice, matchingKeys,
                                   i18n("Encryption Key Selection"),
                                   i18n("if in your language something like "
                                        "'key(s)' isn't possible please "
                                        "use the plural in the translation",
                                        "More than one key matches \"%1\".\n\n"
                                        "Select the key(s) which should "
                                        "be used for this recipient."
                                        ).arg(person),
                                   KeyIDList(),
                                   allowedKeys );
    if( !keyIds.isEmpty() ) {
      if( rememberChoice ) {
        setKeysForAddress( person, keyIds );
      }
      return keyIds;
    }
  }

  return KeyIDList();
}

// check if pgp 2.6.x or 5.0 is installed
// kpgp will prefer to user pgp 5.0
bool
Module::checkForPGP(void)
{
  // get path
  QCString path;
  QStrList pSearchPaths;
  int index = 0;
  int lastindex = -1;

  havePgp=FALSE;

  path = getenv("PATH");
  while((index = path.find(":",lastindex+1)) != -1)
  {
    pSearchPaths.append(path.mid(lastindex+1,index-lastindex-1));
    lastindex = index;
  }
  if(lastindex != (int)path.length() - 1)
    pSearchPaths.append( path.mid(lastindex+1,path.length()-lastindex) );

  QStrListIterator it(pSearchPaths);

  haveGpg=FALSE;
  // lets try gpg

  for ( it.toFirst() ; it.current() ; ++it )
  {
    path = (*it);
    path += "/gpg";
    if ( !access( path, X_OK ) )
    {
      kdDebug(5100) << "Kpgp: gpg found" << endl;
      havePgp=TRUE;
      haveGpg=TRUE;
      break;
    }
  }

  // search for pgp5.0
  havePGP5=FALSE;
  for ( it.toFirst() ; it.current() ; ++it )
  {
    path = (*it);
    path += "/pgpe";
    if ( !access( path, X_OK ) )
    {
      kdDebug(5100) << "Kpgp: pgp 5 found" << endl;
      havePgp=TRUE;
      havePGP5=TRUE;
      break;
    }
  }

  // lets try pgp2.6.x
  if (!havePgp) {
    for ( it.toFirst() ; it.current() ; ++it )
    {
      path = it.current();
      path += "/pgp";
      if ( !access( path, X_OK ) )
      {
	kdDebug(5100) << "Kpgp: pgp 2 or 6 found" << endl;
	havePgp=TRUE;
	break;
      }
    }
  }

  if (!havePgp)
  {
    kdDebug(5100) << "Kpgp: no pgp found" << endl;
  }

  return havePgp;
}

void
Module::assignPGPBase(void)
{
  if (pgp)
    delete pgp;

  if(havePgp)
  {
    switch (pgpType)
    {
      case tGPG:
        kdDebug(5100) << "Kpgp: assign pgp - gpg" << endl;
        pgp = new BaseG();
        break;

      case tPGP2:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 2" << endl;
        pgp = new Base2();
        break;

      case tPGP5:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 5" << endl;
        pgp = new Base5();
        break;

      case tPGP6:
        kdDebug(5100) << "Kpgp: assign pgp - pgp 6" << endl;
        pgp = new Base6();
        break;

      case tOff:
        // dummy handler
        kdDebug(5100) << "Kpgp: pgpBase is dummy " << endl;
        pgp = new Base();
        break;

      case tAuto:
        kdDebug(5100) << "Kpgp: assign pgp - auto" << endl;
      default:
        kdDebug(5100) << "Kpgp: assign pgp - default" << endl;
        if (haveGpg)
        {
          kdDebug(5100) << "Kpgp: pgpBase is gpg " << endl;
          pgp = new BaseG();
          pgpType = tGPG;
        }
        else if(havePGP5)
        {
          kdDebug(5100) << "Kpgp: pgpBase is pgp 5" << endl;
          pgp = new Base5();
          pgpType = tPGP5;
        }
        else
        {
          Base6 *pgp_v6 = new Base6();
          if (!pgp_v6->isVersion6())
          {
            kdDebug(5100) << "Kpgp: pgpBase is pgp 2 " << endl;
            delete pgp_v6;
            pgp = new Base2();
            pgpType = tPGP2;
          }
          else
          {
            kdDebug(5100) << "Kpgp: pgpBase is pgp 6 " << endl;
            pgp = pgp_v6;
            pgpType = tPGP6;
          }
        }
    } // switch
  }
  else
  {
    // dummy handler
    kdDebug(5100) << "Kpgp: pgpBase is dummy " << endl;
    pgp = new Base();
    pgpType = tOff;
  }
}

QString
Module::canonicalAddress( const QString& _adress )
{
  int index,index2;

  QString address = _adress.simplifyWhiteSpace();
  address = address.stripWhiteSpace();

  // just leave pure e-mail address.
  if((index = address.find("<")) != -1)
    if((index2 = address.find("@",index+1)) != -1)
      if((index2 = address.find(">",index2+1)) != -1)
	return address.mid(index,index2-index+1);

  if((index = address.find("@")) == -1)
  {
    // local address
    //char hostname[1024];
    //gethostname(hostname,1024);
    //return "<" + address + "@" + hostname + ">";
    return "<" + address + "@localdomain>";
  }
  else
  {
    int index1 = address.findRev(" ",index);
    int index2 = address.find(" ",index);
    if(index2 == -1) index2 = address.length();
    return "<" + address.mid(index1+1 ,index2-index1-1) + ">";
  }
}

void
Module::readPublicKeys( bool reread )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() )
  {
    mPublicKeys.clear();
    mPublicKeysCached = false;
    return;
  }

  if( !mPublicKeysCached || reread )
  {
    if( mPublicKeys.isEmpty() )
    {
      mPublicKeys = pgp->publicKeys();
    }
    else
    {
      KeyList newPublicKeyList = pgp->publicKeys();

      // merge the trust info from the old key list into the new key list
      // FIXME: This is currently O(K^2) where K = #keys. As the key lists
      //        are sorted this can be done in O(K).
      KeyListIterator it( newPublicKeyList );
      for( it.toFirst(); (*it); ++it )
      {
        Key* oldKey = publicKey( (*it)->primaryKeyID() );
        if( oldKey )
        {
          (*it)->cloneKeyTrust( oldKey );
        }
      }

      mPublicKeys = newPublicKeyList;
    }

    mPublicKeysCached = true;
    mPublicKeys.setAutoDelete( true );
  }
}

void
Module::readSecretKeys( bool reread )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() )
  {
    mSecretKeys.clear();
    mSecretKeysCached = false;
    return;
  }

  if( mSecretKeys.isEmpty() || reread )
  {
    if( mSecretKeys.isEmpty() )
    {
      mSecretKeys = pgp->secretKeys();
    }
    else
    {
      KeyList newSecretKeyList = pgp->secretKeys();

      // merge the trust info from the old key list into the new key list
      // FIXME: This is currently O(K^2) where K = #keys. As the key lists
      //        are sorted this can be done in O(K).
      KeyListIterator it( newSecretKeyList );
      for( it.toFirst(); (*it); ++it )
      {
        Key* oldKey = secretKey( (*it)->primaryKeyID() );
        if( oldKey )
        {
          (*it)->cloneKeyTrust( oldKey );
        }
      }

      mSecretKeys = newSecretKeyList;
    }

    mSecretKeysCached = true;
    mSecretKeys.setAutoDelete( true );
  }
}

KeyID
Module::selectKey( const KeyList& keys,
                   const QString& title,
                   const QString& text /* = QString::null */ ,
                   const KeyID& keyId /* = KeyID() */ ,
                   const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyID retval = KeyID();

  KeySelectionDialog dlg( keys, title, text, KeyIDList( keyId ), false,
                          allowedKeys, false );

  QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
  bool rej = ( dlg.exec() == QDialog::Rejected );
  QApplication::restoreOverrideCursor();

  if( !rej ) {
    retval = dlg.key();
  }

  return retval;
}

KeyIDList
Module::selectKeys( const KeyList& keys,
                    const QString& title,
                    const QString& text /* = QString::null */ ,
                    const KeyIDList& keyIds /* = KeyIDList() */ ,
                    const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyIDList retval = KeyIDList();

  KeySelectionDialog dlg( keys, title, text, keyIds, false, allowedKeys,
                          true );

  QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
  bool rej = ( dlg.exec() == QDialog::Rejected );
  QApplication::restoreOverrideCursor();

  if( !rej ) {
    retval = dlg.keys();
  }

  return retval;
}


KeyID
Module::selectKey( bool& rememberChoice,
                   const KeyList& keys,
                   const QString& title,
                   const QString& text /* = QString::null */ ,
                   const KeyID& keyId /* = KeyID() */ ,
                   const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyID retval = KeyID();

  KeySelectionDialog dlg( keys, title, text, KeyIDList( keyId ), false,
                          allowedKeys, false );

  QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
  bool rej = ( dlg.exec() == QDialog::Rejected );
  QApplication::restoreOverrideCursor();

  if( !rej ) {
    retval = dlg.key();
    rememberChoice = dlg.rememberSelection();
  }
  else {
    rememberChoice = false;
  }

  return retval;
}

KeyIDList
Module::selectKeys( bool& rememberChoice,
                    const KeyList& keys,
                    const QString& title,
                    const QString& text /* = QString::null */ ,
                    const KeyIDList& keyIds /* = KeyIDList() */ ,
                    const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyIDList retval = KeyIDList();

  KeySelectionDialog dlg( keys, title, text, keyIds, true, allowedKeys,
                          true );

  QApplication::setOverrideCursor( QCursor(QCursor::ArrowCursor) );
  bool rej = ( dlg.exec() == QDialog::Rejected );
  QApplication::restoreOverrideCursor();

  if( !rej ) {
    retval = dlg.keys();
    rememberChoice = dlg.rememberSelection();
  }
  else {
    rememberChoice = false;
  }

  return retval;
}

KeyIDList
Module::keysForAddress( const QString& address )
{
  if( address.isEmpty() ) {
    return KeyIDList();
  }
  QString addr = canonicalAddress( address ).lower();
  if( addressDataDict.contains( addr ) ) {
    return addressDataDict[addr].keyIds;
  }
  else {
    return KeyIDList();
  }
}

void
Module::setKeysForAddress( const QString& address, const KeyIDList& keyIds )
{
  if( address.isEmpty() ) {
    return;
  }
  QString addr = canonicalAddress( address ).lower();
  if( addressDataDict.contains( addr ) ) {
    addressDataDict[addr].keyIds = keyIds;
  }
  else {
    AddressData data;
    data.encrPref = UnknownEncryptPref;
    data.keyIds = keyIds;
    addressDataDict.insert( addr, data );
  }

  //writeAddressData();
}

void
Module::readAddressData()
{
  QString address;
  AddressData data;

  KConfigGroup general( config, "General" );
  int num = general.readNumEntry( "addressEntries", 0 );

  addressDataDict.clear();
  for( int i=1; i<=num; i++ ) {
    KConfigGroup addrGroup( config, QString("Address #%1").arg(i).local8Bit() );
    address = addrGroup.readEntry( "Address" );
    data.keyIds = KeyIDList::fromStringList( addrGroup.readListEntry( "Key IDs" ) );
    data.encrPref = (EncryptPref) addrGroup.readNumEntry( "EncryptionPreference",
                                                          UnknownEncryptPref );
//     kdDebug(5100) << "Read address " << i << ": " << address
//                   << "\nKey IDs: 0x" << data.keyIds.toStringList().join(", 0x")
//                   << "\nEncryption preference: " << data.encrPref << endl;
    if ( !address.isEmpty() ) {
      addressDataDict.insert( address, data );
    }
  }
}

void
Module::writeAddressData()
{
  KConfigGroup general( config, "General" );
  general.writeEntry( "addressEntries", addressDataDict.count() );

  int i;
  AddressDataDict::Iterator it;
  for ( i=1, it = addressDataDict.begin();
        it != addressDataDict.end();
        ++it, i++ ) {
    KConfigGroup addrGroup( config, QString("Address #%1").arg(i).local8Bit() );
    addrGroup.writeEntry( "Address", it.key() );
    addrGroup.writeEntry( "Key IDs", it.data().keyIds.toStringList() );
    addrGroup.writeEntry( "EncryptionPreference", it.data().encrPref );
  }

  config->sync();
}

EncryptPref
Module::encryptionPreference( const QString& address )
{
  QString addr = canonicalAddress( address ).lower();
  if( addressDataDict.contains( addr ) ) {
    return addressDataDict[addr].encrPref;
  }
  else {
    return UnknownEncryptPref;
  }
}

void
Module::setEncryptionPreference( const QString& address,
                                 const EncryptPref pref )
{
  if( address.isEmpty() ) {
    return;
  }
  QString addr = canonicalAddress( address ).lower();
  if( addressDataDict.contains( addr ) ) {
    addressDataDict[addr].encrPref = pref;
  }
  else {
    AddressData data;
    data.encrPref = pref;
    addressDataDict.insert( addr, data );
  }
}

} // namespace Kpgp
