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
#include <backends/qgpgme/qgpgmedownloadjob.h>
#include <backends/qgpgme/qgpgmedeletejob.h>
#include <backends/qgpgme/qgpgmesignencryptjob.h>
#include <backends/qgpgme/qgpgmedecryptverifyjob.h>

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
  SignEmail               signEmail;
  bool                    saveSentSignatures;
  bool                    warnNoCertificate;
  PinRequests             numPINRequests;
  bool                    checkSignatureCertificatePathToRoot;
  bool                    signatureUseCRLs;
  EncryptionAlgorithm     encryptionAlgorithm;
  EncryptEmail            encryptEmail;
  bool                    saveMessagesEncrypted;
  bool                    checkEncryptionCertificatePathToRoot;
  bool                    encryptionUseCRLs;
  bool                    encryptionCRLExpiryNearWarning;
  int                     encryptionCRLNearExpiryInterval;
  CertificateSource       certificateSource;
  CertificateSource       cRLSource;
  bool                    warnSendUnsigned;
  int                     numPINRequestsInterval;
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
  signEmail                            = SignEmail_SignAll;
  saveSentSignatures                   = true;
  warnNoCertificate                    = true;
  numPINRequests                       = PinRequest_Always;
  checkSignatureCertificatePathToRoot  = true;
  signatureUseCRLs                     = true;
  encryptionAlgorithm                  = EncryptAlg_RSA;
  encryptEmail                         = EncryptEmail_Ask;
  saveMessagesEncrypted                = true;
  checkEncryptionCertificatePathToRoot = true;
  encryptionUseCRLs                    = true;
  encryptionCRLExpiryNearWarning       = false;
  encryptionCRLNearExpiryInterval      = NEAR_EXPIRY;
  certificateSource                    = CertSrc_Server;
  cRLSource                            = CertSrc_Server;
  warnSendUnsigned                             = true;
  numPINRequestsInterval                       = NEAR_EXPIRY;
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
  config->signatureAlgorithm = sigAlg;
}

SignatureAlgorithm CryptPlugWrapper::signatureAlgorithm()
{
  return config->signatureAlgorithm;
}

void CryptPlugWrapper::setSignatureCompoundMode( SignatureCompoundMode signComp )
{
  config->signatureCompoundMode = signComp;
}

SignatureCompoundMode CryptPlugWrapper::signatureCompoundMode()
{
  return config->signatureCompoundMode;
}

void CryptPlugWrapper::setSendCertificates( SendCertificates sendCert )
{
  config->sendCertificates = sendCert;
}

SendCertificates CryptPlugWrapper::sendCertificates()
{
  return config->sendCertificates;
}

void CryptPlugWrapper::setSignEmail( SignEmail signMail )
{
  config->signEmail = signMail;
}

SignEmail CryptPlugWrapper::signEmail()
{
  return config->signEmail;
}





void CryptPlugWrapper::setWarnSendUnsigned( bool flag )
{
  config->warnSendUnsigned = flag;
}

bool CryptPlugWrapper::warnSendUnsigned()
{
  return config->warnSendUnsigned;
}






void CryptPlugWrapper::setSaveSentSignatures( bool flag )
{
  config->saveSentSignatures = flag;
}

bool CryptPlugWrapper::saveSentSignatures()
{
  return config->saveSentSignatures;
}

void CryptPlugWrapper::setWarnNoCertificate( bool flag )
{
  config->warnNoCertificate = flag;
}

bool CryptPlugWrapper::warnNoCertificate()
{
  return config->warnNoCertificate;
}


void CryptPlugWrapper::setNumPINRequests( PinRequests reqMode )
{
  config->numPINRequests = reqMode;

  /* PENDING(g10) Put this value into gpg and make it ask for the pin
     according to this. Note that there is also
     setNumPINRequestsInterval() which is only used if reqMode ==
     PinRequest_AfterMinutes.
  */
}

PinRequests CryptPlugWrapper::numPINRequests()
{
  return config->numPINRequests;
}



void CryptPlugWrapper::setNumPINRequestsInterval( int interval )
{
  config->numPINRequestsInterval = interval;

  /* PENDING(g10) Put this value into gpg and make it ask for the pin
     according to this. Note that this should only be used if
     config->numPINRequests (set with setNumPINRequests()) has the
     value PinRequest_AfterMinutes.
  */
}

