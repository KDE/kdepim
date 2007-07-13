/**
 * cryptplugwrapper.h
 *
 * Copyright (c) 2001 Karl-Heinz Zimmer, Klaraelvdalens Datakonsult AB
 *
 * This CRYPTPLUG wrapper interface is based on cryptplug.h by
 * Karl-Heinz Zimmer which is based on 'The Aegypten Plugin API' as
 * specified by Matthias Kalle Dalheimer, Klaraelvdalens Datakonsult AB,
 * see file mua-integration.sgml located on Aegypten CVS:
 *          http://www.gnupg.org/aegypten/development.en.html
 *
 * purpose: Wrap up all Aegypten Plugin API functions in one C++ class
 *          for usage by KDE programs, e.g. KMail (or KMime, resp.)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef cryptplugwrapper_h
#define cryptplugwrapper_h

#include <cryptplug.h>

#ifndef LIBKLEOPATRA_NO_COMPAT

/*
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                        *
 *  This file's source comments are optimized for processing by Doxygen.  *
 *                                                                        *
 *  To obtain best results please get an updated version of Doxygen,      *
 *  for sources and binaries goto http://www.doxygen.org/index.html       *
 *                                                                        *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

                                                                          */
#include <libkleo_export.h>
#include <cryptobackend.h>

#include <QDateTime>
#include <q3valuelist.h>
#include <QPair>
#include <QStringList>
#include <QString>

class QGpgMECryptoConfig;

namespace GpgME {
  class ImportResult;
  }

namespace Kleo {
  class KeyListJob;
  class EncryptJob;
  class DecryptJob;
  class SignJob;
  class VerifyDetachedJob;
  class VerifyOpaqueJob;
  class KeyGenerationJob;
  class ImportJob;
  class ExportJob;
  class DownloadJob;
  class DeleteJob;
  class SignEncryptJob;
  class DecryptVerifyJob;
  class CryptoConfig;
  class RefreshKeysJob;
  class SpecialJob;
}

/*! \file cryptplugwrapper.h
    \brief C++ wrapper for the CRYPTPLUG library API.

    This CRYPTPLUG wrapper interface is based on cryptplug.h by
    Karl-Heinz Zimmer which is based on 'The Aegypten Plugin API' as
    specified by Matthias Kalle Dalheimer, Klaraelvdalens Datakonsult AB,
    see file mua-integration.sgml located on Aegypten CVS:
             http://www.gnupg.org/aegypten/development.en.html

    purpose: Wrap up all Aegypten Plugin API functions in one C++ class
             for usage by KDE programs, e.g. KMail (or KMime, resp.)

    CRYPTPLUG is an independent cryptography plug-in API
    developed for Sphinx-enabeling KMail and Mutt.

    CRYPTPLUG was designed for the Aegypten project, but it may
    be used by 3rd party developers as well to design pluggable
    crypto backends for the above mentioned MUAs.

    \note All string parameters appearing in this API are to be
    interpreted as UTF-8 encoded.

    \see cryptplugwrapper.cpp
*/

/*! \defgroup groupAdmin Constructor, destructor and setting of 'active' flag

    The functions in this section are used for general administration of
    this CRYPTPLUG wrapper class and for maintaining a separate \c active flag
    for environments using more than one CRYPTPLUG library simultaneously.
*/

