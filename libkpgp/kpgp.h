/*
    kpgp.h

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

#ifndef KPGP_H
#define KPGP_H

#include "libkpgp_export.h"
#include "kpgpkey.h"

#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QWidget>
#include <QComboBox>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QGridLayout>

#include <stdio.h>

class QCheckBox;
class QGridLayout;

class KConfig;

namespace Kpgp {

/** This enumerated type is used by Kpgp::* to indicate which keys can be
    selected by the user. The following values are available:
    - Kpgp::PublicKeys: Public keys are shown in the selection dialog.
    - Kpgp::SecretKeys: Secret keys are shown in the selection dialog.
    - Kpgp::EncryptionKeys: Encryption keys can be selected.
    - Kpgp::SigningKeys: Signing keys can be selected.
    - Kpgp::ValidKeys: Only valid keys can be selected.
    - Kpgp::TrustedKeys: Only trusted keys can be selected.
    - Kpgp::AllKeys == PublicKeys | SecretKeys | EncryptionKeys | SigningKeys
*/
enum { PublicKeys = 1,
       SecretKeys = 2,
       EncryptionKeys = 4,
       SigningKeys = 8,
       ValidKeys = 16,
       TrustedKeys = 32,
       AllKeys = PublicKeys | SecretKeys | EncryptionKeys | SigningKeys,
       PubSecKeys = PublicKeys | SecretKeys,
       EncrSignKeys = EncryptionKeys | SigningKeys
};

enum Result
{
       Failure = 0,
       Ok = 1,
       Canceled = 2
};

class Base;
class Block;

class LIBKPGP_EXPORT Module
{
  friend class Block;

private:
    // the class running pgp
    Base *pgp;

  /**
   * Singleton pattern: use getKpgp()
   */
  Module();

public:
  virtual ~Module();

  /** the following virtual function form the interface to the
      application using Kpgp
  */
  virtual void readConfig();
  virtual void writeConfig(bool sync);
  virtual void init();

  /** decrypts the given OpenPGP block if the passphrase is good.
      returns false otherwise */
  bool decrypt( Block& block );

  /** Tries to verify the given OpenPGP block */
  bool verify( Block& block );

  /** clearsigns the given OpenPGP block with the key corresponding to the
      given key id. The charset is needed to display the text correctly.
      Returns
      Failure  if there was an unresolvable error
      Canceled if signing was canceled
      Ok       if everything is o.k.
  */
  Kpgp::Result clearsign( Block& block,
                  const KeyID& keyId, const QByteArray& charset = 0 );

  /** encrypts the given OpenPGP block for a list of persons. if sign is true
      then the block is clearsigned with the key corresponding to the given
      key id. The charset is needed to display the text correctly.
      Returns
      Failure  if there was an unresolvable error
      Canceled if encryption was canceled
      Ok       if everything is o.k.
  */
  Kpgp::Result encrypt( Block& block,
                const QStringList& receivers, const KeyID& keyId,
                bool sign, const QByteArray& charset = 0 );

  /** Determines the keys which should be used for encrypting the message
      to the given list of recipients.
      Returns:
      Failure  if there was an unresolvable error
      Canceled if encryption was canceled
      Ok       if everything is o.k.
  */
  Kpgp::Result getEncryptionKeys( KeyIDList& encryptionKeyIds,
                                  const QStringList& recipients,
                                  const KeyID& keyId );

  /** checks if encrypting to the given list of persons is possible and
      desired, i.e. if we have a (trusted) key for every recipient and
      if encryption to all keys is allowed.
      Returns
       0 if encryption is not possible or not desired,
       1 if encryption is possible and desired,
       2 if encryption is possible, but the user wants to be asked and
      -1 if there is a conflict which can't be automatically resolved.
  */
  int encryptionPossible( const QStringList& recipients );

protected:
  int doEncSign( Block& block, const KeyIDList& recipientKeyIds, bool sign );

public:
  /** sign a key in the keyring with users signature. */
  bool signKey( const KeyID& keyID );

  /** get the list of cached public keys. */
  const KeyList publicKeys();

  /** get the list of cached secret keys. */
  const KeyList secretKeys();

  /** Reads the list of public keys if necessary or if <em>reread</em> is true.
   */
  void readPublicKeys( bool reread = false );

  /** Reads the list of secret keys if necessary or if <em>reread</em> is true.
   */
  void readSecretKeys( bool reread = false );

  /** try to get an ascii armored key block for the given public key */
  QByteArray getAsciiPublicKey( const KeyID& keyID );

  /** Returns the public key with the given key ID or null if no matching
      key is found.
  */
  Key* publicKey( const KeyID& keyID );

