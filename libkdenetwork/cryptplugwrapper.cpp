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


#include "cryptplugwrapper.h"

#include <kdebug.h>
#include <klocale.h>

#include <dlfcn.h>

#include <stdlib.h>
#include <stdio.h>

#include "cryptplugwrapper.moc"




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
    if( _wrapper && _wrapper->libPtr() ) {
        void (*p_func)(  struct StructuringInfo* )
            = (void (*)( struct StructuringInfo* ))
                  dlsym(_wrapper->libPtr(), "init_StructuringInfo");
        if ( ! _wrapper->wasDLError( "init_StructuringInfo" ) ) {
            (*p_func)( &data );
            _initDone = true;
        }
   }
}
void StructuringInfoWrapper::freeMe()
{
    if( _wrapper && _wrapper->libPtr() && _initDone ) {
        void (*p_func)(  struct StructuringInfo* )
            = (void (*)( struct StructuringInfo* ))
                  dlsym(_wrapper->libPtr(), "free_StructuringInfo");
        if ( ! _wrapper->wasDLError( "free_StructuringInfo" ) ) {
            (*p_func)( &data );
            _initDone = false;
        }
    }
}






/* some multi purpose functions ******************************************/

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
    return(i18n("Out of core!"));
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

bool CryptPlugWrapper::wasDLError( const char* funcName )
{
    QString thisError = dlerror();
    bool bError = ! thisError.isEmpty();
    if ( bError ) {
        _lastError = thisError;
kdDebug() << "Couldn't find function '%s': %s\n" << funcName << thisError << endl;
    }
    return bError;
}


void CryptPlugWrapper::voidVoidFunc( const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)() = (void (*)()) dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            (*p_func)();
    }
}


bool CryptPlugWrapper::boolConstCharFunc( const char* txt, const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char* )
            = (bool (*)(const char* ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            return (*p_func)( txt );
    }
    return false;
}


bool CryptPlugWrapper::boolConstCharConstCharFunc( const char* txt1,
                                                   const char* txt2,
                                                   const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char*, const char* )
            = (bool (*)(const char*, const char* ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            return (*p_func)( txt1, txt2 );
    }
    return false;
}


int CryptPlugWrapper::intConstCharFunc( const char* txt, const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        int (*p_func)( const char* )
            = (int (*)(const char* ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            return (*p_func)( txt );
    }
    return 0;
}


void CryptPlugWrapper::voidConstCharFunc( const char* txt, const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)( const char* )
            = (void (*)(const char* ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            (*p_func)( txt );
    }
}


const char* CryptPlugWrapper::constCharVoidFunc( const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        const char* (*p_func)(void) = (const char* (*)(void))
                                      dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            return (*p_func)();
    }
    return 0;
}


void CryptPlugWrapper::voidBoolFunc( bool flag, const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)( bool )
            = (void (*)(bool ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            (*p_func)( flag );
    }
}


