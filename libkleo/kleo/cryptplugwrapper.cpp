/**
 * cryptplugwrapper.cpp
 *
 * Copyright (c) 2001 Karl-Heinz Zimmer, Klaraelvdalens Datakonsult AB
 *
 * This CRYPTPLUG wrapper implementation is based on cryptplug.h by
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "cryptplugwrapper.h"
#include "cryptplug.h"

#include <backends/qgpgme/qgpgmekeylistjob.h>
#include <backends/qgpgme/qgpgmeencryptjob.h>
#include <backends/qgpgme/qgpgmedecryptjob.h>
#include <backends/qgpgme/qgpgmesignjob.h>
#include <backends/qgpgme/qgpgmeverifydetachedjob.h>
#include <backends/qgpgme/qgpgmeverifyopaquejob.h>
#include <backends/qgpgme/qgpgmekeygenerationjob.h>
#include <backends/qgpgme/qgpgmeimportjob.h>
#include <backends/qgpgme/qgpgmeexportjob.h>
#include <backends/qgpgme/qgpgmesecretkeyexportjob.h>
#include <backends/qgpgme/qgpgmedownloadjob.h>
#include <backends/qgpgme/qgpgmedeletejob.h>
#include <backends/qgpgme/qgpgmesignencryptjob.h>
#include <backends/qgpgme/qgpgmedecryptverifyjob.h>
#include <backends/qgpgme/qgpgmecryptoconfig.h>
#include <backends/qgpgme/qgpgmerefreshkeysjob.h>

// qgpgme
#include <qgpgme/dataprovider.h>

// gpgme++
#include <gpgme++/data.h>
#include <gpgme++/importresult.h>
#include <gpgme++/keygenerationresult.h>

// kde
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kconfiggroup.h>
// Qt
#include <QByteArray>
#include <QList>


// other
#include <memory>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>




/*
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                    *
 *  This file's source comments - as well as those in interface file  *
 *  cryptplugwrapper.h - are optimized for processing by Doxygen.     *
 *                                                                    *
 *  To obtain best results please get an updated version of Doxygen,  *
 *  for sources and binaries goto http://www.doxygen.org/index.html   *
 *                                                                    *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
                                                                      *
                                                                      */



/*! \file cryptplugwrapper.cpp
    \brief C++ wrapper for the CRYPTPLUG library API.

    This CRYPTPLUG wrapper implementation is based on cryptplug.h by
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

    \see cryptplugwrapper.h
*/


// a little helper class for reordering of DN attributes
class DNBeautifier {
public:
  enum UnknownAttrsHandling { unknownAttrsHide,
                              unknownAttrsPrefix,
                              unknownAttrsPostfix,
                              unknownAttrsInfix };
  // infix: at the position of "_X_", if any, else Postfix

