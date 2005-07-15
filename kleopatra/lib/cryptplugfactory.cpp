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

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kapplication.h>

#include <assert.h>

KMail::CryptPlugFactory * KMail::CryptPlugFactory::mSelf = 0;

//
//
// KMail::CryptPlugFactory: backwards compat stuff (ugly)
//
//


KMail::CryptPlugFactory::CryptPlugFactory()
  : Kleo::CryptoBackendFactory(),
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
  if ( p == "application/pkcs7-signature" || p == "application/x-pkcs7-signature" )
    return smime();
  if ( p == "application/pgp-signature" || p == "application/x-pgp-signature" )
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
  Kleo::CryptoBackendFactory::scanForBackends( reason );
  updateCryptPlugWrapperList();
}

void KMail::CryptPlugFactory::updateCryptPlugWrapperList() {
  mCryptPlugWrapperList->clear();
  for ( std::vector<Kleo::CryptoBackend*>::const_iterator it = mBackendList.begin() ; it != mBackendList.end() ; ++it ) {
    if ( CryptPlugWrapper * w = dynamic_cast<CryptPlugWrapper*>( (*it)->openpgp() ) )
      mCryptPlugWrapperList->append( w );
    if ( CryptPlugWrapper * w = dynamic_cast<CryptPlugWrapper*>( (*it)->smime() ) )
      mCryptPlugWrapperList->append( w );
  }
}

#include "cryptplugfactory.moc"
