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

#include "libkpgp_export.h"
#include "kpgp.h"

#include <QtCore/QStringList>
#include <QtCore/QString>

namespace Kpgp {

#ifdef ERROR
# undef ERROR
#endif

enum BlockType {
  UnknownBlock = -1,        // BEGIN PGP ???
  NoPgpBlock = 0,
  PgpMessageBlock = 1,      // BEGIN PGP MESSAGE
  MultiPgpMessageBlock = 2, // BEGIN PGP MESSAGE, PART X[/Y]
  SignatureBlock = 3,       // BEGIN PGP SIGNATURE
  ClearsignedBlock = 4,     // BEGIN PGP SIGNED MESSAGE
  PublicKeyBlock = 5,       // BEGIN PGP PUBLIC KEY BLOCK
  PrivateKeyBlock = 6       // BEGIN PGP PRIVATE KEY BLOCK (PGP 2.x: ...SECRET...)
};

enum MessageStatus{
  OK          =  0x0000,
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
};


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
class KPGP_EXPORT Block
{
 public:

  explicit Block( const QByteArray& str = QByteArray() );
  ~Block();

  QByteArray text() const;
  void setText( const QByteArray& str );

  void setProcessedText( const QByteArray& str );

  int status() const;
  void setStatus( const int status );

  BlockType type() const;

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
  QByteArray signatureKeyId() const;
  void setSignatureKeyId( const QByteArray& keyId );

  /** date of the signature
      WARNING: Will most likely be changed to QDateTime */
  QByteArray signatureDate() const;
  void setSignatureDate( const QByteArray& date );

  /** the persons who can decrypt the message */
  const QStringList encryptedFor() const;

  /** shows the secret key which is needed
    to decrypt the message */
  QByteArray requiredKey() const;
  void setRequiredKey( const QByteArray& keyId );

  QString requiredUserId() const;
  void setRequiredUserId( const QString& userId );

  QByteArray error() const;
  void setError( const QByteArray& str );

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
  Kpgp::Result clearsign( const QByteArray& keyId,
                  const QByteArray& charset = QByteArray() );

  /** encrypts this OpenPGP block for a list of persons. if sign is true then
      the message is signed with the key corresponding to the given key id.
      Returns
      false  if there was an unresolvable error or if encryption was canceled
      true   if everything is o.k.
  */
  Kpgp::Result encrypt( const QStringList& receivers, const QByteArray& keyId,
                const bool sign, const QByteArray& charset = QByteArray() );

 private:
  void clear();

  BlockType determineType() const;

  QByteArray mText;
  QByteArray mProcessedText;
  QByteArray mError;
  QString mSignatureUserId;
  QByteArray mSignatureKeyId;
  QByteArray mSignatureDate;
  QByteArray mRequiredKey;
  QString mRequiredUserId;
  QStringList mEncryptedFor;
  int mStatus;
  bool mHasBeenProcessed;
  mutable BlockType mType;
};

// -- inlined member functions ---------------------------------------------

inline QByteArray
Block::text() const
{
  if( mHasBeenProcessed )
    return mProcessedText;
  else
    return mText;
}

inline void
Block::setText( const QByteArray& str )
{
  clear();
  mText = str;
}

inline void
Block::setProcessedText( const QByteArray& str )
{
  mProcessedText = str;
  mHasBeenProcessed = true;
}

inline QByteArray
Block::error() const
{
  return mError;
}

inline void
Block::setError( const QByteArray& str )
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
Block::type() const
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

inline QByteArray
Block::signatureKeyId() const
{
  return mSignatureKeyId;
}

inline void
Block::setSignatureKeyId( const QByteArray& keyId )
{
  mSignatureKeyId = keyId;
}

inline QByteArray
Block::signatureDate() const
{
  return mSignatureDate;
}

inline void
Block::setSignatureDate( const QByteArray& date )
{
  mSignatureDate = date;
}

inline QByteArray
Block::requiredKey() const
{
  return mRequiredKey;
}

inline void
Block::setRequiredKey( const QByteArray& keyId )
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

inline const QStringList
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

