/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmebackend.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004,2005 Klarälvdalens Datakonsult AB

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

#include "qgpgmebackend.h"

#include "qgpgmecryptoconfig.h"
#include "cryptplugwrapper.h"

#include <gpgmepp/context.h>
#include <gpgmepp/engineinfo.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include <qfile.h>
#include <qstring.h>

Kleo::QGpgMEBackend::QGpgMEBackend()
  : Kleo::CryptoBackend(),
    mCryptoConfig( 0 ),
    mOpenPGPProtocol( 0 ),
    mSMIMEProtocol( 0 )
{

}

Kleo::QGpgMEBackend::~QGpgMEBackend() {
  delete mCryptoConfig; mCryptoConfig = 0;
  delete mOpenPGPProtocol; mOpenPGPProtocol = 0;
  delete mSMIMEProtocol; mSMIMEProtocol = 0;
}

QString Kleo::QGpgMEBackend::name() const {
  return "gpgme";
}

QString Kleo::QGpgMEBackend::displayName() const {
  return i18n( "GpgME" );
}

Kleo::CryptoConfig * Kleo::QGpgMEBackend::config() const {
  if ( !mCryptoConfig ) {
    static bool hasGpgConf = !KStandardDirs::findExe( "gpgconf" ).isEmpty();
    if ( hasGpgConf )
      mCryptoConfig = new QGpgMECryptoConfig();
  }
  return mCryptoConfig;
}

static bool check( GpgME::Context::Protocol proto, QString * reason ) {
  if ( !GpgME::checkEngine( proto ) )
    return true;
  if ( !reason )
    return false;
  // error, check why:
  const GpgME::EngineInfo ei = GpgME::engineInfo( proto );
  if ( ei.isNull() )
    *reason = i18n("GPGME was compiled without support for %1.").arg( proto == GpgME::Context::CMS ? "S/MIME" : "OpenPGP" );
  else if ( ei.fileName() && !ei.version() )
    *reason = i18n("Engine %1 is not installed properly.").arg( QFile::decodeName( ei.fileName() ) );
  else if ( ei.fileName() && ei.version() && ei.requiredVersion() )
    *reason = i18n("Engine %1 version %2 installed, "
		   "but at least version %3 is required.")
      .arg( QFile::decodeName( ei.fileName() ), ei.version(), ei.requiredVersion() );
  else
    *reason = i18n("Unknown problem with engine for protocol %1.").arg( proto == GpgME::Context::CMS ? "S/MIME" : "OpenPGP" );
  return false;
}

bool Kleo::QGpgMEBackend::checkForOpenPGP( QString * reason ) const {
  return check( GpgME::Context::OpenPGP, reason );
}

bool Kleo::QGpgMEBackend::checkForSMIME( QString * reason ) const {
  return check( GpgME::Context::CMS, reason );
}

bool Kleo::QGpgMEBackend::checkForProtocol( const char * name, QString * reason ) const {
  if ( qstricmp( name, OpenPGP ) == 0 )
    return check( GpgME::Context::OpenPGP, reason );
  if ( qstricmp( name, SMIME ) == 0 )
    return check( GpgME::Context::CMS, reason );
  if ( reason )
    *reason = i18n( "Unsupported protocol \"%1\"" ).arg( name );
  return false;
}

Kleo::CryptoBackend::Protocol * Kleo::QGpgMEBackend::openpgp() const {
  if ( !mOpenPGPProtocol )
    if ( checkForOpenPGP() )
      mOpenPGPProtocol = new CryptPlugWrapper( "gpg", "openpgp" );
  return mOpenPGPProtocol;
}

Kleo::CryptoBackend::Protocol * Kleo::QGpgMEBackend::smime() const {
  if ( !mSMIMEProtocol )
    if ( checkForSMIME() )
      mSMIMEProtocol = new CryptPlugWrapper( "gpgsm", "smime" );
  return mSMIMEProtocol;
}

Kleo::CryptoBackend::Protocol * Kleo::QGpgMEBackend::protocol( const char * name ) const {
  if ( qstricmp( name, OpenPGP ) == 0 )
    return openpgp();
  if ( qstricmp( name, SMIME ) == 0 )
    return smime();
  return 0;
}

bool Kleo::QGpgMEBackend::supportsProtocol( const char * name ) const {
  return qstricmp( name, OpenPGP ) == 0 || qstricmp( name, SMIME );
}

const char * Kleo::QGpgMEBackend::enumerateProtocols( int i ) const {
  switch ( i ) {
  case 0: return OpenPGP;
  case 1: return SMIME;
  default: return 0;
  }
}