bool CryptPlugWrapper::boolVoidFunc( const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( void )
            = (bool (*)(void ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            return (*p_func)();
    }
    return false;
}


void CryptPlugWrapper::voidIntFunc( int value, const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)( int )
            = (void (*)(int ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            (*p_func)( value );
    }
}


int CryptPlugWrapper::intVoidFunc( const char* funcName )
{
    int res = 0;
    if ( /*_active &&*/ _initialized ) {
        int (*p_func)( void )
            = (int (*)(void ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            res = (*p_func)();
    }
    return res;
}


void CryptPlugWrapper::stationeryFunc( void** pixmap,
                                       const char** menutext,
                                       char* accel,
                                       const char** tooltip,
                                       const char** statusbartext,
                                       const char* funcName )
{
    if ( /*_active &&*/ _initialized ) {
        void  (*p_func)( void**, const char**, char*, const char**, const char** )
            = (void (*)( void**, const char**, char*, const char**, const char** ))
                dlsym(_libPtr, funcName);
        if ( ! wasDLError( funcName ) )
            (*p_func)( pixmap, menutext, accel, tooltip, statusbartext );
    }
}





/* some special functions ************************************************/


CryptPlugWrapper::CryptPlugWrapper( QWidget*       parent,
                                    const QString& name,
                                    const QString& libName,
                                    const QString& update,
                                    bool           active ):
    _parent(  parent  ),
    _name( name ),
    _libName( libName ),
    _updateURL( update ),
    _libPtr(  0 ),
    _active(  active  ),
    _initialized( false ),
    _initStatus( InitStatus_undef )
{
    // this space left empty intentionally
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
    bool bOk = ! _initialized;  // Changing the lib name is only allowed
    if( bOk )                   // when either no initialization took
        _libName = libName;     // place or 'deinitialize()' has been
    return bOk;                 // called afterwards.
}

QString CryptPlugWrapper::libName() const
{
    return _libName;
}


void CryptPlugWrapper::setDisplayName( const QString& name )
{
    _name = name;
}


QString CryptPlugWrapper::displayName() const
{
    return _name;
}


QString CryptPlugWrapper::libVersion() const
{
    if ( _initialized || (InitStatus_InitError == _initStatus) ) {
        const char* (*p_func)(void) = (const char* (*)(void))
                                      dlsym(_libPtr, "libVersion");
        QString thisError = dlerror();
        if ( thisError.isEmpty() ) {
            QString s;
            const char* cp = (*p_func)();
            if( cp )
                return QString::fromUtf8( cp );
        }
    }
    return QString("?");
}


void CryptPlugWrapper::setUpdateURL( const QString& url )
{
    _updateURL = url;
}

    
QString CryptPlugWrapper::updateURL() const
{
    return _updateURL;
}


bool CryptPlugWrapper::initialize( InitStatus* initStatus, QString* errorMsg )
{
    bool bOk = false;
    if ( ! _initialized )
    {
        _initStatus = InitStatus_undef;
        /* make sure we have a lib name */
        if ( _libName.isEmpty() ) {
            _initStatus = InitStatus_NoLibName;
kdDebug() << "Couldn't open plug-in library: No library name was given.\n" << endl;
        }
        else
        {
            QString thisError;

            /* try to load the library */
            _libPtr   = dlopen(_libName.latin1(), RTLD_LAZY);
            thisError = dlerror();
            if ( !_libPtr ) {
                _initStatus = InitStatus_LoadError;
kdDebug() << "Couldn't open '%s': %s\n" << _libName.latin1() << thisError << endl;
            } else {
                /* get the symbol */
                bool (*p_initialize)(void) = (bool (*)(void))
                                             dlsym(_libPtr, "initialize");
                thisError = dlerror();
                if ( ! thisError.isEmpty() ) {
                    _initStatus = InitStatus_LoadError;
kdDebug() << "Couldn't find function 'initialize': %s\n" << thisError << endl;
                } else {
                    /* now call the function in the library */
                    if( ! (*p_initialize)() )
                    {
                        _initStatus = InitStatus_InitError;
kdDebug() << "Error while executing function 'initialize'.\n" << endl;
                    } else {
                        bOk = true;
                        _initStatus  = InitStatus_Ok;
                    }
                }
            }
            if( errorMsg )
                *errorMsg = thisError;
            if( ! thisError.isEmpty() )
                _lastError = thisError;
        }
        _initialized = _initStatus == InitStatus_Ok;
    }
    if( initStatus )
        *initStatus = _initStatus;
    return bOk;
}



void CryptPlugWrapper::deinitialize()
{
    if ( _initialized ) {
        bool (*p_deinitialize)(void) = (bool (*)(void))
                                       dlsym(_libPtr, "deinitialize");
        if ( ! wasDLError( "deinitialize" ) )
            (*p_deinitialize)();
        _initialized = false;
    }
}


CryptPlugWrapper::InitStatus CryptPlugWrapper::initStatus( QString* errorMsg ) const
{
    if( errorMsg )
        *errorMsg = _lastError;
    return _initStatus;
}


bool CryptPlugWrapper::hasFeature( Feature flag )
{
    bool bHasIt = false;
    if ( _initialized ) {
        bool (*p_func)(Feature) = (bool (*)(Feature))
                                  dlsym(_libPtr, "hasFeature");
        if ( ! wasDLError( "hasFeature" ) )
            bHasIt = (*p_func)( flag );
    }
    return bHasIt;
}


const char* CryptPlugWrapper::bugURL()
{
    return constCharVoidFunc( "bugURL" );
}



/* normal functions ******************************************************/



void CryptPlugWrapper::unsafeStationery( void** pixmap,
                                         const char** menutext,
                                         char* accel,
                                         const char** tooltip,
                                         const char** statusbartext )
{
    stationeryFunc( pixmap, menutext, accel, tooltip, statusbartext,
                    "unsafeStationery" );
}


void CryptPlugWrapper::signedStationery( void** pixmap,
                                         const char** menutext,
                                         char* accel,
                                         const char** tooltip,
                                         const char** statusbartext )
{
    stationeryFunc( pixmap, menutext, accel, tooltip, statusbartext,
                    "signedStationery" );
}


void CryptPlugWrapper::encryptedStationery( void** pixmap,
                                            const char** menutext,
                                            char* accel,
                                            const char** tooltip,
                                            const char** statusbartext )
{
    stationeryFunc( pixmap, menutext, accel, tooltip, statusbartext,
                    "encryptedStationery" );
}


void CryptPlugWrapper::signedEncryptedStationery(void** pixmap,
                                                 const char** menutext,
                                                 char* accel,
                                                 const char** tooltip,
                                                 const char** statusbartext)
{
    stationeryFunc( pixmap, menutext, accel, tooltip, statusbartext,
                    "signedEncryptedStationery" );
}


const char* CryptPlugWrapper::signatureConfigurationDialog()
{
    return constCharVoidFunc( "signatureConfigurationDialog" );
}


const char* CryptPlugWrapper::signatureKeySelectionDialog()
{
    return constCharVoidFunc( "signatureKeySelectionDialog" );
}


const char* CryptPlugWrapper::signatureAlgorithmDialog()
{
    return constCharVoidFunc( "signatureAlgorithmDialog" );
}


const char* CryptPlugWrapper::signatureHandlingDialog()
{
    return constCharVoidFunc( "signatureHandlingDialog" );
}


void CryptPlugWrapper::setSignatureKeyCertificate( const char* certificate )
{
    voidConstCharFunc( certificate, "setSignatureKeyCertificate" );
}


const char* CryptPlugWrapper::signatureKeyCertificate()
{
    return constCharVoidFunc( "signatureKeyCertificate" );
}


void CryptPlugWrapper::setSignatureAlgorithm( SignatureAlgorithm sigAlg )
{
    voidIntFunc( (int)sigAlg, "setSignatureAlgorithm" );
}


SignatureAlgorithm CryptPlugWrapper::signatureAlgorithm()
{
    return (SignatureAlgorithm) intVoidFunc( "signatureAlgorithm" );
}


void CryptPlugWrapper::setWarnSendUnsigned( bool flag )
{
    voidBoolFunc( flag, "setWarnSendUnsigned" );
}


bool CryptPlugWrapper::warnSendUnsigned()
{
    return boolVoidFunc( "warnSendUnsigned" );
}

                          
void CryptPlugWrapper::setSignatureCompoundMode( SignatureCompoundMode signComp )
{
    voidIntFunc( (int)signComp, "setSignatureCompoundMode" );
}


SignatureCompoundMode CryptPlugWrapper::signatureCompoundMode()
{
    return (SignatureCompoundMode) intVoidFunc( "signatureCompoundMode" );
}


void CryptPlugWrapper::setSendCertificates( SendCertificates sendCert )
{
    voidIntFunc( (int)sendCert, "setSendCertificates" );
}


SendCertificates CryptPlugWrapper::sendCertificates()
{
    return (SendCertificates) intVoidFunc( "sendCertificates" );
}


void CryptPlugWrapper::setSignEmail( SignEmail signMail )
{
    voidIntFunc( (int)signMail, "setSignEmail" );
}


SignEmail CryptPlugWrapper::signEmail()
{
    return (SignEmail) intVoidFunc( "signEmail" );
}


void CryptPlugWrapper::setSaveSentSignatures( bool flag )
{
    voidBoolFunc( flag, "setSaveSentSignatures" );
}


bool CryptPlugWrapper::saveSentSignatures()
{
    return boolVoidFunc( "saveSentSignatures" );
}


void CryptPlugWrapper::setWarnNoCertificate( bool flag )
{
    voidBoolFunc( flag, "setWarnNoCertificate" );
}


bool CryptPlugWrapper::warnNoCertificate()
{
    return boolVoidFunc( "warnNoCertificate" );
}


bool CryptPlugWrapper::isEmailInCertificate( const char* email,
                                             const char* certificate )
{
    return boolConstCharConstCharFunc( email, certificate, "isEmailInCertificate" );
}



void CryptPlugWrapper::setNumPINRequests( PinRequests reqMode )
{
    voidIntFunc( (int)reqMode, "setNumPINRequests" );
}


PinRequests CryptPlugWrapper::numPINRequests()
{
    return (PinRequests) intVoidFunc( "numPINRequests" );
}


void CryptPlugWrapper::setNumPINRequestsInterval( int interval )
{
    voidIntFunc( interval, "setNumPINRequestsInterval" );
}


int CryptPlugWrapper::numPINRequestsInterval()
{
    return intVoidFunc( "numPINRequestsInterval" );
}


void CryptPlugWrapper::setCheckSignatureCertificatePathToRoot( bool flag )
{
    voidBoolFunc( flag, "setCheckSignatureCertificatePathToRoot" );
}


bool CryptPlugWrapper::checkSignatureCertificatePathToRoot()
{
    return boolVoidFunc( "checkSignatureCertificatePathToRoot" );
}


void CryptPlugWrapper::setSignatureUseCRLs( bool flag )
{
    voidBoolFunc( flag, "setSignatureUseCRLs" );
}



bool CryptPlugWrapper::signatureUseCRLs()
{
    return boolVoidFunc( "signatureUseCRLs" );
}


void CryptPlugWrapper::setSignatureCertificateExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setSignatureCertificateExpiryNearWarning" );
}


bool CryptPlugWrapper::signatureCertificateExpiryNearWarning()
{
    return boolVoidFunc( "signatureCertificateExpiryNearWarning" );
}


int CryptPlugWrapper::signatureCertificateDaysLeftToExpiry( const char* certificate )
{
    return intConstCharFunc( certificate,
                             "signatureCertificateDaysLeftToExpiry" );
}


void CryptPlugWrapper::setSignatureCertificateExpiryNearInterval( int interval )
{
    voidIntFunc( interval, "setSignatureCertificateExpiryNearInterval" );
}


int CryptPlugWrapper::signatureCertificateExpiryNearInterval()
{
    return intVoidFunc( "signatureCertificateExpiryNearInterval" );
}


void CryptPlugWrapper::setCACertificateExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setCACertificateExpiryNearWarning" );
}


bool CryptPlugWrapper::caCertificateExpiryNearWarning()
{
    return boolVoidFunc( "caCertificateExpiryNearWarning" );
}


int CryptPlugWrapper::caCertificateDaysLeftToExpiry( const char* certificate )
{
    return intConstCharFunc( certificate,
                             "caCertificateDaysLeftToExpiry" );
}


void CryptPlugWrapper::setCACertificateExpiryNearInterval( int interval )
{
    voidIntFunc( interval, "setCACertificateExpiryNearInterval" );
}


int CryptPlugWrapper::caCertificateExpiryNearInterval()
{
    return intVoidFunc( "caCertificateExpiryNearInterval" );
}


void CryptPlugWrapper::setRootCertificateExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setRootCertificateExpiryNearWarning" );
}


