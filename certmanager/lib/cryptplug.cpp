/* -*- Mode: C++ -*-

  this is a C++-ification of:
  GPGMEPLUG - an GPGME based cryptography plug-in following
              the common CRYPTPLUG specification.

  Copyright (C) 2001 by Klarälvdalens Datakonsult AB
  Copyright (C) 2002 g10 Code GmbH
  Copyright (C) 2004 Klarälvdalens Datakonsult AB

  GPGMEPLUG is free software; you can redistribute it and/or modify
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  GPGMEPLUG is distributed in the hope that it will be useful,
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kleo/oidmap.h"

#include <gpgmepp/context.h>
#include <gpgmepp/data.h>
#include <gpgmepp/importresult.h>

/*! \file gpgmeplug.c
    \brief GPGME implementation of CRYPTPLUG following the
    specification located in common API header cryptplug.h.

    CRYPTPLUG is an independent cryptography plug-in API
    developed for Sphinx-enabeling KMail and Mutt.

    CRYPTPLUG was designed for the Aegypten project, but it may
    be used by 3rd party developers as well to design pluggable
    crypto backends for the above mentioned MUAs.

    \note All string parameters appearing in this API are to be
    interpreted as UTF-8 encoded.

    \see cryptplug.h
*/

#include <qstring.h>

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <locale.h>

#define __GPGMEPLUG_ERROR_CLEARTEXT_IS_ZERO "Error: Cannot run checkMessageSignature() with cleartext == 0"

/* Note: The following specification will result in
       function encryptAndSignMessage() producing
       _empty_ mails.
       This must be changed as soon as our plugin
       is supporting the encryptAndSignMessage() function. */
#ifndef GPGMEPLUG_ENCSIGN_MAKE_MIME_OBJECT
#define GPGMEPLUG_ENCSIGN_INCLUDE_CLEARTEXT false
#define GPGMEPLUG_ENCSIGN_MAKE_MIME_OBJECT  false
#define GPGMEPLUG_ENCSIGN_MAKE_MULTI_MIME   false
#define GPGMEPLUG_ENCSIGN_CTYPE_MAIN        ""
#define GPGMEPLUG_ENCSIGN_CDISP_MAIN        ""
#define GPGMEPLUG_ENCSIGN_CTENC_MAIN        ""
#define GPGMEPLUG_ENCSIGN_CTYPE_VERSION     ""
#define GPGMEPLUG_ENCSIGN_CDISP_VERSION     ""
#define GPGMEPLUG_ENCSIGN_CTENC_VERSION     ""
#define GPGMEPLUG_ENCSIGN_BTEXT_VERSION     ""
#define GPGMEPLUG_ENCSIGN_CTYPE_CODE        ""
#define GPGMEPLUG_ENCSIGN_CDISP_CODE        ""
#define GPGMEPLUG_ENCSIGN_CTENC_CODE        ""
#define GPGMEPLUG_ENCSIGN_FLAT_PREFIX       ""
#define GPGMEPLUG_ENCSIGN_FLAT_SEPARATOR    ""
#define GPGMEPLUG_ENCSIGN_FLAT_POSTFIX      ""
#endif

#include "cryptplug.h"
#include <kdebug.h>

SMIMECryptPlug::SMIMECryptPlug() : CryptPlug() {
  GPGMEPLUG_PROTOCOL = GPGME_PROTOCOL_CMS;
  mProtocol = GpgME::Context::CMS;

  /* definitions for signing */
  // 1. opaque signatures (only used for S/MIME)
  GPGMEPLUG_OPA_SIGN_INCLUDE_CLEARTEXT = false;
  GPGMEPLUG_OPA_SIGN_MAKE_MIME_OBJECT  = true;
  GPGMEPLUG_OPA_SIGN_MAKE_MULTI_MIME   = false;
  GPGMEPLUG_OPA_SIGN_CTYPE_MAIN        = "application/pkcs7-mime; smime-type=signed-data; name=\"smime.p7m\"";
  GPGMEPLUG_OPA_SIGN_CDISP_MAIN        = "attachment; filename=\"smime.p7m\"";
  GPGMEPLUG_OPA_SIGN_CTENC_MAIN        = "base64";
  GPGMEPLUG_OPA_SIGN_CTYPE_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CDISP_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CTENC_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_BTEXT_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CTYPE_CODE        = "";
  GPGMEPLUG_OPA_SIGN_CDISP_CODE        = "";
  GPGMEPLUG_OPA_SIGN_CTENC_CODE        = "";
  GPGMEPLUG_OPA_SIGN_FLAT_PREFIX       = "";
  GPGMEPLUG_OPA_SIGN_FLAT_SEPARATOR    = "";
  GPGMEPLUG_OPA_SIGN_FLAT_POSTFIX      = "";
  // 2. detached signatures (used for S/MIME and for OpenPGP)
  GPGMEPLUG_DET_SIGN_INCLUDE_CLEARTEXT = true;
  GPGMEPLUG_DET_SIGN_MAKE_MIME_OBJECT  = true;
  GPGMEPLUG_DET_SIGN_MAKE_MULTI_MIME   = true;
  GPGMEPLUG_DET_SIGN_CTYPE_MAIN        = "multipart/signed; protocol=\"application/pkcs7-signature\"; micalg=sha1";
  GPGMEPLUG_DET_SIGN_CDISP_MAIN        = "";
  GPGMEPLUG_DET_SIGN_CTENC_MAIN        = "";
  GPGMEPLUG_DET_SIGN_CTYPE_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CDISP_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CTENC_VERSION     = "";
  GPGMEPLUG_DET_SIGN_BTEXT_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CTYPE_CODE        = "application/pkcs7-signature; name=\"smime.p7s\"";
  GPGMEPLUG_DET_SIGN_CDISP_CODE        = "attachment; filename=\"smime.p7s\"";
  GPGMEPLUG_DET_SIGN_CTENC_CODE        = "base64";
  GPGMEPLUG_DET_SIGN_FLAT_PREFIX       = "";
  GPGMEPLUG_DET_SIGN_FLAT_SEPARATOR    = "";
  GPGMEPLUG_DET_SIGN_FLAT_POSTFIX      = "";
  // 3. common definitions for opaque and detached signing
  __GPGMEPLUG_SIGNATURE_CODE_IS_BINARY = true;

  /* definitions for encoding */
  GPGMEPLUG_ENC_INCLUDE_CLEARTEXT  = false;
  GPGMEPLUG_ENC_MAKE_MIME_OBJECT   = true;
  GPGMEPLUG_ENC_MAKE_MULTI_MIME    = false;
  GPGMEPLUG_ENC_CTYPE_MAIN         = "application/pkcs7-mime; smime-type=enveloped-data; name=\"smime.p7m\"";
  GPGMEPLUG_ENC_CDISP_MAIN         = "attachment; filename=\"smime.p7m\"";
  GPGMEPLUG_ENC_CTENC_MAIN         = "base64";
  GPGMEPLUG_ENC_CTYPE_VERSION      = "";
  GPGMEPLUG_ENC_CDISP_VERSION      = "";
  GPGMEPLUG_ENC_CTENC_VERSION      = "";
  GPGMEPLUG_ENC_BTEXT_VERSION      = "";
  GPGMEPLUG_ENC_CTYPE_CODE         = "";
  GPGMEPLUG_ENC_CDISP_CODE         = "";
  GPGMEPLUG_ENC_CTENC_CODE         = "";
  GPGMEPLUG_ENC_FLAT_PREFIX        = "";
  GPGMEPLUG_ENC_FLAT_SEPARATOR     = "";
  GPGMEPLUG_ENC_FLAT_POSTFIX       = "";
  __GPGMEPLUG_ENCRYPTED_CODE_IS_BINARY = true;
}

