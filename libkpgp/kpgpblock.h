/*
    kpgpblock.h

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

#ifndef KPGPBLOCK_H
#define KPGPBLOCK_H

#include <q3cstring.h>
#include <qstring.h>
#include <q3strlist.h>

#include <kdepimmacros.h>

//#include <qstringlist.h>
class QStringList;

#include "kpgp.h"

namespace Kpgp {

typedef enum {
  UnknownBlock = -1,        // BEGIN PGP ???
  NoPgpBlock = 0,
  PgpMessageBlock = 1,      // BEGIN PGP MESSAGE
  MultiPgpMessageBlock = 2, // BEGIN PGP MESSAGE, PART X[/Y]
  SignatureBlock = 3,       // BEGIN PGP SIGNATURE
  ClearsignedBlock = 4,     // BEGIN PGP SIGNED MESSAGE
  PublicKeyBlock = 5,       // BEGIN PGP PUBLIC KEY BLOCK
  PrivateKeyBlock = 6       // BEGIN PGP PRIVATE KEY BLOCK (PGP 2.x: ...SECRET...)
} BlockType;

typedef enum {
  OK          =  0x0000,
  CLEARTEXT   =  0x0000,
  RUN_ERR     =  0x0001,
  ERROR       =  0x0001,
  ENCRYPTED   =  0x0002,
  SIGNED      =  0x0004,
  GOODSIG     =  0x0008,
  ERR_SIGNING =  0x0010,
  UNKNOWN_SIG =  0x0020,
  BADPHRASE   =  0x0040,
  BADKEYS     =  0x0080,
  NO_SEC_KEY  =  0x0100, 
  MISSINGKEY  =  0x0200,
  CANCEL      =  0x8000
} MessageStatus;
  
class Base;
class Module;

  /*
   * BEGIN PGP MESSAGE
   *     Used for signed, encrypted, or compressed files.
   *
   * BEGIN PGP PUBLIC KEY BLOCK
   *     Used for armoring public keys
   *
   * BEGIN PGP PRIVATE KEY BLOCK (PGP 2.x: BEGIN PGP SECRET KEY BLOCK)
   *     Used for armoring private keys
   *
   * BEGIN PGP MESSAGE, PART X/Y
   *     Used for multi-part messages, where the armor is split amongst Y
   *     parts, and this is the Xth part out of Y.
   *
   * BEGIN PGP MESSAGE, PART X
   *     Used for multi-part messages, where this is the Xth part of an
   *     unspecified number of parts. Requires the MESSAGE-ID Armor
   *     Header to be used.
   *
   * BEGIN PGP SIGNATURE
   *     Used for detached signatures, OpenPGP/MIME signatures, and
   *     signatures following clearsigned messages. Note that PGP 2.x
   *     uses BEGIN PGP MESSAGE for detached signatures.
   * 
   * BEGIN PGP SIGNED MESSAGE
   *     Used for cleartext signed messages.
   */
class KDE_EXPORT Block
{
 public:

  Block( const Q3CString& str = Q3CString() );
  ~Block();

  Q3CString text() const;
  void setText( const Q3CString& str );

  void setProcessedText( const Q3CString& str );

  int status() const;
  void setStatus( const int status );

  BlockType type();

  /** is the message encrypted ? */
  bool isEncrypted() const;

  /** is the message signed by someone */
  bool isSigned() const;

  /** is the signature good ? */
  bool goodSignature() const;

  /** returns the primary user id of the signer or a null string if we
      don't have the public key of the signer */
  QString signatureUserId() const;
  void setSignatureUserId( const QString& userId );

  /** keyID of signer */
  Q3CString signatureKeyId() const;
  void setSignatureKeyId( const Q3CString& keyId );

  /** date of the signature 
      WARNING: Will most likely be changed to QDateTime */
  Q3CString signatureDate() const;
  void setSignatureDate( const Q3CString& date );

  /** the persons who can decrypt the message */
  const Q3StrList encryptedFor() const;

  /** shows the secret key which is needed
    to decrypt the message */
  QString requiredKey() const;
  void setRequiredKey( const Q3CString& keyId );

