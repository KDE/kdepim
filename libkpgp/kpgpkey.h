/*
    kpgpkey.h

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

#ifndef KPGPKEY_H
#define KPGPKEY_H

#include <time.h>

#include <qcstring.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvaluelist.h>

namespace Kpgp {

/** These are the possible validity values for a PGP user id and for the owner
    trust.
 */
typedef enum
{ // this is copied from gpgme.h which is a part of GPGME
  KPGP_VALIDITY_UNKNOWN = 0,   // the trust hasn't been determined
  KPGP_VALIDITY_UNDEFINED = 1, // trust is undefined
  KPGP_VALIDITY_NEVER = 2,
  KPGP_VALIDITY_MARGINAL = 3,
  KPGP_VALIDITY_FULL = 4,
  KPGP_VALIDITY_ULTIMATE = 5
} Validity;

/** These are the possible preferences for encryption.
 */
typedef enum
{
  NeverEncrypt = -1,
  UnknownEncryptPref = 0,
  AlwaysEncrypt = 1,
  AlwaysEncryptIfPossible = 2,
  AlwaysAskForEncryption = 3,
  AskWheneverPossible = 4
} EncryptPref;


typedef QCString KeyID;

class KeyIDList : public QValueList<KeyID>
{
 public:
  KeyIDList() { }
  KeyIDList( const KeyIDList& l ) : QValueList<KeyID>(l) { }
  KeyIDList( const QValueList<KeyID>& l ) : QValueList<KeyID>(l) { }
  KeyIDList( const KeyID& i ) { append(i); }

  QStringList toStringList() const;

  static KeyIDList fromStringList( const QStringList& );
};

/** This class is used to store information about a user id of a PGP key.
 */
class UserID
{
 public:
  /** Constructs a new user id with the given values. */
  UserID(const QString& str,
             const Validity validity = KPGP_VALIDITY_UNKNOWN,
             const bool revoked = false,
             const bool invalid = false);
  ~UserID() {};
  
  /** Returns the text of the user id. */
  QString text() const;

  /** Returns true if the user id has been revoked. */
  bool revoked() const;

  /** Returns true if the user id is invalid. */
  bool invalid() const;

  /** Returns the validity of resp. the trust in the user id. */
  Validity validity() const;

  /** Sets the text of the user id to <em>str</em>. */
  void setText(const QString& str);

  /** Sets the flag if the user id has been revoked to <em>revoked</em>. */
  void setRevoked(const bool revoked);

  /** Sets the flag if the user id is invalid to <em>invalid</em>. */
  void setInvalid(const bool invalid);

  /** Sets the validity of resp. the trust in the user id to 
      <em>validity</em>. */
  void setValidity(const Validity validity);

 protected:
  bool mRevoked : 1;
  bool mInvalid : 1;
  Validity mValidity;
  QString mText;
};

typedef QPtrList<UserID> UserIDList;
typedef QPtrListIterator<UserID> UserIDListIterator;

inline QString UserID::text() const
{
  return mText;
}

inline bool UserID::revoked() const
{
  return mRevoked;
}

inline bool UserID::invalid() const
{
  return mInvalid;
}

inline Validity UserID::validity() const
{
  return mValidity;
}

inline void UserID::setText(const QString& str)
{
  mText = str;
}

inline void UserID::setRevoked(const bool revoked)
{
  mRevoked = revoked;
}

inline void UserID::setInvalid(const bool invalid)
{
  mInvalid = invalid;
}

inline void UserID::setValidity(const Validity validity)
{
  mValidity = validity;
}


/** This class is used to store information about a subkey of a PGP key.
 */
class Subkey
{
 public:
  /** Constructs a new subkey with the given key ID. */
  Subkey(const KeyID& keyID, const bool secret = false);
  ~Subkey() {};

  /** Returns true if the subkey is a secret subkey. */
  bool secret() const;

  /** Returns true if the subkey has been revoked. */
  bool revoked() const;

  /** Returns true if the subkey has expired. */
  bool expired() const;

  /** Returns true if the subkey has been disabled. */
  bool disabled() const;

  /** Returns true if the subkey is invalid. */
  bool invalid() const;

  /** Returns true if the subkey can be used to encrypt data. */
  bool canEncrypt() const;

  /** Returns true if the subkey can be used to sign data. */
  bool canSign() const;

  /** Returns true if the subkey can be used to certify keys. */
  bool canCertify() const;

  /** Returns the key algorithm of the subkey. */
  unsigned int keyAlgorithm() const;

  /** Returns the length of the subkey in bits. */
  unsigned int keyLength() const;