OpenPGPCryptPlug::OpenPGPCryptPlug() : CryptPlug() {
  GPGMEPLUG_PROTOCOL = GPGME_PROTOCOL_OpenPGP;
  mProtocol = GpgME::Context::OpenPGP;

  /* definitions for signing */
  // 1. opaque signatures (only used for S/MIME)
  GPGMEPLUG_OPA_SIGN_INCLUDE_CLEARTEXT = false;
  GPGMEPLUG_OPA_SIGN_MAKE_MIME_OBJECT  = false;
  GPGMEPLUG_OPA_SIGN_MAKE_MULTI_MIME   = false;
  GPGMEPLUG_OPA_SIGN_CTYPE_MAIN        = "";
  GPGMEPLUG_OPA_SIGN_CDISP_MAIN        = "";
  GPGMEPLUG_OPA_SIGN_CTENC_MAIN        = "";
  GPGMEPLUG_OPA_SIGN_CTYPE_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CDISP_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CTENC_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_BTEXT_VERSION     = "";
  GPGMEPLUG_OPA_SIGN_CTYPE_CODE        = "";
  GPGMEPLUG_OPA_SIGN_CDISP_CODE        = "";
  GPGMEPLUG_OPA_SIGN_CTENC_CODE        = "";
  GPGMEPLUG_OPA_SIGN_FLAT_PREFIX       = "";
  GPGMEPLUG_OPA_SIGN_FLAT_SEPARATOR    = "";
  GPGMEPLUG_OPA_SIGN_FLAT_POSTFIX      = "";
  // 2. detached signatures (used for S/MIME and for OpenPGP)
  GPGMEPLUG_DET_SIGN_INCLUDE_CLEARTEXT = true;
  GPGMEPLUG_DET_SIGN_MAKE_MIME_OBJECT  = true;
  GPGMEPLUG_DET_SIGN_MAKE_MULTI_MIME   = true;
  GPGMEPLUG_DET_SIGN_CTYPE_MAIN        = "multipart/signed; protocol=\"application/pgp-signature\"; micalg=pgp-sha1";
  GPGMEPLUG_DET_SIGN_CDISP_MAIN        = "";
  GPGMEPLUG_DET_SIGN_CTENC_MAIN        = "";
  GPGMEPLUG_DET_SIGN_CTYPE_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CDISP_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CTENC_VERSION     = "";
  GPGMEPLUG_DET_SIGN_BTEXT_VERSION     = "";
  GPGMEPLUG_DET_SIGN_CTYPE_CODE        = "application/pgp-signature";
  GPGMEPLUG_DET_SIGN_CDISP_CODE        = "";
  GPGMEPLUG_DET_SIGN_CTENC_CODE        = "";
  GPGMEPLUG_DET_SIGN_FLAT_PREFIX       = "";
  GPGMEPLUG_DET_SIGN_FLAT_SEPARATOR    = "";
  GPGMEPLUG_DET_SIGN_FLAT_POSTFIX      = "";
  // 3. common definitions for opaque and detached signing
  __GPGMEPLUG_SIGNATURE_CODE_IS_BINARY = false;

  /* definitions for encoding */
  GPGMEPLUG_ENC_INCLUDE_CLEARTEXT  = false;
  GPGMEPLUG_ENC_MAKE_MIME_OBJECT   = true;
  GPGMEPLUG_ENC_MAKE_MULTI_MIME    = true;
  GPGMEPLUG_ENC_CTYPE_MAIN         = "multipart/encrypted; protocol=\"application/pgp-encrypted\"";
  GPGMEPLUG_ENC_CDISP_MAIN         = "";
  GPGMEPLUG_ENC_CTENC_MAIN         = "";
  GPGMEPLUG_ENC_CTYPE_VERSION      = "application/pgp-encrypted";
  GPGMEPLUG_ENC_CDISP_VERSION      = "attachment";
  GPGMEPLUG_ENC_CTENC_VERSION      = "";
  GPGMEPLUG_ENC_BTEXT_VERSION      = "Version: 1";
  GPGMEPLUG_ENC_CTYPE_CODE         = "application/octet-stream";
  GPGMEPLUG_ENC_CDISP_CODE         = "inline; filename=\"msg.asc\"";
  GPGMEPLUG_ENC_CTENC_CODE         = "";
  GPGMEPLUG_ENC_FLAT_PREFIX        = "";
  GPGMEPLUG_ENC_FLAT_SEPARATOR     = "";
  GPGMEPLUG_ENC_FLAT_POSTFIX       = "";
  __GPGMEPLUG_ENCRYPTED_CODE_IS_BINARY = false;
}

#define days_from_seconds(x) ((x)/86400)

/* Max number of parts in a DN */
#define MAX_GPGME_IDX 20

/* some macros to replace ctype ones and avoid locale problems */
#define spacep(p)   (*(p) == ' ' || *(p) == '\t')
#define digitp(p)   (*(p) >= '0' && *(p) <= '9')
#define hexdigitp(a) (digitp (a)                     \
                      || (*(a) >= 'A' && *(a) <= 'F')  \
                      || (*(a) >= 'a' && *(a) <= 'f'))
/* the atoi macros assume that the buffer has only valid digits */
#define atoi_1(p)   (*(p) - '0' )
#define atoi_2(p)   ((atoi_1(p) * 10) + atoi_1((p)+1))
#define atoi_4(p)   ((atoi_2(p) * 100) + atoi_2((p)+2))
#define xtoi_1(p)   (*(p) <= '9'? (*(p)- '0'): \
                     *(p) <= 'F'? (*(p)-'A'+10):(*(p)-'a'+10))
#define xtoi_2(p)   ((xtoi_1(p) * 16) + xtoi_1((p)+1))

static void *
xmalloc (size_t n)
{
  void *p = malloc (n);
  if (!p)
    {
      fputs ("\nfatal: out of core\n", stderr);
      exit (4);
    }
  return p;
}

/* Please: Don't call an allocation function xfoo when it may return NULL. */
/* Wrong: #define xstrdup( x ) (x)?strdup(x):0 */
/* Right: */
static char *
xstrdup (const char *string)
{
  char *p = (char*)xmalloc (strlen (string)+1);
  strcpy (p, string);
  return p;
}


/* Wrapper to cope with a bug in gpgme 0.4.4. */
static void
my_gpgme_key_release (gpgme_key_t key)
{
  if (key)
    gpgme_key_release (key);
}


CryptPlug::CryptPlug() {
}

CryptPlug::~CryptPlug() {
}

bool CryptPlug::initialize() {
  GpgME::setDefaultLocale( LC_CTYPE, setlocale( LC_CTYPE, 0 ) );
  GpgME::setDefaultLocale( LC_MESSAGES, setlocale( LC_MESSAGES, 0 ) );
  return (gpgme_engine_check_version (GPGMEPLUG_PROTOCOL) == GPG_ERR_NO_ERROR);
};


bool CryptPlug::hasFeature( Feature flag )
{
  /* our own plugins are supposed to support everything */
  switch ( flag ) {
  case Feature_SignMessages:
  case Feature_VerifySignatures:
  case Feature_EncryptMessages:
  case Feature_DecryptMessages:
  case Feature_SendCertificates:
  case Feature_PinEntrySettings:
  case Feature_StoreMessagesWithSigs:
  case Feature_EncryptionCRLs:
  case Feature_StoreMessagesEncrypted:
  case Feature_CheckCertificatePath:
    return true;
  case Feature_WarnSignCertificateExpiry:
  case Feature_WarnSignEmailNotInCertificate:
  case Feature_WarnEncryptCertificateExpiry:
  case Feature_WarnEncryptEmailNotInCertificate:
     return GPGMEPLUG_PROTOCOL == GPGME_PROTOCOL_CMS;
  /* undefined or not yet implemented: */
  case Feature_CRLDirectoryService:
  case Feature_CertificateDirectoryService:
  case Feature_undef:
  default:
    return false;
  }
}


/* Return the current interface version.  This is a simple way for a
   user to check whether all required fucntions are available.  If
   MIN_VERSION is not NULL the lowest supported version of the
   interface is returned in addition. */
int CryptPlug::interfaceVersion (int *min_version)
{
  if (min_version)
    *min_version = 0;
  return 1;
}

static
int getAttrExpireFormKey( gpgme_key_t* rKey)
{
  int daysLeft = CRYPTPLUG_CERT_DOES_NEVER_EXPIRE;
  if ( rKey && *rKey && (*rKey)->subkeys && (*rKey)->subkeys->expires >= 0 ) {
    time_t expire_time = (*rKey)->subkeys->expires;
    time_t cur_time = time (NULL);
    if( cur_time > expire_time ) {
      daysLeft = days_from_seconds(cur_time - expire_time);
      daysLeft *= -1;
    }
    else
      daysLeft = days_from_seconds(expire_time - cur_time);
  }
  return daysLeft;
}


static
void storeNewCharPtr( char** dest, const char* src )
{
  int sLen = strlen( src );
  *dest = (char*)xmalloc( sLen + 1 );
  strcpy( *dest, src );
}


