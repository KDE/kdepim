/* -*- c++ -*-

  CRYPTPLUG - an independent cryptography plug-in
  API. CryptPlugWrapperList holds any number of crypto plug-ins.

  Copyright (C) 2001 by Klarälvdalens Datakonsult AB

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

#include "cryptplugwrapperlist.h"

#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

CryptPlugWrapper* CryptPlugWrapperList::active() const 
{
  for ( QPtrListIterator<CryptPlugWrapper> it( *this ) ; it.current() ; ++it )
    if ( (*it)->active() )
      return *it;
  return 0;
}


void CryptPlugWrapperList::showPluginInitError( 
                                CryptPlugWrapper* wrapper,
                                CryptPlugWrapper::InitStatus initStatus,
                                QString errorMsg )
{
    QString msg;
    switch ( initStatus ) {
    case CryptPlugWrapper::InitStatus_undef:
        msg = i18n( "Error loading plug-in \"%1\"!\n(code: InitStatus_undef)")
            .arg( wrapper->displayName() );
        break;
    case CryptPlugWrapper::InitStatus_NoLibName:
        msg = i18n("Please specify the LOCATION of plug-in \"%1\"!\n"
                "(complete path/filename of the respective library file)")
            .arg( wrapper->displayName() );
        break;
    case CryptPlugWrapper::InitStatus_LoadError:
        msg = i18n( "Error loading plug-in library file \"%1\"!\n"
                    "(code: \"%2\")").arg( wrapper->libName() )
            .arg( errorMsg );
        break;
    case CryptPlugWrapper::InitStatus_InitError: {
            msg = i18n( "Error: Plug-in \"%1\" initialization unsuccessful.\n"
                        "library: %2\n"
                        "version: %3\n"
                        "Plug-in is out-dated or not installed properly.\n" )
                .arg( wrapper->displayName() )
                .arg( wrapper->libName() )
                .arg( wrapper->libVersion() );
        }
        break;
    default:
        msg = i18n( "Cannot load/initialize plug-in library \"%1\"!\n"
                    "(unknown error)").arg( wrapper->libName() );
    }
    // Use a queued message box because a normal message box would block
    // this initialization "thread" and this would lead to a crash in
    // code called by the event loop due to incomplete initialization.
    KMessageBox::queuedMessageBox( 0, KMessageBox::Error, msg );
}


void CryptPlugWrapperList::loadFromConfig( KConfig* config )
{
    clear();
    KConfigGroupSaver saver( config, "General" );
    int numPlugins = config->readNumEntry( "crypt-plug-count", 0 );
    for( int i = 0; i < numPlugins; ++i ) {        
        // load the initial config data for one plugin
        config->setGroup( QString( "CryptPlug #%1" ).arg( i ) );
        
        QString name = config->readEntry( "name" );
        QString location = config->readPathEntry( "location" );
        QString updateURL = config->readPathEntry( "updates" );
        bool active = config->readBoolEntry( "active", false );

        // Try to initialize that plugin
        CryptPlugWrapper::InitStatus initStatus;
        QString errorMsg;
        CryptPlugWrapper* newWrapper = new CryptPlugWrapper( 0,
                                                             name,
                                                             location,
                                                             updateURL,
                                                             active );
        if( !newWrapper->initialize( &initStatus, &errorMsg ) )
            showPluginInitError( newWrapper, initStatus, errorMsg );

        kdDebug( 5100 ) << "Loaded crypto plugin " << name
                        << " at location " << location << endl;
        append( newWrapper );

        // Configure the current plugin by means of the wrapper. If
        // the plugin was not initialized successfully, these calls
        // will end in Nirvana.

          
        // Signature configuration
        QString sigKeyCert = config->readEntry( "SigKeyCert" );
        newWrapper->setSignatureKeyCertificate( sigKeyCert.utf8() );
          
        int sigAlgo = config->readNumEntry( "SigAlgo", SignAlg_SHA1 );
        newWrapper->setSignatureAlgorithm( static_cast<SignatureAlgorithm>( sigAlgo ) );
    
        int compoundMode = config->readNumEntry( "SignatureCompoundMode",
                                                 SignatureCompoundMode_Detached );
        newWrapper->setSignatureCompoundMode( static_cast<SignatureCompoundMode>( compoundMode ) );

        int sendCerts = config->readNumEntry( "SendCerts",    
                                              SendCert_SendOwn );
        newWrapper->setSendCertificates( static_cast<SendCertificates>( sendCerts ) );
          
        int signEmail = config->readNumEntry( "SignEmail", 
                                              SignEmail_SignAll );
        newWrapper->setSignEmail( static_cast<SignEmail>( signEmail ) );

          
        bool warnSendUnsigned = config->readBoolEntry( "WarnSendUnsigned",
                                                      true );
        newWrapper->setWarnSendUnsigned( warnSendUnsigned );
          
        bool saveSentSigs = config->readBoolEntry( "SaveSentSigs", true );
        newWrapper->setSaveSentSignatures( saveSentSigs );
          
        bool warnNoCert = config->readBoolEntry( "WarnNoCert", true );
        newWrapper->setWarnNoCertificate( warnNoCert );
          
        int pinRequests = config->readNumEntry( "NumPINRequests", 
                                                PinRequest_Always );
        newWrapper->setNumPINRequests( static_cast<PinRequests>( pinRequests ) );

        int pinRequestsInt = config->readNumEntry( "NumPINRequestsInt",
                                                   10 );
        newWrapper->setNumPINRequestsInterval( pinRequestsInt );
          
        bool checkSigToRoot = config->readNumEntry( "CheckSigCertToRoot",
                                                    true );
        newWrapper->setCheckSignatureCertificatePathToRoot( checkSigToRoot );
          
        bool sigUseCRLs = config->readBoolEntry( "SigUseCRLs", true );
        newWrapper->setSignatureUseCRLs( sigUseCRLs );

        bool sigCertWarnNearExpire = config->readBoolEntry( "SigCertWarnNearExpire", true );
        newWrapper->setSignatureCertificateExpiryNearWarning( sigCertWarnNearExpire );
          
        int sigCertWarnNearExpireInt = config->readNumEntry( "SigCertWarnNearExpireInt", 14 );
        newWrapper->setSignatureCertificateExpiryNearInterval( sigCertWarnNearExpireInt );

        bool caCertWarnNearExpire = config->readBoolEntry( "CACertWarnNearExpire", true );
        newWrapper->setCACertificateExpiryNearWarning( caCertWarnNearExpire );
          
        int caCertWarnNearExpireInt = config->readNumEntry( "CACertWarnNearExpireInt", 14 );
        newWrapper->setCACertificateExpiryNearInterval( caCertWarnNearExpireInt );

        bool rootCertWarnNearExpire = config->readBoolEntry( "RootCertWarnNearExpire", true );
        newWrapper->setRootCertificateExpiryNearWarning( rootCertWarnNearExpire );
          
        int rootCertWarnNearExpireInt = config->readNumEntry( "RootCertWarnNearExpireInt", 14 );
        newWrapper->setRootCertificateExpiryNearInterval( rootCertWarnNearExpireInt );

        bool warnEmailNotInCert = config->readBoolEntry( "WarnEmailNotInCert", true );
        newWrapper->setWarnNoCertificate( warnEmailNotInCert );

          
        // Encryption configuration
        int encryptAlgo = config->readNumEntry( "EncryptAlgo", EncryptAlg_RSA );
        newWrapper->setEncryptionAlgorithm( static_cast<EncryptionAlgorithm>( encryptAlgo ) );
          
        int encryptEmail = config->readNumEntry( "EncryptEmail", 
                                                 EncryptEmail_EncryptAll );
        newWrapper->setEncryptEmail( static_cast<EncryptEmail>( encryptEmail ) );
          
        bool warnSendUnencrypted = config->readBoolEntry( "WarnSendUnencrypted",
                                                         true );
        newWrapper->setWarnSendUnencrypted( warnSendUnencrypted );
          
        bool alwaysEncryptToSelf = config->readBoolEntry( "AlwaysEncryptToSelf",
                                                         true );
        newWrapper->setAlwaysEncryptToSelf( alwaysEncryptToSelf );
          

        bool warnRecvCertNearExpire = config->readBoolEntry( "WarnRecvCertNearExpire",
                                                             true );
        newWrapper->setReceiverCertificateExpiryNearWarning( warnRecvCertNearExpire );
          
        int warnRecvCertNearExpireInt = config->readNumEntry( "WarnRecvCertNearExpireInt",
                                                              14 );
        newWrapper->setReceiverCertificateExpiryNearWarningInterval( warnRecvCertNearExpireInt );

        bool warnCertInChainNearExpire = config->readBoolEntry( "WarnCertInChainNearExpire",
                                                                true );
        newWrapper->setCertificateInChainExpiryNearWarning( warnCertInChainNearExpire );
          
        int warnCertInChainNearExpireInt = config->readNumEntry( "WarnCertInChainNearExpireInt",
                                                                 14 );
        newWrapper->setCertificateInChainExpiryNearWarningInterval( warnCertInChainNearExpireInt );

          
        bool warnRecvAddrNotInCert = config->readBoolEntry( "WarnRecvAddrNotInCert" );
        newWrapper->setReceiverEmailAddressNotInCertificateWarning( warnRecvAddrNotInCert );
          
        bool saveMsgEncrypted = config->readBoolEntry( "SaveMsgsEncrypted", false );
        newWrapper->setSaveMessagesEncrypted( saveMsgEncrypted );
          
        bool checkCertPath = config->readBoolEntry( "CheckCertPath", true );
        newWrapper->setCheckCertificatePath( checkCertPath );
          
        bool checkEncryptToRoot = config->readBoolEntry( "CheckEncryptCertToRoot",
                                                         true );
        newWrapper->setCheckEncryptionCertificatePathToRoot( checkEncryptToRoot );
          
        bool encryptUseCRLs = config->readBoolEntry( "EncryptUseCRLs", true );
        newWrapper->setEncryptionUseCRLs( encryptUseCRLs );
          
        bool encryptCRLWarnNearExpire = config->readBoolEntry( "EncryptCRLWarnNearExpire", true );
        newWrapper->setEncryptionCRLExpiryNearWarning( encryptCRLWarnNearExpire );
          
        int encryptCRLWarnNearExpireInt = config->readNumEntry( "EncryptCRLWarnNearExpireInt", 14 );
        newWrapper->setEncryptionCRLNearExpiryInterval( encryptCRLWarnNearExpireInt );
                                                    
        // directory services configuration
        int numDirServers = config->readNumEntry( "NumDirServers", 0 );
        for( int j = 0; j < numDirServers; j++ ) {
            QString dirServerName = 
                config->readEntry( QString( "DirServer%1Name" ).arg( j ) );
            int dirServerPort = 
                config->readNumEntry( QString( "DirServer%1Port" ).arg( j ), 0 );
            QString dirServerDescr =
                config->readEntry( QString( "DirServer%1Descr" ).arg( j ) );
            newWrapper->appendDirectoryServer( dirServerName.utf8(),
                                               dirServerPort,
                                               dirServerDescr.utf8() );

        }
          
        int certSource = config->readNumEntry( "CertSource", CertSrc_ServerLocal );
        newWrapper->setCertificateSource( static_cast<CertificateSource>( certSource ) );

        int crlSource = config->readNumEntry( "CRLSource", CertSrc_ServerLocal );
        newWrapper->setCRLSource( static_cast<CertificateSource>( crlSource ) );
    }
}

CryptPlugWrapper * CryptPlugWrapperList::findForLibName( const QString & libName ) const
{
  for ( QPtrListIterator<CryptPlugWrapper> it( *this ) ; it.current() ; ++it )
    if ( (*it)->libName().find( libName, 0, false ) >= 0 )
      return *it;
  return 0;
}