  /** Returns the long 64 bit key ID of the subkey if it's available. 
      Otherwise the short 32 bit key ID is returned. */
  KeyID longKeyID() const;

  /** Returns the (short) 32 bit key ID of the subkey. */
  KeyID keyID() const;
  
  /** Returns the fingerprint of the subkey. */
  QCString fingerprint() const;

  /** Returns the creation date of the subkey. */
  time_t creationDate() const;

  /** Returns the expiration date of the subkey. */
  time_t expirationDate() const;

  /** Sets the flag if the subkey is a secret subkey to <em>secret</em>. */
  void setSecret(const bool secret);

  /** Sets the flag if the subkey has been revoked to <em>revoked</em>. */
  void setRevoked(const bool revoked);

  /** Sets the flag if the subkey has expired to <em>expired</em>. */
  void setExpired(const bool expired);

  /** Sets the flag if the subkey has been disabled to <em>disabled</em>. */
  void setDisabled(const bool disabled);

  /** Sets the flag if the subkey is invalid to <em>invalid</em>. */
  void setInvalid(const bool invalid);

  /** Sets the flag if the subkey can be used to encrypt data to
      <em>canEncrypt</em>. */
  void setCanEncrypt(const bool canEncrypt);

  /** Sets the flag if the subkey can be used to sign data to
      <em>canSign</em>. */
  void setCanSign(const bool canSign);

  /** Sets the flag if the subkey can be used to certify keys to
      <em>canCertify</em>. */
  void setCanCertify(const bool canCertify);

  /** Sets the key algorithm of the subkey to <em>keyAlgo</em>. */
  void setKeyAlgorithm(const unsigned int keyAlgo);

  /** Sets the key length of the subkey to <em>keyLen</em> bits. */
  void setKeyLength(const unsigned int keyLen);

  /** Sets the key ID of the subkey to <em>keyID</em>. */
  void setKeyID(const KeyID& keyID);

  /** Sets the fingerprint of the subkey to <em>fingerprint</em>. */
  void setFingerprint(const QCString& fingerprint);

  /** Sets the creation date of the subkey to <em>creationDate</em> seconds
      since Epoch. */
  void setCreationDate(const time_t creationDate);

  /** Sets the expiration date of the subkey to <em>expirationDate</em> seconds
      since Epoch. */
  void setExpirationDate(const time_t expirationDate);

 protected:
  bool mSecret : 1;
  /* various flags */
  bool mRevoked : 1;
  bool mExpired : 1;
  bool mDisabled : 1;
  bool mInvalid : 1;
  bool mCanEncrypt : 1;
  bool mCanSign : 1;
  bool mCanCertify : 1;

  unsigned int mKeyAlgo;
  unsigned int mKeyLen;
  KeyID mKeyID;
  QCString mFingerprint;
  time_t mTimestamp; /* -1 for invalid, 0 for not available */
  time_t mExpiration; /* -1 for never, 0 for not available */
};

inline bool Subkey::secret() const
{
  return mSecret;
}

inline bool Subkey::revoked() const
{
  return mRevoked;
}

inline bool Subkey::expired() const
{
  return mExpired;
}

inline bool Subkey::disabled() const
{
  return mDisabled;
}

inline bool Subkey::invalid() const
{
  return mInvalid;
}

inline bool Subkey::canEncrypt() const
{
  return mCanEncrypt;
}

inline bool Subkey::canSign() const
{
  return mCanSign;
}

inline bool Subkey::canCertify() const
{
  return mCanCertify;
}

inline unsigned int Subkey::keyAlgorithm() const
{
  return mKeyAlgo;
}

inline unsigned int Subkey::keyLength() const
{
  return mKeyLen;
}

inline KeyID Subkey::longKeyID() const
{
  return mKeyID;
}

inline KeyID Subkey::keyID() const
{
  return mKeyID.right(8);
}

inline QCString Subkey::fingerprint() const
{
  return mFingerprint;
}

inline time_t Subkey::creationDate() const
{
  return mTimestamp;
}

inline time_t Subkey::expirationDate() const
{
  return mExpiration;
}

inline void Subkey::setSecret(const bool secret)
{
  mSecret = secret;
}

inline void Subkey::setRevoked(const bool revoked)
{
  mRevoked = revoked;
}

inline void Subkey::setExpired(const bool expired)
{
  mExpired = expired;
}

inline void Subkey::setDisabled(const bool disabled)
{
  mDisabled = disabled;
}

inline void Subkey::setInvalid(const bool invalid)
{
  mInvalid = invalid;
}

inline void Subkey::setCanEncrypt(const bool canEncrypt)
{
  mCanEncrypt = canEncrypt;
}

inline void Subkey::setCanSign(const bool canSign)
{
  mCanSign = canSign;
}

