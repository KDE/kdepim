/* key.cpp - wraps a gpgme key
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/key.h>

#include <gpgme.h>

namespace GpgME {

  using std::vector;

  struct Key::Private {
    Private( gpgme_key_t aKey )
      : key( aKey ) {}
    gpgme_key_t key;
  };

  Key::Key() {
    d = new Private( 0 );
  }

  Key::Key( gpgme_key_t key, bool ref ) {
    d = new Private( key );
    if ( ref && d->key )
      gpgme_key_ref( d->key );
  }

  Key::Key( const Key & other ) {
    d = new Private( other.d->key );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  Key::~Key() {
    if ( d->key )
      gpgme_key_unref( d->key );
    delete d; d = 0;
  }

  const Key & Key::operator=( const Key & other ) {
    if ( d == other.d ) return *this;

    if ( other.d->key )
      gpgme_key_ref( other.d->key );
    if ( d->key )
      gpgme_key_unref( d->key );
    *d = *other.d;
    return *this;
  }

  bool Key::isNull() const {
    return d->key == 0;
  }

  gpgme_key_t Key::impl() const {
    return d->key;
  }



  UserID Key::userID( unsigned int index ) const {
    return UserID( d->key, index );
  }

  Subkey Key::subkey( unsigned int index ) const {
    return Subkey( d->key, index );
  }


  unsigned int Key::numUserIDs() const {
    if ( !d->key )
      return 0;
    unsigned int count = 0;
    for ( gpgme_user_id_t uid = d->key->uids ; uid ; uid = uid->next )
      ++count;
    return count;
  }

  unsigned int Key::numSubkeys() const {
    if ( !d->key )
      return 0;
    unsigned int count = 0;
    for ( gpgme_sub_key_t subkey = d->key->subkeys ; subkey ; subkey = subkey->next )
      ++count;
    return count;
  }

  vector<UserID> Key::userIDs() const {
    if ( !d->key )
      return vector<UserID>();

    vector<UserID> v;
    v.reserve( numUserIDs() );
    for ( gpgme_user_id_t uid = d->key->uids ; uid ; uid = uid->next )
      v.push_back( UserID( d->key, uid ) );
    return v;
  }

  vector<Subkey> Key::subkeys() const {
    if ( !d->key )
      return vector<Subkey>();

    vector<Subkey> v;
    v.reserve( numSubkeys() );
    for ( gpgme_sub_key_t subkey = d->key->subkeys ; subkey ; subkey = subkey->next )
      v.push_back( Subkey( d->key, subkey ) );
    return v;
  }

  Key::OwnerTrust Key::ownerTrust() const {
    if ( !d->key )
      return Unknown;
    switch ( d->key->owner_trust ) {
    default:
    case GPGME_VALIDITY_UNKNOWN:   return Unknown;
    case GPGME_VALIDITY_UNDEFINED: return Undefined;
    case GPGME_VALIDITY_NEVER:     return Never;
    case GPGME_VALIDITY_MARGINAL:  return Marginal;
    case GPGME_VALIDITY_FULL:     return Full;
    case GPGME_VALIDITY_ULTIMATE: return Ultimate;
    }
  }
  char Key::ownerTrustAsString() const {
    if ( !d->key )
      return '?';
    switch ( d->key->owner_trust ) {
    default:
    case GPGME_VALIDITY_UNKNOWN:   return '?';
    case GPGME_VALIDITY_UNDEFINED: return 'q';
    case GPGME_VALIDITY_NEVER:     return 'n';
    case GPGME_VALIDITY_MARGINAL:  return 'm';
    case GPGME_VALIDITY_FULL:     return 'f';
    case GPGME_VALIDITY_ULTIMATE: return 'u';
    }
  }

#if 0 // declaration in key.h gives syntax error...
  Context::Protocol Key::protocol() const {
    if ( !d->key )
      return Context::Unknown;
    switch ( d->key->protocol ) {
    case GPGME_PROTOCOL_CMS:     return Context::CMS;
    case GPGME_PROTOCOL_OpenPGP: return Context::OpenPGP;
    default:                     return Context::Unknown;
    }
  }
#endif

  const char * Key::protocolAsString() const {
    return d->key ? gpgme_get_protocol_name( d->key->protocol ) : 0 ;
  }

  bool Key::isRevoked() const {
    return d->key && d->key->revoked;
  }

  bool Key::isExpired() const {
    return d->key && d->key->expired;
  }

  bool Key::isDisabled() const {
    return d->key && d->key->disabled;
  }

  bool Key::isInvalid() const {
    return d->key && d->key->invalid;
  }

  bool Key::isSecret() const {
    return d->key && d->key->secret;
  }

  bool Key::canEncrypt() const {
    return d->key && d->key->can_encrypt;
  }

  bool Key::canSign() const {
    return d->key && d->key->can_sign;
  }

  bool Key::canCertify() const {
    return d->key && d->key->can_certify;
  }

  bool Key::canAuthenticate() const {
    return d->key && d->key->can_authenticate;
  }

  const char * Key::issuerSerial() const {
    return d->key ? d->key->issuer_serial : 0 ;
  }
  const char * Key::issuerName() const {
    return d->key ? d->key->issuer_name : 0 ;
  }
  const char * Key::chainID() const {
    return d->key ? d->key->chain_id : 0 ;
  }

  //
  //
  // class Subkey
  //
  //

  struct Subkey::Private {
    Private( gpgme_key_t aKey, unsigned int idx )
      : key( aKey ), subkey( 0 )
    {
      if ( key )
	for ( gpgme_sub_key_t s = key->subkeys ; s ; s = s->next, --idx )
	  if ( idx == 0 ) {
	    subkey = s;
	    break;
	  }
      if ( !subkey )
	key = 0;
    }

    Private( gpgme_key_t aKey, gpgme_sub_key_t aSubkey )
      : key( aKey ), subkey( 0 )
    {
      if ( key )
	for ( gpgme_sub_key_t s = key->subkeys ; s ; s = s->next )
	  if ( s == aSubkey ) { // verify this subkey really belongs to this key
	    subkey = aSubkey;
	    break;
	  }
      if ( !subkey )
	key = 0;
    }

    gpgme_key_t key;
    gpgme_sub_key_t subkey;
  };

  Subkey::Subkey( gpgme_key_t key, unsigned int idx ) {
    d = new Private( key, idx );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  Subkey::Subkey( gpgme_key_t key, gpgme_sub_key_t subkey ) {
    d = new Private( key, subkey );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  Subkey::Subkey( const Subkey & other ) {
    d = new Private( other.d->key, other.d->subkey );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  Subkey::~Subkey() {
    if ( d->key )
      gpgme_key_unref( d->key );
    delete d; d = 0;
  }

  const Subkey & Subkey::operator=( const Subkey & other ) {
    if ( &other == this ) return *this;

    if ( other.d->key )
      gpgme_key_ref( other.d->key );
    if ( d->key )
      gpgme_key_unref( d->key );
    *d = *other.d;
    return *this;
  }

  bool Subkey::isNull() const {
    return !d || !d->key || !d->subkey;
  }

  Key Subkey::parent() const {
    return Key( d->key, true );
  }

  const char * Subkey::keyID() const {
    return d->subkey ? d->subkey->keyid : 0 ;
  }

  const char * Subkey::fingerprint() const {
    return d->subkey ? d->subkey->fpr : 0 ;
  }

  unsigned int Subkey::publicKeyAlgorithm() const {
    return d->subkey ? d->subkey->pubkey_algo : 0 ;
  }

  const char * Subkey::publicKeyAlgorithmAsString() const {
    return gpgme_pubkey_algo_name( d->subkey ? d->subkey->pubkey_algo : (gpgme_pubkey_algo_t)0 );
  }

  bool Subkey::canEncrypt() const {
    return d->subkey && d->subkey->can_encrypt;
  }

  bool Subkey::canSign() const {
    return d->subkey && d->subkey->can_sign;
  }

  bool Subkey::canCertify() const {
    return d->subkey && d->subkey->can_certify;
  }
  
  bool Subkey::canAuthenticate() const {
    return d->subkey && d->subkey->can_authenticate;
  }

  unsigned int Subkey::length() const {
    return d->subkey ? d->subkey->length : 0 ;
  }

  time_t Subkey::creationTime() const {
    return static_cast<time_t>( d->subkey ? d->subkey->timestamp : 0 );
  }

  time_t Subkey::expirationTime() const {
    return static_cast<time_t>( d->subkey ? d->subkey->expires : 0 );
  }

  bool Subkey::neverExpires() const {
    return expirationTime() == time_t(0);
  }

  bool Subkey::isRevoked() const {
    return d->subkey && d->subkey->revoked;
  }

  bool Subkey::isInvalid() const {
    return d->subkey && d->subkey->invalid;
  }

  bool Subkey::isExpired() const {
    return d->subkey && d->subkey->expired;
  }

  bool Subkey::isDisabled() const {
    return d->subkey && d->subkey->disabled;
  }

  //
  //
  // class UserID
  //
  //

  struct UserID::Private {
    Private( gpgme_key_t aKey, unsigned int idx )
      : key( aKey ), uid( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next, --idx )
	  if ( idx == 0 ) {
	    uid = u;
	    break;
	  }
      if ( !uid )
	key = 0;
    }

    Private( gpgme_key_t aKey, gpgme_user_id_t aUid )
      : key( aKey ), uid( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next )
	  if ( u == aUid ) {
	    uid = u;
	    break;
	  }
      if ( !uid )
	key = 0;
    }

    gpgme_key_t key;
    gpgme_user_id_t uid;
  };

  UserID::UserID( gpgme_key_t key, gpgme_user_id_t uid ) {
    d = new Private( key, uid );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::UserID( gpgme_key_t key, unsigned int idx ) {
    d = new Private( key, idx );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::UserID( const UserID & other ) {
    d = new Private( other.d->key, other.d->uid );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::~UserID() {
    if ( d->key )
      gpgme_key_unref( d->key );
    delete d; d = 0;
  }

  const UserID & UserID::operator=( const UserID & other ) {
    if ( &other == this ) return *this;

    if ( other.d->key )
      gpgme_key_ref( other.d->key );
    if ( d->key )
      gpgme_key_unref( d->key );
    *d = *other.d;
    return *this;
  }

  bool UserID::isNull() const {
    return !d || !d->key || !d->uid;
  }

  Key UserID::parent() const {
    return Key( d->key, true );
  }

  UserID::Signature UserID::signature( unsigned int index ) const {
    return Signature( d->key, d->uid, index );
  }

  unsigned int UserID::numSignatures() const {
    if ( !d->uid )
      return 0;
    unsigned int count = 0;
    for ( gpgme_key_sig_t sig = d->uid->signatures ; sig ; sig = sig->next )
      ++count;
    return count;
  }

  vector<UserID::Signature> UserID::signatures() const {
    if ( !d->uid )
      return vector<Signature>();

    vector<Signature> v;
    v.reserve( numSignatures() );
    for ( gpgme_key_sig_t sig = d->uid->signatures ; sig ; sig = sig->next )
      v.push_back( Signature( d->key, d->uid, sig ) );
    return v;
  }

  const char * UserID::id() const {
    return d->uid ? d->uid->uid : 0 ;
  }

  const char * UserID::name() const {
    return d->uid ? d->uid->name : 0 ;
  }

  const char * UserID::email() const {
    return d->uid ? d->uid->email : 0 ;
  }

  const char * UserID::comment() const {
    return d->uid ? d->uid->comment : 0 ;
  }

  UserID::Validity UserID::validity() const {
    if ( !d->uid )
      return Unknown;
    switch ( d->uid->validity ) {
    default:
    case GPGME_VALIDITY_UNKNOWN:   return Unknown;
    case GPGME_VALIDITY_UNDEFINED: return Undefined;
    case GPGME_VALIDITY_NEVER:     return Never;
    case GPGME_VALIDITY_MARGINAL:  return Marginal;
    case GPGME_VALIDITY_FULL:      return Full;
    case GPGME_VALIDITY_ULTIMATE:  return Ultimate;
    }
  }

  char UserID::validityAsString() const {
    if ( !d->uid )
      return '?';
    switch ( d->uid->validity ) {
    default:
    case GPGME_VALIDITY_UNKNOWN:   return '?';
    case GPGME_VALIDITY_UNDEFINED: return 'q';
    case GPGME_VALIDITY_NEVER:     return 'n';
    case GPGME_VALIDITY_MARGINAL:  return 'm';
    case GPGME_VALIDITY_FULL:      return 'f';
    case GPGME_VALIDITY_ULTIMATE:  return 'u';
    }
  }

  bool UserID::isRevoked() const {
    return d->uid && d->uid->revoked;
  }

  bool UserID::isInvalid() const {
    return d->uid && d->uid->invalid;
  }

  //
  //
  // class Signature
  //
  //

  struct UserID::Signature::Private {
    Private( gpgme_key_t aKey, gpgme_user_id_t aUid, unsigned int idx )
      : key( key ), uid( 0 ), sig( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next )
	  if ( u == aUid ) {
	    uid = u;
	    for ( gpgme_key_sig_t s = uid->signatures ; s ; s = s->next, --idx )
	      if ( idx == 0 ) {
		sig = s;
		break;
	      }
	    break;
	  }
      if ( !uid || !sig ) {
	uid = 0;
	sig = 0;
	key = 0;
      }
    }
	  
    Private( gpgme_key_t aKey, gpgme_user_id_t aUid, gpgme_key_sig_t aSig )
      : key( key ), uid( 0 ), sig( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next )
	  if ( u == aUid ) {
	    uid = u;
	    for ( gpgme_key_sig_t s = uid->signatures ; s ; s = s->next )
	      if ( s == aSig ) {
		sig = s;
		break;
	      }
	    break;
	  }
      if ( !uid || !sig ) {
	uid = 0;
	sig = 0;
	key = 0;
      }
    }
	  
    gpgme_key_t key;
    gpgme_user_id_t uid;
    gpgme_key_sig_t sig;
  };

  UserID::Signature::Signature( gpgme_key_t key, gpgme_user_id_t uid, unsigned int idx ) {
    d = new Private( key, uid, idx );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::Signature( gpgme_key_t key, gpgme_user_id_t uid, gpgme_key_sig_t sig ) {
    d = new Private( key, uid, sig );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::Signature( const Signature & other ) {
    d = new Private( other.d->key, other.d->uid, other.d->sig );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::~Signature() {
    if ( d->key )
      gpgme_key_unref( d->key );
    delete d; d = 0;
  }

  const UserID::Signature & UserID::Signature::operator=( const Signature & other ) {
    if ( &other == this ) return *this;

    if ( other.d->key )
      gpgme_key_ref( other.d->key );
    if ( d->key )
      gpgme_key_unref( d->key );
    *d = *other.d;
    return *this;
  }

  bool UserID::Signature::isNull() const {
    return !d || !d->key || !d->uid || !d->sig;
  }

  UserID UserID::Signature::parent() const {
    return UserID( d->key, d->uid );
  }

  const char * UserID::Signature::signerKeyID() const {
    return d->sig ? d->sig->keyid : 0 ;
  }

  const char * UserID::Signature::algorithmAsString() const {
    return gpgme_pubkey_algo_name( d->sig ? d->sig->pubkey_algo : (gpgme_pubkey_algo_t)0 );
  }

  unsigned int UserID::Signature::algorithm() const {
    return d->sig ? d->sig->pubkey_algo : 0 ;
  }

  time_t UserID::Signature::creationTime() const {
    return static_cast<time_t>( d->sig ? d->sig->timestamp : 0 );
  }

  time_t UserID::Signature::expirationTime() const {
    return static_cast<time_t>( d->sig ? d->sig->expires : 0 );
  }

  bool UserID::Signature::neverExpires() const {
    return expirationTime() == time_t(0);
  }

  bool UserID::Signature::isRevokation() const {
    return d->sig && d->sig->revoked;
  }

  bool UserID::Signature::isInvalid() const {
    return d->sig && d->sig->invalid;
  }

  bool UserID::Signature::isExpired() const {
    return d->sig && d->sig->expired;
  }

  bool UserID::Signature::isExportable() const {
    return d->sig && d->sig->exportable;
  }

  const char * UserID::Signature::signerUserID() const {
    return d->sig ? d->sig->uid : 0 ;
  }

  const char * UserID::Signature::signerName() const {
    return d->sig ? d->sig->name : 0 ;
  }

  const char * UserID::Signature::signerEmail() const {
    return d->sig ? d->sig->email : 0 ;
  }

  const char * UserID::Signature::signerComment() const {
    return d->sig ? d->sig->comment : 0 ;
  }

  unsigned int UserID::Signature::certClass() const {
    return d->sig ? d->sig->sig_class : 0 ;
  }

  UserID::Signature::Status UserID::Signature::status() const {
    if ( !d->sig )
      return GeneralError;
    switch ( d->sig->status ) {
    case GPG_ERR_NO_ERROR:      return NoError;
    case GPG_ERR_SIG_EXPIRED:   return SigExpired;
    case GPG_ERR_KEY_EXPIRED:   return KeyExpired;
    case GPG_ERR_BAD_SIGNATURE: return BadSignature;
    case GPG_ERR_NO_PUBKEY:     return NoPublicKey;
    default:
    case GPG_ERR_GENERAL:       return GeneralError;
    }
  }

  const char * UserID::Signature::statusAsString() const {
    return d->sig ? gpgme_strerror( d->sig->status ) : 0 ;
  }

  UserID::Signature::Notation UserID::Signature::notation( unsigned int idx ) const {
    return Notation( d->key, d->uid, d->sig, idx );
  }

  unsigned int UserID::Signature::numNotations() const {
    if ( !d->sig )
      return 0;
    unsigned int count = 0;
#ifdef HAVE_GPGME_KEY_SIG_NOTATIONS
    for ( gpgme_sig_notation_t nota = d->sig->notations ; nota ; nota = nota->next )
      if ( nota->name ) ++count; // others are policy URLs...
#endif
    return count;
  }
      
  vector<UserID::Signature::Notation> UserID::Signature::notations() const {
    if ( !d->sig )
      return vector<Notation>();
    vector<Notation> v;
#ifdef HAVE_GPGME_KEY_SIG_NOTATIONS
    v.reserve( numNotations() );
    for ( gpgme_sig_notation_t nota = d->sig->notations ; nota ; nota = nota->next )
      if ( nota->name )
	v.push_back( Notation( d->key, d->uid, d->sig, nota ) );
#endif
    return v;
  }

  const char * UserID::Signature::policyURL() const {
#ifdef HAVE_GPGME_KEY_SIG_NOTATIONS
    if ( !d->sig )
      return 0;
    for ( gpgme_sig_notation_t nota = d->sig->notations ; nota ; nota = nota->next )
      if ( !nota->name )
	return nota->value;
#endif
    return 0;
  }


  
  //
  //
  // class Notation
  //
  //

  struct UserID::Signature::Notation::Private {
    Private( gpgme_key_t aKey, gpgme_user_id_t aUid,
	     gpgme_key_sig_t aSig, unsigned int idx )
      : key( key ), uid( 0 ), sig( 0 ), nota( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next )
	  if ( u == aUid ) {
	    uid = u;
	    for ( gpgme_key_sig_t s = uid->signatures ; s ; s = s->next )
	      if ( s == aSig ) {
		sig = s;
#ifdef HAVE_GPGME_KEY_SIG_NOTATIONS
		for ( gpgme_sig_notation_t n = sig->notations ; n ; n = n->next, --idx )
		  if ( n == aNota ) {
		    nota = n;
		    break;
		  }
#endif
		break;
	      }
	    break;
	  }
      if ( !uid || !sig || !nota ) {
	uid = 0;
	sig = 0;
	key = 0;
	nota = 0;
      }
    }
	  
    Private( gpgme_key_t aKey, gpgme_user_id_t aUid,
	     gpgme_key_sig_t aSig, gpgme_sig_notation_t aNota )
      : key( key ), uid( 0 ), sig( 0 ), nota( 0 )
    {
      if ( key )
	for ( gpgme_user_id_t u = key->uids ; u ; u = u->next )
	  if ( u == aUid ) {
	    uid = u;
	    for ( gpgme_key_sig_t s = uid->signatures ; s ; s = s->next )
	      if ( s == aSig ) {
		sig = s;
#ifdef HAVE_GPGME_KEY_SIG_NOTATIONS
		for ( gpgme_sig_notation_t n = sig->notations ; n ; n = n->next )
		  if ( n == aNota ) {
		    nota = n;
		    break;
		  }
#endif
		break;
	      }
	    break;
	  }
      if ( !uid || !sig || !nota ) {
	uid = 0;
	sig = 0;
	key = 0;
	nota = 0;
      }
    }
	  
    gpgme_key_t key;
    gpgme_user_id_t uid;
    gpgme_key_sig_t sig;
    gpgme_sig_notation_t nota;
  };

  UserID::Signature::Notation::Notation( gpgme_key_t key, gpgme_user_id_t uid,
					 gpgme_key_sig_t sig, unsigned int idx ) {
    d = new Private( key, uid, sig, idx );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::Notation::Notation( gpgme_key_t key, gpgme_user_id_t uid,
					 gpgme_key_sig_t sig, gpgme_sig_notation_t nota ) {
    d = new Private( key, uid, sig, nota );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::Notation::Notation( const Notation & other ) {
    d = new Private( other.d->key, other.d->uid, other.d->sig, other.d->nota );
    if ( d->key )
      gpgme_key_ref( d->key );
  }

  UserID::Signature::Notation::~Notation() {
    if ( d->key )
      gpgme_key_unref( d->key );
    delete d; d = 0;
  }

  const UserID::Signature::Notation & UserID::Signature::Notation::operator=( const Notation & other ) {
    if ( &other == this ) return *this;

    if ( other.d->key )
      gpgme_key_ref( other.d->key );
    if ( d->key )
      gpgme_key_unref( d->key );
    *d = *other.d;
    return *this;
  }

  bool UserID::Signature::Notation::isNull() const {
    return !d || !d->key || !d->uid || !d->sig || !d->nota;
  }

  UserID::Signature UserID::Signature::Notation::parent() const {
    return Signature( d->key, d->uid, d->sig );
  }

  const char * UserID::Signature::Notation::name() const {
    return d->nota ? d->nota->name : 0 ;
  }

  const char * UserID::Signature::Notation::value() const {
    return d->nota ? d->nota->value : 0 ;
  }

} // namespace GpgME
