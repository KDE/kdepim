/*
    This file is part of Kleopatra's test suite.
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#include <config.h>

#include <kleo/cryptobackendfactory.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/verifyopaquejob.h>

#include <gpgmepp/verificationresult.h>
#include <gpgmepp/key.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include <qdir.h>
#include <qfile.h>

#include <assert.h>

int main( int argc, char **argv )
{
  setenv("GNUPGHOME", KDESRCDIR "/gnupg_home", 1 );
  setenv("LC_ALL", "C", 1);
  setenv("KDEHOME", QFile::encodeName( QDir::homeDirPath() + "/.kde-unit-test" ), 1);

  KAboutData aboutData( "test_verify", "verify job test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app( false, false );

  const QString sigFileName = KDESRCDIR "/test.data.sig";
  const QString dataFileName = KDESRCDIR "/test.data";

  QFile sigFile( sigFileName );
  assert( sigFile.open( IO_ReadOnly ) );
  QFile dataFile( dataFileName );
  assert( dataFile.open( IO_ReadOnly ) );

  const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );

  Kleo::VerifyDetachedJob *job = backend->verifyDetachedJob();
  GpgME::VerificationResult result = job->exec( sigFile.readAll(), dataFile.readAll() );
  assert( !result.error() );
  assert( result.signatures().size() == 1 );

  GpgME::Signature sig = result.signature( 0 );
  assert( (sig.summary() & GpgME::Signature::KeyMissing) == 0 );
  assert( sig.creationTime() == 1189650248L );
  assert( sig.validity() == GpgME::Signature::Full );

  const QString opaqueFileName = KDESRCDIR "/test.data.gpg";
  QFile opaqueFile( opaqueFileName );
  assert( opaqueFile.open( IO_ReadOnly ) );
  QByteArray clearText;

  Kleo::VerifyOpaqueJob *job2 = backend->verifyOpaqueJob();
  result = job2->exec( opaqueFile.readAll(), clearText );
  assert( !result.error() );
  assert( result.signatures().size() == 1 );

  sig = result.signature( 0 );
  assert( (sig.summary() & GpgME::Signature::KeyMissing) == 0 );
  assert( (sig.summary() & GpgME::Signature::Green ) );
  assert( sig.creationTime() > 0 );
  assert( sig.validity() == GpgME::Signature::Full );

  dataFile.reset();
  assert( clearText == dataFile.readAll() );

  return 0;
}