  DNBeautifier()
  {
    // the attrOrder is defaulted to an empty string automatically
    _unknownAttrsHandling = unknownAttrsInfix;
    _unknownAttrsHandlingChar = "INFIX";
  }
  DNBeautifier( KConfig* config,
                const QString& cfgGroup,
                const QString& cfgAttributeOrderEntry,
                const QString& cfgUnknownAttrsEntry,
                const QStringList& fallbackAttrOrder = QStringList(),
                UnknownAttrsHandling fallbackUnknowAttrsHandling = unknownAttrsInfix )
  {
    _unknownAttrsHandling = unknownAttrsInfix;
    _unknownAttrsHandlingChar = "INFIX";
    if( config ){
      KConfigGroup groupCfgGroup( config, cfgGroup );
      _attrOrder =
        groupCfgGroup.readEntry( cfgAttributeOrderEntry , QStringList() );        // e.g. "DNAttributeOrder"
      _unknownAttrsHandlingChar =
        groupCfgGroup.readEntry( cfgUnknownAttrsEntry, QString() ).toUpper().toLatin1(); // e.g. "DNUnknownAttributes"
      if( _unknownAttrsHandlingChar == "HIDE" )
        _unknownAttrsHandling = unknownAttrsHide;
      else if( _unknownAttrsHandlingChar == "PREFIX" )
        _unknownAttrsHandling = unknownAttrsPrefix;
      else if( _unknownAttrsHandlingChar == "POSTFIX" )
        _unknownAttrsHandling = unknownAttrsPostfix;
      else if( _unknownAttrsHandlingChar == "INFIX" )
        _unknownAttrsHandling = unknownAttrsInfix;
      else
        _unknownAttrsHandlingChar = "INFIX";
    }
    if( _attrOrder.isEmpty() && ! fallbackAttrOrder.isEmpty() )
      _attrOrder = fallbackAttrOrder;

    if( _attrOrder.isEmpty() ){
      _attrOrderChar = 0;
    }else{
      _attrOrderChar = new char*[ _attrOrder.count()+1 ];
      int i=0;
      for( QStringList::ConstIterator itOrder = _attrOrder.begin();
           itOrder != _attrOrder.end();
           ++itOrder ){
        _attrOrderChar[ i ] = (char*)malloc( ((*itOrder).length()+1)*sizeof(char) );
        strcpy( _attrOrderChar[ i ], (*itOrder).toLatin1() );
        ++i;
      }
      _attrOrderChar[ i ] = NULL;
    }
  }
  ~DNBeautifier()
  {
    int i=0;
    for( QStringList::ConstIterator itOrder = _attrOrder.begin();
         itOrder != _attrOrder.end();
         ++itOrder ){
      free( _attrOrderChar[ i ] );
      ++i;
    }
    delete[] _attrOrderChar;
  }

  QStringList attrOrder() const
  {
    return _attrOrder;
  }
  char** attrOrderChar()
  {
    return _attrOrderChar;
  }

  UnknownAttrsHandling unknownAttrsHandling() const
  {
    return _unknownAttrsHandling;
  }
  const char* unknownAttrsHandlingChar() const
  {
    return _unknownAttrsHandlingChar;
  }

  QList< QPair<QString,QString> > reorder( const QList< QPair<QString,QString> > & dn ) const
  {
    return reorder( dn, _attrOrder, _unknownAttrsHandling );
  }


  static QList< QPair<QString,QString> > reorder(
    const QList< QPair<QString,QString> > & dn,
    QStringList attrOrder,
    UnknownAttrsHandling unknownAttrsHandling )
  {
    if( !attrOrder.isEmpty() ){
      QList< QPair<QString,QString> > unknownEntries;
      QPair<QString,QString> unknownEntry; // for Q_FOREACH

      QList< QPair<QString,QString> > dnNew;

      QStringList::ConstIterator itOrder;
      QList< QPair<QString,QString> >::ConstIterator itDN;
      bool bFound;

      if( unknownAttrsHandling != unknownAttrsHide ){
        // find all unknown entries in their order of appearance
        for( itDN = dn.begin(); itDN != dn.end(); ++itDN ){
          bFound = false;
          for( itOrder = attrOrder.begin(); itOrder != attrOrder.end(); ++itOrder ){
            if( (*itOrder) == (*itDN).first ){
              bFound = true;
              break;
            }
          }
          if( !bFound )
            unknownEntries.append( (*itDN) );
        }
      }

      // prepend the unknown attrs (if desired)
      if( unknownAttrsHandling == unknownAttrsPrefix ){
        Q_FOREACH( unknownEntry, unknownEntries ) {
          dnNew << unknownEntry;
        }
      }

      // process the known attrs in the desired order
      bool b_X_declared = false;
      for( itOrder = attrOrder.begin(); itOrder != attrOrder.end(); ++itOrder ){
        if( (*itOrder) == "_X_" ){
          b_X_declared = true;
          // insert the unknown attrs (if desired)
          if( unknownAttrsHandling == unknownAttrsInfix ){
            Q_FOREACH( unknownEntry, unknownEntries ) {
              dnNew << unknownEntry;
            }
          }
        }else{
          for( itDN = dn.begin(); itDN != dn.end(); ++itDN ){
            if( (*itOrder) == (*itDN).first ){
              dnNew << *itDN;
              //kDebug(5150) << QString((*itDN).first) <<" =" << QString((*itDN).second);;
            }
          }
        }
      }

      // append the unknown attrs (if desired)
      if( unknownAttrsHandling == unknownAttrsPostfix ||
          ( unknownAttrsHandling == unknownAttrsInfix && ! b_X_declared ) ){
        Q_FOREACH( unknownEntry, unknownEntries ) {
          dnNew << unknownEntry;
        }
      }

      return dnNew;
    }
    return dn;
  }

private:
  QStringList _attrOrder;
  char**      _attrOrderChar;
  UnknownAttrsHandling _unknownAttrsHandling;
  QByteArray    _unknownAttrsHandlingChar;
};