bool CryptPlug::signMessage( const char*  cleartext,
			     char** ciphertext,
			     const size_t* cipherLen,
			     const char*  certificate,
			     struct StructuringInfo* structuring,
			     int* errId,
			     char** errTxt,
			     SendCertificates sendCertificates,
			     SignatureCompoundMode signatureCompoundMode )
{
  bool bIsOpaque;
  gpgme_ctx_t ctx;
  gpgme_error_t err;
  gpgme_key_t rKey;
  gpgme_data_t data,  sig;
  char* rSig  = 0;
  bool  bOk   = false;
  int sendCerts = 1;

  init_StructuringInfo( structuring );

  if( !ciphertext )
    return false;

  err = gpgme_new (&ctx);
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);

  gpgme_set_armor (ctx, __GPGMEPLUG_SIGNATURE_CODE_IS_BINARY ? 0 : 1);
  /*  gpgme_set_textmode (ctx, 1); */

  switch ( sendCertificates ) {
    case SendCert_undef:
      break;
    case SendCert_DontSend:
      sendCerts = 0;
      break;
    case SendCert_SendOwn:
      sendCerts = 1;
      break;
    case SendCert_SendChainWithoutRoot:
      sendCerts = -2;
      break;
    case SendCert_SendChainWithRoot:
      sendCerts = -1;
      break;
    default:
      sendCerts = 0;
      break;
  }
  gpgme_set_include_certs (ctx, sendCerts);

  /* select the signer's key if provided */
  if (certificate != 0) {
      err = gpgme_op_keylist_start(ctx, certificate, 0);
      while (!err) {
          err = gpgme_op_keylist_next(ctx, &rKey);
          if (!err) {
	      if ( rKey && rKey->can_sign ) {
                  /* clear existing signers */
                  gpgme_signers_clear(ctx);
                  /* set the signing key */
                  gpgme_signers_add(ctx, rKey);
                  /* we only support one signer for now */
                  break;
              }
          }
      }
      gpgme_op_keylist_end(ctx);
  }

  /* PENDING(g10) Implement this

     gpgme_set_signature_algorithm( ctx, config->signatureAlgorithm )
     --> This does not make sense.  The algorithm is a property of
     the certificate used [wk 2002-03-23] */

  gpgme_data_new_from_mem (&data, cleartext,
                            strlen( cleartext ), 1 );
  gpgme_data_new ( &sig );

  /* NOTE: Currently we support Opaque signed messages only for S/MIME,
     but not for OpenPGP mode! */
  if( GPGMEPLUG_PROTOCOL == GPGME_PROTOCOL_CMS )
    bIsOpaque = (signatureCompoundMode == SignatureCompoundMode_Opaque);
  else
    bIsOpaque = false;

  err = gpgme_op_sign ( ctx,
                        data,
                        sig,
                        bIsOpaque
                        ? GPGME_SIG_MODE_NORMAL
                        : GPGME_SIG_MODE_DETACH );

  if ( !err ) {
    if( __GPGMEPLUG_SIGNATURE_CODE_IS_BINARY ) {
      *ciphertext = gpgme_data_release_and_get_mem( sig, (size_t*)cipherLen );
      bOk = true;
    }
    else {
      rSig = gpgme_data_release_and_get_mem( sig, (size_t*)cipherLen );
      *ciphertext = (char*)malloc( *cipherLen + 1 );
      if( *ciphertext ) {
        if( *cipherLen ) {
          bOk = true;
          strncpy((char*)*ciphertext, rSig, *cipherLen );
        }
        (*ciphertext)[*cipherLen] = '\0';
      }
      free( rSig );
    }
  }
  else {
    gpgme_data_release( sig );
/*
*ciphertext = malloc( 70 );
strcpy((char*)*ciphertext, "xyz\nsig-dummy\nzyx" );
(*ciphertext)[17] = '\0';
err = 0;
{
*/
    *ciphertext = 0;
    fprintf( stderr, "\n\n    gpgme_op_sign() returned this error code:  %i\n\n", err );
    if( errId )
      *errId = err;
    if( errTxt ) {
      const char* _errTxt = gpgme_strerror( err );
      *errTxt = (char*)malloc( strlen( _errTxt ) + 1 );
      if( *errTxt )
        strcpy(*errTxt, _errTxt );
    }
/*
}
*/
  }
  gpgme_data_release( data );
  gpgme_release (ctx);

  if( bOk && structuring ) {
    if( bIsOpaque ) {
      structuring->includeCleartext = GPGMEPLUG_OPA_SIGN_INCLUDE_CLEARTEXT;
      structuring->makeMimeObject   = GPGMEPLUG_OPA_SIGN_MAKE_MIME_OBJECT;
      if( structuring->makeMimeObject ) {
        structuring->makeMultiMime  = GPGMEPLUG_OPA_SIGN_MAKE_MULTI_MIME;
        storeNewCharPtr( &structuring->contentTypeMain,
                        GPGMEPLUG_OPA_SIGN_CTYPE_MAIN );
        storeNewCharPtr( &structuring->contentDispMain,
                        GPGMEPLUG_OPA_SIGN_CDISP_MAIN );
        storeNewCharPtr( &structuring->contentTEncMain,
                        GPGMEPLUG_OPA_SIGN_CTENC_MAIN );
        if( structuring->makeMultiMime ) {
            storeNewCharPtr( &structuring->contentTypeVersion,
                            GPGMEPLUG_OPA_SIGN_CTYPE_VERSION );
            storeNewCharPtr( &structuring->contentDispVersion,
                            GPGMEPLUG_OPA_SIGN_CDISP_VERSION );
            storeNewCharPtr( &structuring->contentTEncVersion,
                            GPGMEPLUG_OPA_SIGN_CTENC_VERSION );
            storeNewCharPtr( &structuring->bodyTextVersion,
                            GPGMEPLUG_OPA_SIGN_BTEXT_VERSION );
            storeNewCharPtr( &structuring->contentTypeCode,
                            GPGMEPLUG_OPA_SIGN_CTYPE_CODE );
            storeNewCharPtr( &structuring->contentDispCode,
                            GPGMEPLUG_OPA_SIGN_CDISP_CODE );
            storeNewCharPtr( &structuring->contentTEncCode,
                            GPGMEPLUG_OPA_SIGN_CTENC_CODE );
        }
      } else {
        storeNewCharPtr( &structuring->flatTextPrefix,
                        GPGMEPLUG_OPA_SIGN_FLAT_PREFIX );
        storeNewCharPtr( &structuring->flatTextSeparator,
                        GPGMEPLUG_OPA_SIGN_FLAT_SEPARATOR );
        storeNewCharPtr( &structuring->flatTextPostfix,
                        GPGMEPLUG_OPA_SIGN_FLAT_POSTFIX );
      }
    } else {
      structuring->includeCleartext = GPGMEPLUG_DET_SIGN_INCLUDE_CLEARTEXT;
      structuring->makeMimeObject   = GPGMEPLUG_DET_SIGN_MAKE_MIME_OBJECT;
      if( structuring->makeMimeObject ) {
        structuring->makeMultiMime  = GPGMEPLUG_DET_SIGN_MAKE_MULTI_MIME;
        storeNewCharPtr( &structuring->contentTypeMain,
                        GPGMEPLUG_DET_SIGN_CTYPE_MAIN );
        storeNewCharPtr( &structuring->contentDispMain,
                        GPGMEPLUG_DET_SIGN_CDISP_MAIN );
        storeNewCharPtr( &structuring->contentTEncMain,
                        GPGMEPLUG_DET_SIGN_CTENC_MAIN );
        if( structuring->makeMultiMime ) {
            storeNewCharPtr( &structuring->contentTypeVersion,
                            GPGMEPLUG_DET_SIGN_CTYPE_VERSION );
            storeNewCharPtr( &structuring->contentDispVersion,
                            GPGMEPLUG_DET_SIGN_CDISP_VERSION );
            storeNewCharPtr( &structuring->contentTEncVersion,
                            GPGMEPLUG_DET_SIGN_CTENC_VERSION );
            storeNewCharPtr( &structuring->bodyTextVersion,
                            GPGMEPLUG_DET_SIGN_BTEXT_VERSION );
            storeNewCharPtr( &structuring->contentTypeCode,
                            GPGMEPLUG_DET_SIGN_CTYPE_CODE );
            storeNewCharPtr( &structuring->contentDispCode,
                            GPGMEPLUG_DET_SIGN_CDISP_CODE );
            storeNewCharPtr( &structuring->contentTEncCode,
                            GPGMEPLUG_DET_SIGN_CTENC_CODE );
        }
      } else {
        storeNewCharPtr( &structuring->flatTextPrefix,
                        GPGMEPLUG_DET_SIGN_FLAT_PREFIX );
        storeNewCharPtr( &structuring->flatTextSeparator,
                        GPGMEPLUG_DET_SIGN_FLAT_SEPARATOR );
        storeNewCharPtr( &structuring->flatTextPostfix,
                        GPGMEPLUG_DET_SIGN_FLAT_POSTFIX );
      }
    }
  }
  return bOk;
}



bool CryptPlug::storeCertificatesFromMessage( const char* ){ return true; }


/* returns address if address doesn't contain a <xxx> part
 * else it returns a new string xxx and frees address
 */
static char* parseAddress( char* address )
{
  char* result = address;
  char* i;
  char* j;
  if( !result ) return result;
  i = index( address, '<' );
  if( i ) {
    j = index( i+1, '>' );
    if( j == NULL ) j = address+strlen(address);
    result = (char*)xmalloc( j-i );
    strncpy( result, i+1, j-i-1 );
    result[j-i-1] = '\0';
    free( address );
  } else {
    i = address;
    j = i+strlen(address);
  }
  {
    /* remove surrounding whitespace */
    char* k = result+(j-i-1);
    char* l = result;
    while( isspace( *l ) ) ++l;
    while( isspace( *k ) ) --k;
    if( l != result || k != result+(j-i-1) ) {
      char* result2 = (char*)xmalloc( k-l+2 );
      strncpy( result2, l, k-l+1 );
      result2[k-l+1] = '\0';
      free(result);
      result = result2;
    }
  }
  return result;
}

static char* nextAddress( const char** address )
{
  const char *start = *address;
  char* result = NULL;
  int quote = 0;
  int comment = 0;
  int found = 0;
  if( *address == NULL ) return NULL;
  while( **address ) {

    switch( **address ) {
    case '\\': /* escaped character */
      ++(*address);
      break;
    case '"':
      if( comment == 0 ) {
        if( quote > 0 ) --quote;
        else ++quote;
      }
      break;
    case '(': /* comment start */
      if( quote == 0 ) ++comment;
      break;
    case ')': /* comment end */
      if( quote == 0 ) --comment;
      break;
    case '\0':
    case '\1': /* delimiter */
      if( quote == 0 && comment == 0 ) {
        found = 1;
      }
      break;
    }
    ++(*address);
    if( found ) break;
  }
  if( found || **address == 0 ) {
    size_t len;
    len = *address - start;
    if( len > 0 ) {
      if( **address != 0 ) --len;
      result = (char*)xmalloc( len*sizeof(char)+1 );
      strncpy( result, start, len );
      result[len] = '\0';
    }
  }
  return parseAddress(result);
}

