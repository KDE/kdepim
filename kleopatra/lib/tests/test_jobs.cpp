/*
    test_jobs.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include <kleo/cryptobackendfactory.h>
#include <kleo/signjob.h>
#include <kleo/keylistjob.h>

#include <gpgmepp/key.h>
#include <gpgmepp/signingresult.h>
#include <gpgmepp/keylistresult.h>

#include <kdebug.h>
#include <assert.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <memory>

static const char * protocol = 0;

static void testSign()
{
  const Kleo::CryptoBackend::Protocol * proto = protocol == "openpgp" ? Kleo::CryptoBackendFactory::instance()->openpgp() : Kleo::CryptoBackendFactory::instance()->smime() ;
  assert( proto );

  kdDebug() << "Using protocol " << proto->name() << endl;


  std::vector<GpgME::Key> signingKeys;

  std::auto_ptr<Kleo::KeyListJob> listJob( proto->keyListJob( false, false, true ) ); // use validating keylisting
  if ( listJob.get() ) {
      // ##### Adjust this to your own identity
      listJob->exec( "faure@kde.org", true /*secret*/, signingKeys );
      assert( !signingKeys.empty() );
  } else {
      assert( 0 ); // job failed
  }

  Kleo::SignJob* job = proto->signJob( true, true );

  QCString cText = "Hallo Leute\n"; // like gpgme's t-sign.c
  QByteArray plainText;
  plainText.duplicate( cText.data(), cText.length() ); // hrmpf...
  kdDebug() << k_funcinfo << "plainText=" << cText.data() << endl;

  kdDebug() << k_funcinfo << " signing with " << signingKeys[0].primaryFingerprint() << endl;

  QByteArray signature;
  const GpgME::SigningResult res =
    job->exec( signingKeys, plainText, GpgME::Context::Clearsigned, signature );
  if ( res.error().isCanceled() ) {
    kdDebug() << "signing was canceled by user" << endl;
    return;
  }
  if ( res.error() ) {
    kdDebug() << "signing failed: " << res.error().asString() << endl;
    return;
  }
  kdDebug() << k_funcinfo << "signing resulted in signature="
	    << QCString( signature.data(), signature.size() + 1 ) << endl;
}

int main( int argc, char** argv ) {
  protocol = "openpgp";
  if ( argc == 2 ) {
    protocol = argv[1];
    argc = 1; // hide from KDE
  }
  KAboutData aboutData( "test_jobs", "Signing Job Test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  testSign();
}