  /** Returns the first public key with the given user ID or null if no
      matching key is found.
  */
  Key* publicKey( const QString& userID );

  /** Returns the secret key with the given key ID or null if no matching
      key is found.
  */
  Key* secretKey( const KeyID& keyID );

  /** Returns the trust value for the given key. This is the maximal trust
      value of any of the user ids of this key.
   */
  Validity keyTrust( const KeyID& keyID );

  /** Returns the trust value of a key with the given user id. If more than
      one key have this user id then the first key with this user id will be
      chosen.
   */
  Validity keyTrust( const QString& userID );

  /** Returns TRUE if the given key is at least trusted marginally. Otherwise
      FALSE is returned.
   */
  bool isTrusted( const KeyID& keyID );

  /** Rereads the key data for the given key and returns the reread data. If
      <em>readTrust</em> is true then the trust of this key will be determined.
   */
  Key* rereadKey( const KeyID& keyID, const bool readTrust = true );

  /** Request the change of the passphrase of the actual secret
      key. TBI */
  bool changePassPhrase();

  /** set a user identity to use (if you have more than one...)
      by default, pgp uses the identity which was generated last. */
  void setUser(const KeyID& keyID);
  /** Returns the actual key ID of the currently set key. */
  const KeyID user() const;

  /** always encrypt message to oneself? */
  void setEncryptToSelf(bool flag);
  bool encryptToSelf(void) const;

  /** store passphrase in pgp object
      Problem: passphrase stays in memory.
      Advantage: you can call en-/decrypt without always passing the
      passphrase
  */
  void setStorePassPhrase(bool);
  bool storePassPhrase(void) const;

  /** clears everything from memory */
  void clear(const bool erasePassPhrase = false);

  /** returns the last error that occurred */
  const QString lastErrorMsg(void) const;

  // what version of PGP/GPG should we use
  enum PGPType { tAuto, tGPG, tPGP2, tPGP5, tPGP6, tOff } pgpType;

  // did we find a pgp executable?
  bool havePGP(void) const;

  /** Should PGP/GnuPG be used? */
  bool usePGP(void) const { return (havePGP() && (pgpType != tOff)); }

  // show the result of encryption/signing?
  void setShowCipherText(const bool flag);
  bool showCipherText(void) const;

  // show the encryption keys for approval?
  void setShowKeyApprovalDlg(const bool flag);
  bool showKeyApprovalDlg(void) const;

  /** Shows a key selection dialog with all secret keys and the given title
      and the (optional) text. If <em>keyId</em> is given, then the
      corresponding key is selected.
  */
  KeyID selectSecretKey( const QString& title,
                         const QString& text = QString(),
                         const KeyID& keyId = KeyID() );

  /** Shows a key selection dialog with all public keys and the given title
      and the (optional) text. If <em>oldKeyId</em> is given, then the
      corresponding key is selected. If <em>address</em> is given, then the
      chosen key will be stored (if the user wants it to be stored).
      <em>mode</em> specifies which keys can be selected.
  */
  KeyID selectPublicKey( const QString& title,
                         const QString& text = QString(),
                         const KeyID& oldKeyId = KeyID(),
                         const QString& address = QString(),
                         const unsigned int allowedKeys = AllKeys );

  /** Shows a key selection dialog with all public keys and the given title
      and the (optional) text. If <em>oldKeyId</em> is given, then the
      corresponding key is selected. If <em>address</em> is given, then the
      chosen key will be stored (if the user wants it to be stored).
      <em>mode</em> specifies which keys can be selected.
  */
  KeyIDList selectPublicKeys( const QString& title,
                              const QString& text = QString(),
                              const KeyIDList& oldKeyIds = KeyIDList(),
                              const QString& address = QString(),
                              const unsigned int allowedKeys = AllKeys );

  // FIXME: key management

  /** Reads the encryption preference for the given address
      from the config file.
   */
  EncryptPref encryptionPreference( const QString& address );

  /** Writes the given encryption preference for the given address
      to the config file.
   */
  void setEncryptionPreference( const QString& address,
                                const EncryptPref pref );

  // -- static member functions --------------------------------------------

  /** return the actual pgp object */
  static Kpgp::Module *getKpgp();

  /** get the kpgp config object */
  static KConfig *getConfig();