/* special helper class to be used by signing/encrypting functions *******/



StructuringInfoWrapper::StructuringInfoWrapper( CryptPlugWrapper* wrapper )
  : _initDone( false ), _wrapper( wrapper )
{
    initMe();
}
StructuringInfoWrapper::~StructuringInfoWrapper()
{
    freeMe();
}
void StructuringInfoWrapper::reset()
{
    freeMe();
    initMe();
}
void StructuringInfoWrapper::initMe()
{
    if ( _wrapper && _wrapper->cryptPlug() ) {
      _wrapper->cryptPlug()->init_StructuringInfo( &data );
      _initDone = true;
    }
}
void StructuringInfoWrapper::freeMe()
{
    if( _wrapper && _wrapper->cryptPlug() && _initDone ) {
      _wrapper->cryptPlug()->free_StructuringInfo( &data );
      _initDone = false;
    }
}

class CryptPlugWrapper::Config {
public:
  Config( gpgme_protocol_t proto );
  ~Config();

  const char*             signatureKeyCertificate;
  SignatureAlgorithm      signatureAlgorithm;
  SignatureCompoundMode   signatureCompoundMode;
  SendCertificates        sendCertificates;
  bool                    saveSentSignatures;
  bool                    warnNoCertificate;
  bool                    signatureUseCRLs;
  EncryptionAlgorithm     encryptionAlgorithm;
  EncryptEmail            encryptEmail;
  bool                    saveMessagesEncrypted;
  bool                    encryptionUseCRLs;
  bool                    encryptionCRLExpiryNearWarning;
  int                     encryptionCRLNearExpiryInterval;
  CertificateSource       certificateSource;
  bool                    warnSendUnsigned;
  bool                    signatureCertificateExpiryNearWarning;
  int                     signatureCertificateExpiryNearInterval;
  bool                    cACertificateExpiryNearWarning;
  int                     cACertificateExpiryNearInterval;
  bool                    rootCertificateExpiryNearWarning;
  int                     rootCertificateExpiryNearInterval;
  bool                    warnSendUnencrypted;
  bool                    checkCertificatePath;
  bool                    receiverCertificateExpiryNearWarning;
  int                     receiverCertificateExpiryNearWarningInterval;
  bool                    certificateInChainExpiryNearWarning;
  int                     certificateInChainExpiryNearWarningInterval;
  bool                    receiverEmailAddressNotInCertificateWarning;
  const char* libVersion; /* a statically allocated string with the GPGME Version used */
};

static const int NEAR_EXPIRY = 14;

