/*
    cryptobackendfactory.cpp

    This file is part of libkleopatra, the KDE key management library
    Copyright (c) 2001,2004,2005 Klarälvdalens Datakonsult AB

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
#if 0 // disabled for kde-3.3
#include <backends/kpgp/pgp2backend.h>
#include <backends/kpgp/pgp5backend.h>
#include <backends/kpgp/pgp6backend.h>
#include <backends/kpgp/gpg1backend.h>
#endif
#ifdef KLEO_CHIASMUS
#include <backends/chiasmus/chiasmusbackend.h>
#endif
#include <ui/backendconfigwidget.h>

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <iterator>

#include <cassert>

Kleo::CryptoBackendFactory * Kleo::CryptoBackendFactory::mSelf = 0;

static const char * availableProtocols[] = {
#ifdef KLEO_CHIASMUS
  "Chiasmus",
#endif
  "OpenPGP", "SMIME",
};
static const unsigned int numAvailableProtocols = sizeof availableProtocols / sizeof *availableProtocols;

Kleo::CryptoBackendFactory::CryptoBackendFactory()
  : QObject( qApp, "CryptoBackendFactory::instance()" ),
    mConfigObject( 0 ),
    mAvailableProtocols( availableProtocols, availableProtocols + numAvailableProtocols )
{
  mBackendList.push_back( new QGpgMEBackend() );
#if 0 // disabled for kde-3.3
  mBackendList.push_back( new PGP2Backend() );
  mBackendList.push_back( new PGP5Backend() );
  mBackendList.push_back( new PGP6Backend() );
  mBackendList.push_back( new GPG1Backend() );
#endif
#ifdef KLEO_CHIASMUS
  mBackendList.push_back( new ChiasmusBackend() );
#endif
  scanForBackends();
  readConfig();

  mSelf = this; // last!
}

Kleo::CryptoBackendFactory::~CryptoBackendFactory() {
  mSelf = 0; // first!

  for ( std::vector<CryptoBackend*>::iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
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
  const BackendMap::const_iterator it = mBackends.find( "SMIME" );
  if ( it == mBackends.end() )
    return 0;
  if ( !it->second )
    return 0;
  return it->second->smime();
}

const Kleo::CryptoBackend::Protocol * Kleo::CryptoBackendFactory::openpgp() const {
  const BackendMap::const_iterator it = mBackends.find( "OpenPGP" );
  if ( it == mBackends.end() )
    return 0;
  if ( !it->second )
    return 0;
  return it->second->openpgp();
}

const Kleo::CryptoBackend::Protocol * Kleo::CryptoBackendFactory::protocol( const char * name ) const {
  const BackendMap::const_iterator it = mBackends.find( name );
  if ( it == mBackends.end() )
    return 0;
  if ( !it->second )
    return 0;
  return it->second->protocol( name );
}

Kleo::CryptoConfig * Kleo::CryptoBackendFactory::config() const {
  // ## should we use mSMIMEBackend? mOpenPGPBackend? backend(0) i.e. always qgpgme?
  return backend( 0 ) ? backend( 0 )->config() : 0;
}

bool Kleo::CryptoBackendFactory::hasBackends() const {
  return !mBackendList.empty();
}

void Kleo::CryptoBackendFactory::scanForBackends( QStringList * reasons ) {
  for ( std::vector<CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    assert( *it );
    for ( int i = 0 ; const char * protocol = (*it)->enumerateProtocols( i ) ; ++i ) {
      QString reason;
      if ( (*it)->supportsProtocol( protocol ) && !(*it)->checkForProtocol( protocol, &reason ) ) {
        if ( reasons ) {
          reasons->push_back( i18n("While scanning for %1 support in backend %2:")
                              .arg( protocol, (*it)->displayName() ) );
          reasons->push_back( "  " + reason );
        }
      }
    }
  }
}

const Kleo::CryptoBackend * Kleo::CryptoBackendFactory::backend( unsigned int idx ) const {
  return ( idx < mBackendList.size() ) ? mBackendList[idx] : 0 ;
}

const Kleo::CryptoBackend * Kleo::CryptoBackendFactory::backendByName( const QString& name ) const {
  for ( std::vector<CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
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
    // this is unsafe. We're a lib, used by concurrent apps.
    mConfigObject = new KConfig( "libkleopatrarc" );
  return mConfigObject;
}

void Kleo::CryptoBackendFactory::setSMIMEBackend( const CryptoBackend* backend ) {
  setProtocolBackend( "SMIME", backend );
}

void Kleo::CryptoBackendFactory::setOpenPGPBackend( const CryptoBackend* backend ) {
  setProtocolBackend( "OpenPGP", backend );
}

void Kleo::CryptoBackendFactory::setProtocolBackend( const char * protocol, const CryptoBackend * backend ) {
  const QString name = backend ? backend->name() : QString::null ;
  KConfigGroup group( configObject(), "Backends" );
  group.writeEntry( protocol, name );
  configObject()->sync();
  mBackends[protocol] = backend;
}

static const char * defaultBackend( const char * proto ) {
  static const struct {
    const char * proto;
    const char * backend;
  } defaults[] = {
    { "OpenPGP", "gpgme" },
    { "SMIME", "gpgme" },
#ifdef KLEO_CHIASMUS
    { "Chiasmus", "chiasmus" },
#endif
  };
  for ( unsigned int i = 0 ; i < sizeof defaults / sizeof *defaults ; ++i )
    if ( qstricmp( proto, defaults[i].proto ) == 0 )
      return defaults[i].backend;
  return 0;
}

void Kleo::CryptoBackendFactory::readConfig() {
  mBackends.clear();
  const KConfigGroup group( configObject(), "Backends" );
  for ( ProtocolSet::const_iterator it = mAvailableProtocols.begin(), end = mAvailableProtocols.end() ; it != end ; ++it ) {
    const QString backend = group.readEntry( *it, defaultBackend( *it ) );
    mBackends[*it] = backendByName( backend );
  }
}

const char * Kleo::CryptoBackendFactory::enumerateProtocols( int i ) const {
  if ( i < 0 || static_cast<unsigned int>( i ) >= mAvailableProtocols.size() )
    return 0;
  return mAvailableProtocols[i];
}

namespace {
  class CaseInsensitiveString {
    const char * m;
  public:
    CaseInsensitiveString( const char * s ) : m( s ) {}
#define make_operator( op ) \
    bool operator op( const CaseInsensitiveString & other ) const { \
      return qstricmp( m, other.m ) op 0; \
    } \
    bool operator op( const char * other ) const { \
      return qstricmp( m, other ) op 0; \
    }
    make_operator( == )
    make_operator( != )
    make_operator( < )
    make_operator( > )
    make_operator( <= )
    make_operator( >= )
#undef make_operator
    operator const char *() const { return m; }
  };
}

bool Kleo::CryptoBackendFactory::knowsAboutProtocol( const char * name ) const {
  return std::find( mAvailableProtocols.begin(), mAvailableProtocols.end(),
                    CaseInsensitiveString( name ) ) != mAvailableProtocols.end();
}

#include "cryptobackendfactory.moc"