int CryptPlugWrapper::numPINRequestsInterval()
{
  return config->numPINRequestsInterval;
}



void CryptPlugWrapper::setCheckSignatureCertificatePathToRoot( bool flag )
{
  config->checkSignatureCertificatePathToRoot = flag;
}

bool CryptPlugWrapper::checkSignatureCertificatePathToRoot()
{
  return config->checkSignatureCertificatePathToRoot;
}

void CryptPlugWrapper::setSignatureUseCRLs( bool flag )
{
  config->signatureUseCRLs = flag;
}

bool CryptPlugWrapper::signatureUseCRLs()
{
  return config->signatureUseCRLs;
}






void CryptPlugWrapper::setSignatureCertificateExpiryNearWarning( bool flag )
{
  config->signatureCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::signatureCertificateExpiryNearWarning( void )
{
  return config->signatureCertificateExpiryNearWarning;
}

void CryptPlugWrapper::setSignatureCertificateExpiryNearInterval( int interval )
{
  config->signatureCertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::signatureCertificateExpiryNearInterval( void )
{
  return config->signatureCertificateExpiryNearInterval;
}

void CryptPlugWrapper::setCACertificateExpiryNearWarning( bool flag )
{
  config->cACertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::caCertificateExpiryNearWarning( void )
{
  return config->cACertificateExpiryNearWarning;
}

void CryptPlugWrapper::setCACertificateExpiryNearInterval( int interval )
{
  config->cACertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::caCertificateExpiryNearInterval( void )
{
  return config->cACertificateExpiryNearInterval;
}

void CryptPlugWrapper::setRootCertificateExpiryNearWarning( bool flag )
{
  config->rootCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::rootCertificateExpiryNearWarning( void )
{
  return config->rootCertificateExpiryNearWarning;
}

void CryptPlugWrapper::setRootCertificateExpiryNearInterval( int interval )
{
  config->rootCertificateExpiryNearInterval = interval;
}

int CryptPlugWrapper::rootCertificateExpiryNearInterval( void )
{
  return config->rootCertificateExpiryNearInterval;
}





void CryptPlugWrapper::setEncryptionAlgorithm( EncryptionAlgorithm cryptAlg )
{
  config->encryptionAlgorithm = cryptAlg;
}

EncryptionAlgorithm CryptPlugWrapper::encryptionAlgorithm()
{
  return config->encryptionAlgorithm;
}

void CryptPlugWrapper::setEncryptEmail( EncryptEmail cryptMode )
{
  config->encryptEmail = cryptMode;
}

EncryptEmail CryptPlugWrapper::encryptEmail()
{
  return config->encryptEmail;
}






void CryptPlugWrapper::setWarnSendUnencrypted( bool flag )
{
  config->warnSendUnencrypted = flag;
}

bool CryptPlugWrapper::warnSendUnencrypted()
{
  return config->warnSendUnencrypted;
}









void CryptPlugWrapper::setSaveMessagesEncrypted( bool flag )
{
  config->saveMessagesEncrypted = flag;
}

bool CryptPlugWrapper::saveMessagesEncrypted()
{
  return config->saveMessagesEncrypted;
}







void CryptPlugWrapper::setCheckCertificatePath( bool flag )
{
  config->checkCertificatePath = flag;
}

bool CryptPlugWrapper::checkCertificatePath()
{
  return config->checkCertificatePath;
}








void CryptPlugWrapper::setCheckEncryptionCertificatePathToRoot( bool flag )
{
  config->checkEncryptionCertificatePathToRoot = flag;
}

bool CryptPlugWrapper::checkEncryptionCertificatePathToRoot()
{
  return config->checkEncryptionCertificatePathToRoot;
}







void CryptPlugWrapper::setReceiverCertificateExpiryNearWarning( bool flag )
{
  config->receiverCertificateExpiryNearWarning = flag;
}

bool CryptPlugWrapper::receiverCertificateExpiryNearWarning()
{
  return config->receiverCertificateExpiryNearWarning;
}


void CryptPlugWrapper::setReceiverCertificateExpiryNearWarningInterval( int interval )
{
  config->receiverCertificateExpiryNearWarningInterval = interval;
}

int CryptPlugWrapper::receiverCertificateExpiryNearWarningInterval()
{
  return config->receiverCertificateExpiryNearWarningInterval;
}

void CryptPlugWrapper::setCertificateInChainExpiryNearWarning( bool flag )
{
  config->certificateInChainExpiryNearWarning = flag;
}

bool CryptPlugWrapper::certificateInChainExpiryNearWarning()
{
  return config->certificateInChainExpiryNearWarning;
}


void CryptPlugWrapper::setCertificateInChainExpiryNearWarningInterval( int interval )
{
  config->certificateInChainExpiryNearWarningInterval = interval;
}

int CryptPlugWrapper::certificateInChainExpiryNearWarningInterval()
{
  return config->certificateInChainExpiryNearWarningInterval;
}

void CryptPlugWrapper::setReceiverEmailAddressNotInCertificateWarning( bool flag )
{
  config->receiverEmailAddressNotInCertificateWarning = flag;
}

bool CryptPlugWrapper::receiverEmailAddressNotInCertificateWarning()
{
  return config->receiverEmailAddressNotInCertificateWarning;
}








void CryptPlugWrapper::setEncryptionUseCRLs( bool flag )
{
  config->encryptionUseCRLs = flag;

  /* PENDING(g10) Store this setting in gpgme and use it. If true,
     every certificate used for encryption should be checked against
     applicable CRLs.
  */
}

bool CryptPlugWrapper::encryptionUseCRLs()
{
  return config->encryptionUseCRLs;
}


int CryptPlugWrapper::encryptionCRLsDaysLeftToExpiry()
{
  return CRYPTPLUG_CERT_DOES_NEVER_EXPIRE;
}

void CryptPlugWrapper::setEncryptionCRLExpiryNearWarning( bool flag )
{
  config->encryptionCRLExpiryNearWarning = flag;
}

bool CryptPlugWrapper::encryptionCRLExpiryNearWarning()
{
  return config->encryptionCRLExpiryNearWarning;
}

void CryptPlugWrapper::setEncryptionCRLNearExpiryInterval( int interval )
{
  config->encryptionCRLNearExpiryInterval = interval;
}

int CryptPlugWrapper::encryptionCRLNearExpiryInterval()
{
  return config->encryptionCRLNearExpiryInterval;
}


void CryptPlugWrapper::setCertificateSource( CertificateSource source )
{
  config->certificateSource = source;
}

CertificateSource CryptPlugWrapper::certificateSource()
{
  return config->certificateSource;
}

void CryptPlugWrapper::setCRLSource( CertificateSource source )
{
  config->cRLSource = source;
}

CertificateSource CryptPlugWrapper::crlSource()
{
  return config->cRLSource;
}




QString CryptPlugWrapper::libVersion() const {
  return config && config->libVersion ? QString::fromUtf8( config->libVersion ) : QString::null ;
}

/* Some multi purpose functions ******************************************/

QString CryptPlugWrapper::errorIdToText( int errId, bool & isPassphraseError ) {
  /* The error numbers used by GPGME.  */
  /*
    typedef enum
    {
      GPGME_EOF                = -1,
      GPGME_No_Error           = 0,
      GPGME_General_Error      = 1,
      GPGME_Out_Of_Core        = 2,
      GPGME_Invalid_Value      = 3,
      GPGME_Busy               = 4,
      GPGME_No_Request         = 5,
      GPGME_Exec_Error         = 6,
      GPGME_Too_Many_Procs     = 7,
      GPGME_Pipe_Error         = 8,
      GPGME_No_Recipients      = 9,
      GPGME_No_Data            = 10,
      GPGME_Conflict           = 11,
      GPGME_Not_Implemented    = 12,
      GPGME_Read_Error         = 13,
      GPGME_Write_Error        = 14,
      GPGME_Invalid_Type       = 15,
      GPGME_Invalid_Mode       = 16,
      GPGME_File_Error         = 17,  // errno is set in this case.
      GPGME_Decryption_Failed  = 18,
      GPGME_No_Passphrase      = 19,
      GPGME_Canceled           = 20,
      GPGME_Invalid_Key        = 21,
      GPGME_Invalid_Engine     = 22,
      GPGME_Invalid_Recipients = 23
    }
  */

  /*
    NOTE:
    The following hack *must* be changed into something
    using an extra enum specified in the CryptPlug API
    *and* the file error number (case 17) must be taken
    into account.                     (khz, 2002/27/06)
  */

  isPassphraseError = false;
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
}

/* some special functions ************************************************/


CryptPlugWrapper::CryptPlugWrapper( const QString& name,
                                    const QString& libName,
                                    const QString& update,
                                    bool           active )
  : Kleo::CryptoBackend(),
    _name( name ),
    _libName( libName ),
    _updateURL( update ),
    _active(  active  ),
    _initStatus( InitStatus_undef ),
    _cp( 0 ),
    config( 0 )
{
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
    return "smime";
  if ( _libName.contains( "openpgp" ) )
    return "openpgp";
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
      return "S/MIME";
    if ( _libName.contains( "openpgp" ) )
      return "OpenPGP";
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
  return mAlwaysEncryptToSelf;
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
	config = new Config( GPGME_PROTOCOL_CMS );
      } else if ( _libName.contains( "openpgp" ) ) {
	_cp = new OpenPGPCryptPlug();
	config = new Config( GPGME_PROTOCOL_OpenPGP );
      } else {
	_cp = 0;
	config = 0;
      }

      if ( !_cp ) {
	_initStatus = InitStatus_LoadError;
	kdDebug(5150) << "Couldn't create '" << _libName.latin1() << "'" << endl;
      } else {
	/* now call the init function */
	if( !_cp->initialize() ) {
	  _initStatus = InitStatus_InitError;
	  kdDebug(5150) << "Error while executing function 'initialize'.\n" << endl;
	  _lastError = i18n("Error while initializing plugin \"%1\"").arg( _libName );
	  if ( errorMsg )
	    *errorMsg = _lastError;
	  delete _cp; _cp = 0;
	  delete config; config = 0;
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
    delete config; config = 0;
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



int CryptPlugWrapper::certificateInChainDaysLeftToExpiry( const char* certificate )
{
    return _cp ? _cp->certificateInChainDaysLeftToExpiry( certificate ) : 0 ;
}


int CryptPlugWrapper::signatureCertificateDaysLeftToExpiry( const char* certificate )
{
    return _cp ? _cp->signatureCertificateDaysLeftToExpiry( certificate ) : 0 ;
}

int CryptPlugWrapper::rootCertificateDaysLeftToExpiry( const char* certificate )
{
    return _cp ? _cp->rootCertificateDaysLeftToExpiry( certificate ) : 0 ;
}

int CryptPlugWrapper::caCertificateDaysLeftToExpiry( const char* certificate )
{
    return _cp ? _cp->caCertificateDaysLeftToExpiry( certificate ) : 0 ;
}

int CryptPlugWrapper::receiverCertificateDaysLeftToExpiry( const char* certificate )
{
    return _cp ? _cp->receiverCertificateDaysLeftToExpiry( certificate ) : 0 ;
}

bool CryptPlugWrapper::isEmailInCertificate( const char* email, const char* certificate )
{
  return _cp && _cp->isEmailInCertificate( email, certificate );
}

bool CryptPlugWrapper::certificateValidity( const char* certificate,
                          int* level )
{
  return _cp && _cp->certificateValidity( certificate, level );
}


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
				  config->sendCertificates, config->signatureCompoundMode );
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

Kleo::KeyListJob * CryptPlugWrapper::keyListJob( bool remote, bool includeSigs ) const {
  kdWarning( 5150, includeSigs ) << "CryptPlugWrapper::keyListJob(): includeSigs not yet implemented!" << endl;

  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setKeyListMode( remote ? GpgME::Context::Extern : GpgME::Context::Local );
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

Kleo::SignJob * CryptPlugWrapper::signJob( int includedCerts, bool armor, bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setIncludeCertificates( includedCerts );
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

Kleo::ExportJob * CryptPlugWrapper::exportJob( bool armor ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setArmor( armor );
  return new Kleo::QGpgMEExportJob( context );
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

Kleo::SignEncryptJob * CryptPlugWrapper::signEncryptJob( int includedCerts, bool armor, bool textMode ) const {
  if ( !_cp )
    return 0;

  GpgME::Context * context = GpgME::Context::createForProtocol( _cp->mProtocol );
  if ( !context )
    return 0;

  context->setIncludeCertificates( includedCerts );
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