/*! \defgroup groupGeneral Loading and Unloading the Plugin, General Functionality

    The functions in this section are used for loading and
    unloading the respective CRYPTPLUG library, for (re)setting
    it's internal data structures and for retrieving information
    on the implementation state of all functions covered by the CRYPTPLUG API.
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



class CryptPlugWrapper;
/*! \ingroup groupSignCryptAct
  \brief This class provides C++ access to the StructuringInfo helper
         struct that is specified in cryptplug.h to hold information
         returned by signing and by encrypting functions.

  Use this information to compose a MIME object containing signed
  and/or encrypted content (or to build a text frame around your
  flat non-MIME message body, resp.)

  \note This class is different from the respective cryptplug.h class
  because this one takes care for freeing the char** members' memory
  automatically. You must <b>not</b> call the \c free function for
  any of it's members - just ignore the advise given in the
  cryptplug.h documentation!

  <b>If</b> value returned in \c makeMimeObject is <b>TRUE</b> the
  text strings returned in \c contentTypeMain and \c contentDispMain
  and \c contentTEncMain (and, if required, \c content[..]Version and
  \c bodyTextVersion and \c content[..]Sig) should be used to compose
  a respective MIME object.<br>
  If <b>FALSE</b> the texts returned in \c flatTextPrefix and
  \c flatTextSeparator and \c flatTextPostfix are to be used instead.<br>
  Always <b>either</b> the \c content[..] and \c bodyTextVersion
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
  necessary 'Content-[..]' header information you should <b>not need</b>
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

  <b>How to use StructuringInfoWrapper data in your program:</b>
  \li To compose a signed message please act as described below.
  \li For constructing an encrypted message just replace the
  \c signMessage() call by the respective \c encryptMessage() call
  and then proceed exactly the same way.
  \li In any case make <b>sure</b> to free your \c ciphertext when
  you are done with processing the data returned by the signing
  (or encrypting, resp.) function.

\verbatim

    char* ciphertext;
    StructuringInfoWrapper structInf;

    if( ! signMessage( cleartext, &ciphertext, certificate,
                      structInf ) ) {

        myErrorDialog( "Error: could not sign the message!" );

    } else {
      if( structInf.data.makeMimeObject ) {

        // Build the main MIME object.
        // This is done by
        // using the header values returned in
        // structInf.data.contentTypeMain and in
        // structInf.data.contentDispMain and in
        // structInf.data.contentTEncMain.
        ..

        if( ! structInf.data.makeMultiMime ) {

          // Build the main MIME object's body.
          // This is done by
          // using the code bloc returned in
          // ciphertext.
          ..

        } else {

          // Build the encapsulated MIME parts.
          if( structInf.data.includeCleartext ) {

            // Build a MIME part holding the cleartext.
            // This is done by
            // using the original cleartext's headers and by
            // taking it's original body text.
            ..

          }
          if(    structInf.data.contentTypeVersion
              && 0 < strlen( structInf.data.contentTypeVersion ) ) {

            // Build a MIME part holding the version information.
            // This is done by
            // using the header values returned in
            // structInf.data.contentTypeVersion and
            // structInf.data.contentDispVersion and
            // structInf.data.contentTEncVersion and by
            // taking the body contents returned in
            // structInf.data.bodyTextVersion.
            ..

          }
          if(    structInf.data.contentTypeCode
              && 0 < strlen( structInf.data.contentTypeCode ) ) {

            // Build a MIME part holding the code information.
            // This is done by
            // using the header values returned in
            // structInf.data.contentTypeCode and
            // structInf.data.contentDispCode and
            // structInf.data.contentTEncCode and by
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
        //       texts containing all necessary line breaks.
        strcpy( myMessageBody, structInf.data.plainTextPrefix );
        if( structInf.data.includeCleartext ) {
          strcat( myMessageBody, cleartext );
          strcat( myMessageBody, structInf.data.plainTextSeparator );
        }
        strcat( myMessageBody, *ciphertext );
        strcat( myMessageBody, structInf.data.plainTextPostfix );
      }

      // free the memory that was allocated
      // for the ciphertext
      free( ciphertext );
    }

\endverbatim

  \see signMessage, encryptMessage, encryptAndSignMessage
*/
class StructuringInfoWrapper {
public:
  StructuringInfoWrapper( CryptPlugWrapper* wrapper );
  ~StructuringInfoWrapper();
  void reset();
  CryptPlug::StructuringInfo data;
private:
  void initMe();
  void freeMe();
  bool _initDone;
  CryptPlugWrapper* _wrapper;
};



/*!
    \brief This class provides C++ access to the CRYPTPLUG API.
*/
class KLEO_EXPORT CryptPlugWrapper : public Kleo::CryptoBackend::Protocol {
public:
    static QString errorIdToText( int errId, bool & isPassphraseError );