bool CryptPlugWrapper::rootCertificateExpiryNearWarning()
{
    return boolVoidFunc( "rootCertificateExpiryNearWarning" );
}


int CryptPlugWrapper::rootCertificateDaysLeftToExpiry( const char* certificate )
{
    return intConstCharFunc( certificate,
                             "rootCertificateDaysLeftToExpiry" );
}


void CryptPlugWrapper::setRootCertificateExpiryNearInterval( int interval )
{
    voidIntFunc( interval, "setRootCertificateExpiryNearInterval" );
}


int CryptPlugWrapper::rootCertificateExpiryNearInterval()
{
    return intVoidFunc( "rootCertificateExpiryNearInterval" );
}


const char* CryptPlugWrapper::encryptionConfigurationDialog()
{
    return constCharVoidFunc( "encryptionConfigurationDialog" );
}


const char* CryptPlugWrapper::encryptionAlgorithmDialog()
{
    return constCharVoidFunc( "encryptionAlgorithmDialog" );
}


const char* CryptPlugWrapper::encryptionHandlingDialog()
{
    return constCharVoidFunc( "encryptionHandlingDialog" );
}


const char* CryptPlugWrapper::encryptionReceiverDialog()
{
    return constCharVoidFunc( "encryptionReceiverDialog" );
}


