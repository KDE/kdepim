/* -*- Mode: C++ -*-

  this is a C++-ification of:
  CRYPTPLUG - an independent cryptography plug-in API

  Copyright (C) 2001,2004 Klarälvdalens Datakonsult AB

  CRYPTPLUG is free software; you can redistribute it and/or modify
  it under the terms of GNU General Public License as published by
  the Free Software Foundation; version 2 of the License.

  CRYPTPLUG is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifndef CRYPTPLUG_H
#define CRYPTPLUG_H

#include <stdlib.h>

#include <gpgmepp/context.h>
#include <gpgme.h> // need it for gpgme_protocol_t :(
#include <kdepimmacros.h>

namespace GpgME {
  class ImportResult;
}

/*! \file cryptplug.h
    \brief Common API header for CRYPTPLUG.

    CRYPTPLUG is an independent cryptography plug-in API
    developed for Sphinx-enabeling KMail and Mutt.

    CRYPTPLUG was designed for the Aegypten project, but it may
    be used by 3rd party developers as well to design pluggable
    crypto backends for the above mentioned MUAs.

    \note All string parameters appearing in this API are to be
    interpreted as UTF-8 encoded.

    \see pgpplugin.c
    \see gpgplugin.c
*/

/*! \defgroup groupGeneral Loading and Unloading the Plugin, General Functionality

    The functions in this section are used for loading and
    unloading plugins. Note that the actual locating of the plugin
    and the loading and unloading of the dynamic library is not
    covered here; this is MUA-specific code for which support code
    might already exist in the programming environments.
*/

/*! \defgroup groupDisplay Graphical Display Functionality

    The functions in this section return stationery that the
    MUAs can use in order to display security functionality
    graphically. This can be toolbar icons, shortcuts, tooltips,
    etc. Not all MUAs will use all this functionality.
*/

/*! \defgroup groupConfig Configuration Support

    The functions in this section provide the necessary
    functionality to configure the security functionality as well
    as to query configuration settings. Since all configuration
    settings will not be saved with the plugin, but rather with
    the MUA, there are also functions to set configuration
    settings programmatically; these will be used on startup of
    the plugin when the MUA transfers the configuration values it
    has read into the plugin. Usually, the functions to query and
    set the configuration values are not needed for anything but
    saving to and restoring from configuration files.
*/


/*! \defgroup groupConfigSign Signature Configuration
    \ingroup groupConfig

    The functions in this section provide the functionality
    to configure signature handling and set and query the
    signature configuration.
*/

/*! \defgroup groupConfigCrypt Encryption Configuration
    \ingroup groupConfig

    The functions in this section provide the functionality
    to configure encryption handling and set and query the
    encryption configuration.

    \note Whenever the term <b> encryption</b> is used here,
    it is supposed to mean both encryption and decryption,
    unless otherwise specified.
*/

/*! \defgroup groupConfigDir Directory Service Configuration
    \ingroup groupConfig

    This section contains messages for configuring the
    directory service.
*/


/*! \defgroup groupCertHand Certificate Handling

    The following methods are used to maintain and query certificates.
*/


/*! \defgroup groupSignCryptAct Signing and Encrypting Actions

    This section describes methods and structures
    used for signing and/or encrypting your mails.
*/


/*! \defgroup groupSignAct Signature Actions
    \ingroup groupSignCryptAct

    This section describes methods that are used for working
    with signatures.
*/

/*! \defgroup groupCryptAct Encryption and Decryption
    \ingroup groupSignCryptAct

    The following methods are used to encrypt and decrypt
    email messages.
*/

/*! \defgroup groupCertAct Certificate Handling Actions

    The functions in this section provide local certificate management.
*/

/*! \defgroup groupCRLAct CRL Handling Actions

    This section describes functions for managing CRLs.
*/

/*! \defgroup groupAdUsoInterno Important functions to be used by plugin implementors ONLY.

    This section describes functions that have to be used by
    plugin implementors but should not be used by plugin users
    directly.

    If you are not planning to write your own cryptography
    plugin <b>you should ignore this</b> section!
*/

/*! \defgroup certList Certificate Info listing functions
 */

typedef enum {
  Feature_undef             = 0,

  Feature_SignMessages      = 1,
  Feature_VerifySignatures  = 2,
  Feature_EncryptMessages   = 3,
  Feature_DecryptMessages   = 4,
  Feature_SendCertificates  = 5,
  Feature_WarnSignCertificateExpiry = 6,
  Feature_WarnSignEmailNotInCertificate = 7,
  Feature_PinEntrySettings  = 8,
  Feature_StoreMessagesWithSigs = 9,
  Feature_EncryptionCRLs    = 10,
  Feature_WarnEncryptCertificateExpiry = 11,
  Feature_WarnEncryptEmailNotInCertificate = 12,
  Feature_StoreMessagesEncrypted = 13,
  Feature_CheckCertificatePath = 14,
  Feature_CertificateDirectoryService = 15,
  Feature_CRLDirectoryService = 16,
  Feature_CertificateInfo     = 17
} Feature;

