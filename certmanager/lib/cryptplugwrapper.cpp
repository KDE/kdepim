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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

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
#include <gpgmepp/data.h>
#include <gpgmepp/importresult.h>
#include <gpgmepp/keygenerationresult.h>

// kde
#include <kdebug.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>

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
      const QString oldGroup( config->group() );
      config->setGroup( cfgGroup );                             // e.g. "General"
      _attrOrder =
        config->readListEntry( cfgAttributeOrderEntry );        // e.g. "DNAttributeOrder"
      _unknownAttrsHandlingChar =
        config->readEntry( cfgUnknownAttrsEntry ).upper().latin1(); // e.g. "DNUnknownAttributes"
      config->setGroup( oldGroup );
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
        strcpy( _attrOrderChar[ i ], (*itOrder).latin1() );
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

  QValueList< QPair<QString,QString> > reorder( const QValueList< QPair<QString,QString> > & dn ) const
  {
    return reorder( dn, _attrOrder, _unknownAttrsHandling );
  }


  static QValueList< QPair<QString,QString> > reorder(
    const QValueList< QPair<QString,QString> > & dn,
    QStringList attrOrder,
    UnknownAttrsHandling unknownAttrsHandling )
  {
    if( !attrOrder.isEmpty() ){
      QPtrList<   QPair<QString,QString> > unknownEntries;
      QValueList< QPair<QString,QString> > dnNew;

      QPair<QString,QString>* unknownEntry;
      QStringList::ConstIterator itOrder;
      QValueList< QPair<QString,QString> >::ConstIterator itDN;
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
            unknownEntries.append( &(*itDN) );
        }
      }

      // prepend the unknown attrs (if desired)
      if( unknownAttrsHandling == unknownAttrsPrefix ){
        for( unknownEntry = unknownEntries.first(); unknownEntry; unknownEntry = unknownEntries.next() ){
          dnNew << *unknownEntry;
        }
      }

      // process the known attrs in the desired order
      bool b_X_declared = false;
      for( itOrder = attrOrder.begin(); itOrder != attrOrder.end(); ++itOrder ){
        if( (*itOrder) == "_X_" ){
          b_X_declared = true;
          // insert the unknown attrs (if desired)
          if( unknownAttrsHandling == unknownAttrsInfix ){
            for( unknownEntry = unknownEntries.first(); unknownEntry; unknownEntry = unknownEntries.next() ){
              dnNew << *unknownEntry;
            }
          }
        }else{
          for( itDN = dn.begin(); itDN != dn.end(); ++itDN ){
            if( (*itOrder) == (*itDN).first ){
              dnNew << *itDN;
              //kdDebug(5150) << QString((*itDN).first) <<" = " << QString((*itDN).second) << endl;;
            }
          }
        }
      }

      // append the unknown attrs (if desired)
      if( unknownAttrsHandling == unknownAttrsPostfix ||
          ( unknownAttrsHandling == unknownAttrsInfix && ! b_X_declared ) ){
        for( unknownEntry = unknownEntries.first(); unknownEntry; unknownEntry = unknownEntries.next() ){
          dnNew << *unknownEntry;
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
  QCString    _unknownAttrsHandlingChar;
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

const char* CryptPlugWrapper::bugURL(){ return "http://www.gnupg.org/aegypten/"; }


void CryptPlugWrapper::setSignatureAlgorithm( SignatureAlgorithm sigAlg )
{
  _config->signatureAlgorithm = sigAlg;
}

SignatureAlgorithm CryptPlugWrapper::signatureAlgorithm()
{
  return _config->signatureAlgorithm;
}




void CryptPlugWrapper::setWarnSendUnsigned( bool flag )
{
  _config->warnSendUnsigned = flag;
}

bool CryptPlugWrapper::warnSendUnsigned()
{
  return _config->warnSendUnsigned;
}









void CryptPlugWrapper::setSignatureCertificateExpiryNearWarning( bool flag )
{
  _config->signatureCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::signatureCertificateExpiryNearWarning( void )
{
  return _config->signatureCertificateExpiryNearWarning;
}

void CryptPlugWrapper::setSignatureCertificateExpiryNearInterval( int interval )
{
  _config->signatureCertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::signatureCertificateExpiryNearInterval( void )
{
  return _config->signatureCertificateExpiryNearInterval;
}

void CryptPlugWrapper::setCACertificateExpiryNearWarning( bool flag )
{
  _config->cACertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::caCertificateExpiryNearWarning( void )
{
  return _config->cACertificateExpiryNearWarning;
}

void CryptPlugWrapper::setCACertificateExpiryNearInterval( int interval )
{
  _config->cACertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::caCertificateExpiryNearInterval( void )
{
  return _config->cACertificateExpiryNearInterval;
}

void CryptPlugWrapper::setRootCertificateExpiryNearWarning( bool flag )
{
  _config->rootCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::rootCertificateExpiryNearWarning( void )
{
  return _config->rootCertificateExpiryNearWarning;
}

void CryptPlugWrapper::setRootCertificateExpiryNearInterval( int interval )
{
  _config->rootCertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::rootCertificateExpiryNearInterval( void )
{
  return _config->rootCertificateExpiryNearInterval;
}





void CryptPlugWrapper::setEncryptionAlgorithm( EncryptionAlgorithm cryptAlg )
{
  _config->encryptionAlgorithm = cryptAlg;
}

EncryptionAlgorithm CryptPlugWrapper::encryptionAlgorithm()
{
  return _config->encryptionAlgorithm;
}

void CryptPlugWrapper::setEncryptEmail( EncryptEmail cryptMode )
{
  _config->encryptEmail = cryptMode;
}

EncryptEmail CryptPlugWrapper::encryptEmail()
{
  return _config->encryptEmail;
}






void CryptPlugWrapper::setWarnSendUnencrypted( bool flag )
{
  _config->warnSendUnencrypted = flag;
}

bool CryptPlugWrapper::warnSendUnencrypted()
{
  return _config->warnSendUnencrypted;
}









void CryptPlugWrapper::setSaveMessagesEncrypted( bool flag )
{
  _config->saveMessagesEncrypted = flag;
}

bool CryptPlugWrapper::saveMessagesEncrypted()
{
  return _config->saveMessagesEncrypted;
}







void CryptPlugWrapper::setCheckCertificatePath( bool flag )
{
  _config->checkCertificatePath = flag;
}

bool CryptPlugWrapper::checkCertificatePath()
{
  return _config->checkCertificatePath;
}












void CryptPlugWrapper::setReceiverCertificateExpiryNearWarning( bool flag )
{
  _config->receiverCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::receiverCertificateExpiryNearWarning()
{
  return _config->receiverCertificateExpiryNearWarning;
}


void CryptPlugWrapper::setReceiverCertificateExpiryNearWarningInterval( int interval )
{
  _config->receiverCertificateExpiryNearWarningInterval = interval;
}

int CryptPlugWrapper::receiverCertificateExpiryNearWarningInterval()
{
  return _config->receiverCertificateExpiryNearWarningInterval;
}

void CryptPlugWrapper::setCertificateInChainExpiryNearWarning( bool flag )
{
  _config->certificateInChainExpiryNearWarning = flag;
}

bool CryptPlugWrapper::certificateInChainExpiryNearWarning()
{
  return _config->certificateInChainExpiryNearWarning;
}


void CryptPlugWrapper::setCertificateInChainExpiryNearWarningInterval( int interval )
{
  _config->certificateInChainExpiryNearWarningInterval = interval;
}

int CryptPlugWrapper::certificateInChainExpiryNearWarningInterval()
{
  return _config->certificateInChainExpiryNearWarningInterval;
}

void CryptPlugWrapper::setReceiverEmailAddressNotInCertificateWarning( bool flag )
{
  _config->receiverEmailAddressNotInCertificateWarning = flag;
}

bool CryptPlugWrapper::receiverEmailAddressNotInCertificateWarning()
{
  return _config->receiverEmailAddressNotInCertificateWarning;
}








void CryptPlugWrapper::setEncryptionUseCRLs( bool flag )
{
  _config->encryptionUseCRLs = flag;

  /* PENDING(g10) Store this setting in gpgme and use it. If true,
     every certificate used for encryption should be checked against
     applicable CRLs.
  */
}

bool CryptPlugWrapper::encryptionUseCRLs()
{
  return _config->encryptionUseCRLs;
}


void CryptPlugWrapper::setEncryptionCRLExpiryNearWarning( bool flag )
{
  _config->encryptionCRLExpiryNearWarning = flag;
}

bool CryptPlugWrapper::encryptionCRLExpiryNearWarning()
{
  return _config->encryptionCRLExpiryNearWarning;
}

void CryptPlugWrapper::setEncryptionCRLNearExpiryInterval( int interval )
{
  _config->encryptionCRLNearExpiryInterval = interval;
}

int CryptPlugWrapper::encryptionCRLNearExpiryInterval()
{
  return _config->encryptionCRLNearExpiryInterval;
}


void CryptPlugWrapper::setCertificateSource( CertificateSource source )
{
  _config->certificateSource = source;
}

CertificateSource CryptPlugWrapper::certificateSource()
{
  return _config->certificateSource;
}





QString CryptPlugWrapper::libVersion() const {
  return _config && _config->libVersion ? QString::fromUtf8( _config->libVersion ) : QString::null ;
}

/* Some multi purpose functions ******************************************/

QString CryptPlugWrapper::errorIdToText( int errId, bool & isPassphraseError ) {
  const GpgME::Error err( errId );
  isPassphraseError = err.isCanceled()
    || gpgme_err_code( errId ) == GPG_ERR_NO_SECKEY ; // FIXME: more?
  return QString::fromLocal8Bit( err.asString() );
#if 0
  switch( errId ){
  case /*GPGME_EOF                = */-1:
    return(i18n("End of File reached during operation."));
  case /*GPGME_No_Error           = */0:
    return(i18n("No error."));
  case /*GPGME_General_Error      = */1:
    return(i18n("General error."));
  case /*GPGME_Out_Of_Core        = */2:
    return(i18n("Out of core."));
  case /*GPGME_Invalid_Value      = */3:
    return(i18n("Invalid value."));
  case /*GPGME_Busy               = */4:
    return(i18n("Engine is busy."));
  case /*GPGME_No_Request         = */5:
    return(i18n("No request."));
  case /*GPGME_Exec_Error         = */6:
    return(i18n("Execution error."));
  case /*GPGME_Too_Many_Procs     = */7:
    return(i18n("Too many processes."));
  case /*GPGME_Pipe_Error         = */8:
    return(i18n("Pipe error."));
  case /*GPGME_No_Recipients      = */9:
    return(i18n("No recipients."));
  case /*GPGME_No_Data            = */10:
    return(i18n("No data."));
  case /*GPGME_Conflict           = */11:
    return(i18n("Conflict."));
  case /*GPGME_Not_Implemented    = */12:
    return(i18n("Not implemented."));
  case /*GPGME_Read_Error         = */13:
    return(i18n("Read error."));
  case /*GPGME_Write_Error        = */14:
    return(i18n("Write error."));
  case /*GPGME_Invalid_Type       = */15:
    return(i18n("Invalid type."));
  case /*GPGME_Invalid_Mode       = */16:
    return(i18n("Invalid mode."));
  case /*GPGME_File_Error         = */17:  // errno is set in this case.
    return(i18n("File error."));
  case /*GPGME_Decryption_Failed  = */18:
    return(i18n("Decryption failed."));
  case /*GPGME_No_Passphrase      = */19:
    isPassphraseError = true;
    return(i18n("No passphrase."));
  case /*GPGME_Canceled           = */20:
    isPassphraseError = true;
    return(i18n("Canceled."));
  case /*GPGME_Invalid_Key        = */21:
    isPassphraseError = true; // ### ???
    return(i18n("Invalid key."));
  case /*GPGME_Invalid_Engine     = */22:
    return(i18n("Invalid engine."));
  case /*GPGME_Invalid_Recipients = */23:
    return(i18n("Invalid recipients."));
  default:
    return(i18n("Unknown error."));
  }
#endif
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
  return QString::null;
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


void CryptPlugWrapper::setUpdateURL( const QString& url )
{
    _updateURL = url;
}


QString CryptPlugWrapper::updateURL() const
{
    return _updateURL;
}

bool CryptPlugWrapper::alwaysEncryptToSelf() {
  return true;
}


void CryptPlugWrapper::setAlwaysEncryptToSelf( bool enc ) {
  mAlwaysEncryptToSelf = enc;
}

bool CryptPlugWrapper::initialize( InitStatus* initStatus, QString* errorMsg )
{
    if ( _cp )
      return true;

    _initStatus = InitStatus_undef;
    /* make sure we have a lib name */
    if ( _libName.isEmpty() ) {
      _initStatus = InitStatus_NoLibName;
      kdDebug(5150) << "No library name was given.\n" << endl;
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
	kdDebug(5150) << "Couldn't create '" << _libName.latin1() << "'" << endl;
      } else {
	/* now call the init function */
	if( !_cp->initialize() ) {
	  _initStatus = InitStatus_InitError;
	  kdDebug(5150) << "Error while executing function 'initialize' on plugin " << _libName << endl;
	  _lastError = i18n("Error while initializing plugin \"%1\"").arg( _libName );
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


bool CryptPlugWrapper::signMessage( const char* cleartext,
                                    char** ciphertext,
                                    const size_t* cipherLen,
                                    const char* certificate,
                                    StructuringInfoWrapper& structuring,
                                    int* errId,
                                    char** errTxt )
{
  return _cp && _cp->signMessage( cleartext, ciphertext, cipherLen, certificate,
				  &structuring.data, errId, errTxt,
				  _config->sendCertificates, _config->signatureCompoundMode );
}


bool CryptPlugWrapper::checkMessageSignature( char** cleartext,
                                              const char* signaturetext,
                                              bool signatureIsBinary,
                                              int signatureLen,
                                              CryptPlug::SignatureMetaData* sigmeta )
{
  DNBeautifier dnBeautifier( kapp->config(),
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


bool CryptPlugWrapper::storeCertificatesFromMessage( const char* ciphertext )
{
    return _cp && _cp->storeCertificatesFromMessage( ciphertext );
}


bool CryptPlugWrapper::findCertificates( const char*  addressee,
                                         char** certificates,
                                         int* newSize,
                                         bool secretOnly )
{
  DNBeautifier dnBeautifier( kapp->config(),
                             "DN",
                             "AttributeOrder",
                             "UnknownAttributes" );
  return _cp && _cp->findCertificates( addressee,
                                       certificates,
                                       newSize,
                                       secretOnly,
                                       dnBeautifier.attrOrderChar(),
                                       dnBeautifier.unknownAttrsHandlingChar() );
}

bool CryptPlugWrapper::encryptMessage( const char* cleartext,
                                       const char** ciphertext,
                                       const size_t* cipherLen,
                                       const char* addressee,
                                       StructuringInfoWrapper& structuring,
                                       int* errId,
                                       char** errTxt )
{
  return _cp && _cp->encryptMessage( cleartext, ciphertext, cipherLen, addressee,
				     &structuring.data, errId, errTxt );
}


bool CryptPlugWrapper::encryptAndSignMessage( const char* cleartext,
                                              const char** ciphertext,
                                              const char* certificate,
                                              StructuringInfoWrapper& structuring )
{
  return _cp && _cp->encryptAndSignMessage( cleartext, ciphertext, certificate,
					    &structuring.data );
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
  DNBeautifier dnBeautifier( kapp->config(),
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

CryptPlugWrapper::CertificateInfoList CryptPlugWrapper::listKeys( const QString& pattern,
  bool remote,
  bool* truncated )
{
    CertificateInfoList result;
    if ( truncated ) *truncated = false;
    if ( !_cp )
      return result;

    CryptPlug::CertIterator * it = _cp->startListCertificates( pattern.utf8().data(), remote );
    if ( !it )
      return result;

    DNBeautifier dnBeautifier( kapp->config(),
                               "DN",
                               "AttributeOrder",
                               "UnknownAttributes" );

    while ( true ) {
      CryptPlug::CertificateInfo* info = 0;
      if ( _cp->nextCertificate( it,
                                 &info,
                                 dnBeautifier.attrOrderChar(),
                                 dnBeautifier.unknownAttrsHandlingChar() ) != 0 ) {
        kdDebug(5150) << "error" << endl;
        break;
      }
      if ( !info )
        break;

      CryptPlugWrapper::CertificateInfo cpwinfo;
      for ( char** ptr = info->userid; *ptr; ++ptr )
        cpwinfo.userid << QString::fromUtf8( *ptr );
      cpwinfo.userid_0_org = QString::fromUtf8( info->userid_0_org );

      kdDebug(5150) << "CryptPlugWrapper got " << cpwinfo.userid[0] << endl;
      cpwinfo.serial = QString::fromUtf8( info->serial );
      kdDebug(5150) << "fpr=" << info->fingerprint << endl;
      cpwinfo.fingerprint = QString::fromUtf8( info->fingerprint );
      kdDebug(5150) << "Done getting fpr" << endl;

      cpwinfo.issuer_org   = QString::fromUtf8( info->issuer_org );
      cpwinfo.issuer_reord = QString::fromUtf8( info->issuer_reord );
      cpwinfo.chainid = QString::fromUtf8( info->chainid );
      QString caps = QString::fromUtf8( info->caps );

      cpwinfo.sign = caps.contains('s');
      cpwinfo.encrypt = caps.contains('e');
      cpwinfo.certify = caps.contains('c');

      cpwinfo.created.setTime_t(info->created );
      cpwinfo.expire.setTime_t( info->expire );

      cpwinfo.secret = info->secret;
      cpwinfo.invalid = info->invalid;
      cpwinfo.expired = info->expired;
      cpwinfo.disabled = info->disabled;

      for ( CryptPlug::DnPair * a = info->dnarray ; a && a->key&& a->value ; ++a )
        //kdDebug(5150) << "CryptPlugWrapper::listKeys() " << a->key << " = " << a->value << endl;
        cpwinfo.dn.push_back( qMakePair( QString::fromUtf8( a->key ), QString::fromUtf8( a->value ) ) );

      //cpwinfo.dn = dnBeautifier.reorder( cpwinfo.dn );

      result.push_back( cpwinfo );
    }

    if ( _cp->endListCertificates( it ) != 0 )
      if ( truncated ) *truncated = true;

    return result;
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
    mode |= GpgME::Context::Extern;
    mode &= ~GpgME::Context::Local;
  } else {
    mode |= GpgME::Context::Local;
    mode &= ~GpgME::Context::Extern;
  }
  if ( includeSigs ) mode |= GpgME::Context::Signatures;
  if ( validate ) mode |= GpgME::Context::Validate;
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

Kleo::ExportJob * CryptPlugWrapper::secretKeyExportJob( bool armor ) const {
  if ( !_cp || _cp->mProtocol != GpgME::Context::CMS ) // fixme: add support for gpg, too
    return 0;

  // this operation is not supported by gpgme, so we have to call gpgsm ourselves:
  return new Kleo::QGpgMESecretKeyExportJob( armor );
}

Kleo::RefreshKeysJob * CryptPlugWrapper::refreshKeysJob() const {
  if ( !_cp || _cp->mProtocol != GpgME::Context::CMS ) // fixme: add support for gpg, too
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
  context->setKeyListMode( GpgME::Context::Extern );

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