CryptPlugWrapper::Config::Config( gpgme_protocol_t proto )
{
  signatureAlgorithm                   = SignAlg_SHA1;
  if ( proto == GPGME_PROTOCOL_CMS )
    signatureCompoundMode              = SignatureCompoundMode_Opaque;
  else
    signatureCompoundMode              = SignatureCompoundMode_Detached;
  sendCertificates                     = SendCert_SendChainWithRoot;
  saveSentSignatures                   = true;
  warnNoCertificate                    = true;
  signatureUseCRLs                     = true;
  encryptionAlgorithm                  = EncryptAlg_RSA;
  encryptEmail                         = EncryptEmail_Ask;
  saveMessagesEncrypted                = true;
  encryptionUseCRLs                    = true;
  encryptionCRLExpiryNearWarning       = false;
  encryptionCRLNearExpiryInterval      = NEAR_EXPIRY;
  certificateSource                    = CertSrc_Server;
  warnSendUnsigned                             = true;
  signatureCertificateExpiryNearWarning        = true;
  signatureCertificateExpiryNearInterval       = NEAR_EXPIRY;
  cACertificateExpiryNearWarning               = true;
  cACertificateExpiryNearInterval              = NEAR_EXPIRY;
  rootCertificateExpiryNearWarning             = true;
  rootCertificateExpiryNearInterval            = NEAR_EXPIRY;
  warnSendUnencrypted                          = false;
  checkCertificatePath                         = true;
  receiverCertificateExpiryNearWarning         = true;
  receiverCertificateExpiryNearWarningInterval = NEAR_EXPIRY;
  certificateInChainExpiryNearWarning          = true;
  certificateInChainExpiryNearWarningInterval  = NEAR_EXPIRY;
  receiverEmailAddressNotInCertificateWarning  = true;
  libVersion = gpgme_check_version (NULL);
}

CryptPlugWrapper::Config::~Config() {
}

/* Some multi purpose functions ******************************************/

QString CryptPlugWrapper::errorIdToText( int errId, bool & isPassphraseError ) {
  const GpgME::Error err( errId );
  isPassphraseError = err.isCanceled()
    || gpgme_err_code( errId ) == GPG_ERR_NO_SECKEY ; // FIXME: more?
  return QString::fromLocal8Bit( err.asString() );
}

/* some special functions ************************************************/


CryptPlugWrapper::CryptPlugWrapper( const QString& name,
                                    const QString& libName,
                                    const QString& update,
                                    bool           active )
  : Kleo::CryptoBackend::Protocol(),
    _name( name ),
    _libName( libName ),
    _updateURL( update ),
    _active(  active  ),
    _initStatus( InitStatus_undef ),
    _cp( 0 ),
    _config( 0 ),
    _cryptoConfig( 0 )
{
  const bool ok = initialize( 0, 0 );
  assert( ok );
}


CryptPlugWrapper::~CryptPlugWrapper()
{
    deinitialize();
}


void CryptPlugWrapper::setActive( bool active )
{
    _active = active;
}


bool CryptPlugWrapper::active() const
{
    return _active;
}



bool CryptPlugWrapper::setLibName( const QString& libName )
{
    bool bOk = ! _cp;           // Changing the lib name is only allowed
    if( bOk )                   // when either no initialization took
        _libName = libName;     // place or 'deinitialize()' has been
    return bOk;                 // called afterwards.
}

QString CryptPlugWrapper::libName() const
{
    return _libName;
}

QString CryptPlugWrapper::protocol() const
{
  if ( _libName.contains( "smime" ) )
    return "SMIME";
  if ( _libName.contains( "openpgp" ) )
    return "OpenPGP";
  return QString();
}

void CryptPlugWrapper::setDisplayName( const QString& name )
{
    _name = name;
}


QString CryptPlugWrapper::displayName() const
{
    if ( !_name.isEmpty() )
      return _name;
    if ( _libName.contains( "smime" ) )
      return "gpgsm";
    if ( _libName.contains( "openpgp" ) )
      return "gpg";
    return i18n("(Unknown Protocol)");
}

bool CryptPlugWrapper::initialize( InitStatus* initStatus, QString* errorMsg )
{
    if ( _cp )
      return true;

    _initStatus = InitStatus_undef;
    /* make sure we have a lib name */
    if ( _libName.isEmpty() ) {
      _initStatus = InitStatus_NoLibName;
      kDebug(5150) <<"No library name was given.";
    } else {
      if ( _libName.contains( "smime" ) ) {
	_cp = new SMIMECryptPlug();
	_config = new Config( GPGME_PROTOCOL_CMS );
      } else if ( _libName.contains( "openpgp" ) ) {
	_cp = new OpenPGPCryptPlug();
	_config = new Config( GPGME_PROTOCOL_OpenPGP );
      } else {
	_cp = 0;
	_config = 0;
      }

      if ( !_cp ) {
	_initStatus = InitStatus_LoadError;
	kDebug(5150) <<"Couldn't create '" << _libName.toLatin1() <<"'";
      } else {
	/* now call the init function */
	if( !_cp->initialize() ) {
	  _initStatus = InitStatus_InitError;
	  kDebug(5150) <<"Error while executing function 'initialize' on plugin" << _libName;
	  _lastError = i18n("Error while initializing plugin \"%1\"", _libName );
	  if ( errorMsg )
	    *errorMsg = _lastError;
	  delete _cp; _cp = 0;
	  delete _config; _config = 0;
	} else {
	  _initStatus  = InitStatus_Ok;
	}
      }
    }
    if( initStatus )
        *initStatus = _initStatus;
    return _initStatus == InitStatus_Ok;
}