    /*! \ingroup groupGeneral

        \brief Current initialization state.

        This flag holding status of previous call of initialize function.
        If initialize was not called before return value will be
        \c CryptPlugInit_undef.

        \sa initStatus, initialize
    */
    typedef enum {
        InitStatus_undef         = 0,

        InitStatus_Ok            = 1,
        InitStatus_NoLibName     = 2,
        InitStatus_LoadError     = 0x1000,
        InitStatus_InitError     = 0x2000
    } InitStatus;

    /*! \ingroup groupSignAct
        \brief Flags used to compose the SigStatusFlags value.

        This status flags are used to compose the SigStatusFlags value
        returned in \c SignatureMetaDataExtendedInfo after trying to
        verify a signed message part's signature status.

        The normal flags may <b>not</b> be used together with the
        special SigStatus_NUMERICAL_CODE flag. When finding the special
        SigStatus_NUMERICAL_CODE flag in a SigStatusFlags value you
        can obtain the respective error code number by substracting
        the SigStatusFlags value by SigStatus_NUMERICAL_CODE: this is
        used to transport special status information NOT matching
        any of the normal predefined status codes.

        \note to PlugIn developers: Implementations of the CryptPlug API
        should try to express their signature states by bit-wise OR'ing
        the normal SigStatusFlags values. Using the SigStatus_NUMERICAL_CODE
        flag should only be used as for exceptional situations where no
        other flag(s) could be used. By using the normal status flags your
        PlugIn's users will be told an understandable description of the
        status - when using (SigStatus_NUMERICAL_CODE + internalCode) they
        will only be shown the respective code number and have to look
        into your PlugIn's manual to learn about it's meaning...
    */
    enum {
        SigStatus_UNKNOWN     = 0x0000,
        SigStatus_VALID       = SigStat_VALID,
        SigStatus_GREEN       = SigStat_GREEN,
        SigStatus_RED         = SigStat_RED,
        SigStatus_KEY_REVOKED = SigStat_KEY_REVOKED,
        SigStatus_KEY_EXPIRED = SigStat_KEY_EXPIRED,
        SigStatus_SIG_EXPIRED = SigStat_SIG_EXPIRED,
        SigStatus_KEY_MISSING = SigStat_KEY_MISSING,
        SigStatus_CRL_MISSING = SigStat_CRL_MISSING,
        SigStatus_CRL_TOO_OLD = SigStat_CRL_TOO_OLD,
        SigStatus_BAD_POLICY  = SigStat_BAD_POLICY,
        SigStatus_SYS_ERROR   = SigStat_SYS_ERROR,

        SigStatus_NUMERICAL_CODE = 0x8000 /* An other error occurred. */
    };
    typedef unsigned long SigStatusFlags;


    enum {
        CertStatus_EXPIRES_NEVER = CRYPTPLUG_CERT_DOES_NEVER_EXPIRE
    };


    /*! \ingroup groupAdmin
        \brief Constructor of CRYPTPLUG wrapper class.

        This constructor does <b>not</b> call the initialize() method
        but just stores some information for later use.

        \note Since more than one crypto plug-in might be specified (using
              multiple instances of the warpper class) it is necessary to
              set \c active at least one them. Only wrappers that have been
              activated may be initialized or configured or used to perform
              crypto actions.

        \param name    The external name that is visible in lists, messages,
                       etc.
        \param libName Complete path+name of CRYPTPLUG library that is to
                       be used by this instance of CryptPlugWrapper.
        \param update  the URL from where updates can be downloaded
        \param active  Specify whether the relative library is to be used
                       or not.

        \sa ~CryptPlugWrapper, setActive, active, initialize, deinitialize
        \sa initStatus
    */
    CryptPlugWrapper( const QString& name=QString(),
                      const QString& libName=QString(),
                      const QString& update=QString(),
                      bool           active = false );

    /*! \ingroup groupAdmin
        \brief Destructor of CRYPTPLUG wrapper class.

        This destructor <b>does</b> call the deinitialize() method in case
        this was not done by explicitly calling it before.

        \sa deinitialize, initialize, CryptPlugWrapper(), setActive, active
        \sa
    */
    ~CryptPlugWrapper();

    QString protocol() const;

