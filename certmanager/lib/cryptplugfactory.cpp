/*  -*- mode: C++; c-file-style: "gnu" -*-
    cryptplugfactory.cpp

    This file is part of libkleopatra, the KDE key management library
    Copyright (c) 2001,2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cryptplugfactory.h"
#include "cryptplugwrapperlist.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>

Kleo::CryptPlugFactory * Kleo::CryptPlugFactory::mSelf = 0;

Kleo::CryptPlugFactory::CryptPlugFactory()
  : QObject( qApp, "CryptPlugFactory::instance()" ),
    mCryptPlugWrapperList( 0 )
{
  mSelf = this;
  mCryptPlugWrapperList = new CryptPlugWrapperList();
  loadFromConfig( kapp->config() );
  mCryptPlugWrapperList->setAutoDelete( true );
}

Kleo::CryptPlugFactory::~CryptPlugFactory() {
  mSelf = 0;

  delete mCryptPlugWrapperList;
  mCryptPlugWrapperList = 0;
}

Kleo::CryptPlugFactory * Kleo::CryptPlugFactory::instance() {
  if ( !mSelf )
    mSelf = new CryptPlugFactory();
  return mSelf;
}

CryptPlugWrapper * Kleo::CryptPlugFactory::active() const {
  if ( smime() && smime()->active() )
    return smime();
  if ( openpgp() && openpgp()->active() )
    return openpgp();
  return 0;
}

CryptPlugWrapper * Kleo::CryptPlugFactory::createForProtocol( const QString & proto ) const {
  QString p = proto.lower();
  if ( p == "application/pkcs7-signature" )
    return smime();
  if ( p == "application/pgp-signature" )
    return openpgp();
  return 0;
}

CryptPlugWrapper * Kleo::CryptPlugFactory::smime() const {
  return mCryptPlugWrapperList->findForLibName( "smime" );
}

CryptPlugWrapper * Kleo::CryptPlugFactory::openpgp() const {
  return mCryptPlugWrapperList->findForLibName( "openpgp" );
}


void Kleo::CryptPlugFactory::scanForBackends() {
  mCryptPlugWrapperList->clear();

  static const struct {
    const char * displayName;
    const char * libName;
    const char * errorMsg;
    bool active;
  } cryptPlugs[] = {
    { "S/MIME", "smime",
      I18N_NOOP("GpgSM not found. "
		"S/MIME cryptographic backend has beed disabled."),
      true
    },
    { "OpenPGP", "openpgp",
      I18N_NOOP("GnuPG not found. "
		"OpenPGP/MIME cryptographic backend has beed disabled."),
      false
    }
  };
  static const int numCryptPlugs = sizeof cryptPlugs / sizeof *cryptPlugs ;

  for ( int i = 0 ; i < numCryptPlugs ; ++i ) {
    CryptPlugWrapper * w =
      new CryptPlugWrapper( cryptPlugs[i].displayName, cryptPlugs[i].libName,
			    QString::null, cryptPlugs[i].active );

    CryptPlugWrapper::InitStatus initStatus;
    QString errorMsg;
    if ( !w->initialize( &initStatus, &errorMsg ) ) {
      KMessageBox::queuedMessageBox( 0, KMessageBox::Error, i18n( cryptPlugs[i].errorMsg ) );
      delete w; w = 0;
    } else
      mCryptPlugWrapperList->append( w );
  }
}

static void showPluginInitError( const CryptPlugWrapper* wrapper,
				 CryptPlugWrapper::InitStatus initStatus,
				 const QString & errorMsg )
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


void Kleo::CryptPlugFactory::loadFromConfig( KConfig * config ) {
  mCryptPlugWrapperList->clear();

  KConfigGroupSaver saver( config, "General" );

  if ( !config->hasKey( "crypt-plug-count" ) ) {
    scanForBackends();
    return;
  }

  const int numPlugins = config->readNumEntry( "crypt-plug-count", 0 );

  for( int i = 0; i < numPlugins; ++i ) {        
    // load the initial config data for one plugin
    config->setGroup( QString( "CryptPlug #%1" ).arg( i ) );

    const QString name = config->readEntry( "name" );
    const QString location = config->readPathEntry( "location" );
    const QString updateURL = config->readPathEntry( "updates" );
    const bool active = config->readBoolEntry( "active", false );

    // Try to initialize that plugin
    CryptPlugWrapper::InitStatus initStatus;
    QString errorMsg;
    CryptPlugWrapper* newWrapper =
      new CryptPlugWrapper( name, location, updateURL, active );
    if( !newWrapper->initialize( &initStatus, &errorMsg ) ) {
      showPluginInitError( newWrapper, initStatus, errorMsg );
      continue;
    }

    kdDebug( 5100 ) << "Loaded crypto plugin " << name
		    << " at location " << location << endl;
    mCryptPlugWrapperList->append( newWrapper );

    // Configure the current plugin by means of the wrapper. If
    // the plugin was not initialized successfully, these calls
    // will end in Nirvana.


    // Signature configuration
    const int sigAlgo = config->readNumEntry( "SigAlgo", SignAlg_SHA1 );
    newWrapper->setSignatureAlgorithm( static_cast<SignatureAlgorithm>( sigAlgo ) );

    const int compoundMode = config->readNumEntry( "SignatureCompoundMode",
						   SignatureCompoundMode_Detached );
    newWrapper->setSignatureCompoundMode( static_cast<SignatureCompoundMode>( compoundMode ) );

    const int sendCerts = config->readNumEntry( "SendCerts",    
						SendCert_SendOwn );
    newWrapper->setSendCertificates( static_cast<SendCertificates>( sendCerts ) );

    const int signEmail = config->readNumEntry( "SignEmail", 
					  SignEmail_SignAll );
    newWrapper->setSignEmail( static_cast<SignEmail>( signEmail ) );


    const bool warnSendUnsigned = config->readBoolEntry( "WarnSendUnsigned",
							 true );
    newWrapper->setWarnSendUnsigned( warnSendUnsigned );

    const bool saveSentSigs = config->readBoolEntry( "SaveSentSigs", true );
    newWrapper->setSaveSentSignatures( saveSentSigs );

    const bool warnNoCert = config->readBoolEntry( "WarnNoCert", true );
    newWrapper->setWarnNoCertificate( warnNoCert );

    const int pinRequests = config->readNumEntry( "NumPINRequests", 
						  PinRequest_Always );
    newWrapper->setNumPINRequests( static_cast<PinRequests>( pinRequests ) );

    const int pinRequestsInt = config->readNumEntry( "NumPINRequestsInt",
							   10 );
    newWrapper->setNumPINRequestsInterval( pinRequestsInt );
          
    const bool checkSigToRoot = config->readNumEntry( "CheckSigCertToRoot",
						      true );
    newWrapper->setCheckSignatureCertificatePathToRoot( checkSigToRoot );
          
    const bool sigUseCRLs = config->readBoolEntry( "SigUseCRLs", true );
    newWrapper->setSignatureUseCRLs( sigUseCRLs );

    const bool sigCertWarnNearExpire = config->readBoolEntry( "SigCertWarnNearExpire", true );
    newWrapper->setSignatureCertificateExpiryNearWarning( sigCertWarnNearExpire );
          
    const int sigCertWarnNearExpireInt = config->readNumEntry( "SigCertWarnNearExpireInt", 14 );
    newWrapper->setSignatureCertificateExpiryNearInterval( sigCertWarnNearExpireInt );

    const bool caCertWarnNearExpire = config->readBoolEntry( "CACertWarnNearExpire", true );
    newWrapper->setCACertificateExpiryNearWarning( caCertWarnNearExpire );
          
    const int caCertWarnNearExpireInt = config->readNumEntry( "CACertWarnNearExpireInt", 14 );
    newWrapper->setCACertificateExpiryNearInterval( caCertWarnNearExpireInt );
    
    const bool rootCertWarnNearExpire = config->readBoolEntry( "RootCertWarnNearExpire", true );
    newWrapper->setRootCertificateExpiryNearWarning( rootCertWarnNearExpire );
    
    const int rootCertWarnNearExpireInt = config->readNumEntry( "RootCertWarnNearExpireInt", 14 );
    newWrapper->setRootCertificateExpiryNearInterval( rootCertWarnNearExpireInt );
    
    const bool warnEmailNotInCert = config->readBoolEntry( "WarnEmailNotInCert", true );
    newWrapper->setWarnNoCertificate( warnEmailNotInCert );
    
    
    // Encryption configuration
    const int encryptAlgo = config->readNumEntry( "EncryptAlgo", EncryptAlg_RSA );
    newWrapper->setEncryptionAlgorithm( static_cast<EncryptionAlgorithm>( encryptAlgo ) );
    
    const int encryptEmail = config->readNumEntry( "EncryptEmail", 
						   EncryptEmail_EncryptAll );
    newWrapper->setEncryptEmail( static_cast<EncryptEmail>( encryptEmail ) );
    
    const bool warnSendUnencrypted = config->readBoolEntry( "WarnSendUnencrypted",
							    true );
    newWrapper->setWarnSendUnencrypted( warnSendUnencrypted );
    
    const bool alwaysEncryptToSelf = config->readBoolEntry( "AlwaysEncryptToSelf",
							    true );
    newWrapper->setAlwaysEncryptToSelf( alwaysEncryptToSelf );
    
    
    const bool warnRecvCertNearExpire = config->readBoolEntry( "WarnRecvCertNearExpire",
							       true );
    newWrapper->setReceiverCertificateExpiryNearWarning( warnRecvCertNearExpire );
    
    const int warnRecvCertNearExpireInt = config->readNumEntry( "WarnRecvCertNearExpireInt",
								14 );
    newWrapper->setReceiverCertificateExpiryNearWarningInterval( warnRecvCertNearExpireInt );
    
    const bool warnCertInChainNearExpire = config->readBoolEntry( "WarnCertInChainNearExpire",
								  true );
    newWrapper->setCertificateInChainExpiryNearWarning( warnCertInChainNearExpire );
    
    const int warnCertInChainNearExpireInt = config->readNumEntry( "WarnCertInChainNearExpireInt",
								   14 );
    newWrapper->setCertificateInChainExpiryNearWarningInterval( warnCertInChainNearExpireInt );
    
    
    const bool warnRecvAddrNotInCert = config->readBoolEntry( "WarnRecvAddrNotInCert" );
    newWrapper->setReceiverEmailAddressNotInCertificateWarning( warnRecvAddrNotInCert );
    
    const bool saveMsgEncrypted = config->readBoolEntry( "SaveMsgsEncrypted", false );
    newWrapper->setSaveMessagesEncrypted( saveMsgEncrypted );
          
    const bool checkCertPath = config->readBoolEntry( "CheckCertPath", true );
    newWrapper->setCheckCertificatePath( checkCertPath );
    
    const bool checkEncryptToRoot = config->readBoolEntry( "CheckEncryptCertToRoot",
							   true );
    newWrapper->setCheckEncryptionCertificatePathToRoot( checkEncryptToRoot );
    
    const bool encryptUseCRLs = config->readBoolEntry( "EncryptUseCRLs", true );
    newWrapper->setEncryptionUseCRLs( encryptUseCRLs );
    
    const bool encryptCRLWarnNearExpire = config->readBoolEntry( "EncryptCRLWarnNearExpire", true );
    newWrapper->setEncryptionCRLExpiryNearWarning( encryptCRLWarnNearExpire );
    
    const int encryptCRLWarnNearExpireInt = config->readNumEntry( "EncryptCRLWarnNearExpireInt", 14 );
    newWrapper->setEncryptionCRLNearExpiryInterval( encryptCRLWarnNearExpireInt );
    
    const int certSource = config->readNumEntry( "CertSource", CertSrc_ServerLocal );
    newWrapper->setCertificateSource( static_cast<CertificateSource>( certSource ) );
    
    const int crlSource = config->readNumEntry( "CRLSource", CertSrc_ServerLocal );
    newWrapper->setCRLSource( static_cast<CertificateSource>( crlSource ) );
  }
}

#include "cryptplugfactory.moc"
