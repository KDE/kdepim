/*
    cryptobackendfactory.cpp

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

#include "cryptobackendfactory.h"

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

Kleo::CryptoBackendFactory * Kleo::CryptoBackendFactory::mSelf = 0;

Kleo::CryptoBackendFactory::CryptoBackendFactory()
  : QObject( qApp, "CryptoBackendFactory::instance()" )
{
  mSelf = this;
  mConfigObject = 0;
  mBackendList.push_back( new QGpgMEBackend() );
#if 0 // disabled for kde-3.3
  mBackendList.push_back( new PGP2Backend() );
  mBackendList.push_back( new PGP5Backend() );
  mBackendList.push_back( new PGP6Backend() );
  mBackendList.push_back( new GPG1Backend() );
#endif
  scanForBackends();
  readConfig();
}

Kleo::CryptoBackendFactory::~CryptoBackendFactory() {
  mSelf = 0;

  for ( QValueVector<CryptoBackend*>::iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    delete *it;
    *it = 0;
  }
  delete mConfigObject;
  mConfigObject = 0;
}

Kleo::CryptoBackendFactory * Kleo::CryptoBackendFactory::instance() {
  if ( !mSelf )
    mSelf = new CryptoBackendFactory();
  return mSelf;
}


// const Kleo::CryptoBackend* Kleo::CryptoBackendFactory::smimeBackend() const {
//   return mSMIMEBackend;
// }

// const Kleo::CryptoBackend* Kleo::CryptoBackendFactory::openpgpBackend() const {
//   return mOpenPGPBackend;
// }

const Kleo::CryptoBackend::Protocol * Kleo::CryptoBackendFactory::smime() const {
  return mSMIMEBackend ? mSMIMEBackend->smime() : 0 ;
}

const Kleo::CryptoBackend::Protocol * Kleo::CryptoBackendFactory::openpgp() const {
  return mOpenPGPBackend ? mOpenPGPBackend->openpgp() : 0 ;
}

Kleo::CryptoConfig * Kleo::CryptoBackendFactory::config() const {
  // ## should we use mSMIMEBackend? mOpenPGPBackend? backend(0) i.e. always qgpgme?
  return backend( 0 ) ? backend( 0 )->config() : 0;
}

bool Kleo::CryptoBackendFactory::hasBackends() const {
  return !mBackendList.empty();
}

void Kleo::CryptoBackendFactory::scanForBackends( QStringList * reasons ) {
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

const Kleo::CryptoBackend * Kleo::CryptoBackendFactory::backend( unsigned int idx ) const {
  return ( idx < mBackendList.size() ) ? mBackendList[idx] : 0 ;
}

const Kleo::CryptoBackend * Kleo::CryptoBackendFactory::backendByName( const QString& name ) const {
  for ( QValueVector<CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    if ( (*it)->name() == name )
      return *it;
  }
  return 0;
}

Kleo::BackendConfigWidget * Kleo::CryptoBackendFactory::configWidget( QWidget * parent, const char * name ) const {
  return new Kleo::BackendConfigWidget( mSelf, parent, name );
}

KConfig* Kleo::CryptoBackendFactory::configObject() const {
  if ( !mConfigObject )
    mConfigObject = new KConfig( "libkleopatrarc" );
  return mConfigObject;
}

void Kleo::CryptoBackendFactory::setSMIMEBackend( const CryptoBackend* backend ) {
  const QString name = backend ? backend->name() : QString::null;
  KConfigGroup group( configObject(), "Backends" );
  group.writeEntry( "SMIME", name );
  configObject()->sync();
  mSMIMEBackend = backend;
}

void Kleo::CryptoBackendFactory::setOpenPGPBackend( const CryptoBackend* backend ) {
  const QString name = backend ? backend->name() : QString::null;
  KConfigGroup group( configObject(), "Backends" );
  group.writeEntry( "OpenPGP", name );
  configObject()->sync();
  mOpenPGPBackend = backend;
}

void Kleo::CryptoBackendFactory::readConfig() {
  const KConfigGroup group( configObject(), "Backends" );
  const QString smimeBackend = group.readEntry( "SMIME", "gpgme" );
  mSMIMEBackend = backendByName( smimeBackend );

  const QString openPGPBackend = group.readEntry( "OpenPGP", "gpgme" );
  mOpenPGPBackend = backendByName( openPGPBackend );
}

#include "cryptobackendfactory.moc"