bool CryptPlug::encryptMessage( const char*  cleartext,
                     const char** ciphertext,
                     const size_t* cipherLen,
                     const char*  certificate,
                     struct StructuringInfo* structuring,
                     int* errId,
                     char** errTxt )
{
  gpgme_ctx_t ctx;
  gpgme_error_t err;
  gpgme_data_t gCiphertext, gPlaintext;
  char*  rCiph = 0;
  bool   bOk   = false;

  init_StructuringInfo( structuring );

  gpgme_new (&ctx);
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);

  gpgme_set_armor (ctx, __GPGMEPLUG_ENCRYPTED_CODE_IS_BINARY ? 0 : 1);
  /*  gpgme_set_textmode (ctx, 1); */

  gpgme_data_new_from_mem (&gPlaintext, cleartext,
                            1+strlen( cleartext ), 1 );
  err = gpgme_data_new ( &gCiphertext );


  std::vector<char*> recips;

  {
    const char* p = certificate;
    char* tok;
    while( (tok = nextAddress( &p ) ) != 0 ) {
      fprintf( stderr, "\nGPGMEPLUG encryptMessage() using addressee %s\n", tok );
      recips.push_back( tok );
    }
  }

  recips.push_back( 0 );
  const char ** patterns = new const char*[ recips.size() ];
  const char ** patterns_it = patterns;
  std::copy( recips.begin(), recips.end(), patterns_it );
  recips.pop_back();

  std::vector<gpgme_key_t> keys;
#if 0 // BROKEN FOR FINGERPRINTS
  err = gpgme_op_keylist_ext_start( ctx, patterns, 0, 0 );
  while ( !err ) {
    gpgme_key_t key = 0;
    err = gpgme_op_keylist_next( ctx, &key );
    keys.push_back( key );
  }
  if ( gpg_err_code( err ) == GPG_ERR_EOF )
    err = GPG_ERR_NO_ERROR;
  gpgme_op_keylist_end( ctx );
#else
  for ( const char ** pit = patterns ; *pit && !err ; ++pit ) {
    gpgme_key_t key = 0;
    err = gpgme_get_key( ctx, *pit, &key, false );
    if ( key )
      keys.push_back( key );
  }
#endif

  keys.push_back( 0 );
  gpgme_key_t * rset = new gpgme_key_t[ keys.size() ];
  gpgme_key_t * rset_it = rset;
  std::copy( keys.begin(), keys.end(), rset_it );
  keys.pop_back();

  std::for_each( recips.begin(), recips.end(), &free );
  delete[] patterns;

  /* PENDING(g10) Implement this
     Possible values: RSA = 1, SHA1 = 2, TripleDES = 3
     gpgme_set_encryption_algorithm( ctx, config->encryptionAlgorithm );

     -> Your are mixing public key and symmetric algorithms.  The
     latter may be configured but the sphix specifications do opnly
     allow 3-DES so this is not nothing we need to do.  The proper way
     to select the symmetric algorithm is anyway by looking at the
     capabilities of the certificate because this is the only way to
     know what the recipient can accept. [wk 2002-03-23]

     PENDING(g10) Implement this
     gpgme_set_encryption_check_certificate_path(
     config->checkCertificatePath )

     PENDING(g10) Implement this
     gpgme_set_encryption_check_certificate_path_to_root(
     config->checkEncryptionCertificatePathToRoot )

     -> Not checking a certificate up to the ROOT CA is dangerous and
     stupid. There is no need for those options.  [wk 2002-03-23] */


  if ( !err )
    err = gpgme_op_encrypt (ctx, rset, (gpgme_encrypt_flags_t)0, gPlaintext, gCiphertext );
  if( err ) {
    fprintf( stderr, "\ngpgme_op_encrypt() returned this error code:  %i\n", err );
    if( errId )
      *errId = err;
    if( errTxt ) {
      const char* _errTxt = gpgme_strerror( err );
      const size_t errTxtLen = strlen( _errTxt ) + 100;  // leave room for reason string
      *errTxt = (char*)malloc( errTxtLen + 1 );
      if( *errTxt ) {
	std::string str = _errTxt;
	gpgme_encrypt_result_t opInfo = gpgme_op_encrypt_result( ctx );
	if ( opInfo && opInfo->invalid_recipients ) {
	  str += " - ";
	  str += gpgme_strerror( opInfo->invalid_recipients->reason );
	  // PENDING(kdab): there could be more than one!
	}
	strncmp( *errTxt, str.c_str(), errTxtLen );
	(*errTxt)[errTxtLen] = '\0';
      }
    }
  }

  std::for_each( keys.begin(), keys.end(), &my_gpgme_key_release );
  delete[] rset;
  gpgme_data_release (gPlaintext);

  if( !err ) {
    if( __GPGMEPLUG_ENCRYPTED_CODE_IS_BINARY ) {
      *ciphertext = gpgme_data_release_and_get_mem( gCiphertext, (size_t*)cipherLen );
      bOk = true;
    }
    else {
      rCiph = gpgme_data_release_and_get_mem( gCiphertext, (size_t*)cipherLen );
      *ciphertext = (char*)malloc( *cipherLen + 1 );
      if( *ciphertext ) {
        if( *cipherLen ) {
          bOk = true;
          strncpy((char*)*ciphertext, rCiph, *cipherLen );
        }
        ((char*)(*ciphertext))[*cipherLen] = 0;
      }
      free( rCiph );
    }
  }
  else {
    gpgme_data_release ( gCiphertext );
    *ciphertext = 0;
    /* error handling is missing: if only one untrusted key was found
      (or none at all), gpg won't sign the message.  (hier fehlt eine
      Fehlerbehandlung: fuer einen Recipient nur ein untrusted key
      (oder gar keiner) gefunden wurde, verweigert gpg das signieren.)
    */
  }

  gpgme_release (ctx);

  fflush( stderr );

  if( bOk && structuring ) {
    structuring->includeCleartext = GPGMEPLUG_ENC_INCLUDE_CLEARTEXT;
    structuring->makeMimeObject   = GPGMEPLUG_ENC_MAKE_MIME_OBJECT;
    if( structuring->makeMimeObject ) {
      structuring->makeMultiMime  = GPGMEPLUG_ENC_MAKE_MULTI_MIME;
      storeNewCharPtr( &structuring->contentTypeMain,
                       GPGMEPLUG_ENC_CTYPE_MAIN );
      storeNewCharPtr( &structuring->contentDispMain,
                       GPGMEPLUG_ENC_CDISP_MAIN );
      storeNewCharPtr( &structuring->contentTEncMain,
                       GPGMEPLUG_ENC_CTENC_MAIN );
      if( structuring->makeMultiMime ) {
        storeNewCharPtr( &structuring->contentTypeVersion,
                         GPGMEPLUG_ENC_CTYPE_VERSION );
        storeNewCharPtr( &structuring->contentDispVersion,
                         GPGMEPLUG_ENC_CDISP_VERSION );
        storeNewCharPtr( &structuring->contentTEncVersion,
                         GPGMEPLUG_ENC_CTENC_VERSION );
        storeNewCharPtr( &structuring->bodyTextVersion,
                         GPGMEPLUG_ENC_BTEXT_VERSION );
        storeNewCharPtr( &structuring->contentTypeCode,
                         GPGMEPLUG_ENC_CTYPE_CODE );
        storeNewCharPtr( &structuring->contentDispCode,
                         GPGMEPLUG_ENC_CDISP_CODE );
        storeNewCharPtr( &structuring->contentTEncCode,
                         GPGMEPLUG_ENC_CTENC_CODE );
      }
    } else {
      storeNewCharPtr( &structuring->flatTextPrefix,
                       GPGMEPLUG_ENC_FLAT_PREFIX );
      storeNewCharPtr( &structuring->flatTextSeparator,
                       GPGMEPLUG_ENC_FLAT_SEPARATOR );
      storeNewCharPtr( &structuring->flatTextPostfix,
                       GPGMEPLUG_ENC_FLAT_POSTFIX );
    }
  }
  return bOk;
}


bool CryptPlug::encryptAndSignMessage( const char* /*cleartext*/,
				       const char** /*ciphertext*/,
				       const char* /*certificate*/,
				       struct StructuringInfo* structuring )
{
  bool bOk;

  init_StructuringInfo( structuring );

  bOk = false;

  /* implementation of this function is still missing */

  if( bOk && structuring ) {
    structuring->includeCleartext = GPGMEPLUG_ENCSIGN_INCLUDE_CLEARTEXT;
    structuring->makeMimeObject   = GPGMEPLUG_ENCSIGN_MAKE_MIME_OBJECT;
    if( structuring->makeMimeObject ) {
      structuring->makeMultiMime  = GPGMEPLUG_ENCSIGN_MAKE_MULTI_MIME;
      storeNewCharPtr( &structuring->contentTypeMain,
                       GPGMEPLUG_ENCSIGN_CTYPE_MAIN );
      storeNewCharPtr( &structuring->contentDispMain,
                       GPGMEPLUG_ENCSIGN_CDISP_MAIN );
      storeNewCharPtr( &structuring->contentTEncMain,
                       GPGMEPLUG_ENCSIGN_CTENC_MAIN );
      if( structuring->makeMultiMime ) {
        storeNewCharPtr( &structuring->contentTypeVersion,
                         GPGMEPLUG_ENCSIGN_CTYPE_VERSION );
        storeNewCharPtr( &structuring->contentDispVersion,
                         GPGMEPLUG_ENCSIGN_CDISP_VERSION );
        storeNewCharPtr( &structuring->contentTEncVersion,
                         GPGMEPLUG_ENCSIGN_CTENC_VERSION );
        storeNewCharPtr( &structuring->bodyTextVersion,
                         GPGMEPLUG_ENCSIGN_BTEXT_VERSION );
        storeNewCharPtr( &structuring->contentTypeCode,
                         GPGMEPLUG_ENCSIGN_CTYPE_CODE );
        storeNewCharPtr( &structuring->contentDispCode,
                         GPGMEPLUG_ENCSIGN_CDISP_CODE );
        storeNewCharPtr( &structuring->contentTEncCode,
                         GPGMEPLUG_ENCSIGN_CTENC_CODE );
      }
    } else {
      storeNewCharPtr( &structuring->flatTextPrefix,
                       GPGMEPLUG_ENCSIGN_FLAT_PREFIX );
      storeNewCharPtr( &structuring->flatTextSeparator,
                       GPGMEPLUG_ENCSIGN_FLAT_SEPARATOR );
      storeNewCharPtr( &structuring->flatTextPostfix,
                       GPGMEPLUG_ENCSIGN_FLAT_POSTFIX );
    }
  }
  return bOk;
}


