/* context.cpp - wraps a gpgme key context
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
   along with GPGME++; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gpgmepp/context.h>
#include <gpgmepp/eventloopinteractor.h>
#include <gpgmepp/trustitem.h>
#include <gpgmepp/keylistresult.h>
#include <gpgmepp/keygenerationresult.h>
#include <gpgmepp/importresult.h>
#include <gpgmepp/decryptionresult.h>
#include <gpgmepp/verificationresult.h>
#include <gpgmepp/signingresult.h>
#include <gpgmepp/encryptionresult.h>

#include "callbacks.h"
#include "data_p.h"
#include "context_p.h"

#include <gpgme.h>

//#include <string>
//using std::string;
#ifndef NDEBUG
#include <iostream>
using std::cerr;
using std::endl;
#endif

#include <cassert>

namespace GpgME {

  const char * Error::source() const {
    return gpgme_strsource( (gpgme_error_t)mErr );
  }

  const char * Error::asString() const {
    return gpgme_strerror( (gpgme_error_t)mErr );
  }

  Context::Context( gpgme_ctx_t ctx ) {
    d = new Private( ctx );
  }

  Context::~Context() {
    delete d; d = 0;
  }

  Context * Context::createForProtocol( Protocol proto ) {
    gpgme_ctx_t ctx = 0;
    if ( gpgme_new ( &ctx ) != 0 )
      return 0;

    switch ( proto ) {
    case OpenPGP:
      if ( gpgme_set_protocol( ctx, GPGME_PROTOCOL_OpenPGP ) != 0 ) {
	gpgme_release( ctx );
	return 0;
      }
      break;
    case CMS:
      if ( gpgme_set_protocol( ctx, GPGME_PROTOCOL_CMS ) != 0 ) {
	gpgme_release( ctx );
	return 0;
      }
      break;
    default:
      return 0;
    }

    return new Context( ctx );
  }

  //
  //
  // Context attributes:
  //
  //

  Context::Protocol Context::protocol() const {
    gpgme_protocol_t p = gpgme_get_protocol( d->ctx );
    switch ( p ) {
    case GPGME_PROTOCOL_OpenPGP: return OpenPGP;
    case GPGME_PROTOCOL_CMS:     return CMS;
    default:                     return Unknown;
    }
  }
    

  void Context::setArmor( bool useArmor ) {
    gpgme_set_armor( d->ctx, int( useArmor ) );
  }
  bool Context::armor() const {
    return gpgme_get_armor( d->ctx );
  }

  void Context::setTextMode( bool useTextMode ) {
    gpgme_set_textmode( d->ctx, int( useTextMode ) );
  }
  bool Context::textMode() const {
    return gpgme_get_textmode( d->ctx );
  }

  void Context::setIncludeCertificates( int which ) {
    assert( which >= -2 );
    gpgme_set_include_certs( d->ctx, which );
  }
  int Context::includeCertificates() const {
    return gpgme_get_include_certs( d->ctx );
  }

  void Context::setKeyListMode( KeyListMode mode ) {
    switch ( mode ) {
    case Local:
      gpgme_set_keylist_mode( d->ctx, GPGME_KEYLIST_MODE_LOCAL );
      break;
    case Extern:
      gpgme_set_keylist_mode( d->ctx, GPGME_KEYLIST_MODE_EXTERN );
      break;
    default:
#ifndef NDEBUG
      cerr << "GpgME::Context::setKeyListMode: mode must be one of Local or Extern!" << endl;
#else
      ;
#endif
    }
  }
  Context::KeyListMode Context::keyListMode() const {
    switch ( gpgme_get_keylist_mode( d->ctx ) ) {
    case GPGME_KEYLIST_MODE_LOCAL: return Local;
    case GPGME_KEYLIST_MODE_EXTERN: return Extern;
    default: assert( 0 );
    }
  }

  void Context::setProgressProvider( ProgressProvider * provider ) {
    gpgme_set_progress_cb( d->ctx, provider ? &progress_callback : 0, provider );
  }
  ProgressProvider * Context::progressProvider() const {
    void * pp = 0;
    gpgme_progress_cb_t pcb = &progress_callback;
    gpgme_get_progress_cb( d->ctx, &pcb, &pp );
    return static_cast<ProgressProvider*>( pp );
  }

  void Context::setPassphraseProvider( PassphraseProvider * provider ) {
    gpgme_set_passphrase_cb( d->ctx, provider ? &passphrase_callback : 0, provider );
  }

  PassphraseProvider * Context::passphraseProvider() const {
    void * pp = 0;
    gpgme_passphrase_cb_t pcb = &passphrase_callback;
    gpgme_get_passphrase_cb( d->ctx, &pcb, &pp );
    return static_cast<PassphraseProvider*>( pp );
  }

  void Context::setManagedByEventLoopInteractor( bool manage ) {
    if ( !EventLoopInteractor::instance() ) {
#ifndef NDEBUG
      cerr << "Context::setManagedByEventLoopInteractor(): "
	      "You must create an instance of EventLoopInteractor "
	      "before using anything that needs one." << endl;
#endif
      return;
    }
    if ( manage )
      EventLoopInteractor::instance()->manage( this );
    else
      EventLoopInteractor::instance()->unmanage( this );
  }
  bool Context::managedByEventLoopInteractor() const {
    return d->iocbs != 0;
  }
      

  void Context::installIOCallbacks( gpgme_io_cbs * iocbs ) {
    if ( !iocbs ) {
      uninstallIOCallbacks();
      return;
    }
    gpgme_set_io_cbs( d->ctx, iocbs );
    delete d->iocbs; d->iocbs = iocbs;
  }

  void Context::uninstallIOCallbacks() {
    static gpgme_io_cbs noiocbs = { 0, 0, 0, 0, 0 };
    // io.add == 0 means disable io callbacks:
    gpgme_set_io_cbs( d->ctx, &noiocbs );
    delete d->iocbs; d->iocbs = 0;
  }

  //
  //
  // Key Management
  //
  //

  Error Context::startKeyListing( const char * pattern, bool secretOnly ) {
    d->lastop = Private::KeyList;
    return gpgme_op_keylist_start( d->ctx, pattern, int( secretOnly ) );
  }

  Error Context::startKeyListing( const char * patterns[], bool secretOnly ) {
    d->lastop = Private::KeyList;
    return gpgme_op_keylist_ext_start( d->ctx, patterns, int( secretOnly ), 0 );
  }

  Key Context::nextKey( GpgME::Error & e ) {
    d->lastop = Private::KeyList;
    gpgme_key_t key;
    e = gpgme_op_keylist_next( d->ctx, &key );
    return Key( key, false );
  }

  KeyListResult Context::endKeyListing() {
    d->lasterr = gpgme_op_keylist_end( d->ctx );
    return KeyListResult( d->ctx, d->lasterr );
  }

  Key Context::key( const char * fingerprint, GpgME::Error & e , bool secret /*, bool forceUpdate*/ ) {
    d->lastop = Private::KeyList;
    gpgme_key_t key;
    e = gpgme_get_key( d->ctx, fingerprint, &key, int( secret )/*, int( forceUpdate )*/ );
    return Key( key, false );
  }

  KeyGenerationResult Context::generateKey( const char * parameters, Data & pubKey ) {
    d->lastop = Private::KeyGen;
    Data::Private * dp = pubKey.impl();
    d->lasterr = gpgme_op_genkey( d->ctx, parameters, dp ? dp->data : 0, 0 );
    return KeyGenerationResult( d->ctx, d->lasterr );
  }

  Error Context::startKeyGeneration( const char * parameters, Data & pubKey ) {
    d->lastop = Private::KeyGen;
    Data::Private * dp = pubKey.impl();
    return gpgme_op_genkey_start( d->ctx, parameters, dp ? dp->data : 0, 0 );
  }

  KeyGenerationResult Context::keyGenerationResult() const {
    if ( d->lastop & Private::KeyGen )
      return KeyGenerationResult( d->ctx, d->lasterr );
    else
      return KeyGenerationResult();
  }

  Error Context::exportPublicKeys( const char * pattern, Data & keyData ) {
    d->lastop = Private::Export;
    Data::Private * dp = keyData.impl();
    return gpgme_op_export( d->ctx, pattern, 0, dp ? dp->data : 0 );
  }

  Error Context::exportPublicKeys( const char * patterns[], Data & keyData ) {
    d->lastop = Private::Export;
    Data::Private * dp = keyData.impl();
    return gpgme_op_export_ext( d->ctx, patterns, 0, dp ? dp->data : 0 );
  }

  Error Context::startPublicKeyExport( const char * pattern, Data & keyData ) {
    d->lastop = Private::Export;
    Data::Private * dp = keyData.impl();
    return gpgme_op_export_start( d->ctx, pattern, 0, dp ? dp->data : 0 );
  }

  Error Context::startPublicKeyExport( const char * patterns[], Data & keyData ) {
    d->lastop = Private::Export;
    Data::Private * dp = keyData.impl();
    return gpgme_op_export_ext_start( d->ctx, patterns, 0, dp ? dp->data : 0 );
  }


  ImportResult Context::importKeys( const Data & data ) {
    d->lastop = Private::Import;
    Data::Private * dp = data.impl();
    d->lasterr = gpgme_op_import( d->ctx, dp ? dp->data : 0 );
    return ImportResult( d->ctx, d->lasterr );
  }

  Error Context::startKeyImport( const Data & data ) {
    d->lastop = Private::Import;
    Data::Private * dp = data.impl();
    return gpgme_op_import_start( d->ctx, dp ? dp->data : 0 );
  }

  ImportResult Context::importResult() const {
    if ( d->lastop & Private::Import )
      return ImportResult( d->ctx, d->lasterr );
    else
      return ImportResult();
  }

  Error Context::deleteKey( const Key & key, bool allowSecretKeyDeletion ) {
    d->lastop = Private::Delete;
    return gpgme_op_delete( d->ctx, key.impl(), int( allowSecretKeyDeletion ) );
  }

  Error Context::startKeyDeletion( const Key & key, bool allowSecretKeyDeletion ) {
    d->lastop = Private::Delete;
    return gpgme_op_delete_start( d->ctx, key.impl(), int( allowSecretKeyDeletion ) );
  }

  Error Context::startTrustItemListing( const char * pattern, int maxLevel ) {
    d->lastop = Private::TrustList;
    return gpgme_op_trustlist_start( d->ctx, pattern, maxLevel );
  }

  TrustItem Context::nextTrustItem( Error & e ) {
    gpgme_trust_item_t ti = 0;
    e = gpgme_op_trustlist_next( d->ctx, &ti );
    return ti;
  }

  Error Context::endTrustItemListing() {
    return gpgme_op_trustlist_end( d->ctx );
  }

  DecryptionResult Context::decrypt( const Data & cipherText, Data & plainText ) {
    d->lastop = Private::Decrypt;
    Data::Private * cdp = cipherText.impl();
    Data::Private * pdp = plainText.impl();
    d->lasterr = gpgme_op_decrypt( d->ctx, cdp ? cdp->data : 0, pdp ? pdp->data : 0 );
    return DecryptionResult( d->ctx, d->lasterr );
  }

  Error Context::startDecryption( const Data & cipherText, Data & plainText ) {
    d->lastop = Private::Decrypt;
    Data::Private * cdp = cipherText.impl();
    Data::Private * pdp = plainText.impl();
    return gpgme_op_decrypt_start( d->ctx, cdp ? cdp->data : 0, pdp ? pdp->data : 0 );
  }

  DecryptionResult Context::decryptionResult() const {
    if ( d->lastop & Private::Decrypt )
      return DecryptionResult( d->ctx, d->lasterr );
    else
      return DecryptionResult();
  }



  VerificationResult Context::verifyDetachedSignature( const Data & signature, const Data & signedText ) {
    d->lastop = Private::Verify;
    Data::Private * sdp = signature.impl();
    Data::Private * tdp = signedText.impl();
    d->lasterr = gpgme_op_verify( d->ctx, sdp ? sdp->data : 0, tdp ? tdp->data : 0, 0 );
    return VerificationResult( d->ctx, d->lasterr );
  }

  VerificationResult Context::verifyOpaqueSignature( const Data & signedData, Data & plainText ) {
    d->lastop = Private::Verify;
    Data::Private * sdp = signedData.impl();
    Data::Private * pdp = plainText.impl();
    d->lasterr = gpgme_op_verify( d->ctx, sdp ? sdp->data : 0, 0, pdp ? pdp->data : 0 );
    return VerificationResult( d->ctx, d->lasterr );
  }

  Error Context::startDetachedSignatureVerification( const Data & signature, const Data & signedText ) {
    d->lastop = Private::Verify;
    Data::Private * sdp = signature.impl();
    Data::Private * tdp = signedText.impl();
    return gpgme_op_verify_start( d->ctx, sdp ? sdp->data : 0, tdp ? tdp->data : 0, 0 );
  }

  Error Context::startOpaqueSignatureVerification( const Data & signedData, Data & plainText ) {
    d->lastop = Private::Verify;
    Data::Private * sdp = signedData.impl();
    Data::Private * pdp = plainText.impl();
    return gpgme_op_verify_start( d->ctx, sdp ? sdp->data : 0, 0, pdp ? pdp->data : 0 );
  }

  VerificationResult Context::verificationResult() const {
    if ( d->lastop & Private::Verify )
      return VerificationResult( d->ctx, d->lasterr );
    else
      return VerificationResult();
  }




  void Context::clearSigningKeys() {
    gpgme_signers_clear( d->ctx );
  }

  Error Context::addSigningKey( const Key & key ) {
    return gpgme_signers_add( d->ctx, key.impl() );
  }

  Key Context::signingKey( unsigned int idx ) const {
    gpgme_key_t key = gpgme_signers_enum( d->ctx, idx );
    return Key( key, false );
  }


  static gpgme_sig_mode_t sigmode2sigmode( Context::SignatureMode mode ) {
    switch ( mode ) {
    default:
    case Context::Normal:      return GPGME_SIG_MODE_NORMAL;
    case Context::Detached:    return GPGME_SIG_MODE_DETACH;
    case Context::Clearsigned: return GPGME_SIG_MODE_CLEAR;
    }
  }

  SigningResult Context::sign( const Data & plainText, Data & signature, SignatureMode mode ) {
    d->lastop = Private::Sign;
    Data::Private * pdp = plainText.impl();
    Data::Private * sdp = signature.impl();
    d->lasterr = gpgme_op_sign( d->ctx, pdp ? pdp->data : 0, sdp ? sdp->data : 0, sigmode2sigmode( mode ) );
    return SigningResult( d->ctx, d->lasterr );
  }


  Error Context::startSigning( const Data & plainText, Data & signature, SignatureMode mode ) {
    d->lastop = Private::Sign;
    Data::Private * pdp = plainText.impl();
    Data::Private * sdp = signature.impl();
    return gpgme_op_sign_start( d->ctx, pdp ? pdp->data : 0, sdp ? sdp->data : 0, sigmode2sigmode( mode ) );
  }

  SigningResult Context::signingResult() const {
    if ( d->lastop & Private::Sign )
      return SigningResult( d->ctx, d->lasterr );
    else
      return SigningResult();
  }


  EncryptionResult Context::encrypt( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags ) {
    d->lastop = Private::Encrypt;
    Data::Private * pdp = plainText.impl();
    Data::Private * cdp = cipherText.impl();
    gpgme_key_t * keys = new gpgme_key_t[ recipients.size() + 1 ];
    gpgme_key_t * keys_it = keys;
    for ( std::vector<Key>::const_iterator it = recipients.begin() ; it != recipients.end() ; ++it )
      if ( it->impl() )
	*keys_it++ = it->impl();
    *keys_it++ = 0;
    d->lasterr = gpgme_op_encrypt( d->ctx, keys,
				   flags & AlwaysTrust ? GPGME_ENCRYPT_ALWAYS_TRUST : (gpgme_encrypt_flags_t)0,
				   pdp ? pdp->data : 0, cdp ? cdp->data : 0 );
    delete[] keys;
    return EncryptionResult( d->ctx, d->lasterr );
  }

  Error Context::encryptSymmetrically( const Data & plainText, Data & cipherText ) {
    d->lastop = Private::Encrypt;
    Data::Private * pdp = plainText.impl();
    Data::Private * cdp = cipherText.impl();
    return d->lasterr = gpgme_op_encrypt( d->ctx, 0, (gpgme_encrypt_flags_t)0,
					  pdp ? pdp->data : 0, cdp ? cdp->data : 0 );
  }

  Error Context::startEncryption( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags ) {
    d->lastop = Private::Encrypt;
    Data::Private * pdp = plainText.impl();
    Data::Private * cdp = cipherText.impl();
    gpgme_key_t * keys = new gpgme_key_t[ recipients.size() + 1 ];
    gpgme_key_t * keys_it = keys;
    for ( std::vector<Key>::const_iterator it = recipients.begin() ; it != recipients.end() ; ++it )
      if ( it->impl() )
	*keys_it++ = it->impl();
    *keys_it++ = 0;
    gpgme_error_t e = gpgme_op_encrypt_start( d->ctx, keys,
					      flags & AlwaysTrust ? GPGME_ENCRYPT_ALWAYS_TRUST : (gpgme_encrypt_flags_t)0,
					      pdp ? pdp->data : 0, cdp ? cdp->data : 0 );
    delete[] keys;
    return e;
  }

  EncryptionResult Context::encryptionResult() const {
    if ( d->lastop & Private::Encrypt )
      return EncryptionResult( d->ctx, d->lasterr );
    else
      return EncryptionResult();
  }



  void Context::cancelPendingOperation() {
#ifdef HAVE_GPGME_CANCEL
    // missing in gpgme 0.4.x so far :/
    gpgme_cancel( d->ctx );
#endif
  }

  bool Context::poll() {
    gpgme_error_t e = GPG_ERR_NO_ERROR;
    const bool finished = gpgme_wait( d->ctx, &e, 0 );
    if ( finished )
      d->lasterr = e;
    return finished;
  }

  Error Context::wait() {
    gpgme_error_t e = GPG_ERR_NO_ERROR;
    gpgme_wait( d->ctx, &e, 1 );
    return d->lasterr = e;
  }

  Error Context::lastError() const {
    return d->lasterr;
  }

} // namespace GpgME
