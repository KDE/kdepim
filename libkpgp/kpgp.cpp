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
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "kpgp.h"
#include "kpgpbase.h"
#include "kpgpui.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfigbase.h>
#include <kconfiggroup.h>
#include <kconfig.h>
#include <kde_file.h>

#include <QLabel>
#include <QCursor>
#include <QApplication>
#include <QByteArray>
#include <QFileInfo>
#include <KGlobal>

#include <algorithm>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifdef ERROR
#undef ERROR
#endif

namespace Kpgp {

struct ModuleStatic {
  ModuleStatic() : kpgpObject( 0 ) {}
  ~ModuleStatic() { delete kpgpObject; }
  Module* kpgpObject;
};

K_GLOBAL_STATIC( ModuleStatic, s_module )

Module::Module()
  : mPublicKeys(),
    mPublicKeysCached(false),
    mSecretKeys(),
    mSecretKeysCached(false),
    passphrase(0), passphrase_buffer_len(0), havePassPhrase(false)
{
  pgp = 0;

  config = new KConfig(QLatin1String("kpgprc"));

  init();
}

Module::~Module()
{
  writeAddressData();

  if (!s_module.isDestroyed() && s_module->kpgpObject == this)
    s_module->kpgpObject = 0;
  clear(true);
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
  KConfigGroup grp(config, QString());
  storePass = grp.readEntry("storePass", false);
  showEncryptionResult = grp.readEntry("showEncryptionResult", true);
  mShowKeyApprovalDlg = grp.readEntry( "showKeysForApproval", true );
  // We have no config GUI for this key anymore, and the KPGP backend isn't ported,
  // so let's just use Auto all the time.  See #92619.
  ///pgpType = (Module::PGPType) config->readEntry("pgpType", tAuto);
  pgpType = tAuto;
  flagEncryptToSelf = grp.readEntry("encryptToSelf", true);
}

void
Module::writeConfig(bool sync)
{
  KConfigGroup grp(config, QString());
  grp.writeEntry("storePass", storePass);
  grp.writeEntry("showEncryptionResult", showEncryptionResult);
  grp.writeEntry( "showKeysForApproval", mShowKeyApprovalDlg );
  //config->writeEntry("pgpType", (int) pgpType);
  grp.writeEntry("encryptToSelf", flagEncryptToSelf);

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
      kDebug( 5326 ) <<"user uses gpg-agent -> don't ask for passphrase";
      // set dummy passphrase (because else signing doesn't work -> FIXME)
      setPassPhrase( QLatin1String("dummy") );
    }
    else {
      QString ID;
      if( block )
        ID = block->requiredUserId();
      PassphraseDialog passdlg(0, i18n("OpenPGP Security Check"), ID);
#ifndef QT_NO_CURSOR
      QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
      int passdlgResult = passdlg.exec();
#ifndef QT_NO_CURSOR
      QApplication::restoreOverrideCursor();
#endif
      if (passdlgResult == QDialog::Accepted) {
        if (!setPassPhrase(passdlg.passphrase())) {
          if ( passdlg.passphrase().length() >= 1024)
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
      kDebug( 5326 ) <<"wipePassPhrase: passphrase && !passphrase_buffer_len ???";
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

  if(retval & Kpgp::ERROR)
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
    // loop as long as the user enters a wrong passphrase and doesn't abort
    // everything ready
    if( prepare( true, &block ) != 1 )
      return false;
    // ok now try to decrypt the message.
    retval = pgp->decrypt( block, passphrase );
    // loop on bad passphrase
    if( retval & BADPHRASE ) {
      wipePassPhrase();
#ifndef QT_NO_CURSOR
      QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
      int ret = KMessageBox::warningContinueCancel(0,
             i18n("You just entered an invalid passphrase.\n"
                  "Do you want to try again, or "
                  "cancel and view the message undecrypted?"),
             i18n("PGP Warning"), KGuiItem(i18n("&Retry")));
#ifndef QT_NO_CURSOR
      QApplication::restoreOverrideCursor();
#endif
      if ( ret == KMessageBox::Cancel ) break;
    } else
      break;
  } while ( true );

  // erase the passphrase if we do not want to keep it
  cleanupPass();

  if(retval & Kpgp::ERROR)
  {
    errMsg = pgp->lastErrorMessage();
    return false;
  }
  return true;
}

Kpgp::Result
Module::clearsign( Block& block,
                   const KeyID& keyId, const QByteArray& charset )
{
  return encrypt( block, QStringList(), keyId, true, charset );
}

Kpgp::Result
Module::encrypt( Block& block,
                 const QStringList& receivers, const KeyID& keyId,
                 bool sign, const QByteArray& charset )
{
  KeyIDList encryptionKeyIds; // list of keys which are used for encryption
  int status = 0;
  errMsg = QLatin1String("");

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
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               KGuiItem(i18n("&Retry")),
                                               KGuiItem(i18n("Send &Unsigned")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
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
    QString str = i18nc("%1 = 'signing failed' error message",
                       "%1\nDo you want to send the message unsigned, "
                       "or cancel sending the message?",
                    pgp->lastErrorMessage() );
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  KGuiItem(i18n("Send &Unsigned")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    sign = false;
    status = doEncSign( block, encryptionKeyIds, sign );
  }

  // check for bad keys
  if( status & BADKEYS ) {
    QString str = i18nc("%1 = 'bad keys' error message",
                       "%1\nDo you want to encrypt anyway, leave the "
                       "message as-is, or cancel sending the message?",
                    pgp->lastErrorMessage() );

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               KGuiItem(i18n("Send &Encrypted")),
                                               KGuiItem(i18n("Send &Unencrypted")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
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
    QString str = i18nc("%1 = 'missing keys' error message",
                       "%1\nDo you want to leave the message as-is, "
                       "or cancel sending the message?",
                    pgp->lastErrorMessage() );
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  KGuiItem(i18n("&Send As-Is")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    if( ret == KMessageBox::Cancel ) {
      return Kpgp::Canceled;
    }
    block.reset();
    return Kpgp::Ok;
  }

  if( status & Kpgp::ERROR ) {
    // show error dialog
    errMsg = i18n( "The following error occurred:\n%1" ,
               pgp->lastErrorMessage() );
    QString details = i18n( "This is the error message of %1:\n%2" ,
                        ( pgpType == tGPG ) ? QLatin1String("GnuPG") : QLatin1String("PGP") ,
                        QLatin1String(block.error().data()) );
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    KMessageBox::detailedSorry( 0, errMsg, details );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
    return Kpgp::Failure;
  }

  if( showCipherText() ) {
    // show cipher text dialog
    CipherTextDialog *cipherTextDlg = new CipherTextDialog( block.text(), charset );
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    bool result = ( cipherTextDlg->exec() == QDialog::Accepted );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
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
        return Kpgp::ERROR;
    }
    retval = pgp->encsign( block, recipientKeyIds, passphrase );
  }
  else {
    if( !prepare( false, &block ) ) return Kpgp::ERROR;
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
  QVector<KeyIDList> recipientKeyIds( recipients.count() + 1 );
  // add the sender's encryption key(s) to the list of recipient key IDs
  if( encryptToSelf() ) {
    recipientKeyIds[0] = KeyIDList( keyId );
  }
  else {
    recipientKeyIds[0] = KeyIDList();
  }
  bool showKeysForApproval = false;
  int i = 1;
  for( QStringList::ConstIterator it = recipients.constBegin();
       it != recipients.constEnd(); ++it, ++i ) {
    EncryptPref encrPref = encryptionPreference( *it );
    if( ( encrPref == UnknownEncryptPref ) || ( encrPref == NeverEncrypt ) )
      showKeysForApproval = true;

    KeyIDList keyIds = getEncryptionKeys( *it );
    if( keyIds.isEmpty() ) {
      showKeysForApproval = true;
    }
    recipientKeyIds[i] = keyIds;
  }

  kDebug( 5326 ) <<"recipientKeyIds = (";
  QVector<KeyIDList>::const_iterator kit;
  for( kit = recipientKeyIds.constBegin(); kit != recipientKeyIds.constEnd(); ++kit ) {
    kDebug( 5326 ) <<"( 0x" << (*kit).toStringList().join(QLatin1String(", 0x") )
                  << " ),\n";
  }
  kDebug( 5326 ) <<")";

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

#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = dlg.exec();

    if( ret == QDialog::Rejected ) {
#ifndef QT_NO_CURSOR
        QApplication::restoreOverrideCursor();
#endif
        return Kpgp::Canceled;
    }

    recipientKeyIds = dlg.keys();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
  }

  // flatten the list of lists of key IDs and count empty key ID lists
  int emptyListCount = 0;
  for( QVector<KeyIDList>::const_iterator it = recipientKeyIds.constBegin();
       it != recipientKeyIds.constEnd(); ++it ) {
    if( (*it).isEmpty() ) {
      // only count empty key ID lists for the recipients
      if( it != recipientKeyIds.constBegin() ) {
        emptyListCount++;
      }
    }
    else {
      for( KeyIDList::ConstIterator kit = (*it).constBegin();
           kit != (*it).constEnd(); kit++ ) {
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
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningContinueCancel( 0, str,
                                                  i18n("PGP Warning"),
                                                  KGuiItem(i18n("Send &Unencrypted")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
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
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
    int ret = KMessageBox::warningYesNoCancel( 0, str,
                                               i18n("PGP Warning"),
                                               KGuiItem(i18n("Send &Encrypted")),
                                               KGuiItem(i18n("Send &Unencrypted")) );
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif
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
  for( QStringList::ConstIterator it = recipients.constBegin();
       it != recipients.constEnd(); ++it) {
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
    return false;
  if(pgp->signKey(keyId, passphrase) & Kpgp::ERROR)
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

  foreach ( Key* key, mPublicKeys ) {
    if( keyID == key->primaryKeyID() ||
        keyID == key->primaryFingerprint() )
      return key;
  }

  return 0;
}

Key*
Module::publicKey( const QString& userID )
{
  readPublicKeys();

  foreach ( Key* key, mPublicKeys ) {
    if( key->matchesUserID( userID ) )
      return key;
  }

  return 0;
}

Key*
Module::secretKey(const KeyID& keyID)
{
  readSecretKeys();

  foreach ( Key* key, mSecretKeys ) {
    if( keyID == key->primaryKeyID() ||
        keyID == key->primaryFingerprint() )
      return key;
  }

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
    KeyList::Iterator it = std::lower_bound( mPublicKeys.begin(), mPublicKeys.end(), newKey, KeyCompare );
    mPublicKeys.insert( it, newKey );
    kDebug( 5326 ) <<"New public key 0x" << newKey->primaryKeyID() <<" ("
                  << newKey->primaryUserID() << ").\n";
  }
  else if( ( 0 != oldKey ) && ( 0 == newKey ) )
  { // the key has been deleted in the meantime
    kDebug( 5326 ) <<"Public key 0x" << oldKey->primaryKeyID() <<" ("
                  << oldKey->primaryUserID() << ") will be removed.\n";
    mPublicKeys.removeAll( oldKey );
  }

  return newKey;
}

QByteArray
Module::getAsciiPublicKey(const KeyID& keyID)
{
  if (0 == pgp) assignPGPBase();

  return pgp->getAsciiPublicKey(keyID);
}


bool Module::setPassPhrase(const QString& aPass)
{
  // null out old buffer before we touch the new string.  So in case
  // aPass isn't properly null-terminated, we don't leak secret data.
  wipePassPhrase();

  if (!aPass.isNull())
  {
    size_t newlen = aPass.length();
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
    memcpy( passphrase, aPass.toLocal8Bit().data(), newlen + 1 );
    havePassPhrase = true;
  }
  return true;
}

bool
Module::changePassPhrase()
{
  //FIXME...
  KMessageBox::information(0,i18n("This feature is\nstill missing"));
  return false;
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
                         const QString& text /*QString() */,
                         const KeyID& oldKeyId /* = KeyID() */,
                         const QString& address /*QString() */,
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
                          const QString& text /* = QString() */,
                          const KeyIDList& oldKeyIds /* = KeyIDList() */,
                          const QString& address /*= QString() */,
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
  if (!s_module->kpgpObject)
  {
    s_module->kpgpObject = new Module();
  }
  return s_module->kpgpObject;
}


KConfig *
Module::getConfig()
{
  return getKpgp()->config;
}


bool
Module::prepareMessageForDecryption( const QByteArray& msg,
                                     QList<Block>& pgpBlocks,
                                     QList<QByteArray>& nonPgpBlocks )
{
  BlockType pgpBlock = NoPgpBlock;
  int start = -1;   // start of the current PGP block
  int lastEnd = -1; // end of the last PGP block

  pgpBlocks.clear();
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
    start = msg.indexOf( "\n-----BEGIN PGP" ) + 1;
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

    nextEnd = msg.indexOf( "\n-----END PGP", start + 15 );
    if( nextEnd == -1 )
    {
      nonPgpBlocks.append( msg.mid( lastEnd+1 ) );
      break;
    }
    nextStart = msg.indexOf( "\n-----BEGIN PGP", start + 15 );

    if( ( nextStart == -1 ) || ( nextEnd < nextStart ) ||
        ( pgpBlock == ClearsignedBlock ) )
    { // most likely we found a PGP block (but we don't check if it's valid)
      // store the preceding non-PGP block
      nonPgpBlocks.append( msg.mid( lastEnd+1, start-lastEnd-1 ) );
      lastEnd = msg.indexOf( "\n", nextEnd + 14 );
      if( lastEnd == -1 )
      {
        pgpBlocks.append( Block( msg.mid( start ) ) );
        nonPgpBlocks.append( "" );
        break;
      }
      else
      {
        pgpBlocks.append( Block( msg.mid( start, lastEnd+1-start ) ) );
        if( ( nextStart != -1 ) && ( nextEnd > nextStart ) )
          nextStart = msg.indexOf( "\n-----BEGIN PGP", lastEnd+1 );
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

  QString address = canonicalAddress( person ).toLower();

  // First look for this person's address in the address data dictionary
  KeyIDList keyIds = keysForAddress( address );
  if( !keyIds.isEmpty() ) {
    // Check if at least one of the keys is a trusted and valid encryption key
    for( KeyIDList::ConstIterator it = keyIds.constBegin();
         it != keyIds.constEnd(); ++it ) {
      keyTrust( *it ); // this is called to make sure that the trust info
                       // for this key is read
      Key *key = publicKey( *it );
      if( key && ( key->isValidEncryptionKey() ) &&
          ( key->keyTrust() >= KPGP_VALIDITY_MARGINAL ) )
        return true;
    }
  }

  // Now search the public keys for matching keys
  KeyList::Iterator it = mPublicKeys.begin();

  // search a key which matches the complete address
  for(; it != mPublicKeys.end(); ++it ) {
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
  for( it = mPublicKeys.begin(); it != mPublicKeys.end(); ++it ) {
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

  QString address = canonicalAddress( person ).toLower();

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
    kDebug( 5326 ) <<"Using encryption keys 0x"
                  << keyIds.toStringList().join( QLatin1String(", 0x") )
                  << "for" << person;
    // Check if all of the keys are a trusted and valid encryption keys
    bool keysOk = true;
    for( KeyIDList::ConstIterator it = keyIds.constBegin();
         it != keyIds.constEnd(); ++it ) {
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
                           i18nc("if in your language something like "
                                "'key(s)' isn't possible please "
                                "use the plural in the translation",
                                "There is a problem with the "
                                "encryption key(s) for \"%1\".\n\n"
                                "Please re-select the key(s) which should "
                                "be used for this recipient."
                                , person),
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
  KeyList::Iterator it = mPublicKeys.begin();
  KeyList matchingKeys;

  // search all keys which match the complete address
  kDebug( 5326 ) <<"Looking for keys matching" << person <<" ...";
  for( ; it != mPublicKeys.end(); ++it ) {
    // search case insensitively in the list of userIDs of this key
    if( (*it)->matchesUserID( person, false ) ) {
      keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                         // the trust info for this key is read
      if( ( (*it)->isValidEncryptionKey() ) &&
          ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
        kDebug( 5326 ) <<"Matching trusted key found:"
                      << (*it)->primaryKeyID();
        matchingKeys.append( *it );
      }
    }
  }

  // if no keys match the complete address look for keys which match
  // the canonical mail address
  kDebug( 5326 ) <<"Looking for keys matching" << address <<" ...";
  if( matchingKeys.isEmpty() ) {
    for ( it = mPublicKeys.begin(); it != mPublicKeys.end(); ++it ) {
      // search case insensitively in the list of userIDs of this key
      if( (*it)->matchesUserID( address, false ) ) {
        keyTrust( (*it)->primaryKeyID() ); // this is called to make sure that
                                           // the trust info for this key is read
        if( ( (*it)->isValidEncryptionKey() ) &&
            ( (*it)->keyTrust() >= KPGP_VALIDITY_MARGINAL ) ) {
          kDebug( 5326 ) <<"Matching trusted key found:"
                        << (*it)->primaryKeyID();
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
                                   i18nc("if in your language something like "
                                        "'key(s)' isn't possible please "
                                        "use the plural in the translation",
                                        "No valid and trusted OpenPGP key was "
                                        "found for \"%1\".\n\n"
                                        "Select the key(s) which should "
                                        "be used for this recipient."
                                        , person),
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
    return KeyIDList( matchingKeys.first()->primaryKeyID() );
  }
  // more than one key matches; let the user choose the key(s)
  else {
    bool rememberChoice;
    KeyIDList keyIds = selectKeys( rememberChoice, matchingKeys,
                                   i18n("Encryption Key Selection"),
                                   i18nc("if in your language something like "
                                        "'key(s)' isn't possible please "
                                        "use the plural in the translation",
                                        "More than one key matches \"%1\".\n\n"
                                        "Select the key(s) which should "
                                        "be used for this recipient."
                                        , person),
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
  QString path;
  QStringList pSearchPaths;

  havePgp=false;

  path = QString::fromLocal8Bit( getenv("PATH") );
  pSearchPaths = path.split( QLatin1Char(KPATH_SEPARATOR), QString::SkipEmptyParts );

  haveGpg=false;
  // lets try gpg

  foreach( const QString& curPath, pSearchPaths )
  {
    path = curPath;
    path += QLatin1String("/gpg");
    if ( QFileInfo(path).isExecutable() )
    {
      kDebug( 5326 ) <<"Kpgp: gpg found";
      havePgp=true;
      haveGpg=true;
      break;
    }
  }

  // search for pgp5.0
  havePGP5=false;
  foreach( const QString& curPath, pSearchPaths )
  {
    path = curPath;
    path += QLatin1String("/pgpe");
    if ( QFileInfo(path).isExecutable() )
    {
      kDebug( 5326 ) <<"Kpgp: pgp 5 found";
      havePgp=true;
      havePGP5=true;
      break;
    }
  }

  // lets try pgp2.6.x
  if (!havePgp) {
    foreach( const QString& curPath, pSearchPaths )
    {
      path = curPath;
      path += QLatin1String("/pgp");
      if ( QFileInfo(path).isExecutable() )
      {
        kDebug( 5326 ) <<"Kpgp: pgp 2 or 6 found";
        havePgp=true;
        break;
      }
    }
  }

  if (!havePgp)
  {
    kDebug( 5326 ) <<"Kpgp: no pgp found";
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
        kDebug( 5326 ) <<"Kpgp: assign pgp - gpg";
        pgp = new BaseG();
        break;

      case tPGP2:
        kDebug( 5326 ) <<"Kpgp: assign pgp - pgp 2";
        pgp = new Base2();
        break;

      case tPGP5:
        kDebug( 5326 ) <<"Kpgp: assign pgp - pgp 5";
        pgp = new Base5();
        break;

      case tPGP6:
        kDebug( 5326 ) <<"Kpgp: assign pgp - pgp 6";
        pgp = new Base6();
        break;

      case tOff:
        // dummy handler
        kDebug( 5326 ) <<"Kpgp: pgpBase is dummy";
        pgp = new Base();
        break;

      case tAuto:
        kDebug( 5326 ) <<"Kpgp: assign pgp - auto";
        // fall through
      default:
        kDebug( 5326 ) <<"Kpgp: assign pgp - default";
        if (haveGpg)
        {
          kDebug( 5326 ) <<"Kpgp: pgpBase is gpg";
          pgp = new BaseG();
          pgpType = tGPG;
        }
        else if(havePGP5)
        {
          kDebug( 5326 ) <<"Kpgp: pgpBase is pgp 5";
          pgp = new Base5();
          pgpType = tPGP5;
        }
        else
        {
          Base6 *pgp_v6 = new Base6();
          if (!pgp_v6->isVersion6())
          {
            kDebug( 5326 ) <<"Kpgp: pgpBase is pgp 2";
            delete pgp_v6;
            pgp = new Base2();
            pgpType = tPGP2;
          }
          else
          {
            kDebug( 5326 ) <<"Kpgp: pgpBase is pgp 6";
            pgp = pgp_v6;
            pgpType = tPGP6;
          }
        }
    } // switch
  }
  else
  {
    // dummy handler
    kDebug( 5326 ) <<"Kpgp: pgpBase is dummy";
    pgp = new Base();
    pgpType = tOff;
  }
}

QString
Module::canonicalAddress( const QString& _adress )
{
  int index,index2;

  QString address = _adress.simplified();
  address = address.trimmed();

  // just leave pure e-mail address.
  if((index = address.indexOf(QLatin1String("<"))) != -1)
    if((index2 = address.indexOf(QLatin1String("@"),index+1)) != -1)
      if((index2 = address.indexOf(QLatin1String(">"),index2+1)) != -1)
        return address.mid(index,index2-index+1);

  if((index = address.indexOf(QLatin1String("@"))) == -1)
  {
    // local address
    //char hostname[1024];
    //gethostname(hostname,1024);
    //return "<" + address + "@" + hostname + ">";
    return QLatin1Char('<') + address + QLatin1String("@localdomain>");
  }
  else
  {
    int index1 = address.lastIndexOf(QLatin1String(" "),index);
    int index2 = address.indexOf(QLatin1String(" "),index);
    if(index2 == -1) index2 = address.length();
    return QLatin1Char('<') + address.mid(index1+1 ,index2-index1-1) + QLatin1Char('>');
  }
}

void
Module::readPublicKeys( bool reread )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() )
  {
    qDeleteAll( mPublicKeys );
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
      for( KeyList::Iterator it = newPublicKeyList.begin(); it != newPublicKeyList.end(); ++it )
      {
        Key* oldKey = publicKey( (*it)->primaryKeyID() );
        if( oldKey )
        {
          (*it)->cloneKeyTrust( oldKey );
        }
      }

      qDeleteAll( mPublicKeys );
      mPublicKeys = newPublicKeyList;
    }

    mPublicKeysCached = true;
  }
}

void
Module::readSecretKeys( bool reread )
{
  if( 0 == pgp ) assignPGPBase();

  if( !usePGP() )
  {
    qDeleteAll( mSecretKeys );
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
      for( KeyList::Iterator it = newSecretKeyList.begin(); it != newSecretKeyList.end(); ++it )
      {
        Key* oldKey = secretKey( (*it)->primaryKeyID() );
        if( oldKey )
        {
          (*it)->cloneKeyTrust( oldKey );
        }
      }

      qDeleteAll( mSecretKeys );
      mSecretKeys = newSecretKeyList;
    }

    mSecretKeysCached = true;
  }
}

KeyID
Module::selectKey( const KeyList& keys,
                   const QString& title,
                   const QString& text /*=QString() */ ,
                   const KeyID& keyId /* = KeyID() */ ,
                   const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyID retval = KeyID();

#ifndef QT_NO_TREEWIDGET
  KeySelectionDialog dlg( keys, title, text, KeyIDList( keyId ), false,
                          allowedKeys, false );

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
  bool rej = ( dlg.exec() == QDialog::Rejected );
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  if( !rej ) {
    retval = dlg.key();
  }
#endif

  return retval;
}

KeyIDList
Module::selectKeys( const KeyList& keys,
                    const QString& title,
                    const QString& text /*=QString() */ ,
                    const KeyIDList& keyIds /* = KeyIDList() */ ,
                    const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyIDList retval = KeyIDList();

#ifndef QT_NO_TREEWIDGET
  KeySelectionDialog dlg( keys, title, text, keyIds, false, allowedKeys,
                          true );

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
  bool rej = ( dlg.exec() == QDialog::Rejected );
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  if( !rej ) {
    retval = dlg.keys();
  }
#endif

  return retval;
}


KeyID
Module::selectKey( bool& rememberChoice,
                   const KeyList& keys,
                   const QString& title,
                   const QString& text /*=QString() */ ,
                   const KeyID& keyId /* = KeyID() */ ,
                   const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyID retval = KeyID();

#ifndef QT_NO_TREEWIDGET
  KeySelectionDialog dlg( keys, title, text, KeyIDList( keyId ), false,
                          allowedKeys, false );

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
  bool rej = ( dlg.exec() == QDialog::Rejected );
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  if( !rej ) {
    retval = dlg.key();
    rememberChoice = dlg.rememberSelection();
  }
  else {
    rememberChoice = false;
  }
#endif

  return retval;
}

KeyIDList
Module::selectKeys( bool& rememberChoice,
                    const KeyList& keys,
                    const QString& title,
                    const QString& text /*=QString() */ ,
                    const KeyIDList& keyIds /* = KeyIDList() */ ,
                    const unsigned int allowedKeys /* = AllKeys */ )
{
  KeyIDList retval = KeyIDList();

#ifndef QT_NO_TREEWIDGET
  KeySelectionDialog dlg( keys, title, text, keyIds, true, allowedKeys,
                          true );

#ifndef QT_NO_CURSOR
  QApplication::setOverrideCursor( QCursor(Qt::ArrowCursor) );
#endif
  bool rej = ( dlg.exec() == QDialog::Rejected );
#ifndef QT_NO_CURSOR
  QApplication::restoreOverrideCursor();
#endif

  if( !rej ) {
    retval = dlg.keys();
    rememberChoice = dlg.rememberSelection();
  }
  else {
    rememberChoice = false;
  }
#endif

  return retval;
}

KeyIDList
Module::keysForAddress( const QString& address )
{
  if( address.isEmpty() ) {
    return KeyIDList();
  }
  QString addr = canonicalAddress( address ).toLower();
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
  QString addr = canonicalAddress( address ).toLower();
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
  int num = general.readEntry( "addressEntries", 0 );

  addressDataDict.clear();
  for( int i=1; i<=num; ++i ) {
    KConfigGroup addrGroup( config, QString::fromLatin1("Address #%1").arg(i) );
    address = addrGroup.readEntry( "Address" );
    data.keyIds = KeyIDList::fromStringList( addrGroup.readEntry( "Key IDs" , QStringList() ) );
    data.encrPref = (EncryptPref) addrGroup.readEntry( "EncryptionPreference",
                                                          int(UnknownEncryptPref ));
//     kDebug( 5326 ) <<"Read address" << i <<":" << address
//                   << "\nKey IDs: 0x" << data.keyIds.toStringList().join(", 0x")
//                   << "\nEncryption preference:" << data.encrPref;
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
        ++it, ++i ) {
    KConfigGroup addrGroup( config, QString::fromLatin1("Address #%1").arg(i));
    addrGroup.writeEntry( "Address", it.key() );
    addrGroup.writeEntry( "Key IDs", it.value().keyIds.toStringList() );
    addrGroup.writeEntry( "EncryptionPreference", (int)it.value().encrPref );
  }

  config->sync();
}

EncryptPref
Module::encryptionPreference( const QString& address )
{
  QString addr = canonicalAddress( address ).toLower();
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
  QString addr = canonicalAddress( address ).toLower();
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