void CryptPlugWrapper::deinitialize()
{
    delete _cp; _cp = 0;
    delete _config; _config = 0;
    delete _cryptoConfig; _cryptoConfig = 0;
}


CryptPlugWrapper::InitStatus CryptPlugWrapper::initStatus( QString* errorMsg ) const
{
    if( errorMsg )
        *errorMsg = _lastError;
    return _initStatus;
}


bool CryptPlugWrapper::hasFeature( Feature flag )
{
  return _cp && _cp->hasFeature( flag );
}


/* normal functions ******************************************************/

bool CryptPlugWrapper::checkMessageSignature( char** cleartext,
                                              const char* signaturetext,
                                              bool signatureIsBinary,
                                              int signatureLen,
                                              CryptPlug::SignatureMetaData* sigmeta )
{
  DNBeautifier dnBeautifier( KGlobal::config().data(),
                             "DN",
                             "AttributeOrder",
                             "UnknownAttributes" );
  return _cp && _cp->checkMessageSignature( cleartext,
                                            signaturetext,
                                            signatureIsBinary,
                                            signatureLen,
                                            sigmeta,
                                            dnBeautifier.attrOrderChar(),
                                            dnBeautifier.unknownAttrsHandlingChar() );
}


bool CryptPlugWrapper::decryptMessage( const char* ciphertext,
                                       bool        cipherIsBinary,
                                       int         cipherLen,
                                       char**      cleartext,
                                       const char* certificate,
                                       int* errId,
                                       char** errTxt )
{
  return _cp && _cp->decryptMessage( ciphertext, cipherIsBinary, cipherLen,
				     (const char**)cleartext, certificate, errId, errTxt );
}


bool CryptPlugWrapper::decryptAndCheckMessage(
                            const char*  ciphertext,
                            bool         cipherIsBinary,
                            int          cipherLen,
                            char**       cleartext,
                            const char*  certificate,
                            bool*        signatureFound,
                            CryptPlug::SignatureMetaData* sigmeta,
                            int*   errId,
                            char** errTxt )
{
  DNBeautifier dnBeautifier( KGlobal::config().data(),
                             "DN",
                             "AttributeOrder",
                             "UnknownAttributes" );
  return _cp && _cp->decryptAndCheckMessage( ciphertext,
                                             cipherIsBinary,
                                             cipherLen,
                                             (const char**)cleartext,
                                             certificate,
                                             signatureFound,
                                             sigmeta,
                                             errId,
                                             errTxt,
                                             dnBeautifier.attrOrderChar(),
                                             dnBeautifier.unknownAttrsHandlingChar() );
}




void CryptPlugWrapper::freeSignatureMetaData( CryptPlug::SignatureMetaData* sigmeta )
{
    if ( !sigmeta )
      return;
    free( sigmeta->status );
    for( int i = 0; i < sigmeta->extended_info_count; ++i ) {
        free( sigmeta->extended_info[i].creation_time );
        free( (void*)sigmeta->extended_info[i].status_text );
        free( (void*)sigmeta->extended_info[i].keyid );
        free( (void*)sigmeta->extended_info[i].fingerprint );
        free( (void*)sigmeta->extended_info[i].algo );
        free( (void*)sigmeta->extended_info[i].userid );
        free( (void*)sigmeta->extended_info[i].name );
        free( (void*)sigmeta->extended_info[i].comment );
        if( sigmeta->extended_info[i].emailCount ){
            for( int j=0; j < sigmeta->extended_info[i].emailCount; ++j)
                if( sigmeta->extended_info[i].emailList[j] )
                    free( (void*)sigmeta->extended_info[i].emailList[j] );
            free( (void*)sigmeta->extended_info[i].emailList );
        }
    }
    free( sigmeta->extended_info );
}