void CryptPlugWrapper::setEncryptionAlgorithm( EncryptionAlgorithm cryptAlg )
{
    voidIntFunc( (int)cryptAlg, "setEncryptionAlgorithm" );
}



EncryptionAlgorithm CryptPlugWrapper::encryptionAlgorithm()
{
    return (EncryptionAlgorithm) intVoidFunc( "encryptionAlgorithm" );
}


void CryptPlugWrapper::setEncryptEmail( EncryptEmail cryptMode )
{
    voidIntFunc( (int)cryptMode, "setEncryptEmail" );
}



EncryptEmail CryptPlugWrapper::encryptEmail()
{
    return (EncryptEmail) intVoidFunc( "encryptEmail" );
}


void CryptPlugWrapper::setWarnSendUnencrypted( bool flag )
{
    voidBoolFunc( flag, "setWarnSendUnencrypted" );
}


bool CryptPlugWrapper::warnSendUnencrypted()
{
    return boolVoidFunc( "warnSendUnencrypted" );
}


void CryptPlugWrapper::setAlwaysEncryptToSelf( bool flag )
{
    mAlwaysEncryptToSelf = flag;
}


bool CryptPlugWrapper::alwaysEncryptToSelf()
{
    return mAlwaysEncryptToSelf;
}


void CryptPlugWrapper::setSaveMessagesEncrypted( bool flag )
{
    voidBoolFunc( flag, "setSaveMessagesEncrypted" );
}


bool CryptPlugWrapper::saveMessagesEncrypted()
{
    return boolVoidFunc( "saveMessagesEncrypted" );
}


void CryptPlugWrapper::setCheckCertificatePath( bool flag )
{
    voidBoolFunc( flag, "setCheckCertificatePath" );
}