bool CryptPlug::decryptMessage( const char* ciphertext,
                     bool        cipherIsBinary,
                     int         cipherLen,
                     const char** cleartext,
				const char* /*certificate*/,
                     int* errId,
                     char** errTxt )
{
  gpgme_ctx_t ctx;
  gpgme_error_t err;
  gpgme_data_t gCiphertext, gPlaintext;
  size_t rCLen = 0;
  char*  rCiph = 0;
  bool bOk = false;

  if( !ciphertext )
    return false;

  err = gpgme_new (&ctx);
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);

  gpgme_set_armor (ctx, cipherIsBinary ? 0 : 1);
  /*  gpgme_set_textmode (ctx, cipherIsBinary ? 0 : 1); */

  /*
  gpgme_data_new_from_mem( &gCiphertext, ciphertext,
                           1+strlen( ciphertext ), 1 ); */
  gpgme_data_new_from_mem( &gCiphertext,
                           ciphertext,
                           cipherIsBinary
                           ? cipherLen
                           : strlen( ciphertext ),
                           1 );

  gpgme_data_new( &gPlaintext );

  err = gpgme_op_decrypt( ctx, gCiphertext, gPlaintext );
  if( err ) {
    fprintf( stderr, "\ngpgme_op_decrypt() returned this error code:  %i\n\n", err );
    if( errId )
      *errId = err;
    if( errTxt ) {
      const char* _errTxt = gpgme_strerror( err );
      *errTxt = (char*)malloc( strlen( _errTxt ) + 1 );
      if( *errTxt )
        strcpy(*errTxt, _errTxt );
    }
  }

  gpgme_data_release( gCiphertext );

  rCiph = gpgme_data_release_and_get_mem( gPlaintext,  &rCLen );

  *cleartext = (char*)malloc( rCLen + 1 );
  if( *cleartext ) {
      if( rCLen ) {
          bOk = true;
          strncpy((char*)*cleartext, rCiph, rCLen );
      }
      ((char*)(*cleartext))[rCLen] = 0;
  }

  free( rCiph );
  gpgme_release( ctx );
  return bOk;
}


static char *
trim_trailing_spaces( char *string )
{
    char *p, *mark;

    for( mark = NULL, p = string; *p; p++ ) {
	if( isspace( *p ) ) {
	    if( !mark )
		mark = p;
	}
	else
	    mark = NULL;
    }
    if( mark )
	*mark = '\0' ;

    return string ;
}

/* Parse a DN and return an array-ized one.  This is not a validating
   parser and it does not support any old-stylish syntax; gpgme is
   expected to return only rfc2253 compatible strings. */
static const unsigned char *
parse_dn_part (CryptPlug::DnPair *array, const unsigned char *string)
{
  const unsigned char *s, *s1;
  size_t n;
  char *p;

  /* parse attributeType */
  for (s = string+1; *s && *s != '='; s++)
    ;
  if (!*s)
    return NULL; /* error */
  n = s - string;
  if (!n)
    return NULL; /* empty key */
  p = (char*)xmalloc (n+1);


  memcpy (p, string, n);
  p[n] = 0;
  trim_trailing_spaces ((char*)p);
  // map OIDs to their names:
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i )
    if ( !strcasecmp ((char*)p, oidmap[i].oid) ) {
      free( p );
      p = xstrdup (oidmap[i].name);
      break;
    }
  array->key = p;
  string = s + 1;

  if (*string == '#')
    { /* hexstring */
      string++;
      for (s=string; hexdigitp (s); s++)
        s++;
      n = s - string;
      if (!n || (n & 1))
        return NULL; /* empty or odd number of digits */
      n /= 2;
      array->value = p = (char*)xmalloc (n+1);


      for (s1=string; n; s1 += 2, n--)
        *p++ = xtoi_2 (s1);
      *p = 0;
   }
  else
    { /* regular v3 quoted string */
      for (n=0, s=string; *s; s++)
        {
          if (*s == '\\')
            { /* pair */
              s++;
              if (*s == ',' || *s == '=' || *s == '+'
                  || *s == '<' || *s == '>' || *s == '#' || *s == ';'
                  || *s == '\\' || *s == '\"' || *s == ' ')
                n++;
              else if (hexdigitp (s) && hexdigitp (s+1))
                {
                  s++;
                  n++;
                }
              else
                return NULL; /* invalid escape sequence */
            }
          else if (*s == '\"')
            return NULL; /* invalid encoding */
          else if (*s == ',' || *s == '=' || *s == '+'
                   || *s == '<' || *s == '>' || *s == '#' || *s == ';' )
            break;
          else
            n++;
        }

      array->value = p = (char*)xmalloc (n+1);


      for (s=string; n; s++, n--)
        {
          if (*s == '\\')
            {
              s++;
              if (hexdigitp (s))
                {
                  *p++ = xtoi_2 (s);
                  s++;
                }
              else
                *p++ = *s;
            }
          else
            *p++ = *s;
        }
      *p = 0;
    }
  return s;
}


/* Parse a DN and return an array-ized one.  This is not a validating
   parser and it does not support any old-stylish syntax; gpgme is
   expected to return only rfc2253 compatible strings. */
static CryptPlug::DnPair *
parse_dn (const unsigned char *string)
{
  struct CryptPlug::DnPair *array;
  size_t arrayidx, arraysize;

  if( !string )
    return NULL;

  arraysize = 7; /* C,ST,L,O,OU,CN,email */
  arrayidx = 0;
  array = (CryptPlug::DnPair*)xmalloc ((arraysize+1) * sizeof *array);


  while (*string)
    {
      while (*string == ' ')
        string++;
      if (!*string)
        break; /* ready */
      if (arrayidx >= arraysize)
        { /* mutt lacks a real safe_realoc - so we need to copy */
          struct CryptPlug::DnPair *a2;

          arraysize += 5;
          a2 = (CryptPlug::DnPair*)xmalloc ((arraysize+1) * sizeof *array);
          for (unsigned int i=0; i < arrayidx; i++)
            {
              a2[i].key = array[i].key;
              a2[i].value = array[i].value;
            }
          free (array);
          array = a2;
        }
      array[arrayidx].key = NULL;
      array[arrayidx].value = NULL;
      string = parse_dn_part (array+arrayidx, string);
      arrayidx++;
      if (!string)
        goto failure;
      while (*string == ' ')
        string++;
      if (*string && *string != ',' && *string != ';' && *string != '+')
        goto failure; /* invalid delimiter */
      if (*string)
        string++;
    }
  array[arrayidx].key = NULL;
  array[arrayidx].value = NULL;
  return array;

 failure:
  for (unsigned i=0; i < arrayidx; i++)
    {
      free (array[i].key);
      free (array[i].value);
    }
  free (array);
  return NULL;
}

static void
add_dn_part( QCString& result, struct CryptPlug::DnPair& dnPair )
{
  /* email hack */
  QCString mappedPart( dnPair.key );
  for ( unsigned int i = 0 ; i < numOidMaps ; ++i ){
    if( !strcasecmp( dnPair.key, oidmap[i].oid ) ) {
      mappedPart = oidmap[i].name;
      break;
    }
  }
  result.append( mappedPart );
  result.append( "=" );
  result.append( dnPair.value );
}

static int
add_dn_parts( QCString& result, struct CryptPlug::DnPair* dn, const char* part )
{
  int any = 0;

  if( dn ) {
    for(; dn->key; ++dn ) {
      if( !strcmp( dn->key, part ) ) {
        if( any )
          result.append( "," );
        add_dn_part( result, *dn );
        any = 1;
      }
    }
  }
  return any;
}