/* dummy values */
typedef enum {
  PinRequest_undef            = 0,

  PinRequest_Always          = 1,
  PinRequest_WhenAddingCerts = 2,
  PinRequest_AlwaysWhenSigning = 3,
  PinRequest_OncePerSession   = 4,
  PinRequest_AfterMinutes     = 5
} PinRequests;


typedef enum {
  SignatureCompoundMode_undef    = 0,

  SignatureCompoundMode_Opaque   = 1,
  SignatureCompoundMode_Detached = 2
} SignatureCompoundMode;


typedef enum {
  SendCert_undef              = 0,

  SendCert_DontSend           = 1,
  SendCert_SendOwn            = 2,
  SendCert_SendChainWithoutRoot = 3,
  SendCert_SendChainWithRoot  = 4
} SendCertificates;


typedef enum {
  SignAlg_undef               = 0,

  SignAlg_SHA1                = 1
} SignatureAlgorithm;



typedef enum {
  EncryptAlg_undef            = 0,

  EncryptAlg_RSA              = 1,
  EncryptAlg_SHA1             = 2,
  EncryptAlg_TripleDES        = 3
} EncryptionAlgorithm;

typedef enum {
  SignEmail_undef             = 0,

  SignEmail_SignAll           = 1,
  SignEmail_Ask               = 2,
  SignEmail_DontSign          = 3
} SignEmail;

typedef enum {
  EncryptEmail_undef          = 0,

  EncryptEmail_EncryptAll     = 1,
  EncryptEmail_Ask            = 2,
  EncryptEmail_DontEncrypt    = 3
} EncryptEmail;

typedef enum {
  CertSrc_undef               = 0,

  CertSrc_Server              = 1,
  CertSrc_Local               = 2,
  CertSrc_ServerLocal         = CertSrc_Server | CertSrc_Local
} CertificateSource;


/*! \ingroup groupSignAct
    \brief Flags used to compose the SigStatusFlags value.

    This status flags are used to compose the SigStatusFlags value
    returned in \c SignatureMetaDataExtendedInfo after trying to
    verify a signed message part's signature status.

    The normal flags may <b>not</b> be used together with the
    special SigStat_NUMERICAL_CODE flag. When finding the special
    SigStat_NUMERICAL_CODE flag in a SigStatusFlags value you
    can obtain the respective error code number by substracting
    the SigStatusFlags value by SigStat_NUMERICAL_CODE: this is
    used to transport special status information NOT matching
    any of the normal predefined status codes.

    \note to PlugIn developers: Implementations of the CryptPlug API
    should try to express their signature states by bit-wise OR'ing
    the normal SigStatusFlags values. Using the SigStat_NUMERICAL_CODE
    flag should only be used as for exceptional situations where no
    other flag(s) could be used. By using the normal status flags your
    PlugIn's users will be told an understandable description of the
    status - when using (SigStat_NUMERICAL_CODE + internalCode) they
    will only be shown the respective code number and have to look
    into your PlugIn's manual to learn about it's meaning...
*/
enum {
    SigStat_VALID       = 0x0001,   /* The signature is fully valid */
    SigStat_GREEN       = 0x0002,   /* The signature is good. */
    SigStat_RED         = 0x0004,   /* The signature is bad. */
    SigStat_KEY_REVOKED = 0x0010,   /* One key has been revoked. */
    SigStat_KEY_EXPIRED = 0x0020,   /* One key has expired. */
    SigStat_SIG_EXPIRED = 0x0040,   /* The signature has expired. */
    SigStat_KEY_MISSING = 0x0080,   /* Can't verify: key missing. */
    SigStat_CRL_MISSING = 0x0100,   /* CRL not available. */
    SigStat_CRL_TOO_OLD = 0x0200,   /* Available CRL is too old. */
    SigStat_BAD_POLICY  = 0x0400,   /* A policy was not met. */
    SigStat_SYS_ERROR   = 0x0800,   /* A system error occured. */

    SigStat_NUMERICAL_CODE = 0x8000 /* An other error occured. */
};
typedef unsigned long SigStatusFlags;

class CryptPlugWrapper;

class KDE_EXPORT CryptPlug {
  friend class CryptPlugWrapper;
protected:
  CryptPlug();
  virtual ~CryptPlug();

  // these must be set by subclasses:
  gpgme_protocol_t GPGMEPLUG_PROTOCOL;
  GpgME::Context::Protocol mProtocol;