bool CryptPlugWrapper::checkCertificatePath()
{
    return boolVoidFunc( "checkCertificatePath" );
}


void CryptPlugWrapper::setCheckEncryptionCertificatePathToRoot( bool flag )
{
    voidBoolFunc( flag, "setCheckEncryptionCertificatePathToRoot" );
}



bool CryptPlugWrapper::checkEncryptionCertificatePathToRoot()
{
    return boolVoidFunc( "checkEncryptionCertificatePathToRoot" );
}


void CryptPlugWrapper::setReceiverCertificateExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setReceiverCertificateExpiryNearWarning" );
}


bool CryptPlugWrapper::receiverCertificateExpiryNearWarning()
{
    return boolVoidFunc( "receiverCertificateExpiryNearWarning" );
}


int CryptPlugWrapper::receiverCertificateDaysLeftToExpiry( const char* certificate )
{
    return intConstCharFunc( certificate, "receiverCertificateDaysLeftToExpiry" );
}


void CryptPlugWrapper::setReceiverCertificateExpiryNearWarningInterval( int interval )
{
    voidIntFunc( interval, "setReceiverCertificateExpiryNearWarningInterval" );
}


int CryptPlugWrapper::receiverCertificateExpiryNearWarningInterval()
{
    return intVoidFunc( "receiverCertificateExpiryNearWarningInterval" );
}


void CryptPlugWrapper::setCertificateInChainExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setCertificateInChainExpiryNearWarning" );
}


bool CryptPlugWrapper::certificateInChainExpiryNearWarning()
{
    return boolVoidFunc( "certificateInChainExpiryNearWarning" );
}


int CryptPlugWrapper::certificateInChainDaysLeftToExpiry( const char* certificate )
{
    return intConstCharFunc( certificate, "certificateInChainDaysLeftToExpiry" );
}


void CryptPlugWrapper::setCertificateInChainExpiryNearWarningInterval( int interval )
{
    voidIntFunc( interval, "setCertificateInChainExpiryNearWarningInterval" );
}

int CryptPlugWrapper::certificateInChainExpiryNearWarningInterval()
{
    return intVoidFunc( "certificateInChainExpiryNearWarningInterval" );
}


void CryptPlugWrapper::setReceiverEmailAddressNotInCertificateWarning( bool flag )
{
    voidBoolFunc( flag, "setReceiverEmailAddressNotInCertificateWarning" );
}

bool CryptPlugWrapper::receiverEmailAddressNotInCertificateWarning()
{
    return boolVoidFunc( "receiverEmailAddressNotInCertificateWarning" );
}


void CryptPlugWrapper::setEncryptionUseCRLs( bool flag )
{
    voidBoolFunc( flag, "setEncryptionUseCRLs" );
}


bool CryptPlugWrapper::encryptionUseCRLs()
{
    return boolVoidFunc( "encryptionUseCRLs" );
}


void CryptPlugWrapper::setEncryptionCRLExpiryNearWarning( bool flag )
{
    voidBoolFunc( flag, "setEncryptionCRLExpiryNearWarning" );
}



bool CryptPlugWrapper::encryptionCRLExpiryNearWarning()
{
    return boolVoidFunc( "encryptionCRLExpiryNearWarning" );
}


void CryptPlugWrapper::setEncryptionCRLNearExpiryInterval( int interval )
{
    voidIntFunc( interval, "setEncryptionCRLNearExpiryInterval" );
}



int CryptPlugWrapper::encryptionCRLNearExpiryInterval()
{
    return intVoidFunc( "encryptionCRLNearExpiryInterval" );
}


int CryptPlugWrapper::encryptionCRLsDaysLeftToExpiry()
{
    return intVoidFunc( "encryptionCRLsDaysLeftToExpiry" );
}



const char* CryptPlugWrapper::directoryServiceConfigurationDialog()
{
    return constCharVoidFunc( "directoryServiceConfigurationDialog" );
}


void CryptPlugWrapper::appendDirectoryServer( const char* servername, int port,
                                              const char* description )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)( const char*, int, const char* )
            = (void (*)(const char*, int, const char* ))
                dlsym(_libPtr, "appendDirectoryServer");
        if ( ! wasDLError( "appendDirectoryServer" ) )
            (*p_func)( servername, port, description );
    }
}



void CryptPlugWrapper::setDirectoryServers( struct DirectoryServer servers[],
                                            unsigned int size )
{
    if ( /*_active &&*/ _initialized ) {
        void (*p_func)( struct DirectoryServer[], unsigned int )
            = (void (*)(struct DirectoryServer[], unsigned int ))
                dlsym(_libPtr, "setDirectoryServers");
        if ( ! wasDLError( "setDirectoryServers" ) )
            (*p_func)( servers, size );
    }
}



