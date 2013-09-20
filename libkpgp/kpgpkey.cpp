/*
    kpgpkey.cpp

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

#include "kpgpkey.h"
#include "kdebug.h"

#include <QByteArray>

namespace Kpgp {

/* member functions of Kpgp::KeyIDList --------------------------------- */

/** Converts from a KeyIDList to a QStringList.
*/
QStringList KeyIDList::toStringList() const
{
  QStringList res;
  for( KeyIDList::ConstIterator it = begin(); it != end(); ++it ) {
    res << QLatin1String((*it).data());
  }
  return res;
}

/** Converts from a QStringList to a KeyIDList.
*/
KeyIDList KeyIDList::fromStringList( const QStringList& l )
{
  KeyIDList res;
  QStringList::ConstIterator end(l.end());
  for( QStringList::ConstIterator it = l.begin(); it != end; ++it ) {
    res << (*it).toLocal8Bit();
  }
  return res;
}

/* member functions of Kpgp::UserID ------------------------------------ */

UserID::UserID(const QString& str, const Validity validity,
               const bool revoked, const bool invalid)
{
  mText = str;
  mValidity = validity;
  mRevoked = revoked;
  mInvalid = invalid;
}


/* member functions of Kpgp::Subkey ------------------------------------ */

Subkey::Subkey(const KeyID& keyID, const bool secret)
{
  mSecret = secret;
  mKeyID = keyID;

  mRevoked = false;
  mExpired = false;
  mDisabled = false;
  mInvalid = false;
  mCanEncrypt = false;
  mCanSign = false;
  mCanCertify = false;
  mKeyAlgo = 0;
  mKeyLen = 0;
  mFingerprint = 0;
  mTimestamp = 0;
  mExpiration = 0;
}


/* member functions of Kpgp::Key --------------------------------------- */

Key::Key(const KeyID& keyid, const QString& uid, const bool secret) :
  mSubkeys(), mUserIDs()
{
  mSecret = secret;
  if (!keyid.isEmpty())
    addSubkey(keyid, secret);
  if (!uid.isEmpty())
    addUserID(uid);

  mRevoked = false;
  mExpired = false;
  mDisabled = false;
  mInvalid = false;
  mCanEncrypt = false;
  mCanSign = false;
  mCanCertify = false;

  mEncryptPref = UnknownEncryptPref;
}

Key::~Key()
{
  //kDebug( 5326 ) <<"Kpgp::Key: Deleting key" << primaryUserID();
  qDeleteAll( mUserIDs );
  mUserIDs.clear();
  qDeleteAll( mSubkeys );
  mSubkeys.clear();
}

void
Key::clear()
{
  mSecret = false;
  mRevoked = false;
  mExpired = false;
  mDisabled = false;
  mInvalid = false;
  mCanEncrypt = false;
  mCanSign = false;
  mCanCertify = false;

  mEncryptPref = UnknownEncryptPref;

  qDeleteAll( mSubkeys );
  mSubkeys.clear();
  qDeleteAll( mUserIDs );
  mUserIDs.clear();
}

Validity
Key::keyTrust() const
{
  Validity trust = KPGP_VALIDITY_UNKNOWN;

  foreach ( UserID* userId, mUserIDs )
  {
    if( userId->validity() > trust )
      trust = userId->validity();
  }
  
  return trust;
}

Validity
Key::keyTrust( const QString& uid ) const
{
  Validity trust = KPGP_VALIDITY_UNKNOWN;

  if( uid.isEmpty() )
    return trust;

  foreach ( UserID* userId, mUserIDs )
  {
    if( userId->text() == uid )
      trust = userId->validity();
  }
  
  return trust;
}

void
Key::cloneKeyTrust( const Key* key )
{
  if( !key )
    return;

  foreach ( UserID* userId, mUserIDs )
  {
    userId->setValidity( key->keyTrust( userId->text() ) );
  }
}

bool
Key::isValid() const
{
  return ( !mRevoked && !mExpired && !mDisabled && !mInvalid );
}


bool
Key::isValidEncryptionKey() const
{
  return ( !mRevoked && !mExpired && !mDisabled && !mInvalid && mCanEncrypt );
}


bool
Key::isValidSigningKey() const
{
  return ( !mRevoked && !mExpired && !mDisabled && !mInvalid && mCanSign );
}


void Key::addUserID(const QString &uid, const Validity validity,
                    const bool revoked, const bool invalid)
{ 
  if (!uid.isEmpty()) {
    UserID *userID = new UserID(uid, validity, revoked, invalid);
    mUserIDs.append(userID);
  }
}

bool Key::matchesUserID(const QString& str, bool cs)
{
  if (str.isEmpty() || mUserIDs.isEmpty())
    return false;
  
  foreach ( UserID *userId, mUserIDs ) {
    if ((userId->text().indexOf(str, 0, cs?Qt::CaseSensitive:Qt::CaseInsensitive)) != -1)
      return true;
  }

  return false;
}

void Key::addSubkey(const KeyID& keyID, const bool secret)
{
  if (!keyID.isEmpty()) {
    Subkey *key = new Subkey(keyID, secret);
    mSubkeys.append(key);
  }
}

Subkey *Key::getSubkey(const KeyID& keyID)
{
  if (keyID.isEmpty() || mSubkeys.isEmpty())
    return 0;
  
  // is the given key ID a long (16 chars) or a short (8 chars) key ID?
  bool longKeyID = (keyID.length() == 16);

  foreach ( Subkey* subKey, mSubkeys ) {
    if (longKeyID) {
      if (subKey->longKeyID() == keyID)
        return subKey;
    }
    else {
      if (subKey->keyID() == keyID)
        return subKey;
    }
  }

  return 0;
}

void Key::setFingerprint(const KeyID& keyID, const QByteArray &fpr)
{
  Subkey *key;
  if ((key = getSubkey(keyID)) != 0) {
    key->setFingerprint(fpr);
  }
  else
    kDebug(5326) <<"Error: Can't set fingerprint. A subkey with key ID 0x"
                  << keyID << "doesn't exist.";
}

} // namespace Kpgp