  /* definitions for signing */
  // 1. opaque signatures (only used for S/MIME)
  int GPGMEPLUG_OPA_SIGN_INCLUDE_CLEARTEXT;
  int GPGMEPLUG_OPA_SIGN_MAKE_MIME_OBJECT;
  int GPGMEPLUG_OPA_SIGN_MAKE_MULTI_MIME;
  const char * GPGMEPLUG_OPA_SIGN_CTYPE_MAIN;
  const char * GPGMEPLUG_OPA_SIGN_CDISP_MAIN;
  const char * GPGMEPLUG_OPA_SIGN_CTENC_MAIN;
  const char * GPGMEPLUG_OPA_SIGN_CTYPE_VERSION;
  const char * GPGMEPLUG_OPA_SIGN_CDISP_VERSION;
  const char * GPGMEPLUG_OPA_SIGN_CTENC_VERSION;
  const char * GPGMEPLUG_OPA_SIGN_BTEXT_VERSION;
  const char * GPGMEPLUG_OPA_SIGN_CTYPE_CODE;
  const char * GPGMEPLUG_OPA_SIGN_CDISP_CODE;
  const char * GPGMEPLUG_OPA_SIGN_CTENC_CODE;
  const char * GPGMEPLUG_OPA_SIGN_FLAT_PREFIX;
  const char * GPGMEPLUG_OPA_SIGN_FLAT_SEPARATOR;
  const char * GPGMEPLUG_OPA_SIGN_FLAT_POSTFIX;
  // 2. detached signatures (used for S/MIME and for OpenPGP)
  int GPGMEPLUG_DET_SIGN_INCLUDE_CLEARTEXT;
  int GPGMEPLUG_DET_SIGN_MAKE_MIME_OBJECT;
  int GPGMEPLUG_DET_SIGN_MAKE_MULTI_MIME;
  const char * GPGMEPLUG_DET_SIGN_CTYPE_MAIN;
  const char * GPGMEPLUG_DET_SIGN_CDISP_MAIN;
  const char * GPGMEPLUG_DET_SIGN_CTENC_MAIN;
  const char * GPGMEPLUG_DET_SIGN_CTYPE_VERSION;
  const char * GPGMEPLUG_DET_SIGN_CDISP_VERSION;
  const char * GPGMEPLUG_DET_SIGN_CTENC_VERSION;
  const char * GPGMEPLUG_DET_SIGN_BTEXT_VERSION;
  const char * GPGMEPLUG_DET_SIGN_CTYPE_CODE;
  const char * GPGMEPLUG_DET_SIGN_CDISP_CODE;
  const char * GPGMEPLUG_DET_SIGN_CTENC_CODE;
  const char * GPGMEPLUG_DET_SIGN_FLAT_PREFIX;
  const char * GPGMEPLUG_DET_SIGN_FLAT_SEPARATOR;
  const char * GPGMEPLUG_DET_SIGN_FLAT_POSTFIX;
  // 3. common definitions for opaque and detached signing
  int __GPGMEPLUG_SIGNATURE_CODE_IS_BINARY;