  /** Parses the given message and splits it into OpenPGP blocks and
      Non-OpenPGP blocks.
      Returns TRUE if the message contains at least one OpenPGP block and
      FALSE otherwise.
      The format is then:
      <pre>
      1st Non-OpenPGP block
      1st OpenPGP block
      2nd Non-OpenPGP block
      ...
      n-th OpenPGP block
      (n+1)-th Non-OpenPGP block
      </pre>
  */
  static bool prepareMessageForDecryption( const QByteArray& msg,
                                           QList<Block>& pgpBlocks,
                                           QList<QByteArray>& nonPgpBlocks );

private:
  /** check if we have a trusted encryption key for the given person */
  bool haveTrustedEncryptionKey( const QString& person );

  /** get a list of encryption keys to be used for the given recipient */
  KeyIDList getEncryptionKeys( const QString& person );

  /** Set pass phrase */
  bool setPassPhrase(const QString &pass);

  /** test if the PGP executable is found and if there is a passphrase
      set or given. Returns:
       1 if everything is ok
       0 (together with some warning message) if something is missing
      -1 if the passphrase dialog was canceled
  */
  int prepare(bool needPassPhrase=false, Block* block = 0 );

  /** cleanup passphrase if it should not be stored. */
  void cleanupPass() { if (!storePass) wipePassPhrase(); }

  /** Wipes and optionally frees the memory used to hold the
      passphrase. */
  void wipePassPhrase(bool free=false);

  // transform an address into canonical form
  QString canonicalAddress( const QString& person );

  /** Shows a dialog to let the user select a key from the given list of keys
   */
  KeyID selectKey( const KeyList& keys,
                   const QString& title,
                   const QString& text = QString(),
                   const KeyID& keyId = KeyID(),
                   const unsigned int allowedKeys = AllKeys );

  /** Shows a dialog to let the user select a key from the given list of keys
   */
  KeyIDList selectKeys( const KeyList& keys,
                        const QString& title,
                        const QString& text = QString(),
                        const KeyIDList& keyIds = KeyIDList(),
                        const unsigned int allowedKeys = AllKeys );

  /** Shows a dialog to let the user select a key from the given list of keys.
      The dialog includes a checkbox ("Remember decision"). The state of the
      checkbox is returned in rememberChoice.
   */
  KeyID selectKey( bool& rememberChoice,
                   const KeyList& keys,
                   const QString& title,
                   const QString& text = QString(),
                   const KeyID& keyId = KeyID(),
                   const unsigned int allowedKeys = AllKeys );

  /** Shows a dialog to let the user select a list of keys from the given
      list of keys. The dialog includes a checkbox ("Remember decision").
      The state of the checkbox is returned in rememberChoice.
   */
  KeyIDList selectKeys( bool& rememberChoice,
                        const KeyList& keys,
                        const QString& title,
                        const QString& text = QString(),
                        const KeyIDList& keyIds = KeyIDList(),
                        const unsigned int allowedKeys = AllKeys );

  /** Returns the OpenPGP keys which should be used for encryption to the
      given address.
  */
  KeyIDList keysForAddress( const QString& address );

  /** Set an email address -> list of OpenPGP keys association.
  */
  void setKeysForAddress( const QString& address, const KeyIDList& keyIDs );

  /** Remove an email address -> OpenPGP key association. */
  void removeKeyForAddress( const QString& address );

  /** Reads the email address -> OpenPGP key associations from the config
      file.
  */
  void readAddressData();

  /** Writes the email address -> OpenPGP key associations to the config
      file.
  */
  void writeAddressData();

  bool checkForPGP(void);
  void assignPGPBase(void);

  KConfig *config;

  struct AddressData {
    KeyIDList keyIds;
    EncryptPref encrPref;
  };
  typedef QMap<QString, AddressData> AddressDataDict;
  AddressDataDict addressDataDict;

  KeyList mPublicKeys;
  bool mPublicKeysCached : 1; // did we already read the public keys?
  KeyList mSecretKeys;
  bool mSecretKeysCached : 1; // did we already read the secret keys?

  bool storePass : 1;
  char * passphrase;
  size_t passphrase_buffer_len;

  QString errMsg;

  KeyID pgpUser; // the key ID which is used to sign/encrypt to self
  bool flagEncryptToSelf : 1;

  bool havePgp : 1;
  bool havePGP5 : 1;
  bool haveGpg : 1;
  bool havePassPhrase : 1;
  bool showEncryptionResult : 1;
  bool mShowKeyApprovalDlg : 1;
}; // class Module

// -- inlined member functions ---------------------------------------------

inline void
Module::setShowKeyApprovalDlg( const bool flag )
{
  mShowKeyApprovalDlg = flag;
}

inline bool
Module::showKeyApprovalDlg( void ) const
{
  return mShowKeyApprovalDlg;
}

// -------------------------------------------------------------------------

} // namespace Kpgp

#endif