CryptPlugWrapper::DirectoryServer * CryptPlugWrapper::directoryServers( int* numServers )
{
    CryptPlugWrapper::DirectoryServer * res = 0;
    if ( /*_active &&*/ _initialized ) {
        CryptPlugWrapper::DirectoryServer * (*p_func)( int* )
            = (CryptPlugWrapper::DirectoryServer * (*)(int* ))
                dlsym(_libPtr, "directoryServers");
        if ( ! wasDLError( "directoryServers" ) )
            res = (*p_func)( numServers );
    }
    return res;
}


void CryptPlugWrapper::setCertificateSource( CertificateSource source )
{
    voidIntFunc( (int)source, "setCertificateSource" );
}


CertificateSource CryptPlugWrapper::certificateSource()
{
    return (CertificateSource) intVoidFunc( "certificateSource" );
}


void CryptPlugWrapper::setCRLSource( CertificateSource source )
{
    voidIntFunc( (int)source, "setCRLSource" );
}


CertificateSource CryptPlugWrapper::crlSource()
{
    return (CertificateSource) intVoidFunc( "crlSource" );
}



bool CryptPlugWrapper::certificateValidity( const char* certificate,
                          int* level )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char*, int* )
            = (bool (*)(const char*, int* ))
                dlsym(_libPtr, "certificateValidity");
        if ( ! wasDLError( "certificateValidity" ) )
            res = (*p_func)( certificate, level );
    }
    return res;
}


bool CryptPlugWrapper::signMessage( const char* cleartext,
                                    char** ciphertext,
                                    const size_t* cipherLen,
                                    const char* certificate,
                                    StructuringInfoWrapper& structuring,
                                    int* errId,
                                    char** errTxt )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        // first (re-)initialize the StructuringInfo
        structuring.reset();
        // then call the signing function
        bool (*p_func)( const char*, char**, const size_t*, const char*, struct StructuringInfo*, int*, char** )
            = (bool (*)(const char*, char**, const size_t*, const char*, struct StructuringInfo*, int*, char** ))
                dlsym(_libPtr, "signMessage");
        if ( ! wasDLError( "signMessage" ) )
            res = (*p_func)( cleartext, ciphertext, cipherLen, certificate, &structuring.data, errId, errTxt );
    }
    return res;
}


bool CryptPlugWrapper::checkMessageSignature( char** cleartext,
                                              const char* signaturetext,
                                              bool signatureIsBinary,
                                              int signatureLen,
                                              struct SignatureMetaData* sigmeta )
{
    bool res = false;
    if ( _initialized ) {
        bool (*p_func)( char**,
                        const char*,
                        bool,
                        int,
                        struct SignatureMetaData* )
            = (bool (*)(char**,
                        const char*,
                        bool,
                        int,
                        struct SignatureMetaData* ))
                dlsym(_libPtr, "checkMessageSignature");
        if ( ! wasDLError( "checkMessageSignature" ) )
            res = (*p_func)( cleartext,
                             signaturetext,
                             signatureIsBinary,
                             signatureLen,
                             sigmeta );
    }
    return res;
}


bool CryptPlugWrapper::storeCertificatesFromMessage(
        const char* ciphertext )
{
    return boolConstCharFunc( ciphertext, "storeCertificatesFromMessage" );
}


bool CryptPlugWrapper::findCertificates( const char*  addressee,
                                         char** certificates,
                                         int* newSize,
                                         bool secretOnly )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        // then call the findCertificates function
        bool (*p_func)( const char*, char**, int*, bool )
            = (bool (*)(const char*, char**, int*, bool ))
                dlsym(_libPtr, "findCertificates");
        if ( ! wasDLError( "findCertificates" ) )
            res = (*p_func)( addressee, certificates, newSize, secretOnly );
    }
    return res;
}

bool CryptPlugWrapper::encryptMessage( const char* cleartext,
                                       const char** ciphertext,
                                       const size_t* cipherLen,
                                       const char* addressee,
                                       StructuringInfoWrapper& structuring,
                                       int* errId,
                                       char** errTxt )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        // first (re-)initialize the StructuringInfo
        structuring.reset();
        // then call the encrypting function
        bool (*p_func)( const char*, const char**, const size_t*, const char*, struct StructuringInfo*, int*, char** )
            = (bool (*)(const char*, const char**, const size_t*, const char*, struct StructuringInfo*, int*, char** ))
                dlsym(_libPtr, "encryptMessage");
        if ( ! wasDLError( "encryptMessage" ) )
            res = (*p_func)( cleartext, ciphertext, cipherLen, addressee, &structuring.data, errId, errTxt );
    }
    return res;
}