  /* definitions for encoding */
  int GPGMEPLUG_ENC_INCLUDE_CLEARTEXT;
  int GPGMEPLUG_ENC_MAKE_MIME_OBJECT;
  int GPGMEPLUG_ENC_MAKE_MULTI_MIME;
  const char * GPGMEPLUG_ENC_CTYPE_MAIN;
  const char * GPGMEPLUG_ENC_CDISP_MAIN;
  const char * GPGMEPLUG_ENC_CTENC_MAIN;
  const char * GPGMEPLUG_ENC_CTYPE_VERSION;
  const char * GPGMEPLUG_ENC_CDISP_VERSION;
  const char * GPGMEPLUG_ENC_CTENC_VERSION;
  const char * GPGMEPLUG_ENC_BTEXT_VERSION;
  const char * GPGMEPLUG_ENC_CTYPE_CODE;
  const char * GPGMEPLUG_ENC_CDISP_CODE;
  const char * GPGMEPLUG_ENC_CTENC_CODE;
  const char * GPGMEPLUG_ENC_FLAT_PREFIX;
  const char * GPGMEPLUG_ENC_FLAT_SEPARATOR;
  const char * GPGMEPLUG_ENC_FLAT_POSTFIX;
  int __GPGMEPLUG_ENCRYPTED_CODE_IS_BINARY;
  // end-of(these must be set by subclasses)

public:

#define CRYPTPLUG_CERT_DOES_NEVER_EXPIRE 365000




/*! \ingroup groupGeneral
    \brief This function returns the version string of this cryptography
           plug-in.

   If the plugins initialization fails the calling process might want
   to display the library version number to the user for checking if
   there is an old version of the library installed...

   \note This function <b>must</b> be implemented by each plug-in using
   this API specification.
*/
const char* libVersion( void );

/*! \ingroup groupGeneral
    \brief This function returns a URL to be used for reporting a bug that
           you found (or suspect, resp.) in this cryptography plug-in.

   If the plugins for some reason cannot specify an appropriate URL you
   should at least be provided with a text giving you some advise on
   how to report a bug.

   \note This function <b>must</b> be implemented by each plug-in using
   this API specification.
*/
const char* bugURL( void );


/*! \ingroup groupGeneral
    \brief Return the current interface version of the plugin.

   Return the current interface version.  This is a simple way for a
   user to check whether all required fucntions are available.  If
   MIN_VERSION is not NULL the lowest supported version of the
   interface is returned in addition.

   The version is a positive integer.  A user should check for the
   existance of this function before using it; if the fucntion does
   not exist, a interface version of 0 should be assumed.

   This function may be called prior to initialize().
  */
int interfaceVersion (int *min_version);
#define CRYPTPLUG_ERR_WRONG_KEY_USAGE 0x7070

/*! \ingroup groupGeneral
    \brief This function sets up all internal structures.

   Plugins that need no initialization should provide an empty
   implementation. The method returns \c true if the initialization was
   successful and \c false otherwise. Before this function is called,
   no other plugin functions should be called; the behavior is
   undefined in this case.

   \note This function <b>must</b> be implemented by each plug-in using
   this API specification.
*/
bool initialize( void );

/*! \ingroup groupGeneral
    \brief This function frees all internal structures.

    Plugins that do not keep any internal structures should provide an
    empty implementation. After this function has been called,
    no other plugin functions should be called; the behavior is
    undefined in this case.

   \note This function <b>must</b> be implemented by each plug-in using
   this API specification.
*/
//void deinitialize( void );

/*! \ingroup groupGeneral
   \brief This function returns \c true if the
          specified feature is available in the plugin, and
          \c false otherwise.

   Not all plugins will support all features; a complete Sphinx
   implementation will support all features contained in the enum,
   however.

   \note This function <b>must</b> be implemented by each plug-in using
   this API specification.
*/
bool hasFeature( Feature );

/*! \ingroup groupSignCryptAct
   \brief Information record returned by signing and by encrypting
   functions - this record should be used together with a
   corresponding \c free_StructuringInfo() function call.

   Use this information to compose a MIME object containing signed
   and/or encrypted content (or to build a text frame around your
   flat non-MIME message body, resp.)

   <b>If</b> value returned in \c makeMimeObject is <b>TRUE</b> the
   text strings returned in \c contentTypeMain and \c contentDispMain
   and \c contentTEncMain (and, if required, \c content[..]Version and
   \c bodyTextVersion and \c content[..]Sig) should be used to compose
   a respective MIME object.<br>
   If <b>FALSE</b> the texts returned in \c flatTextPrefix and
   \c flatTextSeparator and \c flatTextPostfix are to be used instead.<br>
   Allways <b>either</b> the \c content[..] and \c bodyTextVersion
   parameters <b>or</b> the \c flatText[..] parameters are holding
   valid data - never both of them may be used simultaneously
   as plugins will just ignore the parameters not matching their
   \c makeMimeObject setting.

   When creating your MIME object please observe these common rules:
   \li Parameters named \c contentType[..] and \c contentDisp[..] and
   \c contentTEnc[..] will return the values for the respective MIME
   headers 'Content-Type' and 'Content-Disposition' and
   'Content-Transfer-Encoding'. The following applies to these parameters:
   \li The relevant MIME part may <b>only</b> be created if the respective
   \c contentType[..] parameter is holding a non-zero-length string. If the
   \c contentType[..] parameter value is invalid or holding an empty string
   the respective \c contentDisp[..] and \c contentTEnc[..] parameters
   should be ignored.
   \li If the respective \c contentDisp[..] or \c contentTEnc[..] parameter
   is NULL or holding a zero-length string it is up to you whether you want
   to add the relevant MIME header yourself, but since it in in the
   responsibility of the plugin implementors to provide you with all
   neccessary 'Content-[..]' header information you should <b>not need</b>
   to define them if they are not returned by the signing or encrypting
   function - otherwise this may be considered as a bug in the plugin and
   you could report the missing MIME header information to the address
   returned by the \c bugURL() function.

   If \c makeMultiMime returns FALSE the \c contentTypeMain returned must
   not be altered but used to specify a single part mime object holding the
   code bloc, e.g. this is used for 'enveloped-data' single part MIME
   objects. In this case you should ignore both the \c content[..]Version
   and \c content[..]Code parameters.

   If \c makeMultiMime returns TRUE also the following rules apply:
   \li If \c includeCleartext is TRUE you should include the cleartext
   as first part of our multipart MIME object, typically this is TRUE
   when signing mails but FALSE when encrypting.
   \li The \c contentTypeMain returned typically starts with
   "multipart/" while providing a "protocol" and a "micalg" parameter: just
   add an appropriate \c "; boundary=[your \c boundary \c string]" to get
   the complete Content-Type value to be used for the MIME object embedding
   both the signed part and the signature part (or - in case of
   encrypting - the version part and the code part, resp.).
   \li If \c contentTypeVersion is holding a non-zero-length string an
   additional MIME part must added immediately before the code part, this
   version part's MIME headers must have the unaltered values of
   \c contentTypeVersion and (if they are holding non-zero-length strings)
   \c contentDispVersion and \c contentTEncVersion, the unaltered contents
   of \c bodyTextVersion must be it's body.
   \li The value returned in \c contentTypeCode is specifying the complete
   Content-Type to be used for this multipart MIME object's signature part
   (or - in case of encrypting - for the code part following after the
   version part, resp.), you should not add/change/remove anything here
   but just use it's unaltered value for specifying the Content-Type header
   of the respective MIME part.
   \li The same applies to the \c contentDispCode value: just use it's
   unaltered value to specify the Content-Disposition header entry of
   the respective MIME part.
   \li The same applies to the \c contentTEncCode value: just use it's
   unaltered value to specify the Content-Transfer-Encoding header of
   the respective MIME part.

   <b>If</b> value returned in \c makeMimeObject is <b>FALSE</b> the
   text strings returned in \c flatTextPrefix and \c flatTextPostfix
   should be used to build a frame around the cleartext and the code
   bloc holding the signature (or - in case of encrypting - the encoded
   data bloc, resp.).<br>
   If \c includeCleartext is TRUE this frame should also include the
   cleartext as first bloc, this bloc should be divided from the code bloc
   by the contents of \c flatTextSeparator - typically this is used for
   signing but not when encrypting.<br>
   If \c includeCleartext is FALSE you should ignore both the cleartext
   and the \c flatTextSeparator parameter.

   <b>How to use StructuringInfo data in your program:</b>
   \li To compose a signed message please act as described below.
   \li For constructing an encrypted message just replace the
   \c signMessage() call by the respective \c encryptMessage() call
   and then proceed exactly the same way.
   \li In any case make <b>sure</b> to free your \c ciphertext <b>and</b>
   to call \c free_StructuringInfo() when you are done with processing
   the data returned by the signing (or encrypting, resp.) function.

\verbatim

    char* ciphertext;
    StructuringInfo structInf;

    if( ! signMessage( cleartext, &ciphertext, certificate,
                       &structuring ) ) {

        myErrorDialog( "Error: could not sign the message!" );

    } else {
      if( structInf.makeMimeObject ) {

        // Build the main MIME object.
        // This is done by
        // using the header values returned in
        // structInf.contentTypeMain and in
        // structInf.contentDispMain and in
        // structInf.contentTEncMain.
        ..

        if( ! structInf.makeMultiMime ) {

          // Build the main MIME object's body.
          // This is done by
          // using the code bloc returned in
          // ciphertext.
          ..

        } else {

          // Build the encapsulated MIME parts.
          if( structInf.includeCleartext ) {

            // Build a MIME part holding the cleartext.
            // This is done by
            // using the original cleartext's headers and by
            // taking it's original body text.
            ..

          }
          if(    structInf.contentTypeVersion
              && 0 < strlen( structInf.contentTypeVersion ) ) {

            // Build a MIME part holding the version information.
            // This is done by
            // using the header values returned in
            // structInf.contentTypeVersion and
            // structInf.contentDispVersion and
            // structInf.contentTEncVersion and by
            // taking the body contents returned in
            // structInf.bodyTextVersion.
            ..

          }
          if(    structInf.contentTypeCode
              && 0 < strlen( structInf.contentTypeCode ) ) {

            // Build a MIME part holding the code information.
            // This is done by
            // using the header values returned in
            // structInf.contentTypeCode and
            // structInf.contentDispCode and
            // structInf.contentTEncCode and by
            // taking the body contents returned in
            // ciphertext.
            ..

          } else {

            // Plugin error!
            myErrorDialog( "Error: Cryptography plugin returned a main"
                           "Content-Type=Multipart/.. but did not "
                           "specify the code bloc's Content-Type header."
                           "\nYou may report this bug:"
                           "\n" + cryptplug.bugURL() );
          }
        }
      } else  {

        // Build a plain message body
        // based on the values returned in structInf.
        // Note: We do _not_ insert line breaks between the parts since
        //       it is the plugin job to provide us with ready-to-use
        //       texts containing all neccessary line breaks.
        strcpy( myMessageBody, structInf.plainTextPrefix );
        if( structInf.includeCleartext ) {
          strcat( myMessageBody, cleartext );
          strcat( myMessageBody, structInf.plainTextSeparator );
        }
        strcat( myMessageBody, *ciphertext );
        strcat( myMessageBody, structInf.plainTextPostfix );
      }

      // free the memory that was allocated
      // for the ciphertext
      free( ciphertext );

      // free the memory that was allocated
      // for our StructuringInfo's char* members
      free_StructuringInfo( &structuring );
    }

\endverbatim

   \note Make sure to call \c free_StructuringInfo() when you are done
   with processing the StructuringInfo data!

  \see free_StructuringInfo
  \see signMessage, encryptMessage, encryptAndSignMessage
*/
struct StructuringInfo {
  bool includeCleartext;     /*!< specifies whether we should include the
                                  cleartext as first part of our multipart
                                  MIME object (or - for non-MIME
                                  messages - as flat text to be set before
                                  the ciphertext, resp.), typically this
                                  is TRUE when signing mails but FALSE
                                  when encrypting<br>
                                  (this parameter is relevant no matter
                                  whether \c makeMimeObject is TRUE or
                                  FALSE) */
  bool  makeMimeObject;      /*!< specifies whether we should create a MIME
                                  object or a flat text message body */
  /* the following are used for MIME messages only */
  bool  makeMultiMime;       /*!< specifies whether we should create a
                                  'Multipart' MIME object or a single part
                                  object, if FALSE only \c contentTypeMain,
                                  \c contentDispMain and \c contentTEncMain
                                  may be used and all other parameters have
                                  to be ignored<br>
                                  (ignore this parameter if \c makeMimeObject
                                  is FALSE) */
  char* contentTypeMain;     /*!< value of the main 'Content-Type'
                                  header<br>
                                  (ignore this parameter if \c makeMimeObject
                                  is FALSE) */
  char* contentDispMain;     /*!< value of the main 'Content-Disposition'
                                  header<br>
                                  (ignore this parameter if \c makeMimeObject
                                  is FALSE) */
  char* contentTEncMain;     /*!< value of the main
                                  'Content-TransferEncoding' header<br>
                                  (ignore this parameter if \c makeMimeObject
                                  is FALSE) */
  char* contentTypeVersion;  /*!< 'Content-Type' of the additional version
                                  part that might preceed the code part -
                                  if NULL or zero length no version part
                                  must be created<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE) */
  char* contentDispVersion;  /*!< 'Content-Disposition' of the additional
                                  preceeding the code part (only valid if
                                  \c contentTypeVersion holds a
                                  non-zero-length string)<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE or if \c contentTypeVersion does
                                  not return a non-zero-length string) */
  char* contentTEncVersion;  /*!< 'Content-Transfer-Encoding' of the
                                  additional version part (only valid if
                                  \c contentTypeVersion holds a
                                  non-zero-length string)<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE or if \c contentTypeVersion does
                                  not return a non-zero-length string) */
  char* bodyTextVersion;     /*!< body text of the additional version part
                                  (only valid if \c contentTypeVersion
                                  holds a non-zero-length string)<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE or if \c contentTypeVersion does
                                  not return a non-zero-length string) */
  char* contentTypeCode;     /*!< 'Content-Type' of the code part holding
                                  the signature code (or the encrypted
                                  data, resp.)<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE) */
  char* contentDispCode;     /*!< 'Content-Disposition' of the code part<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE or if \c contentTypeCode does
                                  not return a non-zero-length string) */
  char* contentTEncCode;     /*!< 'Content-Type' of the code part<br>
                                  (ignore this parameter if either
                                  \c makeMimeObject or \c makeMultiMime
                                  is FALSE or if \c contentTypeCode does
                                  not return a non-zero-length string) */
  /* the following are used for flat non-MIME messages only */
  char* flatTextPrefix;      /*!< text to preceed the main text (or the
                                  code bloc containing the encrypted main
                                  text, resp.)<br>
                                  (ignore this parameter if
                                  \c makeMimeObject is TRUE) */
  char* flatTextSeparator;   /*!< text to be put between the main text and
                                  the signature code bloc (not used when
                                  encrypting)<br>
                                  (ignore this parameter if
                                  \c makeMimeObject is TRUE or if
                                  \c includeCleartext is FALSE) */
  char* flatTextPostfix;     /*!< text to follow the signature code bloc
                                  (or the encrypted data bloc, resp.)<br>
                                  (ignore this parameter if
                                  \c makeMimeObject is TRUE) */
};


/*! \ingroup groupAdUsoInterno
    \brief If you are not planning to write your own cryptography
    plugin <b>you should ignore this</b> function!

    Usage of this function is depreciated for plugin users but highly
    recommended for plugin implementors since this is an internal
    function for initializing all char* members of a \c StructuringInfo
    struct.<br>
    This function <b>must</b> be called in <b>any</b> plugin's
    implementations of the following functions:

    \c signMessage() <br>
    \c encryptMessage() <br>
    \c encryptAndSignMessage()

    Calling this function makes sure the corresponding
    \c free_StructuringInfo() calls which will be embedded by
    your plugin's users into their code will be able to
    determine which of the char* members belonging to the
    respective's StructuringInfo had been allocated memory
    for during previous signing or encrypting actions.

    \see free_StructuringInfo, StructuringInfo
    \see signMessage, encryptMessage, encryptAndSignMessage
*/
  inline void init_StructuringInfo( struct StructuringInfo* s )
  {
    if( ! s ) return;

    s->includeCleartext = false;

    s->makeMimeObject = false;
    s->makeMultiMime = false;

    s->contentTypeMain = 0;
    s->contentDispMain = 0;
    s->contentTEncMain = 0;

    s->contentTypeVersion = 0;
    s->contentDispVersion = 0;
    s->contentTEncVersion = 0;
    s->bodyTextVersion = 0;

    s->contentTypeCode = 0;
    s->contentDispCode = 0;
    s->contentTEncCode = 0;

    s->flatTextPrefix = 0;
    s->flatTextSeparator = 0;
    s->flatTextPostfix = 0;
  }

/*! \ingroup groupSignCryptAct
    \brief Important method for freeing all memory that was allocated
    for the char* members of a \c StructuringInfo struct - use
    this function after <b>each</b> signing or encrypting function
    call.

    \note Even when intending to call \c encryptMessage() immediately
    after having called \c signMessage() you first <b>must</b> call
    the \c free_StructuringInfo() function to make sure all memory is
    set free that was allocated for your StructuringInfo's char* members
    by the \c signMessage() function!

    \see StructuringInfo
*/
  inline void free_StructuringInfo( struct StructuringInfo* s )
  {
    if( ! s ) return;
    if( s->contentTypeMain )    free( s->contentTypeMain );
    if( s->contentDispMain )    free( s->contentDispMain );
    if( s->contentTEncMain )    free( s->contentTEncMain );
    if( s->contentTypeVersion ) free( s->contentTypeVersion );
    if( s->contentDispVersion ) free( s->contentDispVersion );
    if( s->contentTEncVersion ) free( s->contentTEncVersion );
    if( s->bodyTextVersion )    free( s->bodyTextVersion );
    if( s->contentTypeCode )    free( s->contentTypeCode );
    if( s->contentDispCode )    free( s->contentDispCode );
    if( s->contentTEncCode )    free( s->contentTEncCode );
    if( s->flatTextPrefix )     free( s->flatTextPrefix );
    if( s->flatTextSeparator )  free( s->flatTextSeparator );
    if( s->flatTextPostfix )    free( s->flatTextPostfix );
  }


/*! \ingroup groupSignAct
   \brief Signs a message \c cleartext and returns
          in \c *ciphertext the signature data bloc that
          is to be added to the message. The length returned
          in \c *cipherLen tells you the size (==amount of bytes)
          of the ciphertext, if the structuring information
          would return with contentTEncCode set to "base64"
          the ciphertext might contain a char 0x00
          and has to be converted into base64 before sending.

   The signature role is specified by \c certificate.
   If \c certificate is \c NULL, the default certificate is used.

   If the message could be signed, the function returns
          \c true, otherwise
          \c false.

   Use the StructuringInfo data returned in parameter \c structuring
   to find out how to build the respective MIME object (or the plain
   text message body, resp.).

   \note The function allocates memory for the \c *ciphertext, so
         make sure you set free that memory when no longer needing
         it (as shown in example code provided with documentation
         of the struct \c StructuringInfo).

   \note The function also allocates memory for some char* members
    of the StructuringInfo* parameter that you are providing,
    therefore you <b>must</b> call the \c free_StructuringInfo() function
    to make sure all memory is set free that was allocated. This must be
    done <b>before</b> calling the next cryptography function - even if
    you intend to call \c encryptMessage() immediately after
    \c signMessage().

   \see StructuringInfo, free_StructuringInfo
*/
bool signMessage( const char*  cleartext,
                  char** ciphertext,
                  const size_t* cipherLen,
                  const char*  certificate,
                  struct StructuringInfo* structuring,
                  int* errId,
                  char** errTxt,
		  SendCertificates sendCertificates,
		  SignatureCompoundMode signatureCompoundMode );


/*! \ingroup groupSignAct
 */
struct SignatureMetaDataExtendedInfo
{
    struct tm* creation_time;
    SigStatusFlags sigStatusFlags;
    char* status_text;
    char* keyid;
    char* fingerprint;
    char* algo;
    char* userid;
    char* name;
    char* comment;
    char** emailList;
    int    emailCount;
    unsigned long algo_num;
    unsigned long validity;
    unsigned long userid_num;
    unsigned long keylen;
    unsigned long key_created;
    unsigned long key_expires;
};

/*! \ingroup groupSignAct
*/
struct SignatureMetaData {
    char* status;
    struct SignatureMetaDataExtendedInfo* extended_info;
    int extended_info_count;
    int status_code;
};

/*! \ingroup groupSignAct
   \brief Checks whether the signature of a message is
          valid.

   \c cleartext must never be 0 but be a valid pointer.

   If \c *cleartext > 0 then **cleartext specifies the message text
   that was signed and \c signaturetext is the signature itself.

   If \c *cleartext == 0 is an empty string then \c signaturetext is
   supposed to contain an opaque signed message part. After checking the
   data and verifying the signature the cleartext of the message will be
   returned in \c cleartext.  The user must free the respective memory
   ocupied by *cleartext.

   Depending on the configuration, MUAs might not need to use this.
   If \c sigmeta is non-null, the
          \c SignatureMetaData object pointed to will
          contain meta information about the signature after the
          function call.
*/
bool checkMessageSignature( char** cleartext,
                            const char* signaturetext,
                            bool signatureIsBinary,
                            int signatureLen,
                            struct SignatureMetaData* sigmeta,
                            char** attrOrder,
                            const char* unknownAttrsHandling );

/*! \ingroup groupSignAct
   \brief Stores the certificates that follow with the message
          \c ciphertext locally.
*/
bool storeCertificatesFromMessage( const char* ciphertext );


/*! \ingroup groupCryptAct
   \brief Find all certificate for a given addressee.

  NOTE: The certificate parameter must point to a not-yet allocated
        char*.  The function will allocate the memory needed and
        return the size in newSize.
   If secretOnly is true, only secret keys are returned.
*/
bool findCertificates( const char* addressee,
                       char** certificates,
                       int* newSize,
                       bool secretOnly,
                       char** attrOrder,
                       const char* unknownAttrsHandling );

/*! \ingroup groupCryptAct
   \brief Encrypts an email message in
          \c cleartext according to the \c addressee and
          the current settings (algorithm, etc.) and
          returns the encoded data bloc in \c *ciphertext.
          The length returned in \c *cipherLen tells you the
          size (==amount of bytes) of the ciphertext, if the
          structuring information would return with
          contentTEncCode set to "base64" the ciphertext
          might contain a char 0x00 and has to be converted
          into base64 before sending.

   If the message could be encrypted, the function returns
          \c true, otherwise
          \c false.

   Use the StructuringInfo data returned in parameter \c structuring
   to find out how to build the respective MIME object (or the plain
   text message body, resp.).

   \note The function allocates memory for the \c *ciphertext, so
         make sure you set free that memory when no longer needing
         it (as shown in example code provided with documentation
         of the struct \c StructuringInfo).

   \note The function also allocates memory for some char* members
    of the StructuringInfo* parameter that you are providing,
    therefore you <b>must</b> call the \c free_StructuringInfo() function
    to make sure all memory is set free that was allocated. This must be
    done <b>before</b> calling the next cryptography function!

   \see StructuringInfo, free_StructuringInfo
*/
bool encryptMessage( const char*  cleartext,
                     const char** ciphertext,
                     const size_t* cipherLen,
                     const char*  addressee,
                     struct StructuringInfo* structuring,
                     int* errId,
                     char** errTxt );


/*! \ingroup groupCryptAct
   \brief Combines the functionality of
          \c encryptMessage() and
          \c signMessage().

   If \c certificate is \c NULL,
   the default certificate will be used.

   If the message could be signed and encrypted, the function returns
          \c true, otherwise
          \c false.

   Use the StructuringInfo data returned in parameter \c structuring
   to find out how to build the respective MIME object (or the plain
   text message body, resp.).

   \note The function allocates memory for the \c *ciphertext, so
         make sure you set free that memory when no longer needing
         it (as shown in example code provided with documentation
         of the struct \c StructuringInfo).

   \note The function also allocates memory for some char* members
    of the StructuringInfo* parameter that you are providing,
    therefore you <b>must</b> call the \c free_StructuringInfo() function
    to make sure all memory is set free that was allocated. This must be
    done <b>before</b> calling the next cryptography function!

   \see StructuringInfo, free_StructuringInfo
*/
bool encryptAndSignMessage( const char* cleartext,
                            const char** ciphertext,
                            const char* certificate,
                            struct StructuringInfo* structuring );

/*! \ingroup groupCryptAct
   \brief Tries to decrypt an email message
          \c ciphertext and returns the decrypted
          message in \c cleartext.

   The \c certificate is used for decryption. If
          the message could be decrypted, the function returns
          \c true, otherwise
          \c false.
*/
bool decryptMessage( const char*  ciphertext,
                     bool         cipherIsBinary,
                     int          cipherLen,
                     const char** cleartext,
                     const char*  certificate,
                     int* errId,
                     char** errTxt );

/*! \ingroup groupCryptAct
   \brief Combines the functionality of
          \c checkMessageSignature() and
          \c decryptMessage().

   If \c certificate is \c NULL,
   the default certificate will be used.
   If \c sigmeta is non-null, the \c SignatureMetaData
   object pointed to will contain meta information about
   the signature after the function call.
*/
bool decryptAndCheckMessage( const char*  ciphertext,
                             bool         cipherIsBinary,
                             int          cipherLen,
                             const char** cleartext,
                             const char*  certificate,
                             bool*        signatureFound,
                             struct SignatureMetaData* sigmeta,
                             int*   errId,
                             char** errTxt,
                             char** attrOrder,
                             const char* unknownAttrsHandling );


struct CertIterator;

struct DnPair {
    char *key;
    char *value;
};

struct CertificateInfo {
  char** userid;
  char* userid_0_org;
  char* serial;
  char* fingerprint;

