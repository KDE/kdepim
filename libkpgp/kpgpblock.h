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
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA
 */

#ifndef KPGPBLOCK_H
#define KPGPBLOCK_H

#include <qcstring.h>
#include <qstring.h>
#include <qstrlist.h>

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

  Block( const QCString& str = QCString() );
  ~Block();

  QCString text() const;
  void setText( const QCString& str );

  void setProcessedText( const QCString& str );

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
  QCString signatureKeyId() const;
  void setSignatureKeyId( const QCString& keyId );

  /** date of the signature 
      WARNING: Will most likely be changed to QDateTime */
  QCString signatureDate() const;
  void setSignatureDate( const QCString& date );

  /** the persons who can decrypt the message */
  const QStrList encryptedFor() const;

  /** shows the secret key which is needed
    to decrypt the message */
  QString requiredKey() const;
  void setRequiredKey( const QCString& keyId );

  QString requiredUserId() const;
  void setRequiredUserId( const QString& userId );

  QCString error() const;
  void setError( const QCString& str );

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
  Kpgp::Result clearsign( const QCString& keyId,
                  const QCString& charset = QCString() );

  /** encrypts this OpenPGP block for a list of persons. if sign is true then
      the message is signed with the key corresponding to the given key id.
      Returns
      false  if there was an unresolvable error or if encryption was canceled
      true   if everything is o.k.
  */
  Kpgp::Result encrypt( const QStringList& receivers, const QCString& keyId,
                const bool sign, const QCString& charset = QCString() );

 private:
  void clear();

  BlockType determineType() const;

  QCString mText;
  QCString mProcessedText;
  QCString mError;
  QString mSignatureUserId;
  QCString mSignatureKeyId;
  QCString mSignatureDate;
  QCString mRequiredKey;
  QString mRequiredUserId;
  QStrList mEncryptedFor;
  int mStatus;
  bool mHasBeenProcessed;
  BlockType mType;
};

// -- inlined member functions ---------------------------------------------

inline QCString
Block::text() const
{
  if( mHasBeenProcessed )
    return mProcessedText;
  else
    return mText;
}

inline void
Block::setText( const QCString& str )
{
  clear();
  mText = str;
}

inline void
Block::setProcessedText( const QCString& str )
{
  mProcessedText = str;
  mHasBeenProcessed = true;
}

inline QCString
Block::error() const
{
  return mError;
}

inline void
Block::setError( const QCString& str )
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

inline QCString
Block::signatureKeyId() const
{
  return mSignatureKeyId;
}

inline void
Block::setSignatureKeyId( const QCString& keyId )
{
  mSignatureKeyId = keyId;
}

inline QCString
Block::signatureDate() const
{
  return mSignatureDate;
}

inline void
Block::setSignatureDate( const QCString& date )
{
  mSignatureDate = date;
}

inline QString
Block::requiredKey() const
{
  return mRequiredKey;
}

inline void
Block::setRequiredKey( const QCString& keyId )
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

inline const QStrList
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