  QString requiredUserId() const;
  void setRequiredUserId( const QString& userId );

  Q3CString error() const;
  void setError( const Q3CString& str );

  /** Resets all information about this OpenPGP block */
  void reset();

  /** decrypts this OpenPGP block if the passphrase is good.
      returns false otherwise */
  bool decrypt();

  /** tries to verify this (clearsigned) OpenPGP block */
  bool verify();

  /** clearsigns this OpenPGP block with the key corresponding to the
      given key id. The charset is needed to display the text correctly.
      Returns
      false  if there was an unresolvable error or if signing was canceled
      true   if everything is o.k.
  */
  Kpgp::Result clearsign( const Q3CString& keyId,
                  const Q3CString& charset = Q3CString() );

  /** encrypts this OpenPGP block for a list of persons. if sign is true then
      the message is signed with the key corresponding to the given key id.
      Returns
      false  if there was an unresolvable error or if encryption was canceled
      true   if everything is o.k.
  */
  Kpgp::Result encrypt( const QStringList& receivers, const Q3CString& keyId,
                const bool sign, const Q3CString& charset = Q3CString() );

 private:
  void clear();

  BlockType determineType() const;

  Q3CString mText;
  Q3CString mProcessedText;
  Q3CString mError;
  QString mSignatureUserId;
  Q3CString mSignatureKeyId;
  Q3CString mSignatureDate;
  Q3CString mRequiredKey;
  QString mRequiredUserId;
  Q3StrList mEncryptedFor;
  int mStatus;
  bool mHasBeenProcessed;
  BlockType mType;
};

// -- inlined member functions ---------------------------------------------

inline Q3CString
Block::text() const
{
  if( mHasBeenProcessed )
    return mProcessedText;
  else
    return mText;
}

inline void
Block::setText( const Q3CString& str )
{
  clear();
  mText = str;
}

inline void
Block::setProcessedText( const Q3CString& str )
{
  mProcessedText = str;
  mHasBeenProcessed = true;
}

inline Q3CString
Block::error() const
{
  return mError;
}

inline void
Block::setError( const Q3CString& str )
{
  mError = str;
}

inline int
Block::status() const
{
  return mStatus;
}

inline void
Block::setStatus( const int status )
{
  mStatus = status;
}

inline BlockType
Block::type()
{
  if( mType == NoPgpBlock )
    mType = determineType();
  return mType;
}

inline QString
Block::signatureUserId() const
{
  return mSignatureUserId;
}

inline void
Block::setSignatureUserId( const QString& userId )
{
  mSignatureUserId = userId;
}

inline Q3CString
Block::signatureKeyId() const
{
  return mSignatureKeyId;
}

inline void
Block::setSignatureKeyId( const Q3CString& keyId )
{
  mSignatureKeyId = keyId;
}

inline Q3CString
Block::signatureDate() const
{
  return mSignatureDate;
}

inline void
Block::setSignatureDate( const Q3CString& date )
{
  mSignatureDate = date;
}

inline QString
Block::requiredKey() const
{
  return mRequiredKey;
}

inline void
Block::setRequiredKey( const Q3CString& keyId )
{
  mRequiredKey = keyId;
}

inline QString
Block::requiredUserId() const
{
  return mRequiredUserId;
}

inline void
Block::setRequiredUserId( const QString& userId )
{
  mRequiredUserId = userId;
}

inline const Q3StrList
Block::encryptedFor() const
{
  return mEncryptedFor;
}

inline bool 
Block::isEncrypted() const
{
  if( mStatus & ENCRYPTED )
    return true;
  return false;
}

inline bool 
Block::isSigned() const
{
  if( mStatus & SIGNED )
    return true;
  return false;
}

inline bool 
Block::goodSignature() const
{
  if( mStatus & GOODSIG )
    return true;
  return false;
}

/*
inline bool
Block::unknownSigner() const
{
  if( mStatus & UNKNOWN_SIG )
    return true;
  return false;
}
*/

// -------------------------------------------------------------------------

} // namespace Kpgp

#endif