  char* issuer_org;
  char* issuer_reord;
  char* chainid;

  char* caps;

  unsigned long created;
  unsigned long expire;

  int secret   : 1;
  int invalid  : 1;
  int expired  : 1;
  int disabled : 1;

  struct DnPair *dnarray; /* parsed values from userid[0] */
};

/*! \fn struct CertIterator*  startListCertificates( const char* pattern );
    \fn struct CertificateInfo*  nextCertificate( struct CertIterator* );
    \fn void endListCertificates( struct CertIterator* );

    \ingroup certList
  Example that runs through certs matching "Steffen":
\verbatim
  struct CertificateInfo* info;
  struct CertIterator* it = startListCertificates("Steffen", 0 );
  while( nextCertificate( it, &info ) == GPGME_No_Error && info ) {
    do something with info.
    dont free() it, the struct will be reused
    by the next call to nextCertificate()
  }
  int truncated = endListCertificates( it );
\endverbatim
*/
struct CertIterator*
startListCertificates( const char* pattern, int remote );

int
nextCertificate( struct CertIterator*,
                 struct CertificateInfo** result,
                 char** attrOrder,
                 const char* unknownAttrsHandling );

int
endListCertificates( struct CertIterator* );

  /*!
    Import a certificate from memory.
  */
  GpgME::ImportResult importCertificateFromMem( const char* data, size_t length );
}; // class CryptPlug

class SMIMECryptPlug : public CryptPlug {
public:
  SMIMECryptPlug();
};

class OpenPGPCryptPlug : public CryptPlug {
public:
  OpenPGPCryptPlug();
};

#endif /*CRYPTPLUG_H*/