    QString name() const {
      return protocol();
    }

    /*! \ingroup groupAdmin
        \brief Set this CRYPTPLUG wrapper's internal \c active flag.

        Since more than one crypto plug-in might be specified (using
        multiple instances of the warpper class) it is necessary to
        set \c active at least one them. Only wrappers that have been
        activated may be initialized or configured or used to perform
        crypto actions.

        This flag may be set in the constructor or by calling setActive().

        \note Deactivating does <b>not</b> mean resetting the internal
              structures - if just prevents the normal functions from
              being called erroneously. When deactivated only the following
              functions are operational: constructor , destructor ,
              setActive , active, setLibName , libName , initStatus;
              calling other functions will be ignored and their return
              values will be undefined.

        \param active  Specify whether the relative library is to be used
                       or not.

        \sa active, CryptPlugWrapper(), ~CryptPlugWrapper
        \sa deinitialize, initialize, initStatus
    */
    void setActive( bool active );

    /*! \ingroup groupAdmin
        \brief Returns this CRYPTPLUG wrapper's internal \c active flag.

        \return  whether the relative library is to be used or not.

        \sa setActive
    */
    bool active() const;


    /*! \ingroup groupAdmin
        \brief Set the CRYPTPLUG library name.

        Complete path+name of CRYPTPLUG library that is to
                       be used by this instance of CryptPlugWrapper.

        This name may be set in the constructor or by calling setLibName().

        \note Setting/changing the library name may only be done when
              the initStatus() is <b>not</b> \c InitStatus_Ok.
              If you want to change the name of the library after
              successfully having called initialize() please make
              sure to unload it by calling the deinitialize() function.

        \param libName libName Complete path+name of CRYPTPLUG library
                       that is to be used by this CryptPlugWrapper.

        \return whether the library name could be changed; library name
                can only be changed when library is not initialized - see
                above 'note'.

        \sa libName, CryptPlugWrapper(), ~CryptPlugWrapper
        \sa deinitialize, initialize, initStatus
    */
    bool setLibName( const QString& libName );

    /*! \ingroup groupAdmin
        \brief Returns the CRYPTPLUG library name.

        \return  the complete path+name of CRYPTPLUG library that is to
                 be used by this instance of CryptPlugWrapper.

        \sa setLibName
    */
    QString libName() const;


    /*! \ingroup groupAdmin
      \brief Specifies the external name that is visible in lists,
             messages, etc.
    */
    void setDisplayName( const QString& name );


    /*! \ingroup groupAdmin
      \brief Returns the external name.
      \return the external name used for display purposes
    */
    QString displayName() const;

private:
    /*! \ingroup groupGeneral
    \brief This function does two things: (a) load the lib and (b) set up all internal structures.

        The method tries to load the CRYPTPLUG library specified
        in the constructor and returns \c true if the both <b>loading
        and initializing</b> the internal data structures was successful
        and \c false otherwise. Before this function is called,
        no other plugin functions should be called; the behavior is
        undefined in this case, this rule does not apply to the functions
        \c setActive() and \c setLibName().

        \param initStatus will receive the resulting InitStatus if not NULL
        \param errorMsg will receive the system error message if not NULL

        \sa initStatus, deinitialize, CryptPlugWrapper(), ~CryptPlugWrapper
        \sa setActive, active
    */
    bool initialize( InitStatus* initStatus, QString* errorMsg );

public:
    /*! \ingroup groupGeneral
    \brief This function unloads the lib and frees all internal structures.

        After this function has been called, no other plugin functions
        should be called; the behavior is undefined in this case.

        \note Deinitializing sets the internal initStatus value back
              to \c InitStatus_undef.

        \sa initStatus, initialize, CryptPlugWrapper, ~CryptPlugWrapper
        \sa setActive, active
    */
    void deinitialize();

    /*! \ingroup groupGeneral
        \brief Returns this CRYPTPLUG wrapper's initialization state.

        \param errorMsg receives the last system error message, this value
        should be ignored if InitStatus value equals \c InitStatus_Ok.

        \return  whether the relative library was loaded and initialized
                 correctly

        \sa initialize, deinitialize, CryptPlugWrapper(), ~CryptPlugWrapper
        \sa setActive, active
    */
    InitStatus initStatus( QString* errorMsg ) const;