static char*
reorder_dn( struct CryptPlug::DnPair *dn,
            char** attrOrder = 0,
            const char* unknownAttrsHandling = 0 )
{
  struct CryptPlug::DnPair *dnOrg = dn;

  /* note: The must parts are: CN, L, OU, O, C */
  const char* defaultpart[] = {
    "CN", "S", "SN", "GN", "T", "UID",
          "MAIL", "EMAIL", "MOBILE", "TEL", "FAX", "STREET",
    "L",  "PC", "SP", "ST",
    "OU",
    "O",
    "C",
    NULL
  };
  const char** stdpart = attrOrder ? ((const char**)attrOrder) : defaultpart;
  int any=0, any2=0, found_X_=0, i;
  QCString result;
  QCString resultUnknowns;

  /* find and save the non-standard parts in their original order */
  if( dn ){
    for(; dn->key; ++dn ) {
      for( i = 0; stdpart[i]; ++i ) {
        if( !strcmp( dn->key, stdpart[i] ) ) {
          break;
        }
      }
      if( !stdpart[i] ) {
        if( any2 )
          resultUnknowns.append( "," );
        add_dn_part( resultUnknowns, *dn );
        any2 = 1;
      }
    }
    dn = dnOrg;
  }

  /* prepend the unknown attrs if desired */
  if( unknownAttrsHandling &&
      !strcmp(unknownAttrsHandling, "PREFIX")
      && *resultUnknowns ){
    result.append( resultUnknowns );
    any = 1;
  }else{
    any = 0;
  }

  /* add standard parts */
  for( i = 0; stdpart[i]; ++i ) {
    dn = dnOrg;
    if( any ) {
      result.append( "," );
    }
    if( any2 &&
      !strcmp(stdpart[i], "_X_") &&
      unknownAttrsHandling &&
      !strcmp(unknownAttrsHandling, "INFIX") ){
      if ( !resultUnknowns.isEmpty() ) {
        result.append( resultUnknowns );
        any = 1;
      }
      found_X_ = 1;
    }else{
      any = add_dn_parts( result, dn, stdpart[i] );
    }
  }

  /* append the unknown attrs if desired */
  if( !unknownAttrsHandling ||
      !strcmp(unknownAttrsHandling, "POSTFIX") ||
      ( !strcmp(unknownAttrsHandling, "INFIX") && !found_X_ ) ){
    if( !resultUnknowns.isEmpty() ) {
      if( any ){
        result.append( "," );
      }
      result.append( resultUnknowns );
    }
  }

  char* cResult = (char*)xmalloc( (result.length()+1)*sizeof(char) );
  if( result.isEmpty() )
    *cResult = 0;
  else
    strcpy( cResult, result );
  return cResult;
}

struct CryptPlug::CertIterator {
  gpgme_ctx_t ctx;
  struct CertificateInfo info;
};

CryptPlug::CertIterator*
CryptPlug::startListCertificates( const char* pattern, int remote )
{
    gpgme_error_t err;
    struct CertIterator* it;
    const char* patterns[] = { pattern, NULL };
    fprintf( stderr,  "startListCertificates( \"%s\", %d )\n", pattern, remote );

    it = (CertIterator*)xmalloc( sizeof( struct CertIterator ) );

    err = gpgme_new (&(it->ctx));
    /*fprintf( stderr,  "2: gpgme returned %d\n", err );*/
    if( err ) {
      free( it );
      return NULL;
    }

    gpgme_set_protocol (it->ctx, GPGME_PROTOCOL_CMS);
    if( remote ) gpgme_set_keylist_mode ( it->ctx, GPGME_KEYLIST_MODE_EXTERN );
    else gpgme_set_keylist_mode ( it->ctx, GPGME_KEYLIST_MODE_LOCAL );
    err =  gpgme_op_keylist_ext_start ( it->ctx, patterns, 0, 0);
    memset( &(it->info), 0, sizeof( struct CertificateInfo ) );
    if( err ) {
      fprintf( stderr,  "gpgme_op_keylist_ext_start returned %d", err );
      endListCertificates( it );
      return NULL;
    }
    return it;
}

/* free() each string in a char*[] and the array itself */
static void
freeStringArray( char** c )
{
    char** _c = c;

    while( c && *c ) {
      /*fprintf( stderr, "freeing \"%s\"\n", *c );*/
      free( *c );
      ++c;
    }
    free( _c );
}

/* free all malloc'ed data in a struct CertificateInfo */
static void
freeInfo( struct CryptPlug::CertificateInfo* info )
{
  struct CryptPlug::DnPair* a = info->dnarray;
  assert( info );
  freeStringArray( info->userid );
  free( info->serial);
  free( info->fingerprint );
  free( info->issuer_org );
  free( info->issuer_reord );
  free( info->chainid );
  free( info->caps );
  while( a && a->key && a->value ) {
    free (a->key);
    free (a->value);
    ++a;
  }
  free (info->dnarray);
  memset( info, 0, sizeof( *info ) );
}

/* Format the fingerprint nicely. The caller should
   free the returned value using free() */
static char* make_fingerprint( const char* fpr )
{
  int len = strlen(fpr);
  int i = 0;
  char* result = (char*)xmalloc( (len + len/2 + 1)*sizeof(char) );

  for(; *fpr; ++fpr, ++i ) {
    if( i%3 == 2) {
      result[i] = ':'; ++i;
    }
    result[i] = *fpr;
  }
  result[i] = 0;
  return result;
}

// from gpgme 0.4.3:
static const char *
capabilities_to_string (gpgme_subkey_t subkey)
{
  static const char *const strings[8] =
    {
      "",
      "c",
      "s",
      "sc",
      "e",
      "ec",
      "es",
      "esc"
    };
  return strings[(!!subkey->can_encrypt << 2)
                 | (!!subkey->can_sign << 1)
                 | (!!subkey->can_certify)];
}

int
CryptPlug::nextCertificate( CryptPlug::CertIterator* it,
                            CryptPlug::CertificateInfo** result,
                            char** attrOrder,
                            const char* unknownAttrsHandling )
{
  gpgme_error_t err;
  gpgme_key_t   key;
  int retval = GPG_ERR_NO_ERROR;
  assert( it );
  fprintf( stderr,  "nextCertificates( %p, %p )\n", it, result );
  err = gpgme_op_keylist_next ( it->ctx, &key);
  if( !err ) {
    int idx;
    const char* s = 0;
    unsigned long u;
    char* names[MAX_GPGME_IDX+1];
    struct DnPair *issuer_dn, *tmp_dn;
    retval = err;
    memset( names, 0, sizeof( names ) );
    freeInfo( &(it->info) );

    std::cerr << "nextCertificate..." << std::endl;
    if ( key ) {
      idx = 0;
      for ( gpgme_user_id_t uid = key->uids ; uid && idx < MAX_GPGME_IDX ; uid = uid->next, ++idx )
        names[idx] = xstrdup( uid->uid );
    }
    std::cerr << "DEBUG: " << s << ":" << names[0] << std::endl;
    it->info.userid = (char**)xmalloc( sizeof( char* ) * (idx+1) );
    memset( it->info.userid, 0, sizeof( char* ) * (idx+1) );
    it->info.dnarray = 0;
    for( idx = 0; names[idx] != 0; ++idx ) {

      struct DnPair* a = parse_dn( (unsigned char*)names[idx] );
      it->info.userid[idx] = reorder_dn( a, attrOrder, unknownAttrsHandling );

      if( idx == 0 ) {
        it->info.userid_0_org = names[idx];
        it->info.dnarray = a;
      }else{
        free (names[idx]);
        names[idx] = NULL;
        while( a && a->key && a->value ) {
          free( a->key );
          free( a->value );
          ++a;
        }
      }
    }
    it->info.userid[idx] = 0;

    s = key->issuer_serial;
    it->info.serial = s? xstrdup(s) : NULL;

    s = key->subkeys ? key->subkeys->fpr : 0 ;
    it->info.fingerprint = make_fingerprint( s );

    s = key->issuer_name;
    it->info.issuer_org = s? xstrdup(s): NULL;
    if( s ) {
      issuer_dn = tmp_dn = parse_dn( (const unsigned char*)s );
      /*it->info.issuer = xstrdup(s);*/
      it->info.issuer_reord = reorder_dn( issuer_dn, attrOrder, unknownAttrsHandling );
      while( tmp_dn && tmp_dn->key ) {
        free( tmp_dn->key );
        free( tmp_dn->value );
        ++tmp_dn;
      }
      free( issuer_dn );
      issuer_dn = tmp_dn = NULL;
    } else {
      it->info.issuer_reord = NULL;
    }
    s = key->chain_id;
    it->info.chainid = s? xstrdup(s): NULL;

    s = key->subkeys ? capabilities_to_string( key->subkeys ) : 0 ;
    it->info.caps = s? xstrdup(s) : NULL;

    u = key->subkeys && key->subkeys->timestamp >= 0 ? key->subkeys->timestamp : 0 ;
    it->info.created = u;

    u = key->subkeys && key->subkeys->expires >= 0 ? key->subkeys->expires : 0 ;
    it->info.expire = u;

    u = key->secret;
    it->info.secret = u;

    u = key->uids ? key->uids->invalid : 0 ;
    it->info.invalid = u;

    u = key->subkeys ? key->subkeys->expired : 0 ;
    it->info.expired = u;

    u = key->subkeys ? key->subkeys->disabled : 0 ;
    it->info.disabled = u;

    my_gpgme_key_release (key);
    /*return &(it->info);*/
    *result =  &(it->info);
  } else {
    *result = NULL;
  }
  return retval;
}

int
CryptPlug::endListCertificates( CryptPlug::CertIterator* it )
{
  fprintf( stderr,  "endListCertificates( %p )\n", it );
  assert(it);
  gpgme_keylist_result_t result = gpgme_op_keylist_result( it->ctx );
  int truncated = result && result->truncated;
  freeInfo( &(it->info) );
  gpgme_op_keylist_end(it->ctx);
  gpgme_release (it->ctx);
  free( it );
  return truncated;
}