inline void Subkey::setCanCertify(const bool canCertify)
{
  mCanCertify = canCertify;
}

inline void Subkey::setKeyAlgorithm(const unsigned int keyAlgo)
{
  mKeyAlgo = keyAlgo;
}

inline void Subkey::setKeyLength(const unsigned int keyLen)
{
  mKeyLen = keyLen;
}

inline void Subkey::setKeyID(const KeyID& keyID)
{
  mKeyID = keyID;
}

inline void Subkey::setFingerprint(const QCString& fingerprint)
{
  mFingerprint = fingerprint;
}

inline void Subkey::setCreationDate(const time_t creationDate)
{
  mTimestamp = creationDate;
}

inline void Subkey::setExpirationDate(const time_t expirationDate)
{
  mExpiration = expirationDate;
}

typedef QPtrList<Subkey> SubkeyList;
typedef QPtrListIterator<Subkey> SubkeyListIterator;


/** This class is used to store information about a PGP key.
 */
class Key
{
 public:
  /** Constructs a new PGP key with <em>keyid</em> as key ID of the
      primary key and <em>uid</em> as primary user ID. */
  Key( const KeyID& keyid = KeyID(),
       const QString& uid = QString::null,
       const bool secret = false);
  ~Key();

  /** Clears/resets all key data. */
  void clear();

  /** Returns true if the key is a secret key. */
  bool secret() const;

  /** Returns true if the key has been revoked. */
  bool revoked() const;

  /** Returns true if the key has expired. */
  bool expired() const;

  /** Returns true if the key has been disabled. */
  bool disabled() const;

  /** Returns true if the key is invalid. */
  bool invalid() const;

  /** Returns true if the key can be used to encrypt data. */
  bool canEncrypt() const;

  /** Returns true if the key can be used to sign data. */
  bool canSign() const;

  /** Returns true if the key can be used to certify keys. */
  bool canCertify() const;

  /** Sets the flag if the key is a secret key to <em>secret</em>. */
  void setSecret(const bool secret);

  /** Sets the flag if the key has been revoked to <em>revoked</em>. */
  void setRevoked(const bool revoked);

  /** Sets the flag if the key has expired to <em>expired</em>. */
  void setExpired(const bool expired);

  /** Sets the flag if the key has been disabled to <em>disabled</em>. */
  void setDisabled(const bool disabled);

  /** Sets the flag if the key is invalid to <em>invalid</em>. */
  void setInvalid(const bool invalid);

  /** Sets the flag if the key can be used to encrypt data to
      <em>canEncrypt</em>. */
  void setCanEncrypt(const bool canEncrypt);

  /** Sets the flag if the key can be used to sign data to
      <em>canSign</em>. */
  void setCanSign(const bool canSign);

  /** Sets the flag if the key can be used to certify keys to
      <em>canCertify</em>. */
  void setCanCertify(const bool canCertify);


  /** Returns the encryption preference for this key. */
  EncryptPref encryptionPreference();

  /** Sets the encryption preference for this key to <em>encrPref</em>. */
  void setEncryptionPreference( const EncryptPref encrPref );


  /** Returns the primary user ID or a null string if there are no
      user IDs. */
  QString primaryUserID() const;

  /** Returns the key ID of the primary key or a null string if there
      are no subkeys. */
  KeyID primaryKeyID() const;

  /** Returns the fingerprint of the primary key or a null string if there
      are no subkeys. */
  QCString primaryFingerprint() const;

  /** Returns true if there are no user IDs or no subkeys.*/
  bool isNull() const;

  /** Returns the creation date of the primary subkey.
   */
  time_t creationDate() const;

  /** Returns the trust value of this key. This is the maximal trust value
      of any of the user ids of this key.
   */
  Validity keyTrust() const;

  /** Returns the trust value for the given user id of this key.
   */
  Validity keyTrust( const QString& uid ) const;

  /** Set the validity values for the user ids to the validity values of
      the given key. This is useful after rereading a key without expensive
      trust checking.
   */
  void cloneKeyTrust( const Key* key );

  /** Returns true if the key is valid, i.e. not revoked, expired, disabled
      or invalid.
   */
  bool isValid() const;

  /** Returns true if the key is a valid encryption key. The trust is not
      checked.
   */
  bool isValidEncryptionKey() const;

  /** Returns true if the key is a valid signing key. The trust is not checked.
   */
  bool isValidSigningKey() const;

  /** Returns the list of userIDs. */
  const UserIDList userIDs() const;

  /** Returns the list of subkeys. */
  const SubkeyList subkeys() const;

