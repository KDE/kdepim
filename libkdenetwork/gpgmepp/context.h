/* context.h - wraps a gpgme key context
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

// -*- c++ -*-
#ifndef __GPGMEPP_CONTEXT_H__
#define __GPGMEPP_CONTEXT_H__

#include <gpgmepp/gpgmefw.h>

#include <vector>
#include <utility>
#include <kdepimmacros.h>

namespace GpgME {

  class Key;
  class Data;
  class TrustItem;
  class ProgressProvider;
  class PassphraseProvider;
  class EventLoopInteractor;

  class KeyListResult;
  class KeyGenerationResult;
  class ImportResult;
  class DecryptionResult;
  class VerificationResult;
  class SigningResult;
  class EncryptionResult;

  class EngineInfo;

  class KDE_EXPORT Error {
  public:
    Error( int e=0 ) : mErr( e ) {}

    const char * source() const;
    const char * asString() const;

    int code() const;
    int sourceID() const;

    bool isCanceled() const;

    operator int() const { return mErr; }
    operator bool() const { return mErr && !isCanceled(); }
  private:
    int mErr;
  };

  class KDE_EXPORT Context {
    Context( gpgme_ctx_t );
  public:
    enum Protocol { OpenPGP, CMS, Unknown };

    //
    // Creation and destruction:
    //

    static Context * createForProtocol( Protocol proto );
    virtual ~Context();

    //
    // Context Attributes
    //

    Protocol protocol() const;

    void setArmor( bool useArmor );
    bool armor() const;

    void setTextMode( bool useTextMode );
    bool textMode() const;

    enum CertificateInclusion {
      AllCertificatesExceptRoot = -2,
      AllCertificates = -1,
      NoCertificates = 0,
      OnlySenderCertificate = 1
    };
    void setIncludeCertificates( int which );
    int includeCertificates() const;

    enum KeyListMode {
      Local = 0x1,
      Extern = 0x2,
      Signatures = 0x4,
      Validate = 0x10
    };
    void setKeyListMode( unsigned int keyListMode );
    void addKeyListMode( unsigned int keyListMode );
    unsigned int keyListMode() const;

    void setPassphraseProvider( PassphraseProvider * provider );
    PassphraseProvider * passphraseProvider() const;

    void setProgressProvider( ProgressProvider * provider );
    ProgressProvider * progressProvider() const;

    void setManagedByEventLoopInteractor( bool managed );
    bool managedByEventLoopInteractor() const;

    GpgME::Error setLocale( int category, const char * value );

  private:
    friend class EventLoopInteractor;
    void installIOCallbacks( gpgme_io_cbs * iocbs );
    void uninstallIOCallbacks();

  public:
    //
    //
    // Key Management
    //
    //

    //
    // Key Listing
    //

    GpgME::Error startKeyListing( const char * pattern=0, bool secretOnly=false );
    GpgME::Error startKeyListing( const char * patterns[], bool secretOnly=false );

    Key nextKey( GpgME::Error & e );
    
    KeyListResult endKeyListing();
    KeyListResult keyListResult() const;

    Key key( const char * fingerprint, GpgME::Error & e, bool secret=false );

    //
    // Key Generation
    //

    KeyGenerationResult generateKey( const char * parameters, Data & pubKey );
    GpgME::Error startKeyGeneration( const char * parameters, Data & pubkey );
    KeyGenerationResult keyGenerationResult() const;

    //
    // Key Export
    //

    GpgME::Error exportPublicKeys( const char * pattern, Data & keyData );
    GpgME::Error exportPublicKeys( const char * pattern[], Data & keyData );
    GpgME::Error startPublicKeyExport( const char * pattern, Data & keyData );
    GpgME::Error startPublicKeyExport( const char * pattern[], Data & keyData );

    //
    // Key Import
    //

    ImportResult importKeys( const Data & data );
    GpgME::Error startKeyImport( const Data & data );
    ImportResult importResult() const;

    //
    // Key Deletion
    //

    GpgME::Error deleteKey( const Key & key, bool allowSecretKeyDeletion=false );
    GpgME::Error startKeyDeletion( const Key & key, bool allowSecretKeyDeletion=false );

    //
    // Trust Item Management
    //    

    GpgME::Error startTrustItemListing( const char * pattern, int maxLevel );
    TrustItem nextTrustItem( GpgME::Error & e );
    GpgME::Error endTrustItemListing();

    //
    //
    // Crypto Operations
    //
    //

    //
    // Decryption
    //

    DecryptionResult decrypt( const Data & cipherText, Data & plainText );
    GpgME::Error startDecryption( const Data & cipherText, Data & plainText );
    DecryptionResult decryptionResult() const;

    //
    // Signature Verification
    //

    VerificationResult verifyDetachedSignature( const Data & signature, const Data & signedText );
    VerificationResult verifyOpaqueSignature( const Data & signedData, Data & plainText );
    GpgME::Error startDetachedSignatureVerification( const Data & signature, const Data & signedText );
    GpgME::Error startOpaqueSignatureVerification( const Data & signedData, Data & plainText );
    VerificationResult verificationResult() const;

    //
    // Combined Decryption and Signature Verification
    //

    std::pair<DecryptionResult,VerificationResult> decryptAndVerify( const Data & cipherText, Data & plainText );
    GpgME::Error startCombinedDecryptionAndVerification( const Data & cipherText, Data & plainText );
    // use verificationResult() and decryptionResult() to retrieve the result objects...

    //
    // Signing
    //

    void clearSigningKeys();
    GpgME::Error addSigningKey( const Key & signer );
    Key signingKey( unsigned int index ) const;

    enum SignatureMode { Normal, Detached, Clearsigned };
    SigningResult sign( const Data & plainText, Data & signature, SignatureMode mode );
    GpgME::Error startSigning( const Data & plainText, Data & signature, SignatureMode mode );
    SigningResult signingResult() const;

    //
    // Encryption
    //

    enum EncryptionFlags { None=0, AlwaysTrust=1 };
    EncryptionResult encrypt( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags );
    GpgME::Error encryptSymmetrically( const Data & plainText, Data & cipherText );
    GpgME::Error startEncryption( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags );
    EncryptionResult encryptionResult() const;

    //
    // Combined Signing and Encryption
    //

    std::pair<SigningResult,EncryptionResult> signAndEncrypt( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags );
    GpgME::Error startCombinedSigningAndEncryption( const std::vector<Key> & recipients, const Data & plainText, Data & cipherText, EncryptionFlags flags );
    // use encryptionResult() and signingResult() to retrieve the result objects...

    //
    //
    // Run Control
    //
    //

    bool poll();
    GpgME::Error wait();
    GpgME::Error lastError() const;
    GpgME::Error cancelPendingOperation();

    class Private;
    Private * impl() const { return d; }
  private:
    Private * d;

  private: // disable...
    Context( const Context & );
    const Context & operator=( const Context & );
  };

  //
  //
  // Globals
  //
  //

  KDE_EXPORT GpgME::Error setDefaultLocale( int category, const char * value );

  Context * wait( GpgME::Error & e, bool hang=true );
  typedef void (*IdleFunction)(void);
  IdleFunction registerIdleFunction( IdleFunction idleFunction );

  typedef void (*IOCallback)( void * data, int fd );

  KDE_EXPORT EngineInfo engineInfo( Context::Protocol proto );

  KDE_EXPORT GpgME::Error checkEngine( Context::Protocol proto );

} // namespace GpgME

#endif // __GPGMEPP_CONTEXT_H__