    /*! \ingroup groupGeneral
        \brief This function returns \c true if the
            specified feature is available in the plugin, and
            \c false otherwise.

        Not all plugins will support all features; a complete Sphinx
        implementation will support all features contained in the enum,
        however.

        \note In case this function cannot be executed the system's error
        message may be retrieved by calling initStatus( QString* ).

        \return  whether the relative feature is implemented or not
    */
    bool hasFeature( Feature );


    /* \ingroup groupSignAct
     * Frees the members of a signature meta data struct, but not the
     * signature meta data struct itself as this could be allocated on
     * the stack.
     */
    void freeSignatureMetaData( CryptPlug::SignatureMetaData* );

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
      occupied by *cleartext.

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
                                CryptPlug::SignatureMetaData* sigmeta );

    /*! \ingroup groupCryptAct
    \brief Tries to decrypt an email message
            \c ciphertext and returns the decrypted
            message in \c cleartext.

    The \c certificate is used for decryption. If
            the message could be decrypted, the function returns
            \c true, otherwise
            \c false.
    */
    bool decryptMessage( const char* ciphertext,
                         bool        cipherIsBinary,
                         int         cipherLen,
                         char**      cleartext,
                         const char* certificate,
                         int* errId,
                         char** errTxt );

    /*! \ingroup groupCryptAct
    \brief Combines the functionality of
            \c checkMessageSignature() and
            \c decryptMessage().

    If \c certificate is \c NULL,
            the default certificate will be used.  If
            \c sigmeta is non-null, the
            \c SignatureMetaData object pointed to will
            contain meta information about the signature after the
            function call.
    */
    bool decryptAndCheckMessage( const char*  ciphertext,
                                 bool         cipherIsBinary,
                                 int          cipherLen,
                                 char**       cleartext,
                                 const char*  certificate,
                                 bool*        signatureFound,
                                 CryptPlug::SignatureMetaData* sigmeta,
                                 int*   errId,
                                 char** errTxt );

    Kleo::KeyListJob * keyListJob( bool remote=false, bool includeSigs=false, bool validate=true ) const;
    Kleo::EncryptJob * encryptJob( bool armor=false, bool textmode=false ) const;
    Kleo::DecryptJob * decryptJob() const;
    Kleo::SignJob * signJob( bool armor=false, bool textMode=false ) const;
    Kleo::VerifyDetachedJob * verifyDetachedJob( bool textmode=false) const;
    Kleo::VerifyOpaqueJob * verifyOpaqueJob( bool textmode=false ) const;
    Kleo::KeyGenerationJob * keyGenerationJob() const;

    Kleo::ImportJob * importJob() const;
    Kleo::ExportJob * publicKeyExportJob( bool armor=false ) const;
    Kleo::ExportJob * secretKeyExportJob( bool armor=false, const QString& charset = QString::null ) const;
    Kleo::DownloadJob * downloadJob( bool armor=false ) const;
    Kleo::DeleteJob * deleteJob() const;

    Kleo::SignEncryptJob * signEncryptJob( bool armor=false, bool textmode=false ) const;
    Kleo::DecryptVerifyJob * decryptVerifyJob( bool textmode=false ) const;
    Kleo::RefreshKeysJob * refreshKeysJob() const;

    Kleo::SpecialJob * specialJob( const char *, const QMap<QString,QVariant> & ) const { return 0; }

    GpgME::ImportResult importCertificate( const char* data, size_t length );

    CryptPlug * cryptPlug() const { return _cp; }

private:
    QString    _name;
    QString    _libName;
    QString    _updateURL;
    bool       _active;
    InitStatus _initStatus;
    QString    _lastError;
    CryptPlug* _cp;
    // local parameters without representation in cryptplug.h
    bool mAlwaysEncryptToSelf;
    class Config;
    Config * _config;
    QGpgMECryptoConfig * _cryptoConfig;
};

#endif // !LIBKLEOPATRA_NO_COMPAT

#endif // cryptplugwrapper_h
