/*
    cryptplugfactory.cpp

    This file is part of libkleopatra, the KDE key management library
    Copyright (c) 2001,2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

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
#include <backends/qgpgme/qgpgmebackend.h>
#include <backends/kpgp/pgp2backend.h>
#include <backends/kpgp/pgp5backend.h>
#include <backends/kpgp/pgp6backend.h>
#include <backends/kpgp/gpg1backend.h>
#include <ui/backendconfigwidget.h>

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <assert.h>

Kleo::CryptPlugFactory * Kleo::CryptPlugFactory::mSelf = 0;
KMail::CryptPlugFactory * KMail::CryptPlugFactory::mSelf = 0;

Kleo::CryptPlugFactory::CryptPlugFactory()
  : QObject( qApp, "CryptPlugFactory::instance()" )
{
  mSelf = this;
  mConfigObject = 0;
  mBackendList.push_back( new QGpgMEBackend() );
  mBackendList.push_back( new PGP2Backend() );
  mBackendList.push_back( new PGP5Backend() );
  mBackendList.push_back( new PGP6Backend() );
  mBackendList.push_back( new GPG1Backend() );
  scanForBackends();
  readConfig();
}

Kleo::CryptPlugFactory::~CryptPlugFactory() {
  mSelf = 0;

  for ( QValueVector<CryptoBackend*>::iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    delete *it;
    *it = 0;
  }
  delete mConfigObject;
  mConfigObject = 0;
}

Kleo::CryptPlugFactory * Kleo::CryptPlugFactory::instance() {
  if ( !mSelf )
    mSelf = new CryptPlugFactory();
  return mSelf;
}


// const Kleo::CryptoBackend* Kleo::CryptPlugFactory::smimeBackend() const {
//   return mSMIMEBackend;
// }

// const Kleo::CryptoBackend* Kleo::CryptPlugFactory::openpgpBackend() const {
//   return mOpenPGPBackend;
// }

const Kleo::CryptoBackend::Protocol * Kleo::CryptPlugFactory::smime() const {
  return mSMIMEBackend ? mSMIMEBackend->smime() : 0 ;
}

const Kleo::CryptoBackend::Protocol * Kleo::CryptPlugFactory::openpgp() const {
  return mOpenPGPBackend ? mOpenPGPBackend->openpgp() : 0 ;
}

Kleo::CryptoConfig * Kleo::CryptPlugFactory::config() const {
  // ## should we use mSMIMEBackend? mOpenPGPBackend? backend(0) i.e. always qgpgme?
  return backend( 0 ) ? backend( 0 )->config() : 0;
}

bool Kleo::CryptPlugFactory::hasBackends() const {
  return !mBackendList.empty();
}

void Kleo::CryptPlugFactory::scanForBackends( QStringList * reasons ) {
  if ( !reasons )
    return;
  for ( QValueVector<CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    assert( *it );
    QString reason;
    if ( (*it)->supportsOpenPGP() && !(*it)->checkForOpenPGP( &reason ) ) {
      reasons->push_back( i18n("While scanning for OpenPGP support in backend %1:")
			  .arg( (*it)->displayName() ) );
      reasons->push_back( "  " + reason );
    }
    if ( (*it)->supportsSMIME() && !(*it)->checkForSMIME( &reason ) ) {
      reasons->push_back( i18n("While scanning for S/MIME support in backend %1:")
			  .arg( (*it)->displayName() ) );
      reasons->push_back( "  " + reason );
    }
  }
}

const Kleo::CryptoBackend * Kleo::CryptPlugFactory::backend( unsigned int idx ) const {
  return ( idx < mBackendList.size() ) ? mBackendList[idx] : 0 ;
}

const Kleo::CryptoBackend * Kleo::CryptPlugFactory::backendByName( const QString& name ) const {
  for ( QValueVector<CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    if ( (*it)->name() == name )
      return *it;
  }
  return 0;
}

Kleo::BackendConfigWidget * Kleo::CryptPlugFactory::configWidget( QWidget * parent, const char * name ) const {
  return new Kleo::BackendConfigWidget( const_cast<Kleo::CryptPlugFactory*>( this ), parent, name );
}

KConfig* Kleo::CryptPlugFactory::configObject() const {
  if ( !mConfigObject )
    mConfigObject = new KConfig( "libkleopatrarc" );
  return mConfigObject;
}

void Kleo::CryptPlugFactory::setSMIMEBackend( const CryptoBackend* backend ) {
  QString name = backend ? backend->name() : QString::null;
  KConfigGroup group( configObject(), "Backends" );
  group.writeEntry( "SMIME", name );
}

void Kleo::CryptPlugFactory::setOpenPGPBackend( const CryptoBackend* backend ) {
  QString name = backend ? backend->name() : QString::null;
  KConfigGroup group( configObject(), "Backends" );
  group.writeEntry( "OpenPGP", name );
}

void Kleo::CryptPlugFactory::readConfig() {
  KConfigGroup group( configObject(), "Backends" );
  QString smimeBackend = group.readEntry( "SMIME", "gpgme" );
  mSMIMEBackend = backendByName( smimeBackend );

  QString openPGPBackend = group.readEntry( "OpenPGP", "gpgme" );
  mOpenPGPBackend = backendByName( openPGPBackend );
}

//
//
// KMail::CryptPlugFactory: backwards compat stuff (ugly)
//
//


KMail::CryptPlugFactory::CryptPlugFactory()
  : Kleo::CryptPlugFactory(),
    mCryptPlugWrapperList( 0 )
{
  mSelf = this;

  mCryptPlugWrapperList = new CryptPlugWrapperList();
  mCryptPlugWrapperList->setAutoDelete( false );
  updateCryptPlugWrapperList();
}

KMail::CryptPlugFactory::~CryptPlugFactory() {
  mSelf = 0;
  delete mCryptPlugWrapperList; mCryptPlugWrapperList = 0;
}

KMail::CryptPlugFactory * KMail::CryptPlugFactory::instance() {
  if ( !mSelf )
    mSelf = new CryptPlugFactory();
  return mSelf;
}

CryptPlugWrapper * KMail::CryptPlugFactory::active() const {
  if ( smime() && smime()->active() )
    return smime();
  if ( openpgp() && openpgp()->active() )
    return openpgp();
  return 0;
}

CryptPlugWrapper * KMail::CryptPlugFactory::createForProtocol( const QString & proto ) const {
  QString p = proto.lower();
  if ( p == "application/pkcs7-signature" )
    return smime();
  if ( p == "application/pgp-signature" )
    return openpgp();
  return 0;
}

CryptPlugWrapper * KMail::CryptPlugFactory::smime() const {
  return mCryptPlugWrapperList->findForLibName( "smime" );
}

CryptPlugWrapper * KMail::CryptPlugFactory::openpgp() const {
  return mCryptPlugWrapperList->findForLibName( "openpgp" );
}

void KMail::CryptPlugFactory::scanForBackends( QStringList * reason ) {
  Kleo::CryptPlugFactory::scanForBackends( reason );
  updateCryptPlugWrapperList();
}

void KMail::CryptPlugFactory::updateCryptPlugWrapperList() {
  mCryptPlugWrapperList->clear();
  for ( QValueVector<Kleo::CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    if ( CryptPlugWrapper * w = dynamic_cast<CryptPlugWrapper*>( (*it)->openpgp() ) )
      mCryptPlugWrapperList->append( w );
    if ( CryptPlugWrapper * w = dynamic_cast<CryptPlugWrapper*>( (*it)->smime() ) )
      mCryptPlugWrapperList->append( w );
  }
}

#include "cryptplugfactory.moc"