GpgME::ImportResult CryptPlugWrapper::importCertificate( const char* data, size_t length )
{
    if ( !_cp )
      return GpgME::ImportResult();


   return _cp->importCertificateFromMem( data, length );
}

Kleo::KeyListJob * CryptPlugWrapper::keyListJob( bool remote, bool includeSigs, bool validate ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  unsigned int mode = context->keyListMode();
  if ( remote ) {
    mode |= GpgME::Extern;
    mode &= ~GpgME::Local;
  } else {
    mode |= GpgME::Local;
    mode &= ~GpgME::Extern;
  }
  if ( includeSigs ) mode |= GpgME::Signatures;
  if ( validate ) mode |= GpgME::Validate;
  context->setKeyListMode( mode );
  return new Kleo::QGpgMEKeyListJob( context );
}

Kleo::EncryptJob * CryptPlugWrapper::encryptJob( bool armor, bool textmode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  context->setTextMode( textmode );
  return new Kleo::QGpgMEEncryptJob( context );             
}

Kleo::DecryptJob * CryptPlugWrapper::decryptJob() const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;
  return new Kleo::QGpgMEDecryptJob( context );             
}

Kleo::SignJob * CryptPlugWrapper::signJob( bool armor, bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  context->setTextMode( textMode );
  return new Kleo::QGpgMESignJob( context );                    
}

Kleo::VerifyDetachedJob * CryptPlugWrapper::verifyDetachedJob( bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setTextMode( textMode );
  return new Kleo::QGpgMEVerifyDetachedJob( context );             
}

Kleo::VerifyOpaqueJob * CryptPlugWrapper::verifyOpaqueJob( bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setTextMode( textMode );
  return new Kleo::QGpgMEVerifyOpaqueJob( context );   
}

Kleo::KeyGenerationJob * CryptPlugWrapper::keyGenerationJob() const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;
  return new Kleo::QGpgMEKeyGenerationJob( context );             
}

Kleo::ImportJob * CryptPlugWrapper::importJob() const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;
  return new Kleo::QGpgMEImportJob( context );             
}

Kleo::ExportJob * CryptPlugWrapper::publicKeyExportJob( bool armor ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  return new Kleo::QGpgMEExportJob( context );             
}

Kleo::ExportJob * CryptPlugWrapper::secretKeyExportJob( bool armor, const QString& charset ) const {
  if ( !_cp || _cp->mProtocol != GpgME::CMS ) // fixme: add support for gpg, too
    return 0;

  // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
  return new Kleo::QGpgMESecretKeyExportJob( armor, charset );            
}

Kleo::RefreshKeysJob * CryptPlugWrapper::refreshKeysJob() const {
  if ( !_cp || _cp->mProtocol != GpgME::CMS ) // fixme: add support for gpg, too
    return 0;

  // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
  return new Kleo::QGpgMERefreshKeysJob();             
}

Kleo::DownloadJob * CryptPlugWrapper::downloadJob( bool armor ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  // this is the hackish interface for downloading from keyserers currently:
  context->setKeyListMode( GpgME::Extern );
  return new Kleo::QGpgMEDownloadJob( context );             
}

Kleo::DeleteJob * CryptPlugWrapper::deleteJob() const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;
  return new Kleo::QGpgMEDeleteJob( context );             
}

Kleo::SignEncryptJob * CryptPlugWrapper::signEncryptJob( bool armor, bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  context->setTextMode( textMode );
  return new Kleo::QGpgMESignEncryptJob( context );             
}

Kleo::DecryptVerifyJob * CryptPlugWrapper::decryptVerifyJob( bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setTextMode( textMode );
  return new Kleo::QGpgMEDecryptVerifyJob( context );             
}