GpgME::ImportResult CryptPlug::importCertificateFromMem( const char* data, size_t length )
{
  using namespace GpgME;

  std::auto_ptr<Context> context( Context::createForProtocol( mProtocol ) );
  if ( !context.get() )
    return ImportResult();

  Data keydata( data, length, false );
  if ( keydata.isNull() )
    return ImportResult();

  return context->importKeys( keydata );
}


/*  == == == == == == == == == == == == == == == == == == == == == == == == ==
   ==                                                                      ==
  ==         Continuation of CryptPlug code                               ==
 ==                                                                      ==
== == == == == == == == == == == == == == == == == == == == == == == == ==  */


/*
  Find all certificate for a given addressee and return them in a
  '\1' separated list.
  NOTE: The certificate parameter must point to a not-yet allocated
        char*.  The function will allocate the memory needed and
        return the size in newSize.
  If secretOnly is true, only secret keys are returned.
*/
bool CryptPlug::findCertificates( const char* addressee,
                                  char** certificates,
                                  int* newSize,
                                  bool secretOnly,
                                  char** attrOrder,
                                  const char* unknownAttrsHandling )
{
#define MAXCERTS 1024
  /* use const char declarations since all of them are needed twice */
  const char* delimiter = "\1";
  const char* openBracket = "    (";
  const char* closeBracket = ")";

  gpgme_ctx_t ctx;
  gpgme_error_t err;
  gpgme_key_t rKey;
  const char *s;
  const char *s2;
  char* dn;
  struct DnPair* a;
  int nFound = 0;
  int iFound = 0;
  int siz = 0;
  char* DNs[MAXCERTS];
  char* FPRs[MAXCERTS];

  if( ! certificates ){
    fprintf( stderr, "gpgme: findCertificates called with invalid *certificates pointer\n" );
    return false;
  }

  if( ! newSize ){
    fprintf( stderr, "gpgme: findCertificates called with invalid newSize pointer\n" );
    return false;
  }

  *certificates = 0;
  *newSize = 0;

  /* calculate length of buffer needed for certs plus fingerprints */
  gpgme_new (&ctx);
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);
  err = gpgme_op_keylist_start(ctx, addressee, secretOnly ? 1 : 0);
  while( !err ) {
    err = gpgme_op_keylist_next(ctx, &rKey);
    if( !err ) {
      s = rKey && rKey->uids ? rKey->uids->uid : 0 ;
      if( s ) {
        dn = xstrdup( s );
        s2 = rKey && rKey->subkeys ? rKey->subkeys->fpr : 0 ;
        if( s2 ) {
          if( nFound )
            siz += strlen( delimiter );

          //fprintf( stderr, "gpgme: before reordering (%s)\n", dn );

          a = parse_dn( (unsigned char*)dn );
          free( dn );
          dn = reorder_dn( a, attrOrder, unknownAttrsHandling );

          //fprintf( stderr, "gpgme: after reordering  (%s)\n\n", dn );

          siz += strlen( dn );
          siz += strlen( openBracket );
          siz += strlen( s2 );
          siz += strlen( closeBracket );
          DNs[ nFound ] = dn;
          dn = NULL; /* prevent it from being free'ed below. */
          FPRs[nFound ] = xstrdup( s2 );
          ++nFound;
          if( nFound >= MAXCERTS ) {
            fprintf( stderr,
                     "gpgme: findCertificates found too many certificates (%d)\n",
                     MAXCERTS );
            break;
          }
        }
        free (dn);
      }
    }
  }
  gpgme_op_keylist_end( ctx );
  gpgme_release (ctx);


  if( 0 < siz ) {
    /* add one for trailing ZERO char */
    ++siz;
    *newSize = siz;
    /* allocate the buffer */
    *certificates = (char*)xmalloc( sizeof(char) * siz );
    memset( *certificates, 0, sizeof(char) * siz );
    /* fill the buffer */
    for (iFound=0; iFound < nFound; iFound++) {
      if( !iFound )
        strcpy(*certificates, DNs[iFound] );
      else {
        strcat(*certificates, delimiter );
        strcat(*certificates, DNs[iFound] );
      }
      strcat(  *certificates, openBracket );
      strcat(  *certificates, FPRs[iFound] );
      strcat(  *certificates, closeBracket );
      free( DNs[ iFound ] );
      free( FPRs[iFound ] );
    }
  }

  return ( 0 < nFound );
}

// these are from gpgme-0.4.3:
static gpgme_sig_stat_t
sig_stat_from_status( gpgme_error_t err )
{
  switch ( gpg_err_code(err) ) {
  case GPG_ERR_NO_ERROR:
    return GPGME_SIG_STAT_GOOD;
  case GPG_ERR_BAD_SIGNATURE:
    return GPGME_SIG_STAT_BAD;
  case GPG_ERR_NO_PUBKEY:
    return GPGME_SIG_STAT_NOKEY;
  case GPG_ERR_NO_DATA:
    return GPGME_SIG_STAT_NOSIG;
  case GPG_ERR_SIG_EXPIRED:
    return GPGME_SIG_STAT_GOOD_EXP;
  case GPG_ERR_KEY_EXPIRED:
    return GPGME_SIG_STAT_GOOD_EXPKEY;
  default:
    return GPGME_SIG_STAT_ERROR;
  }
}


static gpgme_sig_stat_t
intersect_stati( gpgme_signature_t first )
{
  if ( !first )
    return GPGME_SIG_STAT_NONE;
  gpgme_sig_stat_t result = sig_stat_from_status( first->status );
  for ( gpgme_signature_t sig = first->next ; sig ; sig = sig->next )
    if ( sig_stat_from_status( sig->status ) != result )
      return GPGME_SIG_STAT_DIFF;
  return result;
}

static const char*
sig_status_to_string( gpgme_sig_stat_t status )
{
  const char *result;

  switch (status) {
    case GPGME_SIG_STAT_NONE:
      result = "Oops: Signature not verified";
      break;
    case GPGME_SIG_STAT_NOSIG:
      result = "No signature found";
      break;
    case GPGME_SIG_STAT_GOOD:
      result = "Good signature";
      break;
    case GPGME_SIG_STAT_BAD:
      result = "BAD signature";
      break;
    case GPGME_SIG_STAT_NOKEY:
      result = "No public key to verify the signature";
      break;
    case GPGME_SIG_STAT_ERROR:
      result = "Error verifying the signature";
      break;
    case GPGME_SIG_STAT_DIFF:
      result = "Different results for signatures";
      break;
    default:
      result = "Error: Unknown status";
      break;
  }

  return result;
}

