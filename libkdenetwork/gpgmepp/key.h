/* key.h - wraps a gpgme key
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

   This file is part of GPGME++.
 
   GPGME++ is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   GPGME++ is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

// -*- c++ -*-
#ifndef __GPGMEPP_KEY_H__
#define __GPGMEPP_KEY_H__

#include <gpgmepp/gpgmefw.h>
#include <gpgmepp/context.h>

#include <sys/time.h>

#include <vector>
#include <kdepimmacros.h>

#include <kdemacros.h>

namespace GpgME {

  class Subkey;
  class UserID;

  //
  // class Key
  //

  class KDE_EXPORT Key {
    friend class Context;
  public:
    Key();
    Key( gpgme_key_t key, bool acquireRef, unsigned int keyListMode=0 );
    Key( const Key & key );
    ~Key();

    static Key null;

    const Key & operator=( const Key & other );

    bool isNull() const;

    UserID userID( unsigned int index ) const;
    Subkey subkey( unsigned int index ) const;

    unsigned int numUserIDs() const;
    unsigned int numSubkeys() const;

    std::vector<UserID> userIDs() const;
    std::vector<Subkey> subkeys() const;

    bool isRevoked() const;
    bool isExpired() const;
    bool isDisabled() const;
    bool isInvalid() const;

    bool canEncrypt() const;
    bool canSign() const;
    bool canCertify() const;
    bool canAuthenticate() const;

    bool hasSecret() const;
    bool isSecret() const { return hasSecret(); }

    /*!
      @return true if this is a X.509 root certificate (currently
      equivalent to something like
      strcmp( chainID(), subkey(0).fingerprint() ) == 0 )
    */
    bool isRoot() const;

    enum OwnerTrust { Unknown=0, Undefined=1, Never=2,
		    Marginal=3, Full=4, Ultimate=5 };

    OwnerTrust ownerTrust() const;
    char ownerTrustAsString() const;

    typedef Context::Protocol Protocol;
    Protocol protocol() const;
    const char * protocolAsString() const;

    const char * issuerSerial() const;
    const char * issuerName() const;
    const char * chainID() const;

    const char * keyID() const;
    const char * shortKeyID() const;
    const char * primaryFingerprint() const;

    typedef Context::KeyListMode KeyListMode;
    unsigned int keyListMode() const;

  private:
    gpgme_key_t impl() const;
    class Private;
    Private * d;
  };

  //
  // class Subkey
  //

  class KDE_EXPORT Subkey {
  public:
    Subkey( gpgme_key_t key=0, gpgme_sub_key_t subkey=0 );
    Subkey( gpgme_key_t key, unsigned int idx );
    Subkey( const Subkey & other );
    ~Subkey();

    const Subkey & operator=( const Subkey & other );

    bool isNull() const;

    Key parent() const;

    const char * keyID() const;
    const char * fingerprint() const;

    time_t creationTime() const;
    time_t expirationTime() const;
    bool neverExpires() const;

    bool isRevoked() const;
    bool isExpired() const;
    bool isInvalid() const;
    bool isDisabled() const;

    bool canEncrypt() const;
    bool canSign() const;
    bool canCertify() const;
    bool canAuthenticate() const;

    bool isSecred() const;

    unsigned int publicKeyAlgorithm() const;
    const char * publicKeyAlgorithmAsString() const;

    unsigned int length() const;

  private:
    class Private;
    Private * d;
  };

  //
  // class UserID
  //

  class KDE_EXPORT UserID {
  public:
    class Signature;

    UserID( gpgme_key_t key=0, gpgme_user_id_t uid=0 );
    UserID( gpgme_key_t key, unsigned int idx );
    UserID( const UserID & other );
    ~UserID();

    const UserID & operator=( const UserID & other );

    bool isNull() const;

    Key parent() const;

    unsigned int numSignatures() const;
    Signature signature( unsigned int index ) const;
    std::vector<Signature> signatures() const;

    const char * id() const;
    const char * name() const;
    const char * email() const;
    const char * comment() const;

    enum Validity { Unknown=0, Undefined=1, Never=2,
		    Marginal=3, Full=4, Ultimate=5 };

    Validity validity() const;
    char validityAsString() const;

    bool isRevoked() const;
    bool isInvalid() const;

  private:
    class Private;
    Private * d;
  };

  //
  // class UserID::Signature
  //

  class KDE_EXPORT UserID::Signature {
  public:
    class Notation;

    Signature( gpgme_key_t key=0, gpgme_user_id_t uid=0, gpgme_key_sig_t sig=0 );
    Signature( gpgme_key_t key, gpgme_user_id_t uid, unsigned int idx );
    Signature( const Signature & other );
    ~Signature();

    const Signature & operator=( const Signature & other );

    bool isNull() const;

    UserID parent() const;

    const char * signerKeyID() const;

    const char * algorithmAsString() const;
    unsigned int algorithm() const;
    time_t creationTime() const;
    time_t expirationTime() const;
    bool neverExpires() const;

    bool isRevokation() const;
    bool isInvalid() const;
    bool isExpired() const;
    bool isExportable() const;

    const char * signerUserID() const;
    const char * signerName() const;
    const char * signerEmail() const;
    const char * signerComment() const;

    unsigned int certClass() const;

    enum Status { NoError = 0, SigExpired, KeyExpired,
		  BadSignature, NoPublicKey, GeneralError };
    Status status() const;
    const char * statusAsString() const;

    const char * policyURL() const;

    unsigned int numNotations() const;
    Notation notation( unsigned int idx ) const;
    std::vector<Notation> notations() const;

  private:
    class Private;
    Private * d;
  };

  //
  //
  // class UserID::Signature::Notation
  //
  //

  class KDE_EXPORT UserID::Signature::Notation {
  public:
    Notation( gpgme_key_t key=0, gpgme_user_id_t uid=0,
	      gpgme_key_sig_t sig=0, gpgme_sig_notation_t nota=0 );
    Notation( gpgme_key_t key, gpgme_user_id_t uid,
	      gpgme_key_sig_t sig, unsigned int idx );
    Notation( const Notation & other );
    ~Notation();

    const Notation & operator=( const Notation & other );

    bool isNull() const;

    Signature parent() const;

    const char * name() const;
    const char * value() const;

  private:
    class Private;
    Private * d;
  };

} // namespace GpgME

#endif // __GPGMEPP_KEY_H__