  /** Adds a user ID with the given values to the key if <em>uid</em> isn't
      an empty string. */
  void addUserID(const QString& uid,
                 const Validity validity = KPGP_VALIDITY_UNKNOWN,
                 const bool revoked = false,
                 const bool invalid = false);

  /** Adds the given user ID to the key. */
  void addUserID(const UserID *userID);

  /** Returns true if the given string matches one of the user IDs.
      The match is case sensitive if <em>cs</em> is true or case insensitive
      if <em>cs</em> is false. */
  bool matchesUserID(const QString& str, bool cs = true);

  /** Adds a subkey with the given values to the key if <em>keyID</em> isn't
      an empty string. */
  void addSubkey(const KeyID& keyID, const bool secret = false);

  /** Adds the given subkey to the key. */
  void addSubkey(const Subkey *subkey);

  /** Returns a pointer to the subkey with the given key ID. */
  Subkey *getSubkey(const KeyID& keyID);

  /** Sets the fingerprint of the given subkey to <em>fpr</em>. */
  void setFingerprint(const KeyID& keyID, const QCString& fpr);

 protected:
  bool mSecret : 1;
  /* global flags */
  bool mRevoked : 1;
  bool mExpired : 1;
  bool mDisabled : 1;
  bool mInvalid : 1;
  bool mCanEncrypt : 1;
  bool mCanSign : 1;
  bool mCanCertify : 1;
  
  EncryptPref mEncryptPref;

  SubkeyList mSubkeys;
  UserIDList mUserIDs;
};

inline bool Key::secret() const
{
  return mSecret;
}

inline bool Key::revoked() const
{
  return mRevoked;
}

inline bool Key::expired() const
{
  return mExpired;
}

inline bool Key::disabled() const
{
  return mDisabled;
}

inline bool Key::invalid() const
{
  return mInvalid;
}

inline bool Key::canEncrypt() const
{
  return mCanEncrypt;
}

inline bool Key::canSign() const
{
  return mCanSign;
}

inline bool Key::canCertify() const
{
  return mCanCertify;
}

inline void Key::setSecret(const bool secret)
{
  mSecret = secret;
}

inline void Key::setRevoked(const bool revoked)
{
  mRevoked = revoked;
}

inline void Key::setExpired(const bool expired)
{
  mExpired = expired;
}

inline void Key::setDisabled(const bool disabled)
{
  mDisabled = disabled;
}

inline void Key::setInvalid(const bool invalid)
{
  mInvalid = invalid;
}

inline void Key::setCanEncrypt(const bool canEncrypt)
{
  mCanEncrypt = canEncrypt;
}

inline void Key::setCanSign(const bool canSign)
{
  mCanSign = canSign;
}

inline void Key::setCanCertify(const bool canCertify)
{
  mCanCertify = canCertify;
}

inline EncryptPref Key::encryptionPreference()
{
  return mEncryptPref;
}

inline void Key::setEncryptionPreference( const EncryptPref encrPref )
{
  mEncryptPref = encrPref;
}

inline QString Key::primaryUserID() const
{
  UserID *uid = mUserIDs.getFirst();

  if (uid)
    return uid->text();
  else
    return QString::null;
}

inline KeyID Key::primaryKeyID() const 
{
  Subkey *key = mSubkeys.getFirst();

  if (key)
    return key->keyID();
  else
    return KeyID();
}

inline QCString Key::primaryFingerprint() const
{
  Subkey *key = mSubkeys.getFirst();

  if (key)
    return key->fingerprint();
  else
    return QCString();
}

inline const UserIDList Key::userIDs() const
{ 
  return mUserIDs;
}

inline const SubkeyList Key::subkeys() const
{ 
  return mSubkeys;
}

inline bool Key::isNull() const
{
  return (mUserIDs.isEmpty() || mSubkeys.isEmpty());
}

inline time_t Key::creationDate() const
{
  if( !mSubkeys.isEmpty() )
    return mSubkeys.getFirst()->creationDate();
  else
    return -1;
}

inline void Key::addUserID(const UserID *userID)
{
  if (userID)
    mUserIDs.append(userID);
}

inline void Key::addSubkey(const Subkey *subkey)
{
  if (subkey)
    mSubkeys.append(subkey);
}



typedef QPtrList<Key> KeyListBase;
typedef QPtrListIterator<Key> KeyListIterator;

class KeyList : public KeyListBase
{
 public:
  ~KeyList()
    { clear(); }

 private:
  int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
    { 
      // sort case insensitively by the primary User IDs
      return QString::compare((static_cast<Key*>(s1))->primaryUserID().lower(),
                              (static_cast<Key*>(s2))->primaryUserID().lower());
    }
};

} // namespace Kpgp

#endif