// WARNING: if you fix a bug here, you have to likely fix it in the
// gpgme 0.3 version below, too!
static
void obtain_signature_information( gpgme_ctx_t ctx,
                                   gpgme_sig_stat_t & overallStatus,
                                   struct CryptPlug::SignatureMetaData* sigmeta,
                                   char** attrOrder,
                                   const char* unknownAttrsHandling,
                                   bool * signatureFound=0 )
{
  gpgme_error_t err;
  unsigned long sumGPGME;
  SigStatusFlags sumPlug;
  struct CryptPlug::DnPair* a;
  int sig_idx=0;

  assert( ctx );
  assert( sigmeta );

  sigmeta->extended_info = 0;
  gpgme_verify_result_t result = gpgme_op_verify_result( ctx );
  if ( !result )
    return;
  for ( gpgme_signature_t signature = result->signatures ; signature ; signature = signature->next, ++sig_idx ) {
    void* alloc_return = realloc( sigmeta->extended_info,
                                  sizeof( CryptPlug::SignatureMetaDataExtendedInfo )
                                  * ( sig_idx + 1 ) );
    if ( !alloc_return )
      break;
    sigmeta->extended_info = (CryptPlug::SignatureMetaDataExtendedInfo*)alloc_return;

    /* shorthand notation :) */
    CryptPlug::SignatureMetaDataExtendedInfo & this_info = sigmeta->extended_info[sig_idx];

    /* clear the data area */
    memset( &this_info, 0, sizeof (CryptPlug::SignatureMetaDataExtendedInfo) );

    /* the creation time */
    if ( signature->timestamp ) {
      this_info.creation_time = (tm*)malloc( sizeof( struct tm ) );
      if ( this_info.creation_time ) {
        struct tm * ctime_val = localtime( (time_t*)&signature->timestamp );
        memcpy( this_info.creation_time,
                ctime_val, sizeof( struct tm ) );
      }
    }

    /* the extended signature verification status */
    sumGPGME = signature->summary;
    fprintf( stderr, "gpgmeplug checkMessageSignature status flags: %lX\n", sumGPGME );
    /* translate GPGME status flags to common CryptPlug status flags */
    sumPlug = 0;
#define convert(X) if ( sumGPGME & GPGME_SIGSUM_##X ) sumPlug |= SigStat_##X
    convert(VALID);
    convert(GREEN);
    convert(RED);
    convert(KEY_REVOKED);
    convert(KEY_EXPIRED);
    convert(SIG_EXPIRED);
    convert(KEY_MISSING);
    convert(CRL_MISSING);
    convert(CRL_TOO_OLD);
    convert(BAD_POLICY);
    convert(SYS_ERROR);
#undef convert
    if( sumGPGME && !sumPlug )
      sumPlug = SigStat_NUMERICAL_CODE | sumGPGME;
    this_info.sigStatusFlags = sumPlug;

    /* extract finger print */
    if ( signature->fpr )
      storeNewCharPtr( &this_info.fingerprint, signature->fpr );

    /* validity */
    this_info.validity = GPGME_VALIDITY_UNKNOWN;

    /* sig key data */
    gpgme_key_t key = 0;
    // PENDING(marc) if this is deprecated, how shall we get at all
    // the infos below?
    err = gpgme_get_sig_key (ctx, sig_idx, &key);

    if ( !err && key ) {
      const char* attr_string;
      unsigned long attr_ulong;

      /* extract key identidy */
      attr_string = key->subkeys ? key->subkeys->keyid : 0 ;
      if ( attr_string )
	storeNewCharPtr( &this_info.keyid, attr_string );

      /* pubkey algorithm */
      attr_string = key->subkeys ? gpgme_pubkey_algo_name( key->subkeys->pubkey_algo ) : 0 ;
      if (attr_string != 0)
	storeNewCharPtr( &this_info.algo, attr_string );
      attr_ulong = key->subkeys ? key->subkeys->pubkey_algo : 0 ;
      this_info.algo_num = attr_ulong;

      /* extract key validity */
      attr_ulong = key->uids ? key->uids->validity : 0 ;
      this_info.validity = attr_ulong;

      /* extract user id, according to the documentation it's representable
       * as a number, but it seems that it also has a string representation
       */
      attr_string = key->uids ? key->uids->uid : 0 ;
      if (attr_string != 0) {
        a = parse_dn( (const unsigned char*)attr_string );
        this_info.userid = reorder_dn( a, attrOrder, unknownAttrsHandling );
      }

      attr_ulong = 0;
      this_info.userid_num = attr_ulong;

      /* extract the length */
      this_info.keylen = key->subkeys ? key->subkeys->length : 0 ;

      /* extract the creation time of the key */
      attr_ulong = key->subkeys ? key->subkeys->timestamp : 0 ;
      this_info.key_created = attr_ulong;

      /* extract the expiration time of the key */
      attr_ulong = key->subkeys ? key->subkeys->expires : 0 ;
      this_info.key_expires = attr_ulong;

      /* extract user name */
      attr_string = key->uids ? key->uids->name : 0 ;
      if (attr_string != 0) {
        a = parse_dn( (const unsigned char*)attr_string );
        this_info.name = reorder_dn( a, attrOrder, unknownAttrsHandling );
      }

      /* extract email(s) */
      this_info.emailCount = 0;
      this_info.emailList = 0;
      for ( gpgme_user_id_t uid = key->uids ; uid ; uid = uid->next ) {
        attr_string = uid->email;
        if ( attr_string && *attr_string) {
          fprintf( stderr, "gpgmeplug checkMessageSignature found email: %s\n", attr_string );
          if( !this_info.emailCount )
            alloc_return = malloc( sizeof( char*) );
          else
            alloc_return = realloc( this_info.emailList,
                  sizeof( char*)
                  * (this_info.emailCount + 1) );
          if( alloc_return ) {
            this_info.emailList = (char**)alloc_return;
            storeNewCharPtr( &( this_info.emailList[ this_info.emailCount ] ),
                attr_string );
            ++this_info.emailCount;
          }
        }
      }
      if( !this_info.emailCount )
	fprintf( stderr, "gpgmeplug checkMessageSignature found NO EMAIL\n" );

      /* extract the comment */
      attr_string = key->uids ? key->uids->comment : 0 ;
      if (attr_string != 0)
	storeNewCharPtr( &this_info.comment, attr_string );
    }

    gpgme_sig_stat_t status = sig_stat_from_status( signature->status );
    const char* sig_status = sig_status_to_string( status );
    storeNewCharPtr( &this_info.status_text, sig_status );
  }
  sigmeta->extended_info_count = sig_idx;
  overallStatus = intersect_stati( result->signatures );
  sigmeta->status_code = overallStatus;
  storeNewCharPtr( &sigmeta->status, sig_status_to_string( overallStatus ) );
  if ( signatureFound )
    *signatureFound = ( overallStatus != GPGME_SIG_STAT_NONE );
}


bool CryptPlug::checkMessageSignature( char** cleartext,
                            const char* signaturetext,
                            bool signatureIsBinary,
                            int signatureLen,
                            struct CryptPlug::SignatureMetaData* sigmeta,
                            char** attrOrder,
                            const char* unknownAttrsHandling )
{
  gpgme_ctx_t ctx;
  gpgme_sig_stat_t status = GPGME_SIG_STAT_NONE;
  gpgme_data_t datapart, sigpart;
  char* rClear = 0;
  size_t clearLen;
  bool isOpaqueSigned;

  if( !cleartext ) {
    if( sigmeta )
      storeNewCharPtr( &sigmeta->status,
                        __GPGMEPLUG_ERROR_CLEARTEXT_IS_ZERO );

    return false;
  }

  isOpaqueSigned = !*cleartext;

  gpgme_new( &ctx );
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);
  gpgme_set_armor (ctx,    signatureIsBinary ? 0 : 1);
  /*  gpgme_set_textmode (ctx, signatureIsBinary ? 0 : 1); */

  if( isOpaqueSigned )
    gpgme_data_new( &datapart );
  else
    gpgme_data_new_from_mem( &datapart, *cleartext,
                             strlen( *cleartext ), 1 );

  gpgme_data_new_from_mem( &sigpart,
                           signaturetext,
                           signatureIsBinary
                           ? signatureLen
                           : strlen( signaturetext ),
                           1 );

  if ( isOpaqueSigned )
    gpgme_op_verify( ctx, sigpart, 0, datapart );
  else
    gpgme_op_verify( ctx, sigpart, datapart, 0 );

  if( isOpaqueSigned ) {
    rClear = gpgme_data_release_and_get_mem( datapart, &clearLen );
    *cleartext = (char*)malloc( clearLen + 1 );
    if( *cleartext ) {
      if( clearLen )
        strncpy(*cleartext, rClear, clearLen );
      (*cleartext)[clearLen] = '\0';
    }
    free( rClear );
  }
  else
    gpgme_data_release( datapart );

  gpgme_data_release( sigpart );

  obtain_signature_information( ctx, status, sigmeta,
                                attrOrder, unknownAttrsHandling );

  gpgme_release( ctx );
  return ( status == GPGME_SIG_STAT_GOOD );
}


bool CryptPlug::decryptAndCheckMessage( const char*  ciphertext,
                                  bool         cipherIsBinary,
                                  int          cipherLen,
                                  const char** cleartext,
                                  const char*  /*certificate*/,
                                  bool*        signatureFound,
                                  struct CryptPlug::SignatureMetaData* sigmeta,
                                  int*   errId,
                                  char** errTxt,
                                  char** attrOrder,
                                  const char* unknownAttrsHandling  )
{
  gpgme_ctx_t ctx;
  gpgme_error_t err;
  gpgme_decrypt_result_t decryptresult;
  gpgme_data_t gCiphertext, gPlaintext;
  gpgme_sig_stat_t sigstatus = GPGME_SIG_STAT_NONE;
  size_t rCLen = 0;
  char*  rCiph = 0;
  bool bOk = false;
  bool bWrongKeyUsage = false;

  if( !ciphertext )
    return false;

  err = gpgme_new (&ctx);
  gpgme_set_protocol (ctx, GPGMEPLUG_PROTOCOL);

  gpgme_set_armor (ctx, cipherIsBinary ? 0 : 1);
  /*  gpgme_set_textmode (ctx, cipherIsBinary ? 0 : 1); */

  /*
  gpgme_data_new_from_mem( &gCiphertext, ciphertext,
                           1+strlen( ciphertext ), 1 ); */
  gpgme_data_new_from_mem( &gCiphertext,
                           ciphertext,
                           cipherIsBinary
                           ? cipherLen
                           : strlen( ciphertext ),
                           1 );

  gpgme_data_new( &gPlaintext );

  err = gpgme_op_decrypt_verify( ctx, gCiphertext, gPlaintext );
  gpgme_data_release( gCiphertext );
  
  decryptresult = gpgme_op_decrypt_result( ctx );
  if( decryptresult->wrong_key_usage )
    bWrongKeyUsage = true;
  
  if( err ) {
    fprintf( stderr, "\ngpgme_op_decrypt_verify() returned this error code:  %i\n\n", err );
    if( errId )
      *errId = err;
    if( errTxt ) {
      const char* _errTxt = gpgme_strerror( err );
      *errTxt = (char*)malloc( strlen( _errTxt ) + 1 );
      if( *errTxt )
        strcpy(*errTxt, _errTxt );
    }
    gpgme_data_release( gPlaintext );
    gpgme_release( ctx );
    return bOk;
  }
  
  if( bWrongKeyUsage ) {
    if( errId )
      *errId = CRYPTPLUG_ERR_WRONG_KEY_USAGE; // report the wrong key usage
  }  
  
  rCiph = gpgme_data_release_and_get_mem( gPlaintext,  &rCLen );

  *cleartext = (char*)malloc( rCLen + 1 );
  if( *cleartext ) {
      if( rCLen ) {
          bOk = true;
          strncpy((char*)*cleartext, rCiph, rCLen );
      }
      ((char*)(*cleartext))[rCLen] = 0;
  }
  free( rCiph );

  obtain_signature_information( ctx, sigstatus, sigmeta,
                                attrOrder, unknownAttrsHandling,
                                signatureFound );

  gpgme_release( ctx );
  return bOk;
}