bool CryptPlugWrapper::encryptAndSignMessage( const char* cleartext,
                                              const char** ciphertext,
                                              const char* certificate,
                                              StructuringInfoWrapper& structuring )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        // first (re-)initialize the StructuringInfo
        structuring.reset();
        // then call the signing/encrypting function
        bool (*p_func)( const char*, const char**, const char*, struct StructuringInfo* )
            = (bool (*)(const char*, const char**, const char*, struct StructuringInfo* ))
                dlsym(_libPtr, "encryptAndSignMessage");
        if ( ! wasDLError( "encryptAndSignMessage" ) )
            res = (*p_func)( cleartext, ciphertext, certificate, &structuring.data );
    }
    return res;
}


bool CryptPlugWrapper::decryptMessage( const char* ciphertext,
                                       bool        cipherIsBinary,
                                       int         cipherLen,
                                       char**      cleartext,
                                       const char* certificate,
                                       int* errId,
                                       char** errTxt )
{
    bool res = false;
    if ( _initialized ) {
        bool (*p_func)( const char*, bool, int, const char**, const char*, int*, char** )
            = (bool (*)(const char*, bool, int, const char**, const char*, int*, char** ))
                dlsym(_libPtr, "decryptMessage");
        if ( ! wasDLError( "decryptMessage" ) )
            res = (*p_func)( ciphertext, cipherIsBinary, cipherLen,
                             (const char**)cleartext,
                             certificate,
                             errId,
                             errTxt );
    }
    return res;
}


bool CryptPlugWrapper::decryptAndCheckMessage(
                            const char*  ciphertext,
                            bool         cipherIsBinary,
                            int          cipherLen,
                            char**       cleartext,
                            const char*  certificate,
                            bool*        signatureFound,
                            struct SignatureMetaData* sigmeta,
                            int*   errId,
                            char** errTxt )
{
    bool res = false;
    if ( _initialized ) {
        bool (*p_func)( const char*,
                        bool,
                        int,
                        const char**,
                        const char*,
                        bool*,
                        struct SignatureMetaData*,
                        int*,
                        char** )
            = (bool (*)(const char*,
                        bool,
                        int,
                        const char**,
                        const char*,
                        bool*,
                        struct SignatureMetaData*,
                        int*,
                        char** ))
                dlsym(_libPtr, "decryptAndCheckMessage");
        if ( ! wasDLError( "decryptAndCheckMessage" ) )
            res= (*p_func)( ciphertext,
                            cipherIsBinary,
                            cipherLen,
                            (const char**)cleartext,
                            certificate,
                            signatureFound,
                            sigmeta,
                            errId,
                            errTxt );
    }
    return res;
}



const char* CryptPlugWrapper::requestCertificateDialog()
{
    return constCharVoidFunc( "requestCertificateDialog" );
}


bool CryptPlugWrapper::requestDecentralCertificate( const char* certparms,
                                                    char** generatedKey,
                                                    int* keyLength )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char*, char**, int*  )
            = (bool (*)(const char*, char**, int*  ))
                dlsym(_libPtr, "requestDecentralCertificate");
        if ( ! wasDLError( "requestDecentralCertificate" ) )
            res = (*p_func)( certparms, generatedKey, keyLength );
    }
    return res;
}


bool CryptPlugWrapper::requestCentralCertificateAndPSE( const char* name,
          const char* email, const char* organization, const char* department,
          const char* ca_address )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char*, const char*, const char*, const char*, const char*  )
            = (bool (*)(const char*, const char*, const char*, const char*, const char*  ))
                dlsym(_libPtr, "requestCentralCertificateAndPSE");
        if ( ! wasDLError( "requestCentralCertificateAndPSE" ) )
            res = (*p_func)( name, email, organization, department, ca_address );
    }
    return res;
}


bool CryptPlugWrapper::createPSE()
{
    return boolVoidFunc( "createPSE" );
}


bool CryptPlugWrapper::registerCertificate( const char* certificate )
{
    return boolConstCharFunc( certificate, "registerCertificate" );
}


bool CryptPlugWrapper::requestCertificateProlongation( const char* certificate,
                                     const char* ca_address )
{
    bool res = false;
    if ( /*_active &&*/ _initialized ) {
        bool (*p_func)( const char*, const char*  )
            = (bool (*)(const char*, const char*  ))
                dlsym(_libPtr, "requestCertificateProlongation");
        if ( ! wasDLError( "requestCertificateProlongation" ) )
            res = (*p_func)( certificate, ca_address );
    }
    return res;
}


const char* CryptPlugWrapper::certificateChain()
{
    return constCharVoidFunc( "certificateChain" );
}


bool CryptPlugWrapper::deleteCertificate( const char* certificate )
{
    return boolConstCharFunc( certificate, "deleteCertificate" );
}


bool CryptPlugWrapper::archiveCertificate( const char* certificate )
{
    return boolConstCharFunc( certificate, "archiveCertificate" );
}



const char* CryptPlugWrapper::displayCRL()
{
    return constCharVoidFunc( "displayCRL" );
}


void CryptPlugWrapper::updateCRL()
{
    voidVoidFunc( "updateCRL" );
}


void CryptPlugWrapper::freeSignatureMetaData( struct SignatureMetaData* sigmeta )
{
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
    free( sigmeta->nota_xml );
}

CryptPlugWrapper::CertificateInfoList CryptPlugWrapper::listKeys( const QString& pattern,
								  bool remote,
								  bool* truncated)
{
    CertificateInfoList result;
    if( truncated ) *truncated = false;
    if ( /*_active &&*/ _initialized ) {
	CertIterator*  it = 0;
	{
	     CertIterator* (*p_func)( const char*, int  )
		  = (CertIterator* (*)( const char*, int  ))
		  dlsym(_libPtr, "startListCertificates");
	     if ( ! wasDLError( "startListCertificates" ) )
		  it = (*p_func)( pattern.utf8().data(), remote );
	     else return result;
	}

	::CertificateInfo* info = 0;
	do {
	     int (*p_func)( CertIterator*, ::CertificateInfo** )
		  = (int (*)( CertIterator*, ::CertificateInfo** ))
		  dlsym(_libPtr, "nextCertificate");
	     if ( ! wasDLError( "nextCertificate" ) ) {
	       int rv = (*p_func)( it, &info  );
	       if( rv ) {
		 kdDebug() << "error" << endl;
	       }
	     } else {
	       info = 0;
	     }
	     if( info ) {
		  CryptPlugWrapper::CertificateInfo cpwinfo;
		  for( char** ptr = info->userid; *ptr; ++ptr ) {
		    cpwinfo.userid << QString::fromUtf8( *ptr );
		  }
		  kdDebug() << "CryptPlugWrapper got " << cpwinfo.userid[0] << endl;
		  cpwinfo.serial = QString::fromUtf8( info->serial );
		  kdDebug() << "fpr=" << info->fingerprint << endl;
		  cpwinfo.fingerprint = QString::fromUtf8( info->fingerprint );
		  kdDebug() << "Done getting fpr" << endl;

		  cpwinfo.issuer = QString::fromUtf8( info->issuer );
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

		  ::DnPair* a = info->dnarray;
		  while( a && a->key && a->value ) {
		    //kdDebug() << "CryptPlugWrapper::listKeys() " << a->key << " = " << a->value << endl;
		    if( cpwinfo.dn[QString::fromUtf8(a->key)].isNull() )
		      cpwinfo.dn[QString::fromUtf8(a->key).upper()] = QString::fromUtf8(a->value);
		    else 
		      cpwinfo.dn[QString::fromUtf8(a->key).upper()] =+ ", "+ QString::fromUtf8(a->value);
		    ++a;
		  }
		  result << cpwinfo;
	     }
	} while( info );
	
	{
	     int (*p_func)( CertIterator*  )
		  = (int (*)( CertIterator*  ))
		  dlsym(_libPtr, "endListCertificates");
	     if ( ! wasDLError( "endListCertificates" ) ) {
	       int retval = (*p_func)( it  );
	       if( retval && truncated ) *truncated = true;
	     }
	}
    }
    return result;
}

int CryptPlugWrapper::importCertificate( const QString& fingerprint, QString* info )
{
    int res = -1;
    if ( _initialized ) {
      char* tmpinfo = NULL;
        int (*p_func)( const char*, char**  )
            = (int (*)(const char*, char**  ))
                dlsym(_libPtr, "importCertificateWithFPR");
        if ( ! wasDLError( "importCertificateWithFPR" ) )
            res = (*p_func)( fingerprint.utf8().data(), &tmpinfo );
	if( tmpinfo && info ) {
	  *info = QString::fromUtf8( tmpinfo );
	  free( tmpinfo );
	}
    } else {
      if( info ) *info = "";
    }
    return res;
}

int CryptPlugWrapper::importCertificate( const char* data, size_t length, QString* info )
{
    int res = -1;
    if ( _initialized ) {
      char* tmpinfo = NULL;
        int (*p_func)( const char*, size_t, char**  )
            = (int (*)(const char*, size_t, char**  ))
                dlsym(_libPtr, "importCertificateFromMem");
        if ( ! wasDLError( "importCertificateFromMem" ) )
            res = (*p_func)( data, length, &tmpinfo );
	if( tmpinfo && info ) {
	  *info = QString::fromUtf8( tmpinfo );
	  free( tmpinfo );
	}
    } else {
      if( info ) *info = "";
    }
    return res;
}
